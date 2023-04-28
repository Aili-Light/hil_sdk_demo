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
#include "v4l2_camera.h"

void close_device(struct v4l2_dev *dev)
{
    if (dev->buffers)
    {
        for (unsigned int i = 0; i < dev->req_count; ++i)
        {
            if (dev->buffers[i].start)
            {
                munmap(dev->buffers[i].start, dev->buffers[i].length);
            }
        }
        free(dev->buffers);
    }
    if (-1 != dev->fd)
    {
        close(dev->fd);
    }
    // if (-1 != dev->sub_fd) {
    //     close(dev->sub_fd);
    // }
    return;
}

void exit_failure(struct v4l2_dev *dev)
{
    close_device(dev);
    exit(EXIT_FAILURE);
}

void open_device(struct v4l2_dev *dev)
{
    dev->fd = open(dev->path, O_RDWR | O_CLOEXEC, 0);
    // dev->fd = open(dev->path, O_RDWR, 0);
    if (dev->fd < 0)
    {
        printf("Cannot open %s\n\n", dev->path);
        exit_failure(dev);
    }
    printf("Open %s succeed - %d\n\n", dev->path, dev->fd);

    // dev->sub_fd = open(dev->subdev_path, O_RDWR|O_CLOEXEC, 0);
    // if (dev->sub_fd < 0) {
    //     printf("Cannot open %s\n\n", dev->subdev_path);
    //     exit_failure(dev);
    // }
    printf("Open %s succeed\n\n", dev->subdev_path);
    return;
}

void get_capabilities(struct v4l2_dev *dev)
{
    struct v4l2_capability cap;
    if (ioctl(dev->fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        printf("VIDIOC_QUERYCAP failed\n");
        return;
    }
    printf("------- VIDIOC_QUERYCAP ----\n");
    printf("  driver: %s\n", cap.driver);
    printf("  card: %s\n", cap.card);
    printf("  bus_info: %s\n", cap.bus_info);
    printf("  version: %d.%d.%d\n",
           (cap.version >> 16) & 0xff,
           (cap.version >> 8) & 0xff,
           (cap.version & 0xff));
    printf("  capabilities: %08X\n", cap.capabilities);

    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
        printf("        Video Capture\n");
    if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)
        printf("        Video Output\n");
    if (cap.capabilities & V4L2_CAP_VIDEO_OVERLAY)
        printf("        Video Overly\n");
    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        printf("        Video Capture Mplane\n");
    if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT_MPLANE)
        printf("        Video Output Mplane\n");
    if (cap.capabilities & V4L2_CAP_READWRITE)
        printf("        Read / Write\n");
    if (cap.capabilities & V4L2_CAP_STREAMING)
        printf("        Streaming\n");
    printf("\n");
    return;
}

