//--------------------------------------------------
//!
//!	\file targetcache_pc.h
//!	pcside implementation of the target cache object
//!
//--------------------------------------------------

#include "gfx/targetcache.h"
#include "gfx/graphicsdevice.h"

//-----------------------------------------------------
//!
//! TargetCache::SetPrimaryColourTarget
//! Internal method used by target cache
//!
//-----------------------------------------------------
void TargetCache::SetPrimaryColourTarget( RenderTarget::Ptr colour )
{
	// we never accept an empty colour pointer for the primary surface
	ntAssert_p( colour, ("Must have a valid render target pointer in SetPrimaryColourTarget()") );
	ntAssert_p( colour->m_Platform.GetSurface(), ("Must have a valid render target pointer in SetPrimaryColourTarget()") );

	if (m_colourTargets[0] != colour)
	{
		// set the surface itself
		m_iWidth = colour->GetWidth();
		m_iHeight = colour->GetHeight();
		GetD3DDevice()->SetRenderTarget( 0, colour->m_Platform.GetSurface() );

		// set the texel offset used to finalise pixel positions
		CVector texelAdjustment(-1.0f/_R(m_iWidth), 1.0f/_R(m_iHeight), 0.0f, 0.0f);
		GetD3DDevice()->SetVertexShaderConstantF( 0, reinterpret_cast<float const*>( &texelAdjustment ), 1 );

		// cache this
		m_colourTargets[0] = colour;
	}
}

//-----------------------------------------------------
//!
//! TargetCache::SetSecondaryColourTarget
//! Internal method used by target cache
//!
//-----------------------------------------------------
void TargetCache::SetSecondaryColourTarget( RenderTarget::Ptr colour, uint32_t i )
{
	ntAssert_p( (i > 0) && (i < MAX_DRAW_TARGETS), ("Render target index invalid") );

	if (colour)
	{
		// validate, set and cache
		ntAssert_p( colour->m_Platform.GetSurface(), ("Must have a valid render target pointer in SetSecondaryColourTarget()") );
		ntAssert_p( colour->GetWidth() == m_iWidth, ("Buffer must match dimensions of primary colour target") );
		ntAssert_p( colour->GetHeight() == m_iHeight, ("Buffer must match dimensions of primary colour target") );
	
		GetD3DDevice()->SetRenderTarget( i, colour->m_Platform.GetSurface() );
		m_colourTargets[i] = colour;
	}
	else if (m_colourTargets[i])
	{
		// user has requested a clear by providing an empty colour pointer
		GetD3DDevice()->SetRenderTarget( i, 0 );
		m_colourTargets[i].Reset();
	}
}

//-----------------------------------------------------
//!
//! TargetCache::SetDepthTarget
//! Internal method used by target cache
//!
//-----------------------------------------------------
void TargetCache::SetDepthTarget( ZBuffer::Ptr depth )
{
	if (depth)
	{
		// validate, set and cache
		ntAssert_p( depth->m_Platform.GetSurface(), ("Must have a valid render target pointer in SetSecondaryColourTarget()") );
		ntAssert_p( depth->GetWidth() == m_iWidth, ("ZBuffer must match dimensions of primary colour target") );
		ntAssert_p( depth->GetHeight() == m_iHeight, ("ZBuffer must match dimensions of primary colour target") );
	
		GetD3DDevice()->SetDepthStencilSurface( depth->m_Platform.GetSurface() );
		m_depthTarget = depth;
	}
	else if (m_depthTarget)
	{
		// user has requested a clear by providing an empty buffer pointer
		GetD3DDevice()->SetDepthStencilSurface( 0 );
		m_depthTarget.Reset();
	}
}

//-----------------------------------------------------
//!
//! TargetCache::SubmitToGPU
//! Internal method used by target cache
//!
//-----------------------------------------------------
void TargetCache::SubmitToGPU()
{
	// does nothing on PC, render target sets already submitted.
}
