#ifndef __VFS_H__
#define __VFS_H__

#include <stddef.h>
#include <sys/types.h>
#include "Filesystem/Filesystem.h"

#define O_READ		0x00
#define O_WRITE		0x01
#define O_RDWR		(O_READ|O_WRITE)
#define O_WRONLY	O_WRITE
#define O_APPEND	0
#define O_CREAT		0
#define O_TRUNC		0

struct stat {
	size_t size;
};

/// @brief Initialize the virtual filesystem
void VFS_init();

/// @brief Interface for the filesystem drivers to register themselves
void VFS_registerFilesystem(struct Filesystem* fs);

/// @brief Mount a filesystem onto the virtual filesystem
/// @return A mugOS errno value (0 on success E_* on error)
int VFS_mount(struct Filesystem* fs, const char* path);

/// @brief Read the stats of a file or directory
/// @param path of the file to read
/// @param statbuff output buffer where statistics are written
/// @return 0 on success, -1 on error
int VFS_stat(const char* path, struct stat* statbuff);

/// @brief Open a file in the VFS
/// @param path The path to the file to open
/// @param flags Opening flags, e.g. `O_READ|O_WRITE|O_CREAT`
/// @return The opened file on success, NULL on error (e.g. if path points to a directory)
struct File* VFS_open(const char* path, int flags);

/// @brief Close a previously opened file
/// @param node The file to close (nullable)
void VFS_close(struct File* node);

/// @brief Read count bytes from the opened file f into buff
/// @param f File to read
/// @param buff Buffer where to write the data read
/// @param count How many bytes to read
/// @return The number of bytes read, or -1 on error. If return value is less than count, it means
///         no more data is available (e.g. end of file)
ssize_t VFS_read(struct File* f, void* buff, size_t count);

/// @brief Write to a file
/// @param f File to write to
/// @param buff Data to write into the file
/// @param count Number of bytes from buff to write
/// @return The number of bytes written, or -1 on error
ssize_t VFS_write(struct File* f, void* buff, size_t count);

/// @brief Rename a file (including moving directory if needed)
/// @param oldpath Current file path
/// @param newpath Future file path, where to move the file
/// @return
int VFS_rename(const char* oldpath, const char* newpath);

/// @brief Make directory
/// @param path Path to the directory to create
void VFS_mkdir(const char* path);

/// @brief Remove directory
/// @param path Path to the directory to remove
void VFS_rmdir(const char* path);

#endif
