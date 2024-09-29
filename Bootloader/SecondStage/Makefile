TARGET_ASMFLAGS+=-f elf
TARGET_CFLAGS+=-ffreestanding -nostdlib
TARGET_LIBS+=-lgcc
TARGET_LDFLAGS+=-T Linker.map -nostdlib

BUILD_SUBDIR:=$(BUILD_DIR)/second-stage

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, $(BUILD_SUBDIR)/%.o, $(SOURCES_C))
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_SUBDIR)/%.obj, $(SOURCES_ASM))

all: second-stage

.PHONY: all clean second-stage

second-stage: $(BUILD_DIR)/bootloader-second-stage.bin

$(BUILD_DIR)/bootloader-second-stage.bin: $(OBJECTS_C) $(OBJECTS_ASM)
	$(TARGET_LD) $(TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR)/bootloader-second-stage.map -o $@ $(BUILD_SUBDIR)/*.o $(BUILD_SUBDIR)/*.obj $(TARGET_LIBS)

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIR)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<

$(BUILD_SUBDIR)/%.obj: %.asm | $(BUILD_SUBDIR)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<

# Build dirs

$(BUILD_SUBDIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILD_SUBDIR) $(BUILD_DIR)/bootloader-second-stage.bin
