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
	# gcc -lm -Wall -o $(BIN_DIR)pico_emu *.o
#gcc -Wall $(SRC_DIR)*.c -o $(BIN_DIR)pico_emu

debug:
	mkdir -p $(BIN_DIR)
	#gcc -g -lm -Wall $(SRC_DIR)*.c -o $(BIN_DIR)pico_emu
	g++ -g -c -Wall $(SRC_DIR)*.cpp
	gcc -g -c -Wall $(SRC_DIR)*.c
	g++ -g -lm -Wall -o $(BIN_DIR)$(BIN_NAME) *.o

# Produce object files
build:
	g++ -c -Wall $(SRC_DIR)*.cpp
	gcc -c -Wall $(SRC_DIR)*.c

# Produce assembly files
asm:
	g++ -Wall -S -fverbose-asm $(SRC_DIR)*.cpp
	gcc -Wall -S -fverbose-asm $(SRC_DIR)*.c

clean:
	rm *.o

run:
	$(BIN_DIR)$(BIN_NAME)

