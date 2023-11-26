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
        device_handler.HILDevFile_GetInstance.argtypes = []
        device_handler.HILDevFile_GetInstance.restype = ctypes.c_void_p

        device_handler.HILDevFile_RegisterDevice.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
        device_handler.HILDevFile_RegisterDevice.restype = ctypes.c_void_p

        device_handler.HILDevFile_Init.argtypes = [ctypes.c_void_p]
        device_handler.HILDevFile_Init.restype = ctypes.c_bool

        device_handler.HILDevFile_StartStreamAll.argtypes = [ctypes.c_void_p]
        device_handler.HILDevFile_StartStreamAll.restype = ctypes.c_void_p

        device_handler.HILDevFile_CloseStreamAll.argtypes = [ctypes.c_void_p]
        device_handler.HILDevFile_CloseStreamAll.restype = ctypes.c_void_p

        device_handler.HILDevFile_Wait.argtypes = [ctypes.c_void_p]
        device_handler.HILDevFile_Wait.restype = ctypes.c_void_p

        self.obj = device_handler.HILDevFile_GetInstance()

    def Init(self):
        return device_handler.HILDevFile_Init(self.obj)

    def RegisterDevice(self, param):
        device_handler.HILDevFile_RegisterDevice(self.obj, ctypes.pointer(param))
    
    def StartStreamAll(self):
        device_handler.HILDevFile_StartStreamAll(self.obj)

    def CloseStreamAll(self):
        device_handler.HILDevFile_CloseStreamAll(self.obj)

    def Wait(self):
        device_handler.HILDevFile_Wait(self.obj)


class HILDeviceFromDir(object):
    def __init__(self):
        # Declare input and output types for each method you intend to use
        device_handler.HILDevDir_GetInstance.argtypes = []
        device_handler.HILDevDir_GetInstance.restype = ctypes.c_void_p

        device_handler.HILDevDir_RegisterDevice.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
        device_handler.HILDevDir_RegisterDevice.restype = ctypes.c_void_p

        device_handler.HILDevDir_Init.argtypes = [ctypes.c_void_p]
        device_handler.HILDevDir_Init.restype = ctypes.c_bool

        device_handler.HILDevDir_StartStreamAll.argtypes = [ctypes.c_void_p]
        device_handler.HILDevDir_StartStreamAll.restype = ctypes.c_void_p

        device_handler.HILDevDir_CloseStreamAll.argtypes = [ctypes.c_void_p]
        device_handler.HILDevDir_CloseStreamAll.restype = ctypes.c_void_p

        device_handler.HILDevDir_Wait.argtypes = [ctypes.c_void_p]
        device_handler.HILDevDir_Wait.restype = ctypes.c_void_p

        self.obj = device_handler.HILDevDir_GetInstance()

    def Init(self):
        return device_handler.HILDevDir_Init(self.obj)

    def RegisterDevice(self, param):
        device_handler.HILDevDir_RegisterDevice(self.obj, ctypes.pointer(param))
    
    def StartStreamAll(self):
        device_handler.HILDevDir_StartStreamAll(self.obj)

    def CloseStreamAll(self):
        device_handler.HILDevDir_CloseStreamAll(self.obj)

    def Wait(self):
        device_handler.HILDevDir_Wait(self.obj)