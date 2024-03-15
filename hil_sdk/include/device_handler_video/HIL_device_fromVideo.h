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
#ifndef __HIL_DEVICE_FROMVIDEO_H__
#define __HIL_DEVICE_FROMVIDEO_H__
#include "device_handler/alg_input_device.h"
#include "device_handler_video/video_source_fromVideo.h"

class HILDeviceFromVideo : public ALGInputDevice
{
protected:
    HILDeviceFromVideo                   ();
    virtual ~HILDeviceFromVideo          ();
public:
    virtual int                         GetWidth(const int ch_id)const ;
    virtual int                         GetHeight(const int ch_id)const ;
    virtual VideoSource*                GetVideoSource(const int ch_id);
    virtual VideoSource*                GetVideoSourceByIndex(int index);
           
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
    virtual void                        SetLogLevel(int level);

public:           
    static HILDeviceFromVideo            *GetInstance();

protected:
    std::vector<VideoSourceFromVideo*>   m_sources;

private:
    static void                         onMSGCallback(void *data);   

private:
    virtual int                         SetSyncMode();
    virtual void                        Loop(void* p);
    virtual void                        LoopSync(void* p);
    virtual void                        LoopBuffer(void* p);
};

#ifdef __cplusplus
extern "C" {
#endif
	HILDeviceFromVideo* HILDevVideo_GetInstance()
	{
		return HILDeviceFromVideo::GetInstance();
	}

    void HILDevVideo_RegisterDevice(HILDeviceFromVideo* self, VideoSourceParam* param)
    {
        self->RegisterDevice(param);
    }

    bool HILDevVideo_Init(HILDeviceFromVideo* self)
    {
        return self->Init();
    }

    void HILDevVideo_StartStreamAll(HILDeviceFromVideo* self)
    {
        self->StartStreamAll();
    }

    void HILDevVideo_CloseStreamAll(HILDeviceFromVideo* self)
    {
        self->CloseStreamAll();
    }

    void HILDevVideo_Wait(HILDeviceFromVideo* self)
    {
        self->Wait();
    }

    void HILDevVideo_Release(HILDeviceFromVideo* self)
    {
        self->Release();
    }

    void HILDevVideo_SetCallbackFunc(HILDeviceFromVideo* self, alg_hil_callback_func func)
    {
        self->SetCallbackFunc(func);
    }
#ifdef __cplusplus
}
#endif

#endif