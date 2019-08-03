/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


HK_FORCE_INLINE hkTriangleShape::hkTriangleShape(hkReal radius)
: hkConvexShape(radius)
{
}

HK_FORCE_INLINE hkTriangleShape::hkTriangleShape(const hkVector4& v0, const hkVector4& v1, const hkVector4& v2, hkReal radius)
: hkConvexShape(radius)
{
	m_vertexA = v0;
	m_vertexB = v1;
	m_vertexC = v2;
}

const hkVector4* hkTriangleShape::getVertices() const
{
	return &m_vertexA;
}

hkVector4& hkTriangleShape::getVertex(int i)
{
	HK_ASSERT2(0x312d54aa,  i>=0 && i < 3, "A triangle has only 3 vertices");
	return (&m_vertexA)[i];
}

const hkVector4& hkTriangleShape::getVertex(int i) const
{
	HK_ASSERT2(0x7d790924,  i>=0 && i < 3, "A triangle has only 3 vertices");
	return (&m_vertexA)[i];
}

void hkTriangleShape::setVertex(int i, const hkVector4& vertex)
{
	HK_ASSERT2(0x18d4155c,  i>=0 && i < 3, "A triangle has only 3 vertices");
	(&m_vertexA)[i] = vertex;
}


/*

hkUint32 hkTriangleShape::getUserIdB() const
{
	return m_userIdB;
}


void hkTriangleShape::setUserIdB( hkUint32 id )
{
	m_userIdB = id;
}
*/

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
