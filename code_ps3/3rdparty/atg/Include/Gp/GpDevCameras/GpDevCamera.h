//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Base Development Camera Class

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_DEV_CAMERA_H
#define GP_DEV_CAMERA_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fw/FwMaths/FwMathsConstants.h>
#include <Fw/FwMaths/FwMathsHelpers.h>
#include <Fw/FwMaths/FwMatrix44.h>
#include <Fw/FwMaths/FwTransform.h>
#include <Fw/FwMaths/FwPoint.h>
#include <Fw/FwMaths/FwVector.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

class GpDevCamera : public FwNonCopyable
{
public:                               

	// Constants
	
	static const float	kDefaultFovy;
	static const float	kDefaultNearClip;
	static const float	kDefaultFarClip;

	
	// Construction & Destruction
	
	GpDevCamera();
	
    virtual ~GpDevCamera();

	
	// Operations

	virtual void	Reset() = 0;
	
	virtual void	InputHandler() = 0;
	
	virtual void	Update(float deltaTime) = 0;
	

	// Access

	FwPoint				GetViewPos() const;
	
	float				GetFovY() const;
	float				GetAspect() const;
	
	float				GetNearClip() const;
	float				GetFarClip() const;
	
	FwVector		   	GetViewDirection() const;
	
	const FwTransform&	GetViewToWorldMatrix() const;
	const FwTransform&	GetWorldToViewMatrix() const;
	
	const FwMatrix44&	GetProjectionMatrix() const;

	
	void				SetFovY(float fovy);
	void				SetAspect(float aspect);
	
	void 				SetNearClip(float nearClip);
	void 				SetFarClip(float farClip);
	
	
protected:

	// Attributes
	
	float			m_fovy;						///< Vertical field of view (in view space Y-Z plane) in radians.
	float			m_aspect;					///< View frustum aspect raito.
	
	float			m_nearClip;					///< Near clipping plane distance (from view position).
	float			m_farClip;					///< Far clipping plane distance (from view position).

	FwTransform		m_viewToWorldMtx;			///< View to world space transform.
	FwTransform		m_worldToViewMtx;			///< World to view space transform.
	FwMatrix44		m_projectionMtx;			///< Projection matrix i.e. view to clip space transform.
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline FwPoint 		GpDevCamera::GetViewPos() const			{ return m_viewToWorldMtx.GetTranslation(); }
inline float		GpDevCamera::GetFovY() const			{ return m_fovy; }
inline float		GpDevCamera::GetAspect() const			{ return m_aspect; }
inline float 		GpDevCamera::GetNearClip() const		{ return m_nearClip; }
inline float 		GpDevCamera::GetFarClip() const			{ return m_farClip; }
inline FwVector 	GpDevCamera::GetViewDirection() const	{ return m_viewToWorldMtx.GetZAxis(); }

//--------------------------------------------------------------------------------------------------

inline const FwTransform&	GpDevCamera::GetViewToWorldMatrix() const	{ return m_viewToWorldMtx; }
inline const FwTransform&	GpDevCamera::GetWorldToViewMatrix() const	{ return m_worldToViewMtx; }
inline const FwMatrix44& 	GpDevCamera::GetProjectionMatrix() const	{ return m_projectionMtx; }

//--------------------------------------------------------------------------------------------------

inline void GpDevCamera::SetFovY(float fovy)
{
	FW_ASSERT((fovy > 0.0f) && (fovy < FwMaths::kPi));
	
	m_fovy = fovy;
}

//--------------------------------------------------------------------------------------------------

inline void GpDevCamera::SetAspect(float aspect)
{
	FW_ASSERT(aspect > 0.0f);
	
	m_aspect = aspect;
}

//--------------------------------------------------------------------------------------------------

inline void GpDevCamera::SetNearClip(float nearClip)
{
	FW_ASSERT((nearClip > 0.0f) && (nearClip < m_farClip));
	
	m_nearClip = nearClip;
}

//--------------------------------------------------------------------------------------------------

inline void GpDevCamera::SetFarClip(float farClip)
{
	FW_ASSERT((farClip > 0.0f) && (farClip > m_nearClip));
	
	m_farClip = farClip;
}

//--------------------------------------------------------------------------------------------------

#endif // GP_DEV_CAMERA_H

