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

class VideoPublisher():
    def __init__(self):
        self.server = algSDKServer()
        self.feed = ImageFeed()
        self.data = None
        self.frame_index = 0
        self.timestamp = 0
    
    def init_server(self):
        ret = self.server.InitServer()
        return ret

    def init_feed(self,h,w,ch_id,size,dt):
        self.feed.init_feed(h,w,ch_id,size,dt)
        self.feed.make_data()
        self.feed.set_data_type('YUYV')  # decode video only support yuv

    def run(self):
        th1 = threading.Thread(target=VideoPublisher.publish, args=(self,))
        th1.start()

    def publish(self):
        while(1):
            # print(self.feed.img_data.image_info_meta.timestamp)
            self.server.Publish(self.feed.img_data,self.feed.ch_id)
            time.sleep(0.0333)

    def my_callback(self, array:np.ndarray):
        self.feed.feed_data(array.tobytes(), self.frame_index, self.timestamp)
        self.frame_index = self.frame_index+1
        self.timestamp = int(time.time()*1000)
        # feed.image_show_yuv(array.tobytes(), feed.height, feed.width)
        # print(self.timestamp)

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

    publisher = VideoPublisher()
    ret = publisher.init_server()
    if ret != 0:
        print("Init Server Failed! [Code:%d]" % ret)
        sys.exit(0) 

    size = h * w * 2  # for yuv, size=h*w*h; for rgb, size=h*w*3
    publisher.init_feed(h,w,ch_id,size,0)
    publisher.run()

    # # init video decoder
    decoder = VideoDecoderGst(w,h,ch_id,size,codec)
    launch_str = decoder.build_launch_str(video_path)
    if launch_str != None:
        print(launch_str)
    else:
        print("Build launch str Error!")
        sys.exit(0)

    decoder.init_decoder()
    decoder.set_callback(publisher.my_callback)
    decoder.decode_h264()

    # kill decoder thread : ctrl+\