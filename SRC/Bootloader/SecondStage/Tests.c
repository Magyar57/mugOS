#include "Tests.h"

void test_printf(){
	printf("Testing printf:\r\n");
	printf("%% %c %s ", 'a', "my_string");
	printf("%d %i %x %p %o ", 1234, -5678, 0x7fff, 0xbeef, 012345);
	printf("%hd %hi %hhu %hhd\r\n", (short)57, (short)-42, (unsigned char) 20, (char)-10);
	printf("%ld %lx %lld %llx\r\n\n", -100000000l, 0x7ffffffful, 10200300400ll, 0xeeeeaaaa7777ffffull);
}
