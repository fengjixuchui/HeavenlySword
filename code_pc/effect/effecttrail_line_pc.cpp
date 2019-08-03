//--------------------------------------------------
//!
//!	\file effecttrail_simple.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "effect/effecttrail_line.h"
#include "effect/effecttrail_linebuffer.h"
#include "effect/colour_function.h"
#include "gfx/textureatlas.h"
#include "gfx/levellighting.h"

//--------------------------------------------------
//!
//!	draw the thingumy wotsit
//!
//--------------------------------------------------
void EffectTrail_Line::RenderEffect()
{
	m_renderstates = m_pDef->m_rsDef;
	m_pLine->PreRender( m_renderstates );
	m_renderstates.SetRenderstates();

	ID3DXEffect* pFX = m_pLine->GetEffect();

	// age
	float fAge = m_fCurrAge;
	FX_SET_VALUE_VALIDATE( pFX, "m_age", &fAge, sizeof(float) );

	float fRCPTexPeriod = m_pDef->m_fFadeTime / m_pDef->m_fTexRepeatPeriod;
	FX_SET_VALUE_VALIDATE( pFX, "m_Texfactor", &fRCPTexPeriod, sizeof(float) );
	
	D3DXHANDLE h;

	// textures
	switch( m_pDef->m_eTexMode )
	{
	case TTM_SIMPLE_TEXTURED:
		{
			FX_GET_HANDLE_FROM_NAME( pFX, h, "m_diffuse0" );
			pFX->SetTexture( h, m_pDef->m_pTex->m_Platform.Get2DTexture() );
		}
		break;

	case TTM_ANIM_TEXTURED:
		{
			FX_GET_HANDLE_FROM_NAME( pFX, h, "m_diffuse0" );
			pFX->SetTexture( h, m_pDef->m_pAtlas->GetAtlasTexture()->m_Platform.Get2DTexture() );

			float fNumTextures = _R(m_pAtlas->GetNumEntries());
			float fTexWidth = m_pDef->m_pAtlas->GetEntryByIndex(0)->GetWidth();

			FX_SET_VALUE_VALIDATE( pFX, "m_TA_numTex", &fNumTextures, sizeof(float) );
			FX_SET_VALUE_VALIDATE( pFX, "m_TA_TexWidth", &fTexWidth, sizeof(float) );
		}
		break;
	}

	FX_GET_HANDLE_FROM_NAME( pFX, h, "m_fadePalette" );
	pFX->SetTexture( h, m_pDef->m_resources.GetFadePalette()->m_Platform.Get2DTexture() );

	FX_GET_HANDLE_FROM_NAME( pFX, h, "m_crossPalette" );
	pFX->SetTexture( h, m_pDef->m_resources.GetCrossPalette()->m_Platform.Get2DTexture() );

	// Time of day global colour modifier
	CVector TODModifier(1.0f,1.0f,1.0f,1.0f);
	if (m_renderstates.m_pTimeOfDayMod)
	{
		float fTime = LevelLighting::Get().GetTimeOfDayN();
		TODModifier = m_renderstates.m_pTimeOfDayMod->GetColour( fTime );
	}

	FX_SET_VALUE_VALIDATE( pFX, "m_TODModifier", &TODModifier, sizeof(float)*4 );

	m_pLine->Render();
	m_renderstates.ClearRenderstates();
}
