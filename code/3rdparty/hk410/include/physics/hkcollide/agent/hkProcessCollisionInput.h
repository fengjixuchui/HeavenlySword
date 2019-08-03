/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_PROCESS_COLLISION_INPUT_H
#define HK_COLLIDE2_PROCESS_COLLISION_INPUT_H

#include <hkmath/basetypes/hkStepInfo.h>
#include <hkcollide/agent/hkCollisionInput.h>

struct hkAgent1nSector;
struct hkCollisionQualityInfo;

	/// This structure is used for all process collision calls queries.
	/// 
struct hkProcessCollisionInput : public hkCollisionInput
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkProcessCollisionInput);

		/// time step information
	hkStepInfo m_stepInfo;

		/// A pointer to a structure containing internal collision tolerances etc.
	hkCollisionAgentConfig* m_config;

		// Factor used to scale collision tolerance for toi events
	//hkReal  m_toiToleranceFactor;


		/// a pointer to hkWorldDynamicsStepInfo if you use the hkDynamics lib, otherwise this can be used as a user pointer
	void*	m_dynamicsInfo;

		/// A pointer to the collision quality information. See hkCollisionDispatcher for more details
	mutable hkCollisionQualityInfo* m_collisionQualityInfo;
};

#endif // HK_COLLIDE2_PROCESS_COLLISION_INPUT_H

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
