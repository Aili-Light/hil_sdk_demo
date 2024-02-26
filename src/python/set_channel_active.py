import argparse
import algSDKpy
from algSDKpy import service_set_channel_active
from algSDKpy import ALG_SDK_MAX_CHANNEL
from algSDKpy import ALG_SDK_CHANNEL_PER_DEV

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Set sensor active"
    )
    parser.add_argument('--channel',
                        type=str,
                        help="channel id to stream on, seperated with coma (e.g. --channel=0,1,2,3)",
                        required=True
    )
    parser.add_argument('--active',
                        type=int,
                        help="set channel active status 0:disable 1:enable",
                        default=1
    )
    parser.add_argument('-time_out',
                        type=int,
                        help="Timeout value for request (in milliseconds)",
                        default=5000
    )
    args = parser.parse_args()

    channel = args.channel
    timeo = args.time_out
    active_status = args.active
    ch_split = channel.split(",")

    cmd_topic = b"/service/camera/active"
    cam_ctl = service_set_channel_active()
    cam_ctl.ack_mode = 1

    for item in ch_split:
        ch_id = int(item)
        if(ch_id < ALG_SDK_MAX_CHANNEL):
            print("active channel [%d]" % ch_id)
            cam_ctl.channel = ch_id % ALG_SDK_CHANNEL_PER_DEV
            cam_ctl.dev_index = ch_id / ALG_SDK_CHANNEL_PER_DEV
            cam_ctl.active_status = active_status
            ret = algSDKpy.CallServices(cmd_topic, cam_ctl, timeo)
            print(' result = %d ' % ret)
   

    print('---------finish-------------')
