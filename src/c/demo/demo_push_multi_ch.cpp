#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "alg_sdk/alg_sdk.h"
#include "alg_sdk/server.h"
#include "alg_sdk/pull.h"
#include "alg_common/basic_types.h"
#include "utils.h"
#ifndef __MINGW32__
#include <netinet/in.h> /* For htonl and ntohl */
#endif
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

static uint8_t payload[ALG_SDK_MAX_CHANNEL][ALG_SDK_PAYLOAD_LEN_MAX];
static pcie_common_head_t img_header[ALG_SDK_MAX_CHANNEL];
static pcie_image_info_meta_t img_info[ALG_SDK_MAX_CHANNEL];
static pcie_image_data_t img_data[ALG_SDK_MAX_CHANNEL];
static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_count[ALG_SDK_MAX_CHANNEL] = {0};
static sem_t sem_push;
static pthread_t g_main_loop;
static pthread_mutex_t g_mutex;
static uint64_t g_timer_last;
static vector<string> img_file_lists[ALG_SDK_MAX_CHANNEL];
static uint32_t file_list_sizes[ALG_SDK_MAX_CHANNEL];
static vector<string>::iterator it_mulch[ALG_SDK_MAX_CHANNEL];
static uint8_t channel_ids[ALG_SDK_MAX_CHANNEL];

int fatal(const char *msg)
{
    fprintf(stderr, "fatal error : %s", msg);
    exit(1);
}

void int_handler(int sig)
{
    printf("Caught signal : %d\n", sig);
    alg_sdk_stop_server();
    alg_sdk_stop_notify();

    pthread_mutex_destroy(&g_mutex);
    sem_destroy(&sem_push);

    exit(sig);
}

void safe_free(void *p)
{
    if (p != NULL)
        free(p);
}

int load_image(const char *filename, uint8_t *buffer, uint32_t *data_len)
{
    FILE *fp = fopen(filename, "r");
    uint32_t lSize;
    size_t result;

    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        lSize = ftell(fp);
        // printf("file size = %d\n", lSize);
        rewind(fp);
        // allocate memory to contain the whole file:
        // buffer = (uint8_t*) malloc (sizeof(uint8_t)*lSize);
        if (buffer == NULL)
        {
            fatal("Buffer allocation error\n");
        }
        // copy the file into the buffer:
        result = fread(buffer, 1, lSize, fp);
#ifndef __MINGW32__
        if (result != lSize)
        {
            fatal("Read file error\n");
        }
#endif
        // printf("size = %d, data = %d\n", lSize, buffer[4]);

        fclose(fp);
        *data_len = lSize;

        return 0;
    }
    else
    {
        fatal("Failed to load image\n");
    }

    return 0;
}

void load_image_path(string img_dir_path, vector<string> &img_path)
{
    DIR *pDir;
    struct dirent *ptr;
    if (!(pDir = opendir(img_dir_path.c_str())))
    {
        cout << "Folder doesn't Exist!" << endl;
        return;
    }

    while ((ptr = readdir(pDir)) != 0)
    {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
        {
            img_path.push_back(img_dir_path + "/" + ptr->d_name);
        }
    }
    sort(img_path.begin(), img_path.end());

    closedir(pDir);
}

void frame_monitor(const int ch_id, float *fps, const int frame_index)
{
    /* Monitor Frame Drop : current frame should be last frame +1,
        otherwise some frames may be lost.
    */

    /* Calculate Frame Rate */
    uint64_t t_now = milliseconds();
    g_f_count[ch_id]++;

    uint64_t delta_t = t_now - g_t_last[ch_id];
    if (delta_t > 1000) // for 1000 milliseconds
    {
        g_t_last[ch_id] = t_now;
        *fps = (float)g_f_count[ch_id] / delta_t * 1000.0f;
        printf("Frame Monitor : [Channel %d] [Index %d] [frm rate (SW) = %f]\n", ch_id, frame_index, *fps);
        g_f_count[ch_id] = 0;
    }
}

void push_callback(void *p)
{
    char *msg = (char *)p;
    printf("Notify Message : %s\n", msg);
}

