#include "stdio.h"
#include "stdint.h"

#include "string.h"

// Find a char "chr" in the string
// @return a pointer to the chr. NULL if not found
const char* strchr(const char* str, char chr){
	if (str==NULL) return NULL;

	while(*str){
		if (*str == chr) return str;
		str++;
	}

	return NULL;
}

// Copies the string src into a destination dst
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

size_t strlen(const char* str){
	unsigned int res = 0;
	while(*str){
		str++;
		res++;
	}

	return res;
}
