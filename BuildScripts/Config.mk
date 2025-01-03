# Config.mk: configurations variables
# Note: the paths are relative to the root folder (mugOS)

# Compilation for host system
export CFLAGS:=-g -Wall -std=c2x
export ASMFLAGS:=
export CC:=gcc
export CXX:=g++
export LD:=gcc
export ASM:=nasm
export LINKFLAGS:=
export LIBS:=
export CLEAR_ENV:=CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS=

# Compilation for target system
export TARGET:=i686-elf
export TARGET_ASM:=nasm
export TARGET_ASMFLAGS:=
export TARGET_CC:=i686-elf-gcc
export TARGET_CFLAGS:=-g -Wall -std=c2x
export TARGET_CXX:=i686-elf-g++
export TARGET_CXXFLAGS:=
export TARGET_LD:=i686-elf-gcc
export TARGET_LDFLAGS:=
export TARGET_LIBS:=
# Architecture to compile mugOS for
export ARCH?=x86

export MAKE_FLAGS:=--no-print-directory

# Output folders
export TOOLCHAIN_PATH:=$(abspath toolchain)
export BUILD_DIR:=$(abspath build)
export BUILD_TOOLS_DIR:=$(BUILD_DIR)/tools
export BUILD_TOOLS_FAT_DIR:=$(BUILD_TOOLS_DIR)/fat

# Links for binutils and gcc versions to compile
# Get the latest versions: https://ftp.gnu.org/gnu/binutils/ and https://gcc.gnu.org/releases.html
BINUTILS_URL:=https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.xz
GCC_URL:=https://ftp.gwdg.de/pub/misc/gcc/releases/gcc-14.1.0/gcc-14.1.0.tar.gz
