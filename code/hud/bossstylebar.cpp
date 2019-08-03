//------------------------------------------------------------------------------------------
//!
//!	\file bossstylebar.cpp
//!
//------------------------------------------------------------------------------------------


// Necessary includes
#include "hud/hudmanager.h"
#include "game/combatstyle.h"
#include "hud/bossstylebar.h"
#include "objectdatabase/dataobject.h"
#include "game/attackdebugger.h"
#include "game/query.h"
#include "gui/guimanager.h"

#include "game/nsmanager.h"

#include "movies/movieinstance.h"
#include "movies/movieplayer.h"

#include "hud/buttonhinter.h"

// ---- Forward references ----

// ---- Interfaces ----
START_STD_INTERFACE( BossHitCounterRenderDef )
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
	PUBLISH_VAR_AS( m_fGlowXDelta, GlowXDelta )
	PUBLISH_VAR_AS( m_fGlowWidth, GlowWidth )
	PUBLISH_VAR_AS( m_fGlowHeight, GlowHeight )
	PUBLISH_VAR_AS( m_fGlowXOffset, GlowXOffset )
	PUBLISH_VAR_AS( m_fGlowYOffset, GlowYOffset )
	PUBLISH_VAR_AS( m_fBarMin, BarMin )
	PUBLISH_VAR_AS( m_fBarMax, BarMax )
	PUBLISH_VAR_AS( m_fGlowFadeTime, GlowFadeTime )
	PUBLISH_VAR_AS( m_fGlowGrowTime, GlowGrowTime )
	PUBLISH_VAR_AS( m_fBarFadeTime, BarFadeTime )
	PUBLISH_VAR_AS( m_fBarGrowTime, BarGrowTime )
	PUBLISH_VAR_AS( m_fTimeToHint, TimeToHint )
	PUBLISH_VAR_AS( m_fButtonHintRadius, ButtonHintRadius )

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_obSSGlowTexture, "hud\\ssglow_colour_alpha_nomip.dds", SSGlowTexture );
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_obSSBaseTexture, "hud\\ssbossbase_colour_alpha_nomip.dds", SSBaseTexture );
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_obSSBarTexture, "hud\\ssbossbar_colour_alpha_nomip.dds", SSBarTexture );

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_obStyleLevelSound, "inc_stlevel_10", StyleLevelSound );

	PUBLISH_GLOBAL_ENUM_AS			(m_eBlendMode, BlendMode, EFFECT_BLENDMODE)
END_STD_INTERFACE

void ForceLinkFunctionBossStyleBar()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionSpecialStyleBar() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
//!
//!	SpecialHitCounterRenderDef::SpecialHitCounterRenderDef
//! Construction
//!
//------------------------------------------------------------------------------------------
BossHitCounterRenderDef::BossHitCounterRenderDef( )
:	m_fTopLeftX( 0.1f )
,	m_fTopLeftY( 0.9f )
,	m_fScale( 0.75f )
,	m_fStyleHeight( 128.0f )
,	m_fStyleWidth( 512.0f )
,	m_fTimeToHint	( 30.0f )
,	m_fButtonHintRadius ( 1.0f )
,	m_pobEntity ( 0 )
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
}


//------------------------------------------------------------------------------------------
//!
//!	BossHitCounterRenderDef::~BossHitCounterRenderDef
//! Destruction
//!
//------------------------------------------------------------------------------------------
BossHitCounterRenderDef::~BossHitCounterRenderDef( void )
{
}

//------------------------------------------------------------------------------------------
//!
//!	SpecialHitCounterRenderDef::CreateInstance
//! Create instance of renderer
//!
//------------------------------------------------------------------------------------------

CHudUnit* BossHitCounterRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) BossHitCounterRenderer( (BossHitCounterRenderDef*)this );
}

//------------------------------------------------------------------------------------------
//!
//!	BossHitCounterRenderer::BossHitCounterRenderer
//! Construction
//!
//------------------------------------------------------------------------------------------
BossHitCounterRenderer::BossHitCounterRenderer( BossHitCounterRenderDef*  pobRenderDef )
:  CBlendableHudUnit( pobRenderDef )	
,	m_fAlpha( 1.0f )
,	m_fPulseScalar( 1.0f )
,	m_pobDef ( pobRenderDef )
,	m_eLastHitLevel( HL_ZERO )
,	m_fLastStylePoints ( 0.0f )
,	m_fAimStylePoints ( 0.0f )
,	m_fFadeStylePoints ( 0.0f )
,	m_fStyleBlendTime(0.0f )
,	m_fStyleFadeTime( 0.0f )
,	m_fButtonHintTime( 0.0f )
,	m_bHintActive( false )
{
	m_fGlowAlpha = 0.0f;
}


