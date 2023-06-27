import os
import platform
import signal
import ctypes
from ctypes import *
import numpy as np

ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE = 8192
ALG_SDK_SERVICE_SENSOR_PAYLOAD_SIZE = 7*ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE

ALG_SDK_MAX_CHANNEL = 16
ALG_SDK_MAX_DESERDES = 8
ALG_SDK_CHANNEL_PER_DEV = 8

ALG_SDK_MIPI_DATA_TYPE_UYVY = 0x1C,    # Type UYVY (2-bytes) */
ALG_SDK_MIPI_DATA_TYPE_VYUY = 0x1D,    # Type VYUY (2-bytes) */
ALG_SDK_MIPI_DATA_TYPE_YUYV = 0x1E,    # Type YUYV (2-bytes) */
ALG_SDK_MIPI_DATA_TYPE_YVYU = 0x1F,    # Type YVYU (2-bytes) */
ALG_SDK_MIPI_DATA_TYPE_RAW10 = 0x2B,   # Type RAW10 (1.25-bytes) */
ALG_SDK_MIPI_DATA_TYPE_RAW12 = 0x2C,   # Type RAW12 (1.5-bytes) */

class service_camera_config(Structure):
    _fields_ = [("ack_mode",c_uint8),
    ("ch_id",c_uint8),
    ("module_type", c_uint16),
    ("width", c_uint16),
    ("height", c_uint16),
    ("deser_mode", c_uint8),
    ("camera_num", c_uint8),
    ("data_type", c_uint8),
    ("line_len", c_uint16),
    ("payload", c_uint8*ALG_SDK_SERVICE_SENSOR_PAYLOAD_SIZE),
    ("ack_code", c_uint8),
    ("channel", c_uint8)
    ]

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

class pcie_common_head_t(Structure):
            _fields_ = [("head",c_uint8),
    ("version", c_uint8),
    ("topic_name",c_char*128),
    ("crc8", c_uint8),
    ("resv", c_uint8*125)
    ]

class pcie_image_info_meta_t(Structure):
            _fields_ = [("frame_index",c_uint32),
    ("width", c_uint16),
    ("height",c_uint16),
    ("data_type", c_uint16),
    ("exposure", c_float),
    ("again", c_float),
    ("dgain", c_float),
    ("temp", c_float),
    ("img_size", c_size_t),
    ("timestamp", c_uint64),
    ]

class pcie_image_data_t(Structure):
        _fields_ = [("common_head",pcie_common_head_t),
    ("image_info_meta",pcie_image_info_meta_t),
    ("payload", c_void_p),
    ]

callbackFunc_t = ctypes.CFUNCTYPE(c_void_p, c_void_p)
notifyFunc_t = ctypes.CFUNCTYPE(c_void_p, c_void_p)

if os.name == 'nt' :
    pcie_sdk = ctypes.CDLL('../../hil_sdk/lib/mingw32/libhil_sdk.dll', winmode=0)
elif os.name == 'posix' :
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
            
    def InitSDK(self, frq):
        self.pcie_sdk.alg_sdk_init.argtypes = [c_int]
        self.pcie_sdk.alg_sdk_init.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_init(frq)

        return ret

    def Stop(self):
        print("stop")
        self.pcie_sdk.alg_sdk_stop()

    def Spin(self):
        self.pcie_sdk.alg_sdk_spin_on()

class algSDKServer():
    def __init__(self):
        self.pcie_sdk = pcie_sdk    

    def InitServer(self):
        self.pcie_sdk.alg_sdk_init_server.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_init_server()
    
        return ret

    def Publish(self, msg, ch_id):
        self.pcie_sdk.alg_sdk_push2q.restype = ctypes.c_int
        self.pcie_sdk.alg_sdk_push2q.argtypes = [c_void_p, c_int]

        ptr = cast(pointer(msg), c_void_p)
        ret = self.pcie_sdk.alg_sdk_push2q(ptr, ch_id)
    
    def Spin(self):
        self.pcie_sdk.alg_sdk_server_spin_on.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_server_spin_on()
    
class algSDKClient():
    def __init__(self):
        self.pcie_sdk = pcie_sdk

    def InitClient(self):
        self.pcie_sdk.alg_sdk_init_client.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_init_client()

        return ret

    def Subscribe(self, topic_ptr, callback_func):
        self.pcie_sdk.alg_sdk_subscribe.argtypes = [c_char_p, callbackFunc_t]
        self.pcie_sdk.alg_sdk_subscribe.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_subscribe(topic_ptr, callback_func)

        return ret

    def Spin(self):
        self.pcie_sdk.alg_sdk_client_spin_on.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_client_spin_on()

        return ret

class algSDKNotify():
    def __init__(self):
        self.pcie_sdk = pcie_sdk

    def SetNotify(self, notify_func):
        self.pcie_sdk.alg_sdk_notify.argtypes = [notifyFunc_t]
        self.pcie_sdk.alg_sdk_notify.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_notify(notify_func)

        return ret
    
    def Spin(self):
        self.pcie_sdk.alg_sdk_notify_spin_on()

    def Stop(self):
        self.pcie_sdk.alg_sdk_notify.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_stop_notify()

        return ret

class ImageFeed():
    def __init__(self, h, w, ch, size, type):
        self.img_header = pcie_common_head_t()
        self.img_info = pcie_image_info_meta_t()
        self.img_data = pcie_image_data_t()

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

    def feed_data_raw10(self, payload, frame_index, timestamp):
        p_array = np.frombuffer(np.ctypeslib.as_array(payload, shape=((self.img_size, 1, 1))), dtype=np.uint8)
        p_data = np.zeros(shape=(self.height*self.width, 1, 1), dtype=np.uint16)
        for i in range(0, int(self.height*self.width/4)):
            p_data[4*i] = (((np.ushort(p_array[5*i]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 0) & 0x0003));
            p_data[4*i+1] = (((np.ushort(p_array[5*i+1]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 2) & 0x0003));
            p_data[4*i+2] = (((np.ushort(p_array[5*i+2]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 4) & 0x0003));
            p_data[4*i+3] = (((np.ushort(p_array[5*i+3]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 6) & 0x0003));

        img_ptr = p_data.ctypes.data_as(ctypes.c_char_p)
        # s = img_ptr.value
        self.img_data.image_info_meta.frame_index = frame_index
        self.img_data.image_info_meta.timestamp = timestamp
        self.img_data.payload = ctypes.cast(img_ptr, ctypes.c_void_p)

    def feed_data_raw12(self, payload, frame_index, timestamp):
        p_array = np.frombuffer(np.ctypeslib.as_array(payload, shape=((self.img_size, 1, 1))), dtype=np.uint8)
        p_data = np.zeros(shape=(self.height*self.width, 1, 1), dtype=np.uint16)
        for i in range(0, int(self.height*self.width/2)):
            p_data[2*i] = (((np.ushort(p_array[3*i]) << 4) & 0x0FF0) | np.ushort((p_array[3*i+2] >> 0) & 0x000F));
            p_data[2*i+1] = (((np.ushort(p_array[3*i+1]) << 4) & 0x0FF0) | np.ushort((p_array[3*i+2] >> 4) & 0x000F));

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
        else:
            self.data_type = 0

        return self.data_type