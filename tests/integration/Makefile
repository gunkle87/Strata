CC = gcc
CFLAGS = -Wall -Wextra -std=c99

INCLUDES = \
    -I../../include \
    -I../../src/common \
    -I../../src/forge

FORGE_SRCS = \
    ../../src/forge/forge_registry.c \
    ../../src/forge/forge_policy.c \
    ../../src/forge/forge_api.c \
    ../../src/common/forge_diagnostic.c

BREADBOARD_SRCS = \
    ../../src/breadboard/breadboard_api.c

TESTS = \
    test_breadboard_forge_placeholder_handoff

TEST_EXES = $(addsuffix .exe, $(TESTS))

all: $(TEST_EXES)

test_breadboard_forge_placeholder_handoff.exe: test_breadboard_forge_placeholder_handoff.c $(FORGE_SRCS) $(BREADBOARD_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS) $(BREADBOARD_SRCS)

test: all
	@echo --- Running integration tests ---
	@test_breadboard_forge_placeholder_handoff.exe
	@echo --- All integration tests passed ---

clean:
	cmd /C "del /Q /F $(TEST_EXES) 2>NUL"

.PHONY: all test clean
