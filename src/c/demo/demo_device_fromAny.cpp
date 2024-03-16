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
#include <unistd.h>
#include <signal.h>
#include "nlohmann/json.hpp"
#include <fstream>
#include "device_handler/HIL_device_fromFile.h"
#include "device_handler/HIL_device_fromDir.h"
#include "device_handler/HIL_device_fromALGPub.h"
#include "device_handler/HIL_device_fromTCP.h"
#ifdef WITH_OPENCV
#include "device_handler_image/HIL_device_fromImage.h"
#endif
#ifdef WITH_FFMPEG
#include "device_handler_video/HIL_device_fromVideo.h"
#endif

using json = nlohmann::json;

bool b_start_main_loop = true;
ALGInputDevice* hil_device;

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

int main(int argc, char **argv)
{
    signal(SIGINT, int_handler);

    if (argc > 1)
    {
        /* parse json file */
        const char* config_file=argv[1];
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

        /* get source type */
        std::string s_source_type;
        for (auto cfg : config["channels"])
        {
            if (cfg.contains(std::string{"source_type"}))
            {
                s_source_type = cfg["source_type"].get<std::string>();
                break;
            }
        }

        /* Create Instance of HIL Device */
        if(s_source_type == "File")
        {
            hil_device = HILDeviceFromFile::GetInstance();
        }
        else if(s_source_type == "Dir")
        {
            hil_device = HILDeviceFromDir::GetInstance();
        }
        else if(s_source_type == "ALG Pub")
        {
            hil_device = HILDeviceFromALGPub::GetInstance();
        }
        else if(s_source_type == "TCP")
        {
            hil_device = HILDeviceFromTCP::GetInstance();
        }
#ifdef WITH_OPENCV
        else if(s_source_type == "Image")
        {
            hil_device = HILDeviceFromImage::GetInstance();
        }
#endif
#ifdef WITH_FFMPEG
        else if(s_source_type == "Video")
        {
            hil_device = HILDeviceFromVideo::GetInstance();
        }
#endif
        else
        {
            fprintf(stderr,  "source type [%s] does not match any case !\n", s_source_type.c_str());
            exit(1);
        }

        /* Register Devices */
        for (int i = 0; i < num_channel; i++)
        {
            VideoSourceParam param;
            param.source_id = i;
            memcpy(param.config_file, config_file, strlen(config_file)+1);
            hil_device->RegisterDevice(&param);
        }

        /* Set HIL Device Parameters */
        if (argc > 2)
        {
            // printf("argv : %s, %s\n", argv[3], argv[4]);
            for (int i=2; i <(argc-1); )
            {
                if (strncmp(argv[i], "--debug_level", strlen("--debug_level")) == 0)
                {
                    int debug_level=atoi(argv[i+1]);
                    printf("set debug level:%d\n", debug_level);
                    hil_device->SetLogLevel(debug_level);
                }
                i=i+2;
            }
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

        // Main Loop
        while(b_start_main_loop)
        {
            usleep(1000);
        }

        // Wait Until Stream Finish
        hil_device->Wait();

        // Release Device
        hil_device->Release();
    }
    else
    {
        fprintf(stderr, "Usage: %s <num_channel> <config_file>", argv[0]);
        exit(0);
    }

    return 0;
}