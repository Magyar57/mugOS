all: build
.PHONY: build run clean

IMAGE:=disk.img

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, %.o, $(SOURCES_C))

export CPATH=$(abspath .):$(abspath EFI/Arch/$(ARCH))
CFLAGS=-g -Wall -ffreestanding -fshort-wchar -mno-red-zone -O0
LDFLAGS=-nostdlib -Wl,-entry:bmain,-subsystem:efi_application,/MAP:bootloader.map -fuse-ld=lld-link

ARCH?=x86_64

# Architecture dependent stuff
ifeq ($(ARCH), x86_64)
TARGET=x86_64-none-windows
EFI_EXEC=BOOTX64.EFI
FIRMWARE=/usr/share/edk2/x64/OVMF.4m.fd
else ifeq ($(ARCH), IA32)
TARGET=i386-none-windows
EFI_EXEC=BOOTX32.EFI
FIRMWARE=/usr/share/edk2/ia32/OVMF.4m.fd
endif

build: $(IMAGE)

$(IMAGE): $(EFI_EXEC) kernel.bin
	dd if=/dev/zero of=$@ bs=1k count=1440 status=none
	mformat -i $@ -f 1440 ::
	mmd -i $@ ::/EFI ::/EFI/BOOT
	mcopy -i $@ $(EFI_EXEC) ::/EFI/BOOT
	mcopy -i $@ *.bin ::/

$(EFI_EXEC): $(OBJECTS_C)
	clang --target=$(TARGET) $(LDFLAGS) -o $(EFI_EXEC) Main.o stdlib.o

kernel.bin: Kernel.c
	clang --target=x86_64-none-elf -g -Wall -std=c2x -ffreestanding -mno-red-zone -fPIC -fPIE -O0 -c Kernel.c -o Kernel.o
	# ld.lld -e kmain -nostdlib -pie -T Kernel.ld -Map=kernel.map --oformat binary Kernel.o -o $@ # fucked, it outputs a header for some reasons
	ld -e kmain -nostdlib -pie -pic -T Kernel.ld -Map=kernel.map --oformat binary Kernel.o -o $@

%.o: %.c
	clang --target=$(TARGET) $(CFLAGS) -c $< -o $@

run:
	qemu-system-x86_64 -drive if=pflash,file=$(FIRMWARE),format=raw,readonly=on -drive if=ide,media=disk,file=$(IMAGE),format=raw

# 32 bits:
# qemu-system-i386   -drive if=pflash,file=/usr/share/edk2/ia32/OVMF.4m.fd,format=raw,readonly=on -drive if=ide,media=disk,file=$(IMAGE),format=raw

clean:
	rm -rf *.o kernel.bin *.elf *.EFI *.img *.map
