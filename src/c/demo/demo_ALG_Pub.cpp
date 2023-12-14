/*
 The MIT License (MIT)

Copyright (c) 2022 Aili-Light. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include "alg_sdk/alg_sdk.h"
#include "alg_sdk/server.h"
#include "utils/utils.h"
#ifdef __cplusplus
extern "C" {
#endif

bool g_ready = false;

void int_handler(int sig)
{
    g_ready = false;
    /* stop server */
    alg_sdk_stop_server();
    /* terminate program */
    exit(sig);
}

int main (int argc, char **argv)
{
    signal(SIGINT, int_handler);

    if (argc > 1)
    {
        const char*   image_file_path = argv[1];
        const uint8_t channel_id = atoi(argv[2]);

        /* Init Servers */
        int rc = 0;
        rc = alg_sdk_init_server(ALG_SDK_SERVER_FLAG_PUB2USER);
        if (rc < 0)
        {
            printf("Init server failed\n");
            return false;
        }

        uint8_t* p_payload = (uint8_t*)malloc(sizeof(uint8_t) * ALG_SDK_PAYLOAD_LEN_MAX);
        uint32_t m_payload_len = 0;
        int ret = 0;
        uint32_t m_seq = 0;
        ret = load_image(image_file_path, p_payload, &m_payload_len);

        pcie_common_head_t img_header;
        img_header.head = 55;
        img_header.version = 1;
        char topic_name[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {};
        sprintf(topic_name, "/image_data/output/%02d", channel_id);
        strncpy(img_header.topic_name, topic_name, ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN);
        img_header.crc8 = crc_array((unsigned char *)&img_header, 130);

        pcie_image_info_meta_t img_info;
        img_info.frame_index = 0;
        img_info.width = 1920;
        img_info.height = 1080;
        img_info.data_type = ALG_SDK_MIPI_DATA_TYPE_YUYV;
        img_info.exposure = 0;
        img_info.again = 0;
        img_info.dgain = 0;
        img_info.temp = 0;
        img_info.timestamp = 0;
        img_info.img_size = m_payload_len;

        pcie_image_data_t img_data;
        g_ready = true;
        while (g_ready)
        {
            img_info.frame_index = m_seq;

            img_data.common_head = img_header;
            img_data.image_info_meta = img_info;
            img_data.payload = p_payload;
            alg_sdk_push2q(&img_data, channel_id);
            
            printf("*** publish image on CH : %d, Frame : %d\n", channel_id, m_seq);
            usleep(33333);
            m_seq++;
        }

        alg_sdk_server_spin_on();
        free(p_payload);
    }
    else
    {
        fprintf(stderr, "Usage: %s <image_file> <channel_id>", argv[0]);
        exit(0);
    }


    return 0;
}
#ifdef __cplusplus
}
#endif
