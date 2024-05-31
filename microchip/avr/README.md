# microchip/avr

These projects are tested on macOS, although it'll probably work elsewhere. Like `wch`, adding a new project is as simple as including the SDK's toolchain and CMake folder.

The following legacy projects have not been updated to use the SDK.
- lightsaber
- sleepManager

## setup

Only need do this when adding support for a microcontroller that isn't fully supported in `avr-gcc`.

1. Download the correct DFP from [packs.download.atmel.com](http://packs.download.atmel.com). Change the extension to `.zip`, extract it, and copy over the needed files.
