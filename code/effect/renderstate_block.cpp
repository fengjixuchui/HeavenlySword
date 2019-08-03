//--------------------------------------------------
//!
//!	\file renderstate_block.cpp
//!	Render state management object for effects
//!
//--------------------------------------------------

#include "renderstate_block.h"
#include "gfx/renderer.h"
#include "gfx/renderstates.h"

//--------------------------------------------------
//!
//! RenderStateBlock::SetRenderstates
//!	
//--------------------------------------------------
void RenderStateBlock::SetRenderstates()
{
	if (m_bPointSprite)
		Renderer::Get().SetPointSpriteEnable( true );

	if (m_bDisablePolyCulling)
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	if (m_bZTestEnable)
	{
		if (m_bZWriteEnable)
			Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL );
		else
			Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL_READONLY );			
	}
	else
		Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );

	if (m_bAlphaTestEnable)
		Renderer::Get().SetAlphaTestModeN( ConvertCmpFunc(m_alphaTestFunc), m_fAlphaTestRef );

	SetBlendMode(m_blendMode);
}

//--------------------------------------------------
//!
//! RenderStateBlock::SetRenderstates
//!	
//--------------------------------------------------
void RenderStateBlock::ClearRenderstates()
{
	if (m_bPointSprite)
		Renderer::Get().SetPointSpriteEnable( false );

	if (m_bDisablePolyCulling)
		Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );

	Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL );

	if (m_bAlphaTestEnable)
		Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_NONE );

	SetBlendMode(EBM_OVERWRITE);
}

//--------------------------------------------------
//!
//! RenderStateBlock::SetBlendMode
//!	
//--------------------------------------------------
void RenderStateBlock::SetBlendMode( EFFECT_BLENDMODE mode )
{
	switch ( mode )
	{
		case EBM_OVERWRITE:			Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );	break;
		case EBM_LERP:				Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );			break;
		case EBM_ADD:				Renderer::Get().SetBlendMode( GFX_BLENDMODE_ADD );			break;
		case EBM_ADD_SRCALPHA:		Renderer::Get().SetBlendMode( GFX_BLENDMODE_ADD_SRCALPHA );	break;
		case EBM_SUB:				Renderer::Get().SetBlendMode( GFX_BLENDMODE_SUB );			break;
		case EBM_SUB_SRCALPHA:		Renderer::Get().SetBlendMode( GFX_BLENDMODE_SUB_SRCALPHA );	break;
		case EBM_DISABLED:			Renderer::Get().SetBlendMode( GFX_BLENDMODE_DISABLED );		break;
	}
}
