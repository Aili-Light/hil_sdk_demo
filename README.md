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
* init sdk (C++)  
   `cd <install path>`  
   `sudo ./hil_sdk_demo_init -s`   

* init sdk (python)  
   `cd <src/python>`  
   `sudo python3 init_sdk.py`  

Stream On/Off
------------------------------------
   `cd <src/python>`  
*  Stream on
   `python3 stream_on_by_channel.py --channel='x,y'`  
   
*  Stream off
   `python3 stream_off.py`  

Push single image
------------------------------------
*  push an image to one channel (c++)  
   `cd <install path>`  
   `sudo ./hil_sdk_demo_push_1_ch --publish <image_path> <width> <height> <channel_id> <data_type>`  
   note :   
   #1. `<image_path>`   is the path to image file    
   #2. `<width>`  is image width  
   #3. `<height>` is image height  
   #4. `<channel_id>` is the channel index to push image to   
   #5. `<data_type>` is data_type ('YUYV'/'RAW10'/'RAW12')   

*  push an image to one channel (python)  
   `cd <src/python>`  
   `sudo python3 push_image_single.py --image_path=<image_path>' --width=<width> --height=<height> --channel=<channel_id> --data_type=<data_type>`  
   note :   
   #1. `<image_path>`   is the path to image file    
   #2. `<width>`  is image width  
   #3. `<height>` is image height  
   #4. `<channel_id>` is the channel index to push image to   
   #5. `<data_type>` is data_type ('YUYV'/'RAW10'/'RAW12')   

*  push multiple images to multiple channels (c++)  
   `sudo ./hil_sdk_demo_push_multi_ch --publish_multi <num_channels> <image_path_#1> <width_#1> <height_#1> <channel_id_#1> <data_type_#1> <image_path_#2> <width_#2> <height_#2> <channel_id_#2> <data_type_#2> `  
   note :   
   #1. `<num_channels>`   is totol number of channels  
   #2. `<image_path_#1>` `<image_path_#2>`  are image files for each channel  

Push consecutive images
------------------------------------
*  push images in one folder to one channel (c++)    
   `cd <install path>`  
   `sudo ./hil_sdk_demo_push_1_ch --feedon <folder> <width> <height> <channel_id> <data_type>`  
   note :   
   #1. `<folder>` is the folder to consecutive image files   
   #2. `<width>`  is image width  
   #3. `<height>` is image height  
   #4. `<channel_id>` is the channel index to push image to   
   #5. `<data_type>` is data_type (YUYV/RAW)   

*  push images in multiple folders to multiple channels (c++)    
   `sudo ./hil_sdk_demo_push_multi_ch --feedin_multi <num_channels> <folder_#1> <width_#1> <height_#1> <channel_id_#1> <data_type_#1> <folder_#2> <width_#2> <height_#2> <channel_id_#2> <data_type_#2> ` 
   note :   
   #1. `<num_channels>`   is totol number of channels  
   #2. `<folder_#1>` `<folder_#2>`  are image folders for each channel  

Push video
------------------------------------
*  decode a video and push to one channel (python)  
   `cd <src/python>`  
   `sudo python3 push_video_decode.py --video_path=<video_path>' --width=<width> --height=<height> --channel=<channel_id> --codec_type=<codec_type>`
   note :   
   #1. `<video_path>`   is the path to video file    
   #2. `<width>`  is image width  
   #3. `<height>` is image height  
   #4. `<channel_id>` is the channel index to push image to   
   #5. `<codec_type>` is video codec type ('h264'/'h265')   

# Support
contact : jimmy@ailiteam.com

# Copyright
2020-2023 Shenzhen Aili-Light Co.,Ltd  