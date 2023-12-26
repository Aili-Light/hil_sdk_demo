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
#ifndef __DECODER_FF_H__
#define __DECODER_FF_H__
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <semaphore.h>
#include "alg_video_decode/alg_ffmpeg_avdecode.h"

struct DecoderParam
{
    int         fps;
    int         ch_id;
    std::string codec_name;
    std::string input_filename;
};

class Decoder_FF
{
public:
    Decoder_FF              (DecoderParam &param);
    ~Decoder_FF             ();
public:            
    int                     Init();
    void                    SetCallback(alg_decode_callback_func func);
    void                    Start();
    void                    Start(int flag);
    void                    End();
    void                    NextFrame();
    void                    WaitForFinish();

public:
    std::string             GetCodecName() {return m_codec_name;};
    int                     GetFrameRate() {return m_fps;};

private:
    static void             OnFrameArrive(void*, int);

private:
    void                    Loop();
    void                    LoopSync(int ch_id);
    int                     Decode();
    int                     InitDecoder();
    int                     OpenEncoder();
    int                     OpenFileOut();
    void                    Free();
    void                    CloseFileOut();
    void                    FlushDecoder();

private:
    int                     m_fps;
    int                     m_flag;
    int                     m_ch_id;
    bool                    m_thread_started;
    DecoderParam            m_dec_param;

    std::string             m_codec_name;
    std::string             m_output_file;
    std::string             m_input_file;
    alg_ffmpeg_avdecode     decoder;

    std::thread             m_thr;
    std::mutex              m_mutex;
    std::condition_variable cond_var; 

};

#endif
