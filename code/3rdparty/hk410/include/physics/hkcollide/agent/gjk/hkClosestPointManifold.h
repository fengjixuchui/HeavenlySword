/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CLOSEST_POINT_MANIFOLD_H
#define HK_COLLIDE2_CLOSEST_POINT_MANIFOLD_H


class hkCdBody;
struct hkProcessCollisionOutput;
class hkContactMgr;

class hkClosestPointManifold
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO, hkClosestPointManifold );

		enum { HK_NUM_MAX_CONTACTS = 4 };


		class hkAgentContactPoint
		{
			public:
					//
					// members
					//
				hkVector4 m_pointA;			// in object space A  .w is the pointA weight
				hkVector4 m_pointB;			// in object space B  .wi is the contactPointId

				hkVector4 m_normal;			// in world space     .w is the distance
				
				//
				//	The next three functions are only to be called after the above hkVector4 have been set
				//
				float& getDistance(){ return m_normal(3); }
				float getDistance() const { return m_normal(3); }

				const hkVector4& getSeparatingNormal() const { return m_normal; }

				float & getPointAWeight(){ return m_pointA(3); }

				unsigned getContactPointId() const { return m_pointB.getInt24W(); }
				void setContactPointId( int id ){ m_pointB.setInt24W( id ); 	}
		};

		inline hkClosestPointManifold( );

			/// Adds a point to the point array. Note: this class does not check for overflows, therefore the maximum number of points is 4
		static void HK_CALL addPoint(  const hkCdBody& ca, const hkCdBody& cb, const hkProcessCollisionInput &input, hkProcessCollisionOutput& output, const struct hkExtendedGskOut& cpInfo, hkReal createContactRangeMax, hkContactMgr* contactMgr, hkCollisionConstraintOwner& constraintOwner, hkAgentContactPoint* pointArray, int& numPoints );
		static void HK_CALL getPoints( const hkCdBody& ca, const hkCdBody& cb, const hkProcessCollisionInput &input, hkReal dist, hkAgentContactPoint* pointArray, int& numPoints, hkProcessCollisionOutput& contactPointsOut, hkContactMgr* contactMgr, hkCollisionConstraintOwner& constraintOwner  );
		static void HK_CALL cleanup( hkAgentContactPoint* pointArray, int& numPoints, hkContactMgr* mgr, hkCollisionConstraintOwner& info );
		static int HK_CALL  findRedundant5thPoint( const hkVector4** points);

	public:
		int	m_numPoints;
		hkAgentContactPoint m_contactPoints[HK_NUM_MAX_CONTACTS];
};

hkClosestPointManifold::hkClosestPointManifold()
{
	m_numPoints = 0;
}

#endif // HK_COLLIDE2_CLOSEST_POINT_MANIFOLD_H

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
