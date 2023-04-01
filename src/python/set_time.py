import argparse, sys
import algSDKpy
from algSDKpy import service_set_time
from algSDKpy import service_utc_time

cmd_topic = b"/service/set_time/set"
timeo = 5000

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Set PCIE board time"
    )
    parser.add_argument('--device',
                        type=int,
                        help="device id to set time (e.g. --device=0)",
                        required=True
    )
    parser.add_argument('--time_mode',
                        type=int,
                        help="set time mode (1=utc_time, 2=unix_time, 4=relative_time)",
                        required=True
    )
    parser.add_argument('--utc_time',
                        type=str,
                        help="utc time, split by ':' (e.g. --utc_time=2022:08:23:15:25:30)"
    )
    parser.add_argument('--unix_time',
                        type=int,
                        help="unix time (e.g. --unix_time=1660972543000000)"
    )
    parser.add_argument('--relative_time',
                        type=int,
                        help="relative time (e.g. --relative_time=0)"
    )
    parser.add_argument('-time_out',
                        type=int,
                        help="Timeout value for request (in milliseconds)",
                        default=5000
    )

    args = parser.parse_args()
    time_mode = args.time_mode
    dev_id = args.device

    cfg = service_set_time()
    cfg.ack_mode = 1
    cfg.time_mode = time_mode
    cfg.dev_index = dev_id

    if time_mode == 1: # utc time
        utc_time_str = args.utc_time
        time_set = utc_time_str.split(":")
        if len(time_set) == 6 :
            year = int(time_set[0])
            month = int(time_set[1])
            day = int(time_set[2])
            wk_day = 0
            hour = int(time_set[3])
            minute = int(time_set[4])
            second = int(time_set[5])

            utc = service_utc_time()
            utc.year_month = (month<<12) | (year & 0x0FFF)
            utc.day_wkday = (wk_day<<5) | (day & 0x1F)
            utc.hour = hour
            utc.minute = minute
            utc.second = second
            utc.us = 0
            cfg.utc_time = utc
        else :
            print("UTC Time Format Error !")
            sys.exit(0)
    elif time_mode == 2: # unix time
        unix_time = args.unix_time
        try:
            cfg.unix_time = int(unix_time)
        except Exception as e:
            print("Set Time Error : %s!" % e)
            sys.exit(0)
    elif time_mode == 4: # relativee time
        relative_time = args.relative_time
        try:
            cfg.relative_time = relative_time
        except Exception as e:
            print("Set Time Error : %s!" % e)
            sys.exit(0)

    ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
    print(' result = %d, ack code [%d]' % (ret, cfg.ack_code))

    print('---------finish-------------')
