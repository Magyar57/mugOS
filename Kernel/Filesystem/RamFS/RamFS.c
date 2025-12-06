#include "Logging.h"
#include "Filesystem/VFS.h"

#include "RamFS.h"
#define MODULE "RamFS"

struct Filesystem m_ramfs = {
	.name = "ramfs",
	.private = NULL
};

void RamFS_init(){
	VFS_registerFilesystem(&m_ramfs);
}
