# hazeRemoval
  This is an C++ implementation of the haze removal algorithm in paper "Single Image Haze Removal Using Dark Channel Prior" by Kaiming He.
## Dependencies
* OpenCV 2.4: follow the instructions on http://opencv.org/ or install
  via

        sudo apt-get install libopencv-dev

* CMake,

        sudo apt-get install cmake

## Installation(Tested in Ubuntu 14.04)
1. Clone this repository via

        git clone https://github.com/RichardChe/hazeRemoval.git

2. Build

        mkdir build && cd build
        cmake ..
        make

(NOTE: When your OpenCV installation was not in default directory, you need to set OpenCV_DIR in CMakeLists.txt)
## Run
* In build/ :

        ./hazeRemovalBin ../example_img/river.png

and you can see the original image and the corresponding result.
