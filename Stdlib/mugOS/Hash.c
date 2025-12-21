#include <stddef.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "mugOS/Hash.h"

#define HASH_MULT		257						// First prime number bigger than 2^8
#define HASH_MODULOUS	18446744073709551557UL	// First prime number smaller than 2^64

/// @brief Computes the hash of a string
/// @param str The (non-nullable) string to hash
/// @returns The hash of string s, which evaluates to `sum(i=0 to n-1) s[i] x p^i [m]`,
///          with p and m large prime numbers (p larger than the alphabet, m first prime number
///          smaller than 2^64)
uint64_t hashString(const char* str){
	uint64_t hash, mult;
	size_t n;

	if (str == NULL){
		fprintf(stderr, "String passed to hashString mustn't be NULL !");
		abort();
	}

	// With p=HASH_MULT, m=HASH_MODULOUS:
	// hash(s) = sum(i=0 to n-1) s[i] x p^i [m]

	hash = 0;
	mult = 1; // p^i
	n = strlen(str);

	for (size_t i=0 ; i<n ; i++){
		mult *= HASH_MULT;
		hash += str[i]*mult % HASH_MODULOUS;
	}

	return hash;
}
