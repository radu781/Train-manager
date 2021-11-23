FLAGS = -c -Wall -O0 -g --std=c++2a -fdiagnostics-color=always
LIBS = -lpthread
BUILD_DIR = build
BIN_DIR = bin

build: dir connection.o exceptions.o commands.o iomanager.o main.o
	g++ build/*.o -o $(BIN_DIR)/main $(LIBS)
dir:
	-mkdir $(BIN_DIR)
	-mkdir $(BUILD_DIR)
connection.o:
	g++ $(FLAGS) src/connection.cpp -o $(BUILD_DIR)/connection.o $(LIBS)
exceptions.o:
	g++ $(FLAGS) src/exceptions.cpp -o $(BUILD_DIR)/exceptions.o $(LIBS)
iomanager.o:
	g++ $(FLAGS) src/iomanager.cpp -o $(BUILD_DIR)/iomanager.o $(LIBS)
main.o:
	g++ $(FLAGS) src/main.cpp -o $(BUILD_DIR)/main.o $(LIBS)

clean:
	-rm -r $(BUILD_DIR) $(BIN_DIR)
