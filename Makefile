
SRC_DIR = tall_compiler/src
INC_DIR = tall_compiler/inc

OUTPUT_DIR = bin

SRC_FILES = $(SRC_DIR)/*.c
OUTPUT = $(OUTPUT_DIR)/tall

CC = gcc
CFLAGS = -Wall -Werror -g -I$(INC_DIR)

all : $(OUTPUT)

$(OUTPUT) : $(SRC_FILES)
	mkdir $p $(OUTPUT_DIR)
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -f $(OUTPUT)

