//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Viewer Camera - Similar to Maya's default camera.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_VIEWER_CAMERA_H
#define GP_VIEWER_CAMERA_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gp/GpDevCameras/GpArcBall.h>
#include <Gp/GpDevCameras/GpDevCamera.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

class GpViewerCamera : public GpDevCamera
{
public:                               

	// Constants
	
	static const float	kDefaultDollyDistance;
	static const float	kDefaultDollyScalar;
	
	static const float	kDefaultPanningScalar;
	
	
	// Construction & Destruction
	
	GpViewerCamera(FwPoint_arg focusPos, float dolly, uint width, uint height, float aspect);
	
	virtual ~GpViewerCamera();

	
	// Operations

	virtual void	Reset();
	
	virtual void	InputHandler();
	
	virtual void	Update(float deltaTime = 0.0f);
	
	
	// Access

	FwPoint		GetFocusPos() const;
	void		SetFocusPos(FwPoint_arg focusPos);
	
	float		GetDollyDistance() const;
	void		SetDollyDistance(float d);

	float		GetDollyScalar() const;
	void		SetDollyScalar(float s);

	float		GetPanningScalar() const;
	void		SetPanningScalar(float s);
	
	GpArcBall&	GetViewRotArcBall();
	
				   
private:

	// Attributes
	
	FwPoint		m_focusPos;						///< World space position the camera is focusing on.
	float		m_dolly;						///< Camera dolly signed distance (in world space units).

	bool		m_isSpinning;					///< True if camera is spinning.
	bool		m_isDollying;					///< True if camera is dollying.
	bool		m_isPanning;					///< True if camera is panning.
	
	float		m_dollyScalar;					///< Dolly update scalar.
	float		m_panningScalar;				///< Panning update scalar.
	
	GpArcBall	m_viewRotArcBall;				///< View rotation arcball.
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline FwPoint		GpViewerCamera::GetFocusPos() const					{	return m_focusPos;			}
inline void			GpViewerCamera::SetFocusPos(FwPoint_arg focusPos)	{	m_focusPos = focusPos;		}
inline float		GpViewerCamera::GetDollyDistance() const 			{	return m_dolly;				}
inline void 		GpViewerCamera::SetDollyDistance(float d)			{	m_dolly = d;				}
inline float		GpViewerCamera::GetDollyScalar() const	 			{	return m_dollyScalar;		}
inline void 		GpViewerCamera::SetDollyScalar(float s)				{	m_dollyScalar = s;			}
inline float		GpViewerCamera::GetPanningScalar() const			{	return m_panningScalar;		}
inline void 		GpViewerCamera::SetPanningScalar(float s)			{	m_panningScalar = s;		}
inline GpArcBall&	GpViewerCamera::GetViewRotArcBall()					{	return m_viewRotArcBall;	}

//--------------------------------------------------------------------------------------------------

#endif // GP_VIEWER_CAMERA_H

