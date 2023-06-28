import sys
import argparse
import os
import numpy as np
import cv2
import ctypes
from ctypes import *
import time
import typing as typ
import threading
import numpy as np

import algSDKpy
from algSDKpy import algSDKServer
from algSDKpy import pcie_image_data_t
from video_decoder import VideoDecoderGst
from image_feed import ImageFeed

server = algSDKServer()
feed = ImageFeed()

def my_callback(array:np.ndarray):
    feed.image_show_yuv(array.tobytes(), feed.height, feed.width)
    # print("callback : publish")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Feed Video to HIL"
    )
    parser.add_argument('--video_path',
                        type=str,
                        help="video path",
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
    parser.add_argument('--codec_type',
                        type=str,
                        help="codec type",
                        required=True
    )

    args = parser.parse_args()
    video_path = args.video_path
    w = args.width
    h = args.height
    ch_id = args.channel
    codec = args.codec_type

    # ret = server.InitServer()
    # if ret != 0:
    #     print("Init Server Failed! [Code:%d]" % ret)
    #     sys.exit(0)

    size = h * w * 2  # for yuv, size=h*w*h; for rgb, size=h*w*3
    feed.init_feed(h,w,ch_id,size,30)
    feed.make_data()

    # init video decoder
    decoder = VideoDecoderGst(w,h,ch_id,size,'h264')
    decoder.build_launch_str(video_path)
    decoder.init_decoder()
    decoder.set_publisher(my_callback)
    decoder.decode_h264()