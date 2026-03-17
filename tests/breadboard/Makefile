CC = gcc
CFLAGS = -Wall -Wextra -I../../include
SRC_DIR = ../../src/breadboard

all: test_breadboard_skeleton.exe

test_breadboard_skeleton.exe: test_breadboard_skeleton.c $(SRC_DIR)/breadboard_api.c
	$(CC) $(CFLAGS) -o $@ test_breadboard_skeleton.c $(SRC_DIR)/breadboard_api.c

test: all
	@echo --- Running Breadboard skeleton tests ---
	@.\test_breadboard_skeleton.exe
	@echo --- All Breadboard skeleton tests passed ---

clean:
	cmd /C "del /Q /F test_breadboard_skeleton.exe 2>NUL"

.PHONY: all test clean
