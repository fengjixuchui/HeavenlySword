/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/mopp/hkMoppFitToleranceRequirements.h>

hkMoppFitToleranceRequirements::hkMoppFitToleranceRequirements()
{
	m_absoluteFitToleranceOfTriangles = 0.3f;
	m_relativeFitToleranceOfInternalNodes = 0.4f;
	m_absoluteFitToleranceOfInternalNodes = 0.1f;
	m_absoluteFitToleranceOfAxisAlignedTriangles.set( 0.05f, 0.05f, 0.05f );
	m_useShapeKeys = true;
	m_enablePrimitiveSplitting = true;
	m_enableInterleavedBuilding = true;
	m_cachePrimitiveExtents = false;
}

void hkMoppFitToleranceRequirements::setAbsoluteFitToleranceOfTriangles(float inTight)
{
	m_absoluteFitToleranceOfTriangles = inTight;
}

float hkMoppFitToleranceRequirements::getAbsoluteFitToleranceOfTriangles() const
{
	return m_absoluteFitToleranceOfTriangles;
}

void hkMoppFitToleranceRequirements::setAbsoluteFitToleranceOfAxisAlignedTriangles(const hkVector4& inTight)
{
	m_absoluteFitToleranceOfAxisAlignedTriangles = inTight;
}

hkVector4 hkMoppFitToleranceRequirements::getAbsoluteFitToleranceOfAxisAlignedTriangles() const
{
	return m_absoluteFitToleranceOfAxisAlignedTriangles;
}

void hkMoppFitToleranceRequirements::setRelativeFitToleranceOfInternalNodes(float inUnused)
{
	m_relativeFitToleranceOfInternalNodes = inUnused;
}

float hkMoppFitToleranceRequirements::getRelativeFitToleranceOfInternalNodes() const
{
	return m_relativeFitToleranceOfInternalNodes;
}

void hkMoppFitToleranceRequirements::setAbsoluteFitToleranceOfInternalNodes(float inMin)
{
	m_absoluteFitToleranceOfInternalNodes = inMin;
}

float hkMoppFitToleranceRequirements::getAbsoluteFitToleranceOfInternalNodes() const
{
	return m_absoluteFitToleranceOfInternalNodes;
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
