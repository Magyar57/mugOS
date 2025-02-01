# Toolchain.mk: makefile for the toolchain compilation
# Note: you need to have the build dependencies installed. See the README for more informations.

TOOLCHAIN_PREFIX=$(TARGET_TOOLCHAIN)

all: toolchain
toolchain: limine toolchain_binutils toolchain_gcc
limine:				| $(TOOLCHAIN_PREFIX)/bin/limine
toolchain_binutils:	| $(TOOLCHAIN_PREFIX)/bin/$(TARGET)-ld
toolchain_gcc:		| $(TOOLCHAIN_PREFIX)/bin/$(TARGET)-gcc

.PHONY: toolchain limine clean-toolchain toolchain_binutils toolchain_gcc clean-toolchain remove-toolchain

$(TOOLCHAIN_PREFIX)/bin/limine:
	if [ ! -d "$(TOOLCHAIN_PATH)/limine" ]; then git clone https://github.com/limine-bootloader/limine.git $(TOOLCHAIN_PATH)/limine --branch=v8.x-binary --depth=1; fi
	$(MAKE) -C $(TOOLCHAIN_PATH)/limine install PREFIX=$(TOOLCHAIN_PREFIX)

$(TOOLCHAIN_PREFIX)/bin/$(TARGET)-ld: | $(TOOLCHAIN_PATH)
	if [ ! -f "$(TOOLCHAIN_PATH)/binutils.tar.xz" ]; then wget $(BINUTILS_URL) -O $(TOOLCHAIN_PATH)/binutils.tar.xz; fi
	mkdir -p $(TOOLCHAIN_PATH)/binutils-src
	tar -xf $(TOOLCHAIN_PATH)/binutils.tar.xz -C $(TOOLCHAIN_PATH)/binutils-src --strip-components 1
	rm -rf $(TOOLCHAIN_PATH)/binutils-build && mkdir -p $(TOOLCHAIN_PATH)/binutils-build && cd $(TOOLCHAIN_PATH)/binutils-build && \
	$(CLEAR_ENV) ../binutils-src/configure --target=$(TARGET) --prefix=$(TOOLCHAIN_PREFIX) --with-sysroot --disable-nls --disable-werror
	$(MAKE) -C $(TOOLCHAIN_PATH)/binutils-build
	$(MAKE) -C $(TOOLCHAIN_PATH)/binutils-build install

$(TOOLCHAIN_PREFIX)/bin/$(TARGET)-gcc: | $(TOOLCHAIN_PATH)
	if [ ! -f "$(TOOLCHAIN_PATH)/gcc.tar.gz" ]; then wget $(GCC_URL) -O $(TOOLCHAIN_PATH)/gcc.tar.gz; fi
	mkdir -p $(TOOLCHAIN_PATH)/gcc-src
	tar -xf $(TOOLCHAIN_PATH)/gcc.tar.gz -C $(TOOLCHAIN_PATH)/gcc-src --strip-components 1
	rm -rf $(TOOLCHAIN_PATH)/gcc-build && mkdir -p $(TOOLCHAIN_PATH)/gcc-build && cd $(TOOLCHAIN_PATH)/gcc-build && \
	$(CLEAR_ENV) ../gcc-src/configure --target=$(TARGET) --prefix=$(TOOLCHAIN_PREFIX) --disable-nls --enable-languages=c,c++ --without-headers
	$(MAKE) -C $(TOOLCHAIN_PATH)/gcc-build all-gcc
	$(MAKE) -C $(TOOLCHAIN_PATH)/gcc-build all-target-libgcc
	$(MAKE) -C $(TOOLCHAIN_PATH)/gcc-build install-gcc
	$(MAKE) -C $(TOOLCHAIN_PATH)/gcc-build install-target-libgcc

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
