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
#include "device_handler_video/HIL_device_fromVideo.h"

void callback(void* data)
{
    hil_mesg_t *msg = (hil_mesg_t*)data;
    // printf("MSG : %s\n", msg->msg_meta.msg);
    printf("ALG HIL CB [time : %ld], [ch : %d], [Frame : %d], [Count : %d/%d]\n", msg->msg_meta.timestamp, msg->msg_meta.ch_id, 
    msg->msg_meta.frame_index, msg->msg_meta.buffer_count, msg->msg_meta.buffer_len);
}

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        const uint16_t num_channel = atoi(argv[1]);
        printf("num channels : %d\n", num_channel);
        if (num_channel < 1)
        {
            fprintf(stderr, "ERROR! Number of Channel is incorrect!");
            exit(1);
        }

        /* Create Instance of HIL Device */
        HILDeviceFromVideo *hil_device = HILDeviceFromVideo::GetInstance();
        
        /* Register Devices */
        for (int i = 0; i < num_channel; i++)
        {
            const char *config_file = argv[2];
            VideoSourceParam param;
            param.source_id = i;
            memcpy(param.config_file, config_file, strlen(config_file)+1);

            hil_device->RegisterDevice(&param);
        }

        /* Set HIL Device Parameters */
        if (argc > 3)
        {
            // printf("argv : %s, %s\n", argv[3], argv[4]);
            for (int i=3; i <(argc-1); )
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
        while(1)
        {
            usleep(1000);
        }

        // Wait Until Stream Finish
        hil_device->Wait();
    }
    else
    {
        fprintf(stderr, "Usage: %s <num_channel> <config_file>", argv[0]);
        exit(0);
    }

    return 0;
}