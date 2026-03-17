# Makefile for Forge skeleton tests
#
# Builds all Forge test executables by compiling each test file directly with the
# Forge source files. No shared library is used at this stage.
#
# Usage:
#   make          - build all test executables
#   make test     - build and run all tests
#   make clean    - remove built executables

CC     = gcc
CFLAGS = -Wall -Wextra -std=c99

INCLUDES = \
    -I../../include \
    -I../../src/common \
    -I../../src/forge

FORGE_SRCS = \
    ../../src/forge/forge_registry.c \
    ../../src/forge/forge_api.c \
    ../../src/common/forge_diagnostic.c

TESTS = \
    test_backend_enum \
    test_backend_info \
    test_capabilities \
    test_invalid_handle \
    test_artifact_stub \
    test_session_lifecycle \
    test_runtime_stubs

TEST_EXES = $(addsuffix .exe, $(TESTS))

all: $(TEST_EXES)

test_backend_enum.exe: test_backend_enum.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_backend_info.exe: test_backend_info.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_capabilities.exe: test_capabilities.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_invalid_handle.exe: test_invalid_handle.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_artifact_stub.exe: test_artifact_stub.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_session_lifecycle.exe: test_session_lifecycle.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_runtime_stubs.exe: test_runtime_stubs.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test: all
	@echo --- Running Forge skeleton tests ---
	@test_backend_enum.exe
	@test_backend_info.exe
	@test_capabilities.exe
	@test_invalid_handle.exe
	@test_artifact_stub.exe
	@test_session_lifecycle.exe
	@test_runtime_stubs.exe
	@echo --- All Forge skeleton tests passed ---

clean:
	cmd /C "del /Q /F $(TEST_EXES) 2>NUL"

.PHONY: all test clean
