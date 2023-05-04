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
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#include <alg_camera/v4l2_camera.h>
#include "alg_sdk/alg_sdk.h"
#include "alg_sdk/server.h"
#include "alg_sdk/pull.h"
#include "alg_common/basic_types.h"
#include "utils.h"

#define FMT_NUM_PLANES 1

bool g_signal_recieved = false;

struct v4l2_dev v4l2loop_device = {
    .fd = -1,
    .path = (char *)"/dev/video100",
    .name = (char *)"v4l2loop_dev",
    .subdev_path = NULL,
    .out_type = {0},
    .buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
    .format = -1,
    .width = -1,
    .height = -1,
    .req_count = 2,
    .memory_type = V4L2_MEMORY_MMAP,
    .buffers = NULL,
    .sequence = 0,
    .timestamp = 0,
    .data_len = 0,
    .out_data = NULL,
    .buf_index = 0,
};

int fatal(const char *msg)
{
    fprintf(stderr, "fatal error : %s", msg);
    exit(1);
}

void int_handler(int sig)
{
    g_signal_recieved = true;
    stream_off(&v4l2loop_device);   // 8 关闭视频流
    close_device(&v4l2loop_device); // 9 释放内存关闭文件
    alg_sdk_stop_server();
    alg_sdk_stop_notify();

    /* terminate program */
    exit(sig);
}

void push_callback(void *p)
{
    char *msg = (char *)p;
    printf("Notify Message : %s\n", msg);
}

void save_image_raw(const char *filename, void *image_ptr, size_t image_size)
{
    std::ofstream storage_file(filename, std::ios::out | std::ios::binary);
    storage_file.write((char *)image_ptr, image_size);
    storage_file.close();
}

int main(int argc, char *argv[])
{
    struct v4l2_dev *v4l2_device = &v4l2loop_device;
    const char *v4l2_dev_name = 0;
    char name[40] = {0};

    if ((argc > 2) && (strcmp(argv[1], "--v4l2") == 0))
    {
        int rc;

        /* Setup notify callback */
        rc = alg_sdk_notify(push_callback);
        if (rc < 0)
        {
            fatal("Setup notify failed\n");
        }
        /* end */

        /* Init Servers */
        rc = alg_sdk_init_server();
        if (rc < 0)
        {
            fatal("Init server failed\n");
        }
        /* end */

        v4l2_device->path = argv[2];

        const uint32_t image_width = atoi(argv[3]);
        const uint32_t image_height = atoi(argv[4]);
        const uint8_t channel_id = atoi(argv[5]);
        const uint32_t image_size = image_width * image_height * 2;

        open_device(v4l2_device); // 1 打开摄像头设备
        get_fmt(v4l2_device);     // 2 查询出图格式
        get_capabilities(v4l2_device);
        require_buf(v4l2_device); // 3 申请缓冲区
        alloc_buf(v4l2_device);   // 4 内存映射
        queue_buf(v4l2_device);   // 5 将缓存帧加入队列
        stream_on(v4l2_device);   // 6 开启视频流

        printf("Init V4L2 Camera [%s]\n", v4l2_device->path);

        /* Generate pcie image data head */
        pcie_common_head_t img_header;
        img_header.head = 55;
        img_header.version = 1;
        int ch = channel_id;
        char topic_name[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {};
        sprintf(topic_name, "/image_data/stream/%02d", ch);
        strcpy(img_header.topic_name, topic_name);
        // printf("%s\n", img_header[i].topic_name);
        img_header.crc8 = crc_array((unsigned char *)&img_header, 130);
        /* end */

        /* Generate pcie image data info */
        pcie_image_info_meta_t img_info;
        img_info.frame_index = 0;
        /* IMPORTANT NOTE :
         *  Image size must MATCH the input image!
         */
        img_info.width = image_width;
        img_info.height = image_height;
        /* */
        img_info.data_type = ALG_SDK_MIPI_DATA_TYPE_YUYV;
        img_info.exposure = 1.5;
        img_info.again = 1.0;
        img_info.dgain = 1.0;
        img_info.temp = 25.0;
        img_info.timestamp = milliseconds();
        img_info.img_size = image_size;
        /* end */

        int data_type;
        uint32_t data_len;

        while (!g_signal_recieved)
        {
            capture_frame(v4l2_device); // 7 取一帧数据

            switch (v4l2_device->format)
            {
            case V4L2_PIX_FMT_YVYU:
                data_type = ALG_SDK_MIPI_DATA_TYPE_YVYU;
                break;
            case V4L2_PIX_FMT_YUYV:
                data_type = ALG_SDK_MIPI_DATA_TYPE_YUYV;
                break;
            case V4L2_PIX_FMT_UYVY:
                data_type = ALG_SDK_MIPI_DATA_TYPE_UYVY;
                break;
            case V4L2_PIX_FMT_VYUY:
                data_type = ALG_SDK_MIPI_DATA_TYPE_VYUY;
                break;
            default:
                data_type = ALG_SDK_MIPI_DATA_TYPE_DEFAULT;
                fatal("ERROR! Image Type Not Match!!!\n");
                break;
            }

            data_len = v4l2_device->buffers[v4l2_device->buf_index].length;
            if (image_size != data_len)
            {
                fatal("ERROR! Image Size Do Not Match!!!\n");
            }
            printf("[SEQ:%d] [Type:%d]  [Width:%d] [Height:%d] [Len:%d]\n", v4l2_device->sequence, data_type, v4l2_device->width, v4l2_device->height, data_len);

            img_info.frame_index = v4l2_device->sequence;
            img_info.timestamp = v4l2_device->timestamp;

            pcie_image_data_t img_data;
            img_data.common_head = img_header;
            img_data.image_info_meta = img_info;
            img_data.payload = (uint8_t *)v4l2_device->out_data;
            alg_sdk_push2q(&img_data, channel_id);

            // char filename_raw[128] = {};
            // sprintf(filename_raw, "data/image_%lu.raw", v4l2_device->timestamp);
            // save_image_raw(filename_raw, v4l2_device->out_data, v4l2_device->buffers[v4l2_device->buf_index].length);
        }
        alg_sdk_server_spin_on();
        alg_sdk_notify_spin_on();
    }
    else
    {
        fprintf(stderr, "Usage: ./hil_sdk_demo_push_v4l2 <TYP> <DEVICE_ID> <ARG1> <ARG2> <ARG3>...\n");
        fprintf(stderr, "e.g. ./hil_sdk_demo_push_v4l2 --v4l2 /dev/video0 1920 1280 0\n");
    }

    return 0;
}