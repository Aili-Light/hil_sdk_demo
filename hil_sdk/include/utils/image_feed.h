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
#ifndef __IMAGE_FEED_H__
#define __IMAGE_FEED_H__
#include <stdint.h>
#include <string.h>
#include "alg_common/basic_types.h"

class ImageFeed
{
public:
    ImageFeed              ();
    ~ImageFeed             ();

    /* Initialize Feed */
    int                    InitFeed(const int w,const int h,const int ch,const size_t size, const int type);
 
    /* Feed Data */ 
    int                    FeedData(const unsigned char* payload, const unsigned int frame_index, const unsigned long timestamp);
    int                    SetSourceType(const char *c_data_type);
    int                    SetOutputType(const char *c_data_type);
    int                    SetOutputBayerType(const char *c_pattern_type);

    /* Get Data */
    void*                  GetDataPtr();
    size_t                 GetImageSize();
    int                    GetChannelID();
    uint32_t               GetLastFrameIndex();
    uint64_t               GetLastTimeStamp();
private:
    /* Make structure */
    int                    MakePCIEDataStruct();
    int                    MakePCIEDataHeader();
    int                    MakePCIEDataInfo();
    int                    SetDataType(const char *c_data_type);
    int                    SetBayerType(const char *c_pattern_type);
    int                    EndFeed();

private:
    int                    feed_data_yuv422(const unsigned char *payload, const unsigned int frame_index, const unsigned long timestamp);
    int                    feed_data_raw10(const unsigned char *payload, const unsigned int frame_index, const unsigned long timestamp);
    int                    feed_data_raw12(const unsigned char *payload, const unsigned int frame_index, const unsigned long timestamp);
    int                    feed_data_raw10_pad(const unsigned char *payload, const unsigned int frame_index, const unsigned long timestamp);
    int                    feed_data_raw12_pad(const unsigned char *payload, const unsigned int frame_index, const unsigned long timestamp);
    int                    feed_data_rgb888(const unsigned char *payload, const unsigned int frame_index, const unsigned long timestamp);

private:
    int                    image_width;
    int                    image_height;
    int                    channel_id;
    size_t                 image_size;
    int                    source_data_type;
    int                    output_data_type;
    int                    bayer_pattern_type;
    uint16_t*              pdata;
    uint8_t*               pdata_8;
    uint8_t*               pdata_bayer;

    pcie_common_head_t     img_header;
    pcie_image_info_meta_t img_info;
    pcie_image_data_t      img_data;


};

#endif