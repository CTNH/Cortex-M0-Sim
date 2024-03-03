# Variables
SRC_DIR := ./src/
OBJ_DIR := ./bin/
BIN_DIR := ./bin/
BIN_NAME := pico_emu

# Find source files, strip directory part for object file names
SRC_FILES := $(shell find $(SRC_DIR) -type f \( -name "*.c" -o -name "*.cpp" \))
OBJ_FILES := $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRC_FILES:.c=.o))

CC := gcc
CXX := g++
# Add '-g' to flags for debug messages
CFLAGS :=
CXXFLAGS := -lncurses -lm

.PHONY: all warn debug createDir clean run

all: createDir $(BIN_DIR)$(BIN_NAME)
	$(info > All Done.)

warn: CFLAGS += -Wall
warn: CXXFLAGS += -Wall
warn: all
debug: CFLAGS += -g
debug: CXXFLAGS += -g
debug: warn

# Compile source to object files
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	$(info > Compiling $< to $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(info > Compiling $< to $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create executable
$(BIN_DIR)$(BIN_NAME): $(OBJ_FILES)
	$(info > Creating executable from object files)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Ensure directories are created
createDir:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

clean:
	rm -f $(OBJ_DIR)*.o $(BIN_DIR)$(BIN_NAME)

run: all
	./$(BIN_DIR)$(BIN_NAME)

