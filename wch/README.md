# wch

These projects are tested on macOS, although it'll probably work elsewhere.

## setup

Build the [RISC-V GCC toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain). You'll need to modify `toolchain.cmake` as needed. Make sure to run the following in a case sensitive volume.

```
git clone https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain
./configure --prefix=/path/to/miscboards/wch/SDK/riscv-toolchain --disable-gdb --with-cmodel=medany --with-multilib-generator="rv32ec-ilp32e--;rv32imac-ilp32--"
make
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
