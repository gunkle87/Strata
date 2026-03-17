# Standalone Makefile for bench_convert

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99

SRCS = src/main.c src/ast.c src/blif_reader.c src/verilog_reader.c src/bench_writer.c
OBJS = $(SRCS:.c=.o)
TARGET = bench_convert.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	cmd /C "del /Q /F src\main.o src\ast.o src\blif_reader.o src\verilog_reader.o src\bench_writer.o bench_convert.exe 2>NUL"

test: $(TARGET)
	powershell -ExecutionPolicy Bypass -File ..\..\tests\tools\bench_convert\test_runner.ps1

.PHONY: all clean test
