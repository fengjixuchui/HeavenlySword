/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_COLLISION_AGENT3_FILE_H
#define HK_COLLIDE2_COLLISION_AGENT3_FILE_H

#include <hkbase/hkBase.h>
typedef void hkAgentData;

struct hkAgent1nSector
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkAgent1nSector);
	
	enum { SECTOR_SIZE = 512, NET_SECTOR_SIZE = SECTOR_SIZE - 16 };

	hkUint32 m_bytesAllocated;

	hkUint32 m_pad0;
	hkUint32 m_pad1;
	hkUint32 m_pad2;
	
	char m_data[ NET_SECTOR_SIZE ];

	HK_FORCE_INLINE hkAgentData* getBegin()
	{
		return reinterpret_cast<hkAgentData*>( &m_data[0] );
	}

	hkAgentData* getEnd()
	{
		return reinterpret_cast<hkAgentData*>( &m_data[m_bytesAllocated] );
	}

	int getCapacity()
	{
		return NET_SECTOR_SIZE;
	}
	
	int getFreeBytes()
	{
		return static_cast<int>(NET_SECTOR_SIZE - m_bytesAllocated);
	}
	
	hkAgent1nSector(): m_bytesAllocated(0){}
};

struct hkAgent1nTrack
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CDINFO, hkAgent1nTrack );
	struct SectorDirEntry
	{
		hkAgent1nSector*  m_sector;
	};
	hkInplaceArray<SectorDirEntry,1> m_sectors;
};




#endif // HK_COLLIDE2_COLLISION_AGENT3_FILE_H


/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
