import argparse, sys
import algSDKpy
from algSDKpy import service_trigger_slvcmd
from algSDKpy import service_set_trigger
from algSDKpy import ALG_SDK_CHANNEL_PER_DEV
from algSDKpy import ALG_SDK_MAX_CHANNEL

cmd_topic = b"/service/set_trigger/set"
timeo = 5000

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Set Trigger Mode"
    )
    parser.add_argument('--device',
                        type=int,
                        help="device id to set trigger (e.g. --device=0)",
                        required=True
    )
    parser.add_argument('--mode',
                        type=int,
                        help="trigger mode (0=no trigger, 1=External Trigger, 2=Internal Trigger)",
                        required=True
    )
    parser.add_argument('-time_out',
                        type=int,
                        help="Timeout value for request (in milliseconds)",
                        default=5000
    )
    args = parser.parse_args()
    trigger_mode = args.mode
    dev_id = args.device
    timeo = args.time_out

    cfg = service_set_trigger()
    cfg.ack_mode = 1
    cfg.set_mode = 1    # 0=set trigger param, 1=set trigger mode 
    cfg.trigger_mode = trigger_mode # 0=No_TRG, 1=Ext_TRG, 2=Int_TRG

    for i in range(0, ALG_SDK_MAX_CHANNEL):
        cfg.select[i] = 0

    cfg.select[dev_id*ALG_SDK_CHANNEL_PER_DEV] = 1 # convert device id to channel id

    ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
    print(' result = %d, ack code [%d]' % (ret, cfg.ack_code))

    print('---------finish-------------')
