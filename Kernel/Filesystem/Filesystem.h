#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "mugOS/List.h"

// Declared in Interface.h
struct FsTree;
struct NodeFunctions;
struct FileFunctions;

// ================ Housekeeping structures ================

// Here's an example of how several disks could be mounted serveral times:
// - Filesystem driver (FAT12)
//   - FilesystemInstance (disk1)
//     - Mount on /mnt/disk1
//     - Mount on /wherever/disk1
//   - FilesystemInstance (disk2)
//     - Mount on /mnt/disk2
//     - Mount on /somewhere_else/disk2

/// @brief The structure representing a filesystem driver registered to the VFS
struct Filesystem {
	const char* name;

	struct Node* (*allocNode)(struct Filesystem* this);
	void (*freeNode)(struct Filesystem* this, struct Node* node);
	int (*getTree)(struct Filesystem* this, void* device, struct FsTree* treeOut);

	lnode_t lnode;
	list_t instances;
};

/// @brief A filesystem instance representing a specific [virtual] device on the VFS
struct FilesystemInstance {
	struct Filesystem* fs;
	void* dev;

	lnode_t lnode;
	list_t mounts;
};

/// @brief A mountpoint in the VFS. A mount is a filesystem instance (any [virtual] disk) that
///        is accessible from an entry in its base tree.
struct Mount {
	// What device (and file tree) this mount represents
	struct FilesystemInstance* instance;
	// The root entry of the device
	struct Entry* root;
	// The mount point, aka what entry in the parent tree this mount overrides
	struct Entry* mountPoint;

	lnode_t lnode;
};

// ================ Internal structs ================

/// @brief A string with cached info for accelerated computations
struct QuickString {
	const char* str;
	size_t len;
	unsigned long hash;
};

// ================ Filesystem components ================

struct NodeFlags {
	int isDirectory : 1;
	// rwx
	int read : 1;
	int write : 1;
	int execute : 1;
};

struct EntryFlags {
	int isMounted : 1;
};

/// @brief Base node on the VFS. This is your usual UNIX inode
struct Node {
	struct NodeFlags flags;
	// File size in bytes
	size_t size;
	// Creation date, last access date, last modification date (unimplemented)
	long creation, access, modification;
	// Pointer to the filesystem that manages this node
	struct Filesystem* fs;

	struct NodeFunctions* funcs;
};

/// @brief A directory entry in the virtual filesystem: file, directory, mount point...
struct Entry {
	struct QuickString name;
	struct EntryFlags flags;

	struct Node* node;
	struct Entry* parent;
	list_t children;
	lnode_t sibling;
};

/// @brief A file opened by a process. There can be several opened files, that all point
/// to the same Entry, that itself points to the Node
struct File {
	struct Entry* entry;
	// Mode in which the file was opened (read, write...)
	struct Mode {
		int write : 1;
	} mode;
	// The read/write position in the file
	long readHead;

	struct FileFunctions* funcs;
};

#endif
