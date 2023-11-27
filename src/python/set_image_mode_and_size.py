import argparse
import algSDKpy
from algSDKpy import service_set_image_write_param
from algSDKpy import ALG_SDK_MAX_DEVICE

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Set device image right mode and once write size"
    )
    parser.add_argument('--dev',
                        type=str,
                        help="current need set board id max 3 (e.g. --dev=0,1,2)",
                        required=True
    )
    parser.add_argument('--mode',
                        type=int,
                        help="set current dev image write mode 1:total write 2:splite write",
                        default=1
    )
    parser.add_argument('--size',
                        type=int,
                        help="set current dev once write size must 4k align",
                        default=16384
    )
    parser.add_argument('-time_out',
                        type=int,
                        help="Timeout value for request (in milliseconds)",
                        default=5000
    )
    args = parser.parse_args()

    dev = args.dev
    timeo = args.time_out
    write_mode = args.mode
    size = args.size
    ch_split = dev.split(",")

    cmd_topic = b"/service/param/set_image_write"
    cam_ctl = service_set_image_write_param()
    cam_ctl.ack_mode = 1

    for item in ch_split:
        dev_id = int(item)
        if(dev_id < ALG_SDK_MAX_DEVICE):
            print("active dev [%d]" % dev_id)
            cam_ctl.dev_index = dev_id
            cam_ctl.write_mode = write_mode
            cam_ctl.once_write_size = size
            ret = algSDKpy.CallServices(cmd_topic, cam_ctl, timeo)
            print(' result = %d ' % ret)
   

    print('---------finish-------------')
