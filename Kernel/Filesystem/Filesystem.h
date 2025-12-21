#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "mugOS/List.h"

struct Node;
struct Entry;
struct File;

// ================ Functional interfaces ================

struct FilesystemFunctions {
	struct Node* (*allocNode)();
	void (*freeNode)(struct Node* node);
};

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
	void (*write)(struct File* this, void* buff, size_t count);
	void (*flush)(struct File* this);
	void (*close)(struct File* this);
};

// ================ Housekeeping structures ================

/// @brief The structure representing a filesystem in the VFS
struct Filesystem {
	const char* name;
	void* private;

	lnode_t lnode;

	struct FilesystemFunctions* funcs;
};

/// @brief A mountpoint in the VFS
struct Mount {
	// A mount is a filesystem...
	struct Filesystem* fs;
	// ... that has an entry in the VFS
	struct Entry* root;

	lnode_t lnode;
};

// ================ Filesystem components ================

/// @brief Base node on the VFS. This is your usual UNIX inode
struct Node {
	struct Permissions {
		int read : 1;
		int write : 1;
		int execute : 1;
	} perms;
	// creation date, last access date, last modification date (unimplemented)
	long creation, access, modification;
	// Pointer to the filesystem that manages this node
	struct Filesystem* fs;

	struct NodeFunctions* funcs;
};

/// @brief A directory entry in the virtual filesystem: file, directory, mount point...
struct Entry {
	struct Entry* parent;
	const char* name;
	struct Node* node;
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
