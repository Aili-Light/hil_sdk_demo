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
#include "utils.h"
#include "image_feed.h"
#ifndef __MINGW32__
#include <netinet/in.h> /* For htonl and ntohl */
#endif

static uint8_t payload[ALG_SDK_MAX_CHANNEL][ALG_SDK_PAYLOAD_LEN_MAX];
static pcie_common_head_t img_header[ALG_SDK_MAX_CHANNEL];
static pcie_image_info_meta_t img_info[ALG_SDK_MAX_CHANNEL];
static pcie_image_data_t img_data[ALG_SDK_MAX_CHANNEL];
static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_count[ALG_SDK_MAX_CHANNEL] = {0};
static sem_t sem_push;
static pthread_t g_main_loop;
static pthread_mutex_t g_mutex;
static uint64_t g_timer_last;
static vector<string> img_file_lists[ALG_SDK_MAX_CHANNEL];
static uint32_t file_list_sizes[ALG_SDK_MAX_CHANNEL];
static vector<string>::iterator it_mulch[ALG_SDK_MAX_CHANNEL];
static uint8_t channel_ids[ALG_SDK_MAX_CHANNEL];
static ImageFeed image_feed[ALG_SDK_MAX_CHANNEL];

void int_handler(int sig)
{
    printf("Caught signal : %d\n", sig);
    alg_sdk_stop_server();

    pthread_mutex_destroy(&g_mutex);
    sem_destroy(&sem_push);

    exit(sig);
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
    uint32_t seq_ch[ALG_SDK_MAX_CHANNEL] = {0};

    int rc;
    /* Init Servers */
    rc = alg_sdk_init_server(0);
    if (rc < 0)
    {
        fatal("Init server failed\n");
    }
    /* end */

    if ((argc > 2) && (strcmp(argv[1], "--publish_multi") == 0))
    {
        const uint16_t num_channel = atoi(argv[2]);
        printf("num : %d\n", num_channel);
        if (num_channel < 1)
        {
            fatal("ERROR! Number of Channel is 0!! \n");
        }

        /* Init Parameters */
        memset(&image_feed, 0, sizeof(image_feed));
        memset(&channel_ids, 0, sizeof(channel_ids));

        for (int i = 0; i < num_channel; i++)
        {
            const char *filename = argv[5 * i + 3];
            const uint32_t image_width = atoi(argv[5 * i + 4]);
            const uint32_t image_height = atoi(argv[5 * i + 5]);
            const uint8_t channel_id = atoi(argv[5 * i + 6]);
            char data_type_c[64] = {"Default"};
            strncpy(data_type_c, argv[5 * i + 7], 12);
            printf("width=%d, height=%d, channel=%d, data type=%s\n", image_width, image_height, channel_id, data_type_c);

            const uint32_t image_size = image_width * image_height * 2;
            channel_ids[i] = channel_id;

            uint32_t p_len;
            if (load_image(filename, payload[i], &p_len))
            {
                fatal("Read File Error!\n");
            }

            if (p_len == 0)
            {
                fatal("Data Size Error!\n");
            }

            if (image_size != p_len)
            {
                fatal("Image Size Not Match!\n");
            }

            /* Generate pcie image data  */
            image_feed[i].init_feed(image_width, image_height, channel_id, image_size, 0);
            image_feed[i].make_data_struct();
            /* end */

            /* Set up data type */
            int data_type = image_feed[i].set_data_type(data_type_c);
            if (data_type == 0)
            {
                fatal("Wrong Data Type!");
            }
            /* end */
        }

        char topic_name[ALG_SDK_MAX_CHANNEL][256];
        for (int i=0; i<num_channel; i++)
        {
            snprintf(topic_name[i], 256, "/feedback/hil_message/%02d", channel_ids[i]);
            rc = alg_sdk_subscribe(topic_name[i], callback_hil_message);
            if (rc < 0)
            {
                fatal("Subscribe to topic Error!\n");
            }
        }
        if (alg_sdk_init_client())
        {
            fatal("Init Client Error!\n");
        }

        float freq = 30.0f;
        g_timer_last = macroseconds();
        /* end */

        /* Main Loop */
        while (1)
        {
            /* Set frequency  */
            uint64_t t_now, delta_t;
            t_now = macroseconds();
            delta_t = t_now - g_timer_last;

            if (delta_t < 1000000 / freq)
                continue;

            g_timer_last = t_now;

            for (int i = 0; i < num_channel; i++)
            {
                uint64_t timestamp = milliseconds();

                image_feed[i].feed_data((uint8_t *)payload[i], seq_ch[i], timestamp);
                void *img_data_ptr = image_feed[i].get_data_ptr();
                // pcie_image_data_t *data_p = (pcie_image_data_t *)img_data_ptr;
                // printf("topic : %s, index : [%d]\n", data_p->common_head.topic_name, data_p->image_info_meta.frame_index);
                alg_sdk_push2q(img_data_ptr, channel_ids[i]);
                // frame_monitor(channel_id, &fps, seq);
                seq_ch[i]++;
            }
            usleep(10000);
        }
        /* end */
    }
    else if ((argc > 2) && (strcmp(argv[1], "--feedin_multi") == 0))
    {
        const uint16_t num_channel = atoi(argv[2]);
        if (num_channel < 1)
        {
            fatal("ERROR! Number of Channel is 0!! \n");
        }

        /* Init Parameters */
        memset(&image_feed, 0, sizeof(image_feed));
        memset(&img_file_lists, 0, sizeof(img_file_lists));
        memset(&file_list_sizes, 0, sizeof(file_list_sizes));
        memset(&channel_ids, 0, sizeof(channel_ids));

        for (int i = 0; i < num_channel; i++)
        {
            const char *foldername = argv[5 * i + 3];
            const uint32_t image_width = atoi(argv[5 * i + 4]);
            const uint32_t image_height = atoi(argv[5 * i + 5]);
            const uint8_t channel_id = atoi(argv[5 * i + 6]);
            char data_type_c[64] = {"Default"};
            strncpy(data_type_c, argv[5 * i + 7], 12);
            printf("width=%d, height=%d, channel=%d, data type=%s\n", image_width, image_height, channel_id, data_type_c);

            const uint32_t image_size = image_width * image_height * 2;
            channel_ids[i] = channel_id;

            /* Generate pcie image data  */
            image_feed[i].init_feed(image_width, image_height, channel_id, image_size, 0);
            image_feed[i].make_data_struct();
            /* end */

            /* Set up data type */
            int data_type = image_feed[i].set_data_type(data_type_c);
            if (data_type == 0)
            {
                fatal("Wrong Data Type!");
            }
            /* end */

            /* Read files from folder */
            printf("Open folder : %s\n", foldername);
            string folder_name = foldername;
            load_image_path(foldername, img_file_lists[i]);
            file_list_sizes[i] = img_file_lists[i].size();
            /* end */

            if (file_list_sizes[i] > 0)
            {
                it_mulch[i] = img_file_lists[i].begin();
            }
            else
            {
                fatal("Read image list error!\n");
            }
        }

        char topic_name[ALG_SDK_MAX_CHANNEL][256];
        for (int i=0; i<num_channel; i++)
        {
            snprintf(topic_name[i], 256, "/feedback/hil_message/%02d", channel_ids[i]);
            rc = alg_sdk_subscribe(topic_name[i], callback_hil_message);
            if (rc < 0)
            {
                fatal("Subscribe to topic Error!\n");
            }
        }
        if (alg_sdk_init_client())
        {
            fatal("Init Client Error!\n");
        }

        float freq = 30.0f;
        uint32_t seq = 0;
        uint32_t p_len = 0;
        g_timer_last = macroseconds();
        /* end */

        /* Main Loop */
        while (1)
        {
            /* Set frequency  */
            uint64_t t_now, delta_t;
            t_now = macroseconds();
            delta_t = t_now - g_timer_last;

            if (delta_t < 1000000 / freq)
                continue;

            g_timer_last = t_now;

            /* Feed data on each channel */
            for (int i = 0; i < num_channel; i++)
            {
                /* read image data from file */
                const char *filename = (*it_mulch[i]).c_str();
                load_image(filename, payload[i], &p_len);
                // printf("[SEQ:%d] [FILE:%s] [LEN:%d]\n", seq, filename, p_len);

                if (p_len != image_feed[i].get_image_size()) // image size does not match
                {
                    fatal("Image size do not match!\n");
                }

                uint64_t timestamp = milliseconds();

                image_feed[i].feed_data((uint8_t *)payload[i], seq, timestamp);
                void *img_data_ptr = image_feed[i].get_data_ptr();
                // pcie_image_data_t *data_p = (pcie_image_data_t *)img_data_ptr;
                // printf("topic : %s, index : [%d]\n", data_p->common_head.topic_name, data_p->image_info_meta.frame_index);
                alg_sdk_push2q(img_data_ptr, channel_ids[i]);

                it_mulch[i]++;
                /* iterator never reach the end */
                if (it_mulch[i] == img_file_lists[i].end())
                {
                    // printf("reach end [%d]\n", i);
                    it_mulch[i] = img_file_lists[i].begin();
                }
            }
            usleep(10000);

            /* update sequence */
            seq++;
        }
        /* end */

        alg_sdk_server_spin_on();
        alg_sdk_client_spin_on();
    }
    else
    {
        fprintf(stderr, "Usage: ./hil_sdk_demo_multi_ch_ch <TYP> <NUM_CHANNEL> <FILENAME> <ARG1> <ARG2> <ARG3> <ARG4>...\n");
        fprintf(stderr, "e.g. ./hil_sdk_demo_multi_ch_ch --publish_multi 2 'test_image_0.yuv' 1920 1280 0 'YUYV' 'test_image_1' 3840 2160 1 'RAW12'\n");
        fprintf(stderr, "e.g. ./hil_sdk_demo_multi_ch_ch --feedin_multi 2 'folder0/' 1920 1280 0 'YUYV' 'folder1' 3840 2160 1 'RAW12'\n");
    }

    return 0;
}