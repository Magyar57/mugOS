#ifndef __VFS_H__
#define __VFS_H__

#include "Filesystem/Filesystem.h"

/// @brief Initialize the virtual filesystem
void VFS_init();

/// @brief Interface for the filesystem drivers to register themselves
void VFS_registerFilesystem(struct Filesystem* fs);

/// @brief Mount a filesystem onto the virtual filesystem
/// @return A mugOS errno value (0 on success E_* on error)
int VFS_mount(struct Filesystem* fs, const char* path);

struct FsNode* VFS_open(const char* path, int mode);

void VFS_close(struct FsNode* node);

#endif
