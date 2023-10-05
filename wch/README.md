# wch

These projects are tested on macOS, although it can definitely be modified to work elsewhere.

## setup

Install a RISC-V toolchain (with RV32E support).

```
npm install --location=global xpm@latest
xpm install --global @xpack-dev-tools/riscv-none-elf-gcc@latest --verbose
```

Download the latest [toolchain](http://www.mounriver.com/download) and extract the contents of `bin/` for OpenOCD into `SDK/openocd`.

## build

Follow the standard CMake procedure for each project. There's a few extra targets to ease debugging.

```
mkdir build && cd build && cmake ..
cmake --build .
cmake --build . -t flash
cmake --build . -t server
cmake --build . -t gdb
```
