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
#ifndef __HIL_DEVICE_FROMVTD_H__
#define __HIL_DEVICE_FROMVTD_H__
#include "device_handler/alg_input_device.h"
#include "device_handler_vtd/video_source_fromVTD.h"

class HILDeviceFromVTD : public ALGInputDevice
{
protected:
    HILDeviceFromVTD();
    virtual ~HILDeviceFromVTD();
public:
    virtual int   GetWidth(const int ch_id)const ;
    virtual int   GetHeight(const int ch_id)const ;
    virtual void* GetVideoSource(const int ch_id);

public:
    virtual void  RegisterDevice(VideoSourceParam* param);
    virtual bool  Init();
    virtual void  Wait();
    virtual void  Release();
    virtual void  CloseStreamAll();
    virtual void  StartStreamAll();
    virtual int   GetVideoSourceNum() const;

public:
    static HILDeviceFromVTD *GetInstance();
    static void onMSGCallback(void *data);   

protected:
    std::vector<VideoSourceFromVTD*>   m_sources;
 
};

#ifdef __cplusplus
extern "C" {
#endif
	HILDeviceFromVTD* HILDevVTD_GetInstance()
	{
		return HILDeviceFromVTD::GetInstance();
	}

    void HILDevVTD_RegisterDevice(HILDeviceFromVTD* self, VideoSourceParam* param)
    {
        self->RegisterDevice(param);
    }

    bool HILDevVTD_Init(HILDeviceFromVTD* self)
    {
        return self->Init();
    }

    void HILDevVTD_StartStreamAll(HILDeviceFromVTD* self)
    {
        self->StartStreamAll();
    }

    void HILDevVTD_CloseStreamAll(HILDeviceFromVTD* self)
    {
        self->CloseStreamAll();
    }

    void HILDevVTD_Wait(HILDeviceFromVTD* self)
    {
        self->Wait();
    }
}

#endif