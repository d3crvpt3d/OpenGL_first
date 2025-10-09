PROGRAM_NAME := program
DEBUG_NAME := dbg_program
BUILD_DIR := build
SRC_DIR := src
LINKER_FLAGS := -lglfw -lX11 -lGL -lm
FLAGS := -std=c++17


ONELINER := $(SRC_DIR)/*.cpp $(LINKER_FLAGS) -march=native $(FLAGS)

default:
	g++ -o $(BUILD_DIR)/$(PROGRAM_NAME) $(ONELINER) -Ofast

dbg:
	g++ -o $(BUILD_DIR)/$(DEBUG_NAME) $(ONELINER) -pg

clean:
	rm build/*

.PHONY: dbg default clean
