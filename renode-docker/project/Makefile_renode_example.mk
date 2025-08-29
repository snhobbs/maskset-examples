PROJECT = renode-example

BASE?=$(PWD)
BUILD_DIR = $(BASE)/build

CFILES = $(BASE)/source/renode-example.c

DEVICE=stm32f407vgt6
OOCD_FILE = $(BASE)/board/stm32f4discovery.cfg

OPENCM3_DIR?=$(BASE)/libopencm3
OPENCM3_INC=$(OPENCM3_DIR)/include


include $(OPENCM3_DIR)/mk/genlink-config.mk
include $(BASE)/rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
