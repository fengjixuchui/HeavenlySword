/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkcollide/hkCollide.h>
#include <hkcollide/shape/trisampledheightfield/hkTriSampledHeightFieldBvTreeShape.h>
#include <hkcollide/shape/trisampledheightfield/hkTriSampledHeightFieldCollection.h>
#include <hkcollide/shape/sampledheightfield/hkSampledHeightFieldShape.h>
#include <hkcollide/util/hkAabbUtil.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkTriSampledHeightFieldBvTreeShape);

const hkTriSampledHeightFieldCollection* hkTriSampledHeightFieldBvTreeShape::getChild() const
{
	return static_cast<const hkTriSampledHeightFieldCollection*>(m_child.getChild());
}

hkTriSampledHeightFieldBvTreeShape::hkTriSampledHeightFieldBvTreeShape( const hkTriSampledHeightFieldCollection* c,  hkBool doAabbRejection  )
	:	hkBvTreeShape( c )
{
	m_wantAabbRejectionTest = doAabbRejection;
}

void hkTriSampledHeightFieldBvTreeShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	getChild()->getAabb( localToWorld, tolerance, out );
}

hkBool hkTriSampledHeightFieldBvTreeShape::castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& output ) const
{
	return getChild()->getHeightFieldShape()->castRay( input, output );
}
void hkTriSampledHeightFieldBvTreeShape::castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
	getChild()->getHeightFieldShape()->castRayWithCollector( input, cdBody, collector );
}

void hkTriSampledHeightFieldBvTreeShape::querySphere( const hkSphere &sphereQuery, hkArray< hkShapeKey >& hits ) const
{
	hkAabb aabb;
	aabb.m_max.setAll( sphereQuery.getRadius() );
	aabb.m_min.setAll( -sphereQuery.getRadius() );
	aabb.m_max.add4( sphereQuery.getPosition() );
	aabb.m_min.add4( sphereQuery.getPosition() );


	queryAabb( aabb, hits );
}

void hkTriSampledHeightFieldBvTreeShape::queryObb( const hkTransform& obbTransform, const hkVector4& obbExtent, hkReal tolerance, hkArray< hkShapeKey >& hits ) const
{
	hkAabb aabb;
	hkAabbUtil::calcAabb( obbTransform, obbExtent, tolerance, aabb );

	queryAabb( aabb, hits );
}


void hkTriSampledHeightFieldBvTreeShape::queryAabb( const hkAabb& aabbIn, hkArray<hkShapeKey>& hits ) const
{
	const hkSampledHeightFieldShape* hf = getChild()->getHeightFieldShape();

	hkAabb aabb;
	aabb = aabbIn;
	aabb.m_max.setAll( getChild()->getRadius() );
	aabb.m_min.setAll( -getChild()->getRadius() );
	aabb.m_max.add4(aabbIn.m_max);
	aabb.m_min.add4(aabbIn.m_min);

	hkIntUnion64 out;

	hkVector4Util::convertToUint16( aabb.m_min, hf->m_floatToIntOffsetFloorCorrected, hf->m_floatToIntScale, out );
	hkUint32 minX = out.u16[0];
	hkUint32 minZ = out.u16[2];

	hkVector4Util::convertToUint16( aabb.m_max, hf->m_floatToIntOffsetFloorCorrected, hf->m_floatToIntScale, out );
	hkUint32 maxX = out.u16[0];
	hkUint32 maxZ = out.u16[2];


	//
	//	If outside, return
	//
	if ((minX >= hkUint32(hf->m_xRes-1)) && (maxX >= hkUint32(hf->m_xRes-1) ))
	{
		return;
	}

	if ((minZ >= hkUint32(hf->m_zRes-1) ) && (maxZ >= hkUint32(hf->m_zRes-1) ))
	{
		return;
	}

	//
	// Clip to boundaries
	//

	if (minX >= hkUint32(hf->m_xRes-1) )
	{
		minX = 0;
	}

	if (minZ >= hkUint32(hf->m_zRes-1) )
	{
		minZ = 0;
	}

	if (maxX >= hkUint32(hf->m_xRes-1) )
	{
		maxX = hkUint32(hf->m_xRes-2);
	}

	if (maxZ >= hkUint32(hf->m_zRes-1) )
	{
		maxZ = hkUint32(hf->m_zRes-2);
	}

	int initialSize = hits.getSize();
	//
	// Write out list of keys
	//

	if (m_wantAabbRejectionTest)
	{

		hkBool aboveHeightField = true;
		hkBool belowHeightField = true;

		for ( hkUint32 x = minX; x <= maxX; x++ )
		{
			for ( hkUint32 z = minZ; z <= maxZ; z++ )
			{
				hits.pushBack((x << 1) + (z << 16));
				hits.pushBack( ((x << 1) + (z << 16)) | 1);

				if ( aboveHeightField ||  belowHeightField)
				{
					hkReal height = hf->m_intToFloatScale(1) * hf->getHeightAt( x, z );
					if ( aabb.m_min(1) < height )
					{
						aboveHeightField = false;
					}
					if ( aabb.m_max(1) > height )
					{
						belowHeightField = false;
					}
				}
			}
		}

		if ( aboveHeightField ||  belowHeightField )
		{
			for ( hkUint32 x = minX; x <= maxX + 1; x++ )
			{
				hkReal height = hf->m_intToFloatScale(1) * hf->getHeightAt( x, maxZ + 1 );
				if ( aabb.m_min(1) < height )
				{
					aboveHeightField = false;
				}
				if ( aabb.m_max(1) > height )
				{
					belowHeightField = false;
				}
			}
			for ( hkUint32 z = minZ; z <= maxZ + 1; z++ )
			{
				hkReal height = hf->m_intToFloatScale(1) * hf->getHeightAt( maxX + 1, z );
				if ( aabb.m_min(1) < height )
				{
					aboveHeightField = false;
				}
				if ( aabb.m_max(1) > height )
				{
					belowHeightField = false;
				}
			}

		}

		if (aboveHeightField ||  belowHeightField )
		{
			hits.setSize( initialSize );
		}
	}
	else
	{
		for ( hkUint32 x = minX; x <= maxX; x++ )
		{
			for ( hkUint32 z = minZ; z <= maxZ; z++ )
			{
				hits.pushBack((x << 1) + (z << 16));
				hits.pushBack( ((x << 1) + (z << 16)) | 1);
			}
		}
	}
}

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
