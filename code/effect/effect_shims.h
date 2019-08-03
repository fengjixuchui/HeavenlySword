//--------------------------------------------------
//!
//!	\file effect_shims.h
//!	classes that delay render of effects to the right time
//!
//--------------------------------------------------

#ifndef _EFFECT_SHIM
#define _EFFECT_SHIM

#ifndef _EFFECT_H
#include "effect/effect.h"
#endif

#ifndef GFX_RENDERSTATES_H
#include "gfx/renderstates.h"
#endif

#ifndef GFX_RENDERER_H
#include "gfx/renderer.h"
#endif

#ifndef SCREEN_SPRITE_H
#include "effect/screensprite.h"
#endif

#include "effect/moviesprite.h"

//--------------------------------------------------
//!
//!	Effect_Shim
//! Delays the render of T to the right time, where
//! T is any class that provides the method Render()
//!
//!	[scee_st] C is the Mem::MC chunk type that the T class has been allocated with
//!
//--------------------------------------------------
template<class T, int C> class Effect_Shim : public Effect
{
public:
	Effect_Shim( T* pSprite ) :
		m_bHDR( false ),
		m_bAlpha( false ),
		m_bRendered( false ),
		m_pSprite( pSprite )
	{}

	virtual ~Effect_Shim()
	{
		NT_DELETE_CHUNK( C, m_pSprite );
	}

	virtual bool UpdateEffect() { return m_bRendered; }
	virtual bool WaitingForResources() const { return false; }
	virtual void RenderEffect()
	{
		GFX_ZMODE_TYPE eZ = Renderer::Get().GetZBufferMode();
		if( m_bAlpha )
		{
			Renderer::Get().SetZBufferMode(GFX_ZMODE_DISABLED);
			Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
		}

		m_pSprite->Render();

		if( m_bAlpha )
		{
			Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
			Renderer::Get().SetZBufferMode(eZ);
		}

		m_bRendered = true;
	}

	virtual bool HighDynamicRange() const { return m_bHDR; }
	bool	m_bHDR;
	bool	m_bAlpha;

private:
	bool	m_bRendered;
	T*		m_pSprite;
};

typedef Effect_Shim<ScreenSprite, Mem::MC_MISC>	ScreenSpriteShim;
typedef Effect_Shim<MovieSprite, Mem::MC_MISC> MovieSpriteShim;

#endif
