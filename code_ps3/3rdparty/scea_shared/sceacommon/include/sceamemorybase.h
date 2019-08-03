#ifndef _SCEAMEMORYBASE_H
#define _SCEAMEMORYBASE_H

#include "sceabasetypes.h"
#include "sceasystemids.h"
			
void *sceaAllocMem(I32 size, U32 requester_system_id, void *other_info);
void  sceaFreeMem(void *ptr);
	  
#endif