//------------------------------------------------------------------------------------------
//!
//!	BossHitCounterRenderer::~BossHitCounterRenderer
//! Destruction
//!
//------------------------------------------------------------------------------------------
BossHitCounterRenderer::~BossHitCounterRenderer( void )
{
}

bool BossHitCounterRenderer::Initialise( void )
{
	// Was done per frame in the old version, I don't think this remains a requirement.
	// Could be moved to Update() if this is incorrect.
	if ( StyleManager::Exists() && StyleManager::Get().GetHitCounter() )
		m_pobHitCounter = StyleManager::Get().GetHitCounter();
	else
		m_pobHitCounter = NULL;

	// Set sprite textures
	m_obStyleBaseSprite.SetTexture ((m_pobDef->m_obSSBaseTexture));	
	m_obStyleBarSprite.SetTexture ((m_pobDef->m_obSSBarTexture));
	m_obStyleGlowSprite.SetTexture((m_pobDef->m_obSSGlowTexture));

	return true;
}
//------------------------------------------------------------------------------------------
//!
//!	BossHitCounterRenderer::RenderStyleBar
//!
//------------------------------------------------------------------------------------------
bool BossHitCounterRenderer::Render( void)
{
	if ( !m_pobHitCounter )
		return false;
	if ( IsDeactive() )
		return false;

	CPoint obPosition = CPoint(	m_fTopLeftX, m_fTopLeftY, 0.0f);

	RenderStateBlock::SetBlendMode(m_pobDef->m_eBlendMode);

	// Scale our heights and positions accordingly
	float fBBHeight = CGuiManager::Get().BBHeight();
	float fBBWidth = CGuiManager::Get().BBWidth();

	CPoint obBlobPos = obPosition;

	float fStyleThisFrame = m_fLastStylePoints;
	float fSuperStyleProgression = 0.0f;
	//float fFadeFactor = 1.0f;

	
	//Grow
	if (m_fBarBlendTime > 0.0f )
	{
		fStyleThisFrame = m_fLastStylePoints + (m_fAimStylePoints - m_fLastStylePoints)*CMaths::SmoothStep(1.0f - m_fBarBlendTime/m_pobDef->m_fBarGrowTime);
	}
	
	float fCurrentSuperStyleThreshold = (float)m_pobHitCounter->GetCurrentSuperStyleThreshold();
	float fNextSuperStyleThreshold = (float)m_pobHitCounter->GetNextSuperStyleThreshold();

	fSuperStyleProgression = 
		(float)( fStyleThisFrame - fCurrentSuperStyleThreshold ) / 
		(float)( fNextSuperStyleThreshold - fCurrentSuperStyleThreshold );	

	obBlobPos.X() = m_fTopLeftX;
	obBlobPos.Y() = m_fTopLeftY;

	float fSSBarHeight = m_fStyleHeight * m_pobDef->m_fScale;
	float fSSBarWidth = m_fStyleWidth * m_pobDef->m_fScale;

	float fBlobXDelta = m_pobDef->m_fGlowXDelta * fBBWidth * m_pobDef->m_fScale;
	float fBlobHeight = m_pobDef->m_fGlowHeight * fBBHeight * m_pobDef->m_fScale;
	float fBlobWidth = m_pobDef->m_fGlowWidth * fBBWidth * m_pobDef->m_fScale;

	// Background
	m_obStyleBaseSprite.SetPosition( obBlobPos );
	m_obStyleBaseSprite.SetHeight(fSSBarHeight);
	m_obStyleBaseSprite.SetWidth(fSSBarWidth);
	m_obStyleBaseSprite.SetColour( CVector(1.0f, 1.0f, 1.0f, m_pobDef->m_fAlphaForStyleBar) );
	m_obStyleBaseSprite.Render();

	// Bar
	CPoint obBarPos = obBlobPos;
	float fBarProgression = m_pobDef->m_fBarMin + fSuperStyleProgression * (m_pobDef->m_fBarMax - m_pobDef->m_fBarMin);
	obBarPos.X() -= 0.5f * fSSBarWidth * (1.0f - fBarProgression) ;

	m_obStyleBarSprite.SetPosition( obBarPos );
	m_obStyleBarSprite.SetHeight(fSSBarHeight);
	m_obStyleBarSprite.SetWidth(fSSBarWidth*fBarProgression);
	m_obStyleBarSprite.SetUV(0.0f,0.0f,1.0f,fBarProgression);
	m_obStyleBarSprite.SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaForStyleBar) );
	m_obStyleBarSprite.Render();

	obBlobPos.X() -= fSSBarWidth*0.5f;
	obBlobPos.X() += m_pobDef->m_fGlowXOffset * fBBWidth * m_pobDef->m_fScale;
	obBlobPos.Y() += m_pobDef->m_fGlowYOffset * fBBHeight * m_pobDef->m_fScale;

	// A full blob, pluse it
	if (m_pobHitCounter->GetCurrentHitLevel() == HL_SPECIAL )
	{
		m_obStyleGlowSprite.SetPosition( obBlobPos );

		m_obStyleGlowSprite.SetHeight(fBlobHeight * m_fPulseScalar);
		m_obStyleGlowSprite.SetWidth(fBlobWidth * m_fPulseScalar);
		if (m_fStyleBlendTime > 0.0f)
			m_obStyleGlowSprite.SetColour( CVector(1.0,1.0,1.0,m_fAlpha * CMaths::SmoothStep(1.0f - m_fStyleBlendTime/m_pobDef->m_fGlowGrowTime ) ) );
		else
			m_obStyleGlowSprite.SetColour( CVector(1.0,1.0,1.0,m_fAlpha) );
		
		// Need to lerp it otherwise it gets lost
		m_obStyleGlowSprite.Render();
		
		// Move along screen for next blob
		obBlobPos.X() += fBlobXDelta;
	}

	return true;
}



