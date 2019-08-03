//--------------------------------------------------
//!
//!	\file targetcache_ps3.h
//!	ps3 side implementation of the target cache object
//!
//--------------------------------------------------

#include "gfx/targetcache.h"
#include "gfx/renderer.h"

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
	ntAssert_p( colour->m_Platform.GetRenderBuffer(), ("Must have a valid render target pointer in SetPrimaryColourTarget()") );

	if (m_colourTargets[0] != colour)
	{
		// set the surface itself
		m_iWidth = colour->GetWidth();
		m_iHeight = colour->GetHeight();

		// cache this
		m_colourTargets[0] = colour;
		m_bSubmitReq = true;
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
		ntAssert_p( colour->m_Platform.GetRenderBuffer(), ("Must have a valid render target pointer in SetSecondaryColourTarget()") );
		ntAssert_p( colour->GetWidth() == m_iWidth, ("Buffer must match dimensions of primary colour target") );
		ntAssert_p( colour->GetHeight() == m_iHeight, ("Buffer must match dimensions of primary colour target") );
	
		m_colourTargets[i] = colour;
		m_bSubmitReq = true;
	}
	else if (m_colourTargets[i])
	{
		// user has requested a clear by providing an empty colour pointer
		m_colourTargets[i].Reset();
		m_bSubmitReq = true;
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
		ntAssert_p( depth->m_Platform.GetRenderBuffer(), ("Must have a valid render target pointer in SetSecondaryColourTarget()") );
		ntAssert_p( depth->GetWidth() == m_iWidth, ("ZBuffer must match dimensions of primary colour target") );
		ntAssert_p( depth->GetHeight() == m_iHeight, ("ZBuffer must match dimensions of primary colour target") );
	
		m_depthTarget = depth;
		m_bSubmitReq = true;
	}
	else if (m_depthTarget)
	{
		// user has requested a clear by providing an empty buffer pointer
		m_depthTarget.Reset();
		m_bSubmitReq = true;
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
	if (m_bSubmitReq)
	{
		m_bSubmitReq = false;
		GcKernel::SetRenderTarget(	m_depthTarget ? m_depthTarget->m_Platform.GetRenderBuffer() : GcRenderBufferHandle(),
									m_colourTargets[0] ? m_colourTargets[0]->m_Platform.GetRenderBuffer() : GcRenderBufferHandle(),
									m_colourTargets[1] ? m_colourTargets[1]->m_Platform.GetRenderBuffer() : GcRenderBufferHandle(),
									m_colourTargets[2] ? m_colourTargets[2]->m_Platform.GetRenderBuffer() : GcRenderBufferHandle(),
									m_colourTargets[3] ? m_colourTargets[3]->m_Platform.GetRenderBuffer() : GcRenderBufferHandle() );


		// I'm not doing any kind of validation on multisampled RTs
		// Obviously we don't want to set the multisampling mode everytime we switch render target and
		// that's only a tentative (and ugly) implementation (marco)
		if (m_colourTargets[0] != NULL)
		{	
			GFXAAMODE eAAMode = m_colourTargets[0]->GetAAMode();
			if (eAAMode == GAA_MULTISAMPLE_4X || eAAMode == GAA_MULTISAMPLE_ORDERED_GRID_4X)
			{
				GcKernel::SetMultisampleParameters(true, false, false, 0xffff);
			}
			else if (eAAMode != GAA_MULTISAMPLE_4X && eAAMode != GAA_MULTISAMPLE_ORDERED_GRID_4X)
			{
				GcKernel::SetMultisampleParameters(false, false, false, 0xffff);
			}
		}
	}
}
