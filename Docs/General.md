# General documentation

This section aims to explain the general concepts needed to understand the code of the operating system.

## Booting modes

As this day, there are two booting modes possible:

Legacy booting:
- BIOS loads first sector of each bootable device into memory (at location 0x7c00)
- BIOS checks for signature (the 1st sector's 2 last bytes are expected to be 0xaa55)
- If found, it starts executing code

EFI:
- BIOS looks into special EFI partitions
- OS must be compiled as an EFI program

For simplicity reasons, we chose to work with the legacy mode.

## Filesystems

FAT12: see online documentation https://wiki.osdev.org/FAT12

## Disks organization

CHS scheme: Cylinders - Heads - Sectors
Describes how (old) disks (floppy, hard drives...) are organized
2 constants:
- number of sectors per cylinder (on a single side)
- number of heads per cylinder

LBA scheme: Logical Block Addressing
Logical abstraction to make accessing a disk easier
We address the disk with an unique sector number, a sector often being 512 bytes

LBA to CHS conversion:
sector 	= (LBA % sectors per track) + 1
head 		= (LBA / sectors per track) % heads
cylinder 	= (LBA / sectors per track) / heads
