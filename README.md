## Simple USB manager script in C++  
Should be run as root to prevent udev permission issues  
Created for educational purposes  
### Features:
- Viewing usb information
- Generate custom udev scripts
- Monitor kernel & udev events
- Monitor HID input events via evtest
### Prerequisites:
- evtest
- libudev: sudo apt-get install libudev-dev
- A compiler such as g++
### Compile:    
g++ main.cpp -ludev or ./compile.sh  
### Arguments:
| Argument | Description |
| -------- | ----------- |
| -v | Verbose mode |
| -d | Debug mode, print debug variables |