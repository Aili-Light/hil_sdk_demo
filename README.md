ALG HIL SDK Demo
====================================  

This is demostration codes for developer on ALG HIL System.

For more information check the [website](https://aili-light.com)

# Prerequisites
1. Linux (Ubuntu 18.04)
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
   `sudo python3 init_sdk.py --type=--subscribe`  

Setup Python Library Environment
------------------------------------
*  only need for using python interface  
   `export LD_LIBRARY_PATH=<path_to_hil_sdk/lib/linux/x86_64/:$LD_LIBRARY_PATH`  
   note:  
   <path_to_hil_sdk> is the absolute path to the folder of <hil_sdk>  

Stream On/Off
------------------------------------
*  Stream on (c++)
   `cd <install path>`  
   `./hil_sdk_demo_service -stream_on -c x,y`   
   note :   
   #1. `x,y`  are the channel id (user can stream on multiple channels, split channel id by coma)   

*  Stream on (python)
   `cd <src/python>`  
   `python3 stream_on_by_channel.py --channel='x,y'`  
   note :   
   #1. `x,y`  are the channel id (user can stream on multiple channels, split channel id by coma)   

*  Stream off (c++)
   `cd <install path>`  
   `./hil_sdk_demo_service -stream_off`   

*  Stream off (python)
   `cd <src/python>`  
   `python3 stream_off.py`  

Push from File
------------------------------------
*  Push one file continuously (c++)  
   `cd <install path>`  
   `sudo ./hil_sdk_demo_device_fromFile <num_channels> <config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

*  Push one file continuously (python)  
   `cd <src/python>`  
   `sudo python3 demo_device_fromFile.py --num_channels=<num_channels>' --config_file=<config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

*  Push all files of a directory (c++)  
   `cd <install path>`  
   `sudo ./hil_sdk_demo_device_fromDir <num_channels> <config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

*  Push all files of a directory (python) 
   `cd <src/python>`  
   `sudo python3 demo_device_fromDir.py --num_channels=<num_channels>' --config_file=<config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

Push from Image
------------------------------------
*  Rebuild hil_sdk_demo with CMake option: -DWITH_OPENCV=ON  
   `cmake -DWITH_OPENCV=ON -DCMAKE_INSTALL_PREFIX=<install path> ..`  
   `make`  
   `make install`  

*  Push image (c++)  
   `cd <install path>`  
   `sudo ./hil_sdk_demo_device_fromImage <num_channels> <config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

*  Push image (python)  
   `cd <src/python>`  
   `sudo python3 demo_device_fromImage.py --num_channels=<num_channels>' --config_file=<config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

Push from Video
------------------------------------
*  Rebuild hil_sdk_demo with CMake option: -DWITH_FFMPEG=ON  
   `cmake -DWITH_FFMPEG=ON -DCMAKE_INSTALL_PREFIX=<install path> ..`  
   `make`  
   `make install`  

*  Push video (c++)  
   `cd <install path>`  
   `sudo ./hil_sdk_demo_device_fromVideo <num_channels> <config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

*  Push video (python)  
   `cd <src/python>`  
   `sudo python3 demo_device_fromVideo.py --num_channels=<num_channels>' --config_file=<config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

# Support
contact : jimmy@ailiteam.com

# Copyright
2020-2024 Shenzhen Aili-Light Co.,Ltd  