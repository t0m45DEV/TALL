
MAIN_DIR = tall

SRC_DIR = $(MAIN_DIR)/src
INC_DIR = $(MAIN_DIR)/inc

OUTPUT_DIR = bin

SRC_FILES = $(SRC_DIR)/*.c

OUTPUT_RELEASE = $(OUTPUT_DIR)/tall
OUTPUT_DEBUG = $(OUTPUT_RELEASE)_debug

CC = gcc
CFLAGS = -Wall -Werror -I$(INC_DIR)
DEBUGFLAGS = $(CFLAGS) -g -DVERBOSE

all : $(OUTPUT_RELEASE) $(OUTPUT_DEBUG)

$(OUTPUT_RELEASE) : $(SRC_FILES)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $^ $(CFLAGS) -o $@

$(OUTPUT_DEBUG) : $(SRC_FILES)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $^ $(DEBUGFLAGS) -o $@

clean:
	rm -f $(OUTPUT_DIR)/*

