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
#ifndef __VIDEO_SOURCE_H__
#define __VIDEO_SOURCE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include "alg_common/basic_types.h"

struct VideoSourceParam
{
    int         source_id;
    char        config_file[1024];
};

class VideoSource
{
protected:
    VideoSource  ();
public:
    virtual ~VideoSource ();
public:
    void                    SetSourceName(const char* _source_name) {m_source_name=_source_name;};
    void                    SetFrameRate(const float _fps) {m_fps=_fps;};
    void                    SetChannelID(const int _ch_id) {m_ch_id=_ch_id;};
    void                    SetOutputType(const char* _output_type) {m_output_type=_output_type;};
protected:           
    virtual int             InitVideoSource();
    virtual int             OpenVideoSource();
    virtual void            StartSource();
    virtual void            EndSource();
    virtual void            Loop();
    virtual void            WaitForFinish();
    virtual void            FreeVideoSource();
    virtual bool            StreamOn(bool b_on);
public:           
    int                     GetChannelId()const{ return m_ch_id; }
    int                     GetWidth()const{ return m_image_width; }
    int                     GetHeight()const{ return m_image_height; }
    float                   GetFrameRate()const{ return m_fps; }

protected:
    int                     m_image_width;
    int                     m_image_height;
    float                   m_fps;
    int                     m_ch_id;
    bool                    m_thread_started;

    std::string             m_source_name;
    std::string             m_output_type;
    std::string             m_source_type;
    std::string             m_bayer_type;

    std::thread             m_thr;
    std::mutex              m_mutex;
    sem_t                   full;
    sem_t                   empty;
};


#endif