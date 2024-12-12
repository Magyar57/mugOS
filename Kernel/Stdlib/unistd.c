#include <stdint.h>
#include "Drivers/VGA.h"

#include "unistd.h"

ssize_t write(int fd, const void* buff, size_t count){
	size_t wrote = 0;
	uint8_t* data = (uint8_t*) buff;

	if (fd == STDIN_FILENO)
		return -1; // cannot write to standard input

	// For now, we hardcode stdout and stderr to display to VGA
	if (fd == STDOUT_FILENO || fd == STDERR_FILENO){
		while ((wrote < count) && (data[wrote])){
			VGA_putchar(data[wrote]);
			wrote++;
		}
		return wrote;
	}

	// Other fd are yet unsupported
	return -2;
}
