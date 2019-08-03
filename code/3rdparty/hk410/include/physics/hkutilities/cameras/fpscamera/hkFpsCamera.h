/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_FPS_CAMERA_H
#define HK_FPS_CAMERA_H

#include <hkutilities/cameras/hkCamera.h>
#include <hkutilities/cameras/fpscamera/hkFpsCameraCi.h>

/// The hkCamera attaches a first person shooter camera to a character to aid rendering.
class hkFpsCamera : public hkCamera 
{
public:

		/// Constructor.
	hkFpsCamera (const class hkFpsCameraCinfo& fpsci);

		/// Destructor.
	virtual ~hkFpsCamera();

		/// Virtual, resets the camera to a stable position.
	virtual void resetCameraToBody( const hkRigidBody* body);

		/// Virtual, calculates the camera configuration for the
		/// current time.
	virtual void calculateCamera ( const CameraInput &in, CameraOutput &out );

	void	getFullOrientation( hkQuaternion& fullOrientatonOut);

		/// get the look at point in world space
	hkVector4 getLookAtWs(const hkTransform& trans);

		/// the up/down angle of the camera
	hkReal						m_angleY;

		/// the direction of the camera
	hkReal						m_angleX;

	hkVector4					m_oldFrom;

	hkVector4					m_oldTo;

	int							m_smoothingIndex;

	hkVector4					m_upDir;

	// used to smooth the motion of the camera by interpolating the new and the old position with this factor.
	hkReal						m_heightInterpolate;

	hkReal						m_viewingHeight;

	//cached rotation
	hkQuaternion				m_fullOrientaton;

};


#endif //HK_FPS_CAMERA_H

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
