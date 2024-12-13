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
	if (s==NULL) return EOF;

	while(*s){
		int res = fputc(*s, stream);
		if (res == EOF) return EOF;
		s++;
	}

	return 1;
}

int puts(const char *s){
	int res = fputs(s, stdout);
	if (res == EOF) return EOF;

	res = fputc('\n', stdout);
	if (res == EOF) return EOF;

	return 1;
}
