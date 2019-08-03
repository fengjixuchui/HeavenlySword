/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICEDEBUG_H
#define ICEDEBUG_H

#include "icemeshinternal.h"

namespace Ice
{
	namespace MeshProc
	{
		void DebugDumpVertexStream( U8* pData, U32* pFormat );
		void DebugDumpLodInfo( U16* pData );
		void DebugDumpQWords( U32* pData, U32 numQWords );
	}
}

#endif//ICEDEBUG_H
