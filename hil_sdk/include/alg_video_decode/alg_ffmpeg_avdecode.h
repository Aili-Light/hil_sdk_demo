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
#ifndef __ALG_FFMPEG_AVDECODE_H__
#define __ALG_FFMPEG_AVDECODE_H__
#include <stdio.h>
#include <stdlib.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}

typedef void(*alg_decode_callback_func)(void* p, int ch);

struct alg_ffmpeg_avdecode
{
    const AVCodec *codec;
    AVCodecContext *context;
    AVCodecParserContext* parser;
    AVFrame  *frame;
    AVPacket *pkt;
    AVFormatContext* format_ctx;

    FILE*    fp;
    uint8_t  *data_read;
    int      ch_id;
    int      stream_index;
    uint32_t frame_count;
    char     decoder_name[128];
};

int alg_ffmpeg_avdecode_init (void* c, const char* codec_name, int ch_id);
int alg_ffmpeg_avdecode_open(void* c, const char* filename);
void alg_ffmpeg_avdecode_close(void* c);
int alg_ffmpeg_avdecode_decode_file(void* c);
void alg_ffmpeg_avdecode_flush_decode(void* c);
int alg_ffmpeg_open_input_file(void* c, const char* filename);
void alg_ffmpeg_close_input_file(void* c);
void alg_ffmpeg_set_callback(alg_decode_callback_func func);

#endif