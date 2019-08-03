//--------------------------------------------------
//!
//!	\file targetcache.h
//!	Renderer helper class
//!
//--------------------------------------------------

#ifndef GFX_TARGET_CACHE_H
#define GFX_TARGET_CACHE_H

#ifndef GFX_SURFACE_H
#include "gfx/surface.h"
#endif

#ifndef GFX_RENDERTARGET_H
#include "gfx/rendertarget.h"
#endif

#if defined( PLATFORM_PC )
typedef Surface ZBuffer;
#elif defined( PLATFORM_PS3 )
typedef RenderTarget ZBuffer;
#endif

//--------------------------------------------------
//!
//! TargetCache
//! class that represents the surfaces in use by the 
//! renderer at any given moment.
//! Also mediates access to device calls.
//!
//--------------------------------------------------
class TargetCache
{
public:
	TargetCache() : m_bSubmitReq( true ) {};

	static const u_int MAX_DRAW_TARGETS = 4;

	void SetColourTarget( RenderTarget::Ptr colour );

	void SetColourAndDepthTargets(	RenderTarget::Ptr colour,
									ZBuffer::Ptr depth );

	void SetMultipleRenderTargets(	RenderTarget::Ptr colour0,
									RenderTarget::Ptr colour1,
									RenderTarget::Ptr colour2,
									RenderTarget::Ptr colour3,
									ZBuffer::Ptr depth );

	// NOTE! only has meaning on PS3
	void ForceFlush() { m_bSubmitReq = true; }

	// accessors to see what is set
	ZBuffer::Ptr		GetDepthTarget() const { return m_depthTarget; }
	RenderTarget::Ptr	GetPrimaryColourTarget() const { return m_colourTargets[0]; }
	RenderTarget::Ptr	GetColourTarget(uint32_t index) const
	{
		ntAssert_p( index < MAX_DRAW_TARGETS, ("Invalid index for colour target") );
		return m_colourTargets[index];
	}

	//! scalars to aide sprite rendering to render targets
	float GetWidthScalar() const;
	float GetHeightScalar() const;

	//! dimensions of current render targets
	float GetWidth() const { return _R(m_iWidth); }
	float GetHeight() const { return _R(m_iHeight); }

	//! the aspect ratuo of current render targets
	float GetAspectRatio() const { return (GetWidth()/GetHeight()); }

private:
	// platform specific internal methods of the target cache
	void SetPrimaryColourTarget( RenderTarget::Ptr colour );
	void SetSecondaryColourTarget( RenderTarget::Ptr colour, uint32_t iIndex );
	void SetDepthTarget( ZBuffer::Ptr depth );
	void SubmitToGPU();

	// the cached colour buffers
	RenderTarget::Ptr	m_colourTargets[MAX_DRAW_TARGETS];

	// the cached depth target
	ZBuffer::Ptr		m_depthTarget;

	// the dimensions of our targets (all MUST match)
	uint32_t			m_iWidth, m_iHeight;

	// used on PS3 to signal a set of render targets is required
	bool				m_bSubmitReq;
};

#endif
