#include "errno.h"
#include "assert.h"
#include "stdlib.h"
#include "string.h"
#include "Panic.h"
#include "mugOS/List.h"
#include "Logging.h"
#include "Filesystem/RamFS/RamFS.h"

#include "VFS.h"
#define MODULE "VFS"

static struct Node m_rootNode = {
	.fs = NULL
};

static struct Entry m_rootEntry = {
	.parent = &m_rootEntry,
	.name = "/",
	.node = &m_rootNode,
};

static cache_t* m_entriesCache;
static cache_t* m_filesCache;
static cache_t* m_mountsCache;

static list_t m_entries = LIST_STATIC_INIT(m_entries);
static list_t m_mounts = LIST_STATIC_INIT(m_mounts);
static list_t m_filesystems = LIST_STATIC_INIT(m_filesystems);

static struct Entry* subdir(struct Entry* entry, const char* path){
	if (strchr(path, '/') == NULL)
		return NULL;

	if (strlen(path) < 1)
		return NULL;

	// entry->name = path; // lol, temp. we should search the entry instead (:
	return entry;
}

static const char* nextNonSeparator(const char* path){
	while(*path == '/'){
		path++;
	}

	if (*path == '\0')
		return NULL;

	return path;
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
	while(next != NULL){
		entry_name = nextNonSeparator(entry_name); // skip leading slash(es)
		cur = next;
		next = subdir(cur, entry_name);
		if (next == NULL && parent)
			return cur;
		entry_name = strchr(entry_name, '/');
	}

	return cur;
}

// ================ Public API ================

void VFS_init(){
	m_entriesCache = Cache_create("fs-entries", sizeof(struct Entry), NULL);
	m_mountsCache = Cache_create("mountpoints", sizeof(struct Mount), NULL);
	m_filesCache = Cache_create("files", sizeof(struct File), NULL);
	if (m_entriesCache == NULL || m_mountsCache == NULL){
		log(PANIC, MODULE, "Could not initialize caches allocators");
		panic();
	}

	// Initialize the filesystems
	RamFS_init();

	// Mount everything
	// root "/"
	extern struct Filesystem m_ramfs;
	VFS_mount(&m_ramfs, "/");
	// - Boot/EFI partition "/boot"
	// VFS_mount(NULL, "/boot");

	log(SUCCESS, MODULE, "Virtual filesystem initialized");
}

void VFS_registerFilesystem(struct Filesystem* fs){
	List_pushBack(&m_filesystems, &fs->lnode);
	log(INFO, MODULE, "Registered '%s' filesystem", fs->name);
}

int VFS_mount(struct Filesystem* fs, const char* path){
	struct Mount* mount;
	struct Entry* mountRoot;

	mountRoot = resolve(path, false);
	if (mountRoot == NULL)
		return E_NOENT;

	mount = Cache_malloc(m_mountsCache);
	if (mount == NULL)
		return E_NOMEM;

	mount->root = Cache_malloc(m_entriesCache);
	if (mount->root == NULL){
		Cache_free(m_mountsCache, mount);
		return E_NOMEM;
	}

	mount->fs = fs;
	mount->root = mountRoot;
	List_pushBack(&m_mounts, &mount->lnode);

	return E_SUCCESS;
}

int VFS_stat(const char* path, struct Stat* statBuff);
ssize_t VFS_getDirEntries(struct File* dir, void* entriesBuff, size_t count);
void VFS_close(struct File* node);
ssize_t VFS_read(struct File* f, void* buff, size_t count);
ssize_t VFS_write(struct File* f, void* buff, size_t count);
int VFS_rename(const char* oldpath, const char* newpath);
void VFS_mkdir(const char* path);
void VFS_rmdir(const char* path);
