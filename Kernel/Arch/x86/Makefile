TARGET_ASMFLAGS+=
TARGET_CFLAGS+=

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, $(BUILD_SUBDIR)/%.o, $(SOURCES_C))
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_SUBDIR)/%.obj, $(SOURCES_ASM))

SUB_SOURCES=$(shell find . -mindepth 2 -type f)

all: arch

.PHONY: all arch

arch: $(OBJECTS_C) $(OBJECTS_ASM) $(SUB_SOURCES)
	@$(MAKE) -C Drivers $(MAKE_FLAGS)

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIR)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<

$(BUILD_SUBDIR)/%.obj: %.asm | $(BUILD_SUBDIR)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
