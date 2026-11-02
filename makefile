# project structure
TARGET := cerve

# inputs
SRC_DIR := src
SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')

INC_DIR := inc
INCS := $(shell find $(INC_DIR) -type f -name '*.h')

# outputs
BUILD_DIR := build

OBJ_DIR := $(BUILD_DIR)/obj
OBJS := $(patsubst $(SRC_DIR)%.c,$(OBJ_DIR)%.o,$(SRCS))

DEPS := $(OBJS:.o=.d)

OUT_DIR := bin


# toolchain
CFLAGS ?= -Wall -Wpedantic -std=gnu23 -MMD -MP
CPPFLAGS += -I $(INC_DIR)


# rules
.PHONY: build debug format clean help
## build the main binary
build: $(OUT_DIR)/$(TARGET)

## build the binary with debug symbols attached
debug: CFLAGS += -g
debug: clean build

## apply formatting to sources and headers
format:
	clang-format -i -style=file $(SRCS) $(INCS)

## remove all build output
clean:
	$(RM) "$(OUT_DIR)/$(TARGET)" $(OBJS) $(DEPS)
	rmdir --ignore-fail-on-non-empty $(OUT_DIR) $(OBJ_DIR) $(BUILD_DIR)

## show help for targets
help:
	@grep -A1 '^##' $(MAKEFILE_LIST) | sed 's/:.*//'

$(OUT_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


# misc
# recompile sources if their headers change
-include $(DEPS)