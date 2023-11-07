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
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "alg_sdk/alg_sdk.h"
#include "alg_sdk/server.h"
#include "alg_sdk/client.h"
#include "alg_sdk/pull.h"
#include "alg_common/basic_types.h"
#include "image_feed.h"
#include "utils.h"
#ifndef __MINGW32__
#include <netinet/in.h> /* For htonl and ntohl */
#endif

static uint8_t payload[ALG_SDK_PAYLOAD_LEN_MAX];
static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_count[ALG_SDK_MAX_CHANNEL] = {0};
static sem_t sem_push;
static pthread_t g_main_loop;
static pthread_mutex_t g_mutex;
static uint64_t g_timer_last;

void int_handler(int sig)
{
    printf("Caught signal : %d\n", sig);
    alg_sdk_stop_server();

    pthread_mutex_destroy(&g_mutex);
    sem_destroy(&sem_push);

    exit(sig);
}

void frame_monitor(const int ch_id, float *fps, const int frame_index)
{
    /* Monitor Frame Drop : current frame should be last frame +1,
        otherwise some frames may be lost.
    */

    /* Calculate Frame Rate */
    uint64_t t_now = milliseconds();
    g_f_count[ch_id]++;

    uint64_t delta_t = t_now - g_t_last[ch_id];
    if (delta_t > 1000) // for 1000 milliseconds
    {
        g_t_last[ch_id] = t_now;
        *fps = (float)g_f_count[ch_id] / delta_t * 1000.0f;
        printf("Frame Monitor : [Channel %d] [Index %d] [frm rate (SW) = %f]\n", ch_id, frame_index, *fps);
        g_f_count[ch_id] = 0;
    }
}

void callback_hil_message(void *p)
{
    hil_mesg_t *msg = (hil_mesg_t*)p;
    // printf("MSG : %s\n", msg->msg_meta.msg);
    printf("[time : %ld], [ch : %d], [Frame : %d], [Count : %d]\n", msg->msg_meta.timestamp, msg->msg_meta.ch_id, 
    msg->msg_meta.frame_index, msg->msg_meta.buffer_count);
}

