import sys
import argparse
import os
import gi
import typing as typ
import numpy as np
import cv2

gi.require_version('Gst', '1.0')
from gi.repository import Gst, GObject, GLib, GstApp, GstVideo
import gstreamer.utils as utils

def save_image_cv2(image_in, index):
    image_path = ('./images/image_%d.jpg') % index
    # print(image_path)
    cv2.imwrite(image_path, image_in)

def on_debug(category, level, dfile, dfctn, dline, source, message, user_data):
    if source:
        print('Debug {} {}: {}'.format(
            Gst.DebugLevel.get_name(level), source.name, message.get()))
    else:
        print('Debug {}: {}'.format(
            Gst.DebugLevel.get_name(level), message.get()))
            
def extract_buffer(sample: Gst.Sample) -> np.ndarray:
    """Extracts Gst.Buffer from Gst.Sample and converts to np.ndarray"""

    buffer = sample.get_buffer()  # Gst.Buffer

    print(buffer.pts, buffer.dts, buffer.offset)

    caps_format = sample.get_caps().get_structure(0)  # Gst.Structure

    # GstVideo.VideoFormat
    video_format = GstVideo.VideoFormat.from_string(
        caps_format.get_value('format'))

    w, h = caps_format.get_value('width'), caps_format.get_value('height')
    c = utils.get_num_channels(video_format)

    buffer_size = buffer.get_size()
    shape = (h, w, c) if (h * w * c == buffer_size) else buffer_size
    array = np.ndarray(shape=shape, buffer=buffer.extract_dup(0, buffer_size),
                       dtype=utils.get_np_dtype(video_format))

    # save_image_cv2(array, buffer.offset)
    return np.squeeze(array)  # remove single dimension if exists

def on_buffer(sink: GstApp.AppSink, data: typ.Any) -> Gst.FlowReturn:
    """Callback on 'new-sample' signal"""
    # Emit 'pull-sample' signal
    # https://lazka.github.io/pgi-docs/GstApp-1.0/classes/AppSink.html#GstApp.AppSink.signals.pull_sample

    sample = sink.emit("pull-sample")  # Gst.Sample

    if isinstance(sample, Gst.Sample):
        array = extract_buffer(sample)
        print(
            "Received {type} with shape {shape} of type {dtype}".format(type=type(array),
                                                                        shape=array.shape,
                                                                        dtype=array.dtype))
        return Gst.FlowReturn.OK

    return Gst.FlowReturn.ERROR

def video_convert_avi2mp4(file_in):
    filename = (os.path.splitext(video_file)[0])
    filetype = (os.path.splitext(video_file)[1])
    file_out = filename + '.mp4'

    if filetype != '.avi':
        print("Wrong File Type ! Must be .avi !")
        return
    
    launch_str = ("filesrc location=%s ! avidemux name=demux ! video/x-h264 ! h264parse ! qtmux ! filesink location=%s") % (video_file, file_out)
    print(launch_str)

    Gst.init()
    pipeline = Gst.parse_launch(launch_str)
    pipeline.set_state(Gst.State.PLAYING)

    # wait until EOS or error
    bus = pipeline.get_bus()
    msg = bus.timed_pop_filtered(
        Gst.CLOCK_TIME_NONE,
        Gst.MessageType.ERROR | Gst.MessageType.EOS
    )
    # free resources
    pipeline.set_state(Gst.State.NULL)

def video_play_avi_h264(file_in):
    filetype = (os.path.splitext(video_file)[1])
    print(filetype)

    if filetype != '.avi':
        print("Wrong File Type ! Must be .avi !")
        return
    
    launch_str = ("filesrc location=%s ! avidemux name=demux ! video/x-h264 ! h264parse ! nvh264dec ! autovideosink sync=false") % (video_file)
    print(launch_str)

    Gst.init()
    pipeline = Gst.parse_launch(launch_str)
    pipeline.set_state(Gst.State.PLAYING)

    # wait until EOS or error
    bus = pipeline.get_bus()
    msg = bus.timed_pop_filtered(
        Gst.CLOCK_TIME_NONE,
        Gst.MessageType.ERROR | Gst.MessageType.EOS
    )
    # free resources
    pipeline.set_state(Gst.State.NULL)


def video_play_mp4_h264(file_in):
    filetype = (os.path.splitext(video_file)[1])
    print(filetype)

    if filetype != '.mp4':
        print("Wrong File Type ! Must be .mp4 !")
        return
    
    launch_str = ("filesrc location=%s ! qtdemux name=demux ! video/x-h264 ! h264parse ! nvh264dec ! autovideosink sync=false") % (video_file)
    print(launch_str)

    Gst.init()
    pipeline = Gst.parse_launch(launch_str)
    pipeline.set_state(Gst.State.PLAYING)

    # wait until EOS or error
    bus = pipeline.get_bus()
    msg = bus.timed_pop_filtered(
        Gst.CLOCK_TIME_NONE,
        Gst.MessageType.ERROR | Gst.MessageType.EOS
    )
    # free resources
    pipeline.set_state(Gst.State.NULL)

