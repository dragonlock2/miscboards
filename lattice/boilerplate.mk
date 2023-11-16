# adapted from https://github.com/YosysHQ/nextpnr

RUN_CMD := docker run --rm -v $(PROJECT_DIR):/project yosys

.PHONY: all clean flash
.PRECIOUS: %.asc %.json

all: $(BUILD_DIR)/$(PROJECT).bit

%.bit: %.asc
	$(RUN_CMD) icepack $< $@

%.asc: %.json $(PINS) $(PREPACK)
	$(RUN_CMD) nextpnr-ice40 --$(ARCH) --package $(PACKAGE) --pre-pack $(PREPACK) --json $< --pcf $(PINS) --asc $@

%.json: $(SRCS)
	mkdir -p $(BUILD_DIR)
	$(RUN_CMD) yosys -p 'synth_ice40 -top $(TOP_MODULE) -json $@' $^

clean:
	rm -rf $(BUILD_DIR)

flash: $(BUILD_DIR)/$(PROJECT).bit
	$(FLASHER) $<
