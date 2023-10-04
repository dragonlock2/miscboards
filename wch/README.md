# wch

These projects are tested on macOS, although it can definitely be modified to work elsewhere.

## setup

Download the latest [toolchain](http://www.mounriver.com/download) and extract OpenOCD and GCC into `SDK/toolchain`. The CMake toolchain is set up to look in `SDK/toolchain/openocd/bin` and `SDK/toolchain/gcc/bin`.

TODO use updated toolchain

## build

Follow the standard CMake procedure for each project. There's a few extra targets to ease debugging.

```
mkdir build && cd build && cmake ..
cmake --build .
cmake --build . -t flash
cmake --build . -t server
cmake --build . -t gdb
```
