//--------------------------------------------------------------------------------------------------
/**
	@file		FwTransformHelpers.h

	@brief		methods for setting up transform matrices

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_TRANSFORM_HELPERS_H
#define FW_TRANSFORM_HELPERS_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fw/FwMaths/FwPoint.h>
#include	<Fw/FwMaths/FwVector.h>

//--------------------------------------------------------------------------------------------------
//  SWITCHES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

class FwMatrix44;
class FwTransform;

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@namespace		Fw
**/
//--------------------------------------------------------------------------------------------------

namespace Fw
{

	// projection matrix setting methods

	void		Perspective( FwMatrix44& matrix, float fieldOfViewY, float aspect, float nearClip, float farClip );

	void		Perspective( FwMatrix44& matrix, float fieldOfViewY, float aspect, float nearClip );

	void		Frustum( FwMatrix44& matrix, float left, float right, float bottom, float top, float nearClip, float farClip );

	inline void	Frustum( FwMatrix44& matrix, float width, float height, float nearClip, float farClip );

	void		Ortho( FwMatrix44& matrix, float left, float right, float bottom, float top, float nearClip, float farClip );

	// view matrix setting methods

	void		LookAt( FwTransform& matrix, FwPoint_arg eyePos, FwPoint_arg targetPos, FwVector_arg up=FwMaths::kUnitYAxis );

};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------


namespace Fw
{

//--------------------------------------------------------------------------------------------------
/**
	@brief			symmetric version of Frustum()

	@param			matrix			-	matrix to set
	@param			width			-	width of frustum at near plane
	@param			height			-	height of frustum at near plane
	@param			nearClip		-	near clip distance (positive)
	@param			farClip			-	far clip distance (positive)
**/
//--------------------------------------------------------------------------------------------------

void	Frustum( FwMatrix44& matrix, float width, float height, float nearClip, float farClip )
{
	float w = 0.5f * width;
	float h = 0.5f * height;
	Frustum( matrix, -w, w, -h, h, nearClip, farClip );
}

};

#endif // FW_TRANSFORM_HELPERS_H
