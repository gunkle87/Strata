CC = gcc
CFLAGS ?= -Wall -Wextra -std=c99
INCLUDES = -I../../include
SRC_DIR = ../../src/breadboard

TESTS = \
	test_breadboard_skeleton \
	test_executable_payload_contract \
	test_state_legality

TEST_EXES = $(addsuffix .exe, $(TESTS))

all: $(TEST_EXES)

test_breadboard_skeleton.exe: test_breadboard_skeleton.c $(SRC_DIR)/breadboard_api.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ test_breadboard_skeleton.c $(SRC_DIR)/breadboard_api.c

test_executable_payload_contract.exe: test_executable_payload_contract.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ test_executable_payload_contract.c

test_state_legality.exe: test_state_legality.c $(SRC_DIR)/breadboard_api.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ test_state_legality.c $(SRC_DIR)/breadboard_api.c

test: all
	@echo --- Running Breadboard skeleton tests ---
	@.\test_breadboard_skeleton.exe
	@.\test_executable_payload_contract.exe
	@.\test_state_legality.exe
	@echo --- All Breadboard skeleton tests passed ---

clean:
	cmd /C "del /Q /F $(TEST_EXES) 2>NUL"

.PHONY: all test clean
