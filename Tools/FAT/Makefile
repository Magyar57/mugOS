# Tools/FAT: FAT filesystem tools

all: fat_tools

.PHONY: all fat_tools

tools_test:
	@echo $(BUILD_DIR)

fat_tools: $(BUILD_DIR)/tools/fat/tests $(BUILD_DIR)/tools/fat/cli

# Executables

$(BUILD_DIR)/tools/fat/tests: $(BUILD_DIR)/tools/fat/Tests.o $(BUILD_DIR)/tools/fat/Fat.o | $(BUILD_DIR)/tools/fat
	gcc -g -Wall -std=c2x $^ -o $@

$(BUILD_DIR)/tools/fat/cli: $(BUILD_DIR)/tools/fat/FatCLI.o $(BUILD_DIR)/tools/fat/Fat.o | $(BUILD_DIR)/tools/fat
	gcc -g -Wall -std=c2x $^ -o $@

# Generic object rule

$(BUILD_DIR)/tools/fat/%.o: %.c | $(BUILD_DIR)/tools/fat
	gcc -g -Wall -std=c2x -c $< -o $@

# Build dir
$(BUILD_DIR)/tools/fat:
	@mkdir -p $@
