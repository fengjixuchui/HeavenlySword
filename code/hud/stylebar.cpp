//------------------------------------------------------------------------------------------
//!
//!	\file stylebar.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "game/combatstyle.h"
#include "hud/hudmanager.h"
#include "hud/stylebar.h"
#include "hud/stylelabel.h"
#include "objectdatabase/dataobject.h"
#include "game/hitcounter.h"
#include "gui/guimanager.h"
#include "hud/hudsound.h"

#include "gfx/texturemanager.h"
#include "hud/buttonhinter.h"
#include "game/nsmanager.h"

#include "core/visualdebugger.h"
#include "movies/movieinstance.h"
#include "movies/movieplayer.h"

#include "game/awareness.h"

// ---- Forward references ----

// ---- Interfaces ----
START_STD_INTERFACE( StyleBarFillDef )
	PUBLISH_VAR_AS( m_fBarMin, BarMin )
	PUBLISH_VAR_AS( m_fBarMax, BarMax )
END_STD_INTERFACE

START_STD_INTERFACE( HitCounterRenderDef )
	DEFINE_INTERFACE_INHERITANCE(CBlendableHudUnitDef)
	COPY_INTERFACE_FROM(CBlendableHudUnitDef)

	PUBLISH_VAR_AS( m_fTopLeftX, TopLeftX )
	PUBLISH_VAR_AS( m_fTopLeftY, TopLeftY )
	PUBLISH_VAR_AS( m_fScale, OverallScale )
	PUBLISH_VAR_AS( m_fStyleHeight, StyleHeight )
	PUBLISH_VAR_AS( m_fStyleWidth, StyleWidth )
	PUBLISH_VAR_AS( m_fAlphaForLogoFadeHigh, AlphaForLogoFadeHigh )
	PUBLISH_VAR_AS( m_fAlphaForLogoFadeLow, AlphaForLogoFadeLow )
	PUBLISH_VAR_AS( m_fAlphaForLogoFadeDelta, AlphaForLogoFadeDelta )
	PUBLISH_VAR_AS( m_fAlphaForStyleBar, AlphaForStyleBar )
	PUBLISH_VAR_AS( m_fAlphaForLogoActive, AlphaForLogoActive ) 
	PUBLISH_VAR_AS( m_fScalarForLogoPulseHigh, ScalarForLogoPulseHigh )
	PUBLISH_VAR_AS( m_fScalarForLogoPulseLow, ScalarForLogoPulseLow )
	PUBLISH_VAR_AS( m_fScalarForLogoPulseDelta, ScalarForLogoPulseDelta )
	PUBLISH_VAR_AS( m_iMaxBlobs, MaxBlobs)
	PUBLISH_VAR_AS( m_fGlowXDelta[0], GlowXDelta1 )
	PUBLISH_VAR_AS( m_fGlowXDelta[1], GlowXDelta2 )
	PUBLISH_VAR_AS( m_fGlowXDelta[2], GlowXDelta3 )
	PUBLISH_VAR_AS( m_fGlowXDelta[3], GlowXDelta4 )
	PUBLISH_VAR_AS( m_fGlowXDelta[4], GlowXDelta5 )
	PUBLISH_VAR_AS( m_fGlowWidth, GlowWidth )
	PUBLISH_VAR_AS( m_fGlowHeight, GlowHeight )
	PUBLISH_VAR_AS( m_fGlowXOffset, GlowXOffset )
	PUBLISH_VAR_AS( m_fGlowYOffset, GlowYOffset )
	PUBLISH_VAR_AS( m_fGlowPercent, GlowPercent )
	PUBLISH_VAR_AS( m_fGlowFadeTime, GlowFadeTime )
	PUBLISH_VAR_AS( m_fGlowGrowTime, GlowGrowTime )
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fAlphaForBarHigh,		0.8f, AlphaForBarHigh)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fAlphaForBarLow,			0.4f, AlphaForBarLow)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fAlphaForBarDelta,		0.2f, AlphaForBarDelta)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fScalarForBarPulseHigh,	1.2f, ScalarForBarPulseHigh)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fScalarForBarPulseLow,	0.8f, ScalarForBarPulseLow)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fScalarForBarPulseDelta,	0.2f, ScalarForBarPulseDelta)
	PUBLISH_PTR_CONTAINER_AS( m_aobBarDefList, BarDefList)
	PUBLISH_VAR_AS( m_fBarFadeTime, BarFadeTime)
	PUBLISH_VAR_AS( m_fBarGrowTime, BarGrowTime)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fInactivityTimeout, 10.0f, InactivityTimeout)
	

	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSSBaseTexture[HL_ZERO], "hud/ssbase_colour_alpha.dds", SSBaseTexture)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSSBaseTexture[HL_ONE], "hud/ssbase_one_colour_alpha.dds", SSBaseOneTexture)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSSBaseTexture[HL_TWO], "hud/ssbase_two_colour_alpha.dds", SSBaseTwoTexture)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSSBaseTexture[HL_THREE], "hud/ssbase_three_colour_alpha.dds", SSBaseThreeTexture)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSSBaseTexture[HL_FOUR], "hud/ssbase_four_colour_alpha.dds", SSBaseFourTexture)

	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSSBarGlowTexture, "hud/ssbarglow_colour_alpha.dds", SSBarGlowTexture)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSSBarTexture, "hud/ssbar_four_colour_alpha.dds", SSBarTexture)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSSGlowTexture, "hud/ssglow_colour_alpha.dds", SSGlowTexture)
	PUBLISH_PTR_AS( m_pobLabelRenderDef, LabelRenderDef)
	PUBLISH_GLOBAL_ENUM_AS			(m_eBlendMode, BlendMode, EFFECT_BLENDMODE)

	DECLARE_POSTCONSTRUCT_CALLBACK ( PostConstruct )