void set_fmt(struct v4l2_dev *dev)
{
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = dev->buf_type;
    fmt.fmt.pix.pixelformat = dev->format;
    fmt.fmt.pix.width = dev->width;
    fmt.fmt.pix.height = dev->height;
    if (ioctl(dev->fd, VIDIOC_S_FMT, &fmt) < 0)
    {
        printf("VIDIOC_S_FMT failed - [%d]!\n", errno);
        exit_failure(dev);
    }
    printf("VIDIOC_S_FMT succeed!\n");
    dev->data_len = fmt.fmt.pix.sizeimage;
    printf("width %d, height %d, size %d, bytesperline %d, format %c%c%c%c\n\n",
           fmt.fmt.pix.width, fmt.fmt.pix.height, dev->data_len,
           fmt.fmt.pix.bytesperline,
           fmt.fmt.pix.pixelformat & 0xFF,
           (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
           (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
           (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
    return;
}

void get_fmt(struct v4l2_dev *dev)
{
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = dev->buf_type;

    if (ioctl(dev->fd, VIDIOC_G_FMT, &fmt) < 0)
    {
        printf("VIDIOC_G_FMT failed - [%d]!\n", errno);
        exit_failure(dev);
    }
    dev->buf_type = (v4l2_buf_type)fmt.type;
    dev->format = fmt.fmt.pix.pixelformat;
    dev->width = fmt.fmt.pix.width;
    dev->height = fmt.fmt.pix.height;
    dev->data_len = fmt.fmt.pix.sizeimage;

    snprintf(dev->out_type, 5, "%c%c%c%c",
             fmt.fmt.pix.pixelformat & 0xFF,
             (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
             (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
             (fmt.fmt.pix.pixelformat >> 24) & 0xFF);

    printf("VIDIOC_G_FMT succeed!\n");
    printf("width %d, height %d, size %d, bytesperline %d, format %c%c%c%c\n\n",
           fmt.fmt.pix.width, fmt.fmt.pix.height, dev->data_len,
           fmt.fmt.pix.bytesperline,
           fmt.fmt.pix.pixelformat & 0xFF,
           (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
           (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
           (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
    return;
}

void require_buf(struct v4l2_dev *dev)
{
    // 申请缓冲区
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = dev->req_count;
    req.type = dev->buf_type;
    req.memory = dev->memory_type;
    if (ioctl(dev->fd, VIDIOC_REQBUFS, &req) == -1)
    {
        printf("VIDIOC_REQBUFS failed!\n\n");
        exit_failure(dev);
    }
    if (dev->req_count != req.count)
    {
        printf("!!! req count = %d\n", req.count);
        dev->req_count = req.count;
    }
    printf("VIDIOC_REQBUFS succeed!\n\n");
    return;
}

void alloc_buf(struct v4l2_dev *dev)
{
    dev->buffers = (struct buffer *)calloc(dev->req_count, sizeof(*(dev->buffers)));
    for (unsigned int i = 0; i < dev->req_count; ++i)
    {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[FMT_NUM_PLANES];
        memset(&buf, 0, sizeof(buf));
        memset(&planes, 0, sizeof(planes));
        buf.type = dev->buf_type;
        buf.memory = dev->memory_type;
        buf.index = i;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == dev->buf_type)
        {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (ioctl(dev->fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            printf("VIDIOC_QUERYBUF failed!\n\n");
            exit_failure(dev);
        }
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == dev->buf_type)
        {
            dev->buffers[i].length = buf.m.planes[0].length;
            dev->buffers[i].start =
                mmap(NULL /* start anywhere */,
                     buf.m.planes[0].length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     dev->fd, buf.m.planes[0].m.mem_offset);
        }
        else
        {
            dev->buffers[i].length = buf.length;
            dev->buffers[i].start = mmap(NULL,
                                         buf.length,
                                         PROT_READ | PROT_WRITE,
                                         MAP_SHARED,
                                         dev->fd,
                                         buf.m.offset);
        }

        if (dev->buffers[i].start == MAP_FAILED)
        {
            printf("Memory map failed!\n\n");
            exit_failure(dev);
        }
    }
    printf("Memory map succeed!\n\n");
    return;
}

void queue_buf(struct v4l2_dev *dev)
{
    for (unsigned int i = 0; i < dev->req_count; ++i)
    {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[FMT_NUM_PLANES];
        memset(&buf, 0, sizeof(buf));
        memset(&planes, 0, sizeof(planes));
        buf.type = dev->buf_type;
        buf.memory = dev->memory_type;
        buf.index = i;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == dev->buf_type)
        {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (ioctl(dev->fd, VIDIOC_QBUF, &buf) < 0)
        {
            printf("VIDIOC_QBUF failed!\n\n");
            exit_failure(dev);
        }
    }
    printf("VIDIOC_QBUF succeed!\n\n");
    return;
}

void stream_on(struct v4l2_dev *dev)
{
    enum v4l2_buf_type type = dev->buf_type;
    if (ioctl(dev->fd, VIDIOC_STREAMON, &type) == -1)
    {
        printf("VIDIOC_STREAMON failed!\n\n");
        exit_failure(dev);
    }
    printf("VIDIOC_STREAMON succeed!\n\n");
    return;
}

void capture_frame(struct v4l2_dev *dev)
{
    struct v4l2_buffer buf;
    struct v4l2_plane planes[FMT_NUM_PLANES];

    memset(&buf, 0, sizeof(buf));
    buf.type = dev->buf_type;
    buf.memory = dev->memory_type;

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == dev->buf_type)
    {
        buf.m.planes = planes;
        buf.length = FMT_NUM_PLANES;
    }

    if (ioctl(dev->fd, VIDIOC_DQBUF, &buf) == -1)
    {
        printf("VIDIOC_DQBUF failed!\n\n");
        exit_failure(dev);
    }

    dev->out_data = (unsigned char *)dev->buffers[buf.index].start;
    dev->timestamp = buf.timestamp.tv_sec * 1000000 + buf.timestamp.tv_usec;
    dev->sequence = buf.sequence;
    dev->buf_index = buf.index;
    // printf("image: sequence(frame index) = %d, timestamp = %lu\n", dev->sequence, dev->timestamp);
    // printf("frame=%d,stamp=%ld,size=%ld\n", buf.sequence, dev->timestamp, dev->buffers[buf.index].length);

    if (ioctl(dev->fd, VIDIOC_QBUF, &buf) == -1)
    {
        printf("VIDIOC_QBUF failed!\n");
        exit_failure(dev);
    }

    return;
}

void save_picture(const char *filename, unsigned char *file_data, unsigned int len, int is_overwrite)
{
    FILE *fp;
    if (is_overwrite)
        fp = fopen(filename, "wb");
    else
        fp = fopen(filename, "ab");
    if (fp < 0)
    {
        printf("Open frame data file failed\n\n");
        return;
    }
    if (fwrite(file_data, 1, len, fp) < len)
    {
        printf("Out of memory!\n");
    }
    fflush(fp);
    fclose(fp);
    printf("Save one frame to %s succeed!\n\n", filename);
    return;
}

void stream_off(struct v4l2_dev *dev)
{
    enum v4l2_buf_type type;
    type = dev->buf_type;
    if (ioctl(dev->fd, VIDIOC_STREAMOFF, &type) == -1)
    {
        printf("VIDIOC_STREAMOFF failed!\n\n");
        exit_failure(dev);
    }
    printf("VIDIOC_STREAMOFF succeed!\n\n");
    return;
}

void set_fps(struct v4l2_dev *dev, unsigned int fps)
{
    int ret;
    struct v4l2_subdev_frame_interval frame_int;

    if (fps == 0)
        return;

    memset(&frame_int, 0x00, sizeof(frame_int));

    frame_int.interval.numerator = 10000;
    frame_int.interval.denominator = fps * 10000;

    printf("VIDIOC_SUBDEV_S_FRAME_INTERVAL [%u fps] OK\n", fps);
set_fps_err:
    return;
}