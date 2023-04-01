import argparse
import algSDKpy
from algSDKpy import service_stream_control
from algSDKpy import ALG_SDK_MAX_CHANNEL

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Set sensor stream on"
    )
    parser.add_argument('--channel',
                        type=str,
                        help="channel id to stream on, seperated with coma (e.g. --channel=0,2,8,10)",
                        required=True
    )
    parser.add_argument('-time_out',
                        type=int,
                        help="Timeout value for request (in milliseconds)",
                        default=5000
    )
    args = parser.parse_args()

    channel = args.channel
    timeo = args.time_out
    ch_split = channel.split(",")

    cmd_topic = b"/service/camera/stream_on"
    cam_ctl = service_stream_control()
    cam_ctl.ack_mode = 1

    for item in ch_split:
        ch_id = int(item)
        if(ch_id < ALG_SDK_MAX_CHANNEL):
            print("stream on channel [%d]" % ch_id)
            cam_ctl.select[ch_id] = 1
            cam_ctl.control[ch_id] = 1

    ret = algSDKpy.CallServices(cmd_topic, cam_ctl, timeo)
    print(' result = %d ' % ret)

    print('---------finish-------------')
