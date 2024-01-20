import os
import platform
import signal
import ctypes
from ctypes import *

ALG_SDK_NOTIFY_MSG_SIZE = 1024

processor_name = platform.processor()
device_handler = ctypes.CDLL('../../hil_sdk/lib/linux/'+processor_name+'/libhil_sdk_device.so', mode=ctypes.RTLD_GLOBAL)
# device_handler_qcap = ctypes.CDLL('../../hil_sdk/lib/linux/'+processor_name+'/libhil_sdk_device_qcap.so', mode=ctypes.RTLD_GLOBAL)
callbackFunc_t = ctypes.CFUNCTYPE(c_void_p, c_void_p)

class pcie_common_head_t(Structure):
            _fields_ = [("head",c_uint8),
    ("version", c_uint8),
    ("topic_name",c_char*128),
    ("crc8", c_uint8),
    ("resv", c_uint8*125)
    ]

class hil_mesg_meta_t(Structure):
        _fields_ = [("ch_id",c_uint8),
    ("frame_index",c_uint32),
    ("timestamp",c_uint64),
    ("buffer_count",c_uint32),
    ("buffer_len",c_uint32),
    ("msg_len",c_uint32),
    ("msg",c_uint8*ALG_SDK_NOTIFY_MSG_SIZE),
    ]

class hil_mesg_t(Structure):
        _fields_ = [("common_head",pcie_common_head_t),
    ("msg_meta",hil_mesg_meta_t),
    ]

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

        device_handler.HILDevFile_SetCallbackFunc.argtypes = [ctypes.c_void_p, callbackFunc_t]
        device_handler.HILDevFile_SetCallbackFunc.restype = ctypes.c_void_p
        
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
        
    def SetCallbackFunc(self, callback_func):
        device_handler.HILDevFile_SetCallbackFunc(self.obj, callback_func)

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

        device_handler.HILDevDir_SetCallbackFunc.argtypes = [ctypes.c_void_p, callbackFunc_t]
        device_handler.HILDevDir_SetCallbackFunc.restype = ctypes.c_void_p
        
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

    def SetCallbackFunc(self, callback_func):
        device_handler.HILDevDir_SetCallbackFunc(self.obj, callback_func)

# only with FFMPEG
device_handler_decode = ctypes.CDLL('../../hil_sdk/lib/linux/'+processor_name+'/libhil_sdk_device_decode.so', mode=ctypes.RTLD_GLOBAL)
class HILDeviceFromVideo(object):
    def __init__(self):
        # Declare input and output types for each method you intend to use
        device_handler_decode.HILDevVideo_GetInstance.argtypes = []
        device_handler_decode.HILDevVideo_GetInstance.restype = ctypes.c_void_p

        device_handler_decode.HILDevVideo_RegisterDevice.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
        device_handler_decode.HILDevVideo_RegisterDevice.restype = ctypes.c_void_p

        device_handler_decode.HILDevVideo_Init.argtypes = [ctypes.c_void_p]
        device_handler_decode.HILDevVideo_Init.restype = ctypes.c_bool

        device_handler_decode.HILDevVideo_StartStreamAll.argtypes = [ctypes.c_void_p]
        device_handler_decode.HILDevVideo_StartStreamAll.restype = ctypes.c_void_p

        device_handler_decode.HILDevVideo_CloseStreamAll.argtypes = [ctypes.c_void_p]
        device_handler_decode.HILDevVideo_CloseStreamAll.restype = ctypes.c_void_p

        device_handler_decode.HILDevVideo_Wait.argtypes = [ctypes.c_void_p]
        device_handler_decode.HILDevVideo_Wait.restype = ctypes.c_void_p

        device_handler_decode.HILDevVideo_SetCallbackFunc.argtypes = [ctypes.c_void_p, callbackFunc_t]
        device_handler_decode.HILDevVideo_SetCallbackFunc.restype = ctypes.c_void_p
        
        self.obj = device_handler_decode.HILDevVideo_GetInstance()

    def Init(self):
        return device_handler_decode.HILDevVideo_Init(self.obj)

    def RegisterDevice(self, param):
        device_handler_decode.HILDevVideo_RegisterDevice(self.obj, ctypes.pointer(param))
    
    def StartStreamAll(self):
        device_handler_decode.HILDevVideo_StartStreamAll(self.obj)

    def CloseStreamAll(self):
        device_handler_decode.HILDevVideo_CloseStreamAll(self.obj)

    def Wait(self):
        device_handler_decode.HILDevVideo_Wait(self.obj)
        
    def SetCallbackFunc(self, callback_func):
        device_handler_decode.HILDevVideo_SetCallbackFunc(self.obj, callback_func)

# only with OPENCV
device_handler_image = ctypes.CDLL('../../hil_sdk/lib/linux/'+processor_name+'/libhil_sdk_device_image.so', mode=ctypes.RTLD_GLOBAL)
class HILDeviceFromImage(object):
    def __init__(self):
        # Declare input and output types for each method you intend to use
        device_handler_image.HILDevImage_GetInstance.argtypes = []
        device_handler_image.HILDevImage_GetInstance.restype = ctypes.c_void_p

        device_handler_image.HILDevImage_RegisterDevice.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
        device_handler_image.HILDevImage_RegisterDevice.restype = ctypes.c_void_p

        device_handler_image.HILDevImage_Init.argtypes = [ctypes.c_void_p]
        device_handler_image.HILDevImage_Init.restype = ctypes.c_bool

        device_handler_image.HILDevImage_StartStreamAll.argtypes = [ctypes.c_void_p]
        device_handler_image.HILDevImage_StartStreamAll.restype = ctypes.c_void_p

        device_handler_image.HILDevImage_CloseStreamAll.argtypes = [ctypes.c_void_p]
        device_handler_image.HILDevImage_CloseStreamAll.restype = ctypes.c_void_p

        device_handler_image.HILDevImage_Wait.argtypes = [ctypes.c_void_p]
        device_handler_image.HILDevImage_Wait.restype = ctypes.c_void_p

        device_handler_image.HILDevImage_SetCallbackFunc.argtypes = [ctypes.c_void_p, callbackFunc_t]
        device_handler_image.HILDevImage_SetCallbackFunc.restype = ctypes.c_void_p
        
        self.obj = device_handler_image.HILDevImage_GetInstance()

    def Init(self):
        return device_handler_image.HILDevImage_Init(self.obj)

    def RegisterDevice(self, param):
        device_handler_image.HILDevImage_RegisterDevice(self.obj, ctypes.pointer(param))
    
    def StartStreamAll(self):
        device_handler_image.HILDevImage_StartStreamAll(self.obj)

    def CloseStreamAll(self):
        device_handler_image.HILDevImage_CloseStreamAll(self.obj)

    def Wait(self):
        device_handler_image.HILDevImage_Wait(self.obj)
        
    def SetCallbackFunc(self, callback_func):
        device_handler_image.HILDevImage_SetCallbackFunc(self.obj, callback_func)
