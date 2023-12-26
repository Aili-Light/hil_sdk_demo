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
#ifndef __VIDEO_SOURCE_FROM_VIDEO_H__
#define __VIDEO_SOURCE_FROM_VIDEO_H__
#include "device_handler/video_source.h"
#include "utils/image_feed.h"
#include "alg_video_decode/decoder_ff.h"

class VideoSourceFromVideo : public VideoSource
{
public:
    VideoSourceFromVideo(VideoSourceParam *param);
    virtual ~VideoSourceFromVideo();

public:
    virtual int  InitVideoSource();
    virtual int  OpenVideoSource();
    virtual void StartSource();
    virtual void EndSource();
    virtual void Loop();
    virtual void WaitForFinish();
    virtual void FreeVideoSource();
    virtual void DecodeNext();

private:
    static void  onDecodeCallback(void *data, int ch_id);   

private:
    uint8_t*     p_payload;
    std::string  m_filename;
    std::string  m_decoder_name;
    uint32_t     m_payload_len;
    uint32_t     m_image_size;

    ImageFeed    m_image_feed;
    DecoderParam m_decode_param;
    Decoder_FF   *decoder;
};


#endif