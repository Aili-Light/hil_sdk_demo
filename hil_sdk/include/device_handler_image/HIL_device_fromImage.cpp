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
#include "HIL_device_fromImage.h"
#include "alg_sdk/alg_sdk.h"
#include "alg_sdk/server.h"
#include "alg_sdk/client.h"
#include "alg_common/log.h"
#include "utils/utils.h"

static alg_hil_callback_func callback_func_fromImage;
static DeviceRuntimeArgs_t g_device_args_fromImage[ALG_SDK_MAX_CHANNEL];
HILDeviceFromImage* p_device_fromImage = nullptr;

HILDeviceFromImage::HILDeviceFromImage() 
{
    m_thread_started = false;
    m_sync_mode = 0;

    for (int i=0; i<ALG_SDK_MAX_CHANNEL; i++)
    {
        g_device_args_fromImage[i].buffer_count = 0;
        g_device_args_fromImage[i].last_frame = 0;
        g_device_args_fromImage[i].t_now = 0;
        g_device_args_fromImage[i].t_last = 0;
        g_device_args_fromImage[i].thread_ready = false;
    }    
}

HILDeviceFromImage::~HILDeviceFromImage() 
{
    for(std::thread* p_thread : thread_list)
    {
        if (p_thread != NULL)
        {
            delete p_thread;
        }
    }

    for(VideoSourceFromImage* p_source : m_sources)
    {
        if (p_source != NULL)
        {
            delete p_source;
        }
    }

    if (p_device_fromImage!=NULL)
        delete p_device_fromImage;
}

HILDeviceFromImage *HILDeviceFromImage::GetInstance()
{
    if(p_device_fromImage == nullptr)
        p_device_fromImage = new HILDeviceFromImage();

    return p_device_fromImage;
}

int HILDeviceFromImage::GetHeight(const int ch_id) const
{
    for(VideoSourceFromImage* p_source : m_sources)
    {
        if(p_source->GetChannelId() == ch_id)
            return p_source->GetHeight();
    }

    return 0;
}

int HILDeviceFromImage::GetWidth(const int ch_id) const
{
    for(VideoSourceFromImage* p_source : m_sources)
    {
        if(p_source->GetChannelId() == ch_id)
            return p_source->GetWidth();
    }

    return 0;
}

VideoSource* HILDeviceFromImage::GetVideoSource(const int ch_id)
{
    for(VideoSourceFromImage* p_source : m_sources)
    {
        if(p_source->GetChannelId() == ch_id)
            return p_source;
    }

    return nullptr;
}

VideoSource* HILDeviceFromImage::GetVideoSourceByIndex(int index)
{
    if(index < 0 || index >= m_sources.size())
        return nullptr;
    else
        return m_sources[index];
}

void HILDeviceFromImage::RegisterDevice(VideoSourceParam* param)
{
    m_sources.push_back(new VideoSourceFromImage(param));
}

bool HILDeviceFromImage::Init()
{
    /* Init Servers */
    int rc = 0;
    rc = alg_sdk_init_server(ALG_SDK_SERVER_FLAG_PUB2DEV);
    if (rc < 0)
    {
        alg_sdk_log(LOG_ERROR, "Init server failed\n");
        return false;
    }

    static char topic_name[ALG_SDK_MAX_CHANNEL][256] = { {0} };

    for(VideoSourceFromImage* p_source : m_sources)
    {
        if(p_source->InitVideoSource())
        {
            alg_sdk_log(LOG_ERROR, "Init Video Source ERROR!\n");
            return false;
        }

        if(p_source->OpenVideoSource())
        {
            alg_sdk_log(LOG_ERROR, "Open Video Source ERROR!\n");
            return false;
        }

        if(p_source->GetChannelId() < 0 || p_source->GetChannelId() >= ALG_SDK_MAX_CHANNEL)
        {
            alg_sdk_log(LOG_ERROR, "Channel ID [%d] out of boudary!!\n", p_source->GetChannelId());
            return false;
        }

        snprintf(topic_name[p_source->GetChannelId()], 256, "/feedback/hil_message/%02d", p_source->GetChannelId());
        rc = alg_sdk_subscribe(topic_name[p_source->GetChannelId()], &HILDeviceFromImage::onMSGCallback);
        if (rc < 0)
        {
            alg_sdk_log(LOG_ERROR, "Subscribe to topic Error\n");
            return false;
        }
    }

    if (alg_sdk_init_client())
    {
        alg_sdk_log(LOG_ERROR, "Init Client Error!\n");
        return false;
    }

    if(SetSyncMode())
    {
        alg_sdk_log(LOG_ERROR, "Set Sync Mode Error!\n");
        return false;
    }

    if (m_sync_mode == 0)
    {
        for(VideoSourceFromImage* p_source : m_sources)
        {
            std::thread* p_thread = new std::thread(&HILDeviceFromImage::Loop, this, (void*)p_source);
            thread_list.push_back(p_thread);
        }
    }
    else if(m_sync_mode == 1)
    {
        m_loop_th = std::thread(&HILDeviceFromImage::LoopTimeSync, this);
        for(VideoSourceFromImage* p_source : m_sources)
        {
            std::thread* p_thread = new std::thread(&HILDeviceFromImage::LoopSync, this, (void*)p_source);
            thread_list.push_back(p_thread);
        }
    }
    else if (m_sync_mode == 2)
    {
        m_loop_th = std::thread(&HILDeviceFromImage::LoopFrameSync, this);
        for(VideoSourceFromImage* p_source : m_sources)
        {
            std::thread* p_thread = new std::thread(&HILDeviceFromImage::LoopSync, this, (void*)p_source);
            thread_list.push_back(p_thread);
        }
    }
    else if (m_sync_mode == 3)
    {
        for(VideoSourceFromImage* p_source : m_sources)
        {
            std::thread* p_thread = new std::thread(&HILDeviceFromImage::LoopBuffer, this, (void*)p_source);
            thread_list.push_back(p_thread);
        }
    }
    else
    {
        alg_sdk_log(LOG_ERROR, "Set Sync Mode ERROR! Expect [0, 1, 2, 3], but got [%d]\n", m_sync_mode);
        return false;
    }

    alg_sdk_log(LOG_VERBOSE, "---------Init source success----------\n");
    return true;
}

