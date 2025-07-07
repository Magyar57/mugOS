#include "Logging.h"

#include "SMP.h"
#define MODULE "SMP"

void SMP_init(){
	ArchSMP_init();

	log(INFO, MODULE, "Boostrap Processor is CPU#%d", SMP_getCpuId());
	log(SUCCESS, MODULE, "Initialization success, found %d CPUs/threads", g_nCPUs);
}
