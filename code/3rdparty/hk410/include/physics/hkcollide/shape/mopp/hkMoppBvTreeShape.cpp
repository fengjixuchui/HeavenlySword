/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkbase/debugutil/hkStatisticsCollector.h>

#include <hkcollide/shape/mopp/hkMoppBvTreeShape.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkcollide/shape/hkRayShapeCollectionFilter.h>

#include <hkinternal/collide/mopp/code/hkMoppCode.h>

#include <hkinternal/collide/mopp/machine/hkMoppLongRayVirtualMachine.h>
#include <hkinternal/collide/mopp/machine/hkMoppVirtualMachine.h>
#include <hkinternal/collide/mopp/machine/hkMoppObbVirtualMachine.h>
#include <hkinternal/collide/mopp/machine/hkMoppSphereVirtualMachine.h>
#include <hkinternal/collide/mopp/utility/hkMoppDebugger.h>

#include <hkcollide/util/hkAabbUtil.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkMoppBvTreeShape);

const char* HK_MOPP_CODE_ERROR = "You can not supply a null MoppCode. Was the Mopp setup correctly?.";

hkMoppBvTreeShape::hkMoppBvTreeShape( const hkShapeCollection* collection, const hkMoppCode *code )
	:	hkBvTreeShape( collection ), 
		m_code( code )
{
	HK_ASSERT2(0xcf45fedb, m_code, HK_MOPP_CODE_ERROR);
	m_code->addReference();
}

hkMoppBvTreeShape::~hkMoppBvTreeShape()
{
	m_code->removeReference();
}


hkShapeType hkMoppBvTreeShape::getType() const
{
	return HK_SHAPE_MOPP;
}

void hkMoppBvTreeShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& aabbOut ) const
{
	m_child->getAabb( localToWorld, tolerance, aabbOut );
}



void hkMoppBvTreeShape::querySphere( const class hkSphere &sphere, hkArray<hkShapeKey>& hits ) const
{
	hkMoppSphereVirtualMachine sphereMachine;
	HK_ASSERT2(0xcf45fedb, m_code, HK_MOPP_CODE_ERROR);
	sphereMachine.querySphere( m_code, sphere, (hkArray<hkMoppPrimitiveInfo>*)&hits );
}


void hkMoppBvTreeShape::queryObb( const hkTransform& obbToMopp, const hkVector4& extent, hkReal tolerance, hkArray<hkShapeKey>& hits ) const
{
	hkMoppObbVirtualMachine obbMachine;
	HK_ASSERT2(0xcf45fedb, m_code, HK_MOPP_CODE_ERROR);
	obbMachine.queryObb( m_code, obbToMopp, extent, tolerance, (hkArray<hkMoppPrimitiveInfo>*)&hits );
}

void hkMoppBvTreeShape::queryAabb( const hkAabb& aabb, hkArray<hkShapeKey>& hits ) const
{
	hkVector4 offset = m_code->m_info.m_offset; 

	hkReal width = 16777216.0f/m_code->m_info.getScale();

	hkAabb moppAabb;
	moppAabb.m_min = offset;
	moppAabb.m_max = offset; 
	hkVector4 w;
	w.set(width, width, width);
	moppAabb.m_max.add4(w);

	hkAabb newAabb = moppAabb;
	newAabb.m_min.setMax4(newAabb.m_min, aabb.m_min);
	newAabb.m_max.setMin4(newAabb.m_max, aabb.m_max); 

	hkMoppObbVirtualMachine obbMachine; 
	HK_ASSERT2(0xcf45fedb, m_code, HK_MOPP_CODE_ERROR);
	obbMachine.queryAabb( m_code, newAabb, (hkArray<hkMoppPrimitiveInfo>*)&hits ); 
}

HK_COMPILE_TIME_ASSERT( sizeof( hkShapeKey ) == sizeof( hkMoppPrimitiveInfo ) );


hkBool hkMoppBvTreeShape::castRay(const hkShapeRayCastInput& input, hkShapeRayCastOutput& results) const
{
#ifdef HK_MOPP_DEBUGGER_ENABLED
	{
		hkShapeRayCastOutput testResults;
		m_shapeCollection->castRay( input, testResults );
		hkMoppDebugger::getInstance().initDisabled();
		if ( testResults.m_hitFraction < 1.0f )
		{
			hkMoppDebugger::getInstance().initUsingCodeAndTri(m_code, testResults.m_shapeKey);
		}
	}
#endif
	HK_TIMER_BEGIN("rcMopp", HK_NULL);
	hkMoppLongRayVirtualMachine longray;
	results.changeLevel(1);
	hkBool result = longray.queryLongRay( getShapeCollection(), m_code, input, results);
	results.changeLevel(-1);
	if( result )
	{
		results.setKey(0);
	}
	HK_TIMER_END();
	return result;
}


void hkMoppBvTreeShape::castRayWithCollector(const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
#ifdef HK_MOPP_DEBUGGER_ENABLED
	{
		hkShapeRayCastOutput testResults;
		m_shapeCollection->castRay( input, testResults );
		hkMoppDebugger::getInstance().initDisabled();
		if ( testResults.m_hitFraction < 1.0f )
		{
			hkMoppDebugger::getInstance().initUsingCodeAndTri(m_code, testResults.m_shapeKey);
		}
	}
#endif
	HK_TIMER_BEGIN("rcMopp", HK_NULL);
	hkMoppLongRayVirtualMachine longray;
	hkCdBody body(&cdBody);
	body.setShape( getShapeCollection(), 0);
	longray.queryLongRay( getShapeCollection(), m_code, input, body, collector );
	HK_TIMER_END();
}


void hkMoppBvTreeShape::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject("MoppShape", collector->MEMORY_SHARED, this);
	collector->addChildObject( "Mesh", collector->MEMORY_SHARED, m_child.getChild() );
	collector->addChunk( "Mopp", collector->MEMORY_SHARED, this->m_code, m_code->getCodeSize() );
	collector->endObject();
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
