#include <stdint.h>
#include "Drivers/Graphics.h"

#include "unistd.h"

ssize_t write(int fd, const void* buff, size_t count){
	size_t wrote = 0;
	uint8_t* data = (uint8_t*) buff;

	if (fd == STDIN_FILENO)
		return -1; // cannot write to standard input

	// For now, we hardcode stdout and stderr to display to the Graphics subsystem
	// Later, we'll need to those as files/streams, and make Graphics support that
	if (fd == STDOUT_FILENO || fd == STDERR_FILENO){
		while ((wrote < count) && (data[wrote])){
			Graphics_putchar(data[wrote]);
			wrote++;
		}
		return wrote;
	}

	// Other fd are yet unsupported
	return -2;
}
