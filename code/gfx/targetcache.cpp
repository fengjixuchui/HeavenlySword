//--------------------------------------------------
//!
//!	\file targetcache.h
//!	platform agnostic portions of target cache.
//!
//--------------------------------------------------

#include "gfx/targetcache.h"
#include "gfx/rendercontext.h"

//-----------------------------------------------------
//!
//! TargetCache::SetColourTarget
//! Set the primary renderer target, clear the other 
//! render targets and the zbuffer.
//!
//-----------------------------------------------------
void TargetCache::SetColourTarget( RenderTarget::Ptr colour )
{
	SetPrimaryColourTarget( colour );

	for (u_int i = 1; i < MAX_DRAW_TARGETS; i++ )
		SetSecondaryColourTarget( RenderTarget::Ptr(), i );

	SetDepthTarget( ZBuffer::Ptr() );
	SubmitToGPU();
}

//-----------------------------------------------------
//!
//! TargetCache::SetColourAndDepthTargets
//! Set the primary renderer target and the depth target.
//! Clear the other targets
//!
//-----------------------------------------------------
void TargetCache::SetColourAndDepthTargets( RenderTarget::Ptr colour,
											ZBuffer::Ptr depth )
{
	SetPrimaryColourTarget( colour );

	for (u_int i = 1; i < MAX_DRAW_TARGETS; i++ )
		SetSecondaryColourTarget( RenderTarget::Ptr(), i );

	// we never accept an empty depth pointer within this call.
	ntAssert_p( depth, ("Must have a valid depth target pointer in SetColourAndDepthTargets()") );

#ifdef PLATFORM_PC
	ntAssert_p( depth->m_Platform.GetSurface(), ("Must have a valid depth target pointer in SetColourAndDepthTargets()") );
#elif defined(PLATFORM_PS3)
	ntAssert_p( depth->m_Platform.GetRenderBuffer(), ("Must have a valid depth target pointer in SetColourAndDepthTargets()") );
#endif

	SetDepthTarget( depth );
	SubmitToGPU();
}

//-----------------------------------------------------
//!
//! TargetCache::SetColourAndDepthTargets
//! Set all targets if required. Only one that must be 
//! valid is the primary target
//!
//-----------------------------------------------------
void TargetCache::SetMultipleRenderTargets(	RenderTarget::Ptr colour0,
											RenderTarget::Ptr colour1,
											RenderTarget::Ptr colour2,
											RenderTarget::Ptr colour3,
											ZBuffer::Ptr depth )
{
	SetPrimaryColourTarget( colour0 );
	SetSecondaryColourTarget( colour1, 1 );
	SetSecondaryColourTarget( colour2, 2 );
	SetSecondaryColourTarget( colour3, 3 );
	SetDepthTarget( depth );
	SubmitToGPU();
}

/***************************************************************************************************
*
*	FUNCTION		TargetCache::GetWidthScalar
*
*	DESCRIPTION		Get a scalar for calculating the pixel size of a world space object.
*
*					This function allows one to calculate the pixel width of a world object.  There 
*					are seperate scalars for each dimension since the pixels of the	rendertarget may not
*					be square.
*
*					So the pixel width of an object may be calculated as:
*
*					                       true width
*					pixel width = ------------------------------ * fWidthScalar
*					              distance of object from camera
*
*					...the same follows for the height using the height scalar value.
*
***************************************************************************************************/
float TargetCache::GetWidthScalar() const
{
	ntAssert_p( RenderingContext::GetIndex() >= 0, ("Cannot call this outside of a render context") );
	float fFieldOfView = RenderingContext::Get()->m_pViewCamera->GetFOVAngle();
	return (_R(m_iWidth) * 0.5f) / ftanf(fFieldOfView * GetAspectRatio() * 0.5f);
}

/***************************************************************************************************
*
*	FUNCTION		TargetCache::GetHeightScalar
*
*	DESCRIPTION		See above
*
***************************************************************************************************/
float TargetCache::GetHeightScalar() const
{
	ntAssert_p( RenderingContext::GetIndex() >= 0, ("Cannot call this outside of a render context") );
	float fFieldOfView = RenderingContext::Get()->m_pViewCamera->GetFOVAngle();
	return (_R(m_iHeight) * 0.5f) / ftanf(fFieldOfView * 0.5f);
}
