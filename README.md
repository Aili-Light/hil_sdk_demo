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
   `sudo python3 init_sdk.py --type=--subscribe`  

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

Push from image
------------------------------------
*  push one image file continuously (c++)  
   `cd <install path>`  
   `sudo ./hil_sdk_demo_device_fromFile <num_channels> <config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

*  push one image file continuously (python)  
   `cd <src/python>`  
   `sudo python3 demo_device_fromFile.py --num_channels=<num_channels>' --config_file=<config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

*  push all images of a directory (c++)  
   `cd <install path>`  
   `sudo ./hil_sdk_demo_device_fromDir <num_channels> <config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

*  push all images of a directory (python) 
   `cd <src/python>`  
   `sudo python3 demo_device_fromDir.py --num_channels=<num_channels>' --config_file=<config_file>`  
   note :   
   #1. `<num_channels>`  the total number of channels    
   #2. `<config_file>`  configuration file path  

# Support
contact : jimmy@ailiteam.com

# Copyright
2020-2023 Shenzhen Aili-Light Co.,Ltd  