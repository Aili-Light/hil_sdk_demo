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
#ifndef __HIL_DEVICE_FROMDIR_H__
#define __HIL_DEVICE_FROMDIR_H__
#include "device_handler/alg_input_device.h"
#include "device_handler/video_source_fromDir.h"

class HILDeviceFromDir : public ALGInputDevice
{
protected:
    HILDeviceFromDir                   ();
    virtual ~HILDeviceFromDir          ();
public:
    virtual int                         GetWidth(const int ch_id)const ;
    virtual int                         GetHeight(const int ch_id)const ;
    virtual void*                       GetVideoSource(const int ch_id);
           
public:           
    virtual void                        RegisterDevice(VideoSourceParam* param);
    virtual bool                        Init();
    virtual void                        Wait();
    virtual void                        Release();
    virtual void                        CloseStreamAll();
    virtual void                        StartStreamAll();
    virtual int                         GetVideoSourceNum() const;
    virtual void                        LoopTimeSync();
    virtual void                        LoopFrameSync();
    virtual void                        SetCallbackFunc(alg_hil_callback_func func);

public:           
    static HILDeviceFromDir             *GetInstance();

protected:
    std::vector<VideoSourceFromDir*>   m_sources;

private:
    static void                         onMSGCallback(void *data);   

private:
    virtual int                         SetSyncMode();
    virtual void                        Loop(void* p);
    virtual void                        LoopSync(void* p);
};

#ifdef __cplusplus
extern "C" {
#endif
	HILDeviceFromDir* HILDevDir_GetInstance()
	{
		return HILDeviceFromDir::GetInstance();
	}

    void HILDevDir_RegisterDevice(HILDeviceFromDir* self, VideoSourceParam* param)
    {
        self->RegisterDevice(param);
    }

    bool HILDevDir_Init(HILDeviceFromDir* self)
    {
        return self->Init();
    }

    void HILDevDir_StartStreamAll(HILDeviceFromDir* self)
    {
        self->StartStreamAll();
    }

    void HILDevDir_CloseStreamAll(HILDeviceFromDir* self)
    {
        self->CloseStreamAll();
    }

    void HILDevDir_Wait(HILDeviceFromDir* self)
    {
        self->Wait();
    }

    void HILDevDir_SetCallbackFunc(HILDeviceFromDir* self, alg_hil_callback_func func)
    {
        self->SetCallbackFunc(func);
    }
}

#endif