#!/bin/bash
dir=`pwd`
export LD_LIBRARY_PATH=$dir/qcap_lib/lib:$LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH

./release/linux/bin/hil_sdk_demo_device_fromQCap 1 '/home/test/hil_sdk_demo/config/hil_config_fromQCAP.json' 
