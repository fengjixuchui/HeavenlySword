/***************************************************************************************************
*
*	DESCRIPTION		Simple gui sprite based effect.
*
*	NOTES			This SHOULDNT be rendered with the particles, as they're done in 8-bit, not HDR
*					lighting. Hence, the effect is a sneaky renderable that pokes itself into the
*					renderable list.
*
***************************************************************************************************/

#include "guieffect.h"
#include "effect/worldsprite.h"
#include "effect/effect_manager.h"

#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/levellighting.h"

#include "objectdatabase/dataobject.h"

/*START_STD_INTERFACE (CEffectMoonDef)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	IFLOAT		(CEffectMoonDef, AngularSize)
	IFLOAT		(CEffectMoonDef, Luminosity)
	ILIGHT		(CEffectMoonDef, Colour )
END_STD_INTERFACE

void ForceLinkFunction16()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction16() !ATTN!\n");
}*/

/***************************************************************************************************
*
*	FUNCTION		CEffectMoonDef::Constructor
*
*	DESCRIPTION		setup sensible defaults
*
***************************************************************************************************/
/*CEffectMoonDef::CEffectMoonDef( void )
{
	m_fAngularSize = 10.0f;
	m_fLuminosity = 12.0f;
	m_obColour = CVector(1.0f,1.0f,1.0f,0.0f);
}*/

/***************************************************************************************************
*
*	FUNCTION		CEffectMoonDef::Constructor
*
*	DESCRIPTION		auto create an effect once we're loaded
*
***************************************************************************************************/
/*void	CEffectMoonDef::PostConstruct( void )
{
	// NOTE! EffectMoon adds itself to the effect manager
	NT_NEW_CHUNK( Mem::MC_GFX ) EffectMoon( this );
}
*/



/***************************************************************************************************
*
*	FUNCTION		EffectGui::Constructor
*
*	DESCRIPTION		setup the effect
*
***************************************************************************************************/
EffectGui::EffectGui( WorldSprite*	pWorldSprite ) :
	m_pGuiSprite ( pWorldSprite )
{
	ntAssert( pWorldSprite );

	m_iEffectId = EffectManager::Get().AddEffect( this );
};

EffectGui::~EffectGui()
{
	//EffectManager::Get().KillEffectWhenReady( m_iEffectId );
	//EffectManager::Get().KillEffectNow( m_iEffectId );
}

/***************************************************************************************************
*
*	FUNCTION		EffectGui::UpdateEffect
*
*	DESCRIPTION		Update the position and size of the sprite
*
***************************************************************************************************/
bool	EffectGui::UpdateEffect()
{
	Effect* pEffect = EffectManager::Get().GetEffect( m_iEffectId );

	if ((m_bKillMeRequested) && (pEffect))
		pEffect->KillMeWhenReady();

	if ((m_bKillMeNow) && (pEffect))
		pEffect->KillMeNow();

	if ((!pEffect) || (m_bKillMeNow) || (m_bKillMeRequested))
		return true;

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		EffectGui::RenderEffect
*
*	DESCRIPTION		render
*
***************************************************************************************************/
void	EffectGui::RenderEffect()
{

	Effect* pEffect = EffectManager::Get().GetEffect( m_iEffectId );

	if (pEffect)
	{
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

		m_pGuiSprite->Render();

		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
	}
}