void HILDeviceFromImage::CloseStreamAll()
{
    for(VideoSourceFromImage* p_source : m_sources)
    {
        p_source->EndSource();
    }
    m_thread_started = false;
}

void HILDeviceFromImage::StartStreamAll()
{
    for(VideoSourceFromImage* p_source : m_sources)
    {
        p_source->StartSource();
    }
    m_thread_started = true;
}

void HILDeviceFromImage::Wait()
{
    for(std::thread* p_thread : thread_list)
    {
        if (p_thread->joinable())
        {
            p_thread->join();
        }
    }

    for(VideoSourceFromImage* p_source : m_sources)
    {
        p_source->WaitForFinish();
    }

    if (m_loop_th.joinable())
        m_loop_th.join();
    
    alg_sdk_client_spin_on();
}

int  HILDeviceFromImage::GetVideoSourceNum() const
{
    return m_sources.size();
}

void  HILDeviceFromImage::Release()
{
    for(VideoSourceFromImage* p_source : m_sources)
    {
        p_source->FreeVideoSource();
    }
}

void  HILDeviceFromImage::onMSGCallback(void *data)
{
    if (data == NULL)
        return;

    hil_mesg_t *msg = (hil_mesg_t*)data;
    g_device_args_fromImage[msg->msg_meta.ch_id].last_frame = msg->msg_meta.frame_index;
    g_device_args_fromImage[msg->msg_meta.ch_id].buffer_count = msg->msg_meta.buffer_count;

    if (callback_func_fromImage != NULL)
        callback_func_fromImage(data);
}

void HILDeviceFromImage::LoopTimeSync()
{
    alg_sdk_log(LOG_VERBOSE, "Start Control Loop Time Sync At Rate [%f]\n", m_frame_rate);
    while (1)
    {
        /* Set frequency  */
        uint64_t delta_t;
        t_now = macroseconds();
        delta_t = t_now - t_last;
        if (delta_t < 1000000 / m_frame_rate)
        {
            usleep(1000);
            continue;
        }
        t_last = t_now;

        std::lock_guard<std::mutex> locker{m_mutex};
        for(VideoSourceFromImage* p_source : m_sources)
        {
            g_device_args_fromImage[p_source->GetChannelId()].thread_ready = true;
        }
        cond_var.notify_all();
    }
}

