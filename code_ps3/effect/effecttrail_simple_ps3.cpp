//--------------------------------------------------
//!
//!	\file effecttrail_simple.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "effect/effecttrail_simple.h"
#include "effect/effecttrail_buffer.h"
#include "effect/colour_function.h"
#include "gfx/textureatlas.h"
#include "gfx/levellighting.h"
#include "gfx/renderer.h"

//--------------------------------------------------
//!
//!	draw the thingumy wotsit
//!
//--------------------------------------------------
void EffectTrail_Simple::RenderEffect()
{
	m_renderstates = m_pDef->m_rsDef;
	m_pTrail->PreRender( m_renderstates );
	m_renderstates.SetRenderstates();

	Shader& vertexShader = m_pTrail->GetVertexShader();
	Shader& pixelShader = m_pTrail->GetPixelShader();

	// age
	CVector age( m_fCurrAge, 0.0f, 0.0f, 0.0f );
	vertexShader.SetVSConstantByName( "m_age", age );

	CVector RCPTexPeriod( m_pDef->m_fFadeTime / m_pDef->m_fTexRepeatPeriod, 0.0f, 0.0f, 0.0f );
	
	// textures
	switch( m_pDef->m_eTexMode )
	{
	case TTM_SIMPLE_TEXTURED:
		{
			pixelShader.SetPSConstantByName( "m_Texfactor", RCPTexPeriod );
			
			Renderer::Get().SetTexture( 0, m_pDef->m_pTex );
			Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WCC );
			Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );
		}
		break;

	case TTM_ANIM_TEXTURED:
		{
			pixelShader.SetPSConstantByName( "m_Texfactor", RCPTexPeriod );

			CVector NumTextures( _R(m_pDef->m_pAtlas->GetNumEntries()), 0.0f, 0.0f, 0.0f );
			pixelShader.SetPSConstantByName( "m_TA_numTex", NumTextures );

			CVector TexWidth( m_pDef->m_pAtlas->GetEntryByIndex(0)->GetWidth(), 0.0f, 0.0f, 0.0f );
			pixelShader.SetPSConstantByName( "m_TA_TexWidth", TexWidth );

			Renderer::Get().SetTexture( 0, m_pDef->m_pAtlas->GetAtlasTexture() );
			Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WCC );
			Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );
		}
		break;

	default:
		break;
	}

	Renderer::Get().SetTexture( 1, m_pDef->m_resources.GetFadePalette() );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 1, TEXTUREFILTER_BILINEAR );

	Renderer::Get().SetTexture( 2, m_pDef->m_resources.GetCrossPalette() );
	Renderer::Get().SetSamplerAddressMode( 2, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 2, TEXTUREFILTER_BILINEAR );

	// Time of day global colour modifier
	CVector TODModifier(1.0f,1.0f,1.0f,1.0f);
	if (m_renderstates.m_pTimeOfDayMod)
	{
		float fTime = LevelLighting::Get().GetTimeOfDayN();
		TODModifier = m_renderstates.m_pTimeOfDayMod->GetColour( fTime );
	}

	pixelShader.SetPSConstantByName( "m_TODModifier", TODModifier );

	m_pTrail->Render();
	m_renderstates.ClearRenderstates();

	Renderer::Get().SetTexture( 0, Texture::NONE );
	Renderer::Get().SetTexture( 1, Texture::NONE );
	Renderer::Get().SetTexture( 2, Texture::NONE );
}
