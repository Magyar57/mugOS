#include "errno.h"
#include "assert.h"
#include "stdlib.h"
#include "string.h"
#include "Panic.h"
#include "mugOS/List.h"
#include "mugOS/Hash.h"
#include "Logging.h"
#include "Filesystem/Interface.h"
#include "Filesystem/RamFS/RamFS.h"

#include "VFS.h"
#define MODULE "VFS"

static struct Node m_rootNode = {
	.flags = {
		.isDirectory = true,
		.read = true,
		.write = true,
		.execute = true
	},
	.size = 0,
	.creation = 0,
	.access = 0,
	.modification = 0,
	.fs = NULL,
	.funcs = NULL,
};

static struct Entry m_rootEntry = {
	.parent = &m_rootEntry,
	.name = {
		.str = "/",
		.len = sizeof("/"),
		.hash = 0x0 // Computed at runtime
	},
	.node = &m_rootNode,
};

static cache_t* m_instancesCache;
static cache_t* m_entriesCache;
static cache_t* m_filesCache;
static cache_t* m_mountsCache;

static list_t m_filesystems = LIST_STATIC_INIT(m_filesystems);

// ================ Path manipulations as strings ================

static const char* nextNonSeparator(const char* path){
	while (*path == '/'){
		path++;
	}

	if (*path == '\0')
		return NULL;

	return path;
}

/// @return The first subdirectory of the path, e.g. `/i/love/manatees` => `love/manatees`
static const char* subdir(const char* path){
	while (*path == '/'){
		path++;
	}

	while (*path != '/' && *path != '\0'){
		path++;
	}

	while (*path == '/'){
		path++;
	}

	return path;
}

/// @return The basename of the given path. E.g. `/i/love/manatees` => `manatees`
static const char* basename(const char* path){
	const char* cur;
	const char* next;

	next = path;
	do {
		cur = next;
		next = subdir(cur);
	} while (*next != '\0');

	return cur;
}

// ================ Helpers ================

static struct Entry* findSubEntry(struct Entry* entry, const char* name){
	if (strchr(name, '/') == NULL)
		return NULL;

	if (strlen(name) < 1)
		return NULL;

	// entry->name = name; // lol, temp. we should search the entry instead (:
	return entry;
}

/// @brief Resolve a path in the VFS
/// @param path The path to resolve (must be absolute)
/// @param parent Whether to resolve the path's parent directory (instead of the actual path)
/// @return The Entry corresponding to the path, or NULL if it does not resolve
static struct Entry* resolve(const char* path, bool parent){
	struct Entry *cur, *next;
	const char* entry_name;

	// Handle foward slash
	entry_name = strchr(path, '/');
	if (entry_name != path) {
		log(ERROR, MODULE, "Cannot resolve non-absolute path '%s'", path);
		return NULL;
	}

	next = &m_rootEntry;
	while (next != NULL){
		entry_name = nextNonSeparator(entry_name); // skip leading slash(es)
		cur = next;
		next = findSubEntry(cur, entry_name);
		if (next == NULL && parent)
			return cur;
		entry_name = strchr(entry_name, '/');
	}

	return cur;
}

/// @brief Find a registered filesystem, based on its name
/// @param fsName The name (identifier) of the filesystem to mount
/// @return The filesystem, or NULL if no fs is registered with this name
static struct Filesystem* resolveFilesystem(const char* fsName){
	lnode_t* node;
	struct Filesystem* fs;

	List_foreach(&m_filesystems, node){
		fs = List_getObject(node, struct Filesystem, lnode);
		if (strncmp(fs->name, fsName, strlen(fs->name)) == 0)
			return fs;
	}

	return NULL;
}

static struct FilesystemInstance* newFsInstance(struct Filesystem* fs){
	struct FilesystemInstance* instance;

	instance = Cache_malloc(m_instancesCache);
	if (instance == NULL)
		return NULL;

	// Init instance
	instance->fs = fs;
	List_init(&instance->mounts);

	// Update filesystem
	List_pushBack(&fs->instances, &instance->lnode);

	return instance;
}

// ================ Drivers API ================

int VFS_getTreeNoDevice(struct Filesystem* fs, struct FsTree* treeOut){
	struct FilesystemInstance* instance;

	instance = newFsInstance(fs);
	if (instance == NULL)
		return -ENOMEM;

	instance->dev = NULL; // no device to be associated with

	treeOut->instance = instance;

	// TODO using fill_sb callback:
	// 	Inode Allocation: The driver calls iget_locked(sb, root_ino).
	//     Allocation: This allocates a struct inode from the inode_cache slab.
	//     Initialization: The driver fills inode->i_op (inode operations) and inode->i_fop (file operations).
	// Dentry Allocation: The driver calls d_make_root(root_inode).
	//     Allocation: This allocates a struct dentry from the dentry_cache slab.
	//     Linking: It sets dentry->d_inode = root_inode and initializes the dentry name as "/".
	//     Superblock Link: It sets sb->s_root = dentry.
	treeOut->rootEntry = NULL;
	treeOut->rootNode = NULL;

	return 0;
}

