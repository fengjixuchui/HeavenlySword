//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		ArcBall interface for controlling rotations with the mouse.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_ARC_BALL_H
#define GP_ARC_BALL_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fw/FwMaths/FwQuat.h>
#include <Fw/FwMaths/FwPoint.h>
#include <Fw/FwMaths/FwTransform.h>

//--------------------------------------------------------------------------------------------------
//	SWITCHES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

class GpArcBall : public FwNonCopyable
{
public:                               

	
	// Enumerations
	
	/// Arcball rotation mode.
	enum RotationMode
	{
		kArbitraryRotation,				///< Arcball rotation is unconstrained - it can rotate around an arbitrary axis.
		kYxRotation,					///< Arcball rotation is constrained - it can only rotate around the Y and X axes.
	};
	
	
	/// Arcball radius mode. Describes the relationship of the arcball radius to its bounding rectangle.
	enum RadiusMode
	{
		kWithinBoundingRect,			///< Arcball within the bounding rectangle. Uses manually set radius between [0..1].
		kCoversBoundingRect				///< Arcball covers the bounding rectangle. Uses automatically set radius > 1.0f.
	};
	
	
	// Construction
	
	GpArcBall(	uint			width,
				uint			height,
				float			aspect,
				RotationMode 	rotationMode = kArbitraryRotation,
				RadiusMode		radiusMode = kCoversBoundingRect,
				float			radius = 1.0f);
	
	
	// Operations

	void	BeginDrag(int x, int y);
	void	UpdateDrag(int x, int y);
	void	EndDrag();
	
	void	InputHandler();
	
	void	Reset();
	
	void	SetBoundingRect(uint width, uint height, float aspect, RadiusMode radiusMode = kCoversBoundingRect, float radius = 1.0f);

	// Access

	const FwTransform&	GetRotationMatrix() const;
	
	FwQuat				GetRotationQuat() const;
	
	FwQuat				GetXRotationQuat() const;
	FwQuat				GetYRotationQuat() const;
	

	// Inquiry

	bool	IsBeingDragged() const;
	

private:

	// Attributes
	
	uint			m_width;					///< Width of arcball rectangle (in pixels).
	uint			m_height;					///< Height of arcball rectangle (in pixels).

	float			m_aspect;					///< Aspect ratio of arcball rectangle.
	
	RotationMode	m_rotationMode;				///< Arcball rotation mode.
	
	RadiusMode		m_radiusMode;				///< Relationship of the arcball radius to its bounding rectangle.
	
	float			m_radius;					///< Normalised arcball radius with respect to its bounding rectangle.
	
	float			m_centreX;					///< Centre X of arcball rectangle (in pixels).
	float			m_centreY;					///< Centre Y of arcball rectangle (in pixels).
		
	FwQuat			m_initialQuat;				///< Initial quaternion. Represents rotation when dragging begins.
	FwQuat			m_currentQuat;				///< Current quaternion. Represents total rotation.
	
	FwQuat			m_initialQuatX;				///< Initial quaterion representing X rotation when dragging begins.
	FwQuat			m_initialQuatY;				///< Initial quaterion representing Y rotation when dragging begins.
	FwQuat			m_currentQuatX;				///< Current quaterion representing total X rotation.
	FwQuat			m_currentQuatY;				///< Current quaterion representing total Y rotation.
	
	FwTransform		m_rotationMtx;				///< Resultant arcball orientation.
	
	FwPoint			m_initialPt;				///< Initial point on unit sphere. Set when rotational dragging begins.
	FwPoint			m_currentPt;				///< Current point on unit sphere. Derived from current mouse position.
	
	FwPoint			m_initialPtX;				///< Initial point on unit sphere to compute X rotation.
	FwPoint			m_initialPtY;				///< Initial point on unit sphere to compute Y rotation.
	FwPoint			m_currentPtX;				///< Current point on unit sphere to compute X rotation.
	FwPoint			m_currentPtY;				///< Current point on unit sphere to compute Y rotation.
	
	bool			m_negateYRotation;			///< True if the Y rotation needs negating.
	
	bool			m_isBeingDragged;			///< True if the arcball is being dragged i.e. rotated.
		

	// Operations
	
	FwPoint MapMouseToUnitSphere(float x, float y) const;
	
	FwQuat	QuatFromUnitSpherePoints(FwPoint_arg fromPt, FwPoint_arg toPt) const;
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Get arcball rotation matrix.

	@return		Rotation matrix defining the arcball orientation.
**/
//--------------------------------------------------------------------------------------------------

inline const FwTransform& GpArcBall::GetRotationMatrix() const
{
	return m_rotationMtx;
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief		Get arcball rotation quaternion.

	@return		Unit quaternion defining the total arcball orientation.
**/
//--------------------------------------------------------------------------------------------------
    
inline FwQuat GpArcBall::GetRotationQuat() const
{
	return m_currentQuat;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Get arcball X rotation quaternion.

	@return		Unit quaternion defining the arcball X rotation.
	
	@note		Can only be called when in GpArcBall::kYxRotation mode.
**/
//--------------------------------------------------------------------------------------------------
    
inline FwQuat GpArcBall::GetXRotationQuat() const
{
	FW_ASSERT(m_rotationMode == kYxRotation);
	
	return m_currentQuatX;
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief		Get arcball Y rotation quaternion.

	@return		Unit quaternion defining the arcball Y rotation.
	
	@note		Can only be called when in GpArcBall::kYxRotation mode.
**/
//--------------------------------------------------------------------------------------------------
    
inline FwQuat GpArcBall::GetYRotationQuat() const
{
	FW_ASSERT(m_rotationMode == kYxRotation);
	
	return m_currentQuatY;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Is arcball being dragged?

	@return		True if arcball is being dragged (i.e. rotated).
**/
//--------------------------------------------------------------------------------------------------

inline bool GpArcBall::IsBeingDragged() const
{
	return m_isBeingDragged;
}

//--------------------------------------------------------------------------------------------------

#endif // GP_ARC_BALL_H
