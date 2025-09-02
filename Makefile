CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
LDFLAGS = -municode -lm -lgdi32 -luser32

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TARGET = $(BUILD_DIR)/main

ifeq ($(OS),Windows_NT)
  EXE := .exe
  RM := rmdir /s /q
  RUN := $(TARGET)$(EXE)
else
  EXE :=
  RM := rm -rf
  RUN := ./$(TARGET)
endif


.PHONY: all clean run

all: $(BUILD_DIR) $(TARGET)$(EXE)

$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

$(TARGET)$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) "$(BUILD_DIR)"

run: $(TARGET)$(EXE)
	$(RUN)
