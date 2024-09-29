include BuildScripts/Config.mk
include BuildScripts/Toolchain.mk

all: floppy tools

.PHONY: all clean
.PHONY: floppy bootloader kernel
.PHONY: tools tools_fat

#
# Floppy image
#
floppy: $(BUILD_DIR)/floppy.img
FLOPPY_FILES:=$(BUILD_DIR)/bootloader-first-stage.bin
FLOPPY_FILES+=$(BUILD_DIR)/bootloader-second-stage.bin
FLOPPY_FILES+=$(BUILD_DIR)/kernel.bin
FLOPPY_FILES+=$(BUILD_DIR)/test.txt
FLOPPY_FILES+=$(BUILD_DIR)/test_sub.txt

$(BUILD_DIR)/floppy.img: $(FLOPPY_FILES) | $(BUILD_DIR)
	dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	mkfs.fat -F 12 -n "MUGOS" $@
	dd if=$(BUILD_DIR)/bootloader-first-stage.bin of=$@ conv=notrunc status=none
	mcopy -i $@ $(BUILD_DIR)/bootloader-second-stage.bin "::2ndStage.bin"
	mcopy -i $@ $(BUILD_DIR)/kernel.bin "::kernel.bin"
	mcopy -i $@ $(BUILD_DIR)/test.txt "::test.txt"
	mmd -i $@ "::dir"
	mcopy -i $@ $(BUILD_DIR)/test_sub.txt "::dir/test_sub.txt"

$(BUILD_DIR)/test.txt:
	printf "This is a test file :D\n" >$@

$(BUILD_DIR)/test_sub.txt:
	printf "This is a test file, in a subdirectory !\n" >$@

#
# Bootloader
#
bootloader: $(BUILD_DIR)/bootloader-first-stage.bin $(BUILD_DIR)/bootloader-second-stage.bin

$(BUILD_DIR)/bootloader-first-stage.bin: $(shell find Bootloader/FirstStage/** -type f) | $(BUILD_DIR)
	@$(MAKE) -C Bootloader/FirstStage $(MAKE_FLAGS)

$(BUILD_DIR)/bootloader-second-stage.bin: $(shell find Bootloader/SecondStage/** -type f) | $(BUILD_DIR)
	@$(MAKE) -C Bootloader/SecondStage $(MAKE_FLAGS)

#
# Kernel
#
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(shell find Kernel/** -type f) | $(BUILD_DIR)
	@$(MAKE) -C Kernel $(MAKE_FLAGS)

#
# Tools
#
tools: tools_fat
tools_fat: $(BUILD_TOOLS_FAT_DIR)/tests $(BUILD_TOOLS_FAT_DIR)/cli

$(BUILD_TOOLS_FAT_DIR)/tests: $(BUILD_TOOLS_FAT_DIR)/Tests.o $(BUILD_TOOLS_FAT_DIR)/Fat.o | $(BUILD_TOOLS_FAT_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_TOOLS_FAT_DIR)/cli: $(BUILD_TOOLS_FAT_DIR)/FatCLI.o $(BUILD_TOOLS_FAT_DIR)/Fat.o | $(BUILD_TOOLS_FAT_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_TOOLS_FAT_DIR)/%.o : Tools/FAT/%.c | $(BUILD_TOOLS_FAT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

#
# Build directories
#

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_TOOLS_DIR): | $(BUILD_DIR)
	@mkdir -p $@

$(BUILD_TOOLS_FAT_DIR): | $(BUILD_TOOLS_DIR)
	@mkdir -p $@

#
# Clean
#
clean:
	rm -rf $(BUILD_DIR)
