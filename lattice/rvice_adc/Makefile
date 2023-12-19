PROJECT     := rvice_adc
PROJECT_DIR := .
BUILD_DIR   := $(PROJECT_DIR)/build
TOP_MODULE  := top
ARCH        := u1k
PACKAGE     := sg48
PREPACK     := $(PROJECT_DIR)/scripts/clocks.py
PINS        := $(PROJECT_DIR)/scripts/pins.pcf
SRCS        := $(shell find $(PROJECT_DIR)/src -type f -name '*.v')
FLASHER     := python scripts/flash.py

# yosys incorrectly shows limits for iCE5LP4K instead of iCE5LP1K

include ../boilerplate.mk
