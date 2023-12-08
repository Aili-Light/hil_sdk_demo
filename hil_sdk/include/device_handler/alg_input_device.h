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
#ifndef __ALG_INPUT_DEVICE_H__
#define __ALG_INPUT_DEVICE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "device_handler/video_source.h"

class ALGInputDevice
{
protected:
    ALGInputDevice();
    virtual ~ALGInputDevice();
public:
    virtual int   GetWidth(const int ch_id) const ;
    virtual int   GetHeight(const int ch_id) const ;
    virtual void* GetVideoSource(const int ch_id);

public:
    virtual void  RegisterDevice(VideoSourceParam* param);
    virtual bool  Init();
    virtual void  Wait();
    virtual void  Release();
    virtual void  CloseStreamAll();
    virtual void  StartStreamAll();
    virtual void  StartStream(const int ch_id);
    virtual int   GetVideoSourceNum()const;
    virtual void  LoopTimeSync();
    virtual void  LoopFrameSync();

public:
    static ALGInputDevice *GetInstance();
    static void onMSGCallback(void *data);   

protected:
    virtual int   SetSyncMode();

protected:
    std::thread   m_loop_th;

    uint64_t      t_now;
    uint64_t      t_last;
    float         m_frame_rate;
    int           m_sync_mode;
};


#endif