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
	@ar rcs $@ $^

$(STDLIB_USERSPACE_STATIC): $(OBJECTS_C_USERSPACE)
	@echo "AR $@"
	@ar rcs $@ $^

$(STDLIB_USERSPACE_DYNAMIC): $(OBJECTS_C_USERSPACE)
	@echo "LD $@"
	@$(LD) $(U_LDFLAGS) -shared $^ -o $@

# Generic object rules

$(BUILD_K_SUBDIR)/%.o: %.c | $(BUILD_K_SUBDIRS)
	@echo "CC $< => $@"
	@$(CC) $(K_CFLAGS) -DKERNEL -I../Kernel -I../Kernel/Arch/$(ARCH)/Include -c $< -o $@

$(BUILD_U_SUBDIR)/%.o: %.c | $(BUILD_U_SUBDIRS)
	@echo "CC $< => $@"
	@$(CC) $(U_CFLAGS) -fPIC -c $< -o $@

$(BUILD_K_SUBDIRS):
	@mkdir -p $@

$(BUILD_U_SUBDIRS):
	@mkdir -p $@
