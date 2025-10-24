# Stdlib: compiles the mugOS C standard library
# For both kernel and userspace (kmlibc & mlibc)

export CPATH=$(abspath .)

BUILD_SUBDIR:=$(BUILD_DIR)/stdlib
BUILD_K_SUBDIR:=$(BUILD_SUBDIR)/libkernel
BUILD_U_SUBDIR:=$(BUILD_SUBDIR)/libc
BUILD_K_SUBDIRS:=$(patsubst ./%, $(BUILD_K_SUBDIR)/%, $(shell find . -mindepth 1 -type d))
BUILD_U_SUBDIRS:=$(patsubst ./%, $(BUILD_U_SUBDIR)/%, $(shell find . -mindepth 1 -type d))

SOURCES_C=$(shell find . -type f -name '*.c')
OBJECTS_C_KERNEL=$(patsubst ./%, $(BUILD_K_SUBDIR)/%, $(SOURCES_C:.c=.o))
OBJECTS_C_USERSPACE=$(patsubst ./%, $(BUILD_U_SUBDIR)/%, $(SOURCES_C:.c=.o))

all: $(STDLIB_KERNEL) $(STDLIB_USERSPACE)

.PHONY: all

# Libraries files

$(STDLIB_KERNEL): $(OBJECTS_C_KERNEL)
	@echo "AR $@"
	@$(TARGET_AR) $(TARGET_ARFLAGS) $@ $^

$(STDLIB_USERSPACE_STATIC): $(OBJECTS_C_USERSPACE)
	@echo "AR $@"
	@$(TARGET_AR) $(TARGET_ARFLAGS) $@ $^

$(STDLIB_USERSPACE_DYNAMIC): $(OBJECTS_C_USERSPACE)
	@echo "LD $@"
	@$(TARGET_LD) -nostdlib -shared $^ -o $@

# Generic object rules

$(BUILD_K_SUBDIR)/%.o: %.c | $(BUILD_K_SUBDIRS)
	@echo "CC $< => $@"
	@$(TARGET_CC) $(TARGET_CFLAGS) -fPIC -DKERNEL -I../Kernel -I../Kernel/Arch/$(ARCH)/Include -c $< -o $@

$(BUILD_U_SUBDIR)/%.o: %.c | $(BUILD_U_SUBDIRS)
	@echo "CC $< => $@"
	@$(TARGET_CC) $(TARGET_CFLAGS) -fPIC -c $< -o $@

$(BUILD_K_SUBDIRS):
	@mkdir -p $@

$(BUILD_U_SUBDIRS):
	@mkdir -p $@
