# First stage bootloader makefile

all: first-stage

.PHONY: all first-stage

first-stage: $(BUILD_DIR)/legacy-bootloader-first-stage.bin

$(BUILD_DIR)/legacy-bootloader-first-stage.bin: Boot.asm
	$(TARGET_ASM) $< -f bin -o $@
