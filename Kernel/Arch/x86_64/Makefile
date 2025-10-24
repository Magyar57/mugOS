# mugOS x86_64 architecture makefile

# Includes paths for the architecture
export CPATH=$(abspath $(TOOLCHAIN_PATH)/include):$(abspath .):$(abspath ./Include):$(abspath ../..):$(abspath ../../../Stdlib)

BUILD_SUBDIR:=$(BUILD_DIR)/kernel/Arch/$(ARCH)
BUILD_SUBDIRS:=$(patsubst ./%, $(BUILD_SUBDIR)/%, $(shell find . -mindepth 1 -type d))

SOURCES_C=$(shell find . -type f -name "*.c")
SOURCES_ASM=$(shell find . -type f -name "*.asm")
OBJECTS_C=$(patsubst ./%, $(BUILD_SUBDIR)/%, $(SOURCES_C:.c=.o))
OBJECTS_ASM=$(patsubst ./%, $(BUILD_SUBDIR)/%, $(SOURCES_ASM:.asm=.obj))

all: arch

.PHONY: all arch

arch: $(OBJECTS_C) $(OBJECTS_ASM)

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIRS)
	@echo "CC $< -> $@"
	@$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

$(BUILD_SUBDIR)/%.obj: %.asm | $(BUILD_SUBDIRS)
	@echo "AS $< -> $@"
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) $< -o $@

$(BUILD_SUBDIRS):
	@mkdir -p $@
