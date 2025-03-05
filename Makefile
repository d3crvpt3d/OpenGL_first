PROGRAM_NAME := program
DEBUG_NAME := dbg_program
BUILD_DIR := build
SRC_DIR := src
INCLUDE_FLAGS := -Iinclude -I$(GLFW_PATH)/Include
LINKER_FLAGS := -L$(GLFW_PATH)/lib-mingw-w64 -lglfw3 -lgdi32 -lopengl32

ONELINER := $(SRC_DIR)/*.c $(INCLUDE_FLAGS) $(LINKER_FLAGS) -march=native

default:
	gcc -o $(BUILD_DIR)/$(PROGRAM_NAME) $(ONELINER) -O3

dbg:
	gcc -o $(BUILD_DIR)/$(DEBUG_NAME) $(ONELINER) -g