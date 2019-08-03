/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_COLLISION_AGENT_TRACK_H
#define HK_COLLIDE2_COLLISION_AGENT_TRACK_H

#include <hkbase/hkBase.h>
#include <hkinternal/collide/agent3/hkAgent3.h>
typedef void hkAgentData;
struct hkAgentNnEntry;
struct hkAgentNnSector; 
class hkCollidable;

	// Make sure if you use this entry that you pad to get a 16 byte alignment
	// Notes: 
	//
	//   HK_POINTER_SIZE == 4
	//   --------------------
	//   - sizeof(hkAgentEntry) = 8
	//   - sizeof(hkAgentNnEntry) = 8 + 20
	//
	//   HK_POINTER_SIZE == 8
	//   --------------------
	//   - sizeof(hkAgentEntry) = 16
	//   - sizeof(hkAgentNnEntry) = 16 + 32
struct hkAgentNnEntry: hkAgentEntry
{
	hkUchar			m_collisionQualityIndex;
	hkUchar			m_padding[3];
	hkObjectIndex	m_agentIndexOnCollidable[2];
	hkContactMgr*	m_contactMgr;
	hkLinkedCollidable*	m_collidable[2];

	inline hkCollidable*  getCollidableA(){ return reinterpret_cast<hkCollidable*>(m_collidable[0]); }
	inline hkCollidable*  getCollidableB(){ return reinterpret_cast<hkCollidable*>(m_collidable[1]); }
};

struct hkAgentNnTrack
{
	hkAgentNnTrack( int agentSize, int sectorSize )
	{
		m_agentSize = (hkUint16)agentSize;
		m_sectorSize = (hkUint16)sectorSize;

		m_bytesUsedInLastSector = m_sectorSize;

		HK_ASSERT2(0xf0ff0088, m_sectorSize % m_agentSize == 0, "SectorSize must be a multiple of agentSize");
	}

	int getSectorSize( int sectorIndex )
	{
		if ( sectorIndex+1 == m_sectors.getSize())
		{
			return m_bytesUsedInLastSector;
		}
		return m_sectorSize;
	}


	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CDINFO, hkAgentNnTrack );

	hkInplaceArray<hkAgentNnSector*,1> m_sectors;

	hkUint32	 m_bytesUsedInLastSector;
	hkUint16     m_agentSize;
	hkUint16     m_sectorSize;
};

struct hkAgentNnSector
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkAgentNnSector);
	
	char m_data[ 1 ];

	hkAgentNnEntry* getBegin()
	{
		return reinterpret_cast<hkAgentNnEntry*>( &m_data[0] );
	}

	hkAgentNnEntry* getEnd( const hkAgentNnTrack& track )
	{
		return reinterpret_cast<hkAgentNnEntry*>( &m_data[track.m_sectorSize] );
	}
};



#endif // HK_COLLIDE2_COLLISION_AGENT_TRACK_H


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
