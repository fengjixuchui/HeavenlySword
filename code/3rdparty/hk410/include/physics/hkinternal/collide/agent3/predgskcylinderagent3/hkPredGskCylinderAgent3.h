/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_PREDICTIVE_GSK_CYLINDER_AGENT3_H
#define HK_COLLIDE2_PREDICTIVE_GSK_CYLINDER_AGENT3_H

#include <hkinternal/collide/agent3/hkAgent3.h>
#include <hkcollide/shape/hkShapeType.h>

class hkCollisionDispatcher;
class hkGskCache;

/// The pure purpose of hkPredGskCylinderAgent3 is to allow rolling of cylinders.
/// It only changes the functionality of hkPredGskAgent3 in its non-predictive, manifold-generating mode.
namespace hkPredGskCylinderAgent3
{
	hkAgentData* HK_CALL create  ( const hkAgent3Input& input, hkAgentEntry* entry, hkAgentData* freeMemory );

	hkAgentData* HK_CALL process ( const hkAgent3ProcessInput& input, hkAgentEntry* entry, hkAgentData* agentData, hkVector4* separatingNormal, hkProcessCollisionOutput& result);

	void			HK_CALL registerAgent3( hkCollisionDispatcher* dispatcher );
}



#endif // HK_COLLIDE2_PREDICTIVE_GSK_CYLINDER_AGENT3_H

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
