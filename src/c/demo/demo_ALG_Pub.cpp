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
#include "utils/utils.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <thread>
#include "device_handler/HIL_device_fromALGPub.h"

#ifdef __cplusplus
extern "C" {
#endif
using json = nlohmann::json;

bool b_start_main_loop = true;
HILDeviceFromALGPub* hil_device;
static uint8_t payload[ALG_SDK_MAX_CHANNEL][ALG_SDK_PAYLOAD_LEN_MAX];

void int_handler(int sig)
{
    printf("Keyboard Interrupt : %d\n", sig);
    b_start_main_loop = false;

    // Close Stream
    hil_device->CloseStreamAll();
}

void callback(void* data)
{
    hil_mesg_t *msg = (hil_mesg_t*)data;
    // printf("MSG : %s\n", msg->msg_meta.msg);
    printf("ALG HIL CB [time : %ld], [ch : %d], [Frame : %d], [Count : %d/%d]\n", msg->msg_meta.timestamp, msg->msg_meta.ch_id, 
    msg->msg_meta.frame_index, msg->msg_meta.buffer_count, msg->msg_meta.buffer_len);
}

void send_data(int ch_id)
{
    uint32_t m_seq = 0;

    while (b_start_main_loop)
    {
        uint64_t t_now = macroseconds();
        hil_device->Send(&payload[ch_id][0], m_seq, t_now, ch_id, 0);
        printf("*** publish image on CH : %d, Frame : %d\n", ch_id, m_seq);
        std::this_thread::sleep_for(std::chrono::microseconds(33333));
        m_seq++;
    }

    printf("thread send_data finish.\n");
}

int main (int argc, char **argv)
{
    signal(SIGINT, int_handler);

    if (argc > 1)
    {
        const char*   config_file=argv[1];
        std::vector<std::thread*> thread_list;
        std::vector<int> channel_ids;

        std::ifstream f(config_file);

        json config = json::parse(f);
        if (!config.contains(std::string{"num_channels"}))
        {
            fprintf(stderr,  "json must contain key [num_channels] !.\n");
            exit(1);
        }

        /* get channel number */
        int num_channel = config["num_channels"].get<int>();
        printf("num channels : %d\n", num_channel);
        if (num_channel < 1)
        {
            fprintf(stderr, "ERROR! Number of Channel is incorrect!");
            exit(1);
        }

        if (!config.contains(std::string{"channels"}))
        {
            fprintf(stderr,  "json must contain key [channels] !.\n");
            exit(1);
        }

        /* get source file */
        std::string s_filename;
        for (auto cfg : config["channels"])
        {
            int ch_id = 0;
            if (cfg.contains(std::string{"channel_id"}))
            {
                ch_id = cfg["channel_id"].get<int>();
                channel_ids.push_back(ch_id);
            }
            else
            {
                fprintf(stderr,  "json must contain key [channel_id] !.\n");
                exit(1);
            }

            if (cfg.contains(std::string{"filename"}))
            {
                s_filename = cfg["filename"].get<std::string>();
                int ret = 0;
                uint32_t m_payload_len = 0;
                ret = load_image(s_filename.c_str(), &payload[ch_id][0], &m_payload_len);
                printf("load image :%s %d\n", s_filename.c_str(), m_payload_len);
                if (ret)
                {
                    fprintf(stderr,  "load image failed!\n");
                    exit(1);
                }
            }
            else
            {
                fprintf(stderr,  "json must contain key [filename] !.\n");
                exit(1);
            }
        }

        /* Create Instance of HIL Device */
        hil_device = HILDeviceFromALGPub::GetInstance();

        /* Register Devices */
        for (int i = 0; i < num_channel; i++)
        {
            VideoSourceParam param;
            param.source_id = i;
            memcpy(param.config_file, config_file, strlen(config_file)+1);
            hil_device->RegisterDevice(&param);
        }

        /* Set Callback Function */
        hil_device->SetCallbackFunc(callback);

        /* Init HIL Device */
        if(!hil_device->Init())
        {
            fprintf(stderr, "Init device failed");
            exit(1);
        }

        // Start Stream
        hil_device->StartStreamAll();

        // Start send data thread
        for (int i = 0; i < num_channel; i++)
        {
            std::thread* p_thread = new std::thread(send_data, channel_ids[i]);
            thread_list.push_back(p_thread);
            p_thread->detach();
        }

        // Wait Until Stream Finish
        hil_device->Wait();

        // Release Device
        hil_device->Release();
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
