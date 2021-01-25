# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
TARGET_EXEC := ethbridge

CC = gcc
CFLAGS= -Wall -Werror -MD -MMD -g
LDFLAGS= -lpcap

BUILD_DIR := ./build

# Find all the C and C++ files we want to compile
SOURCES = $(wildcard src/*.c) \
		$(wildcard src/interfaces/*.c) \
		$(wildcard src/utils/*.c) \
		$(wildcard src/forward/*.c) \
		$(wildcard src/processing/*.c) \


OBJECTS := $(SOURCES:%=$(BUILD_DIR)/%.o)

DEPS := $(OBJECTS:.o=.d)


$(BUILD_DIR)/$(TARGET_EXEC): $(OBJECTS)
	mkdir -p $(dir $@)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -r $(BUILD_DIR)


-include $(DEPS)