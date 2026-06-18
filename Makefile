
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

CPROFILER = gprof
PROFILER_OUTPUT = profile_report.txt

MEM_CHECKER = valgrind
MEM_CHECKER_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes -s

CHECK_FILE ?= tests/ints.tll

STATIC_CHECKER = clang-tidy
STATIC_CHECKER_FLAGS = --checks="-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,-clang-analyzer-valist.Uninitialized"

TESTS_DIR = tests
TESTS = $(shell find $(TESTS_DIR)/ -name "*.tll")

$(OUTPUT) : $(SRC_FILES)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $^ $(CFLAGS) -o $@

debug : $(SRC_FILES)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $^ $(CFLAGS) -g -o $(OUTPUT)
	$(DEBUGGER) $(OUTPUT)

profiler: $(SRC_FILES)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $^ $(CFLAGS) -pg -o $(OUTPUT)
	$(OUTPUT) $(CHECK_FILE)
	$(CPROFILER) $(OUTPUT) gmon.out > $(PROFILER_OUTPUT)
	cat $(PROFILER_OUTPUT)
	rm $(PROFILER_OUTPUT) gmon.out

analyze : $(SRC_FILES)
	$(STATIC_CHECKER) $^ $(STATIC_CHECKER_FLAGS)

mem_check : $(OUTPUT)
	$(MEM_CHECKER) $(MEM_CHECKER_FLAGS) $(OUTPUT) $(CHECK_FILE)

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

install : $(OUTPUT)
	cp $(OUTPUT) /usr/bin/$(PROGRAM)

uninstall :
	rm /usr/bin/$(PROGRAM)

clean:
	rm -f $(OUTPUT_DIR)/*

