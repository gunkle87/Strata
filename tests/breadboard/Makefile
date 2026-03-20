CC = gcc
CFLAGS ?= -Wall -Wextra -std=c99
INCLUDES = -I../../include
SRC_DIR = ../../src/breadboard

TARGET_SKELETON := test_breadboard_skeleton.exe
TARGET_PAYLOAD := test_executable_payload_contract.exe
TARGET_LEGALITY := test_state_legality.exe
TARGET_UNINIT := test_uninit_handling.exe
TEST_EXES := $(TARGET_SKELETON) $(TARGET_PAYLOAD) $(TARGET_LEGALITY) $(TARGET_UNINIT)

all: $(TEST_EXES)

$(TARGET_SKELETON): test_breadboard_skeleton.c $(SRC_DIR)/breadboard_api.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ test_breadboard_skeleton.c $(SRC_DIR)/breadboard_api.c

$(TARGET_PAYLOAD): test_executable_payload_contract.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ test_executable_payload_contract.c

$(TARGET_LEGALITY): test_state_legality.c $(SRC_DIR)/breadboard_api.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ test_state_legality.c $(SRC_DIR)/breadboard_api.c

$(TARGET_UNINIT): test_uninit_handling.c $(SRC_DIR)/breadboard_api.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ test_uninit_handling.c $(SRC_DIR)/breadboard_api.c

test: all
	./$(TARGET_SKELETON)
	./$(TARGET_PAYLOAD)
	./$(TARGET_LEGALITY)
	./$(TARGET_UNINIT)

clean:
	cmd /C "del /Q /F $(TEST_EXES) 2>NUL"

.PHONY: all test clean
