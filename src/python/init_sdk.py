import signal, sys
import time
import ctypes

import algSDKpy
from algSDKpy import algSDKInit
from algSDKpy import algSDKNotify
from algSDKpy import notifyFunc_t

sdkHandler = algSDKInit()

def int_handler(signum, frame):
    print('---------SDK Stop-------------')
    sdkHandler.Stop()
    sys.exit(0)

def nofity_func(ptr):
    msg_ptr = ctypes.cast(ptr, ctypes.c_char_p)
    msg = bytes(msg_ptr.value)
    msg = msg.decode("utf-8")
    print(msg)

if __name__ == '__main__':
    signal.signal(signal.SIGINT, int_handler)

    frq = 1000
    ret = sdkHandler.InitSDK()
    if(ret < 0):
        print(' Init SDK Failed! result = %d ' % ret)
        sys.exit(0)
 
    sdkHandler.Spin()

    while(1):
        time.sleep(0.001)

    print('---------finish-------------')
