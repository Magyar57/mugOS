#!/bin/bash
set -e

# TODO REMOVE
TOOLCHAIN_PREFIX=/home/alex/Programmes/mugOS/toolchain/x86_64-elf

if [ "$#" -eq 0 ]; then
    echo "Usage: $0 IMAGE_NAME"
	echo "Example: $0 build_dir/disk.img"
	exit 1
fi

PARTITION_OFFSET=2048 # in sector number
IMAGE="$1"
PARTITION1="$(dirname $IMAGE)/partition1.img"

dd if=/dev/zero of=$IMAGE bs=1M count=20 status=none

# Format the disk image with fdisk
sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk $IMAGE
	o					# create an MBR partition table
	n					# new partition
	p					# primary partition
	1					# partition number 1
	$PARTITION_OFFSET	# start partition
						# default - end of disk
	a					# set bootable flag on created partition
	w					# write the partition table
	q					# quit
EOF

# Install limine
limine bios-install $IMAGE

# Install stuff needed in the main partition
dd if=$IMAGE of=$PARTITION1 bs=512 skip=$PARTITION_OFFSET status=none
mkfs.fat $PARTITION1 -F 12 -n "MUGOS"
mmd -i $PARTITION1 ::boot ::EFI ::EFI/BOOT
mcopy -i $PARTITION1 $TOOLCHAIN_PREFIX/share/limine/limine-bios.sys $TOOLCHAIN_PREFIX/share/limine/BOOTX64.EFI Bootloader/limine.conf build/kernel.elf ::boot/
mcopy -i $PARTITION1 $TOOLCHAIN_PREFIX/share/limine/BOOTX64.EFI ::EFI/BOOT
dd if=$PARTITION1 of=$IMAGE bs=512 seek=$PARTITION_OFFSET status=none

# TODO take the file copying on the partition outside of this script ?
