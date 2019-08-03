//--------------------------------------------------
//!
//!	\file ShadowGatherer.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef GFX_SHADOW_GATHERER_H
#define GFX_SHADOW_GATHERER_H

#ifndef GFX_RENDERTARGET_H
#include "gfx/rendertarget.h"
#endif

#ifndef GFX_RENDERERSETTINGS_H
#include "gfx/renderersettings.h"
#endif

class ShadowGatherer_impl;

//--------------------------------------------------------------------------------------------------------
//!
//!	ShadowGatherer
//! gather a per pixel occlusion term coming from multiple shadow maps drawing a full screen quad.
//!
//--------------------------------------------------------------------------------------------------------
class ShadowGatherer
{
public:
	ShadowGatherer( void );
	~ShadowGatherer( void );

	void GatherOcclusionTerm ( const Texture::Ptr& depthTexture );

private:
	ShadowGatherer_impl* m_pImpl;
};

#endif // GFX_SHADOW_GATHERER_H
