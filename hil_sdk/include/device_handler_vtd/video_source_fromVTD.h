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
#ifndef __VIDEO_SOURCE_FROM_VTD_H__
#define __VIDEO_SOURCE_FROM_VTD_H__
#include "device_handler/video_source.h"
#include "utils/image_feed.h"
#include "vtd/RDBHandler.hh"

class VideoSourceFromVTD : public VideoSource
{
public:
    VideoSourceFromVTD(VideoSourceParam *param);
    virtual ~VideoSourceFromVTD();

public:
    virtual int  InitVideoSource();
    virtual int  OpenVideoSource();
    virtual void StartSource();
    virtual void EndSource();
    virtual void Loop();
    virtual void WaitForFinish();
    virtual void FreeVideoSource();

private:
    /**
     * method for checking the contents of the SHM
     */
    int         openShm();
    int         checkShm();

    /**
     * routine for handling an RDB message; to be provided by user;
     * here, only a printing of the message is performed
     * @param msg    pointer to the message that is to be handled
     */
    void        handleMessage(RDB_MSG_t *msg);

    /**
     * Parse message and print it out
     * @param simTime
     * @param simFrame
     * @param entryHdr
     */
    void        parseRDBMessageEntry(const double &simTime, const unsigned int &simFrame, RDB_MSG_ENTRY_HDR_t *entryHdr, int &counter);

    /**
     * Handle a RDBImage and print it out
     * @param simTime
     * @param simFrame
     * @param img
     */
    void        handleRDBitem(const double &simTime, const unsigned int &simFrame, RDB_IMAGE_t *img, int counter);

private:
    uint8_t*     p_payload;
    std::string  m_filename;
    uint32_t     m_payload_len;
    uint32_t     m_image_size;

    uint16_t     m_ShmKey;
    uint16_t     m_CheckMask;
    int          m_ForceBuffer;
    bool         b_Verbose;
    bool         b_Saveimg;
    void*        m_ShmPtr = 0;        // pointer to the SHM segment
    uint32_t     m_ShmTotalSize = 0;  // remember the total size of the SHM segment

    ImageFeed    m_image_feed;
};


#endif