/***************************************************************************************************
*
*	DESCRIPTION		Simple moon sprite based effect.
*
*	NOTES			This SHOULDNT be rendered with the particles, as they're done in 8-bit, not HDR
*					lighting. Hence, the effect is a sneaky renderable that pokes itself into the
*					renderable list.
*
***************************************************************************************************/

#include "effect/effect_moon.h"
#include "effect/worldsprite.h"
#include "effect/effect_manager.h"

#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/levellighting.h"

#include "objectdatabase/dataobject.h"

START_STD_INTERFACE (CEffectMoonDef)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	IFLOAT		(CEffectMoonDef, AngularSize)
	IFLOAT		(CEffectMoonDef, Luminosity)
	ILIGHT		(CEffectMoonDef, Colour )
END_STD_INTERFACE

void ForceLinkFunction16()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction16() !ATTN!\n");
}

/***************************************************************************************************
*
*	FUNCTION		CEffectMoonDef::Constructor
*
*	DESCRIPTION		setup sensible defaults
*
***************************************************************************************************/
CEffectMoonDef::CEffectMoonDef( void )
{
	m_fAngularSize = 10.0f;
	m_fLuminosity = 12.0f;
	m_obColour = CVector(1.0f,1.0f,1.0f,0.0f);
}

/***************************************************************************************************
*
*	FUNCTION		CEffectMoonDef::Constructor
*
*	DESCRIPTION		auto create an effect once we're loaded
*
***************************************************************************************************/
void	CEffectMoonDef::PostConstruct( void )
{
	// NOTE! EffectMoon adds itself to the effect manager
	NT_NEW_CHUNK ( Mem::MC_EFFECTS ) EffectMoon( this );
}




/***************************************************************************************************
*
*	FUNCTION		EffectMoon::Constructor
*
*	DESCRIPTION		setup the effect
*
***************************************************************************************************/
EffectMoon::EffectMoon( const CEffectMoonDef* pDef ) :
	m_pDef( pDef )
{
	ntAssert( m_pDef );

	// construct our sprite
	m_pMoonSprite = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) WorldSprite;
	m_pMoonSprite->SetTexture( "moon.dds" );

	EffectManager::Get().AddEffect( this );
};

EffectMoon::~EffectMoon()
{
	NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pMoonSprite );
}

/***************************************************************************************************
*
*	FUNCTION		EffectMoon::UpdateEffect
*
*	DESCRIPTION		Update the position and size of the sprite
*
***************************************************************************************************/
bool	EffectMoon::UpdateEffect()
{
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		EffectMoon::RenderEffect
*
*	DESCRIPTION		render
*
***************************************************************************************************/
void	EffectMoon::RenderEffect()
{
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	// moon is essentially infinity far away, so we'll just work out where it should be relative to the 
	// current camera (in the direction of the moon light source, and its size for some large distance away
	CDirection toMoon = LevelLighting::Get().GetDirectionToMoon();

	// now, we're gonna put the moon way out of reach of the landscape, say 5K
	float fFarFarAway = 5000.0f;
	CPoint position =	RenderingContext::Get()->GetEyePos() +
						(toMoon * fFarFarAway);

	m_pMoonSprite->SetPosition( position );
	
	// now size up the sprite to be the right size
	float fAngularSize = m_pDef->m_fAngularSize * DEG_TO_RAD_VALUE;
	m_pMoonSprite->SetSize( fFarFarAway * ftanf( fAngularSize ) );

	// now set the right colour
	CVector	col = m_pDef->m_obColour * m_pDef->m_fLuminosity;
	col.W() = 1.0f;
	m_pMoonSprite->SetColour( col );

	m_pMoonSprite->Render();

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
}

