ALG HIL SDK Demo
====================================  

This is demostration codes for developer on ALG HIL System.

For more information check the [website](https://aili-light.com)

# Prerequisites
1. Linux (Ubuntu)
   * CMake 3.5 or newer
   * gcc version 7.5.0 (Ubuntu 7.5.0-3ubuntu1~18.04)

# Quick Build Instructions
1.  `mkdir build`  
2.  `cd build`  
3.  `cmake -DCMAKE_INSTALL_PREFIX=<install path> ..`  
4.  `make`  
5.  `make install`  

# Usuage
Init SDK
------------------------------------
   `cd <install path>`  
   `sudo ./hil_sdk_demo_init -s`   

Set Sensor Config
------------------------------------
   `cd <src/python>`  

   for all sensor (use json file)  
   `python set_sensor_from_json.py --json_file=<path to json file> --channel=xx`  

Stream On/Off
------------------------------------
   `cd <src/python>`  
   Stream on
   `python stream_on_by_channel.py --channel='x,y'`  
   
   Stream off
   `python stream_off.py`  

Push image
------------------------------------
   `cd <install path>`  
   `sudo ./hil_sdk_demo_push_1_ch --publish <file> <width> <height> <channel_id>`  
   note :   
   #1. `<file>` is the path to image file    
   #2. `<width>` is image width  
   #3. `<height>` is image height  
   #4. `<channel_id>` is the channel index to push image to   

Set Trigger
------------------------------------
   `cd <src/python>`  
   Set Trigger Mode (Int Trigger) :  
   `python set_trigger_mode.py --device=xx --mode=2`  
   
   Set Trigger Mode (Ext Trigger) :  
   `python set_trigger_mode.py --device=xx --mode=1`  
   
   Set Trigger Parameters :  
   `python set_trigger_mode.py --channel=xx,yy --mode=2 --delay_time=0 --valid_time=1000 --polarity=0 --freq=30`  

# Support
contact : jimmy@ailiteam.com

# Copyright
2020-2023 Shenzhen Aili-Light Co.,Ltd  