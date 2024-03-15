import signal, sys
import time
import ctypes
from ctypes import *
import argparse
import algSDKpy
from algSDKpy import algSDKInit

sdkHandler = algSDKInit()

def int_handler(signum, frame):
    print('---------SDK Stop-------------')
    sdkHandler.Stop()
    sys.exit(0)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="HIL Init"
    )
    parser.add_argument('--type',
                        type=str,
                        help="init type, e.g. --type=--subscribe",
                        required=True
    )
    args = parser.parse_args()
    init_type = args.type

    signal.signal(signal.SIGINT, int_handler)

    argc = 1
    argv = ctypes.c_char_p(init_type.encode('utf-8'))

    ret = sdkHandler.InitSDK(argc, argv)
    if(ret < 0):
        print(' Init SDK Failed! result = %d ' % ret)
        sys.exit(0)
 
    while(True):
        time.sleep(1/10000.0)

    sdkHandler.Spin()

    print('---------finish-------------')
