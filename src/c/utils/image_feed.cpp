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
#include "stdio.h"
#include "stdlib.h"
#include "image_feed.h"
#include "utils.h"

ImageFeed::ImageFeed() {}

ImageFeed::~ImageFeed()
{
    de_init();
}

int ImageFeed::init_feed(const int w, const int h, const int ch, const size_t size, const int type)
{
    image_width = w;
    image_height = h;
    channel_id = ch;
    image_size = size;
    data_type = type;

    const uint32_t data_size = image_width * image_height;
    pdata = (uint16_t *)malloc(sizeof(uint16_t) * data_size);

    return 0;
}

int ImageFeed::de_init()
{
    if (pdata != NULL)
        free(pdata);
}

int ImageFeed::make_header()
{
    img_header.head = 55;
    img_header.version = 1;
    int ch = channel_id;
    char topic_name[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {};
    sprintf(topic_name, "/image_data/stream/%02d", ch);
    strncpy(img_header.topic_name, topic_name, ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN);
    // printf("%s\n", img_header[i].topic_name);
    img_header.crc8 = crc_array((unsigned char *)&img_header, 130);

    return 0;
}

int ImageFeed::make_info()
{
    img_info.frame_index = 0;
    /* IMPORTANT NOTE :
     *  Image size must MATCH the input image!
     */
    img_info.width = image_width;
    img_info.height = image_height;

    /* data type is user-defined */
    img_info.exposure = 1.5;
    img_info.again = 1.0;
    img_info.dgain = 1.0;
    img_info.temp = 25.0;
    img_info.timestamp = milliseconds();
    img_info.img_size = image_size;

    return 0;
}

int ImageFeed::make_data_struct()
{
    make_header();
    make_info();

    img_data.common_head = img_header;
    img_data.image_info_meta = img_info;

    return 0;
}

int ImageFeed::set_frame_index(uint32_t index)
{
    img_data.image_info_meta.frame_index = index;

    return 0;
}

int ImageFeed::set_timestampe(uint64_t timestamp)
{
    img_data.image_info_meta.timestamp = timestamp;

    return 0;
}

int ImageFeed::feed_data_yuv422(const unsigned char *payload, const unsigned int frame_index, const unsigned long timestamp)
{
    img_data.image_info_meta.frame_index = (uint32_t)frame_index;
    img_data.image_info_meta.timestamp = (uint64_t)timestamp;
    img_data.payload = (void *)payload;

    return 0;
}

int ImageFeed::feed_data_raw10(const unsigned char *payload, const unsigned int frame_index, const unsigned long timestamp)
{
    img_data.image_info_meta.frame_index = (uint32_t)frame_index;
    img_data.image_info_meta.timestamp = (uint64_t)timestamp;

    uint32_t data_size = image_width * image_height;
    for (int i = 0; i < int(data_size / 4); i++)
    {
        pdata[4 * i] = ((((payload[5 * i]) << 2) & 0x03FC) | ((payload[5 * i + 4] >> 0) & 0x0003));
        pdata[4 * i + 1] = ((((payload[5 * i + 1]) << 2) & 0x03FC) | ((payload[5 * i + 4] >> 2) & 0x0003));
        pdata[4 * i + 2] = ((((payload[5 * i + 2]) << 2) & 0x03FC) | ((payload[5 * i + 4] >> 4) & 0x0003));
        pdata[4 * i + 3] = ((((payload[5 * i + 3]) << 2) & 0x03FC) | ((payload[5 * i + 4] >> 6) & 0x0003));
    }

    img_data.payload = (void *)pdata;

    return 0;
}

int ImageFeed::feed_data_raw12(const unsigned char *payload, const unsigned int frame_index, const unsigned long timestamp)
{

    img_data.image_info_meta.frame_index = (uint32_t)frame_index;
    img_data.image_info_meta.timestamp = (uint64_t)timestamp;

    uint32_t data_size = image_width * image_height;
    for (int i = 0; i < int(data_size / 2); i++)
    {
        pdata[2 * i] = ((((payload[3 * i]) << 4) & 0x0FF0) | ((payload[3 * i + 2] >> 0) & 0x000F));
        pdata[2 * i + 1] = ((((payload[3 * i + 1]) << 4) & 0x0FF0) | ((payload[3 * i + 2] >> 4) & 0x000F));
    }
    img_data.payload = (void *)pdata;

    return 0;
}

int ImageFeed::feed_data(const unsigned char *payload, const unsigned int frame_index, const unsigned long timestamp)
{
    img_data.image_info_meta.data_type = data_type;
    switch (data_type)
    {
    case ALG_SDK_MIPI_DATA_TYPE_YUYV:
    case ALG_SDK_MIPI_DATA_TYPE_YVYU:
    case ALG_SDK_MIPI_DATA_TYPE_UYVY:
    case ALG_SDK_MIPI_DATA_TYPE_VYUY:
        feed_data_yuv422(payload, frame_index, timestamp);
        break;
    case ALG_SDK_MIPI_DATA_TYPE_RAW10:
        feed_data_raw10(payload, frame_index, timestamp);
        break;
    case ALG_SDK_MIPI_DATA_TYPE_RAW12:
        feed_data_raw12(payload, frame_index, timestamp);
        break;
    default:
        break;
    }

    // printf("********* payload [%d] [%d], ptr [%p]\n", ((uint8_t *)img_data.payload)[0], ((uint8_t *)img_data.payload)[100245], img_data.payload);
    return 0;
}

int ImageFeed::set_data_type(const char *c_data_type)
{
    int dt = ALG_SDK_MIPI_DATA_TYPE_DEFAULT;
    /* Data Type is UYVY/VYUY/YUYV/YVYU */
    if (strncmp(c_data_type, "YUYV", 4) == 0 || strncmp(c_data_type, "Default", 7) == 0)
    {
        dt = ALG_SDK_MIPI_DATA_TYPE_YUYV;
    }
    else if (strncmp(c_data_type, "YVYU", 4) == 0)
    {
        dt = ALG_SDK_MIPI_DATA_TYPE_YVYU;
    }
    else if (strncmp(c_data_type, "UYVY", 4) == 0)
    {
        dt = ALG_SDK_MIPI_DATA_TYPE_UYVY;
    }
    else if (strncmp(c_data_type, "VYUY", 4) == 0)
    {
        dt = ALG_SDK_MIPI_DATA_TYPE_VYUY;
    }
    else if (strncmp(c_data_type, "RAW10", 5) == 0)
    {
        dt = ALG_SDK_MIPI_DATA_TYPE_RAW10;
    }
    else if (strncmp(c_data_type, "RAW12", 5) == 0)
    {
        dt = ALG_SDK_MIPI_DATA_TYPE_RAW12;
    }
    else
    {
        printf("Set Data Type -- unsupported image format (%s)\n", c_data_type);
        printf("      supported formats are:\n");
        printf("          * YUYV/YVYU/UYVY/VYUY\n");
        printf("          * RAW10\n");
        printf("          * RAW12\n");

        return 0;
    }

    data_type = dt;
    // printf("Data Type : [%s] [%d]\n", c_data_type, data_type);
    return data_type;
}

void *ImageFeed::get_data_ptr()
{
    return (&img_data);
}

size_t ImageFeed::get_image_size()
{
    return (img_data.image_info_meta.img_size);
}
