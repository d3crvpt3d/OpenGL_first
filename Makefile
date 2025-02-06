PROGRAM_NAME := program
BUILD_DIR := build
SRC_DIR := src
INCLUDE_FLAGS := -Iinclude -I$(GLFW_PATH)/Include
LINKER_FLAGS := -L$(GLFW_PATH)/lib-mingw-w64 -lglfw3 -lgdi32 -lopengl32

ONELINER :=-o $(BUILD_DIR)/$(PROGRAM_NAME) $(SRC_DIR)/*.c $(INCLUDE_FLAGS) $(LINKER_FLAGS) -march=native

default:
	gcc $(ONELINER) -O3

dbg:
	gcc $(ONELINER) -g