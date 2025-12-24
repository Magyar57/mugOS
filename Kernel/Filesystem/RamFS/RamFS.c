#include "Logging.h"
#include "Filesystem/VFS.h"

#include "RamFS.h"
#define MODULE "RamFS"

static struct FilesystemFunctions m_fsFuncs = {
	.allocNode = NULL,
	.freeNode = NULL
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
	.write = NULL,
	.flush = NULL,
	.close = NULL
};

struct Filesystem m_ramfs = {
	.name = "ramfs",
	.private = NULL,

	.funcs = &m_fsFuncs
};

void RamFS_init(){
	VFS_registerFilesystem(&m_ramfs);
}
