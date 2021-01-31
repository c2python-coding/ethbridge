# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
TARGET_EXEC := ethbridge
OS := $(shell uname)
LINUX_OS := Linux
MAC_OS := Darwin

CC = gcc
CFLAGS= -Wall -Werror -MD -g 
LDFLAGS= -lpcap

ifeq ($(OS),$(LINUX_OS))
    CFLAGS += -MMD
else ifeq ($(OS),$(MAC_OS))
	CFLAGS += -DOSX
else
    $(error $(OS) is not supported)
endif

BUILD_DIR := ./build

# Find all the C and C++ files we want to compile
SOURCES = $(wildcard src/*.c) \
		$(wildcard src/capture/*.c) \
		$(wildcard src/utils/*.c) \
		$(wildcard src/forward/*.c) \
		$(wildcard src/processing/*.c) \
		$(wildcard src/handshake/*.c) \


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