CC = gcc
CFLAGS = -g -O2 -Wall -Wextra
TARGET = harborDB

BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src
TEST_DIR = tests
DATABASEFILE = mydb.db

SOURCES = main.c repl.c vm.c parser.c storage.c btree.c
OBJECTS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SOURCES))
DEPS = &(OBJECTS:.o=.d)

all: $(BIN_DIR)/$(TARGET)

run : $(BIN_DIR)/$(TARGET)
	$(BIN_DIR)/$(TARGET) $(DATABASEFILE)

$(BIN_DIR)/$(TARGET): $(OBJECTS) | $(BIN_DIR)
	rm $(DATABASEFILE) 
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

rebuild: clean all 

test: $(BIN_DIR)/$(TARGET) | $(TEST_DIR)
	@pytest $(TEST_DIR)/ -vvs

-include $(DEPS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TEST_DIR): 
	mkdir -p $(TEST_DIR)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all run rebuild test clean hello.txt
