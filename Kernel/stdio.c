#include "unistd.h"

#include "stdio.h"

int fileno(FILE* stream){
	if (stream == NULL) return -1;

	return stream->fd;
}

int fputc(int c, FILE* stream){
	int fd = fileno(stream);
	if (fd==-1) return EOF;

	char to_write = (unsigned char) c;
	ssize_t res = write(fd, &to_write, 1);

	return (res == 1) ? to_write : EOF;
}

int putc(int c, FILE* stream){
	return fputc(c, stream);
}

int putchar(int c){
	return putc(c, stdout);
}

int fputs(const char* restrict s, FILE* restrict stream){
	int res;

	while(*s){
		res = fputc(*s, stream);
		if (res == EOF) return EOF;
		s++;
	}

	return 1;
}

int puts(const char *s){
	int res_puts = fputs(s, stdout);
	int res_putc = fputc('\n', stdout);

	if (res_puts == EOF || res_putc == EOF)
		return EOF;

	return 1;
}
