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
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <fstream>
#include "alg_sdk/alg_sdk.h"
#include "alg_sdk/server.h"
#include "alg_sdk/pull.h"
#include "alg_common/basic_types.h"
#include "utils.h"
#include "qcap/qcapdev.h"

bool g_signal_recieved = false;
QCapDev *g_capture;
pcie_common_head_t g_img_header;
pcie_image_info_meta_t g_img_info;
pcie_image_data_t g_img_data;
uint32_t g_seq = 0;

void int_handler(int sig)
{
    g_signal_recieved = true;

    alg_sdk_stop_server();

    /* terminate program */
    exit(sig);
}

uint64_t get_time_unix()
{
    uint64_t time_us;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_us = (((uint64_t)tv.tv_sec * 1000) + ((uint64_t)tv.tv_usec / 1000));

    return time_us;
}

QRETURN on_video_preview_callback(PVOID pDevice, double dSampleTime, BYTE *pFrameBuffer, ULONG nFrameBufferLen, PVOID pUserData)
{
    ULONG i = (uintptr_t)pUserData;
    PVOID pRCBuffer;
    qcap_av_frame_t *pAVFrame;
    int ch_id = i;
    char filename_raw[128] = {};

    uint64_t t_now = get_time_unix();

    pcie_image_info_meta_t *img_info = &g_img_info;
    img_info->frame_index = g_seq++;
    img_info->timestamp = t_now;

    // printf("Time:%lu\n", t_now);
    pRCBuffer = QCAP_BUFFER_GET_RCBUFFER(pFrameBuffer, nFrameBufferLen);
    pAVFrame = (qcap_av_frame_t *)QCAP_RCBUFFER_LOCK_DATA(pRCBuffer);

    uint32_t img_size = pAVFrame->nPitch[0] * g_capture->m_nVideoHeight;

    pcie_image_data_t *img_data = &g_img_data;
    img_data->image_info_meta = *img_info;
    if (img_size != img_data->image_info_meta.img_size)
    {
        printf("Image Size Not Match!!!\n");
        QCAP_RCBUFFER_UNLOCK_DATA(pRCBuffer);
        return QCAP_RT_FAIL;
    }

    memcpy(img_data->payload, pAVFrame->pData[0], img_size);
    // img_data->payload = (uint8_t *)pAVFrame->pData[0];
    printf("SEQ : %d, TS : %ld, size : %d\n", g_seq, img_data->image_info_meta.timestamp,
           img_data->image_info_meta.img_size);

    alg_sdk_push2q(img_data, ch_id);

    if (g_capture->m_Flags == QCapDev::WriteFrame)
    {
        sprintf(filename_raw, "data/image_%02d_%lu.raw", ch_id, t_now);
        FILE *fp = fopen(filename_raw, "wb");
        fwrite(pAVFrame->pData[0], 1, pAVFrame->nPitch[0] * g_capture->m_nVideoHeight, fp);
        fclose(fp);
    }

    QCAP_RCBUFFER_UNLOCK_DATA(pRCBuffer);

    // printf("Data [Id:%ld] [TS:%f] [Len:%ld] [Width:%d] [Height:%d] [Buff0:%d]\n", i, dSampleTime, nFrameBufferLen, pAVFrame->nWidth, pAVFrame->nHeight, pAVFrame->pData[0]);

    return QCAP_RT_OK;
}

QRETURN on_process_format_changed(PVOID pDevice, ULONG nVideoInput, ULONG nAudioInput, ULONG nVideoWidth, ULONG nVideoHeight, BOOL bVideoIsInterleaved, double dVideoFrameRate, ULONG nAudioChannels, ULONG nAudioBitsPerSample, ULONG nAudioSampleFrequency, PVOID pUserData)
{
    ULONG i = (uintptr_t)pUserData;

    printf("format changed Detecte \n");

    CHAR strVideoInput[64] = {0};

    CHAR strAudioInput[64] = {0};

    CHAR strFrameType[64] = {0};

    UINT nVH = 0;

    if (nVideoInput == 0)
    {
        sprintf(strVideoInput, "COMPOSITE");
    }

    if (nVideoInput == 1)
    {
        sprintf(strVideoInput, "SVIDEO");
    }

    if (nVideoInput == 2)
    {
        sprintf(strVideoInput, "HDMI");
    }

    if (nVideoInput == 3)
    {
        sprintf(strVideoInput, "DVI_D");
    }

    if (nVideoInput == 4)
    {
        sprintf(strVideoInput, "COMPONENTS (YCBCR)");
    }

    if (nVideoInput == 5)
    {
        sprintf(strVideoInput, "DVI_A (RGB / VGA)");
    }

    if (nVideoInput == 6)
    {
        sprintf(strVideoInput, "SDI");
    }

    if (nVideoInput == 7)
    {
        sprintf(strVideoInput, "AUTO");
    }

    if (nAudioInput == 0)
    {
        sprintf(strAudioInput, "EMBEDDED_AUDIO");
    }

    if (nAudioInput == 1)
    {
        sprintf(strAudioInput, "LINE_IN");
    }

    if (nAudioInput == 2)
    {
        sprintf(strAudioInput, "SOUNDCARD_MICROPHONE");
    }

    if (nAudioInput == 3)
    {
        sprintf(strAudioInput, "SOUNDCARD_LINE_IN");
    }

    if (bVideoIsInterleaved == TRUE)
    {
        nVH = nVideoHeight / 2;
    }
    else
    {
        nVH = nVideoHeight;
    }

    if (bVideoIsInterleaved == TRUE)
    {
        sprintf(strFrameType, " I ");
    }
    else
    {
        sprintf(strFrameType, " P ");
    }

    printf("INFO : %ld x %d%s @%2.3f FPS , %ld CH x %ld BITS x %ld HZ , VIDEO INPUT : %s , AUDIO INPUT : %s\n",
           nVideoWidth, nVH, strFrameType, dVideoFrameRate, nAudioChannels, nAudioBitsPerSample,
           nAudioSampleFrequency, strVideoInput, strAudioInput);

    g_capture->m_nVideoWidth = nVideoWidth;
    g_capture->m_nVideoHeight = nVideoHeight;
    g_capture->m_dVideoFrameRate = dVideoFrameRate;
    g_capture->m_nAudioChannels = nAudioChannels;
    g_capture->m_nAudioBitsPerSample = nAudioBitsPerSample;
    g_capture->m_nAudioSampleFrequency = nAudioSampleFrequency;
    g_capture->m_bVideoIsInterleaved = bVideoIsInterleaved;

    return QCAP_RT_OK;
}

