PROGRAM_NAME := program
DEBUG_NAME := dbg_program
BUILD_DIR := build
SRC_DIR := src
LINKER_FLAGS := -lglfw -lX11 -lGL -lm
FLAGS := -std=c++17


ONELINER := $(SRC_DIR)/*.cpp $(LINKER_FLAGS) -march=native $(FLAGS)

default:
	g++ -o $(BUILD_DIR)/$(PROGRAM_NAME) $(ONELINER) -Ofast

windows:
	g++ src/*.cpp -o build/win_programm.exe -L "/g/Code/glfw-3.4.bin.WIN64/lib-mingw-w64/" -I "/g/Code/glfw-3.4.bin.WIN64/include/" -lglfw3 -lopengl32 -lgdi32

dbg:
	g++ -o $(BUILD_DIR)/$(DEBUG_NAME) $(ONELINER) -pg

clean:
	rm build/*


.PHONY: dbg default clean windows
