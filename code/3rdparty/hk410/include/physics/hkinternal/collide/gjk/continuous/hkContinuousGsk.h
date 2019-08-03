/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CONTINUOUS_GSK
#define HK_COLLIDE2_CONTINUOUS_GSK

#include <hkinternal/collide/agent3/hkAgent3.h>
#include <hkcollide/shape/convex/hkConvexShape.h>

	// this is minimal timestep. This only affects collisions if
	// a point of an object 
	// at time x				is outside the m_toiDistance
	// at time x+minTimeStep    is outside the m_toiDistance
	// but in between           its inside m_minSeparation
	//
	// This can only happen if you have very high angular velocities
	// Note: Ipion used a value of 1e-3f
class hkGskCache;

struct hk4dGskVertexCollidePointsInput
{
	const hkMotionState* m_motionA;
	const hkMotionState* m_motionB;

	hkVector4*  m_verticesA;
	int			m_numVertices;
	int			m_allocatedNumVertices;

	hkReal m_radiusSum;
	hkReal m_maxAccel;
	hkReal m_invMaxAccel;

	hkVector4   m_planeB;		 // in B space
	hkVector4   m_pointOnPlaneB; // in B space

	const hkStepInfo* m_stepInfo;

	hkReal m_worstCaseApproachingDelta;

	hkVector4  m_linearTimInfo;
	hkVector4  m_deltaAngles[2];

	hkReal m_startRt;	// the relative time to start 
};

struct hk4dGskVertexCollidePointsOutput
{
	hkReal m_Rtoi;
	//hkTime m_toi;
	//hkReal m_seperatingVelocity;
	//hkContactPoint m_toiContact;

	hk4dGskVertexCollidePointsOutput(): m_Rtoi(1.0f){} //m_toi( hkTime(HK_REAL_MAX)){}
};

struct hkProcessCollisionOutput;

struct hk4dGskTolerances
{
	hkReal m_toiSeparation;
	hkReal m_minSeparation;

	hkReal m_minSafeDeltaRt;	// the minimum timestep scaled to [0..1]
	hkReal m_minToiDeltaTime;	    // = 1/maxToiFrequency

	hkReal m_toiAccuracy;
};


extern "C"
{
	void hk4dGskCollidePointsWithPlane( const hk4dGskVertexCollidePointsInput& input, const hk4dGskTolerances& tol, hk4dGskVertexCollidePointsOutput& out );

		/// Find the time of impact.
		/// Rules:
		///    - If we detect a Toi, we return the time when the distance is exactly toiSeparation
		///	   - We will not miss a Toi if the distance goes below minSeparation
		///    - That means  minSeparation < toiSeparation < 0.0f
		///
	void hk4dGskCollideCalcToi( const hkAgent3ProcessInput& in3, const hkReal allowedPenetrationDepth, const hkReal minSeparation, const hkReal toiSeparation, hkGskCache& gskCache, hkVector4& separatingNormal, hkProcessCollisionOutput& output );
}



#endif // HK_COLLIDE2_CONTINUOUS_GSK

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
