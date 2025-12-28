#include "errno.h"
#include "stdlib.h"
#include "Logging.h"
#include "Filesystem/VFS.h"
#include "Filesystem/Interface.h"

#include "RamFS.h"
#define MODULE "RamFS"

static struct Node* allocNode(struct Filesystem* this);
static void freeNode(struct Filesystem* this, struct Node* node);
static int getTree(struct Filesystem* this, void* device, struct FsTree* treeOut);

static void readdir(struct File* this, struct ReaddirCallback* callback);

struct Filesystem m_ramfs = {
	.name = "RamFS",

	.allocNode = allocNode,
	.freeNode = freeNode,
	.getTree = getTree,
};

struct NodeFunctions m_nodeFuncs = {
	.create = NULL,
	.remove = NULL,
	.rename = NULL,
	.mkdir = NULL,
	.rmdir = NULL
};

struct FileFunctions m_FileFuncs = {
	.open = NULL,
	.read = NULL,
	.readdir = readdir,
	.write = NULL,
	.flush = NULL,
	.close = NULL
};

static struct Node* allocNode(struct Filesystem*){
	return NULL;
}

static void freeNode(struct Filesystem*, struct Node*){
	return;
}

static int getTree(struct Filesystem* this, void*, struct FsTree* treeOut){
	return VFS_getTreeNoDevice(this, treeOut);
}

static void readdir(struct File* this, struct ReaddirCallback* callback){
	debug("%p", this);
	callback->fill(callback);
}

void RamFS_init(){
	VFS_registerFilesystem(&m_ramfs);
}
