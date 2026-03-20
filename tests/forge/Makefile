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
    ../../src/forge/forge_policy.c \
    ../../src/forge/forge_api.c \
    ../../src/common/forge_diagnostic.c

BREADBOARD_SRCS = \
    ../../src/breadboard/breadboard_api.c

TESTS = \
    test_backend_enum \
    test_backend_info \
    test_capabilities \
    test_invalid_handle \
    test_artifact_stub \
    test_artifact_admission_policy \
    test_session_lifecycle \
    test_runtime_stubs \
    test_descriptor_stubs \
    test_profile_policy \
    test_extensions \
    test_compatibility_plumbing \
    test_refusal_paths \
    test_projection_contract

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

test_artifact_admission_policy.exe: test_artifact_admission_policy.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_session_lifecycle.exe: test_session_lifecycle.c $(FORGE_SRCS) $(BREADBOARD_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS) $(BREADBOARD_SRCS)

test_runtime_stubs.exe: test_runtime_stubs.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_descriptor_stubs.exe: test_descriptor_stubs.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_profile_policy.exe: test_profile_policy.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_extensions.exe: test_extensions.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_compatibility_plumbing.exe: test_compatibility_plumbing.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_refusal_paths.exe: test_refusal_paths.c $(FORGE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(FORGE_SRCS)

test_projection_contract.exe: test_projection_contract.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

test: all
	@echo --- Running Forge skeleton tests ---
	@test_backend_enum.exe
	@test_backend_info.exe
	@test_capabilities.exe
	@test_invalid_handle.exe
	@test_artifact_stub.exe
	@test_artifact_admission_policy.exe
	@test_session_lifecycle.exe
	@test_runtime_stubs.exe
	@test_descriptor_stubs.exe
	@test_profile_policy.exe
	@test_extensions.exe
	@test_compatibility_plumbing.exe
	@test_refusal_paths.exe
	@test_projection_contract.exe
	@echo --- All Forge skeleton tests passed ---

clean:
	cmd /C "del /Q /F $(TEST_EXES) 2>NUL"

.PHONY: all test clean
