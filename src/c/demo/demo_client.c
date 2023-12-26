#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <sys/time.h>
#include "alg_sdk/client.h"
#include "alg_common/basic_types.h"

const char topic_image_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_IMAGE_DATA};
const char topic_dev_poc_head_d[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_POC_INFO};

static uint64_t g_t_last[ALG_SDK_MAX_CHANNEL] = {0};
static int g_f_count[ALG_SDK_MAX_CHANNEL] = {0};
static uint32_t g_f_last[ALG_SDK_MAX_CHANNEL] = {0};

/*  Return the UNIX time in milliseconds.  You'll need a working
    gettimeofday(), so this won't work on Windows.  */
uint64_t milliseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((uint64_t)tv.tv_sec * 1000) + ((uint64_t)tv.tv_usec / 1000));
}

void frame_rate_monitor(const int ch_id, const int frame_index)
{
    uint64_t t_now = milliseconds();
    g_f_count[ch_id]++;
    uint64_t delta_t = t_now - g_t_last[ch_id];
    if (delta_t > 1000) // for 1000 milliseconds
    {
        g_t_last[ch_id] = t_now;
        printf("Frame Monitor : [Channel %d] [Index %d] [Frame Rate = %f]\n", ch_id, frame_index, (float)g_f_count[ch_id] / delta_t * 1000.0f);
        g_f_count[ch_id] = 0;
    }
}

int get_channel_id(const pcie_image_data_t *msg)
{
    const char *ptr_topic = &msg->common_head.topic_name[19];
    char c_ch_id[2];
    strncpy(c_ch_id, ptr_topic, 2);
    int ch_id = atoi(c_ch_id);

    return ch_id;
}

void save_image_raw(const char *filename, void *image_ptr, uint32_t image_size)
{
    FILE *fp;
    fp = fopen(filename, "wb");
    
    if (fp < 0)
    {
        return;
    }

    if (fwrite(image_ptr, 1, image_size, fp) < image_size)
    {
        printf("Out of memory!\n");
    }

    fclose(fp);
}

void callback_image_data(void *p)
{
    pcie_image_data_t *msg = (pcie_image_data_t *)p;
    // printf("[frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]\n",
    //         msg->image_info_meta.frame_index,  msg->image_info_meta.timestamp, ((uint8_t*)msg->payload)[0], ((uint8_t*)msg->payload)[msg->image_info_meta.img_size - 1]);

    /* check frame rate (every 1 second) */
    frame_rate_monitor(get_channel_id(msg), msg->image_info_meta.frame_index);
}

void callback_image_save(void *p)
{
    pcie_image_data_t *msg = (pcie_image_data_t *)p;
    //    printf("[frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]\n",
    //           msg->image_info_meta.frame_index,  msg->image_info_meta.timestamp, ((uint8_t*)msg->payload)[0], ((uint8_t*)msg->payload)[msg->image_info_meta.img_size - 1]);
    
    /* check frame rate (every 1 second) */
    frame_rate_monitor(get_channel_id(msg), msg->image_info_meta.frame_index);

    int ch_id = get_channel_id(msg);
    uint64_t timestamp = msg->image_info_meta.timestamp;
    uint32_t img_size = msg->image_info_meta.img_size;
    uint8_t* payload = msg->payload;
    char filename_raw[128] = {};
    sprintf(filename_raw, "data/image_%02d_%lu.raw", ch_id, timestamp);
    save_image_raw(filename_raw, payload, img_size);
}

void callback_poc_info(void *p)
{
    pcie_poc_info_t *msg = (pcie_poc_info_t *)p;

    printf("[time : %ld], [cur = %f], [vol = %f]\n",
           msg->poc_info_meta.timestamp, msg->poc_info_meta.cur, msg->poc_info_meta.vol);
}

void int_handler(int sig)
{
    printf("Caught signal : %d\n", sig);
    alg_sdk_stop_client();

    /* terminate program */
    exit(sig);
}

int main(int argc, char **argv)
{
    signal(SIGINT, int_handler);

    if ((argc == 2) && (strcmp(argv[1], "-all_topics") == 0))
    {
        int rc;

        printf("Client subscribe to topic [%s] and [%s]\n", topic_image_head_d, topic_dev_poc_head_d);
        rc = alg_sdk_subscribe(topic_image_head_d, callback_image_data);
        if (rc < 0)
        {
            printf("Subscribe to topic %s Error!\n", topic_image_head_d);
            exit(0);
        }
        rc = alg_sdk_subscribe(topic_dev_poc_head_d, callback_poc_info);
        if (rc < 0)
        {
            printf("Subscribe to topic %s Error!\n", topic_dev_poc_head_d);
            exit(0);
        }

        if (alg_sdk_init_client())
        {
            printf("Init Client Error!\n");
            exit(0);
        }

        alg_sdk_client_spin_on();
    }
    else if ((argc == 3) && (strcmp(argv[1], "-image") == 0))
    {
        int rc;
        const char *topic_name = argv[2];
        printf("Client subscribe to topic [%s]\n", topic_name);

        if (strncmp(topic_name, topic_image_head_d, strlen(topic_image_head_d)) == 0)
        {
            rc = alg_sdk_subscribe(topic_name, callback_image_data);
            if (rc < 0)
            {
                printf("Subscribe to topic %s Error!\n", topic_name);
                exit(0);
            }
        }

        if (alg_sdk_init_client())
        {
            printf("Init Client Error!\n");
            exit(0);
        }

        alg_sdk_client_spin_on();
    }
    else if ((argc == 2) && (strcmp(argv[1], "-all_images") == 0))
    {
        int rc;
        char image_topic_names[ALG_SDK_MAX_CHANNEL][256];
        for (int i = 0; i < ALG_SDK_MAX_CHANNEL; i++)
        {
            snprintf(image_topic_names[i], 256, "/image_data/stream/%02d", i);
            printf("Client [%02d] subscribe to topic [%s]\n", i, image_topic_names[i]);

            rc = alg_sdk_subscribe(image_topic_names[i], callback_image_data);
            if (rc < 0)
            {
                printf("Subscribe to topic %s Error!\n", image_topic_names[i]);
                exit(0);
            }
        }

        if (alg_sdk_init_client())
        {
            printf("Init Client Error!\n");
            exit(0);
        }

        alg_sdk_client_spin_on();
    }
    else if ((argc == 3) && (strcmp(argv[1], "-image_save") == 0))
    {
        int rc;
        const char *topic_name = argv[2];
        printf("Client subscribe to topic [%s]\n", topic_name);

        if (strncmp(topic_name, topic_image_head_d, strlen(topic_image_head_d)) == 0)
        {
            rc = alg_sdk_subscribe(topic_name, callback_image_save);
            if (rc < 0)
            {
                printf("Subscribe to topic %s Error!\n", topic_name);
                exit(0);
            }
        }

        if (alg_sdk_init_client())
        {
            printf("Init Client Error!\n");
            exit(0);
        }

        alg_sdk_client_spin_on();
    }

    return 0;
}
