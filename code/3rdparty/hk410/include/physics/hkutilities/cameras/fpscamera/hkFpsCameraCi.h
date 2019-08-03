/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_FPS_CAMERA_CI
#define HK_FPS_CAMERA_CI


	/// All the parameters needed to construct a hkFpsCamera
class hkFpsCameraCinfo
{
	public:
		
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CAMERA, hkFpsCameraCinfo);

		hkFpsCameraCinfo() {}

		struct CameraSet
		{

			/// The resting position of the camera in unrotated space. Default: ( 0.0f, 12.0f, 5.5f)
			hkVector4 m_positionUS;

			/// The position in unrotated space. Default: (0, 0, 0)
			hkVector4 m_lookAtUS;

			/// The field of view (f.o.v.) in radians. Default: 1.0
			hkReal m_fov;

			/// m_velocity [m/sec] defines the velocity for which this parameter set gets a weight of 1.0f. Default: 0.0
			/// Normally the hkReal parameters are interpolated from two sets of
			/// parameter sets. The m_velocity parameter of set 1 defines the velocity
			/// for which set 1 gets a weight of 1.0f. At velocity 0 parameter set 1
			/// gets a weight of 1.0f and set 0 a weight of 1.0f.
			hkReal m_velocity;
			

			/// This parameter defines the magnitude of the influence of the velocity for the camera direction. Default 0.01
			/// Normally the camera direction is based on the direction the car. 
			/// However in some situations the camera should also use the velocity
			/// direction. So this parameter is a factor to the current velocity
			/// to be added to the direction of the car to calculate the ideal
			/// camera direction.
			hkReal m_speedInfluenceOnCameraDirection;
		
			/// The angular speed the camera follows the car. Default 4.0
			/// m_angularRelaxation is used to make the yaw angle of the camera follow
			/// the direction of the car. Values from 1.0f to 10.0f are a good choice
			hkReal m_angularRelaxation;

			/// Construct with default values
			CameraSet();
		};

	public:


		/// m_oldFrom is the current eye position in world space.
	hkVector4			m_oldFrom;
	
		/// oldTo is the lookat position in world space.
	hkVector4			m_oldTo;
	
		/// describes the up vector of the camera.
	hkVector4			m_upDir;

		/// used to smooth the motion of the camera by interpolating the new and the old position with this factor.
	hkReal				m_heightInterpolate;
	
	hkReal				m_viewingHeight;

	int					m_smoothingIndex;

	
};

#endif /* HK_FPS_CAMERA_CI */

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