struct FilesystemInstance* VFS_getFsInstance(struct Filesystem* fs,  void* device,
	bool (*test)(struct Filesystem*, struct FilesystemInstance*, void*)){
	struct FilesystemInstance* instance;
	lnode_t* node;

	// If we got a test function, try it
	if (test != NULL){
		List_foreach(&fs->instances, node){
			instance = List_getObject(node, struct FilesystemInstance, lnode);
			if (test(fs, instance, device))
				return instance;
		}
	}

	// If the test function yielded nothing or wasn't provided, allocate
	return newFsInstance(fs);
}

void VFS_deleteFsInstance(struct FilesystemInstance* instance){
	if (!List_isEmpty(&instance->mounts))
		return;

	List_pop(&instance->fs->instances, &instance->lnode);
	Cache_free(m_instancesCache, instance);
}

// ================ Public API ================

void VFS_init(){
	m_instancesCache = Cache_create("fs-instances", sizeof(struct FilesystemInstance), NULL);
	m_entriesCache = Cache_create("fs-entries", sizeof(struct Entry), NULL);
	m_mountsCache = Cache_create("fs-mounts", sizeof(struct Mount), NULL);
	m_filesCache = Cache_create("fs-files", sizeof(struct File), NULL);
	if (m_instancesCache == NULL || m_entriesCache == NULL ||
		m_mountsCache == NULL || m_filesCache == NULL){
		log(PANIC, MODULE, "Could not initialize caches allocators");
		panic();
	}

	// Initialize the filesystems
	RamFS_init();

	// Initialize "/" node
	m_rootEntry.name.hash = hashString(m_rootEntry.name.str);

	// Mount everything
	VFS_mount(NULL, "/", "RamFS");
	// VFS_mount(NULL, "/boot", "FAT12"); // EFI partition

	log(SUCCESS, MODULE, "Virtual filesystem initialized");
}

void VFS_registerFilesystem(struct Filesystem* fs){
	List_init(&fs->instances);
	List_pushBack(&m_filesystems, &fs->lnode);

	log(INFO, MODULE, "Registered '%s' filesystem", fs->name);
}

int VFS_mount(void* device, const char* path, const char* fsName){
	int err;
	struct Filesystem* fs;
	struct Mount* mnt;
	struct Entry* mount_point;
	struct FsTree tree;

	// Resolve the mount point
	mount_point = resolve(path, false);
	if (mount_point == NULL)
		return -ENOENT;

	// Search the filesystem in the list
	fs = resolveFilesystem(fsName);
	if (fs == NULL)
		return -ENODEV;

	// Allocate the mount
	mnt = Cache_malloc(m_mountsCache);
	if (mnt == NULL)
		return -ENOMEM;

	// Get/init the tree: the filesystem instance, the root Entry, and the root Node
	err = fs->getTree(fs, device, &tree);
	if (err){
		Cache_free(m_mountsCache, mnt);
		return err;
	}

	// TODO to be thread-safe: lock the mount & verify that the mount point is still valid
	// TODO add the Mount to the parent's list of children
	// TODO mark the mount point's Entry as mounted-upon

	// Finally, initialize everything
	mnt->instance = tree.instance;
	mnt->mountPoint = mount_point;
	List_pushBack(&tree.instance->mounts, &mnt->lnode);

	return 0;
}

int VFS_stat(const char* path, struct Stat* statBuff);

ssize_t VFS_getDirEntries(struct File* dir, void* entriesBuff, size_t count){
	if (dir==NULL || entriesBuff==NULL || count<sizeof(struct DirEntry))
		return -EINVAL;
	if (!dir->entry->node->flags.isDirectory)
		return -ENOTDIR;

	// Call the filesystem directory iteration method
	// dir->funcs->readdir(dir); // TODO

	return 0;
}

struct File* VFS_open(const char* path, int flags){
	struct Entry* f = resolve(path, false);
	if (f == NULL)
		return NULL; // ENOENT

	// TODO we should call the specific filesystem instead
	struct File* file = Cache_malloc(m_filesCache);
	if (file == NULL)
		return NULL; // ENOMEM

	file->entry = f;
	file->mode.write = (flags & O_WRITE);
	file->readHead = 0; // seek the beginning of the file
	file->funcs = NULL; // TODO ?
	// file->funcs->open(file->funcs, flags); // TODO

	return file;
}

void VFS_close(struct File* node);
ssize_t VFS_read(struct File* f, void* buff, size_t count);
ssize_t VFS_write(struct File* f, void* buff, size_t count);
int VFS_rename(const char* oldpath, const char* newpath);
void VFS_mkdir(const char* path);
void VFS_rmdir(const char* path);
