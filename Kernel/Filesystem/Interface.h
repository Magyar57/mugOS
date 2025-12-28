#ifndef __FS_INTERFACE_H__
#define __FS_INTERFACE_H__

#include <stddef.h>
#include "Filesystem/Filesystem.h"

// Interface.h: contracts and API between the VFS and filesystem drivers

// ================ Callbacks & function parameters ================

// Filesystem tree, used in getTree
struct FsTree {
	struct FilesystemInstance* instance;
	struct Entry* rootEntry;
	struct Node* rootNode;
};

struct ReaddirCallback {
	void (*fill)(struct ReaddirCallback* data);
	void* buff;
	int count;
	int error;
};

// ================ Function containers ================

struct NodeFunctions {
	/// @brief Create a file (in a directory)
	void (*create)(struct Node* dir, const char* name);
	/// @brief Remove/delete a file (in a directory)
	void (*remove)(struct Node* dir);
	/// @brief Rename a file
	void (*rename)(struct Node* dir, const char* name);
	/// @brief Create a directory (in a directory)
	void (*mkdir)(struct Node* dir, const char* name);
	/// @brief Remove/delete a directory (in a directory)
	void (*rmdir)(struct Node* dir);
};

struct FileFunctions {
	void (*open)(struct File* this, int flags);
	void (*read)(struct File* this, void* buff, size_t count);
	void (*readdir)(struct File* this, struct ReaddirCallback* callback);
	void (*write)(struct File* this, void* buff, size_t count);
	void (*flush)(struct File* this);
	void (*close)(struct File* this);
};

// ================ Drivers API ================

/// @brief Generic getTree function for filesystem that are not dependent on a device.
///        This helper is to be used in the Filesystem::getTree method
/// @param fs Filesystem to fill the tree for (used to initialize a filesystem instance)
/// @param treeOut The tree parameters to fill
/// @return 0 on success, or a negative errno on error
int VFS_getTreeNoDevice(struct Filesystem* fs, struct FsTree* treeOut);

/// @brief Find, or otherwise allocate and initialize, a new filesystem instance
/// @param fs The filesystem the instance should be attached to
/// @param device The nullable 'device' parameter to be passed to the test function
/// @param test A nullable `bool test(struct Filesystem*, struct FilesystemInstance*, void*)`
///        function that returns whether the instance (2nd argument) corresponds to the one looked
///        for by the filesystem, with an optional 'device' (3rd argument)
/// @return Allocated instance, or NULL on ENOMEM error
struct FilesystemInstance* VFS_getFsInstance(struct Filesystem* fs,  void* device,
	bool (*test)(struct Filesystem*, struct FilesystemInstance*, void*));

/// @brief Delete a filesystem instance that was acquired from VFS_getFsInstance or any VFS_getTree*
/// @param instance The instance to free
/// @note This function is meant to be called during mounting. If a mount was attached to this
///       instance, detach it BEFORE calling this function !
void VFS_deleteFsInstance(struct FilesystemInstance* instance);

#endif
