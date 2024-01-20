import sys
import time
import argparse
import algDeviceHandler
import ctypes
from ctypes import *

from algDeviceHandler import HILDeviceFromFile
from algDeviceHandler import VideoSourceParam
from algDeviceHandler import callbackFunc_t
from algDeviceHandler import hil_mesg_t

def CallbackFunc(ptr):
    p = ctypes.cast(ptr, ctypes.POINTER(hil_mesg_t))
    print('ALG HIL-Py CB [time : %d], [ch : %d], [Frame : %d], [Count : %d/%d]'
          % (p.contents.msg_meta.timestamp, 
             p.contents.msg_meta.ch_id,
             p.contents.msg_meta.frame_index,
             p.contents.msg_meta.buffer_count,
             p.contents.msg_meta.buffer_len))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="HIL Demo for single image"
    )
    parser.add_argument('--num_channels',
                        type=int,
                        help="total number of channels",
                        required=True
    )
    parser.add_argument('--config_file',
                        type=str,
                        help="configuration file path",
                        required=True
    )
    args = parser.parse_args()
    num_channels = args.num_channels
    config_file_path = args.config_file

    if (num_channels < 1):
        print("ERROR! Number of Channel is incorrect!")
        sys.exit(1)

    dev = HILDeviceFromFile()

    # Setup Device Parameters
    for i in range(0, num_channels):
        param = VideoSourceParam()
        param.source_id = i
        param.config_file = config_file_path.encode('utf-8')
        dev.RegisterDevice(param)

    # Set Callback Function
    callback_func = callbackFunc_t(CallbackFunc)
    dev.SetCallbackFunc(callback_func)

    # Init Device Handler
    if (dev.Init() is not True):
        print("Init device failed!")
        sys.exit(1)

    # Start Stream
    dev.StartStreamAll()

    # Main Loop
    while(True):
        time.sleep(1/10000.0)

    #  Wait Until Stream Finish
    dev.Wait()

    print('---------finish-------------')