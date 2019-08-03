/***************************************************************************************************
*
*	$Header:: /game/camera.h 9     13/08/03 14:40 Wil                                              $
*
*	A simple camera class to get a debug camera up and running. 
*
*	CHANGES
*
*	05/03/2003	Simon	Created
*
***************************************************************************************************/

#ifndef GFX_CAMERA_H
#define GFX_CAMERA_H

class Transform;

/***************************************************************************************************
*
*	CLASS			CCamera
*
*	DESCRIPTION		Simple camera class for use as a debug camera. Controls the projection only,
*					and should be created with a transform to control the view transform.
*
***************************************************************************************************/

class CCamera
{
public:
	//! Creates an empty camera.
	CCamera();

	//! Sets a new view transform.
	void SetViewTransform(const Transform* pobTransform);

	//! Gets the current view transform.
	const Transform* GetViewTransform() const { return m_pobViewTransform; }

	//! Sets the vertical field of view angle.
	void SetFOVAngle(float fFOVAngle);

	//! Sets the near Z clip plane.
	void SetZNear(float fZNear);

	//! Sets the far Z clip plane.
	void SetZFar(float fZFar);

	//! Gets the vertical field of view angle.
	float GetFOVAngle() const { return m_fFOVAngle; }

	//! Gets the near Z clip plane.
	float GetZNear() const { return m_fZNear; }

	//! Gets the far Z clip plane.
	float GetZFar() const { return m_fZFar; }

	//! Gets the projection from view space to screen space for this camera.
	void GetProjection( float fAspectRatio, CMatrix& proj ) const;
	
	//! transform a screen-Z into an eye-Z
	inline float GetEyeZ(float fScreenZ) const
	{
		return m_fZNear*m_fZFar / (m_fZFar - fScreenZ *(m_fZFar-m_fZNear));
	}

	//! transform a eye-Z into an screen-Z
	inline float GetScreenZ(float fEyeZ) const
	{
		return (1.0f/m_fZNear - 1.0f/ fEyeZ) / (1.0f/m_fZNear - 1.0f/m_fZFar);
	}

	//! return the position of the eye this camera represents
	CPoint GetEyePos() const;

	//! return true if this point is visible from this camera
	bool CanSeePoint( const CPoint& worldSpacePos ) const;

private:
	float m_fFOVAngle;		//!< The vertical field of view angle.
	float m_fZNear;			//!< The near Z clip plane.
	float m_fZFar;			//!< The far Z clip plane.

	const Transform* m_pobViewTransform;	//!< The current view transform.
};

#endif // ndef GFX_CAMERA_H
