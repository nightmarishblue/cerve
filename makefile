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
CFLAGS ?= -Wall -Wpedantic -std=gnu23
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
	-rmdir $(OUT_DIR) $(OBJ_DIR) $(BUILD_DIR)

## show help for targets
help:
	@grep -A1 '^##' $(MAKEFILE_LIST) | sed 's/:.*//'

# compile each source to an object, plus dependency files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MP -c $< -o $@

# filter out objects with main()
FILTERED := $(filter-out $(OBJ_DIR)/bin/%,$(OBJS))
# link all objects into the desired binary
$(OUT_DIR)/%: $(OBJ_DIR)/bin/%.o $(FILTERED)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) $^ -o $@ $(LDLIBS)

# misc
# recompile sources if their headers change
-include $(DEPS)