import sys
import argparse
import os
import numpy as np
import cv2
import ctypes
import time

import algSDKpy
from algSDKpy import algSDKServer
from algSDKpy import pcie_common_head_t
from algSDKpy import pcie_image_info_meta_t
from algSDKpy import pcie_image_data_t
from algSDKpy import crc_array
from algSDKpy import ImageFeed

def image_show_yuv(buf, height, width):
    img = np.frombuffer(buf, dtype=np.uint8)
    img = img.reshape((height, width, 2))
    img = cv2.cvtColor(img, cv2.COLOR_YUV2BGR_YUYV)
    cv2.namedWindow("Image Show", cv2.WINDOW_NORMAL)
    cv2.imshow("Image Show", img)
    cv2.waitKey(0)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Feed Image to HIL"
    )
    parser.add_argument('--image_path',
                        type=str,
                        help="image path",
                        required=True
    )
    parser.add_argument('--width',
                        type=int,
                        help="image width",
                        required=True
    )
    parser.add_argument('--height',
                        type=int,
                        help="image height",
                        required=True
    )
    parser.add_argument('--channel',
                        type=int,
                        help="channel id",
                        required=True
    )
    parser.add_argument('--data_type',
                        type=str,
                        help="data type",
                        required=True
    )

    args = parser.parse_args()
    image_path = args.image_path
    w = args.width
    h = args.height
    ch_id = args.channel
    dt = args.data_type

    server = algSDKServer()
    ret = server.InitServer()
    
    if ret != 0:
        print("Init Server Failed! [Code:%d]" % ret)
        sys.exit(0)

    try:
        with open(image_path, mode='rb') as file:
            payload = file.read()
    except IOError:
        print("Failed to load image from [%s]!" % image_path)
    else:
        img_size = len(payload)
        size = h * w * 2
        if img_size !=size:
            print('image size [%d] NOT match [H:%d][W:%d]!' % (img_size, h, w))
            sys.exit(0)
 
        # image_show_yuv(payload, h, w)
        image_feed = ImageFeed(h,w,ch_id,img_size,0)
        image_feed.make_data()

        # Set data type
        data_type = image_feed.set_data_type(dt)
        if data_type == 0:
            print("Set Data Type Error -- unsupported image format! [%s]" % dt)
            sys.exit(0)

        frm_cnt = 0
        while(1):
            # image_show_yuv(payload, h, w)
            image_feed.feed_data(payload, frm_cnt, 0)
            server.Publish(image_feed.img_data, ch_id)
            frm_cnt = frm_cnt + 1
            time.sleep(0.033333)

        server.Spin()