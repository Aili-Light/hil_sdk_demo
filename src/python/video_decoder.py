import sys
import argparse
import os
import numpy as np
import cv2
import ctypes
import time
import typing as typ

import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GObject, GLib, GstApp, GstVideo
import gstreamer.utils as utils
            
class VideoDecoderGst():
    def __init__(self, h, w, ch_id, size, codec):
        self.height = h
        self.width = w
        self.codec_type = codec
        self.app_sink = 'mysink'
        self.img_size = size
        self.publish_cb = None
        self.launch_str = None

    def register_callback(self, callback):
        self.callback = callback

    def build_launch_str(self, filename):
        if self.codec_type == 'h264':
            self.launch_str = ("filesrc location=%s ! qtdemux ! h264parse ! video/x-h264,stream-format=byte-stream,alignment=au ! nvh264dec  ! videoconvert ! video/x-raw,format=YUY2,pixel-aspect-ratio=1/1  ! appsink emit-signals=True name=%s") % (filename, self.app_sink)
        elif self.codec_type == 'h265':
            self.launch_str = ("filesrc location=%s ! qtdemux ! h265parse ! video/x-h265,stream-format=byte-stream,alignment=au ! nvh265dec  ! videoconvert ! video/x-raw,format=YUY2,pixel-aspect-ratio=1/1  ! appsink emit-signals=True name=%s") % (filename, self.app_sink)
        else:
            self.launch_str = None
            print("Set codec type error -- unsupported codec type! [%s]" % self.codec_type)
        
        return self.launch_str

    def init_decoder(self):
        Gst.init()
        self.pipeline = Gst.parse_launch(self.launch_str)
        appsink = self.pipeline.get_by_name(self.app_sink)
        # subscribe to <new-sample> signal
        appsink.connect("new-sample", self.on_buffer, None)

    def decode_h264(self):
        self.pipeline.set_state(Gst.State.PLAYING)
        # wait until EOS or error
        bus = self.pipeline.get_bus()
        msg = bus.timed_pop_filtered(
            Gst.CLOCK_TIME_NONE,
            Gst.MessageType.ERROR | Gst.MessageType.EOS
        )
        # free resources
        self.pipeline.set_state(Gst.State.NULL)

    def on_buffer(self, sink: GstApp.AppSink, data: typ.Any) -> Gst.FlowReturn:
        """Callback on 'new-sample' signal"""
        # Emit 'pull-sample' signal

        sample = sink.emit("pull-sample")  # Gst.Sample
        if isinstance(sample, Gst.Sample):
            array = self.extract_buffer(sample)
            print(
                "Received {type} with shape {shape} of type {dtype}".format(type=type(array),
                                                                            shape=array.shape,
                                                                            dtype=array.dtype))

            self.publish_cb(array)
            return Gst.FlowReturn.OK

        return Gst.FlowReturn.ERROR

    def extract_buffer(self, sample: Gst.Sample) -> np.ndarray:
        """Extracts Gst.Buffer from Gst.Sample and converts to np.ndarray"""

        buffer = sample.get_buffer()  # Gst.Buffer

        # print(buffer.pts, buffer.dts, buffer.offset)
        caps_format = sample.get_caps().get_structure(0)  # Gst.Structure

        # GstVideo.VideoFormat
        video_format = GstVideo.VideoFormat.from_string(
            caps_format.get_value('format'))

        w, h = caps_format.get_value('width'), caps_format.get_value('height')
        c = utils.get_num_channels(video_format)

        buffer_size = buffer.get_size()
        # print("h=%d,w=%d,c=%d" % (h,w,c))
        shape = (h, w, c) if (h * w * c == buffer_size) else buffer_size
        array = np.ndarray(shape=shape, buffer=buffer.extract_dup(0, buffer_size),
                        dtype=utils.get_np_dtype(video_format))

        # save_image_cv2(array, buffer.offset)
        return np.squeeze(array)  # remove single dimension if exists
    
    def set_callback(self, callback):
        self.publish_cb = callback