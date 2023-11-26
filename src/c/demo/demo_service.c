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
#include "alg_sdk/alg_sdk.h"
#include "alg_sdk/service.h"

int main (int argc, char **argv)
{
    int rc;
    int timeout = 5000;

    if ((argc == 4) && (strncmp(argv[1], "-stream_on", 16) == 0) && (strncmp(argv[2], "-c", 4) == 0))
    {
        const char* c_ch = argv[3];

        int ch_sel[ALG_SDK_MAX_CHANNEL] = {0};
        int ch_ids[ALG_SDK_MAX_CHANNEL] = {0};

        int ch_cnt = 0;

        printf("command : stream on : [%s], len=%ld\n", c_ch, strlen(c_ch));

        for(int i=0; i < strlen(c_ch); i++)
        {   
            if (strncmp(c_ch+i, ",", 1) != 0)
            {
                ch_sel[i] = 1;
                ch_ids[i] = atoi(c_ch+i);
                printf("Stream on channel : %d\n", ch_ids[i]);
                ch_cnt++;
            }
        }

        /* Example : Stream On/Off */
        const char *topic_name = "/service/camera/stream_on";

        service_stream_control_t t1 = {
            .ack_mode = 1,
        };

        uint8_t sel[ALG_SDK_MAX_CHANNEL*2] = {0};
        uint8_t ctl[ALG_SDK_MAX_CHANNEL*2] = {0};

        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            if (ch_sel[i] == 1)
            {
                t1.select[ch_ids[i]] = 1;
                t1.control[ch_ids[i]] = 1;
            }

        }

        rc = alg_sdk_call_service(topic_name, &t1, timeout);
        if (rc < 0)
        {
            printf("Request Service : [%s] Error!\n", topic_name);
            return 0;
        }

        printf("[ack : %d], Channel SEL : \n", t1.ack_code);
        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            printf("%d|", t1.ch_sel[i]);
        }
        printf("\n");
        /* End */
    }
    else if ((argc == 2) && (strcmp(argv[1], "-stream_off") == 0))
    {
        /* Example : Stream On/Off */
        const char *topic_name = "/service/camera/stream_on";

        service_stream_control_t t1 = {
            .ack_mode = 1,
        };

        uint8_t sel[ALG_SDK_MAX_CHANNEL*2] = {0};
        uint8_t ctl[ALG_SDK_MAX_CHANNEL*2] = {0};

        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            t1.select[i] = 1;
            t1.control[i] = 0;
        }

        rc = alg_sdk_call_service(topic_name, &t1, timeout);
        if (rc < 0)
        {
            printf("Request Service : [%s] Error!\n", topic_name);
            return 0;
        }

        printf("[ack : %d], Channel SEL : \n", t1.ack_code);
        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            printf("%d|", t1.ch_sel[i]);
        }
        printf("\n");
        /* End */
    }
    else
    {
        fprintf(stderr, "Usage: %s -stream_on -c 0,1,2\n", argv[0]);
        fprintf(stderr, "Usage: %s -stream_off\n", argv[0]);
        exit(0);
    }


    return 0;
}
