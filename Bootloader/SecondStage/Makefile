# Second stage bootloader makefile

BUILD_SUBDIR:=$(BUILD_DIR)/second-stage

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, $(BUILD_SUBDIR)/%.o, $(SOURCES_C))
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_SUBDIR)/%.obj, $(SOURCES_ASM))

all: second-stage

.PHONY: all second-stage

second-stage: $(BUILD_DIR)/bootloader-second-stage.bin

$(BUILD_DIR)/bootloader-second-stage.bin: $(OBJECTS_C) $(OBJECTS_ASM)
	$(TARGET_LD) $(TARGET_LDFLAGS) -T Linker.map -Wl,-Map=$(BUILD_DIR)/bootloader-second-stage.map -o $@ $(BUILD_SUBDIR)/*.{o,obj} $(TARGET_LIBS)

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIR)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<

$(BUILD_SUBDIR)/%.obj: %.asm | $(BUILD_SUBDIR)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<

# Build dirs

$(BUILD_SUBDIR):
	@mkdir -p $@
