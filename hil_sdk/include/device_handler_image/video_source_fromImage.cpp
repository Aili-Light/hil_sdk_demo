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
#include "video_source_fromImage.h"
#include "utils/utils.h"
#include "utils/image_feed.h"
#include "alg_sdk/alg_sdk.h"
#include "alg_sdk/server.h"
#include "json.hpp"
#include "alg_common/log.h"
#include <fstream>

#include "opencv/cv.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

using json = nlohmann::json;

VideoSourceFromImage::VideoSourceFromImage(VideoSourceParam *param)
{
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, 1);
    m_thread_started = false;
    m_sync_mode = 0;
    m_is_dir = 0;

    std::ifstream f(param->config_file);
    printf("loading from config [%s]\n", param->config_file);

    json config = json::parse(f);
    if (!config.contains(std::string{"num_channels"}))
    {
        alg_sdk_log(LOG_ERROR,  "json must contain key [num_channels] !.\n");
        return;
    }

    int num_channels = config["num_channels"].get<int>();
    if (num_channels < 1)
    {
        alg_sdk_log(LOG_ERROR,  "Number of Channels must be larger than 1 [%d]!.\n", num_channels);
        return;
    }
    
    if (config.contains(std::string{"sync_mode"}))
    {
        m_sync_mode = config["sync_mode"].get<int>();
    }

    if (!config.contains(std::string{"channels"}))
    {
        alg_sdk_log(LOG_ERROR,  "json must contain key [channels] !.\n");
        return;
    }
    for (auto cfg : config["channels"])
    {
        // std::cout << "channel = " << cfg["channel_id"].get<int>() << "\n\n";
        int ch_id = cfg["source_id"].get<int>();
        if (param->source_id == ch_id)
        {
            m_image_width = cfg["source_width"].get<int>();
            m_image_height = cfg["source_height"].get<int>();
            m_ch_id = cfg["channel_id"].get<int>();
            m_fps = cfg["frame_rate"].get<float>();
            m_source_name = cfg["pathname"].get<std::string>();
            m_is_dir = cfg["is_dir"].get<int>();
            m_source_type = cfg["source_data_type"].get<std::string>();
            m_output_type = cfg["output_data_type"].get<std::string>();
            if (cfg.contains(std::string{"bayer_pattern"}))
            {
                m_bayer_type = cfg["bayer_pattern"].get<std::string>();
            }
            else
            {
                m_bayer_type = "RGGB";
            }
            break;
        }
    }
    // printf("[ch:%d],[height:%d],[width:%d],[file:%s],[fps:%f],[S_TYPE:%s],[O_TYPE:%s]\n", m_ch_id,m_image_height,
    // m_image_width,m_source_name.c_str(),m_fps,m_source_type.c_str(),m_output_type.c_str());
}

VideoSourceFromImage::~VideoSourceFromImage() 
{}

void VideoSourceFromImage::LoadImageFromDir()
{
    load_image_path(m_filepath.c_str(), s_img_filenames);
    m_filename_size = s_img_filenames.size();
}

int VideoSourceFromImage::LoadImageFromFile()
{
    int ret = 0;

    cv::Mat img_src = cv::imread(m_filepath.c_str(), cv::IMREAD_COLOR);
    if ( img_src.cols != m_image_width || img_src.rows != m_image_height)
    {
        alg_sdk_log(LOG_ERROR, "FILE SIZE not MATCH! Expect [%dx%d], but got [%dx%d]\n", m_image_width, m_image_height, img_src.cols, img_src.rows);
        return 1;
    }

    if ( img_src.channels() != 3 )
    {
        alg_sdk_log(LOG_ERROR, "Image channel must be 3 ! (Input [%d]\n)", img_src.channels());
        return 1;
    }
    
    m_payload_len = img_src.cols * img_src.rows * img_src.channels();
    memcpy(p_payload, img_src.data, m_payload_len);

    return ret;
}

