# src directory makefile

FLAGS = -c -Wall -O0 -g --std=c++2a -fdiagnostics-color=always $(INCLUDES)
INCLUDES = -I ../include
LIBS = -lpthread

BUILD_DIR = ../build
BIN_DIR = ../bin

objects = connection.o exceptions.o iomanager.o main.o

all: dir $(objects)
	g++ -g $(BUILD_DIR)/*.o -o $(BIN_DIR)/main $(LIBS)
dir:
	-mkdir $(BUILD_DIR)
	-mkdir $(BIN_DIR)
$(objects): %.o : %.cpp
	g++ $(FLAGS) $< -o $(BUILD_DIR)/$@
