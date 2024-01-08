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
#ifndef __HIL_DEVICE_FROMFILE_H__
#define __HIL_DEVICE_FROMFILE_H__
#include "device_handler/alg_input_device.h"
#include "device_handler/video_source_fromFile.h"

class HILDeviceFromFile : public ALGInputDevice
{
protected:
    HILDeviceFromFile                   ();
    virtual ~HILDeviceFromFile          ();
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
     
public:           
    static HILDeviceFromFile            *GetInstance();

protected:
    std::vector<VideoSourceFromFile*>   m_sources;

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
	HILDeviceFromFile* HILDevFile_GetInstance()
	{
		return HILDeviceFromFile::GetInstance();
	}

    void HILDevFile_RegisterDevice(HILDeviceFromFile* self, VideoSourceParam* param)
    {
        self->RegisterDevice(param);
    }

    bool HILDevFile_Init(HILDeviceFromFile* self)
    {
        return self->Init();
    }

    void HILDevFile_StartStreamAll(HILDeviceFromFile* self)
    {
        self->StartStreamAll();
    }

    void HILDevFile_CloseStreamAll(HILDeviceFromFile* self)
    {
        self->CloseStreamAll();
    }

    void HILDevFile_Wait(HILDeviceFromFile* self)
    {
        self->Wait();
    }

    void HILDevFile_SetCallbackFunc(HILDeviceFromFile* self, alg_hil_callback_func func)
    {
        self->SetCallbackFunc(func);
    }
#ifdef __cplusplus
}
#endif

#endif