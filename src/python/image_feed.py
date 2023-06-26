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

def image_show_yuv(buf, height, width):
    img = np.frombuffer(buf, dtype=np.uint8)
    img = img.reshape((height, width, 2))
    img = cv2.cvtColor(img, cv2.COLOR_YUV2BGR_YUYV)
    cv2.namedWindow("Image Show", cv2.WINDOW_NORMAL)
    cv2.imshow("Image Show", img)
    cv2.waitKey(0)

class ImageFeed():
    def __init__(self, h, w, ch, size):
        self.img_header = pcie_common_head_t()
        self.img_info = pcie_image_info_meta_t()
        self.img_data = pcie_image_data_t()

        self.height = h
        self.width = w
        self.img_size = size
        self.ch_id = ch
    
    def make_header(self):
        self.img_header.head = 55
        self.img_header.version = 1

        topic_name_s = ("/image_data/stream/%02d") % self.ch_id
        # print(topic_name_s)
        topic_name_s = topic_name_s.encode("utf-8")
        self.img_header.topic_name = topic_name_s
        self.img_header.crc8 = crc_array(ctypes.c_char_p(bytes(self.img_header)), 130)
        # print("*******crc8: %d" % self.img_header.crc8)

    def make_info(self):
        self.img_info.width = self.width
        self.img_info.height = self.height
        self.img_info.img_size = self.img_size
        self.img_info.exposure = 1.5
        self.img_info.again = 1.0
        self.img_info.dgain = 1.0
        self.img_info.temp = 25.0

    def make_data(self):
        self.make_header()
        self.make_info()
        self.img_data.common_head = self.img_header
        self.img_data.image_info_meta = self.img_info

    def feed_data(self, payload, frame_index, timestamp):
        self.img_data.image_info_meta.frame_index = frame_index
        self.img_data.image_info_meta.timestamp = timestamp
        img_ptr = ctypes.c_char_p(bytes(payload))
        # s = img_ptr.value
        self.img_data.payload = ctypes.cast(img_ptr, ctypes.c_void_p)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Feed Image to HIL"
    )
    parser.add_argument('--image_path',
                        type=str,
                        help="path of image",
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

    args = parser.parse_args()
    image_path = args.image_path
    w = args.width
    h = args.height
    ch_id = args.channel

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
        image_feed = ImageFeed(h,w,ch_id,img_size)
        image_feed.make_data()

        frm_cnt = 0
        while(1):
            # image_show_yuv(payload, h, w)
            image_feed.feed_data(payload, frm_cnt, 0)
            server.Publish(image_feed.img_data, ch_id)
            frm_cnt = frm_cnt + 1
            time.sleep(0.033333)

        server.Spin()

    # files = os.listdir(image_folder)
    # for file in files:
    #     file_path = os.path.join(image_folder, file)
    #     # print(file_path)
    #     img = cv2.imread(file_path, cv2.IMREAD_COLOR)
    #     # print(img.shape)

    # img_data = pcie_image_data_t()
    # img_data.payload = ctypes.c_void_p(img)

