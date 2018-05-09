## 开发环境
* Ubuntu16.04

* QT5.8.0

* OpenCV3.4.1

#### QT5.8.0下载与安装
- 下载：
进入[下载页面](http://download.qt.io/archive/qt/5.8/5.8.0/)，选择 qt-opensource-linux-x64-5.8.0.run

- 安装：
```
sudo ./qt-opensource-linux-x64-5.8.0.run
```

#### OpenCV3.4.1编译
- 1、安装gcc cmake
```
sudo apt-get install build-essential pkg-config cmake cmake-gui
```
- 2、安装依赖
```
sudo apt-get install git libgtk2.0-dev libavcodec-dev libavformat-dev libswscale-dev python-dev python-numpy
sudo apt-get install libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev # 处理图像所需的包
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev liblapacke-dev
sudo apt-get install libxvidcore-dev libx264-dev # 处理视频所需的包
sudo apt-get install libatlas-base-dev gfortran # 优化opencv功能
sudo apt-get install ffmpeg
```
- 3、下载OpenCV3.4.1
```
wget https://github.com/opencv/opencv/archive/3.4.1.zip 
wget https://github.com/opencv/opencv_contrib/archive/3.4.1.zip
```
- 4、编译
```
cd opencv-3.4.1
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local/opencv3.4.1 -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-3.4.1/modules ..
make
sudo make install
sudo sh -c 'echo "/usr/local/opencv3.4.1/lib" >> /etc/ld.so.conf.d/opencv.conf'
sudo ldconfig
```
其中opencv_contrib在编译的时候有些module在编译的时候需要下载的文件无法下载（可能是链接失效），可以在modules文件夹内删除不需要的module。
