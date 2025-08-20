# Toolchain.mk: makefile for the toolchain compilation
# Note: you need to have the build dependencies installed.
# See the docs for more informations.

# This is needed only if these variables are defined in the environment
CLEAR_ENV:=ASM= ASMFLAGS= CC= CFLAGS= CXX= LD= LDFLAGS= LINKFLAGS= LIBS=

all: toolchain
toolchain: toolchain-firmware toolchain-limine

toolchain-firmware:	| $(UEFI_FIRMWARE)
toolchain-limine:	| $(TOOLCHAIN_PATH)/bin/limine
toolchain-binutils:	| $(TOOLCHAIN_PATH)/bin/$(TARGET)-ld
toolchain-gcc:		| $(TOOLCHAIN_PATH)/bin/$(TARGET)-gcc

.PHONY: all toolchain toolchain-firmware toolchain-limine toolchain-binutils toolchain-gcc
.PHONY: clean-toolchain remove-toolchain

$(TOOLCHAIN_PATH)/bin/limine: | $(TOOLCHAIN_PATH)
	if [ ! -d "$(TOOLCHAIN_PATH)/limine" ]; then git clone https://github.com/limine-bootloader/limine.git $(TOOLCHAIN_PATH)/limine --branch=$(LIMINE_BRANCH) --depth=1; fi
	$(MAKE) -C $(TOOLCHAIN_PATH)/limine install PREFIX=$(TOOLCHAIN_PATH)

$(UEFI_FIRMWARE): | $(TOOLCHAIN_PATH)
	mkdir -p $(dir $@)
	if [ ! -d "$(TOOLCHAIN_PATH)/uefi-firmware" ]; then git clone $(OVMF_URL) $(TOOLCHAIN_PATH)/uefi-firmware; fi
	cp $(TOOLCHAIN_PATH)/uefi-firmware/OVMF_X64.fd $@
	rm -rf $(TOOLCHAIN_PATH)/uefi-firmware

$(TOOLCHAIN_PATH)/bin/$(TARGET)-ld: | $(TOOLCHAIN_PATH)
	if [ ! -f "$(TOOLCHAIN_PATH)/binutils.tar.xz" ]; then wget $(BINUTILS_URL) -O $(TOOLCHAIN_PATH)/binutils.tar.xz; fi
	mkdir -p $(TOOLCHAIN_PATH)/binutils-src
	tar -xf $(TOOLCHAIN_PATH)/binutils.tar.xz -C $(TOOLCHAIN_PATH)/binutils-src --strip-components 1
	rm -rf $(TOOLCHAIN_PATH)/binutils-build && mkdir -p $(TOOLCHAIN_PATH)/binutils-build && cd $(TOOLCHAIN_PATH)/binutils-build && \
	$(CLEAR_ENV) ../binutils-src/configure --target=$(TARGET) --prefix=$(TOOLCHAIN_PATH) --with-sysroot --disable-nls --disable-werror
	$(MAKE) -C $(TOOLCHAIN_PATH)/binutils-build
	$(MAKE) -C $(TOOLCHAIN_PATH)/binutils-build install

$(TOOLCHAIN_PATH)/bin/$(TARGET)-gcc: | $(TOOLCHAIN_PATH)
	if [ ! -f "$(TOOLCHAIN_PATH)/gcc.tar.gz" ]; then wget $(GCC_URL) -O $(TOOLCHAIN_PATH)/gcc.tar.gz; fi
	mkdir -p $(TOOLCHAIN_PATH)/gcc-src
	tar -xf $(TOOLCHAIN_PATH)/gcc.tar.gz -C $(TOOLCHAIN_PATH)/gcc-src --strip-components 1
	if [ "$(uname)" = "Darwin" ]; then cd $(TOOLCHAIN_PATH)/gcc-src && ./contrib/download_prerequisites; fi
	rm -rf $(TOOLCHAIN_PATH)/gcc-build && mkdir -p $(TOOLCHAIN_PATH)/gcc-build && cd $(TOOLCHAIN_PATH)/gcc-build && \
	$(CLEAR_ENV) ../gcc-src/configure --target=$(TARGET) --prefix=$(TOOLCHAIN_PATH) --disable-nls --enable-languages=c,c++ --without-headers
	$(MAKE) -C $(TOOLCHAIN_PATH)/gcc-build all-gcc all-target-libgcc
	$(MAKE) -C $(TOOLCHAIN_PATH)/gcc-build install-gcc install-target-libgcc

$(TOOLCHAIN_PATH):
	mkdir -p $@

#
# Clean
#
clean-toolchain:
	rm -rf $(TOOLCHAIN_PATH)/binutils.tar.xz $(TOOLCHAIN_PATH)/binutils-src $(TOOLCHAIN_PATH)/binutils-build
	rm -rf $(TOOLCHAIN_PATH)/gcc.tar.gz $(TOOLCHAIN_PATH)/gcc-src $(TOOLCHAIN_PATH)/gcc-build
	rm -rf $(TOOLCHAIN_PATH)/limine

remove-toolchain:
	rm -rf $(TOOLCHAIN_PATH)
