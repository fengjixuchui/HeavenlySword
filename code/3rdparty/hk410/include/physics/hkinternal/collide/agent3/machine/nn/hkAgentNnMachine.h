/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_AGENT3_COLLECTION_MACHINE_H
#define HK_COLLIDE2_AGENT3_COLLECTION_MACHINE_H

#include <hkinternal/collide/agent3/hkAgent3.h>
#include <hkinternal/collide/agent3/machine/nn/hkAgentNnTrack.h>

class hkLinkedCollidable;

struct hkAgentNnMachinePaddedEntry: hkAgentNnEntry
{
#if HK_POINTER_SIZE == 4
	hkUint32 m_padOutTo16Bytes;
#endif
};

struct hkAgentNnMachineTimEntry: hkAgentNnEntry
{
	hkTime     m_timeOfSeparatingNormal;		// only used if tims are enabled
	hkVector4  m_separatingNormal;
};

#define HK_FOR_ALL_AGENT_ENTRIES_BEGIN( TRACK, ENTRY )											\
	{																							\
		for (int HKLOOP_sectorIndex = 0; HKLOOP_sectorIndex < TRACK.m_sectors.getSize(); )		\
		{																						\
			hkAgentNnSector* HKLOOP_currentSector = TRACK.m_sectors[HKLOOP_sectorIndex];			\
			hkAgentNnEntry* ENTRY = HKLOOP_currentSector->getBegin();							\
			HKLOOP_sectorIndex++;																\
			hkAgentNnEntry* HKLOOP_endEntry =  (HKLOOP_sectorIndex == TRACK.m_sectors.getSize()) ?		\
					hkAddByteOffset(ENTRY, TRACK.m_bytesUsedInLastSector)						\
					: HKLOOP_currentSector->getEnd(TRACK);											\
			for( ; ENTRY < HKLOOP_endEntry; ENTRY = hkAddByteOffset( ENTRY, ENTRY->m_size ) )			\
			{

#define HK_FOR_ALL_AGENT_ENTRIES_END } } }


extern "C"
{
		// Just use the constructor, dummy ;-)
	//void HK_CALL hkAgentNnMachine_CreateTrack( hkAgentNnTrack& track );

	void HK_CALL hkAgentNnMachine_DestroyTrack( hkAgentNnTrack& track, hkCollisionDispatcher* dispatch );

	void HK_CALL hkAgentNnMachine_GetAgentType( hkCollidable* collA, hkCollidable* collB, const hkProcessCollisionInput& input, int& agentTypeOut, int& isFlippedOut );

	hkAgentNnEntry* HK_CALL hkAgentNnMachine_CreateAgent( hkAgentNnTrack& track, hkLinkedCollidable* collA, hkLinkedCollidable* collB, int agentType, const hkProcessCollisionInput& input, hkContactMgr* mgr);

	void HK_CALL hkAgentNnMachine_DestroyAgent( hkAgentNnTrack& track, hkAgentNnEntry* entry, hkCollisionDispatcher* dispatch, hkCollisionConstraintOwner& constraintOwner );


	void HK_CALL hkAgentNnMachine_AppendTrack( hkAgentNnTrack& track, hkAgentNnTrack& appendee);


	void HK_CALL hkAgentNnMachine_UpdateShapeCollectionFilter( hkAgentNnEntry* entry, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner );

	//
	// processing
	//

	void HK_CALL hkAgentNnMachine_ProcessAgent( hkAgentNnEntry* entry,	const hkProcessCollisionInput& input, hkProcessCollisionOutput& output );

		// process a track, stops if out of memory
	void HK_CALL hkAgentNnMachine_ProcessTrack( class hkConstraintOwner* owner, hkAgentNnTrack& track,	const hkProcessCollisionInput& input );



	void HK_CALL hkAgentNnMachine_InternalDeallocateEntry(hkAgentNnTrack& track, hkAgentNnEntry* entry);

	bool HK_CALL hkAgentNnMachine_IsEntryOnTrack(hkAgentNnTrack& track, hkAgentEntry* entry);

	// ALSO FOR EXTERNAL USE

		/// Makes a copy of the entry in the destTrack and redirects all pointers to entry to the new copy.
		/// Does not touch the entry. In the end, no pointers will point to the original entry
	hkAgentNnEntry* HK_CALL hkAgentNnMachine_CopyAndRelinkAgentEntry( hkAgentNnTrack& destTrack, hkAgentEntry* entry );

	hkAgentNnEntry* HK_CALL hkAgentNnMachine_FindAgent( hkLinkedCollidable* collA, hkLinkedCollidable* collB );


		// activation/deactiation/setPositionOnEntity
	void HK_CALL hkAgentNnMachine_InvalidateTimInAgent( hkAgentNnEntry* entry, hkCollisionInput& input );

	void HK_CALL hkAgentNnMachine_WarpTimeInAgent( hkAgentNnEntry* entry, hkTime oldTime, hkTime newTime, hkCollisionInput& input );


	//
	// Debugging
	//

	void HK_CALL hkAgentNnMachine_AssertTrackValidity( hkAgentNnTrack& track );


	//
	// UNDONE
	//

	void HK_CALL hkAgentNnMachine_TouchAgent( hkAgentEntry* entry,	const hkProcessCollisionInput& input );

}



#endif // HK_COLLIDE2_AGENT3_COLLECTION_MACHINE_H

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
