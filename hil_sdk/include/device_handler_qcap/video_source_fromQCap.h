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
#ifndef __VIDEO_SOURCE_FROM_QCAP_H__
#define __VIDEO_SOURCE_FROM_QCAP_H__
#include "device_handler/video_source.h"
#include "utils/image_feed.h"
#include "qcap/qcapdev.h"

class VideoSourceFromQCap : public VideoSource
{
public:
    VideoSourceFromQCap(VideoSourceParam *param);
    virtual ~VideoSourceFromQCap();

public:
    virtual int  InitVideoSource();
    virtual int  OpenVideoSource();
    virtual void StartSource();
    virtual void EndSource();
    virtual void Loop();
    virtual void WaitForFinish();
    virtual void FreeVideoSource();

private:
    static QRETURN on_process_signal_removed(void* pDevice, ULONG nVideoInput, ULONG nAudioInput, void* pUserData);
    static QRETURN on_process_no_signal_detected(void* pDevice, ULONG nVideoInput, ULONG nAudioInput, void* pUserData);    
    static QRETURN on_process_format_changed(PVOID pDevice, ULONG nVideoInput, ULONG nAudioInput, ULONG nVideoWidth, ULONG nVideoHeight, BOOL bVideoIsInterleaved, double dVideoFrameRate, ULONG nAudioChannels, ULONG nAudioBitsPerSample, ULONG nAudioSampleFrequency, PVOID pUserData);
    static QRETURN on_video_preview_callback(void* pDevice, double dSampleTime, BYTE *pFrameBuffer, ULONG nFrameBufferLen, void* pUserData);

private:
    std::string  m_filename;
    uint32_t     m_payload_len;
    uint32_t     m_image_size;
    uint32_t     m_seq;
    uint64_t     t_now;
    uint64_t     t_last;

    ImageFeed    *p_image_feed;
    QCapDev      *p_cap;
};


#endif