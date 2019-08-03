/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/motion/rigid/ThinBoxMotion/hkThinBoxMotion.h>
#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
#include <hkmath/linear/hkSweptTransformUtil.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkThinBoxMotion);

HK_COMPILE_TIME_ASSERT( sizeof(hkThinBoxMotion) == sizeof(hkMotion) );

hkThinBoxMotion::hkThinBoxMotion(const hkVector4& position, const hkQuaternion& rotation)
	:hkBoxMotion( position, rotation )
{
	m_type = MOTION_THIN_BOX_INERTIA;
}



/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
