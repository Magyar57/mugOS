#ifndef __ARCH_IO_H__
#define __ARCH_IO_H__

#define MMIO_defineRead(name, size, type, reg, barrier) \
static inline type name(const volatile void *addr) { \
	type res; \
	__asm__ volatile("mov" size " %1, %0" \
		: reg (res) \
		:"m" (*(volatile type*)addr) barrier); \
	return res; \
}

#define MMIO_defineWrite(name, size, type, reg, barrier) \
static inline void name(type val, volatile void *addr) { \
	__asm__ volatile("mov" size " %0,%1" \
		: \
		: reg (val), "m" (*(volatile type*)addr) barrier); \
}

MMIO_defineRead(read8,  "b", unsigned char,  "=q", :"memory")
MMIO_defineRead(read16, "w", unsigned short, "=r", :"memory")
MMIO_defineRead(read32, "l", unsigned int,   "=r", :"memory")
MMIO_defineRead(read64, "q", unsigned long,  "=r", :"memory")

MMIO_defineWrite(write8,  "b", unsigned char,  "q", :"memory")
MMIO_defineWrite(write16, "w", unsigned short, "r", :"memory")
MMIO_defineWrite(write32, "l", unsigned int,   "r", :"memory")
MMIO_defineWrite(write64, "q", unsigned long,  "r", :"memory")

// inb and outb are in IO.asm

#endif
