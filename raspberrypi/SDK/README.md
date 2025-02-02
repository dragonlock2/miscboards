# SDK

## OpenOCD

Run the following to build OpenOCD from source.
```
cd openocd
./bootstrap
./configure --disable-werror
make
```

## RISC-V

Download the [latest toolchain](https://github.com/raspberrypi/pico-sdk-tools/releases/tag/v2.0.0-5) and place it under `SDK/riscv-toolchain`.
