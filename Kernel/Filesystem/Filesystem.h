#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "mugOS/List.h"

// ================ Functional interfaces ================

struct FilesystemFunctions {
	struct FsNode* (*allocNode)();
	void (*freeNode)(struct FsNode* node);
};

struct FsNodeFunctions {
	/// @brief Create a file (in a directory)
	void (*create)(struct FsNode* dir, const char* name);
	/// @brief Remove/delete a file (in a directory)
	void (*remove)(struct FsNode* dir);
	/// @brief Create a directory (in a directory)
	void (*createDir)(struct FsNode* dir);
	/// @brief Remove/delete a directory (in a directory)
	void (*deleteDir)(struct FsNode* dir);
};

struct FileFunctions {
	void (*open)();
	void (*read)();
	void (*write)();
	void (*close)();
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
struct FsMount {
	// A mount is a filesystem...
	struct Filesystem* fs;
	// ... that has an entry in the VFS
	struct FsEntry* root;

	lnode_t lnode;
};

// ================ Filesystem components ================

/// @brief Base node on the VFS. This is your usual UNIX inode
struct FsNode {
	struct Permissions {
		int read : 1;
		int write : 1;
		int execute : 1;
	} perms;
	// creation date, last access date, last modification date (unimplemented)
	long creation, access, modification;
	// Pointer to the filesystem that manages this node
	struct Filesystem* fs;

	struct FsNodeFunctions* funcs;
};

/// @brief A directory entry in the virtual filesystem: file, directory, mount point...
struct FsEntry {
	struct FsEntry* parent;
	const char* name;
	struct FsNode* node;
};

/// @brief A file opened by a process. There can be several opened files, that all point
/// to the same FsEntry, that itself points to the FsNode
struct FsFile {
	struct FsEntry* entry;
	// Mode in which the file was opened (read, write...)
	struct Mode {
		int read : 1;
		int write : 1;
	} mode;
	// The read/write position in the file
	long readHead;

	struct FileFunctions* funcs;
};

#endif
