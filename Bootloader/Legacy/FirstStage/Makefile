# First stage bootloader makefile

all: first-stage

.PHONY: all first-stage

first-stage: $(BUILD_DIR)/bootloader-first-stage.bin

$(BUILD_DIR)/bootloader-first-stage.bin: Boot.asm
	$(ASM) $< -f bin -o $@
