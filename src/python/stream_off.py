import algSDKpy
from algSDKpy import service_stream_control
from algSDKpy import ALG_SDK_MAX_CHANNEL

cmd_topic = b"/service/camera/stream_on"
timeo = 5000

if __name__ == '__main__':
    cam_ctl = service_stream_control()
    cam_ctl.ack_mode = 1

    for i in range(0, ALG_SDK_MAX_CHANNEL):
        cam_ctl.select[i] = 1
        cam_ctl.control[i] = 0

    ret = algSDKpy.CallServices(cmd_topic, cam_ctl, timeo)
    print(' result = %d ' % ret)

    print('---------finish-------------')
