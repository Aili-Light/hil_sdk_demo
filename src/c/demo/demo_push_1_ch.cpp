#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "alg_sdk/alg_sdk.h"
#include "alg_sdk/server.h"
#include "alg_sdk/pull.h"
#include "alg_common/basic_types.h"
#include "utils.h"
#ifndef __MINGW32__
#include <netinet/in.h>  /* For htonl and ntohl */
#endif
#include <unistd.h>

static uint8_t payload[ALG_SDK_PAYLOAD_LEN_MAX];
static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_count[ALG_SDK_MAX_CHANNEL] = {0};

int fatal(const char* msg)
{
    fprintf(stderr, "fatal error : %s", msg);
    exit(1);
}

void int_handler(int sig)
{
    printf("Caught signal : %d\n", sig);
    alg_sdk_stop_server();
    alg_sdk_stop_notify();
    exit(sig);
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
        printf("size = %d, data = %d\n", lSize, buffer[4]);

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

void push_callback(void* p)
{
    char* msg = (char*)p;
    printf("Notify Message : %s\n", msg);
}

int main(int argc, char **argv)
{
    uint32_t seq = 0;

    if ((argc > 2) && (strcmp(argv[1], "--publish") == 0))
    {
        int rc;

        /* Setup notify callback */
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

        char* filename = argv[2];
        uint32_t p_len;
        const uint32_t image_width = atoi(argv[3]);
        const uint32_t image_height = atoi(argv[4]);
        const uint8_t  channel_id = atoi(argv[5]);
        const uint32_t image_size = image_width * image_height * 2;

        if (load_image(filename, payload, &p_len))
        {
            fatal("Read File Error!\n");
        }

        if(p_len == 0)
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
        /* */
        img_info.data_type = ALG_SDK_MIPI_DATA_TYPE_YUYV;
        img_info.exposure = 1.5;
        img_info.again = 1.0;
        img_info.dgain = 1.0;
        img_info.temp = 25.0;
        img_info.timestamp = milliseconds();
        img_info.img_size = p_len;
        /* end */

        while (1)
        {
            if (!payload || !p_len)
            {
                fatal("Empty data.\n");
            }

            float fps = 0.0f;
            img_info.frame_index = seq;
            pcie_image_data_t img_data;

            img_data.common_head = img_header;
            img_data.image_info_meta = img_info;
            img_data.payload = (uint8_t *)payload;
            alg_sdk_push2q(&img_data, channel_id);

            // frame_monitor(channel_id, &fps, seq);
            usleep(33333);
            // usleep(50000);
            seq++;
        }

        alg_sdk_server_spin_on();
        alg_sdk_notify_spin_on();
    }
    else
    {
        fprintf(stderr, "Usage: ./hil_sdk_demo_push_1_ch <TYP> <FILENAME> <ARG1> <ARG2> <ARG3>...\n");
        fprintf(stderr, "e.g. ./hil_sdk_demo_push_1_ch --publish 'test_image.yuv' 1920 1280 0\n");
    }

    return 0;
}