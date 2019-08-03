/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_LOCALMEMORY_H
#define ICE_LOCALMEMORY_H

#include <stdio.h>
#include "icebase.h"

namespace Ice
{
    namespace BatchJob
    {
		/*!
		 * This class simulates the local memory on an SPU when running tasks on the PPU. 
		 */
		struct LocalMemory
		{
			enum { kLocalStoreSize = 256 * 1024 };
			static ICE_ALIGN(16) U8 s_memory[kLocalStoreSize];

			int m_loc;

			LocalMemory() : m_loc(0) 
			{}

			U8 *GetLocalMemory(U32 offset = 0) const { return s_memory + offset; }

			void *Allocate(U32 size)
			{
				void *retVal = &s_memory[m_loc];
				m_loc += (size + 15) & ~15;
				if (m_loc > kLocalStoreSize) ::printf("Warning: local store memory exceeded!\n");
				return retVal;
			}
		};
	}
}

#endif	//ICE_LOCALMEMORY_H

