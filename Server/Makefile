FLAGS = -c -Wall -O0 -g --std=c++2a -fdiagnostics-color=always
BUILD_DIR = build
BIN_DIR = bin

build: dir connection.o exceptions.o main.o
	g++ build/*.o -o $(BIN_DIR)/main
dir:
	-mkdir $(BIN_DIR)
	-mkdir $(BUILD_DIR)
connection.o:
	g++ $(FLAGS) src/connection.cpp -o $(BUILD_DIR)/connection.o
exceptions.o:
	g++ $(FLAGS) src/exceptions.cpp -o $(BUILD_DIR)/exceptions.o
main.o:
	g++ $(FLAGS) src/main.cpp -o $(BUILD_DIR)/main.o

clean:
	-rm -r $(BUILD_DIR) $(BIN_DIR)