END_STD_INTERFACE

void ForceLinkFunctionStyleBar()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionStyleBar() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounterRenderDef::HitCounterRenderDef
//! Construction
//!
//------------------------------------------------------------------------------------------
HitCounterRenderDef::HitCounterRenderDef( )
:	m_fTopLeftX( 0.1f )
,	m_fTopLeftY( 0.9f )
,	m_fScale( 1.0f )
,	m_fStyleHeight( 128.0f )
,	m_fStyleWidth( 512.0f )
,	m_iMaxBlobs ( 3 )
,	m_pobLabelRenderDef ( 0 )
{
	// Apply scale given
	m_fStyleHeight *= m_fScale;

	m_fAlphaForLogoFadeHigh = 0.95f;
	m_fAlphaForLogoFadeLow = 0.05f;
	m_fAlphaForLogoFadeDelta = -4.0f;
	m_fAlphaForStyleBar = 0.8f;

	m_fScalarForLogoPulseHigh = 1.2f;
	m_fScalarForLogoPulseLow = 0.8f;
	m_fScalarForLogoPulseDelta = -0.5f;

	m_eBlendMode = EBM_LERP;

	// Lookup table for style level sounds.
	m_aobStyleLevelSounds[HL_ZERO] = CHashedString ("StyleLevelZero");
	m_aobStyleLevelSounds[HL_ONE] = CHashedString ("StyleLevelOne");
	m_aobStyleLevelSounds[HL_TWO] = CHashedString ("StyleLevelTwo");
	m_aobStyleLevelSounds[HL_THREE] = CHashedString ("StyleLevelThree");
	m_aobStyleLevelSounds[HL_FOUR] = CHashedString ("StyleLevelFour");
	m_aobStyleLevelSounds[HL_SPECIAL] = CHashedString ("StyleLevelSpecial");
}


