/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkcollide/hkCollide.h>

//#include <hkcollide/agent/hkProcessCollisionOutputs.h>
#include <hkcollide/agent/boxbox/hkBoxBoxManifold.h>
#include <hkcollide/agent/hkCollisionInput.h>
#include <hkcollide/agent/hkContactMgr.h>


HK_COMPILE_TIME_ASSERT( sizeof(hkProcessCdPoint) == 48);

HK_COMPILE_TIME_ASSERT( sizeof(hkBoxBoxManifold) <= 64 + 32 );


hkBoxBoxManifold::hkBoxBoxManifold()
{
	m_numPoints = 0;
	m_faceVertexFeatureCount = 0;
	m_isComplete = false;
}




int hkBoxBoxManifold::addPoint( const hkCdBody& bodyA, const hkCdBody& bodyB, hkFeatureContactPoint& fcp )
{

	//!me could have a faster lookup for agent specific manifolds.
	int size = m_numPoints;

	//!me
	if( size > HK_BOXBOX_MANIFOLD_MAX_POINTS )
		return -1;

	if ( 1 )
	{
		if ( findInManifold( fcp ) )
		{
			// this point is already in the manifold
			HK_ASSERT(0x72283b85, 0);
			return -1;
		}
	}

	// ok, we've got a new point
	const int i = m_numPoints;
	if( i < HK_BOXBOX_MANIFOLD_MAX_POINTS )
	{
		m_contactPoints[i] = fcp;

		m_numPoints++;

	}
	else
	{
		// out of manifold points
		HK_ASSERT(0x1eca4c57, 0); 
		return -1;
	}

	return i;

}


void hkBoxBoxManifold::removePoint( int i )
{

	m_isComplete = false;

	m_contactPoints[i] = m_contactPoints[m_numPoints - 1];

	m_numPoints--;


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
