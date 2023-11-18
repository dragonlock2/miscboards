PART_NUM := xc7s15csga225-1
NUM_CPU := 6
TOP_MODULE := Test

CABLE := ft4232
LOADER_FOLDER := /usr/local/Cellar/openfpgaloader/0.8.0/share/openFPGALoader

IMAGE_NAME := vivado
CONTAINER_NAME := vivado-run
CONTAINER_ROOT := $(CONTAINER_NAME):/root
PROJ_NAME := $(TOP_MODULE)
TOP_FILE := $(TOP_MODULE).v
XDC_FILE := top.xdc
SCRIPTS := src/main/script
BUILD_DIR := build
RUN_CMD := docker exec -it $(CONTAINER_NAME) bash -i -c
BIT_FILE := $(BUILD_DIR)/$(PROJ_NAME)/$(PROJ_NAME).runs/impl_1/$(PROJ_NAME).bit

.PHONY: build synth synth_qspi flash flash_qspi gui setup clean

build:
	sbt 'runMain Top -td $(BUILD_DIR)/verilog --target:fpga'

synth: build
	docker run --rm -dit --net=host --name $(CONTAINER_NAME) $(IMAGE_NAME)
	docker cp $(BUILD_DIR)/verilog $(CONTAINER_ROOT)/verilog
	docker cp $(SCRIPTS)/$(XDC_FILE) $(CONTAINER_ROOT)
	docker cp $(SCRIPTS)/build-project.tcl $(CONTAINER_ROOT)
	docker cp $(SCRIPTS)/ip.tcl $(CONTAINER_ROOT)
	$(RUN_CMD) 'vivado -mode batch -source build-project.tcl -tclargs $(PART_NUM) $(PROJ_NAME) $(TOP_MODULE) $(TOP_FILE) $(XDC_FILE) $(NUM_CPU)'
	docker cp $(CONTAINER_ROOT)/$(PROJ_NAME) $(BUILD_DIR)
	docker container kill $(CONTAINER_NAME)

synth_qspi:
	mkdir -p $(BUILD_DIR)
	docker run --rm -dit --net=host --name $(CONTAINER_NAME) $(IMAGE_NAME)
	$(RUN_CMD) 'mkdir verilog'
	docker cp $(SCRIPTS)/qspi.v $(CONTAINER_ROOT)/verilog/qspi.v
	docker cp $(SCRIPTS)/qspi.xdc $(CONTAINER_ROOT)
	docker cp $(SCRIPTS)/build-project.tcl $(CONTAINER_ROOT)
	$(RUN_CMD) 'touch ip.tcl'
	$(RUN_CMD) 'vivado -mode batch -source build-project.tcl -tclargs $(PART_NUM) qspi qspi qspi.v qspi.xdc $(NUM_CPU)'
	docker cp $(CONTAINER_ROOT)/qspi $(BUILD_DIR)
	cp $(BUILD_DIR)/qspi/qspi.runs/impl_1/qspi.bit $(LOADER_FOLDER)/spiOverJtag_$(PART_NUM).bit
	docker container kill $(CONTAINER_NAME)

flash:
	openfpgaloader -c $(CABLE) $(BIT_FILE)

flash_qspi:
	openfpgaloader --fpga-part $(PART_NUM) -c $(CABLE) -f $(BIT_FILE)

gui:
	XQuartz :0 -listen tcp &
	xhost + localhost &
	docker run --rm -dit --net=host -e DISPLAY=host.docker.internal:0 --name $(CONTAINER_NAME) $(IMAGE_NAME)
	docker cp $(BUILD_DIR)/$(PROJ_NAME) $(CONTAINER_ROOT)
	$(RUN_CMD) 'vivado $(PROJ_NAME)/$(PROJ_NAME).xpr'
	docker cp $(CONTAINER_ROOT)/$(PROJ_NAME) $(BUILD_DIR)
	docker container kill $(CONTAINER_NAME)

setup:
	docker build --squash -t $(IMAGE_NAME) .

clean:
	rm -rf $(BUILD_DIR)
	rm $(LOADER_FOLDER)/spiOverJtag_$(PART_NUM).bit.gz
