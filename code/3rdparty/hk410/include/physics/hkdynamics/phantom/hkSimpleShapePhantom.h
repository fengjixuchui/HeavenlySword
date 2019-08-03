/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_SIMPLE_SHAPE_PHANTOM_H
#define HK_DYNAMICS2_SIMPLE_SHAPE_PHANTOM_H

#include <hkdynamics/phantom/hkShapePhantom.h>

class hkSimpleStatisticsCollector;

class hkCollisionEnvironment;
class hkCollisionDispatcher;
class hkCollidable;
class hkCollisionAgent;

struct hkLinearCastCollisionInput;
struct hkCollisionInput;
class hkCdPointCollector;
class hkCdBodyPairCollector;

extern const hkClass hkSimpleShapePhantomClass;

/// Please read hkShapePhantom documentation first.<br>
/// The hkSimpleShapePhantom class implements an hkShapePhantom with minimal memory overhead
/// (all collision results are recalculated every time).
/// Because hkSimpleShapePhantom does not cache collision information
/// you may wish to use it (in preference to the hkCachingShapePhantom) if any of the following criteria apply:
///  - You are short of memory.
///  - Your shape is an hkSphereShape or an hkCapsuleShape (caches are usually not so important with spheres and capsules because they create full manifolds against triangles on a single call).
///  - You move the phantom a large distance every frame, so the caches are useless.
class hkSimpleShapePhantom : public hkShapePhantom 
{
	public:

		HK_DECLARE_REFLECTION();

			/// Constructor takes a shape, a transform, and an optional collision filter info
		hkSimpleShapePhantom( const hkShape* shape, const hkTransform& transform, hkUint32 m_collisionFilterInfo = 0 );

		~hkSimpleShapePhantom();

			/// Gets the hkPhantom type. For this class the type is HK_PHANTOM_SIMPLE_SHAPE
		virtual hkPhantomType getType() const;

			// Implementation of hkShapePhantom::setPositionAndLinearCast
		virtual void setPositionAndLinearCast( const hkVector4& position, const hkLinearCastInput& input, hkCdPointCollector& castCollector, hkCdPointCollector* startCollector );

			// Implementation of hkShapePhantom::getClosestPoints
		void getClosestPoints( hkCdPointCollector& collector );

			// Implementation of hkShapePhantom::getPenetrations
		void getPenetrations( hkCdBodyPairCollector& collector );
	
			/// hkPhantom clone functionality
		virtual hkPhantom* clone() const; 

	public:

		//
		// hkPhantom interface
		//
		virtual void addOverlappingCollidable( hkCollidable* collidable );

			// hkPhantom interface implementation
		virtual hkBool isOverlappingCollidableAdded( hkCollidable* collidable );

		virtual void removeOverlappingCollidable( hkCollidable* collidable );

		void calcStatistics( hkStatisticsCollector* collector ) const;

	public:

		struct hkCollisionDetail
		{
			class hkCollidable* m_collidable;
		};

		inline hkArray<struct hkCollisionDetail>& getCollisionDetails();
	
	protected:

		hkArray<struct hkCollisionDetail> m_collisionDetails; //+nosave

	public:
		hkSimpleShapePhantom( class hkFinishLoadedObjectFlag flag ) : hkShapePhantom( flag ) {}

		//
		// INTERNAL USE ONLY
		//

		virtual void deallocateInternalArrays();
};


#include <hkdynamics/phantom/hkSimpleShapePhantom.inl>


#endif //HK_DYNAMICS2_SIMPLE_SHAPE_PHANTOM_H


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