int VideoSourceFromImage::InitVideoSource()
{
    m_filepath = m_source_name;
    m_image_size = m_image_width * m_image_height * 2;
    p_payload = (uint8_t*)malloc(ALG_SDK_PAYLOAD_LEN_MAX);
    m_seq = 0;
    m_filename_size = 0;

    /* Generate pcie image data */
    m_image_feed.InitFeed(m_image_width, m_image_height, m_ch_id, m_image_size, 0);
    /* end */

    /* Set source data type */
    if (m_image_feed.SetSourceType(m_source_type.c_str()) == 0)
    {
        alg_sdk_log(LOG_ERROR,  "Wrong Data Type!\n");
        return 1;
    }
    /* end */

    /* Set output data type */
    if (m_image_feed.SetOutputType(m_output_type.c_str()) == 0)
    {
        alg_sdk_log(LOG_ERROR,  "Wrong Data Type!\n");
        return 1;
    }
    /* end */

    /* Set bayer type */
    if (m_image_feed.SetOutputBayerType(m_bayer_type.c_str()) == 0)
    {
        alg_sdk_log(LOG_ERROR,  "Wrong Bayer Type!\n");
        return 1;
    }
    /* end */

    /* Set Frame Rate */
    m_image_feed.SetFrameRate(m_fps);
    /* end */

    return 0;
}

void VideoSourceFromImage::FreeVideoSource()
{
    if (p_payload)
    {
        free(p_payload);
    }
}

int  VideoSourceFromImage::OpenVideoSource()
{
    if (m_is_dir == 1)
    {
        /* Input is directory */
        LoadImageFromDir();

        if (m_filename_size == 0)
        {
            alg_sdk_log(LOG_ERROR,  "Data Size Error!\n");
            return 1;
        }
    }
    else
    {
        /* Input is an image */
        if (LoadImageFromFile())
        {
            alg_sdk_log(LOG_ERROR,  "Load image failed !\n");
            return 1;
        }

        if (!p_payload)
        {
            alg_sdk_log(LOG_ERROR,  "Empty data.\n");
            return 1;
        }

        if (m_payload_len == 0)
        {
            alg_sdk_log(LOG_ERROR,  "Data Size Error!\n");
            return 1;
        }
    }

    return 0;
}

void VideoSourceFromImage::StartSource()
{
    m_seq = 0;
    it_file = s_img_filenames.begin();
}

void VideoSourceFromImage::EndSource()
{}

void VideoSourceFromImage::Loop()
{}

void VideoSourceFromImage::SendData()
{
    if (m_is_dir == 1)
    {
        /* read image data from file */
        const char *filename = (*it_file).c_str();
        cv::Mat img_src = cv::imread(filename, cv::IMREAD_COLOR);
        if ( img_src.cols != m_image_width || img_src.rows != m_image_height)
        {
            alg_sdk_log(LOG_ERROR, "FILE SIZE not MATCH! Expect [%dx%d], but got [%dx%d]\n", m_image_width, m_image_height, img_src.cols, img_src.rows);
            return;
        }

        if ( img_src.channels() != 3 )
        {
            alg_sdk_log(LOG_ERROR, "Image channel must be 3 ! (Input [%d]\n)", img_src.channels());
            return;
        }

        memcpy(p_payload, img_src.data, img_src.cols * img_src.rows * img_src.channels());

        /* iterator move forward */
        it_file++;
        /* iterator never reach the end */
        if (it_file == s_img_filenames.end())
        {
            it_file = s_img_filenames.begin();
        }
    }

    /* copy data into ringbuffer */
    t_now = milliseconds();
    m_image_feed.FeedData((uint8_t *)p_payload, m_seq, t_now);
    void *img_data_ptr = m_image_feed.GetDataPtr();
    // pcie_image_data_t *data_p = (pcie_image_data_t *)img_data_ptr;
    // printf("topic : %s, index : [%d]\n", data_p->common_head.topic_name, data_p->image_info_meta.frame_index);
    alg_sdk_push2q(img_data_ptr, m_ch_id);

    m_seq++;
}

void VideoSourceFromImage::WaitForFinish()
{}