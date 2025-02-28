# mugOS kernel makefile

TARGET_CFLAGS+=-D $(ARCH)

# Includes paths for the kernel
export CPATH=$(abspath .):$(abspath $(TOOLCHAIN_PATH)/include):$(abspath Stdlib):$(abspath Arch/$(ARCH))

BUILD_SUBDIR:=$(BUILD_DIR)/kernel
BUILD_SUBDIRS:=$(patsubst ./%, $(BUILD_SUBDIR)/%, $(shell find . -mindepth 1 -type d))

# Source files: include all subdirectories, excluding Arch, but including Arch/$(ARCH) ; and append Arch/*.{c,asm}
SOURCES_C=$(shell find . -type f -name "*.c" -and \( -not -path "./Arch/*" -or -path "./Arch/$(ARCH)/*" \))
SOURCES_C+=$(shell find ./Arch -maxdepth 1 -type f -name "*.c")
SOURCES_ASM=$(shell find . -type f -name "*.asm" -and \( -not -path "./Arch/*" -or -path "./Arch/$(ARCH)/*" \))
SOURCES_ASM+=$(shell find ./Arch -maxdepth 1 -type f -name "*.asm")

OBJECTS_C=$(patsubst ./%, $(BUILD_SUBDIR)/%, $(SOURCES_C:.c=.o))
OBJECTS_ASM=$(patsubst ./%, $(BUILD_SUBDIR)/%, $(SOURCES_ASM:.asm=.obj))

all: kernel

.PHONY: all kernel

kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf: $(OBJECTS_C) $(OBJECTS_ASM)
	@echo "LD $@"
	@$(TARGET_LD) $(TARGET_LDFLAGS) -T Kernel.ld -Map=$(BUILD_DIR)/kernel.map -o $@ $^ $(TARGET_LIBS)

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIRS)
	@echo "CC $< => $@"
	@$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

$(BUILD_SUBDIR)/%.obj: %.asm | $(BUILD_SUBDIRS)
	@echo "AS $< => $@"
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) $< -o $@

$(BUILD_SUBDIRS):
	@mkdir -p $@
