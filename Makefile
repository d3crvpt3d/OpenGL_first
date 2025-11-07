PROGRAM_NAME := program
DEBUG_NAME := dbg_program
BUILD_DIR := build
SRC_DIR := src
INCLUDE_DIR := include
LINKER_FLAGS := -lglfw -lX11 -lGL -lm
FLAGS := -std=c++20

ifneq ($(OS), Windows_NT)
OS := $(shell uname -s)
endif

ONELINER := $(SRC_DIR)/*.cpp -march=native $(FLAGS) -I$(INCLUDE_DIR)

ONELINER_LINUX := $(ONELINER) $(LINKER_FLAGS)

default: $(OS)

linux: Linux

win: Windows_NT

Linux:
	g++ -o $(BUILD_DIR)/$(PROGRAM_NAME) $(ONELINER_LINUX) -Ofast 

Windows_NT:
	g++ -o $(BUILD_DIR)/win_programm.exe $(ONELINER) -L "/g/Code/glfw-3.4.bin.WIN64/lib-mingw-w64/" -I "/g/Code/glfw-3.4.bin.WIN64/include/" -lglfw3 -lopengl32 -lgdi32 -Ofast

dbg:
	g++ -o $(BUILD_DIR)/$(DEBUG_NAME) $(ONELINER_LINUX) -Ofast -pg -fsanitize=address

clean:
	rm build/*


.PHONY: dbg default clean windows
