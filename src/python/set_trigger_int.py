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
        description="Set Trigger Parameters"
    )
    parser.add_argument('--channel',
                        type=str,
                        help="channel id to stream on, seperated with coma (e.g. --channel=0,2,8,10)",
                        required=True
    )
    parser.add_argument('--mode',
                        type=int,
                        help="trigger mode (0=no trigger, 1=External Trigger, 2=Internal Trigger)",
                        required=True
    )
    parser.add_argument('--delay_time',
                        type=int,
                        help="trigger delay time (in microsecond)",
                        default=0
    )
    parser.add_argument('--valid_time',
                        type=int,
                        help="trigger valid time (in microsecond)",
                        default=1000
    )
    parser.add_argument('--polarity',
                        type=int,
                        help="trigger polarity (0=positive, 1=negative))",
                        default=0
    )
    parser.add_argument('--freq',
                        type=int,
                        help="trigger frequency (in Hz, only work for master trigger mode))",
                        default=30
    )
    parser.add_argument('-time_out',
                        type=int,
                        help="Timeout value for request (in milliseconds)",
                        default=5000
    )

    args = parser.parse_args()
    trigger_mode = args.mode
    delay_time = args.delay_time
    valid_time = args.valid_time
    polarity = args.polarity
    master_trigger_freq = args.freq
    channel = args.channel
    timeo = args.time_out
    ch_split = channel.split(",")
    
    trg_cfg = service_trigger_slvcmd()
    trg_cfg.trigger_delay_time_us = delay_time
    trg_cfg.trigger_valid_time_us = valid_time
    trg_cfg.trigger_polarity = polarity

    cfg = service_set_trigger()
    cfg.ack_mode = 1
    cfg.set_mode = 0    # 0=set trigger param, 1=set trigger mode 
    cfg.trigger_mode = trigger_mode # 0=No_TRG, 1=Ext_TRG, 2=Int_TRG
    cfg.control_param = trg_cfg
    cfg.master_trigger_freq = master_trigger_freq # master trigger mode frequency

    for i in range(0, ALG_SDK_MAX_CHANNEL):
        cfg.select[i] = 0

    for item in ch_split:
        ch_id = int(item)
        if(ch_id < ALG_SDK_MAX_CHANNEL):
            print("Set Trigger for CH : [%d]" % ch_id)
            cfg.select[ch_id] = 1

    ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
    print(' result = %d, ack code [%d]' % (ret, cfg.ack_code))

    print('---------finish-------------')