def video_avimux_h264(file_in):
    filename = (os.path.splitext(video_file)[0])
    filetype = (os.path.splitext(video_file)[1])
    file_out = filename + '.mp4'
    print(file_out)

    if filetype != '.h264':
        print("Wrong File Type ! Must be .avi !")
        return
    
    launch_str = ("filesrc location=%s ! h264parse ! avimux ! filesink location=%s") % (video_file, file_out)
    print(launch_str)

    Gst.init()
    pipeline = Gst.parse_launch(launch_str)
    pipeline.set_state(Gst.State.PLAYING)

    # wait until EOS or error
    bus = pipeline.get_bus()
    msg = bus.timed_pop_filtered(
        Gst.CLOCK_TIME_NONE,
        Gst.MessageType.ERROR | Gst.MessageType.EOS
    )
    # free resources
    pipeline.set_state(Gst.State.NULL)

def video_decode_h264(video_file):
    print('decode video file [%s]' % video_file)
    app_sink_name = 'mysink'

    # launch_str = ("filesrc location=%s ! h264parse ! decodebin ! queue ! videoconvert ! fakesink") % (video_file)
    launch_str = ("filesrc location=%s ! qtdemux ! video/x-h264 ! h264parse ! avdec_h264  ! videoconvert ! video/x-raw,format=RGB,pixel-aspect-ratio=1/1 ! appsink name=%s") % (video_file, app_sink_name)
    print(launch_str)

    Gst.init()
    pipeline = Gst.parse_launch(launch_str)
    appsink = pipeline.get_by_name(app_sink_name)
    # subscribe to <new-sample> signal
    appsink.connect("new-sample", on_buffer, None)

    pipeline.set_state(Gst.State.PLAYING)
    # wait until EOS or error
    bus = pipeline.get_bus()
    msg = bus.timed_pop_filtered(
        Gst.CLOCK_TIME_NONE,
        Gst.MessageType.ERROR | Gst.MessageType.EOS
    )
    # free resources
    pipeline.set_state(Gst.State.NULL)


def video_convert_test_appsink(video_file):
    app_sink_name = 'mysink'
    launch_str = ("videotestsrc num-buffers=100 ! video/x-raw,format=RGB,width=640, height=480 ! queue ! appsink emit-signals=True name=%s") % (app_sink_name)
    print(launch_str) 

    Gst.init()
    pipeline = Gst.parse_launch(launch_str)
    appsink = pipeline.get_by_name(app_sink_name)

    # appsink = pipeline.get_by_cls(GstApp.AppSink)[0]  # get AppSink
    # subscribe to <new-sample> signal
    appsink.connect("new-sample", on_buffer, None)

    pipeline.set_state(Gst.State.PLAYING)
    # wait until EOS or error
    bus = pipeline.get_bus()
    msg = bus.timed_pop_filtered(
        Gst.CLOCK_TIME_NONE,
        Gst.MessageType.ERROR | Gst.MessageType.EOS
    )
    # free resources
    pipeline.set_state(Gst.State.NULL)


def video_convert_test(video_file):
    app_sink_name = 'mysink'
    launch_str = ("filesrc location=%s ! qtdemux ! h264parse ! video/x-h264,stream-format=byte-stream,alignment=au ! nvh264dec  ! videoconvert ! video/x-raw,format=YUY2,pixel-aspect-ratio=1/1  ! appsink emit-signals=True name=%s") % (video_file, app_sink_name)
    print(launch_str)

    Gst.init()
    pipeline = Gst.parse_launch(launch_str)
    appsink = pipeline.get_by_name(app_sink_name)
    # subscribe to <new-sample> signal
    appsink.connect("new-sample", on_buffer, None)

    # Gst.debug_set_active(True)
    # level = Gst.debug_get_default_threshold()
    # # if level < Gst.DebugLevel.ERROR:
    # Gst.debug_set_default_threshold(Gst.DebugLevel.ERROR)
    # # print(level)
    # Gst.debug_add_log_function(on_debug, None)
    # Gst.debug_remove_log_function(Gst.debug_log_default)

    pipeline.set_state(Gst.State.PLAYING)
    # wait until EOS or error
    bus = pipeline.get_bus()
    msg = bus.timed_pop_filtered(
        Gst.CLOCK_TIME_NONE,
        Gst.MessageType.ERROR | Gst.MessageType.EOS
    )
    # free resources
    pipeline.set_state(Gst.State.NULL)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Video Convert by Gst"
    )
    parser.add_argument('--file',
                        type=str,
                        help="video file input (h264.avi format)",
                        required=False
    )
    parser.add_argument('--method',
                        type=str,
                        help="convert | play | avimux | decode | test)",
                        required=True
    )
    parser.add_argument('--width',
                        type=int,
                        help="video resolution (width))",
                        required=False
    )   
    parser.add_argument('--height',
                        type=int,
                        help="video resolution (height))",
                        required=False
    )   

    args = parser.parse_args()
    video_file = args.file
    method = args.method
    width = args.width    
    height = args.height

    if method == 'convert':
        video_convert_avi2mp4(video_file)
    elif method == 'play':
        video_play_mp4_h264(video_file)
    elif method == 'avimux':
        video_avimux_h264(video_file)
    elif method == 'decode':
        video_decode_h264(video_file)
    elif method == 'test':
        video_convert_test(video_file)
    else:
        print("Wrong input method!")
