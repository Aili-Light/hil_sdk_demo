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
a. init sdk (C++)  
   `cd <install path>`  
   `sudo ./hil_sdk_demo_init -s`   

b. init sdk (python)  
   `cd <src/python>`  
   `sudo python3 init_sdk.py`  

Stream On/Off
------------------------------------
   `cd <src/python>`  
   Stream on
   `python stream_on_by_channel.py --channel='x,y'`  
   
   Stream off
   `python stream_off.py`  

Push single image
------------------------------------
a. push 1 channel (c++)  
   `cd <install path>`  
   `sudo ./hil_sdk_demo_push_1_ch --publish <image_path> <width> <height> <channel_id> <data_type>`  
   note :   
   #1. `<image_path>`   is the path to image file    
   #2. `<width>`  is image width  
   #3. `<height>` is image height  
   #4. `<channel_id>` is the channel index to push image to   
   #5. `<data_type>` is data_type (YUYV/RAW)   

b. push 1 channel (python)  
   `cd <src/python>`  
   `sudo python3 image_feed.py --image_path=<image_path>' --width=<width> --height=<height> --channel=<channel_id>`  

c. push multiple channels
   `sudo ./hil_sdk_demo_push_multi_ch --publish_multi <num_channels> <image_path_#1> <width_#1> <height_#1> <channel_id_#1> <data_type_#1> <image_path_#2> <width_#2> <height_#2> <channel_id_#2> <data_type_#2> `  
   note :   
   #1. `<num_channels>`   is totol number of channels  
   #2. `<image_path_#1>` `<image_path_#2>`  are image files for each channel  

Feed consecutive images
------------------------------------
a. push 1 channel  
   `cd <install path>`  
   `sudo ./hil_sdk_demo_push_1_ch --feedon <folder> <width> <height> <channel_id> <data_type>`  
   note :   
   #1. `<folder>` is the folder to consecutive image files   
   #2. `<width>`  is image width  
   #3. `<height>` is image height  
   #4. `<channel_id>` is the channel index to push image to   
   #5. `<data_type>` is data_type (YUYV/RAW)   

b. push multiple channels
   `sudo ./hil_sdk_demo_push_multi_ch --feedin_multi <num_channels> <folder_#1> <width_#1> <height_#1> <channel_id_#1> <data_type_#1> <folder_#2> <width_#2> <height_#2> <channel_id_#2> <data_type_#2> ` 
   note :   
   #1. `<num_channels>`   is totol number of channels  
   #2. `<folder_#1>` `<folder_#2>`  are image folders for each channel  

# Support
contact : jimmy@ailiteam.com

# Copyright
2020-2023 Shenzhen Aili-Light Co.,Ltd  