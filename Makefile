
MAIN_DIR = tall

SRC_DIR = $(MAIN_DIR)/src
INC_DIR = $(MAIN_DIR)/inc

OUTPUT_DIR = bin

SRC_FILES = $(SRC_DIR)/*.c

PROGRAM = tall

OUTPUT = $(OUTPUT_DIR)/$(PROGRAM)

CC = gcc
CFLAGS = -Wall -Werror -I$(INC_DIR)

DEBUGGER = gdb

MEM_CHECKER = valgrind
MEM_CHECKER_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes -s

$(OUTPUT) : $(SRC_FILES)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $^ $(CFLAGS) -o $@

debug :
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(SRC_FILES) $(CFLAGS) -g -o $(OUTPUT)
	$(DEBUGGER) $(OUTPUT)

mem_check : $(OUTPUT)
	$(MEM_CHECKER) $(MEM_CHECKER_FLAGS) $(OUTPUT) test/test.tll

install : $(OUTPUT)
	cp $(OUTPUT) /usr/bin/$(PROGRAM)

uninstall :
	rm /usr/bin/$(PROGRAM)

clean:
	rm -f $(OUTPUT_DIR)/*

