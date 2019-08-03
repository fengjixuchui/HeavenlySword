/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CAMERA_H
#define HK_CAMERA_H

#include <hkmath/hkMath.h>

class hkRigidBody;


/// The hkCamera attaches a camera to a vehicle to aid rendering.
class hkCamera : public hkReferencedObject
{
public:

	HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CAMERA);

		/// Constructor.
	hkCamera (){}

		/// Destructor.
	virtual ~hkCamera(){}

	// Input for the camera calculations
	struct CameraInput
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CAMERA, CameraInput);
		hkVector4		m_linearVelocity;
		hkVector4		m_angularVelocity;
		hkTransform		m_fromTrans;
		hkReal			m_deltaTime;
	};

	// Output of the camera calculations
	struct CameraOutput
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CAMERA, CameraInput);
		hkVector4  m_positionWS;
		hkVector4  m_lookAtWS;
		hkVector4  m_upDirWS;
		hkReal m_fov;
		hkReal m_pad[3];
	};

		/// Virtual, resets the camera to a stable position.
	virtual void resetCamera( const hkTransform& trans, const hkVector4& linearVelocity, const hkVector4& angularVelocity){}
		/// Virtual, calculates the camera configuration for the
		/// current time.
	virtual void calculateCamera ( const CameraInput &in, CameraOutput &out ) = 0;

};


#endif //HK_CAMERA_H

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