void HILDeviceFromImage::LoopFrameSync()
{
    alg_sdk_log(LOG_VERBOSE, "Start Control Loop Frame Sync At Rate [%f]\n", m_frame_rate);    
    std::vector<int> channel_ids;
    std::vector<int> max_ids;
    std::vector<int> proceed_ids;
    std::vector<unsigned short> frame_ids;

    for(VideoSourceFromImage* p_source : m_sources)
    {
        channel_ids.push_back(p_source->GetChannelId());
        proceed_ids.push_back(p_source->GetChannelId());
    }

    while (1)
    {
        /* Set frequency  */
        uint64_t delta_t;
        t_now = macroseconds();
        delta_t = t_now - t_last;
        if (delta_t < 1000000 / m_frame_rate)
        {
            usleep(1000);
            continue;
        }
        t_last = t_now;

        max_ids.clear();
        frame_ids.clear();

        /* wait for proceeding channels to finish */
        std::lock_guard<std::mutex> locker{m_mutex};

        /* get latest frame index for all channels */
        for(VideoSourceFromImage* p_source : m_sources)
        {
            frame_ids.push_back(p_source->GetLatestFrame());
        }

        /* check if frame index are identical */
        bool b_is_same_frame = check_vector_same(frame_ids);
        
        if (b_is_same_frame)
        {
            /* if all frame index are identical */
            /* proceed all channels */
            proceed_ids.clear();
            for (int i = 0; i < ALG_SDK_MAX_CHANNEL; ++i)
            {
                g_device_args_fromImage[i].thread_ready = true;
            }
        }
        else
        {
            /* if frame ids are NOT identical */
            /* proceed channels except the highest ones */
            /* NOTE: highest frame id can be more than one channel */
            int ret = find_highest_frame(channel_ids, frame_ids, max_ids);
            if (ret == 0)
            {
                proceed_ids.clear();
                std::vector<int>::iterator it_ch_id=channel_ids.begin();
                for(; it_ch_id!=channel_ids.end(); ++it_ch_id)
                {
                    bool b_is_matched = false;
                    /* if ch_id match one of max_ids */
                    std::vector<int>::iterator it_max_id = max_ids.begin();
                    for(; it_max_id!=max_ids.end(); ++it_max_id)
                    {
                        if (*it_ch_id == *it_max_id)
                        {
                            b_is_matched = true;
                            break;
                        }
                    }

                    if (b_is_matched == false)
                    {
                        proceed_ids.push_back(*it_ch_id);
                    }
                }

                /* debug message */
                alg_sdk_log(LOG_VERBOSE, "**Proceed with channels : ");
                std::vector<int>::iterator it_pr_ch_id=proceed_ids.begin();
                for(; it_pr_ch_id!=proceed_ids.end(); ++it_pr_ch_id)
                {
                    alg_sdk_log(LOG_VERBOSE, "[%d] ", *it_pr_ch_id);
                }
                alg_sdk_log(LOG_VERBOSE, "holding channels : ");
                std::vector<int>::iterator it_max_id=max_ids.begin();
                for(; it_max_id!=max_ids.end(); ++it_max_id)
                {
                    alg_sdk_log(LOG_VERBOSE, "[%d] ", *it_max_id);
                }
                alg_sdk_log(LOG_VERBOSE, "\n");

                /* proceed some channels */
                for(it_pr_ch_id=proceed_ids.begin(); it_pr_ch_id!=proceed_ids.end(); ++it_pr_ch_id)
                {
                    g_device_args_fromImage[*it_pr_ch_id].thread_ready = true;
                }
            }
        }

        /* proceed threads */
        cond_var.notify_all();
    }
}

int HILDeviceFromImage::SetSyncMode()
{
    std::vector<float> frame_rates;
    frame_rates.clear();

    /* get frame rate from all channels */
    for(VideoSourceFromImage* p_source : m_sources)
    {
        int sync_mode = p_source->GetSyncMode();
        if (sync_mode!=0)
        {
            m_sync_mode = sync_mode;
            frame_rates.push_back(p_source->GetFrameRate());
        }
    }

    /* check if all channels have same frame rate */
    bool b_is_same_rate = check_vector_same(frame_rates);
    if (m_sync_mode !=0)
    {
        if (b_is_same_rate)
        {
            m_frame_rate = frame_rates[0];
        }
        else
        {
            /* if sync mode is not 0 */
            /* channels must have same frame rate */
            alg_sdk_log(LOG_ERROR, "**Set sync mode Error! Want to set [%d], but frame rates not same!\n", m_sync_mode);
            return 1;
        }
    }

    return 0;
}
void HILDeviceFromImage::LoopSync(void* p)
{
    VideoSourceFromImage* p_source = (VideoSourceFromImage*)p;
    DeviceRuntimeArgs_t* p_devArgs = g_device_args_fromImage;

    int   m_ch_id = p_source->GetChannelId();
    alg_sdk_log(LOG_VERBOSE, "Start injection loop(sync) on channel [%d]\n", m_ch_id);
    while (1)
    {    
        /* Get time now */
        (p_devArgs+m_ch_id)->t_now = macroseconds();
        /* wait for thread start */
        if (!m_thread_started)
        {
            usleep(1000);
            continue;
        }

        /* wait for continue signal */
        unique_lock<mutex> locker(m_mutex);
        cond_var.wait(locker, [m_ch_id]()
            { return g_device_args_fromImage[m_ch_id].thread_ready; });

        (p_devArgs+m_ch_id)->thread_ready = false;
        locker.unlock();

        // alg_sdk_log(LOG_VERBOSE, "****ch : %d, time : %ld, TF : [%d]\n", m_ch_id, g_t_now_fromDir[m_ch_id], g_thread_ready_fromDir[m_ch_id]);
        p_source->SendData();
    }
    
    alg_sdk_log(LOG_VERBOSE, "--------source thread finish--------\n");
}