int main(int argc, char **argv)
{
    uint32_t seq_ch[ALG_SDK_MAX_CHANNEL] = {0};

    if ((argc > 2) && (strcmp(argv[1], "--publish_multi") == 0))
    {
        int rc;
        const uint16_t num_channel = atoi(argv[2]);

        if (num_channel < 1)
        {
            fatal("ERROR! Number of Channel is 0!! \n");
        }

        /*
         *  Setup notify callback .
         */
        rc = alg_sdk_notify(push_callback);
        if (rc < 0)
        {
            fatal("Setup notify failed\n");
        }
        /* end */

        /* Init Servers */
        rc = alg_sdk_init_server();
        if (rc < 0)
        {
            fatal("Init server failed\n");
        }
        /* end */

        /* Init Parameters */
        memset(&img_header, 0, sizeof(img_header));
        memset(&img_info, 0, sizeof(img_info));
        memset(&img_data, 0, sizeof(img_data));
        memset(&channel_ids, 0, sizeof(channel_ids));

        for (int i = 0; i < num_channel; i++)
        {
            const char *filename = argv[4 * i + 3];
            const uint32_t image_width = atoi(argv[4 * i + 4]);
            const uint32_t image_height = atoi(argv[4 * i + 5]);
            const uint8_t channel_id = atoi(argv[4 * i + 6]);
            const uint32_t image_size = image_width * image_height * 2;
            channel_ids[i] = channel_id;

            uint32_t p_len;
            if (load_image(filename, payload[i], &p_len))
            {
                fatal("Read File Error!\n");
            }

            if (p_len == 0)
            {
                fatal("Data Size Error!\n");
            }

            if (image_size != p_len)
            {
                fatal("Image Size Not Match!\n");
            }

            /* Generate pcie image data head */
            img_header[i].head = 55;
            img_header[i].version = 1;
            int ch_id = channel_id;
            char topic_name[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {};
            sprintf(topic_name, "/image_data/stream/%02d", ch_id);
            strcpy(img_header[i].topic_name, topic_name);
            // printf("%s\n", img_header[i].topic_name);
            img_header[i].crc8 = crc_array((unsigned char *)&img_header[i], 130);
            /* end */

            /* Generate pcie image data info */
            img_info[i].frame_index = 0;
            /* **********************************
             *  IMPORTANT : Image size must MATCH the input image!
             * ********************************** */
            img_info[i].width = image_width;
            img_info[i].height = image_height;
            /* ********************************** */
            img_info[i].data_type = ALG_SDK_MIPI_DATA_TYPE_YUYV;
            img_info[i].exposure = 1.5;
            img_info[i].again = 1.0;
            img_info[i].dgain = 4.0;
            img_info[i].temp = 25.0;
            img_info[i].timestamp = milliseconds();
            img_info[i].img_size = image_size;
            /* end */
        }

        int freq = 30;
        g_timer_last = macroseconds();
        /* end */

        /* Main Loop */
        while (1)
        {
            /* Set frequency  */
            uint64_t t_now, delta_t;
            t_now = macroseconds();
            delta_t = t_now - g_timer_last;

            if (delta_t < 1000000 / freq)
                continue;

            g_timer_last = t_now;

            for (int i = 0; i < num_channel; i++)
            {
                img_info[i].frame_index = seq_ch[i];
                img_info[i].timestamp = milliseconds();

                img_data[i].common_head = img_header[i];
                img_data[i].image_info_meta = img_info[i];
                img_data[i].payload = (uint8_t *)payload[i];
                alg_sdk_push2q(&img_data[i], channel_ids[i]);

                // frame_monitor(channel_id, &fps, seq);
                seq_ch[i]++;
            }
            usleep(10000);
        }
        /* end */
    }
    else if ((argc > 2) && (strcmp(argv[1], "--feedin_multi") == 0))
    {
        int rc;
        const uint16_t num_channel = atoi(argv[2]);

        if (num_channel < 1)
        {
            fatal("ERROR! Number of Channel is 0!! \n");
        }

        /*
         *  Setup notify callback .
         */
        rc = alg_sdk_notify(push_callback);
        if (rc < 0)
        {
            fatal("Setup notify failed\n");
        }
        /* end */

        /* Init Servers */
        rc = alg_sdk_init_server();
        if (rc < 0)
        {
            fatal("Init server failed\n");
        }
        /* end */

        /* Init Parameters */
        memset(&img_header, 0, sizeof(img_header));
        memset(&img_info, 0, sizeof(img_info));
        memset(&img_data, 0, sizeof(img_data));
        memset(&img_file_lists, 0, sizeof(img_file_lists));
        memset(&file_list_sizes, 0, sizeof(file_list_sizes));
        memset(&channel_ids, 0, sizeof(channel_ids));

        for (int i = 0; i < num_channel; i++)
        {
            const char *foldername = argv[4 * i + 3];
            const uint32_t image_width = atoi(argv[4 * i + 4]);
            const uint32_t image_height = atoi(argv[4 * i + 5]);
            const uint8_t channel_id = atoi(argv[4 * i + 6]);
            const uint32_t image_size = image_width * image_height * 2;
            channel_ids[i] = channel_id;

            /* Generate pcie image data head */
            img_header[i].head = 55;
            img_header[i].version = 1;
            int ch_id = channel_id;
            char topic_name[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {};
            sprintf(topic_name, "/image_data/stream/%02d", ch_id);
            strcpy(img_header[i].topic_name, topic_name);
            // printf("%s\n", img_header[i].topic_name);
            img_header[i].crc8 = crc_array((unsigned char *)&img_header[i], 130);
            /* end */

            /* Generate pcie image data info */
            img_info[i].frame_index = 0;
            /* **********************************
             *  IMPORTANT : Image size must MATCH the input image!
             * ********************************** */
            img_info[i].width = image_width;
            img_info[i].height = image_height;
            /* ********************************** */
            img_info[i].data_type = ALG_SDK_MIPI_DATA_TYPE_YUYV;
            img_info[i].exposure = 1.5;
            img_info[i].again = 1.0;
            img_info[i].dgain = 4.0;
            img_info[i].temp = 25.0;
            img_info[i].timestamp = milliseconds();
            img_info[i].img_size = image_size;
            /* end */

            /*
             *  Read files from folder
             */
            printf("Open folder : %s\n", foldername);
            string folder_name = foldername;
            load_image_path(foldername, img_file_lists[i]);
            file_list_sizes[i] = img_file_lists[i].size();
            /* end */

            if (file_list_sizes[i] > 0)
            {
                img_data[i].payload = (uint8_t *)malloc(sizeof(uint8_t) * image_size);
                it_mulch[i] = img_file_lists[i].begin();
            }
            else
            {
                fatal("Read image list error!\n");
            }
        }

        int freq = 30;
        uint32_t seq = 0;
        uint32_t p_len = 0;
        g_timer_last = macroseconds();
        /* end */

        /* Main Loop */
        while (1)
        {
            /* Set frequency  */
            uint64_t t_now, delta_t;
            t_now = macroseconds();
            delta_t = t_now - g_timer_last;

            if (delta_t < 1000000 / freq)
                continue;

            g_timer_last = t_now;

            /* Feed data on each channel */
            for (int i = 0; i < num_channel; i++)
            {
                /* read image data from file */
                const char *filename = (*it_mulch[i]).c_str();
                uint8_t *payload = (uint8_t *)img_data[i].payload;
                load_image(filename, payload, &p_len);
                // printf("[SEQ:%d] [FILE:%s] [LEN:%d]\n", seq, filename, p_len);

                if (p_len != img_info[i].img_size) // image size does not match
                {
                    fatal("Image size do not match!\n");
                }

                img_info[i].frame_index = seq;
                img_info[i].timestamp = milliseconds();
                img_data[i].common_head = img_header[i];
                img_data[i].image_info_meta = img_info[i];
                alg_sdk_push2q(&img_data[i], channel_ids[i]);

                it_mulch[i]++;
                /* iterator never reach the end */
                if (it_mulch[i] == img_file_lists[i].end())
                {
                    // printf("reach end [%d]\n", i);
                    it_mulch[i] = img_file_lists[i].begin();
                }
            }
            usleep(10000);

            /* update sequence */
            seq++;
        }
        /* end */

        for (int i = 0; i < num_channel; i++)
        {
            safe_free(img_data[i].payload);
        }

        alg_sdk_server_spin_on();
        alg_sdk_notify_spin_on();
    }
    else
    {
        fprintf(stderr, "Usage: ./hil_sdk_demo_multi_ch_ch <TYP> <NUM_CHANNEL> <FILENAME> <ARG1> <ARG2> <ARG3>...\n");
        fprintf(stderr, "e.g. ./hil_sdk_demo_multi_ch_ch --publish_multi 2 'test_image_0.yuv' 1920 1280 0 'test_image_1' 3840 2160 1\n");
        fprintf(stderr, "e.g. ./hil_sdk_demo_multi_ch_ch --feedin_multi 2 'folder0/' 1920 1280 0 'folder1' 3840 2160 1\n");
    }

    return 0;
}