ASMFLAGS+=-f obj

BUILD_SUBDIR:=$(BUILD_DIR)/second-stage

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, $(BUILD_SUBDIR)/C/%.obj, $(SOURCES_C))
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_SUBDIR)/Asm/%.obj, $(SOURCES_ASM))

all: second-stage

.PHONY: all clean second-stage

second-stage: $(BUILD_DIR)/bootloader-second-stage.bin

$(BUILD_DIR)/bootloader-second-stage.bin: $(OBJECTS_C) $(OBJECTS_ASM)
	$(LD16) NAME $@ FILE \{ $^ } OPTION MAP=$(BUILD_DIR)/bootloader-second-stage.map @Linker.lnk

# Generic object rules

$(BUILD_SUBDIR)/C/%.obj: %.c | $(BUILD_SUBDIR)/C
	$(CC16) $(CFLAGS16) -fo=$@ $<

$(BUILD_SUBDIR)/Asm/%.obj: %.asm | $(BUILD_SUBDIR)/Asm
	$(ASM) $(ASMFLAGS) -o $@ $<

# Build dirs

$(BUILD_SUBDIR):
	mkdir -p $@

$(BUILD_SUBDIR)/C: | $(BUILD_SUBDIR)
	mkdir -p $@

$(BUILD_SUBDIR)/Asm: | $(BUILD_SUBDIR)
	mkdir -p $@

clean:
	rm -rf $(BUILD_SUBDIR) $(BUILD_DIR)/bootloader-second-stage.bin
