PROGRAM_NAME := program
DEBUG_NAME := dbg_program
BUILD_DIR := build
SRC_DIR := src
LINKER_FLAGS := -lglfw -lX11 -lGL -lm

ONELINER := $(SRC_DIR)/*.c $(LINKER_FLAGS) -march=native

default:
	gcc -o $(BUILD_DIR)/$(PROGRAM_NAME) $(ONELINER) -Ofast

dbg:
	gcc -o $(BUILD_DIR)/$(DEBUG_NAME) $(ONELINER) -g
