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
#ifndef __VIDEO_SOURCE_FROM_TCP_H__
#define __VIDEO_SOURCE_FROM_TCP_H__
#include <vector>
#include <memory>
#include "device_handler/video_source.h"
#include "utils/image_feed.h"
#include "utils/tcpclient.h"
#include "utils/ringbuffer.h"

class VideoSourceFromTCP : public VideoSource
{
public:
    VideoSourceFromTCP(VideoSourceParam *param);
    virtual ~VideoSourceFromTCP() = default;

public:
    virtual int  InitVideoSource() override;
    virtual int  OpenVideoSource() override;
    virtual void StartSource() override;
    virtual void EndSource() override;
    virtual void Loop() override;
    virtual void WaitForFinish() override;
    virtual void FreeVideoSource() override;
    virtual void SendData() override;

private:
    bool LoadImageFromTCP(char* buf, int len, std::uint16_t ch);
    bool GetHostAndPort(std::string& host, int& port);

private:
    std::string  m_host;
    int          m_port;
    uint32_t     m_image_size;

    class Payload;
    std::shared_ptr<TcpClient> tcp_client;

    std::shared_ptr<RingBuffer<std::shared_ptr<Payload>>> rbuffer;

    ImageFeed    m_image_feed;
    
};


#endif