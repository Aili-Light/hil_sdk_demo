import os
import platform
import signal
import ctypes
from ctypes import *

processor_name = platform.processor()
device_handler = ctypes.CDLL('../../hil_sdk/lib/linux/'+processor_name+'/libhil_sdk_device.so', mode=ctypes.RTLD_GLOBAL)

class VideoSourceParam(Structure):
    _fields_ = [("source_id",c_int32),
    ("config_file",c_char*1024)
    ]

class HILDeviceFromFile(object):
    def __init__(self):
        # Declare input and output types for each method you intend to use
        device_handler.GetInstance.argtypes = []
        device_handler.GetInstance.restype = ctypes.c_void_p

        device_handler.RegisterDevice.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
        device_handler.RegisterDevice.restype = ctypes.c_void_p

        device_handler.Init.argtypes = [ctypes.c_void_p]
        device_handler.Init.restype = ctypes.c_bool

        device_handler.StartStreamAll.argtypes = [ctypes.c_void_p]
        device_handler.StartStreamAll.restype = ctypes.c_void_p

        device_handler.CloseStreamAll.argtypes = [ctypes.c_void_p]
        device_handler.CloseStreamAll.restype = ctypes.c_void_p

        device_handler.Wait.argtypes = [ctypes.c_void_p]
        device_handler.Wait.restype = ctypes.c_void_p

        self.obj = device_handler.GetInstance()

    def Init(self):
        return device_handler.Init(self.obj)

    def RegisterDevice(self, param):
        device_handler.RegisterDevice(self.obj, ctypes.pointer(param))
    
    def StartStreamAll(self):
        device_handler.StartStreamAll(self.obj)

    def CloseStreamAll(self):
        device_handler.CloseStreamAll(self.obj)

    def Wait(self):
        device_handler.Wait(self.obj)