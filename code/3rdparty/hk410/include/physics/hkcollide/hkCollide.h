/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_COLLIDE2_H
#define HK_COLLIDE2_COLLIDE2_H

#include <hkmath/hkMath.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkbase/debugutil/hkStatisticsCollector.h>

#include <hkmath/basetypes/hkMotionState.h>
#include <hkmath/basetypes/hkAabb.h>
#include <hkmath/basetypes/hkSphere.h>
#include <hkmath/basetypes/hkContactPoint.h>


#include <hkcollide/shape/hkCdVertex.h>
#include <hkcollide/shape/hkShape.h>
#include <hkcollide/shape/hkShapeType.h>
#include <hkcollide/shape/hkShapeRayCastOutput.h>

#include <hkcollide/dispatch/hkCollisionDispatcher.h>

#include <hkcollide/agent/hkCollidable.h>
#include <hkcollide/agent/hkContactMgr.h>
#include <hkcollide/agent/hkProcessCdPoint.h>
#include <hkcollide/agent/hkCdPoint.h>
#include <hkcollide/agent/hkCdPointCollector.h>
#include <hkcollide/agent/hkCollisionAgent.h>
#include <hkcollide/agent/hkCollisionInput.h>
#include <hkcollide/agent/hkLinearCastCollisionInput.h>
#include <hkcollide/agent/symmetric/hkSymmetricAgent.h>

#endif // HK_COLLIDE2_COLLIDE2_H

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
