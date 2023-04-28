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
#ifndef __V4L2_CAMERA_H__
#define __V4L2_CAMERA_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>

#define FMT_NUM_PLANES 1

struct buffer
{
    void *start;
    size_t length;
};

struct v4l2_dev
{
    int fd;
    char *path;
    const char *name;
    const char *subdev_path;
    char out_type[10];
    enum v4l2_buf_type buf_type;
    int format;
    int width;
    int height;
    unsigned int req_count;
    enum v4l2_memory memory_type;
    struct buffer *buffers;
    unsigned int sequence;
    unsigned long int timestamp;
    int data_len;
    unsigned char *out_data;
    unsigned int buf_index;
};

typedef struct v4l2_capture_args
{
    void *arg;
    void *dev;
} v4l2_capture_args_t;

void close_device(struct v4l2_dev *dev);

void exit_failure(struct v4l2_dev *dev);
void open_device(struct v4l2_dev *dev);
void get_capabilities(struct v4l2_dev *dev);
void set_fmt(struct v4l2_dev *dev);
void get_fmt(struct v4l2_dev *dev);
void require_buf(struct v4l2_dev *dev);
void alloc_buf(struct v4l2_dev *dev);
void queue_buf(struct v4l2_dev *dev);
void stream_on(struct v4l2_dev *dev);
void capture_frame(struct v4l2_dev *dev);
void save_picture(const char *filename, unsigned char *file_data, unsigned int len, int is_overwrite);
void stream_off(struct v4l2_dev *dev);
void set_fps(struct v4l2_dev *dev, unsigned int fps);

#endif