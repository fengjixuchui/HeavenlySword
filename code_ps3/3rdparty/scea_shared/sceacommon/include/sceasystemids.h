#ifndef _SCEASYSTEMIDS_H
#define _SCEASYSTEMIDS_H

//
// These IDs are unique ID ranges for use be the various shared systems.
// They are passed to system functions (e.g. sceaAllocMem) to identify
// the requestor. Functions that are looking for these IDs have an argument
// named 'requester_id'.
//

#define SCEA_SHARED_SYSTEM_ID_AUDIO			0x80000000
#define SCEA_SHARED_SYSTEM_ID_AUDIO_LAST   	0x8FFFFFFF

	  
#endif

