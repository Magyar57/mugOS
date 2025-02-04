#include <stdint.h>
#include <stddef.h>
#include "stdio.h"

#include "string.h"

const char* strchr(const char* str, char chr){
	if (str==NULL) return NULL;

	while(*str){
		if (*str == chr) return str;
		str++;
	}

	return NULL;
}

char* strcpy(char* dst, const char* src){
	if (dst == NULL) return NULL;
	if (src == NULL) {
		*dst = '\0';
		return dst;
	}

	char* dst_begin = dst;

	while(*src){
		*dst = *src;
		src++;
		dst++;
	}
	*dst = '\0'; // this was skipped by the while loop

	return dst_begin;
}

char* strncpy(char* dst, const char* src, size_t len){
	if (dst == NULL) return NULL;
	if (src == NULL) {
		*dst = '\0';
		return dst;
	}

	size_t i = 0;
	while ((i<len) && (src[i]!='\0')){
		dst[i] = src[i];
		i++;
	}
	if (i<len) dst[i] = '\0';

	return dst;
}

size_t strlen(const char* str){
	unsigned int res = 0;
	while(*str){
		str++;
		res++;
	}

	return res;
}

void* memcpy(const void* dst, const void* src, size_t size){
	// Note: the can be heavily optimized by writing it in assembly, using specific directives than take advantage of parralelization

	uint8_t* u8dst = (uint8_t*) dst;
	const uint8_t* u8src = (const uint8_t*) src;

	for(size_t i = 0 ; i<size ; i++){
		u8dst[i] = u8src[i];
	}

	return (void*) dst;
}

void* memset(void* ptr, int value, size_t size){
	uint8_t* u8ptdr = (uint8_t*) ptr;

	for(size_t i = 0 ; i<size ; i++){
		u8ptdr[i] = (uint8_t) value;
	}

	return ptr;
}

void* memmove(void* dest, const void* src, size_t n) {
	uint8_t* pdest = (uint8_t*) dest;
	const uint8_t* psrc = (const uint8_t*) src;

	if (src > dest) {
		for (size_t i = 0; i < n; i++) {
			pdest[i] = psrc[i];
		}
	} else if (src < dest) {
		for (size_t i = n; i > 0; i--) {
			pdest[i-1] = psrc[i-1];
		}
	}

	return dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t size){
	uint8_t* u8ptdr1 = (uint8_t*) ptr1;
	uint8_t* u8ptdr2 = (uint8_t*) ptr2;

	for(size_t i = 0 ; i<size ; i++){
		if (u8ptdr1[i] != u8ptdr2[i]) return 1;
	}

	return 0;
}