//------------------------------------------------------------------------------------------
//!
//!	HitCounterRenderDef::~HitCounterRenderDef
//! Destruction
//!
//------------------------------------------------------------------------------------------
HitCounterRenderDef::~HitCounterRenderDef( void )
{
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounterRenderDef::CreateInstance
//! Create instance of renderer
//!
//------------------------------------------------------------------------------------------

CHudUnit* HitCounterRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) HitCounterRenderer( (HitCounterRenderDef*)this );
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounterRenderDef::PostConstruct
//! PostConstruct
//!
//------------------------------------------------------------------------------------------
void HitCounterRenderDef::PostConstruct( void )
{	
	if ( !ntStr::IsNull ( m_obSSBarTexture ) )
		TextureManager::Get().LoadTexture_Neutral( m_obSSBarTexture.GetString() );

	if ( !ntStr::IsNull ( m_obSSBarGlowTexture ) )
		TextureManager::Get().LoadTexture_Neutral( m_obSSBarGlowTexture.GetString() );
	
	for (int i = 0; i < STYLE_LEVELS; i++)
	{
		if ( !ntStr::IsNull ( m_obSSBaseTexture[i] ) )
			TextureManager::Get().LoadTexture_Neutral( m_obSSBaseTexture[i].GetString() );
	}

	if ( !ntStr::IsNull ( m_obSSGlowTexture ) )
		TextureManager::Get().LoadTexture_Neutral( m_obSSGlowTexture.GetString() );
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounterRenderer::HitCounterRenderer
//! Construction
//!
//------------------------------------------------------------------------------------------
HitCounterRenderer::HitCounterRenderer( HitCounterRenderDef*  pobRenderDef )
:	CBlendableHudUnit ( pobRenderDef )
,	m_fBlobAlpha( 1.0f )
,	m_fPulseScalar( 1.0f )
,	m_fBarPluseAlpha( 1.0f )
,	m_fBarPulseScalar( 1.0f )
,	m_pobDef ( pobRenderDef )
,	m_eLastHitLevel( HL_ZERO )
,	m_fLastStylePoints ( 0.0f )
,	m_fAimStylePoints ( 0.0f )
,	m_fFadeStylePoints ( 0.0f )
,	m_fBarFillBlendTime ( 0.0f )
,	m_fStyleBlendTime(0.0f )
,	m_fStyleFadeTime( 0.0f )
,	m_fStyleThisFrame ( 0.0f )
{
	for ( int i = 0; i <= STYLE_LEVELS; i++)
		{
			m_fGlowAlpha[i] = 0.0f;
		}
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounterRenderer::~HitCounterRenderer
//! Destruction
//!
//------------------------------------------------------------------------------------------
HitCounterRenderer::~HitCounterRenderer( void )
{
	// Clean out any labels still being rendered
	for (LabelRenderIter obIt = m_aobLabelRenderList.begin(); 
			obIt != m_aobLabelRenderList.end(); ++obIt )
	{
		NT_DELETE_CHUNK (Mem::MC_MISC, *obIt);
	}
	m_aobLabelRenderList.clear();
}

bool HitCounterRenderer::Initialise( void )
{
	// Was done per frame in the old version, I don't think this remains a requirement.
	// Could be moved to Update() if this is incorrect.
	if ( StyleManager::Exists() && StyleManager::Get().GetHitCounter() )
		m_pobHitCounter = StyleManager::Get().GetHitCounter();
	else
		m_pobHitCounter = NULL;

	// Set sprite textures
	m_obStyleBarSprite.SetTexture ( m_pobDef->m_obSSBarTexture );
	if ( !ntStr::IsNull ( m_pobDef->m_obSSBarGlowTexture ) )
	{
		m_obStyleBarGlowSprite.SetTexture ( m_pobDef->m_obSSBarGlowTexture );
	}
	
	m_obStyleGlowSprite.SetTexture ( m_pobDef->m_obSSGlowTexture );

	return true;
}
//------------------------------------------------------------------------------------------
//!
//!	HitCounterRenderer::Render
//!
//------------------------------------------------------------------------------------------
bool HitCounterRenderer::Render( void)
{
	if (! m_pobHitCounter )
		return true;

#ifdef _DEBUG
	g_VisualDebug->Printf2D( 5.0f, 400.0f, DC_RED, 0, "Overall Alpha %f\n", m_fOverallAlpha );
#endif // _DEBUG

	// If deactive - 'cos we should render Enter and Exit states too
	if ( IsDeactive() )
		return true;

	// Scale our heights and positions accordingly
	float fBBHeight = CGuiManager::Get().BBHeight();
	float fBBWidth = CGuiManager::Get().BBWidth();

	m_fTopLeftX = (m_pobDef->m_fTopLeftX) * fBBWidth;
	m_fTopLeftY = (m_pobDef->m_fTopLeftY) * fBBHeight;
	m_fStyleHeight = m_pobDef->m_fStyleHeight * fBBHeight;
	m_fStyleWidth = m_pobDef->m_fStyleWidth * fBBWidth;

	CPoint obPosition = CPoint(	m_fTopLeftX, m_fTopLeftY, 0.0f);

	RenderStateBlock::SetBlendMode(m_pobDef->m_eBlendMode);


	//int iNumBlobs = (int)m_pobHitCounter->GetCurrentStyleProgressionLevel();
	int iNumLogos = (int)m_pobHitCounter->GetCurrentHitLevel();

	CPoint obBlobPos = obPosition;

	
	float fSuperStyleProgression = 0.0f;

	int iEffectiveHitLevel = (int)m_pobHitCounter->GetHitLevel( (int)m_fStyleThisFrame );

	float fCurrentEffectiveThreshold = (float)m_pobHitCounter->GetSuperStyleThreshold( static_cast<HIT_LEVEL>(iEffectiveHitLevel) );
	float fNextEffectiveThreshold	 = (float)m_pobHitCounter->GetSuperStyleThreshold( static_cast<HIT_LEVEL>(iEffectiveHitLevel + 1 ) );

	fSuperStyleProgression = (m_fStyleThisFrame - fCurrentEffectiveThreshold ) / 
								(fNextEffectiveThreshold - fCurrentEffectiveThreshold );

	float fSSBarHeight = m_fStyleHeight * m_pobDef->m_fScale;
	float fSSBarWidth = m_fStyleWidth * m_pobDef->m_fScale;

	float fBlobHeight = m_pobDef->m_fGlowHeight * fBBHeight * m_pobDef->m_fScale;
	float fBlobWidth = m_pobDef->m_fGlowWidth * fBBWidth * m_pobDef->m_fScale;

	// Background - current progression if we have them
	int iStyleProgressionLevel = (int)m_pobHitCounter->GetCurrentStyleProgressionLevel();
	if ( !ntStr::IsNull ( m_pobDef->m_obSSBaseTexture[ iStyleProgressionLevel ] ) )
		m_obStyleBaseSprite.SetTexture( m_pobDef->m_obSSBaseTexture[ iStyleProgressionLevel ].GetString() );
	else
		m_obStyleBaseSprite.SetTexture( m_pobDef->m_obSSBaseTexture[ 0 ].GetString() );

	m_obStyleBaseSprite.SetPosition( obBlobPos );
	m_obStyleBaseSprite.SetHeight(fSSBarHeight);
	m_obStyleBaseSprite.SetWidth(fSSBarWidth);
	m_obStyleBaseSprite.SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaForStyleBar * m_fOverallAlpha) );
	m_obStyleBaseSprite.Render();

	// Bar
	CPoint obBarPos = obBlobPos;
	ntstd::List<StyleBarFillDef*>::iterator obIt = m_pobDef->m_aobBarDefList.begin();
	float fStyleProgression = 0.0f;
	float fBarStart = 0.0f;
	float fBarProgression = 0.0f;

	//g_VisualDebug->Printf2D( 5.0f, 400.0f, DC_RED, 0, "Style last %f, this frame %f, aim %f\n", m_fLastStylePoints, fStyleThisFrame, m_fAimStylePoints );

	for (int i = 1; i <= (int)m_pobDef->m_aobBarDefList.size(); i++)
	{
		ntAssert (obIt != m_pobDef->m_aobBarDefList.end() );

		if ( m_fStyleThisFrame >= (float)m_pobHitCounter->GetSuperStyleThreshold( (HIT_LEVEL)i ) ) // Full level
		{
			fStyleProgression = 1.0f;
		}
		else	// Partial level
		{
			fStyleProgression = fSuperStyleProgression;
			fSuperStyleProgression = 0.0f;
		}

		//g_VisualDebug->Printf2D( 5.0f, 400.0f + i * 20.0f, DC_RED, 0, "Style level %i, Style Progression %f\n", i, fStyleProgression );

		fBarStart = (*obIt)->m_fBarMin;
		fBarProgression = (*obIt)->m_fBarMin + fStyleProgression * ((*obIt)->m_fBarMax - (*obIt)->m_fBarMin);
		obBarPos = obBlobPos;
		obBarPos.X() -= 0.5f * fSSBarWidth * (1.0f - (fBarProgression - fBarStart) );  // Move back to Top Left specified position
		obBarPos.X() += fSSBarWidth * fBarStart;  // Move in by missing amount

		// Bar Sprite
		m_obStyleBarSprite.SetPosition( obBarPos );
		m_obStyleBarSprite.SetHeight(fSSBarHeight);
		m_obStyleBarSprite.SetWidth( fSSBarWidth * (fBarProgression - fBarStart) );
		m_obStyleBarSprite.SetUV(0.0f, fBarStart, 1.0f, fBarProgression);
		m_obStyleBarSprite.SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaForStyleBar * m_fOverallAlpha) );
		m_obStyleBarSprite.Render();

		// Bar Glow sprite - Have texture for glow and is a full level
		if ( !ntStr::IsNull ( m_pobDef->m_obSSBarGlowTexture ) && ( fStyleProgression == 1.0f ) )
		{
			m_obStyleBarGlowSprite.SetPosition( obBarPos );
			m_obStyleBarGlowSprite.SetHeight(fSSBarHeight * m_fBarPulseScalar);
			m_obStyleBarGlowSprite.SetWidth( fSSBarWidth * (fBarProgression - fBarStart) );
			m_obStyleBarGlowSprite.SetUV(0.0f, fBarStart, 1.0f, fBarProgression);
			
			// Last section and newly full
			if (m_fStyleBlendTime > 0.0f && fSuperStyleProgression == 0.0f )
			{
				m_obStyleBarGlowSprite.SetColour( CVector(1.0f , 1.0f, 1.0f, m_fBarPluseAlpha * CMaths::SmoothStep(1.0f - m_fStyleBlendTime/m_pobDef->m_fGlowGrowTime ) * m_fOverallAlpha) );
			}
			else
			{
				m_obStyleBarGlowSprite.SetColour( CVector(1.0f , 1.0f, 1.0f, m_fBarPluseAlpha * m_fOverallAlpha) );
			}
			m_obStyleBarGlowSprite.Render();
		}

		++obIt;
	}

	// Blobs
	obBlobPos.X() -= fSSBarWidth*0.5f;
	obBlobPos.X() += m_pobDef->m_fGlowXOffset * fBBWidth * m_pobDef->m_fScale;
	obBlobPos.Y() += m_pobDef->m_fGlowYOffset * fBBHeight * m_pobDef->m_fScale;

	// Active blobs
	for (int i = 1; i <= iNumLogos; i++)
		{
		m_obStyleGlowSprite.SetPosition( obBlobPos );
		
		// A full blob, pluse it
		if ((i == iNumLogos) && (m_fStyleBlendTime > 0.0f) )
		{
			m_obStyleGlowSprite.SetHeight(fBlobHeight * m_fPulseScalar);
			m_obStyleGlowSprite.SetWidth(fBlobWidth * m_fPulseScalar);
			m_obStyleGlowSprite.SetColour( CVector(1.0,1.0,1.0,m_fBlobAlpha * CMaths::SmoothStep(1.0f - m_fStyleBlendTime/m_pobDef->m_fGlowGrowTime ) * m_fOverallAlpha ) );
		}
		else
		{
			m_obStyleGlowSprite.SetHeight(fBlobHeight * m_fPulseScalar);
			m_obStyleGlowSprite.SetWidth(fBlobWidth * m_fPulseScalar);
			m_obStyleGlowSprite.SetColour( CVector(1.0,1.0,1.0,m_fBlobAlpha * m_fOverallAlpha) );
			m_fGlowAlpha[i] = m_fBlobAlpha;
		}
			
		// Need to lerp it otherwise it gets lost
		if ( i <= m_pobDef->m_iMaxBlobs )
		        m_obStyleGlowSprite.Render();
		
		// Move along screen for next blob
		obBlobPos.X() += m_pobDef->m_fGlowXDelta[i-1] * fBBWidth * m_pobDef->m_fScale;

		m_iFadeBlobs = iNumLogos;
	}

	// Fading blobs
	if (m_fStyleFadeTime > 0.0f)
	{
		for (int i = 1; i <= m_iFadeBlobs; i++)
		{	
			m_obStyleGlowSprite.SetPosition( obBlobPos );		
			m_obStyleGlowSprite.SetHeight(fBlobHeight * m_fPulseScalar);
			m_obStyleGlowSprite.SetWidth(fBlobWidth * m_fPulseScalar);
			m_obStyleGlowSprite.SetColour( CVector(1.0,1.0,1.0,m_fGlowAlpha[i] * CMaths::SmoothStep(m_fStyleFadeTime/m_pobDef->m_fGlowFadeTime) * m_fOverallAlpha) );

			// Need to lerp it otherwise it gets lost
			if ( i <= m_pobDef->m_iMaxBlobs )
			        m_obStyleGlowSprite.Render();
			
			// Move along screen for next blob
			obBlobPos.X() += m_pobDef->m_fGlowXDelta[i-1] * fBBWidth * m_pobDef->m_fScale;
		}
	}

	// Render any style event labels
	LabelRenderIter obEndIt = m_aobLabelRenderList.end();
	for (LabelRenderIter obIt = m_aobLabelRenderList.begin(); obIt != obEndIt; obIt++ )
	{
		CHudUnit* const pobUnit = *obIt;
		
		if (!pobUnit->IsDeactive())
		{
			pobUnit->Render();
		}
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounterRenderer::Update
//! Update our current settings
//!
//------------------------------------------------------------------------------------------
bool HitCounterRenderer::Update( float fTimeStep )
{
	if (!m_pobHitCounter)
		return true;

	ntAssert( m_pobDef );

	// Update base unit
	CBlendableHudUnit::Update( fTimeStep );

	// Update active labels
	for (LabelRenderIter obIt = m_aobLabelRenderList.begin(); obIt != m_aobLabelRenderList.end(); )
	{
		CHudUnit* const pobUnit = *obIt;

		if ( pobUnit->Update(fTimeStep) )
		{
			obIt++;
		}
		else
		{
			NT_DELETE_CHUNK (Mem::MC_MISC, pobUnit);
			obIt = m_aobLabelRenderList.erase( obIt );
		}
	}

	// If not active - 'cos we shouldn't update while blending
	if ( ! IsActive() )
		return true;

	// Update alpha, hdr and pulse deltas for style blobs
	m_fBlobAlpha += m_pobDef->m_fAlphaForLogoFadeDelta*fTimeStep;
	if (m_fBlobAlpha < m_pobDef->m_fAlphaForLogoFadeLow || m_fBlobAlpha > m_pobDef->m_fAlphaForLogoFadeHigh)
	{
		m_pobDef->m_fAlphaForLogoFadeDelta *= -1;
		m_fBlobAlpha > m_pobDef->m_fAlphaForLogoFadeHigh ? m_fBlobAlpha = m_pobDef->m_fAlphaForLogoFadeHigh : m_fBlobAlpha = m_pobDef->m_fAlphaForLogoFadeLow;
	}
	
	m_fPulseScalar += m_pobDef->m_fScalarForLogoPulseDelta*fTimeStep;
	if (m_fPulseScalar < m_pobDef->m_fScalarForLogoPulseLow || m_fPulseScalar > m_pobDef->m_fScalarForLogoPulseHigh)
	{
		m_pobDef->m_fScalarForLogoPulseDelta *= -1;
		m_fPulseScalar > m_pobDef->m_fScalarForLogoPulseHigh ? m_fPulseScalar = m_pobDef->m_fScalarForLogoPulseHigh : m_fPulseScalar = m_pobDef->m_fScalarForLogoPulseLow;
	}

	// Update alpha, hdr and pulse deltas for style bar
	if ( !ntStr::IsNull ( m_pobDef->m_obSSBarGlowTexture ) )
	{
		m_fBarPluseAlpha += m_pobDef->m_fAlphaForBarDelta*fTimeStep;
		if (m_fBarPluseAlpha < m_pobDef->m_fAlphaForBarLow || m_fBarPluseAlpha > m_pobDef->m_fAlphaForBarHigh)
		{
			m_pobDef->m_fAlphaForBarDelta *= -1;
			m_fBarPluseAlpha > m_pobDef->m_fAlphaForBarHigh ? m_fBarPluseAlpha = m_pobDef->m_fAlphaForBarHigh : m_fBarPluseAlpha = m_pobDef->m_fAlphaForBarLow;
		}

		m_fBarPulseScalar += m_pobDef->m_fScalarForBarPulseDelta*fTimeStep;
		if (m_fBarPulseScalar < m_pobDef->m_fScalarForBarPulseLow || m_fBarPulseScalar > m_pobDef->m_fScalarForBarPulseHigh)
		{
			m_pobDef->m_fScalarForBarPulseDelta *= -1;
			m_fBarPulseScalar > m_pobDef->m_fScalarForBarPulseHigh ? m_fBarPulseScalar = m_pobDef->m_fScalarForBarPulseHigh : m_fBarPulseScalar = m_pobDef->m_fScalarForBarPulseLow;
		}
	}


	// Update barfill blend
	m_fStyleThisFrame = m_fLastStylePoints;
	if (m_fBarFillBlendTime > 0.0f)
	{
		m_fBarFillBlendTime  -= fTimeStep;
		
		// bounds check
		if (m_fBarFillBlendTime <= 0.0f)
		{
			// reset
			m_fBarFillBlendTime = 0.0f;
			m_fLastStylePoints = m_fAimStylePoints;
		}

		m_fStyleThisFrame = m_fLastStylePoints + (m_fAimStylePoints - m_fLastStylePoints)*CMaths::SmoothStep(1.0f - m_fBarFillBlendTime/m_pobDef->m_fBarGrowTime);
	}

	// Update style blend
	if (m_fStyleBlendTime > 0.0f)
	{
		m_fStyleBlendTime  -= fTimeStep;
		
		// bounds check
		if (m_fStyleBlendTime <= 0.0f)
			m_fStyleBlendTime = 0.0f;
	}



	// Update style fade
	if (m_fStyleFadeTime > 0.0f)
	{
		m_fStyleFadeTime  -= fTimeStep;
		
		// bounds check
		if (m_fStyleFadeTime <= 0.0f)
			m_fStyleFadeTime = 0.0f;
	}


	HIT_LEVEL eCurrentHitLevel = m_pobHitCounter->GetCurrentHitLevel();

	if (m_eLastHitLevel != eCurrentHitLevel) 
	{
		// If something's changed, start the HDR glow
		//m_fHDR = m_pobDef->m_fHDRForStyleBarHigh;

		// Was it the final level
		if ( (int)m_eLastHitLevel == m_pobDef->m_iMaxBlobs )
		{
			// Remove the looping style level sound
			if ( CHud::Exists() && CHud::Get().GetHudSoundManager() )
			{
				CHud::Get().GetHudSoundManager()->StopSound( m_pobDef->m_aobStyleLevelSounds[ (int)m_eLastHitLevel ] );
			}
		}

		// Oh we've used them all up
		if (eCurrentHitLevel == HL_ZERO)
		{
			m_fStyleFadeTime = m_pobDef->m_fGlowFadeTime;
		}
		else
		{
			// Play a sound for the new level
			if ( ( (int)eCurrentHitLevel > (int)m_eLastHitLevel ) && CHud::Exists() && CHud::Get().GetHudSoundManager() )
			{
				CHud::Get().GetHudSoundManager()->PlaySound( m_pobDef->m_aobStyleLevelSounds[ (int)eCurrentHitLevel ] );
			}

			m_fStyleBlendTime = m_pobDef->m_fGlowGrowTime;
		}
	}

	// Do we do a style grow?
	if ( (m_fAimStylePoints != (float)m_pobHitCounter->GetStylePoints()) )
	{
		// Start new grow
		if ( m_fBarFillBlendTime <= 0.0f)
		{	
			m_fAimStylePoints = (float)m_pobHitCounter->GetStylePoints();
		}
		// or add extra amount to current grow
		else
		{
			// Need to include the blend done so far so it dosnt look odd
			m_fLastStylePoints = m_fLastStylePoints + (m_fAimStylePoints - m_fLastStylePoints)*CMaths::SmoothStep(1.0f - m_fBarFillBlendTime/m_pobDef->m_fBarGrowTime);
			m_fAimStylePoints = (float)m_pobHitCounter->GetStylePoints();
		}
		m_fBarFillBlendTime = m_pobDef->m_fBarGrowTime;
	}

	m_eLastHitLevel = m_pobHitCounter->GetCurrentHitLevel();

	// Is there a style manager? 
	if(StyleManager::Exists() )
	{	
		StyleEvent obStyleEvent;

		// Any events requiring a renderable component?
		while( StyleManager::Get().GetRenderableEvent( obStyleEvent )  )
		{
			// Create renderable for this event
			if (  m_pobDef->m_pobLabelRenderDef )
			{
				StyleLabelRenderer* pobNewLabel = (StyleLabelRenderer*)m_pobDef->m_pobLabelRenderDef->CreateInstance();

				// Position label in the middle of the changing bit of style bar
				float fPos = BarPosForStyle( (int) m_fLastStylePoints );
				fPos += BarPosForStyle( (int) m_fLastStylePoints + obStyleEvent.GetStyleValue() );
				fPos *= 0.5f;

				pobNewLabel->Initialise( obStyleEvent, fPos );
				pobNewLabel->BeginEnter();
				m_aobLabelRenderList.push_back( pobNewLabel );

				// Reset Inactivity Timer
				m_fInactivityTimer = m_pobDef->m_fInactivityTimeout;
			}
		}
	}

	CEntity* pobEntity = CEntityManager::Get().GetPlayer();
	ntAssert ( pobEntity && pobEntity->IsPlayer() );
	Player* pobCharacter = pobEntity->ToPlayer();

	if ( pobCharacter->IsHero() && pobCharacter->GetAwarenessComponent()->IsAwareOfEnemies() )
	{
		// Reset Inactivity Timer
		m_fInactivityTimer = m_pobDef->m_fInactivityTimeout;
	}

	// Update inactivity timer
	if ( m_fInactivityTimer > 0.0f)
		m_fInactivityTimer -= fTimeStep;

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	HitCounterRenderer::BarPosForStyle( int iStyle )
//! Give the screen position for a specific style point value
//! Helpful for placing the style even labels correctly
//!
//------------------------------------------------------------------------------------------
float HitCounterRenderer::BarPosForStyle( int iStyle )
{
	ntAssert ( m_pobHitCounter );

	float fBBWidth = CGuiManager::Get().BBWidth();
	float fSSBarWidth = m_fStyleWidth * m_pobDef->m_fScale;

	
	float fStyleThisFrame = (float) iStyle;

	// Check bounds
	if ( fStyleThisFrame < 0.0f )
	{
		fStyleThisFrame = 0.0f;
	}

	if ( fStyleThisFrame > (float)m_pobHitCounter->GetSuperStyleThreshold( static_cast<HIT_LEVEL>( m_pobDef->m_aobBarDefList.size() ) ) )
	{
		fStyleThisFrame = (float)m_pobHitCounter->GetSuperStyleThreshold( static_cast<HIT_LEVEL>( m_pobDef->m_aobBarDefList.size() ) );
	}

	int iEffectiveHitLevel = (int)m_pobHitCounter->GetHitLevel( (int) fStyleThisFrame );

	float fCurrentEffectiveThreshold = (float)m_pobHitCounter->GetSuperStyleThreshold( static_cast<HIT_LEVEL>(iEffectiveHitLevel) );
	float fNextEffectiveThreshold	 = (float)m_pobHitCounter->GetSuperStyleThreshold( static_cast<HIT_LEVEL>(iEffectiveHitLevel + 1 ) );

	if ( (iEffectiveHitLevel + 1 ) > (int)m_pobDef->m_aobBarDefList.size() )
	{
		fNextEffectiveThreshold = fCurrentEffectiveThreshold;
	}

	float fSuperStyleProgression = 0.0f;
	fSuperStyleProgression = (fStyleThisFrame - fCurrentEffectiveThreshold ) / 
								(fNextEffectiveThreshold - fCurrentEffectiveThreshold );

	// Bar
	
	CPoint obBarPos (m_fTopLeftX, m_fTopLeftY, 0.0f);
	ntstd::List<StyleBarFillDef*>::iterator obIt = m_pobDef->m_aobBarDefList.begin();
	float fStyleProgression = 0.0f;
	float fBarStart = 0.0f;
	float fBarProgression = 0.0f;

	float fBarPos = 0.0f;

	for (int i = 1; i <= (int)m_pobDef->m_aobBarDefList.size(); i++)
	{
		ntAssert (obIt != m_pobDef->m_aobBarDefList.end() );

		if ( fStyleThisFrame >= (float)m_pobHitCounter->GetSuperStyleThreshold( (HIT_LEVEL)i ) ) // Full level
		{
			fStyleProgression = 1.0f;
		}
		else	// Partial level
		{
			fStyleProgression = fSuperStyleProgression;
			fSuperStyleProgression = 0.0f;
		}

		fBarStart = (*obIt)->m_fBarMin;
		fBarProgression = (*obIt)->m_fBarMin + fStyleProgression * ((*obIt)->m_fBarMax - (*obIt)->m_fBarMin);

		fBarPos = m_fTopLeftX - (0.5f * fSSBarWidth) + ( fBarProgression * fSSBarWidth );

		// All done
		if (fSuperStyleProgression == 0.0f)
		{	
			break;
		}

		++obIt;
	}
	fBarPos /= fBBWidth;
	return fBarPos;
}

/***************************************************************************************************
*
*	FUNCTION		HitCounterRenderer::UpdateActive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void HitCounterRenderer::UpdateActive( float fTimestep )
{
	CBlendableHudUnit::UpdateActive ( fTimestep );

	if ( m_pobHitCounter->IsForceActive() )
		return;

	// Remove due to inactivity?
	if (m_fInactivityTimer <= 0.0f)
	{
		BeginExit();
		return;
	}

	// Could remove this with level specific HUD elements and/or script to create and remove HUD elements
	CEntity* pobEntity = CEntityManager::Get().GetPlayer();
	if ( pobEntity )
	{
		ntAssert ( pobEntity->IsPlayer() );

		Player* pobCharacter = pobEntity->ToPlayer();

		if ( pobCharacter->IsArcher() || ( pobCharacter->IsHero() && !pobCharacter->ToHero()->HasHeavenlySword() ) )
		{
			BeginExit();
		}
	}

	if (NSManager::Get().IsNinjaSequencePlaying() || m_pobHitCounter->GetCurrentStyleProgressionLevel() == HL_SPECIAL 
		|| m_pobHitCounter->GetBossMode()  || MoviePlayer::Get().IsActive() )
	{
		BeginExit();	
	}
}

/***************************************************************************************************
*
*	FUNCTION		HitCounterRenderer::UpdateInactive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void HitCounterRenderer::UpdateInactive( float fTimestep )
{
	CBlendableHudUnit::UpdateInactive ( fTimestep );

	CEntity* pobEntity = CEntityManager::Get().GetPlayer();
	ntAssert ( pobEntity && pobEntity->IsPlayer() );
	Player* pobCharacter = pobEntity->ToPlayer();

	if ( pobCharacter->IsArcher() || ( pobCharacter->IsHero() && !pobCharacter->ToHero()->HasHeavenlySword() ) )
	{
		return;
	}
	
	if ( !NSManager::Get().IsNinjaSequencePlaying() && m_pobHitCounter->GetCurrentStyleProgressionLevel() != HL_SPECIAL
		&& !m_pobHitCounter->GetBossMode() && !MoviePlayer::Get().IsActive() )
	{
		if ( ( pobCharacter->IsHero() && pobCharacter->GetAwarenessComponent()->IsAwareOfEnemies() ) ||
		( m_pobHitCounter->IsForceActive() ) ||
		(StyleManager::Exists() && StyleManager::Get().HasRenderableEvent( ) ) )
		{
			BeginEnter();	
			return;
		}
	}	
}

/***************************************************************************************************
*
*	FUNCTION		HitCounterRenderer::BeginExit
*
*	DESCRIPTION		Kicks off the exit state for the unit.
*
***************************************************************************************************/
bool HitCounterRenderer::BeginExit( bool bForce )
{	
    if ( CBlendableHudUnit::BeginExit( bForce ) )
	{
		// Tell active labels to exit
		for (LabelRenderIter obIt = m_aobLabelRenderList.begin(); obIt != m_aobLabelRenderList.end(); ++obIt )
		{
			(*obIt)->BeginExit( true );
		}
	}
	
	return false;
}