void HILDeviceFromImage::Loop(void* p)
{
    VideoSourceFromImage* p_source = (VideoSourceFromImage*)p;
    DeviceRuntimeArgs_t* p_devArgs = g_device_args_fromImage;

    int   m_ch_id = p_source->GetChannelId();
    float m_fps = p_source->GetFrameRate();
    (p_devArgs+m_ch_id)->t_last = macroseconds();
    alg_sdk_log(LOG_VERBOSE, "Start injection loop on channel [%d] at [%f]\n", m_ch_id, m_fps);
    while (1)
    {    
        /* wait for thread start */
        if (!m_thread_started)
        {
            usleep(1000);
            continue;
        }

        /* Set frequency  */
        uint64_t delta_t;
        (p_devArgs+m_ch_id)->t_now = macroseconds();
        delta_t = (p_devArgs+m_ch_id)->t_now - (p_devArgs+m_ch_id)->t_last;
        if (delta_t < 1000000 / m_fps)
        {
            usleep(1000);
            continue;
        }

        (p_devArgs+m_ch_id)->t_last = (p_devArgs+m_ch_id)->t_now;

        // printf("****ch : %d, time : %ld\n", m_ch_id, g_t_now_fromDir[m_ch_id]);
        p_source->SendData();
    }
    
    alg_sdk_log(LOG_VERBOSE, "--------source thread finish--------\n");
}

void HILDeviceFromImage::LoopBuffer(void* p)
{
    VideoSourceFromImage* p_source = (VideoSourceFromImage*)p;
    DeviceRuntimeArgs_t* p_devArgs = g_device_args_fromImage;

    int   m_ch_id = p_source->GetChannelId();
    float m_fps = p_source->GetFrameRate();
    uint64_t m_frame_count = 0;
    uint64_t delta_t;

    (p_devArgs+m_ch_id)->t_last = macroseconds();
    (p_devArgs+m_ch_id)->frame_rate = m_fps;

    alg_sdk_log(LOG_VERBOSE, "Start injection loop buffer control channel [%d]\n", m_ch_id);
    while (1)
    {    
        /* wait for thread start */
        if (!m_thread_started)
        {
            usleep(1000);
            continue;
        }

        /* Set frequency  */
        (p_devArgs+m_ch_id)->t_now = macroseconds();
        delta_t = (p_devArgs+m_ch_id)->t_now - (p_devArgs+m_ch_id)->t_last;
        if (delta_t < 1000000 / (p_devArgs+m_ch_id)->frame_rate)
        {
            usleep(1000);
            continue;
        }
        (p_devArgs+m_ch_id)->t_last = (p_devArgs+m_ch_id)->t_now;

        if (m_frame_count==0)
        {
            (p_devArgs+m_ch_id)->frame_rate = m_frame_rate;
            p_source->SendData();
        }
        else
        {
            if ((p_devArgs+m_ch_id)->buffer_count >= 0  && (p_devArgs+m_ch_id)->buffer_count < 50)
            {
                /* buffer count < 25% */
                /* buffer is almost empty */
                (p_devArgs+m_ch_id)->frame_rate = m_frame_rate * 2;
                p_source->SendData();
            }
            else if ((p_devArgs+m_ch_id)->buffer_count >= 50 && (p_devArgs+m_ch_id)->buffer_count < 75)
            {
                (p_devArgs+m_ch_id)->frame_rate = m_frame_rate;
                p_source->SendData();
            }
            else if ((p_devArgs+m_ch_id)->buffer_count >= 75)
            {
                /* buffer count > 75% */
                /* buffer is almost full */
                (p_devArgs+m_ch_id)->frame_rate = m_frame_rate;
            }
        }
        m_frame_count++;
    }
    
    alg_sdk_log(LOG_VERBOSE, "--------source thread finish--------\n");
}

void  HILDeviceFromImage::SetCallbackFunc(alg_hil_callback_func func)
{
    callback_func_fromImage = func;
}