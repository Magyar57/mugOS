# mugOS kernel makefile

TARGET_CFLAGS+=-D KERNEL -D $(ARCH)

# Includes paths for the kernel
export CPATH=$(abspath $(TOOLCHAIN_PATH)/include):$(abspath .):$(abspath Stdlib):$(abspath Arch/$(ARCH)/Include)

BUILD_SUBDIR:=$(BUILD_DIR)/kernel
BUILD_SUBDIRS:=$(patsubst ./%, $(BUILD_SUBDIR)/%, $(shell find . -mindepth 1 -type d -and -not -path "./Arch*"))

# Source files: include all sub-directories, excluding Arch
SOURCES_C=$(shell find . -type f -name "*.c" -and -not -path "./Arch/*")
OBJECTS_C=$(patsubst ./%, $(BUILD_SUBDIR)/%, $(SOURCES_C:.c=.o))

# Arch files
ARCH_SOURCES_C=$(shell find ./Arch/$(Arch) -type f -name "*.c")
ARCH_SOURCES_ASM=$(shell find ./Arch/$(Arch) -type f -name "*.asm")
ARCH_OBJECTS_C=$(patsubst ./%, $(BUILD_SUBDIR)/%, $(ARCH_SOURCES_C:.c=.o))
ARCH_OBJECTS_ASM=$(patsubst ./%, $(BUILD_SUBDIR)/%, $(ARCH_SOURCES_ASM:.asm=.obj))

all: kernel

.PHONY: all kernel

kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf: $(OBJECTS_C) $(ARCH_SOURCES_C) $(ARCH_SOURCES_ASM)
	@$(MAKE) $(MAKE_FLAGS) -C Arch/$(ARCH)
	@echo "LD $@"
	@$(TARGET_LD) $(TARGET_LDFLAGS) -T Kernel.ld -Map=$(BUILD_DIR)/kernel.map -o $@ $(OBJECTS_C) $(ARCH_OBJECTS_C) $(ARCH_OBJECTS_ASM) $(TARGET_LIBS)

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIRS)
	@echo "CC $< => $@"
	@$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

$(BUILD_SUBDIRS):
	@mkdir -p $@
