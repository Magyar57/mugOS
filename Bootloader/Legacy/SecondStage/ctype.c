#include "stdio.h"

#include "ctype.h"

bool islower(char c){
	return c >= 'a' && c <= 'z';
}

char toupper(char c){
	return islower(c) ? (c - 'a' + 'A') : c;
}
