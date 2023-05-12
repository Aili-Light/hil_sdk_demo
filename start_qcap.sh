#!/bin/bash
dir=`pwd`
export LD_LIBRARY_PATH=$dir/qcap_lib/lib:$LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH

./release/linux/bin/hil_sdk_demo_push_qcap --qcap 3840 2160 1
