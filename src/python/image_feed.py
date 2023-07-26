import sys
import argparse
import os
import numpy as np
import cv2
import ctypes
import time
import typing as typ

from algSDKpy import pcie_common_head_t
from algSDKpy import pcie_image_info_meta_t
from algSDKpy import pcie_image_data_t
from algSDKpy import crc_array

ALG_SDK_MIPI_DATA_TYPE_UYVY = 0x1C,    # Type UYVY (2-bytes) */
ALG_SDK_MIPI_DATA_TYPE_VYUY = 0x1D,    # Type VYUY (2-bytes) */
ALG_SDK_MIPI_DATA_TYPE_YUYV = 0x1E,    # Type YUYV (2-bytes) */
ALG_SDK_MIPI_DATA_TYPE_YVYU = 0x1F,    # Type YVYU (2-bytes) */
ALG_SDK_MIPI_DATA_TYPE_RAW10 = 0x2B,   # Type RAW10 (1.25-bytes) */
ALG_SDK_MIPI_DATA_TYPE_RAW12 = 0x2C,   # Type RAW12 (1.5-bytes) */
ALG_SDK_MIPI_DATA_TYPE_RAW10_PAD = 0x3B,   # Type RAW10 PADDING (16bit zero-padding) */
ALG_SDK_MIPI_DATA_TYPE_RAW12_PAD = 0x3C,   # Type RAW12 PADDING (16bit zero-padding) */

