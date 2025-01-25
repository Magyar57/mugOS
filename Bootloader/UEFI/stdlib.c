#include <stdint.h>
#include "EFI/EFI.h"

#include "stdlib.h"

extern EFI_SYSTEM_TABLE* g_st;
extern EFI_BOOT_SERVICES* g_bs;
extern EFI_RUNTIME_SERVICES* g_rs;

// string.h

int memcmp(const void* ptr1, const void* ptr2, size_t size){
	uint8_t* u8ptdr1 = (uint8_t*) ptr1;
	uint8_t* u8ptdr2 = (uint8_t*) ptr2;

	for(size_t i = 0 ; i<size ; i++){
		if (u8ptdr1[i] != u8ptdr2[i]) return 1;
	}

	return 0;
}

size_t strlen(const char* str){
	unsigned int res = 0;
	while(*str){
		str++;
		res++;
	}

	return res;
}

// stdio.h

void utoa(unsigned long long number, char* out, size_t n, uint8_t radix){
	size_t i = 0; // pos in buffer
	char buffer[128];
	const char* symbols = "0123456789abcdef";

	// unsigned int workNumber = (unsigned int) number; // for IA32, to avoid undefined symbols __aullrem and __aulldiv

	do {
		unsigned long long rem = number % radix;
		number /= radix;

		buffer[i++] = symbols[rem];
	} while(number > 0 && i<sizeof(buffer));

	// We put i characters in the buffer, in reverse order
	// Now reverse the string back

	const int written = i;
	int j = 0;
	while (j<written && j<n) {
		out[j] = buffer[i-1];
		i--;
		j++;
	}
}

void itoa(long long input, char* out, size_t n, uint8_t radix){
	if (input<0){
		input = -input;
		out[0] = '-';
		out++;
		n--;
	}

	utoa((unsigned long long) input, out, n, radix);
}

inline EFI_STATUS puts_noCRLF(CHAR16* string){
	return g_st->ConOut->OutputString(g_st->ConOut, string);
}

inline EFI_STATUS puts(CHAR16* string){
	EFI_STATUS res;
	res = puts_noCRLF(string);
	if (EFI_ERROR(res)) return res;

	return g_st->ConOut->OutputString(g_st->ConOut, L"\r\n");
}

void putNumber_noCRLF(long long number, uint8_t radix){
	char buff[128], buffUnicode[2*sizeof(buff)];
	for(int i=0 ; i<32 ; i++) buff[i] = '\0';

	itoa(number, buff, sizeof(buff), radix);

	// Now convert buff into buffUnicode
	// CHAR16 string:
	// 48 00 65 00 6c 00 6c 00 [...] 00 00
	//  H  .  e  .  l  .  l  . [...]  .  .
	for(int i=0 ; i<sizeof(buff) ; i++){
		buffUnicode[2*i] = buff[i];
		buffUnicode[2*i + 1] = '\0';
	}

	if (radix == 16) g_st->ConOut->OutputString(g_st->ConOut, L"0x");
	puts_noCRLF((CHAR16*) buffUnicode);
}

void putNumber(long long number, uint8_t radix){
	if (radix == 16)
		putNumberUnsigned_noCRLF(number, radix);
	else
		putNumber_noCRLF(number, radix);
	puts(L"");
}

void putNumberUnsigned_noCRLF(unsigned long long number, uint8_t radix){
	char buff[128], buffUnicode[2*sizeof(buff)];
	for(int i=0 ; i<32 ; i++) buff[i] = '\0';

	utoa(number, buff, sizeof(buff), radix);

	for(int i=0 ; i<sizeof(buff) ; i++){
		buffUnicode[2*i] = buff[i];
		buffUnicode[2*i + 1] = '\0';
	}

	if (radix == 16) g_st->ConOut->OutputString(g_st->ConOut, L"0x");
	puts_noCRLF((CHAR16*) buffUnicode);
}

void putNumberUnsigned(unsigned long long number, uint8_t radix){
	putNumberUnsigned_noCRLF(number, radix);
	puts(L"");
}
