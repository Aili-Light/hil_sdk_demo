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
#ifndef __VIDEO_SOURCE_FROM_ALGPUB_H__
#define __VIDEO_SOURCE_FROM_ALGPUB_H__
#include "device_handler/video_source.h"
#include "utils/image_feed.h"

class VideoSourceFromALGPub : public VideoSource
{
public:
    VideoSourceFromALGPub(VideoSourceParam *param);
    virtual ~VideoSourceFromALGPub();

public:
    virtual int  InitVideoSource();
    virtual int  OpenVideoSource();
    virtual void StartSource();
    virtual void EndSource();
    virtual void Loop();
    virtual void WaitForFinish();
    virtual void FreeVideoSource();
    virtual void SendData();
    void         Publish(const unsigned char *payload, const unsigned int frame_index, const unsigned long timestamp);

private:
    uint8_t*     p_payload;
    int          m_topic_id;
    std::string  m_topic_name;
    uint32_t     m_image_size;
    
    ImageFeed    m_image_feed;
};


#endif