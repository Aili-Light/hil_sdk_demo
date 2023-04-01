import time
import algSDKpy
from algSDKpy import service_stream_control
from algSDKpy import ALG_SDK_MAX_CHANNEL

cmd_topic = b"/service/camera/stream_on"
timeo = 5000

if __name__ == '__main__':
    cam_ctl = service_stream_control()
    cam_ctl.ack_mode = 1
    T1 = time.time()
    for i in range(0, ALG_SDK_MAX_CHANNEL):
        cam_ctl.select[i] = 0
        cam_ctl.control[i] = 0
        
    print('Stream ON')
    for i in range(0, 4):
        cam_ctl.select[2*i] = 1
        cam_ctl.control[2*i] = 1
    ret = algSDKpy.CallServices(cmd_topic, cam_ctl, timeo)
    print(' result = %d ' % ret)
    st = 1
    
    while True:
        T2 =time.time()
        if (T2-T1) > 10 and st==0:
            print('Stream ON')
            for i in range(0, 4):
                cam_ctl.select[2*i] = 1
                cam_ctl.control[2*i] = 1
            ret = algSDKpy.CallServices(cmd_topic, cam_ctl, timeo)
            print(' result = %d ' % ret)
            st = 1
            T1 = T2
            
        if (T2-T1) > 60 and st==1:
            print('Stream OFF')
            for i in range(0, 4):
                cam_ctl.select[2*i] = 1
                cam_ctl.control[2*i] = 0
            ret = algSDKpy.CallServices(cmd_topic, cam_ctl, timeo)
            print(' result = %d ' % ret)
            st = 0
            T1 = T2

    print('---------finish-------------')
