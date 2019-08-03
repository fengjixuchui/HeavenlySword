/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_GSK_MANIFOLD_H
#define HK_COLLIDE2_GSK_MANIFOLD_H

#include <hkmath/basetypes/hkContactPoint.h>

	/// This is class to hold a feature based manifold.
	/// Note: the class only occupies it's first getTotalSizeInBytes.
struct hkGskManifold
{
	//
	//	public structures 
	//
	struct ContactPoint
	{
		hkUchar m_dimA;
		hkUchar m_dimB;
		hkContactPointId m_id;
		union
		{
			hkUchar m_vert[4];	// is a byte offset to a hkVector4 array
			hkUint32 m_allVerts;
		};
	};

	typedef hkUint16 VertexId;

	enum { MAX_POINTS = 4 };
	enum { MAX_VERTICES = MAX_POINTS * 4  };



	HK_FORCE_INLINE VertexId getVertexId( const ContactPoint& cp, int vertNr ) const;

	HK_FORCE_INLINE void init();

	HK_FORCE_INLINE hkGskManifold();

	HK_FORCE_INLINE int getTotalSizeInBytes();

	HK_FORCE_INLINE VertexId* getVertexIds();

	HK_FORCE_INLINE const VertexId* getVertexIds() const;
	//
	//	Members
	//

		/// Total number of vertices used of object A for all contact points
	hkUchar  m_numVertsA;
	hkUchar  m_numVertsB;
	hkUchar  m_numContactPoints;

	ContactPoint m_contactPoints[ MAX_POINTS ];
	// followed the the array of vertexIds
		// add padding so at least we can fit MAX_POINTS and MAX_VERTICES in this structure
	hkUchar m_padding[ MAX_VERTICES * sizeof(VertexId) ];

};

void hkGskManifold::init()	
{		
	*((hkUint32*)this) = 0; 	
}

hkGskManifold::hkGskManifold()
{ 
	init(); 
}

hkGskManifold::VertexId* hkGskManifold::getVertexIds()
{
	return (VertexId*)( &m_contactPoints[m_numContactPoints] );
}

const hkGskManifold::VertexId* hkGskManifold::getVertexIds() const
{
	return (const VertexId*)( &m_contactPoints[m_numContactPoints] );
}

hkUint16 hkGskManifold::getVertexId( const ContactPoint& cp, int vertNr ) const
{
	return *hkAddByteOffsetConst(getVertexIds(), (hkUint8)(cp.m_vert[vertNr] >> 3) );	// sizeof(VertexId)/sizeof(hkVector4)
}

int hkGskManifold::getTotalSizeInBytes()
{
	return 4 //  m_numVertsA + m_numVertsB + m_numContactPoints		// header
		 + m_numContactPoints           * hkSizeOf(ContactPoint)	// contact points
		 + (m_numVertsA + m_numVertsB ) * hkSizeOf(VertexId);		// vertices

}


#endif //HK_COLLIDE2_GSK_MANIFOLD_H

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
