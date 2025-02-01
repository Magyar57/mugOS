# mugOS kernel makefile

TARGET_CFLAGS+=-D $(ARCH)

# Includes paths for the kernel
export CPATH=$(abspath .):$(abspath Stdlib):$(abspath Arch/$(ARCH)):$(abspath $(TARGET_TOOLCHAIN)/include)

BUILD_SUBDIR:=$(BUILD_DIR)/kernel

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, $(BUILD_SUBDIR)/%.o, $(SOURCES_C))

ARCH_SOURCES=$(shell find Arch/$(ARCH)/** -type f)
SUB_SOURCES=$(shell find . -mindepth 2 ! -path "./Arch/*" -type f) # All files in all subdirs EXCEPT Arch

all: kernel

.PHONY: all kernel

kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf: $(OBJECTS_C) $(OBJECTS_ASM) $(BUILD_SUBDIR)/HAL.o Arch/HAL.h $(ARCH_SOURCES) $(SUB_SOURCES)
	@$(MAKE) $(MAKE_FLAGS) BUILD_SUBDIR=$(BUILD_SUBDIR) -C Stdlib
	@$(MAKE) $(MAKE_FLAGS) BUILD_SUBDIR=$(BUILD_SUBDIR) -C Drivers
	@$(MAKE) $(MAKE_FLAGS) BUILD_SUBDIR=$(BUILD_SUBDIR) -C Drivers/Graphics
	@$(MAKE) $(MAKE_FLAGS) BUILD_SUBDIR=$(BUILD_SUBDIR) -C Arch/$(ARCH)
	$(TARGET_LD) $(TARGET_LDFLAGS) -T Kernel.ld -Map=$(BUILD_DIR)/kernel.map -o $@ $(BUILD_SUBDIR)/*.{o,obj} $(TARGET_LIBS)

$(BUILD_SUBDIR)/HAL.o: Arch/HAL.c | $(BUILD_SUBDIR)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIR)
	$(TARGET_CC) $(TARGET_CFLAGS) -c $< -o $@

$(BUILD_SUBDIR)/%.obj: %.asm | $(BUILD_SUBDIR)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) $< -o $@

# Build dir

$(BUILD_SUBDIR):
	@mkdir -p $@
