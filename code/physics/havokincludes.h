/***************************************************************************************************
*
*   $Header:: /game/havokincludes.h $
*
*	
*
*	CREATED
*
*	06.12.2002	John	Created
*
***************************************************************************************************/
#ifndef _HAVOK_INCLUDES_H
#define _HAVOK_INCLUDES_H

#include "config.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#if defined( PLATFORM_PC )
#	pragma comment(lib, "hkanimation.lib")
#	pragma comment(lib, "hkbase.lib")
#	pragma comment(lib, "hkcollide.lib")
#	pragma comment(lib, "hkcompat.lib")
#	pragma comment(lib, "hkcompression.lib")
#	pragma comment(lib, "hkconstraintsolver.lib")
#	pragma comment(lib, "hkdynamics.lib")
#	pragma comment(lib, "hkinternal.lib")
#	pragma comment(lib, "hkmath.lib")
#	pragma comment(lib, "hkscenedata.lib")
//#	pragma comment(lib, "hksceneexport.lib")
#	pragma comment(lib, "hkserialize.lib")
#	pragma comment(lib, "hkutilities.lib")
#	pragma comment(lib, "hkvehicle.lib")
#	pragma comment(lib, "hkvisualize.lib")
#	pragma comment(lib, "hkragdoll.lib") 
#endif

//#undef HK_DECLARE_CLASS_ALLOCATOR
#include <hkbase/hkBase.h>
//#include <hkbase/types/hkBaseTypes.h>

#endif // _PS3_RUN_WITHOUT_HAVOK_BUILD

enum  WS_SYNC_TYPE
{
	WS_SYNC_NONE,
	WS_SYNC_SOFT,
	WS_SYNC_HARD,
};

#endif // _HAVOK_INCLUDES_H

