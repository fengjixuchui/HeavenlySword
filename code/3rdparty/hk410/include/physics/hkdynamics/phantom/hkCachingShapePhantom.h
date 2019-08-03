/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_CACHING_SHAPE_PHANTOM_H
#define HK_DYNAMICS2_CACHING_SHAPE_PHANTOM_H

#include <hkdynamics/phantom/hkShapePhantom.h>

class hkSimpleStatisticsCollector;

class hkCollisionEnvironment;
class hkCollisionDispatcher;
class hkCollidable;
class hkCollisionAgent;

class hkCachingShapePhantomCinfo;
struct hkLinearCastCollisionInput;
struct hkCollisionInput;
class hkCdPointCollector;
class hkCdBodyPairCollector;

extern const hkClass hkCachingShapePhantomClass;

/// This class represents a phantom with an arbitrary shape for query purposes.
/// Please read hkShapePhantom and the hkGskBaseAgent documentation first.<br>
/// This implementation of hkShapePhantom creates collision agents for all potential colliding pairs.
/// It therefore uses more memory than the uncached version (hkSimpleShapePhantom), but can perform
/// collision detection functions up to 40% faster when lots of GJK/GSK agents are involved.<br>
/// Note: In landscapes, caching the linear cast means converting the cast into an extended aabb
/// (the original aabb of the shape plus the path).
/// Therefore long linear casts can result in a huge cached aabb and many many potential triangles as
/// collision partners. The hkSimpleShapePhantom can actually use an optimized direct linear cast for mopp
/// objects.
class hkCachingShapePhantom : public hkShapePhantom 
{
	public:

		HK_DECLARE_REFLECTION();

			/// Constructor takes a shape, a transform, and an optional collision filter info
		hkCachingShapePhantom( const hkShape* shape, const hkTransform& transform, hkUint32 m_collisionFilterInfo = 0 );

		~hkCachingShapePhantom();

			/// Gets the hkPhantom type. For this class the type is HK_PHANTOM_CACHING_SHAPE
		virtual hkPhantomType getType() const;

			/// Implementation of hkShapePhantom::setPositionAndLinearCast
		virtual void setPositionAndLinearCast( const hkVector4& position, const hkLinearCastInput& input, hkCdPointCollector& castCollector, hkCdPointCollector* startCollector );
			
			/// hkPhantom clone functionality
		virtual hkPhantom* clone() const;

			/// Implementation of hkShapePhantom::getClosestPoints
		void getClosestPoints( hkCdPointCollector& collector );

			/// Implementation of hkShapePhantom::getPenetrations
		void getPenetrations( hkCdBodyPairCollector& collector );

		void calcStatistics( hkStatisticsCollector* collector ) const;

	public:

		struct hkCollisionDetail
		{
			hkCollisionAgent* m_agent;
			hkCollidable*     m_collidable;
		};

		inline hkArray<struct hkCollisionDetail>& getCollisionDetails();

		//
		// hkPhantom interface
		//

		virtual void addOverlappingCollidable( hkCollidable* collidable );

		virtual void removeOverlappingCollidable( hkCollidable* collidable );

		virtual hkBool isOverlappingCollidableAdded( hkCollidable* collidable );

		virtual void updateShapeCollectionFilter();

	protected:

		hkArray<struct hkCollisionDetail> m_collisionDetails; //+nosave

	public:
		hkCachingShapePhantom( class hkFinishLoadedObjectFlag flag ) : hkShapePhantom( flag ) {}

		//
		// INTERNAL USE ONLY
		//

		virtual void deallocateInternalArrays();
};

#include <hkdynamics/phantom/hkCachingShapePhantom.inl>


#endif //HK_DYNAMICS2_CACHING_SHAPE_PHANTOM_H


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