int main(int argc, char **argv)
{
    int rc;

    /* Init Servers */
    rc = alg_sdk_init_server(0);
    if (rc < 0)
    {
        fatal("Init server failed\n");
    }

    if ((argc > 2) && (strcmp(argv[1], "--publish") == 0))
    {
        char *filename = argv[2];
        uint32_t p_len;
        const uint32_t image_width = atoi(argv[3]);
        const uint32_t image_height = atoi(argv[4]);
        const uint8_t channel_id = atoi(argv[5]);
        const uint32_t image_size = image_width * image_height * 2;
        float freq = 30.0f;

        /* User defined Data Type
         *  Default=YUYV (0x1E)
         *  Acceptable: UYVY (0x1C) / VYUY (0x1D) / YUYV (0x1E) / YVYU ((0x1F) / RAW10 (0x2B) / RAW12 (0x2C)
         */
        char data_type_c[64] = {"Default"};
        if (argc > 6)
        {
            // printf("argc : %s\n", argv[6]);
            strncpy(data_type_c, argv[6], 12);
            printf("data type : [%s]\n", data_type_c);
        }

        if (argc > 7)
        {
            freq = atof(argv[7]);
            if (freq <= 0)
            {
                freq = 1.0f;
            }

            printf("frequency = [%f]\n", freq);
        }

        if (load_image(filename, payload, &p_len))
        {
            fatal("Read File Error!\n");
        }

        if (!payload)
        {
            fatal("Empty data.\n");
        }

        if (p_len == 0)
        {
            fatal("Data Size Error!\n");
        }

        if (image_size != p_len)
        {
            fatal("Image Size Not Match!\n");
        }

        /* Generate pcie image data */
        ImageFeed image_feed;
        image_feed.init_feed(image_width, image_height, channel_id, image_size, 0, freq);
        image_feed.make_data_struct();
        /* end */

        /* Set up data type */
        int data_type = image_feed.set_data_type(data_type_c);
        if (data_type == 0)
        {
            fatal("Wrong Data Type!");
        }
        /* end */

        /* Set up feedback */
        char topic_name[256];
        snprintf(topic_name, 256, "/feedback/hil_message/%02d", channel_id);
        rc = alg_sdk_subscribe(topic_name, callback_hil_message);
        if (rc < 0)
        {
            fatal("Subscribe to topic Error!\n");
        }
        if (alg_sdk_init_client())
        {
            fatal("Init Client Error!\n");
        }
        /* end */

        /* Main loop : Feed Image */
        uint32_t seq = 0;
        uint64_t t_now = 0;
        g_timer_last = macroseconds();

        while (1)
        {
            /* Set frequency  */
            uint64_t delta_t;
            t_now = macroseconds();
            delta_t = t_now - g_timer_last;
            if (delta_t < 1000000 / freq)
                continue;

            g_timer_last = t_now;

            t_now = milliseconds();
            image_feed.feed_data((uint8_t *)payload, seq, t_now);
            void *img_data_ptr = image_feed.get_data_ptr();
            // pcie_image_data_t *data_p = (pcie_image_data_t *)img_data_ptr;
            // printf("topic : %s, index : [%d]\n", data_p->common_head.topic_name, data_p->image_info_meta.frame_index);
            alg_sdk_push2q(img_data_ptr, channel_id);

            // frame_monitor(channel_id, &fps, seq);
            usleep(10000);
            seq++;
        }

        alg_sdk_server_spin_on();
        alg_sdk_client_spin_on();
    }
    else if ((argc > 2) && (strcmp(argv[1], "--feedin") == 0))
    {
        const char *foldername = argv[2];
        const uint32_t image_width = atoi(argv[3]);
        const uint32_t image_height = atoi(argv[4]);
        const uint8_t channel_id = atoi(argv[5]);
        const uint32_t image_size = image_width * image_height * 2;
        float freq = 30.0f;

        char data_type_c[64] = {"Default"};
        if (argc > 6)
        {
            // printf("argc : %s\n", argv[6]);
            strncpy(data_type_c, argv[6], 12);
            printf("data type : [%s]\n", data_type_c);
        }

        if (argc > 7)
        {
            freq = atof(argv[7]);
            if (freq <= 0)
            {
                freq = 1.0f;
            }

            printf("frequency = [%f]\n", freq);
        }

        /* Generate pcie image data */
        ImageFeed image_feed;
        image_feed.init_feed(image_width, image_height, channel_id, image_size, 0, freq);
        image_feed.make_data_struct();
        /* end */

        /* Set up data type */
        int data_type = image_feed.set_data_type(data_type_c);
        if (data_type == 0)
        {
            fatal("Wrong Data Type!");
        }
        /* end */

        /* Set up feedback */
        char topic_name[256];
        snprintf(topic_name, 256, "/feedback/hil_message/%02d", channel_id);
        rc = alg_sdk_subscribe(topic_name, callback_hil_message);
        if (rc < 0)
        {
            fatal("Subscribe to topic Error!\n");
        }
        if (alg_sdk_init_client())
        {
            fatal("Init Client Error!\n");
        }
        /* end */

        /* Read files from folder */
        printf("Open folder : %s\n", foldername);
        string folder_name = foldername;
        vector<string> img_filenames;
        load_image_path(foldername, img_filenames);
        uint32_t v_size = img_filenames.size();
        /* end */

        if (v_size > 0)
        {
            uint32_t seq = 0;
            uint32_t p_len = 0;

            uint8_t *payload = (uint8_t *)malloc(sizeof(uint8_t) * image_size);

            g_timer_last = macroseconds();
            for (vector<string>::iterator it = img_filenames.begin();;)
            {
                /* Set frequency  */
                uint64_t t_now, delta_t;
                t_now = macroseconds();
                delta_t = t_now - g_timer_last;
                if (delta_t < 1000000 / freq)
                    continue;

                g_timer_last = t_now;

                /* read image data from file */
                const char *filename = (*it).c_str();
                load_image(filename, payload, &p_len);
                // printf("[SEQ:%d] [FILE:%s] [LEN:%d]\n", seq, filename, p_len);

                if (p_len != image_size) // image size does not match
                    break;

                /* copy data into ringbuffer */
                uint64_t timestamp = milliseconds();
                image_feed.feed_data((uint8_t *)payload, seq, timestamp);
                void *img_data_ptr = image_feed.get_data_ptr();
                // pcie_image_data_t *data_p = (pcie_image_data_t *)img_data_ptr;
                // printf("topic : %s, index : [%d]\n", data_p->common_head.topic_name, data_p->image_info_meta.frame_index);
                alg_sdk_push2q(img_data_ptr, channel_id);

                /* update sequence */
                seq++;
                it++;

                /* iterator never reach the end */
                if (it == img_filenames.end())
                {
                    it = img_filenames.begin();
                }

                usleep(25000);
            }

            safe_free(payload);
        }
        else
        {
            fatal("ERROR! folder is empty\n");
        }

        alg_sdk_server_spin_on();
    }
    else
    {
        fprintf(stderr, "Usage: ./hil_sdk_demo_push_1_ch <TYP> <FILENAME> <WIDTH> <HEIGHT> <CHANNEL> <DATA_TYPE> <FREQ>\n");
        fprintf(stderr, "e.g. ./hil_sdk_demo_push_1_ch --publish 'test_image.yuv' 1920 1280 0 'YUYV' 30\n");
        fprintf(stderr, "e.g. ./hil_sdk_demo_push_1_ch --feedin '/image_folder' 3840 2160 1 'RAW12' 30\n");
    }

    return 0;
}