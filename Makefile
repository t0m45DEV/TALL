
MAIN_DIR = tall

SRC_DIR = $(MAIN_DIR)/src
INC_DIR = $(MAIN_DIR)/inc

OUTPUT_DIR = bin

SRC_FILES = $(SRC_DIR)/*.c

PROGRAM = tall

OUTPUT = $(OUTPUT_DIR)/$(PROGRAM)

CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wshadow -fanalyzer -I$(INC_DIR)

DEBUGGER = gdb

MEM_CHECKER = valgrind
MEM_CHECKER_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes -s
MEM_CHECK_FILE ?= tests/ints.tll

STATIC_CHECKER = clang-tidy
STATIC_CHECKER_FLAGS = --checks="-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,-clang-analyzer-valist.Uninitialized"

TESTS_DIR = tests
TESTS = $(shell find $(TESTS_DIR)/ -name "*.tll")

$(OUTPUT) : $(SRC_FILES)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $^ $(CFLAGS) -o $@

debug :
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(SRC_FILES) $(CFLAGS) -g -o $(OUTPUT)
	$(DEBUGGER) $(OUTPUT)

mem_check : $(OUTPUT)
	$(MEM_CHECKER) $(MEM_CHECKER_FLAGS) $(OUTPUT) $(MEM_CHECK_FILE)

test : $(OUTPUT) $(TESTS_DIR)
	@pass=0; fail=0; \
	for t in $(TESTS); do \
		if $(OUTPUT) $$t > /dev/null 2>&1; then \
			echo "   PASS $$t"; \
			pass=$$((pass + 1)); \
		else \
			echo "   FAIL $$t"; \
			fail=$$((fail + 1)); \
		fi \
	done; \
	echo ""; \
	echo "Results: $$pass passed, $$fail failed."

analyze:
	$(STATIC_CHECKER) $(SRC_FILES) $(STATIC_CHECKER_FLAGS)

install : $(OUTPUT)
	cp $(OUTPUT) /usr/bin/$(PROGRAM)

uninstall :
	rm /usr/bin/$(PROGRAM)

clean:
	rm -f $(OUTPUT_DIR)/*

