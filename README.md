## 开发环境
* Ubuntu16.04

* QT5.8.0

* OpenCV3.4.1

* CUDA8.0

#### QT5.8.0下载与安装
- 下载：

进入[下载页面](http://download.qt.io/archive/qt/5.8/5.8.0/)，选择 qt-opensource-linux-x64-5.8.0.run

- 安装：
```
sudo ./qt-opensource-linux-x64-5.8.0.run
```

#### OpenCV3.4.1编译
- 1、安装依赖
```
sudo apt-get install build-essential pkg-config cmake
sudo apt-get install git libgtk2.0-dev libavcodec-dev libavformat-dev libswscale-dev python-dev python-numpy
sudo apt-get install libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev # 处理图像所需的包
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev liblapacke-dev
sudo apt-get install libxvidcore-dev libx264-dev # 处理视频所需的包
sudo apt-get install libatlas-base-dev gfortran # 优化opencv功能
sudo apt-get install ffmpeg
```
- 2、下载OpenCV3.4.1
```
wget https://github.com/opencv/opencv/archive/3.4.1.zip 
wget https://github.com/opencv/opencv_contrib/archive/3.4.1.zip
```
- 3、编译
```
cd opencv-3.4.1
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local/opencv3.4.1 -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-3.4.1/modules -D WITH_CUDA=ON -D WITH_CUBLAS=1 -D CUDA_ARCH_BIN="6.1" -D CUDA_ARCH_PTX="6.1" -D INSTALL_C_EXAMPLES=OFF -D INSTALL_PYTHON_EXAMPLES=ON -D WITH_OPENGL=ON -D WITH_V4L=ON ..

make
sudo make install
sudo sh -c 'echo "/usr/local/opencv3.4.1/lib" >> /etc/ld.so.conf.d/opencv.conf'
sudo ldconfig
```

**注意**
1.CUDA_ARCH_BIN和CUDA_ARCH_PTX这里是指的显卡计算能力，GeForce GTX 1080 Ti的计算能力为6.1，其他型号可以自行在[NVIDIA官网](https://developer.nvidia.com/cuda-gpus)查询对应版本。

2.其中opencv_contrib在编译的时候有些module在编译的时候需要下载的文件无法下载（可能是链接失效）。可以在该[下载页面](https://pan.baidu.com/s/1MSZIrVzl38Xj6rRRzfA0Kg)下载后将opencv-3.4.1/.cache目录下的对应文件进行替换。
