#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stddef.h>
typedef long ssize_t; // size_t or negative value if error (From POSIX <sys/types.h>)

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/// @brief writes string buff in file fd
/// @param fd file descriptor to writeto
/// @param buff string to write
/// @param count size of buff
/// @returns number of bytes written (-1 on error)
ssize_t write(int fd, const void* buff, size_t count);

#endif
