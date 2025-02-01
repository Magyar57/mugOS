#include "unistd.h"
#include "string.h"

#include "stdio.h"

int fileno(FILE* stream){
	if (stream == NULL) return -1;

	return stream->fd;
}

int fputc(int c, FILE* stream){
	int fd = fileno(stream);
	if (fd==-1) return EOF;

	char to_write = (unsigned char) c;
	ssize_t written = write(fd, &to_write, 1);

	if (written != 1) return EOF;
	return to_write;
}

int putc(int c, FILE* stream){
	return fputc(c, stream);
}

int putchar(int c){
	return fputc(c, stdout);
}

int fputs(const char* restrict s, FILE* restrict stream){
	if (s==NULL) return EOF;
	int fd = fileno(stream);
	if (fd==-1) return EOF;

	size_t size = strlen(s);
	ssize_t written = write(fd, s, size);

	if (written != size) return EOF;
	return written;
}

int puts(const char *s){
	int res = fputs(s, stdout);
	if (res == EOF) return EOF;

	res = fputc('\n', stdout);
	if (res == EOF) return EOF;

	return 1;
}
