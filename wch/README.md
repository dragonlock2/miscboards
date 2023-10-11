# wch

These projects are tested on macOS, although it can definitely be modified to work elsewhere.

## setup

Build Clang/LLVM with RISC-V support. Feel free to place it wherever you like, but `toolchain.cmake` will search in `~/opt` so it may need modification.

```
cd ~/opt
# TODO do and test, remove exception hack?
```

Build OpenOCD with WCH-LinkE support. I've attached the archive sent by MounRiver on `10/11/2023`.

```
cd SDK
tar xvf riscv-openocd.tar.xz
cd riscv-openocd
./bootstrap
CFLAGS=-Wno-error=implicit-function-declaration ./configure --disable-werror --enable-wlinke --disable-ch347
make
```

## build

Follow the standard CMake procedure for each project. There's a few extra targets to ease debugging.

```
mkdir build && cd build && cmake ..
cmake --build .
cmake --build . -t flash
cmake --build . -t server
cmake --build . -t gdb
```
