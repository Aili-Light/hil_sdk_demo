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

static uint8_t payload[ALG_SDK_PAYLOAD_LEN_MAX];
static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_count[ALG_SDK_MAX_CHANNEL] = {0};
static sem_t sem_push;
static pthread_t g_main_loop;
static pthread_mutex_t g_mutex;
static uint64_t g_timer_last;

int fatal(const char *msg)
{
    fprintf(stderr, "fatal error : %s", msg);
    exit(1);
}

void int_handler(int sig)
{
    printf("Caught signal : %d\n", sig);
    alg_sdk_stop_server();

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

void copy_to_ringbuffer(const void *buffer, const void *img_data, const void *p_data)
{
    uint8_t *next_img = (uint8_t *)buffer;
    pcie_image_data_t *ptr = (pcie_image_data_t *)img_data;
    pcie_common_head_t *img_header = (pcie_common_head_t *)&(ptr->common_head);
    pcie_image_info_meta_t *img_info = (pcie_image_info_meta_t *)&(ptr->image_info_meta);
    uint8_t *payload = (uint8_t *)p_data;

    uint32_t pos = 0;
    uint32_t image_size = img_info->img_size;

    memcpy(next_img, img_header, sizeof(pcie_common_head_t));
    pos += sizeof(pcie_common_head_t);
    memcpy(next_img + pos, img_info, sizeof(pcie_image_info_meta_t));
    pos += sizeof(pcie_image_info_meta_t);
    uintptr_t addr = (uintptr_t)payload;
    memcpy(next_img + pos, &addr, sizeof(void *));
    pos += sizeof(void *);
    memcpy(next_img + pos, payload, image_size);
}

int main(int argc, char **argv)
{
    uint32_t seq = 0;

    if ((argc > 2) && (strcmp(argv[1], "--publish") == 0))
    {
        int rc;

        /* Init Servers */
        rc = alg_sdk_init_server();
        if (rc < 0)
        {
            fatal("Init server failed\n");
        }

        char *filename = argv[2];
        uint32_t p_len;
        const uint32_t image_width = atoi(argv[3]);
        const uint32_t image_height = atoi(argv[4]);
        const uint8_t channel_id = atoi(argv[5]);
        const uint32_t image_size = image_width * image_height * 2;

        /* User defined Data Type
         *  Default=YUYV (0x1E)
         *  Acceptable: UYVY (0x1C) / VYUY (0x1D) / YUYV (0x1E) / YVYU ((0x1F) / RAW10 (0x2B) / RAW12 (0x2C)
         */
        char data_type[64] = {"Default"};
        if (argc > 6)
        {
            // printf("argc : %s\n", argv[6]);
            strncpy(data_type, argv[6], strlen(argv[6]));
            printf("data type : [%s]\n", data_type);
        }

        if (load_image(filename, payload, &p_len))
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
        pcie_common_head_t img_header;
        img_header.head = 55;
        img_header.version = 1;
        int ch = channel_id;
        char topic_name[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {};
        sprintf(topic_name, "/image_data/stream/%02d", ch);
        strcpy(img_header.topic_name, topic_name);
        // printf("%s\n", img_header[i].topic_name);
        img_header.crc8 = crc_array((unsigned char *)&img_header, 130);
        /* end */

        /* Generate pcie image data info */
        pcie_image_info_meta_t img_info;
        img_info.frame_index = 0;
        /* IMPORTANT NOTE :
         *  Image size must MATCH the input image!
         */
        img_info.width = image_width;
        img_info.height = image_height;

        /* data type is user-defined */
        img_info.exposure = 1.5;
        img_info.again = 1.0;
        img_info.dgain = 1.0;
        img_info.temp = 25.0;
        img_info.timestamp = milliseconds();
        img_info.img_size = p_len;
        /* end */

        /* for raw data */
        const uint32_t data_size = image_width * image_height;
        uint16_t *pdata = (uint16_t *)malloc(sizeof(uint16_t) * data_size);

        while (1)
        {
            if (!payload || !p_len)
            {
                fatal("Empty data.\n");
            }

            float fps = 0.0f;
            img_info.frame_index = seq;
            img_info.timestamp = milliseconds();

            pcie_image_data_t img_data;
            img_data.common_head = img_header;
            img_data.image_info_meta = img_info;

            /* Data Type is UYVY/VYUY/YUYV/YVYU */
            if (strncmp(data_type, "YUYV", 4) == 0 || strncmp(data_type, "Default", 7) == 0)
            {
                // printf("****Match YUYV\n");
                img_data.payload = (uint8_t *)payload;
                img_info.data_type = ALG_SDK_MIPI_DATA_TYPE_YUYV;
            }
            else if (strncmp(data_type, "YVYU", 4) == 0)
            {
                img_data.payload = (uint8_t *)payload;
                img_info.data_type = ALG_SDK_MIPI_DATA_TYPE_YVYU;
            }
            else if (strncmp(data_type, "UYVY", 4) == 0)
            {
                img_data.payload = (uint8_t *)payload;
                img_info.data_type = ALG_SDK_MIPI_DATA_TYPE_UYVY;
            }
            else if (strncmp(data_type, "VYUY", 4) == 0)
            {
                img_data.payload = (uint8_t *)payload;
                img_info.data_type = ALG_SDK_MIPI_DATA_TYPE_VYUY;
            }
            else if (strncmp(data_type, "RAW10", 5) == 0)
            {
                // printf("****Match raw10\n");
                for (int i = 0; i < int(data_size / 4); i++)
                {
                    pdata[4 * i] = ((((payload[5 * i]) << 2) & 0x03FC) | ((payload[5 * i + 4] >> 0) & 0x0003));
                    pdata[4 * i + 1] = ((((payload[5 * i + 1]) << 2) & 0x03FC) | ((payload[5 * i + 4] >> 2) & 0x0003));
                    pdata[4 * i + 2] = ((((payload[5 * i + 2]) << 2) & 0x03FC) | ((payload[5 * i + 4] >> 4) & 0x0003));
                    pdata[4 * i + 3] = ((((payload[5 * i + 3]) << 2) & 0x03FC) | ((payload[5 * i + 4] >> 6) & 0x0003));
                }
                img_data.payload = (uint8_t *)pdata;
                img_info.data_type = ALG_SDK_MIPI_DATA_TYPE_RAW10;
            }
            else if (strncmp(data_type, "RAW12", 5) == 0)
            {
                // printf("****Match raw12\n");
                for (int i = 0; i < int(data_size / 2); i++)
                {
                    pdata[2 * i] = ((((payload[3 * i]) << 4) & 0x0FF0) | ((payload[3 * i + 2] >> 0) & 0x000F));
                    pdata[2 * i + 1] = ((((payload[3 * i + 1]) << 4) & 0x0FF0) | ((payload[3 * i + 2] >> 4) & 0x000F));
                }
                img_data.payload = (uint8_t *)pdata;
                img_info.data_type = ALG_SDK_MIPI_DATA_TYPE_RAW12;
            }
            else
            {
                fatal("Data Type Error!.\n");
            }

            alg_sdk_push2q(&img_data, channel_id);

            // frame_monitor(channel_id, &fps, seq);
            usleep(33333);
            // usleep(50000);
            seq++;
        }

        free(pdata);
        alg_sdk_server_spin_on();
    }
    else if ((argc > 2) && (strcmp(argv[1], "--feedin") == 0))
    {
        int rc;
        const char *foldername = argv[2];

        /* Init Servers */
        rc = alg_sdk_init_server();
        if (rc < 0)
        {
            fatal("Init server failed\n");
        }
        /* end */

        const uint32_t image_width = atoi(argv[3]);
        const uint32_t image_height = atoi(argv[4]);
        const uint8_t channel_id = atoi(argv[5]);
        const uint32_t image_size = image_width * image_height * 2;

        /* Generate pcie image data head */
        pcie_common_head_t img_header;
        img_header.head = 55;
        img_header.version = 1;
        int ch_id = channel_id;
        char topic_name[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {};
        sprintf(topic_name, "/image_data/stream/%02d", ch_id);
        strcpy(img_header.topic_name, topic_name);
        // printf("%s\n", img_header[i].topic_name);
        img_header.crc8 = crc_array((unsigned char *)&img_header, 130);
        /* end */

        /* Generate pcie image data info */
        pcie_image_info_meta_t img_info;
        img_info.frame_index = 0;
        /* **********************************
         *  IMPORTANT : Image size must MATCH the input image!
         * ********************************** */
        img_info.width = image_width;
        img_info.height = image_height;
        /* ********************************** */
        img_info.data_type = ALG_SDK_MIPI_DATA_TYPE_YUYV;
        img_info.exposure = 1.5;
        img_info.again = 1.0;
        img_info.dgain = 4.0;
        img_info.temp = 25.0;
        img_info.timestamp = milliseconds();
        img_info.img_size = image_size;
        /* end */

        /*
         *  Read files from folder
         */
        printf("Open folder : %s\n", foldername);
        string folder_name = foldername;
        vector<string> img_filenames;
        load_image_path(foldername, img_filenames);
        uint32_t v_size = img_filenames.size();
        /* end */

        if (v_size > 0)
        {
            uint32_t seq = 0;
            uint32_t p_len = 0;
            int freq = 30;

            pcie_image_data_t img_data;
            img_data.payload = (uint8_t *)malloc(sizeof(uint8_t) * image_size);

            g_timer_last = macroseconds();
            for (vector<string>::iterator it = img_filenames.begin();;)
            {
                /* Set frequency  */
                uint64_t t_now, delta_t;
                t_now = macroseconds();
                delta_t = t_now - g_timer_last;
                if (delta_t < 1000000 / freq)
                    continue;

                g_timer_last = t_now;

                /* read image data from file */
                const char *filename = (*it).c_str();
                uint8_t *payload = (uint8_t *)img_data.payload;
                load_image(filename, payload, &p_len);
                // printf("[SEQ:%d] [FILE:%s] [LEN:%d]\n", seq, filename, p_len);

                if (p_len != image_size) // image size does not match
                    break;

                /* copy data into ringbuffer */
                img_info.frame_index = seq;
                img_info.timestamp = milliseconds();
                img_data.common_head = img_header;
                img_data.image_info_meta = img_info;
                alg_sdk_push2q(&img_data, ch_id);

                /* update sequence */
                seq++;
                it++;

                /* iterator never reach the end */
                if (it == img_filenames.end())
                {
                    it = img_filenames.begin();
                }

                usleep(25000);
            }

            safe_free(img_data.payload);
        }
        else
        {
            fatal("ERROR! folder is empty\n");
        }

        alg_sdk_server_spin_on();
    }
    else
    {
        fprintf(stderr, "Usage: ./hil_sdk_demo_push_1_ch <TYP> <FILENAME> <ARG1> <ARG2> <ARG3>...\n");
        fprintf(stderr, "e.g. ./hil_sdk_demo_push_1_ch --publish 'test_image.yuv' 1920 1280 0\n");
        fprintf(stderr, "e.g. ./hil_sdk_demo_push_1_ch --feedin '/image_folder' 1920 1280 0\n");
    }

    return 0;
}