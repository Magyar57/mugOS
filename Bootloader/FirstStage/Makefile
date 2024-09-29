# Makefile - First stage bootloader

all: first-stage

.PHONY: all clean first-stage

first-stage: $(BUILD_DIR)/bootloader-first-stage.bin

$(BUILD_DIR)/bootloader-first-stage.bin: Boot.asm
	$(ASM) $< -f bin -o $@

clean:
	rm -f $(BUILD_DIR)/bootloader-first-stage.bin
