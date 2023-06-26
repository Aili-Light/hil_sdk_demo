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

Stream On/Off
------------------------------------
   `cd <src/python>`  
   Stream on
   `python stream_on_by_channel.py --channel='x,y'`  
   
   Stream off
   `python stream_off.py`  

Push single image
------------------------------------
   `cd <install path>`  
   `sudo ./hil_sdk_demo_push_1_ch --publish <file> <width> <height> <channel_id> <data_type>`  
   note :   
   #1. `<file>`   is the path to image file    
   #2. `<width>`  is image width  
   #3. `<height>` is image height  
   #4. `<channel_id>` is the channel index to push image to   
   #5. `<data_type>` is data_type (YUYV/RAW)   

Feed consecutive images
------------------------------------
   `cd <install path>`  
   `sudo ./hil_sdk_demo_push_1_ch --feedon <folder> <width> <height> <channel_id> <data_type>`  
   note :   
   #1. `<folder>` is the folder to consecutive image files   
   #2. `<width>`  is image width  
   #3. `<height>` is image height  
   #4. `<channel_id>` is the channel index to push image to   
   #5. `<data_type>` is data_type (YUYV/RAW)   

# Support
contact : jimmy@ailiteam.com

# Copyright
2020-2023 Shenzhen Aili-Light Co.,Ltd  