# Variables
SRC_DIR = ./src/
# Path to place executable in
BIN_DIR = ./bin/
# Name of executable
BIN_NAME = pico_emu

.PHONY : default_target all debug build asm clean

default_target: all clean

# Compile all directly to an executable file
all:
	mkdir -p $(BIN_DIR)
	@$(MAKE) build
	g++ -lm -Wall -o $(BIN_DIR)$(BIN_NAME) *.o

debug:
	mkdir -p $(BIN_DIR)
	g++ -g -lm -Wall $(SRC_DIR)*.cpp -o $(BIN_DIR)$(BIN_NAME)

# Produce object files
build:
	g++ -c -Wall $(SRC_DIR)*.cpp

# Produce assembly files
asm:
	g++ -Wall -S -fverbose-asm $(SRC_DIR)*.cpp

clean:
	rm *.o

run:
	$(BIN_DIR)$(BIN_NAME)

