FLAGS = -c -Wall -O0 -g --std=c++2a -fdiagnostics-color=always $(DEFINES)
DEFINES = -D ENABLE_COMM_LOGGING -D ENABLE_DEBG_LOGGING
LIBS = -lpthread
BUILD_DIR = build
BIN_DIR = bin

build: dir connection.o exceptions.o client.o iomanager.o logger.o main.o
dir:
	-mkdir $(BIN_DIR)
	-mkdir $(BUILD_DIR)
connection.o:
	g++ $(FLAGS) src/connection.cpp -o $(BUILD_DIR)/connection.o $(LIBS)
exceptions.o:
	g++ $(FLAGS) src/exceptions.cpp -o $(BUILD_DIR)/exceptions.o $(LIBS)
client.o:
	g++ $(FLAGS) src/client.cpp -o $(BUILD_DIR)/client.o $(LIBS)
iomanager.o:
	g++ $(FLAGS) src/iomanager.cpp -o $(BUILD_DIR)/iomanager.o $(LIBS)
logger.o:
	g++ $(FLAGS) src/logger.cpp -o $(BUILD_DIR)/logger.o $(LIBS)
main.o:
	g++ $(FLAGS) src/main.cpp -o $(BUILD_DIR)/main.o $(LIBS)

clean:
	-rm -r $(BUILD_DIR) $(BIN_DIR)
