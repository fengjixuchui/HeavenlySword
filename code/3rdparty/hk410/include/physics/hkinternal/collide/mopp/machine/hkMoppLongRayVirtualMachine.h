/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_MOPP_LONG_RAY_VIRTUAL_MACHINE_H
#define HK_COLLIDE2_MOPP_LONG_RAY_VIRTUAL_MACHINE_H

// Virtual Machine command definitions
#include <hkinternal/collide/mopp/code/hkMoppCode.h>
#include <hkinternal/collide/mopp/machine/hkMoppVirtualMachine.h>
#include <hkcollide/shape/hkShape.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>



// Read detailed comment in the cpp class
class hkMoppLongRayVirtualMachine: public hkMoppVirtualMachine 
{
	public:

		inline hkMoppLongRayVirtualMachine(){}
		inline ~hkMoppLongRayVirtualMachine(){}

		// data driven query
		hkBool queryLongRay(const class hkShapeCollection* collection, const hkMoppCode* code, const hkShapeRayCastInput& input, hkShapeRayCastOutput& rayResult);

		// collector based raycast
		void queryLongRay(const class hkShapeCollection* collection, const hkMoppCode* code, const hkShapeRayCastInput& input, const hkCdBody& body, hkRayHitCollector& collector);

	////////////////////////////////////////////////////////////////
	//
	// THE REMAINDER OF THIS FILE IS FOR INTERNAL USE
	//
	//////////////////////////////////////////////////////////////// 

	protected:

		//we will use the information here to go from int to float space
		const hkMoppCode*	m_code;
		float				m_ItoFScale;
		
		//used for ray intersection test
		//the original ray (must be in primitive space)
		hkShapeRayCastInput m_ray;

		//whether a hkReal hit has been discovered already
		hkBool				m_hitFound;		
		hkReal				m_earlyOutHitFraction;

		// either one of those is set

		// for data driven
		hkShapeRayCastOutput*	m_rayResultPtr;

		// for callback driven
		hkRayHitCollector*      m_collector;
		const hkCdBody*			m_body;

		const hkShapeCollection* m_collection;

	protected:

		struct QueryInt;
		struct QueryFloat;

		void queryRayOnTree	( const QueryInt* query, const unsigned char* commands, QueryFloat* const fQuery);

		//only add a hit if it definitely is a hit
		HK_FORCE_INLINE void addHit(unsigned int id, const unsigned int properties[hkMoppCode::MAX_PRIMITIVE_PROPERTIES]);

		HK_FORCE_INLINE void queryLongRaySub(const hkMoppCode* code,   const hkShapeRayCastInput& input );
};


#endif // HK_COLLIDE2_MOPP_LONG_RAY_VIRTUAL_MACHINE_H

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