//------------------------------------------------------------------------------------------
//!
//!	BossHitCounterRenderer::Update
//! Render our current settings
//!
//------------------------------------------------------------------------------------------
bool BossHitCounterRenderer::Update( float fTimeStep )
{
	if ( !m_pobHitCounter )
		return true;

	// Update base unit
	CBlendableHudUnit::Update( fTimeStep );

	if ( IsDeactive() )
		return true;
	
	ntAssert( m_pobDef );
	
	// Scale our heights and positions accordingly
	float fBBHeight = CGuiManager::Get().BBHeight();
	float fBBWidth = CGuiManager::Get().BBWidth();
	
	m_fScale = (fBBHeight/567.0f) * m_pobDef->m_fScale;

	m_fTopLeftX = (m_pobDef->m_fTopLeftX) * fBBWidth;
	m_fTopLeftY = (m_pobDef->m_fTopLeftY) * fBBHeight;
	m_fStyleHeight = m_pobDef->m_fStyleHeight * fBBHeight;
	m_fStyleWidth = m_pobDef->m_fStyleWidth * fBBWidth;
		
	// Update alpha, hdr and pulse deltas
	m_fAlpha += m_pobDef->m_fAlphaForLogoFadeDelta*fTimeStep;
	if (m_fAlpha < m_pobDef->m_fAlphaForLogoFadeLow || m_fAlpha > m_pobDef->m_fAlphaForLogoFadeHigh)
	{
		m_pobDef->m_fAlphaForLogoFadeDelta *= -1;
		m_fAlpha > m_pobDef->m_fAlphaForLogoFadeHigh ? m_fAlpha = m_pobDef->m_fAlphaForLogoFadeHigh : m_fAlpha = m_pobDef->m_fAlphaForLogoFadeLow;
	}
	
	m_fPulseScalar += m_pobDef->m_fScalarForLogoPulseDelta*fTimeStep;
	if (m_fPulseScalar < m_pobDef->m_fScalarForLogoPulseLow || m_fPulseScalar > m_pobDef->m_fScalarForLogoPulseHigh)
	{
		m_pobDef->m_fScalarForLogoPulseDelta *= -1;
		m_fPulseScalar > m_pobDef->m_fScalarForLogoPulseHigh ? m_fPulseScalar = m_pobDef->m_fScalarForLogoPulseHigh : m_fPulseScalar = m_pobDef->m_fScalarForLogoPulseLow;
	}

	// Update style blend
	if (m_fBarBlendTime > 0.0f)
	{
		m_fBarBlendTime  -= fTimeStep;
		
		// bounds check
		if (m_fBarBlendTime <= 0.0f)
		{
			// reset
			m_fBarBlendTime = 0.0f;
			m_fLastStylePoints = m_fAimStylePoints;
		}
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

	// Update button hint
	if (m_fButtonHintTime > 0.0f)
	{
		m_fButtonHintTime  -= fTimeStep;
		
		// bounds check
		if (m_fButtonHintTime <= 0.0f)
		{
			m_fButtonHintTime = 0.0f;
			m_bHintActive = true;
		}
	}


	HIT_LEVEL eCurrentHitLevel = m_pobHitCounter->GetCurrentHitLevel();

	// NS button hint?
	if (m_bHintActive && eCurrentHitLevel == HL_SPECIAL)
	{
		// Only allow the query to execute on AI's and players
		int iEntTypeMask = CEntity::EntType_Character;

		CEQCProximitySphere obSphereSub;
		obSphereSub.Set ( CEntityManager::Get().GetPlayer()->GetPosition(), m_pobDef->m_fButtonHintRadius);

		// Create my query object
		CEntityQuery obQuery;
		obQuery.AddClause( obSphereSub );

		// Exclude player
		//obQuery.AddExcludedEntity( CEntityManager::Get().GetPlayer() );

		CEQCIsThis obTarget( m_pobDef->m_pobEntity );
		obQuery.AddClause( obTarget );

		// Send my query to the entity manager
		CEntityManager::Get().FindEntitiesByType( obQuery, iEntTypeMask );

		if (obQuery.GetResults().front() == m_pobDef->m_pobEntity) 
			CHud::Get().GetCombatHUDElements()->m_pobButtonHinter->SetHintForButton(AB_GRAB, true);
		else
			CHud::Get().GetCombatHUDElements()->m_pobButtonHinter->SetHintForButton(AB_GRAB, false);

	}

	if (m_eLastHitLevel != eCurrentHitLevel) 
	{
		// If something's changed, start the HDR glow
		//m_fHDR = m_pobDef->m_fHDRForStyleBarHigh;
		
		// Play a sound

		unsigned int id;

		if (AudioSystem::Get().Sound_Prepare(id,"env_sb", ntStr::GetString(m_pobDef->m_obStyleLevelSound) ) )
		{
			AudioSystem::Get().Sound_Play(id);
		}

		// Is a new non special level
		if (eCurrentHitLevel != HL_SPECIAL)
		{
			// Oh we've used them all up
			if (eCurrentHitLevel == HL_ZERO)
			{
				m_fStyleFadeTime = m_pobDef->m_fGlowFadeTime;

				m_bHintActive = false;
				CHud::Get().GetCombatHUDElements()->m_pobButtonHinter->SetHintForButton(AB_GRAB, m_bHintActive);
			}
			else
			{
				m_fStyleBlendTime = m_pobDef->m_fGlowGrowTime;
			}
		}
	}

	// Do we do style grow? 
	if ( (m_fAimStylePoints != (float)m_pobHitCounter->GetStylePoints()) )
	{

		// Start new grow
		if ( m_fStyleBlendTime <= 0.0f)
		{	
			m_fAimStylePoints = (float)m_pobHitCounter->GetStylePoints();
		}
		// or add extra amount to current grow
		else
		{
			// Need to include the blend done so far so it dosnt look odd
			m_fLastStylePoints = m_fLastStylePoints + (m_fAimStylePoints - m_fLastStylePoints)*CMaths::SmoothStep(1.0f - m_fStyleBlendTime/m_pobDef->m_fBarGrowTime);
			m_fAimStylePoints = (float)m_pobHitCounter->GetStylePoints();
		}
		m_fBarBlendTime = m_pobDef->m_fBarGrowTime;
	}

	m_eLastHitLevel = m_pobHitCounter->GetCurrentHitLevel();

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		HitCounterRenderer::UpdateActive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void BossHitCounterRenderer::UpdateActive( float fTimestep )
{
	CBlendableHudUnit::UpdateActive ( fTimestep );

	if (!m_pobHitCounter->GetBossMode())
	{
		BeginExit();	
	}

	if (NSManager::Get().IsNinjaSequencePlaying() || MoviePlayer::Get().IsActive() )
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
void BossHitCounterRenderer::UpdateInactive( float fTimestep )
{
	CBlendableHudUnit::UpdateInactive ( fTimestep );
	
	if (m_pobHitCounter->GetBossMode() && !NSManager::Get().IsNinjaSequencePlaying() && !MoviePlayer::Get().IsActive() )
	{
		BeginEnter();	
	}	
}

/***************************************************************************************************
*
*	FUNCTION		HitCounterRenderer::BeginExit
*
*	DESCRIPTION		Kicks off the exit state for the unit.
*
***************************************************************************************************/
bool BossHitCounterRenderer::BeginExit( bool bForce )
{	
	CBlendableHudUnit::BeginExit( bForce );
	return true;
}

