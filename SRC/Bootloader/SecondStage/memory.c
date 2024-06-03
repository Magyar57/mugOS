#include "stdint.h"
#include "stdio.h"

void far* memcpy(const void far* dst, const void far* src, size_t size){	
	// Note: the can be heavily optimized by writing it in assembly, using specific directives than take advantage of parralelization
	
	uint8_t far* u8dst = (uint8_t far*) dst;
	const uint8_t far* u8src = (const uint8_t far*) src;

	for(size_t i = 0 ; i<size ; i++){
		u8dst[i] = u8src[i];
	}

	return (void far*) dst;
}

void far* memset(void far* ptr, int value, size_t size){
	uint8_t far* u8ptdr = (uint8_t far*) ptr;

	for(size_t i = 0 ; i<size ; i++){
		u8ptdr[i] = (uint8_t) value;
	}

	return ptr;
}

int memcmp(const void far* ptr1, const void far* ptr2, size_t size){
	uint8_t far* u8ptdr1 = (uint8_t far*) ptr1;
	uint8_t far* u8ptdr2 = (uint8_t far*) ptr2;

	for(size_t i = 0 ; i<size ; i++){
		if (u8ptdr1[i] != u8ptdr2[i]) return 1;
	}

	return 0;
}
