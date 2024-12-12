TARGET_ASMFLAGS+=-f elf
TARGET_CFLAGS+=-ffreestanding -nostdlib -D ARCH_$(ARCH)
TARGET_LIBS+=-lgcc
TARGET_LDFLAGS+=-T Linker.map -nostdlib

# Includes paths for the kernel
export CPATH=$(abspath .):$(abspath Stdlib):$(abspath Arch/$(ARCH))

BUILD_SUBDIR:=$(BUILD_DIR)/kernel

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, $(BUILD_SUBDIR)/%.o, $(SOURCES_C))

ARCH_SOURCES=$(shell find Arch/$(ARCH)/** -type f)
SUB_SOURCES=$(shell find . -mindepth 2 ! -path "./Arch/*" -type f) # All files in all subdirs EXECPT Arch

all: kernel

.PHONY: all kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(OBJECTS_C) $(OBJECTS_ASM) $(BUILD_SUBDIR)/HAL.o Arch/HAL.h $(ARCH_SOURCES) $(SUB_SOURCES)
	@$(MAKE) -C Stdlib $(MAKE_FLAGS) BUILD_SUBDIR=$(BUILD_SUBDIR)
	@$(MAKE) -C Drivers $(MAKE_FLAGS) BUILD_SUBDIR=$(BUILD_SUBDIR)
	@$(MAKE) -C Arch/$(ARCH) $(MAKE_FLAGS) BUILD_SUBDIR=$(BUILD_SUBDIR)
	$(TARGET_LD) $(TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $(BUILD_SUBDIR)/*.o $(BUILD_SUBDIR)/*.obj $(TARGET_LIBS)

$(BUILD_SUBDIR)/HAL.o: Arch/HAL.c | $(BUILD_SUBDIR)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIR)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<

$(BUILD_SUBDIR)/%.obj: %.asm | $(BUILD_SUBDIR)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<

# Build dirs

$(BUILD_SUBDIR):
	@mkdir -p $@
