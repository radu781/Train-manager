# server root makefile

BUILD_DIR = build
BIN_DIR = bin
FLAGS = -c -Wall -O0 -g --std=c++2a -fdiagnostics-color=always $(DEFINES) $(INCLUDES)
DEFINES = -D ENABLE_COMM_LOGGING -D ENABLE_DEBG_LOGGING
INCLUDES = -I include

all: pch
	$(MAKE) -C include/commands
	$(MAKE) -C src
pch:
	test -f include/pc.h.gch || g++ $(FLAGS) -x c++-header include/pc.h
clean:
	-rm -r $(BUILD_DIR) $(BIN_DIR)
