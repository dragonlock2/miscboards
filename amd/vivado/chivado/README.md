# chivado

Deploy Chisel on Xilinx FPGAs using Docker and Vivado. Intended for use on MacOS since Vivado isn't supported without a VM.

## Setup

First we need to build the Docker image with Vivado.

1. Download the [Xilinx Unified Installer SFD](https://www.xilinx.com/support/download.html) from Xilinx's website and place it in the repo root. Modify the `INSTALLER` argument and `echo` line in `Dockerfile` to match its name and version.
1. Increase the disk image size limit in Docker to >350GB. The image should end up taking ~100GB.
1. Enable the experimental features in the Docker daemon for `--squash` support.
1. Run `make setup`.
1. Optionally run `docker system prune` to reclaim all that space.
1. Optionally delete the Xilinx Unified Installer file to save space.

Next we need to install local dependencies.

1. Run `brew install sbt openfpgaloader`.
1. If you need QSPI flash programming, change `LOADER_FOLDER` in `Makefile` and [qspi.xdc](src/main/script/qspi.xdc) to match your setup and run `make synth_qspi`.
1. For GUI usage or adding IP, we recommend installing [XQuartz](https://www.xquartz.org).

## Run

Change `PART_NUM`, `NUM_CPU`, and `TOP_MODULE` in `Makefile` as well as [top.xdc](src/main/script/top.xdc) to match your setup. See [src/main/scala](src/main/scala) for how to add your own Chisel and configure the top level.

### Build

Since Chisel IO names are not always consistent, you may need to modify `TOP_MODULE` and [top.xdc](src/main/script/top.xdc) again. Run `make build` to generate the top level Verilog in `build/` and check.

Running `make synth` compiles the Chisel down to Verilog, copies the top level, XDC, and Tcl scripts over to the Docker container, and runs synthesis and implementation there. The final Vivado project, including the bitstream and timing reports, is copied back to the local `build/` folder.

### Flashing

Since Docker on MacOS doesn't support USB device passthrough yet, we'll use [openFPGALoader](https://github.com/trabucayre/openFPGALoader). Change `CABLE` in `Makefile` to match your programming cable.

For SRAM flashing, run `make flash`. For QSPI flash programming, run `make flash_qspi`.

## Extra

### Adding IP

Based on [Creating Vivado IP the Smart Tcl Way](https://grittyengineer.com/creating-vivado-ip-the-smart-tcl-way/), we recommend generating IP with Vivado's GUI to figure out what Tcl commands to run. This can definitely be improved, but configuring IP should be infrequent.

1. Run `make synth` to generate the Vivado project first.
1. Run `make gui` to launch the Vivado GUI in an XQuartz window.
1. Generate and configure your IP (scripts is enough).
1. Open `File > Project > Open Journal File` and copy the `create_ip` and`set_ip` lines for the IP into [ip.tcl](src/main/script/ip.tcl).
1. Run `File > Exit` to close Vivado gracefully. The updated Vivado project is copied back locally.
1. Add the appropriate [BlackBoxes](https://www.chisel-lang.org/chisel3/docs/explanations/blackboxes.html) in Chisel. The IP Verilog templates should be in the Vivado project. See [test.scala](src/main/scala/test/Test.scala) for an example.