class ImageFeed():
    def __init__(self):
        self.img_header = pcie_common_head_t()
        self.img_info = pcie_image_info_meta_t()
        self.img_data = pcie_image_data_t()

        self.height = 0
        self.width = 0
        self.img_size = 0
        self.ch_id = 0
        self.data_type = 0
    
    def init_feed(self, h, w, ch, size, type):
        self.height = h
        self.width = w
        self.img_size = size
        self.ch_id = ch
        self.data_type = type

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

    def feed_data_yuv422(self, payload, frame_index, timestamp):
        self.img_data.image_info_meta.frame_index = frame_index
        self.img_data.image_info_meta.timestamp = timestamp
        img_ptr = ctypes.c_char_p(bytes(payload))
        # s = img_ptr.value
        self.img_data.payload = ctypes.cast(img_ptr, ctypes.c_void_p)

    def feed_data_raw10_pad(self, payload, frame_index, timestamp):
        self.img_data.image_info_meta.frame_index = frame_index
        self.img_data.image_info_meta.timestamp = timestamp
        img_ptr = ctypes.c_char_p(bytes(payload))
        # s = img_ptr.value
        self.img_data.payload = ctypes.cast(img_ptr, ctypes.c_void_p)

    def feed_data_raw12_pad(self, payload, frame_index, timestamp):
        self.img_data.image_info_meta.frame_index = frame_index
        self.img_data.image_info_meta.timestamp = timestamp
        img_ptr = ctypes.c_char_p(bytes(payload))
        # s = img_ptr.value
        self.img_data.payload = ctypes.cast(img_ptr, ctypes.c_void_p)

    def feed_data_raw10(self, payload, frame_index, timestamp):
        p_array = np.frombuffer(np.ctypeslib.as_array(payload, shape=((self.img_size, 1, 1))), dtype=np.uint8)
        p_data = np.zeros(shape=(self.height*self.width, 1, 1), dtype=np.uint16)
        for i in range(0, int(self.height*self.width/4)):
            p_data[4*i] = (((np.ushort(p_array[5*i]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 0) & 0x0003))
            p_data[4*i+1] = (((np.ushort(p_array[5*i+1]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 2) & 0x0003))
            p_data[4*i+2] = (((np.ushort(p_array[5*i+2]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 4) & 0x0003))
            p_data[4*i+3] = (((np.ushort(p_array[5*i+3]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 6) & 0x0003))

        img_ptr = p_data.ctypes.data_as(ctypes.c_char_p)
        # s = img_ptr.value
        self.img_data.image_info_meta.frame_index = frame_index
        self.img_data.image_info_meta.timestamp = timestamp
        self.img_data.payload = ctypes.cast(img_ptr, ctypes.c_void_p)

    def feed_data_raw12(self, payload, frame_index, timestamp):
        p_array = np.frombuffer(np.ctypeslib.as_array(payload, shape=((self.img_size, 1, 1))), dtype=np.uint8)
        p_data = np.zeros(shape=(self.height*self.width, 1, 1), dtype=np.uint16)
        for i in range(0, int(self.height*self.width/2)):
            p_data[2*i] = (((np.ushort(p_array[3*i]) << 4) & 0x0FF0) | np.ushort((p_array[3*i+2] >> 0) & 0x000F))
            p_data[2*i+1] = (((np.ushort(p_array[3*i+1]) << 4) & 0x0FF0) | np.ushort((p_array[3*i+2] >> 4) & 0x000F))

        img_ptr = p_data.ctypes.data_as(ctypes.c_char_p)
        # s = img_ptr.value
        self.img_data.image_info_meta.frame_index = frame_index
        self.img_data.image_info_meta.timestamp = timestamp
        self.img_data.payload = ctypes.cast(img_ptr, ctypes.c_void_p)

    def feed_data(self, payload, frame_index, timestamp):
        if ( self.data_type == ALG_SDK_MIPI_DATA_TYPE_YVYU or self.data_type == ALG_SDK_MIPI_DATA_TYPE_YUYV 
        or self.data_type == ALG_SDK_MIPI_DATA_TYPE_UYVY or self.data_type == ALG_SDK_MIPI_DATA_TYPE_VYUY):
            self.feed_data_yuv422(payload, frame_index, timestamp)
        elif ( self.data_type == ALG_SDK_MIPI_DATA_TYPE_RAW10 ):
            self.feed_data_raw10(payload, frame_index, timestamp)
        elif ( self.data_type == ALG_SDK_MIPI_DATA_TYPE_RAW12 ):
            self.feed_data_raw12(payload, frame_index, timestamp)
        elif ( self.data_type == ALG_SDK_MIPI_DATA_TYPE_RAW10_PAD):
            self.feed_data_raw10_pad(payload, frame_index, timestamp)
        elif ( self.data_type == ALG_SDK_MIPI_DATA_TYPE_RAW12_PAD):
            self.feed_data_raw12_pad(payload, frame_index, timestamp)

    def set_data_type(self, c_data_type):
        if c_data_type == 'YUYV':
            self.data_type = ALG_SDK_MIPI_DATA_TYPE_YUYV
        elif c_data_type == 'YVYU':
            self.data_type = ALG_SDK_MIPI_DATA_TYPE_YVYU
        elif c_data_type == 'UYVY':
            self.data_type = ALG_SDK_MIPI_DATA_TYPE_UYVY
        elif c_data_type == 'VYUY':
            self.data_type = ALG_SDK_MIPI_DATA_TYPE_VYUY
        elif c_data_type == 'RAW10':
            self.data_type = ALG_SDK_MIPI_DATA_TYPE_RAW10
        elif c_data_type == 'RAW12':
            self.data_type = ALG_SDK_MIPI_DATA_TYPE_RAW12
        elif c_data_type == 'RAW10-PAD':
            self.data_type = ALG_SDK_MIPI_DATA_TYPE_RAW10_PAD
        elif c_data_type == 'RAW12-PAD':
            self.data_type = ALG_SDK_MIPI_DATA_TYPE_RAW12_PAD
        else:
            self.data_type = 0

        return self.data_type    

    def image_show_yuv(self, buf, height, width):
        img = np.frombuffer(buf, dtype=np.uint8)
        img = img.reshape((height, width, 2))
        img = cv2.cvtColor(img, cv2.COLOR_YUV2BGR_YUYV)
        cv2.namedWindow("Image Show", cv2.WINDOW_NORMAL)
        cv2.imshow("Image Show", img)
        cv2.waitKey(1)