QRETURN on_process_no_signal_detected(PVOID pDevice, ULONG nVideoInput, ULONG nAudioInput, PVOID pUserData)
{
    ULONG i = (uintptr_t)pUserData;

    printf("INFO : No signal detected\n");

    return QCAP_RT_OK;
}

QRETURN on_process_signal_removed(PVOID pDevice, ULONG nVideoInput, ULONG nAudioInput, PVOID pUserData)
{
    ULONG i = (uintptr_t)pUserData;

    printf("INFO : No signal detected\n");

    return QCAP_RT_OK;
}

void save_image_raw(const char *filename, void *image_ptr, size_t image_size)
{
    std::ofstream storage_file(filename, std::ios::out | std::ios::binary);
    storage_file.write((char *)image_ptr, image_size);
    storage_file.close();
}

int main(int argc, char *argv[])
{
    if ((argc > 2) && (strcmp(argv[1], "--qcap") == 0))
    {
        int rc;

        /* Init Servers */
        rc = alg_sdk_init_server();
        if (rc < 0)
        {
            fatal("Init server failed\n");
        }
        /* end */

        const uint32_t image_width = atoi(argv[2]);
        const uint32_t image_height = atoi(argv[3]);
        const uint32_t channel_id = atoi(argv[4]);

        const int rate = 30;
        const int ops = QCapDev::Default;
        const uint32_t image_size = image_width * image_height * 2;

        QCapDev cap(image_width, image_height, channel_id, rate, ops);
        g_capture = &cap;
        cap.setQcapCallbackNoSignal(on_process_no_signal_detected);
        cap.setQcapCallbackSignalRemoved(on_process_signal_removed);
        cap.setQcapCallbackFormatChanged(on_process_format_changed);
        cap.setQcapCallbackVideoPreview(on_video_preview_callback);
        printf("Init QCap Device [%d]\n", channel_id);

        /* Generate pcie image data head */
        pcie_common_head_t *img_header = &g_img_header;
        img_header->head = 55;
        img_header->version = 1;
        int ch = channel_id;
        char topic_name[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {};
        sprintf(topic_name, "/image_data/stream/%02d", ch);
        strcpy(img_header->topic_name, topic_name);
        // printf("%s\n", img_header->topic_name);
        img_header->crc8 = crc_array((unsigned char *)img_header, 130);
        /* end */

        /* Generate pcie image data info */
        pcie_image_info_meta_t *img_info = &g_img_info;
        img_info->frame_index = 0;
        /* IMPORTANT NOTE :
         *  Image size must MATCH the input image!
         */
        img_info->width = image_width;
        img_info->height = image_height;
        /* */
        img_info->data_type = ALG_SDK_MIPI_DATA_TYPE_YUYV;
        img_info->exposure = 1.5;
        img_info->again = 1.0;
        img_info->dgain = 1.0;
        img_info->temp = 25.0;
        img_info->timestamp = milliseconds();
        img_info->img_size = image_size;
        /* end */

        /* Generate pcie image data */
        pcie_image_data_t *img_data = &g_img_data;
        img_data->common_head = *img_header;
        img_data->image_info_meta = *img_info;
        img_data->payload = (uint8_t *)malloc(sizeof(uint8_t) * image_size);

        int data_type;
        uint32_t data_len;

        while (!g_signal_recieved)
        {
            // img_info.frame_index = v4l2_device->sequence;
            // img_info.timestamp = v4l2_device->timestamp;

            // pcie_image_data_t img_data;
            // img_data.common_head = img_header;
            // img_data.image_info_meta = img_info;
            // img_data.payload = (uint8_t *)v4l2_device->out_data;
            // alg_sdk_push2q(&img_data, channel_id);

            usleep(100);
        }

        free(img_data->payload);
        alg_sdk_server_spin_on();
    }
    else
    {
        fprintf(stderr, "Usage: ./hil_sdk_demo_push_qcap <TYP> <WIDTH> <HEIGHT> <CHANNEL>...\n");
        fprintf(stderr, "e.g. ./hil_sdk_demo_push_qcap --qcap 1920 1080 0\n");
    }

    return 0;
}