KERNEL_SRC_DIR:=.

all: kernel

.PHONY: all clean kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(KERNEL_SRC_DIR)/Main.asm
	$(ASM) $(KERNEL_SRC_DIR)/Main.asm -f bin -o $@

clean:
	rm -f $(BUILD_DIR)/kernel.bin
