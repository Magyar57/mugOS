TARGET_ASMFLAGS+=-f elf
TARGET_CFLAGS+=-ffreestanding -nostdlib -I. -I../..
TARGET_LIBS+=-lgcc
TARGET_LDFLAGS+=-T Linker.map -nostdlib

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, $(BUILD_SUBDIR)/%.o, $(SOURCES_C))
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_SUBDIR)/%.obj, $(SOURCES_ASM))

all: arch

.PHONY: all clean arch

arch: $(OBJECTS_C) $(OBJECTS_ASM)

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIR)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<

$(BUILD_SUBDIR)/%.obj: %.asm | $(BUILD_SUBDIR)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<

# Build dirs

clean:
	rm -rf $(BUILD_SUBDIR) $(BUILD_DIR)/$(OBJECTS_ASM) $(BUILD_DIR)/$(OBJECTS_ASM)
