/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_MOPP_AABB_CAST_VIRTUAL_MACHINE_H
#define HK_COLLIDE2_MOPP_AABB_CAST_VIRTUAL_MACHINE_H

// Virtual Machine command definitions
#include <hkinternal/collide/mopp/code/hkMoppCode.h>
#include <hkinternal/collide/mopp/machine/hkMoppVirtualMachine.h>
#include <hkcollide/shape/hkShape.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>



// Virtual Machine command definitions
struct hkMoppAabbCastVirtualMachineQueryInt
{
	//the offset of the all previous scales are accumulated here (in B space)
	hkVector4 m_FtoBoffset;
	hkVector4 m_extents;	// in B space
	hkReal	  m_extentsSum3;

	//the shifts from all previous scale commands are accumulated here
	int m_shift;

	// this converts  floating point space into form byte space
	hkReal m_FtoBScale;

	// the current offset for the primitives
	unsigned int m_primitiveOffset;  
	unsigned int m_properties[hkMoppCode::MAX_PRIMITIVE_PROPERTIES];
};

struct hkMoppAabbCastVirtualMachineQueryFloat
{
	// this is the ray in local int coordinate space
	hkVector4 m_rayEnds[2];
};

/// This class implements an AABB cast within the mopp.
/// This is pretty much identical to the hkMoppAabbCastVirtualMachine,
/// so make sure that both versions are kept in sync
class hkMoppAabbCastVirtualMachine: public hkMoppVirtualMachine 
{
	public:

		inline hkMoppAabbCastVirtualMachine(){}
		inline ~hkMoppAabbCastVirtualMachine(){}

		/// Input structure to the aabb cast
		struct hkAabbCastInput
		{
			/// The starting position in mopp space
			hkVector4 m_from;
			/// the destination position in mopp space

			hkVector4 m_to;
			/// The halfExtents of the Aabb in mopp space
			hkVector4 m_extents;


			/// collision input, only used in addRealHit to forward calls
			const hkLinearCastCollisionInput* m_collisionInput;

			/// the body which is encapsulated in the aabb, only used in addRealHit to forward calls
			const hkCdBody* m_castBody;

			/// the body containing the mopp space only used in addRealHit to forward calls
			const hkCdBody* m_moppBody;
		};

		/// This functions casts an aabb through the mopp space and for every hit
		/// it gets the child shape from the shapeCollection and call linearCast
		void aabbCast( const hkAabbCastInput& input, hkCdPointCollector& castCollector, hkCdPointCollector* startCollector );

	////////////////////////////////////////////////////////////////
	//
	// THE REMAINDER OF THIS FILE IS FOR INTERNAL USE
	//
	//////////////////////////////////////////////////////////////// 

	protected:

		/// the mopp code, remember it has the very important info struct inside
		const hkMoppCode*	m_code;

		/// converts from 24 bit int space into floating point space
		float				m_ItoFScale;

		/// m_extentsSum3 = 3.0f * extents.horizontalAdd3()
		hkShapeType			m_castObjectType;
		
		/// set to the current hitFraction
		hkReal				m_earlyOutFraction;
		hkReal				m_refEarlyOutFraction;

			/// for being able to forward calls
		hkCdPointCollector*      m_castCollector;
		hkCdPointCollector*		 m_startPointCollector;

		const hkAabbCastInput*	m_input;

		void queryRayOnTree	( const hkMoppAabbCastVirtualMachineQueryInt* query, const unsigned char* commands,hkMoppAabbCastVirtualMachineQueryFloat* const fQuery);

		/// This function forwards a hit to a child cast,
		/// You can look into it's implementation to see what's going on
		/// (maybe this function should be virtual
		HK_FORCE_INLINE void addHit(unsigned int id, const unsigned int properties[hkMoppCode::MAX_PRIMITIVE_PROPERTIES]);

};

#endif // HK_COLLIDE2_MOPP_AABB_CAST_VIRTUAL_MACHINE_H

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
