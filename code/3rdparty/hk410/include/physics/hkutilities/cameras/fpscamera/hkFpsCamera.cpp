/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkutilities/cameras/fpscamera/hkFpsCamera.h>

hkFpsCamera::hkFpsCamera (const class hkFpsCameraCinfo& fpsci)
{
	m_heightInterpolate = fpsci.m_heightInterpolate;
	m_viewingHeight = fpsci.m_viewingHeight;
	m_oldFrom = fpsci.m_oldFrom;
	m_oldTo = fpsci.m_oldTo;
	m_upDir = fpsci.m_upDir;
	m_angleY = 0.f;
	m_angleX = 0.f;
	m_smoothingIndex = fpsci.m_smoothingIndex;

}

hkFpsCamera::~hkFpsCamera()
{

}

void hkFpsCamera::resetCameraToBody( const hkRigidBody* body)
{

}

hkVector4 hkFpsCamera::getLookAtWs(const hkTransform& trans)
{
	hkVector4 newTo = trans.getTranslation();
	const hkVector4& forwardVec = trans.getRotation().getColumn(0);
	newTo.add4(forwardVec);
	return newTo;
}

void hkFpsCamera::calculateCamera ( const CameraInput &in, CameraOutput &out ) 
{
	const hkVector4& infrom = in.m_fromTrans.getTranslation();

	const hkVector4 inTo = getLookAtWs(in.m_fromTrans);
	
	//camera up axis is interpolated to get a smoother stair climbing
	//todo: optimize
	switch (m_smoothingIndex)
	{
	case 0:
		{
			

			m_oldFrom(0) = m_oldFrom(0) * (1.f - m_heightInterpolate) + m_heightInterpolate * (infrom(0)+m_viewingHeight);
			m_oldFrom(1) = infrom(1);
			m_oldFrom(2) = infrom(2);
			m_oldTo(0) = m_oldTo(0) * (1.f - m_heightInterpolate) + m_heightInterpolate * (inTo(0)+m_viewingHeight);
			m_oldTo(1) = inTo(1);
			m_oldTo(2) = inTo(2);
			break;
		}
	case 1:
		{
			m_oldFrom(0) = infrom(0);
			m_oldFrom(1) = m_oldFrom(1) * (1.f - m_heightInterpolate) + m_heightInterpolate * (infrom(1)+m_viewingHeight);
			m_oldFrom(2) = infrom(2);
			m_oldTo(0) = inTo(0);
			m_oldTo(1) = m_oldTo(1) * (1.f - m_heightInterpolate) + m_heightInterpolate * (inTo(1)+m_viewingHeight);
			m_oldTo(2) = inTo(2);
			break;
		}
	case 2:
		{
			m_oldFrom(0) = infrom(0);
			m_oldFrom(1) = infrom(1);
			m_oldFrom(2) = m_oldFrom(2) * (1.f - m_heightInterpolate) + m_heightInterpolate * (infrom(2)+m_viewingHeight);
			m_oldTo(0) = inTo(0);
			m_oldTo(1) = inTo(1);
			m_oldTo(2) = m_oldTo(2) * (1.f - m_heightInterpolate) + m_heightInterpolate * (inTo(2)+m_viewingHeight);
			break;
		}
	default:
		{
			HK_ASSERT2(0x163f9149, m_smoothingIndex < 2,"Error in hkFpsCamera: smoothingIndex should be either 0,1 or 2.");
		}
	}
	
	out.m_lookAtWS = m_oldFrom;

	hkVector4 dir;
	dir.setSub4(m_oldTo, m_oldFrom);

	hkVector4 sideways;
	sideways.setCross(m_upDir,dir);
	sideways.normalize3();
	m_fullOrientaton = hkQuaternion(sideways, m_angleY);

	dir.setRotatedDir(m_fullOrientaton ,dir);

	out.m_lookAtWS.setAdd4(m_oldFrom, dir);

	out.m_positionWS = m_oldFrom;
	
	out.m_upDirWS = m_upDir;
		
}

	//cached rotation
void hkFpsCamera::getFullOrientation( hkQuaternion& fullOrientation)
{
	fullOrientation = m_fullOrientaton;
}
	

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
