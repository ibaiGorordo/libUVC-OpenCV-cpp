# libUVC-OpenCV-Cpp
Example script to capture images from USB cameras using the libUVC library and OpenCV in C++.The example runs for 10 seconds and it automatically stops.
**Tested in Ubuntu 20.04.1 LTS**

# Installation
```
git clone https://github.com/ibaiGorordo/libUVC-OpenCV-Cpp.git 
cd libUVC-OpenCV-Cpp
mkdir build
cd build
cmake ..
make
sudo ./sample

```

# Required libraries
* libusb: ```sudo apt install libusb-1.0-0-dev```
