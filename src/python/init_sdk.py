import signal, sys
import time
import algSDKpy
from algSDKpy import algSDKInit

sdkHandler = algSDKInit()

def int_handler(signum, frame):
    print('---------SDK Stop-------------')
    sdkHandler.Stop()
    sys.exit(0)

if __name__ == '__main__':
    signal.signal(signal.SIGINT, int_handler)

    frq = 1000
    ret = sdkHandler.InitSDK(frq)
    if(ret < 0):
        print(' result = %d ' % ret)

    while(1):
        time.sleep(0.001)
    
    sdkHandler.Spin()

    print('---------finish-------------')
