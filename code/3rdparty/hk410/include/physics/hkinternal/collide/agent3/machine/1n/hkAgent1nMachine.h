/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_AGENT3_1N_MACHINE_H
#define HK_COLLIDE2_AGENT3_1N_MACHINE_H

#include <hkinternal/collide/agent3/hkAgent3.h>


struct hkAgent1nTrack;
struct hkAgent3Input;
class hkShapeCollection;
#ifndef hkCollisionConstraintOwner
class hkConstraintOwner;
#	define hkCollisionConstraintOwner hkConstraintOwner
#endif

// Make sure if you use this header that you pad to get a 16 byte alignment
struct hkAgent1nMachineEntry: hkAgentEntry
{
	hkShapeKey m_shapeKey;
};

struct hkAgent1nMachinePaddedEntry: hkAgent1nMachineEntry
{
	hkUlong m_padUpTo16Bytes;
};

struct hkAgent1nMachineTimEntry: hkAgent1nMachineEntry
{
	hkTime     m_timeOfSeparatingNormal;		// only used if tims are enabled
	hkVector4  m_separatingNormal;
};


struct hkAgent1nMachine_VisitorInput
{
		/// BodyA
	const hkCdBody* m_bodyA;

		/// BvTree or shape collection (if there's no bvTree in the hierarchy) BodyB or 
	const hkCdBody* m_bvTreeBodyB;

		/// The shape collection, which is the shape of bodyB
		/// Note that we do not store the shape key needed to extract the child body of interest from m_bvTreeBodyB. 
		/// The shape key is taken from the agent entry.
	const hkShapeCollection* m_collectionShapeB;

		/// Pointer to hkProcessCollisionInput
	const hkCollisionInput* m_input;

		/// Pointer to hkContactMgr
	hkContactMgr*	m_contactMgr;

	hkCollisionConstraintOwner* m_constraintOwner;

	void* m_clientData;
};

typedef hkAgentData* (*hkAgent1nMachine_VisitorCallback)( hkAgent1nMachine_VisitorInput& vin, hkAgent1nMachineEntry* entry, hkAgentData* agentData );


extern "C"
{
		/// Initialize the agentTrack
	void HK_CALL hkAgent1nMachine_Create( hkAgent1nTrack& agentTrack );

		/// call process collision on all agents
	void HK_CALL hkAgent1nMachine_Process( hkAgent1nTrack& agentTrack, hkAgent3ProcessInput& input, const hkShapeContainer* shapeCollection, const hkShapeKey* hitList, int numHits, hkProcessCollisionOutput& output  );

		/// Update all filters
	void HK_CALL hkAgent1nMachine_UpdateShapeCollectionFilter( hkAgent1nTrack& agentTrack, hkAgent1nMachine_VisitorInput& vin );

		/// revisits all agents, the visitor should return a pointer just after the end of the agent.
		/// visitor can change size of agent. If visitor returns the start of the agent, the agent will be removed 
	void HK_CALL hkAgent1nMachine_VisitAllAgents( hkAgent1nTrack& agentTrack, hkAgent1nMachine_VisitorInput& vin, hkContactMgr* mgr, hkAgent1nMachine_VisitorCallback visitor );


		/// destroy all agents
	void HK_CALL hkAgent1nMachine_Destroy( hkAgent1nTrack& agentTrack, hkCollisionDispatcher* dispatch, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner );

		/// destroy all cache information
	void HK_CALL hkAgent1nMachine_InvalidateTim( hkAgent1nTrack& track, hkCollisionInput& input );
		
	void HK_CALL hkAgent1nMachine_WarpTime( hkAgent1nTrack& track, hkTime oldTime, hkTime newTime, hkCollisionInput& input );
	
	//
	// internal functions
	//
	
		/// Input:
		///   - List of potential points, all potential points have a reserved contact point
		///   - List of reference contact points. This list also includes potential points
	void HK_CALL hkAgent1nMachine_Weld( hkAgent3Input& input, const hkShapeContainer* shapeCollection, hkProcessCollisionOutput& output );

	hkAgent1nMachineEntry* hkAgent1nMachine_FindAgent( hkAgent1nTrack& agentTrack, hkShapeKey key, hkAgentData** agentDataOut);

}



#endif // HK_COLLIDE2_AGENT3_1N_MACHINE_H

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
