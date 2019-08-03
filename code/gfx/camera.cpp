/***************************************************************************************************
*
*	$Header:: /game/camera.cpp 22    13/08/03 14:40 Wil                                            $
*
*	A simple camera class to get a debug camera up and running. 
*
*	CHANGES
*
*	05/03/2003	Simon	Created
*
***************************************************************************************************/

#include "gfx/camera.h"
#include "gfx/renderer.h"

#include "anim/transform.h"

/***************************************************************************************************
*
*	FUNCTION		CCamera::CCamera
*
*	DESCRIPTION		Creates a default camera at the origin.
*
***************************************************************************************************/

CCamera::CCamera() 
  : m_fFOVAngle(PI/4.0f), 
	m_fZNear(0.1f), 
	m_fZFar(10000.0f),
	m_pobViewTransform(0)
{}

void CCamera::SetViewTransform(const Transform* pobTransform)
{
	m_pobViewTransform = pobTransform;
}

/***************************************************************************************************
*
*	FUNCTION		CCamera::SetFOVAngle
*
*	DESCRIPTION		Sets the vertical field of view angle.
*
***************************************************************************************************/

void CCamera::SetFOVAngle(float fFOVAngle)
{
	// clamp the FOV angle to [5, 175] degrees.
	m_fFOVAngle = ntstd::Clamp(fFOVAngle, (5.0f/360.0f)*TWO_PI, (175.0f/360.0f)*TWO_PI);
}

/***************************************************************************************************
*
*	FUNCTION		CCamera::SetZNear
*
*	DESCRIPTION		Sets the near Z clip plane.
*
***************************************************************************************************/

void CCamera::SetZNear(float fZNear)
{
	m_fZNear = fZNear;
}

/***************************************************************************************************
*
*	FUNCTION		CCamera::SetZFar
*
*	DESCRIPTION		Sets the far Z clip plane.
*
***************************************************************************************************/

void CCamera::SetZFar(float fZFar)
{
	m_fZFar = fZFar;
}

/***************************************************************************************************
*
*	FUNCTION		CCamera::GetProjection
*
*	DESCRIPTION		Calculate a projection matrix based on the current camera
*					properties and an input aspect ratio.
*
***************************************************************************************************/
void CCamera::GetProjection( float fAspectRatio, CMatrix& proj ) const
{
	proj.Perspective(m_fFOVAngle, fAspectRatio, m_fZNear, m_fZFar);
}

/***************************************************************************************************
*
*	FUNCTION		CCamera::GetEyePos
*
*	DESCRIPTION		return the position of the eye this camera represents
*
***************************************************************************************************/
CPoint CCamera::GetEyePos() const
{
	ntAssert_p( GetViewTransform(), ("Invalid camera, I have no transform!") );
	return GetViewTransform()->GetWorldMatrix().GetTranslation();
}

/***************************************************************************************************
*
*	FUNCTION		CCamera::CanSeePoint
*
*	DESCRIPTION		return true if this point is visible from this camera
*
***************************************************************************************************/
bool CCamera::CanSeePoint( const CPoint& worldSpacePos ) const
{
	CMatrix currWorld = GetViewTransform()->GetWorldMatrix();
	CPoint test = worldSpacePos - currWorld.GetTranslation();
	
	if (test.Dot( CPoint(currWorld.GetZAxis()) ) > 0.0f)
	{
		// transform point to viewport space (-1.0f to 1.0f)
		CMatrix proj;
		GetProjection( 1.0f, proj );

		CVector temp( worldSpacePos * currWorld.GetAffineInverse() );
		temp.W() = 1.0f;
		temp = temp * proj;

		// deal with points at infinity. sigh
		if( temp.W() < EPSILON )
			temp = CVector( 0.0f, 0.0f, -1.0f, 1.0f );
		else
			temp /= fabsf(temp.W());

		// return if its visible
		if	(
			((temp.X() >= -1.0f) && (temp.X() <= 1.0f)) &&
			((temp.Y() >= -1.0f) && (temp.Y() <= 1.0f))
			)
			return true;
	}
	return false;
}

