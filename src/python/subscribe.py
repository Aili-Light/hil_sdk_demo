import signal, sys
import time
import argparse
import numpy as np
import ctypes
from ctypes import *
import cv2

import algSDKpy
from algSDKpy import algSDKClient
from algSDKpy import pcie_image_data_t
from algSDKpy import callbackFunc_t

ALG_SDK_MIPI_DATA_TYPE_DEFAULT = 0
ALG_SDK_MIPI_DATA_TYPE_UYVY = 0x1C
ALG_SDK_MIPI_DATA_TYPE_VYUY = 0x1D
ALG_SDK_MIPI_DATA_TYPE_YUYV = 0x1E
ALG_SDK_MIPI_DATA_TYPE_YVYU = 0x1F
ALG_SDK_MIPI_DATA_TYPE_RAW10 = 0x2B
ALG_SDK_MIPI_DATA_TYPE_RAW12 = 0x2C

client = algSDKClient()

def array2mat(payload, w, h, data_type, ch_id, frame_index, image_name):
    p_array = np.frombuffer(np.ctypeslib.as_array(payload, shape=((h*w*2, 1, 1))), dtype=np.uint8)
    
    if (data_type == ALG_SDK_MIPI_DATA_TYPE_DEFAULT or data_type == ALG_SDK_MIPI_DATA_TYPE_YVYU 
    or data_type == ALG_SDK_MIPI_DATA_TYPE_YUYV or data_type == ALG_SDK_MIPI_DATA_TYPE_UYVY
    or data_type == ALG_SDK_MIPI_DATA_TYPE_VYUY):
        img_in = p_array.reshape((h, w, 2))
        if (data_type == ALG_SDK_MIPI_DATA_TYPE_DEFAULT or data_type == ALG_SDK_MIPI_DATA_TYPE_YVYU):
            img_out = cv2.cvtColor(img_in, cv2.COLOR_YUV2BGR_YVYU)
        elif (data_type == ALG_SDK_MIPI_DATA_TYPE_YUYV):
            img_out = cv2.cvtColor(img_in, cv2.COLOR_YUV2BGR_YUYV)
        elif (data_type == ALG_SDK_MIPI_DATA_TYPE_UYVY):
            img_out = cv2.cvtColor(img_in, cv2.COLOR_YUV2BGR_UYVY)
        elif (data_type == ALG_SDK_MIPI_DATA_TYPE_VYUY):
            img_out = cv2.cvtColor(img_in, cv2.COLOR_YUV2RGB_UYVY)
        img_disp = cv2.resize(img_out, (640, 360))
        cv2.imshow(bytes(image_name).decode("utf-8"), img_disp)
        
        filename = str("image_")+str("%02d_" % ch_id)+str("%08d_" % frame_index)+str(".bmp")
        c = cv2.waitKey(1)
        if c == 32:
            cv2.imwrite(filename, img_out)

    elif (data_type == ALG_SDK_MIPI_DATA_TYPE_RAW10):
        p_data = np.zeros(shape=(h*w, 1, 1), dtype=np.uint16)
        for i in range(0, int(h*w/4)):
            p_data[4*i] = (((np.ushort(p_array[5*i]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 0) & 0x0003));
            p_data[4*i+1] = (((np.ushort(p_array[5*i+1]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 2) & 0x0003));
            p_data[4*i+2] = (((np.ushort(p_array[5*i+2]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 4) & 0x0003));
            p_data[4*i+3] = (((np.ushort(p_array[5*i+3]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 6) & 0x0003));
        
        # filename = str("image_")+str("%02d_" % ch_id)+str("%08d_" % frame_index)+str(".raw")
        # np.save(filename, p_data)

def get_channel_id(topic_name):
    topic = bytes(topic_name)
    topic = topic.decode("utf-8")
    tx = topic.split("/")
    ch_id = int(tx[3])

    return ch_id

def CallbackFunc(ptr):
    p = ctypes.cast(ptr, ctypes.POINTER(pcie_image_data_t))
    payload = ctypes.cast(p.contents.payload, ctypes.POINTER(c_uint8))
    # print('[frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]' % (p.contents.image_info_meta.frame_index, p.contents.image_info_meta.img_size, payload[0], payload[p.contents.image_info_meta.img_size-1]))
    array2mat(payload, p.contents.image_info_meta.width, p.contents.image_info_meta.height, p.contents.image_info_meta.data_type, get_channel_id(p.contents.common_head.topic_name), p.contents.image_info_meta.frame_index, p.contents.common_head.topic_name)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Subscribe to image data"
    )
    parser.add_argument('--topic',
                        type=str,
                        help="topic name, example : --topic=/image_data/stream/00",
                        required=True
    )
    args = parser.parse_args()
    topic = args.topic
    topic_name = topic.encode('utf-8')
    
    callback_func = callbackFunc_t(CallbackFunc)
    client.Subscribe(topic_name, callback_func)
    ret = client.InitClient()
    if(ret < 0):
        print("Init Client Failed!")
        sys.exit(0)
    
    client.Spin()
