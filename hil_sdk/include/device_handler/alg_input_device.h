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
#include <mutex>
#include <thread>
#include <condition_variable>

typedef void(*alg_hil_callback_func)(void* p);

class VideoSource;
struct VideoSourceParam;

class ALGInputDevice
{
protected:
    ALGInputDevice            ();
    virtual ~ALGInputDevice   ();
public:
    virtual int               GetWidth(const int ch_id) const ;
    virtual int               GetHeight(const int ch_id) const ;
    virtual VideoSource*      GetVideoSource(const int ch_id);
    virtual VideoSource*      GetVideoSourceByIndex(int index);
public:            
    virtual void              RegisterDevice(VideoSourceParam* param);
    virtual bool              Init();
    virtual void              Wait();
    virtual void              Release();
    virtual void              CloseStreamAll();
    virtual void              StartStreamAll();
    virtual void              StartStream(const int ch_id);
    virtual int               GetVideoSourceNum()const;
    virtual void              LoopTimeSync();
    virtual void              LoopFrameSync();
    virtual void              SetCallbackFunc(alg_hil_callback_func func);

public:
    static ALGInputDevice     *GetInstance();

protected:
    virtual int               SetSyncMode();
    virtual void              Loop(void* p);
    virtual void              LoopSync(void* p);
    virtual void              LoopBuffer(void* p);
    static void               onMSGCallback(void *data);   

protected:
    std::thread               m_loop_th;
    std::vector<std::thread*> thread_list;
    std::mutex                m_mutex;
    std::condition_variable   cond_var; 

protected:
    uint64_t                  t_now;
    uint64_t                  t_last;
    float                     m_frame_rate;
    int                       m_sync_mode;
    bool                      m_thread_started;
};

typedef struct DeviceRuntimeArgs
{
    uint64_t                  t_last;      
    uint64_t                  t_now;       
    bool                      thread_ready;
    uint32_t                  last_frame;  
    uint32_t                  buffer_count;
    uint32_t                  buffer_len;
    float                     frame_rate;
}DeviceRuntimeArgs_t;

#endif