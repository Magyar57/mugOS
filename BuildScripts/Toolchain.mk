# Toolchain.mk: makefile for the toolchain compilation
# Note: you need to have the build dependencies installed.
# See the docs for more informations.

# This is needed only if these variables are defined in the environment
# Better safe than sorry, we clear everything
CLEAR_ENV:=ASM= ASMFLAGS= CC= CFLAGS= CXX= CXXFLAGS= LD= LDFLAGS= LINKFLAGS= LIBS= LDLIBS=

toolchain: toolchain-firmware toolchain-limine

toolchain-firmware:	| $(UEFI_FIRMWARE)
toolchain-limine:	| $(TOOLCHAIN_PATH)/bin/limine
toolchain-binutils:	| $(TOOLCHAIN_PATH)/bin/$(TARGET)-ld
toolchain-gcc:		| $(TOOLCHAIN_PATH)/bin/$(TARGET)-gcc

.PHONY: toolchain
.PHONY: toolchain-firmware toolchain-limine toolchain-binutils toolchain-gcc
.PHONY: clean-toolchain

$(TOOLCHAIN_PATH)/bin/limine: | $(TOOLCHAIN_PATH)
	if [ ! -d "$(TOOLCHAIN_PATH)/limine" ]; then \
		git clone $(LIMINE_URL) $(TOOLCHAIN_PATH)/limine --branch=$(LIMINE_BRANCH) --depth=1; \
	fi
	$(MAKE) -C $(TOOLCHAIN_PATH)/limine install PREFIX=$(TOOLCHAIN_PATH)
	rm -rf $(TOOLCHAIN_PATH)/limine

$(UEFI_FIRMWARE): | $(TOOLCHAIN_PATH)
	mkdir -p $(TOOLCHAIN_PATH)/share
	if [ ! -d "$(TOOLCHAIN_PATH)/edk2_ovmf.txz" ]; then \
		wget $(OVMF_URL) -O $(TOOLCHAIN_PATH)/edk2_ovmf.txz; \
	fi
	mkdir -p $(TOOLCHAIN_PATH)/edk2_ovmf
	tar -xf $(TOOLCHAIN_PATH)/edk2_ovmf.txz -C $(TOOLCHAIN_PATH)/edk2_ovmf --strip-component 1
	mv $(TOOLCHAIN_PATH)/edk2_ovmf/* $(TOOLCHAIN_PATH)/share
	rm -rf $(TOOLCHAIN_PATH)/edk2_ovmf.txz $(TOOLCHAIN_PATH)/edk2_ovmf

# Build binutils for cross-compilation
$(TOOLCHAIN_PATH)/bin/$(TARGET)-ld: | $(TOOLCHAIN_PATH)
	if [ ! -f "$(TOOLCHAIN_PATH)/binutils.tar.xz" ]; then \
		wget $(BINUTILS_URL) -O $(TOOLCHAIN_PATH)/binutils.tar.xz; \
	fi
	mkdir -p $(TOOLCHAIN_PATH)/binutils-src
	tar -xf $(TOOLCHAIN_PATH)/binutils.tar.xz -C $(TOOLCHAIN_PATH)/binutils-src --strip-components 1
	rm -rf $(TOOLCHAIN_PATH)/binutils-build && mkdir -p $(TOOLCHAIN_PATH)/binutils-build
	cd $(TOOLCHAIN_PATH)/binutils-build && \
	$(CLEAR_ENV) ../binutils-src/configure \
		--target=$(TARGET) \
		--prefix=$(TOOLCHAIN_PATH) \
		--with-sysroot \
		--disable-nls \
		--disable-werror
	$(MAKE) -C $(TOOLCHAIN_PATH)/binutils-build
	$(MAKE) -C $(TOOLCHAIN_PATH)/binutils-build install
	rm -rf $(TOOLCHAIN_PATH)/binutils*

# Build gcc for cross-compilation
$(TOOLCHAIN_PATH)/bin/$(TARGET)-gcc: | $(TOOLCHAIN_PATH)
	if [ ! -f "$(TOOLCHAIN_PATH)/gcc.tar.gz" ]; then \
		wget $(GCC_URL) -O $(TOOLCHAIN_PATH)/gcc.tar.gz; \
	fi
	mkdir -p $(TOOLCHAIN_PATH)/gcc-src
	tar -xf $(TOOLCHAIN_PATH)/gcc.tar.gz -C $(TOOLCHAIN_PATH)/gcc-src --strip-components 1
	if [ "$(uname)" = "Darwin" ]; \
		then cd $(TOOLCHAIN_PATH)/gcc-src && ./contrib/download_prerequisites; \
	fi
	rm -rf $(TOOLCHAIN_PATH)/gcc-build && mkdir -p $(TOOLCHAIN_PATH)/gcc-build
	cd $(TOOLCHAIN_PATH)/gcc-build && \
	$(CLEAR_ENV) ../gcc-src/configure \
		--target=$(TARGET) \
		--prefix=$(TOOLCHAIN_PATH) \
		--disable-nls \
		--enable-languages=c,c++ \
		--without-headers
	$(MAKE) -C $(TOOLCHAIN_PATH)/gcc-build all-gcc all-target-libgcc
	$(MAKE) -C $(TOOLCHAIN_PATH)/gcc-build install-gcc install-target-libgcc
	rm -rf $(TOOLCHAIN_PATH)/gcc.tar.gz $(TOOLCHAIN_PATH)/gcc-src $(TOOLCHAIN_PATH)/gcc-build

$(TOOLCHAIN_PATH):
	mkdir -p $@

clean-toolchain:
	rm -rf $(TOOLCHAIN_PATH)
