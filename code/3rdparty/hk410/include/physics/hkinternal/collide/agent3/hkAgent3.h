/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_COLLISION_AGENT3_H
#define HK_COLLIDE2_COLLISION_AGENT3_H

#include <hkmath/hkMath.h>
#include <hkmath/basetypes/hkContactPoint.h>

struct hkProcessCollisionInput;
class hkContactMgr;
struct hkProcessCollisionOutput;
struct hkCollisionQualityInfo;
class hkLinkedCollidable;
#ifndef hkCollisionConstraintOwner
class hkConstraintOwner;
#	define hkCollisionConstraintOwner hkConstraintOwner
#endif
struct hkCollisionInput;
class hkCdBody;

	/// the base memory stream 
struct hkAgentEntry
{
		/// see hkAgent3::StreamCommand
	hkUchar     m_streamCommand;	

		/// the type of the agent (to be used with the hkCollisionDispatcher)
	hkUchar     m_agentType;		

		/// The number of contact points, if you don't use this, set it to 1
	hkUchar     m_numContactPoints;			

		/// the size of the agent in bytes
	hkUchar     m_size;										

		/// a user data, which can be used by the agent3 (a.g. contact point id, subagent ....)
	hkUlong     m_userData;					
};


typedef void hkAgentData;

	/// The input structure needed to call an implementation of an agent3
struct hkAgent3Input
{
		/// BodyA
	const hkCdBody* m_bodyA;

		/// BodyB
	const hkCdBody* m_bodyB;

		/// Pointer to hkProcessCollisionInput
	const hkProcessCollisionInput* m_input;

		/// Pointer to hkContactMgr
	hkContactMgr*	m_contactMgr;

		/// a transform converting from b to a space
	hkTransform		m_aTb;
};

struct hkAgent3ProcessInput: public hkAgent3Input
{
		/// for gsk convex agents using tim
	hkReal          m_distAtT1;

		/// for agents using tim
	hkVector4       m_linearTimInfo;
};


namespace hkAgent3
{
		/// An id, which can be used with the hkCollisionDispatcher to get the right function
	typedef int AgentType;

	enum {
		MIN_TOTAL_SIZE = 16,		// 16 = size of end command
		MAX_MEMORY_INCREASE = 80,	// <todo: set back to 48 the maximum amount of memory an agent is allowed to increase every frame
		MAX_INITIAL_SIZE    = MAX_MEMORY_INCREASE + MIN_TOTAL_SIZE
	};

	enum StreamCommand
	{
		STREAM_NULL = 0,		 // not defined
		STREAM_END  = 1,		 // end of last sectors
		STREAM_CALL = 2,		 // call the function declared in the dispatcher using the m_agentType
		STREAM_CALL_FLIPPED = 3, // call the function declared in the dispatcher using the m_agentType with flipped input variables
		STREAM_CALL_WITH_TIM = 4, // same but check tims before
		STREAM_CALL_WITH_TIM_FLIPPED = 5,	// same as STREAM_CALL_WITH_TIM but using flipped input variables
		STREAM_CALL_AGENT = 6,	// special mode for compatability with old style agents.
	};

	//
	//	Functions needed to create a full agent
	//

		/// Create an agent and return a pointer just after the agent (but 16byte aligned)
	typedef hkAgentData* (HK_CALL* CreateFunc)( const hkAgent3Input& input, hkAgentEntry* entry, hkAgentData* agentData );

		/// Destroy the agent
	typedef void  (HK_CALL* DestroyFunc)( hkAgentEntry* entry, hkAgentData* agentData, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner );

		/// call to process collision, which should produce a nice manifold<br>
		/// Notes: 
		///  - separatingNormalOut is set to HK_NULL if SepNormalFunc was set to HK_NULL 
		///  - You should set the entry->m_numContatPoints if possible, else set it to 1
	typedef hkAgentData* (HK_CALL* ProcessFunc)( const hkAgent3ProcessInput& input, hkAgentEntry* entry, hkAgentData* agentData, hkVector4* separatingNormalOut, hkProcessCollisionOutput& result);

	enum Symmetric
	{
		IS_SYMMETRIC,
		IS_NOT_SYMMETRIC,
		IS_NOT_SYMMETRIC_AND_FLIPPED
	};

	//
	//	If an agent uses tim technology it needs to implement:
	//
		/// An optional function, which you need to implement to enable tims.
		/// It calculates a valid separating plane (.w component is distance)
	typedef void  (HK_CALL* SepNormalFunc)( const hkAgent3Input& input, hkAgentData* agentData, hkVector4& separatingNormalOut );

		/// An optional function, which you need to implement to enable tims
		/// This function removes all contact points held by the hkAgent3. You only need to implement
		/// this if SepNormalFunc is defined
	typedef hkAgentData* (HK_CALL* CleanupFunc)( hkAgentEntry* entry, hkAgentData* agentData, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner );



	//
	//	If an agent places contact points in the hkProcessCollisionOutput.m_potentialContacts, 
	//  it needs to implement the following functions
	//

		/// Remove one contact point (including potential ones). This function should not call the contact mgr!
	typedef void (HK_CALL* RemovePointFunc)( hkAgentEntry* entry, hkAgentData* agentData, hkContactPointId idToRemove );

		/// Commit a potential contact point (one which does not have an id yet. This function should not call the contact mgr!
	typedef void (HK_CALL* CommitPotentialFunc)( hkAgentEntry* entry, hkAgentData* agentData, hkContactPointId newId );

		/// Flag a contact point to die at the next process call
		/// This is used when the contact point id is still valid and we need the contact point this very frame
	typedef void (HK_CALL* CreateZombieFunc)( hkAgentEntry* entry, hkAgentData* agentData, hkContactPointId idTobecomeZombie );


	//
	//	Other functions
	//
		/// Update child shapes with collision filter
	typedef void (HK_CALL* UpdateFilterFunc)( hkAgentEntry* entry, hkAgentData* agentData, hkCdBody& bodyA, hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner );

		/// Update child shapes with collision filter
	typedef void (HK_CALL* InvalidateTimFunc)( hkAgentEntry* entry, hkAgentData* agentData, hkCollisionInput& input );

		/// Update child shapes with collision filter
	typedef void (HK_CALL* WarpTimeFunc)( hkAgentEntry* entry, hkAgentData* agentData, hkTime oldTime, hkTime newTime, hkCollisionInput& input );
}


#endif // HK_COLLIDE2_COLLISION_AGENT3_H


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
