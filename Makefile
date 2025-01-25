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
FLOPPY_FILES:=$(BUILD_DIR)/BOOTX64.EFI
FLOPPY_FILES+=$(BUILD_DIR)/legacy-bootloader-first-stage.bin
FLOPPY_FILES+=$(BUILD_DIR)/legacy-bootloader-second-stage.bin
FLOPPY_FILES+=$(BUILD_DIR)/kernel.bin
FLOPPY_FILES+=$(BUILD_DIR)/test.txt
FLOPPY_FILES+=$(BUILD_DIR)/test_sub.txt

$(BUILD_DIR)/floppy.img: $(FLOPPY_FILES) | $(BUILD_DIR)
	dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	# mkfs.fat -F 12 -n "MUGOS" $@
	mformat -i $@ -f 2880 ::
	dd if=$(BUILD_DIR)/legacy-bootloader-first-stage.bin of=$@ conv=notrunc status=none
	mcopy -i $@ $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/test.txt ::
	mcopy -i $@ $(BUILD_DIR)/legacy-bootloader-second-stage.bin ::2ndStage.bin
	mmd -i $@ ::dir ::/EFI ::/EFI/BOOT
	mcopy -i $@ $(BUILD_DIR)/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i $@ $(BUILD_DIR)/test_sub.txt ::

$(BUILD_DIR)/test.txt:
	printf "This is a test file :D\n" >$@

$(BUILD_DIR)/test_sub.txt:
	printf "This is a test file, in a subdirectory !\n" >$@

#
# Bootloader
#
bootloader: $(BUILD_DIR)/BOOTX64.EFI $(BUILD_DIR)/legacy-bootloader-first-stage.bin $(BUILD_DIR)/legacy-bootloader-second-stage.bin

$(BUILD_DIR)/BOOTX64.EFI: $(shell find Bootloader/UEFI/** -type f) | $(BUILD_DIR)
	@$(MAKE) -C Bootloader/UEFI $(MAKE_FLAGS)

$(BUILD_DIR)/legacy-bootloader-first-stage.bin: $(shell find Bootloader/Legacy/FirstStage/** -type f) | $(BUILD_DIR)
	@$(MAKE) -C Bootloader/Legacy/FirstStage $(MAKE_FLAGS)

$(BUILD_DIR)/legacy-bootloader-second-stage.bin: $(shell find Bootloader/Legacy/SecondStage/** -type f) | $(BUILD_DIR)
	@$(MAKE) -C Bootloader/Legacy/SecondStage $(MAKE_FLAGS)

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
# Clean (bclean: bootloader clean, kclean: kernel clean)
#
clean:
	rm -rf $(BUILD_DIR)

bclean:
	rm -rf $(BUILD_DIR)/second-stage $(BUILD_DIR)/bootloader*

kclean:
	rm -rf $(BUILD_DIR)/kernel $(BUILD_DIR)/kernel.bin
