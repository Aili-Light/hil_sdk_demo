import os
import platform
import signal
import ctypes
from ctypes import *

ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE = 8192
ALG_SDK_SERVICE_SENSOR_PAYLOAD_SIZE = 7*ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE

ALG_SDK_MAX_CHANNEL = 12
ALG_SDK_MAX_DESERDES = 12
ALG_SDK_CHANNEL_PER_DEV = 4

class service_utc_time(Structure):
    _fields_ = [("year_month",c_uint16),
    ("day_wkday", c_uint8),
    ("hour", c_uint8),
    ("minute", c_uint8),
    ("second", c_uint8),
    ("us", c_uint32)
    ]

class service_stream_control(Structure):
    _fields_ = [("ack_mode",c_uint8),
    ("select",c_uint8*ALG_SDK_MAX_CHANNEL),
    ("control", c_uint8*ALG_SDK_MAX_CHANNEL),
    ("ack_code", c_uint8),
    ("ch_sel", c_uint8*ALG_SDK_MAX_CHANNEL)
    ]

class service_set_time(Structure):
    _fields_ = [("ack_mode",c_uint8),
    ("dev_index",c_uint8),
    ("time_mode",c_uint8),
    ("unix_time", c_uint64),
    ("relative_time", c_uint64),
    ("utc_time", service_utc_time),
    ("ack_code", c_uint8)
    ]

class service_trigger_slvcmd(Structure):
    _fields_ = [("trigger_delay_time_us",c_uint32),
    ("trigger_valid_time_us",c_uint32),
    ("trigger_polarity",c_uint8)
    ]

class service_set_trigger(Structure):
    _fields_ = [("ack_mode",c_uint8),
    ("select",c_uint8*ALG_SDK_MAX_CHANNEL),
    ("set_mode",c_uint8),
    ("trigger_mode",c_uint8),
    ("master_trigger_freq", c_uint32),
    ("control_param", service_trigger_slvcmd),
    ("ack_code", c_uint8)
    ]

class service_set_channel_active(Structure):
    _fields_ = [("ack_mode",c_uint8),
    ("dev_index",c_uint8),
    ("channel",c_int32),
    ("active_status",c_uint8),
    ("ack_code", c_uint8)
    ]

callbackFunc_t = ctypes.CFUNCTYPE(c_void_p, c_void_p)
notifyFunc_t = ctypes.CFUNCTYPE(c_void_p, c_void_p)

processor_name = platform.processor()
pcie_sdk = ctypes.CDLL('../../hil_sdk/lib/linux/'+processor_name+'/libhil_sdk.so')

def CallServices(topic_ptr, cfg, timeo):
    pcie_sdk.alg_sdk_call_service.argtypes = [c_char_p, c_void_p, c_int]
    pcie_sdk.alg_sdk_call_service.restype = ctypes.c_int

    ret = pcie_sdk.alg_sdk_call_service(c_char_p(topic_ptr), pointer(cfg), timeo)

    return ret

def crc_array(p, counter):
    pcie_sdk.crc_array.argtypes = [c_char_p, c_ubyte]
    pcie_sdk.crc_array.restype = c_ubyte
    ret = pcie_sdk.crc_array(p, counter)

    return ret

class algSDKInit():
    def __init__(self):
        self.pcie_sdk = pcie_sdk
            
    def InitSDK(self, argc, argv):
        self.pcie_sdk.alg_sdk_init_v2.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_char_p)]
        self.pcie_sdk.alg_sdk_init_v2.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_init_v2(argc, argv)
        return ret

    def Stop(self):
        print("stop")
        self.pcie_sdk.alg_sdk_stop()

    def Spin(self):
        self.pcie_sdk.alg_sdk_spin_on()
