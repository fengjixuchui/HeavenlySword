//------------------------------------------------------------------------------------------
//!
//!	\file specialstylebar.cpp
//!
//------------------------------------------------------------------------------------------


// Necessary includes
#include "hud/specialstylebar.h"
#include "hud/stylelabel.h"
#include "hud/hudmanager.h"
#include "game/combatstyle.h"
#include "objectdatabase/dataobject.h"
#include "game/attackdebugger.h"
#include "game/query.h"
#include "gui/guimanager.h"
#include "game/nsmanager.h"
#include "hud/hudsound.h"

#include "hud/buttonhinter.h"
#include "movies/movieinstance.h"
#include "movies/movieplayer.h"

#include "game/awareness.h"

// ---- Forward references ----

// ---- Interfaces ----
START_STD_INTERFACE( SpecialHitCounterRenderDef )
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
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fInactivityTimeout, 10.0f, InactivityTimeout)
	PUBLISH_VAR_AS( m_fTimeToHint, TimeToHint )
	PUBLISH_VAR_AS( m_fButtonHintRadius, ButtonHintRadius )
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSSBaseTexture, "hud/ssbossbase_colour_alpha.dds", SSBaseTexture)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSSBarTexture, "hud/ssbossbar_colour_alpha.dds", SSBarTexture)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obSSGlowTexture, "hud/ssglow_colour_alpha.dds", SSGlowTexture)
	PUBLISH_GLOBAL_ENUM_AS			(m_eBlendMode, BlendMode, EFFECT_BLENDMODE)
	PUBLISH_PTR_AS( m_pobLabelRenderDef, LabelRenderDef)
END_STD_INTERFACE

void ForceLinkFunctionSpecialStyleBar()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionSpecialStyleBar() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
//!
//!	SpecialHitCounterRenderDef::SpecialHitCounterRenderDef
//! Construction
//!
//------------------------------------------------------------------------------------------
SpecialHitCounterRenderDef::SpecialHitCounterRenderDef( )
:	m_fTopLeftX( 0.1f )
,	m_fTopLeftY( 0.9f )
,	m_fScale( 0.75f )
,	m_fStyleHeight( 128.0f )
,	m_fStyleWidth( 512.0f )
,	m_fTimeToHint	( 30.0f )
,	m_fButtonHintRadius ( 1.0f )
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
}


//------------------------------------------------------------------------------------------
//!
//!	SpecialHitCounterRenderDef::~SpecialHitCounterRenderDef
//! Destruction
//!
//------------------------------------------------------------------------------------------
SpecialHitCounterRenderDef::~SpecialHitCounterRenderDef( void )
{
}

//------------------------------------------------------------------------------------------
//!
//!	SpecialHitCounterRenderDef::CreateInstance
//! Create instance of renderer
//!
//------------------------------------------------------------------------------------------

CHudUnit* SpecialHitCounterRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) SpecialHitCounterRenderer( (SpecialHitCounterRenderDef*)this );
}

//------------------------------------------------------------------------------------------
//!
//!	SpecialHitCounterRenderer::SpecialHitCounterRenderer
//! Construction
//!
//------------------------------------------------------------------------------------------
SpecialHitCounterRenderer::SpecialHitCounterRenderer( SpecialHitCounterRenderDef*  pobRenderDef )
:	CBlendableHudUnit ( pobRenderDef )
,	m_fBlobAlpha( 1.0f )
,	m_fPulseScalar( 1.0f )
,	m_pobDef ( pobRenderDef )
,	m_eLastHitLevel( HL_ZERO )
,	m_fLastStylePoints ( 0.0f )
,	m_fAimStylePoints ( 0.0f )
,	m_fFadeStylePoints ( 0.0f )
,	m_fStyleBlendTime(0.0f )
,	m_fStyleFadeTime( 0.0f )
,	m_fStyleThisFrame ( 0.0f )
,	m_fButtonHintTime( 0.0f )
,	m_bHintActive( false )
{
	m_fGlowAlpha = 0.0f;
}


//------------------------------------------------------------------------------------------
//!
//!	SpecialHitCounterRenderer::~SpecialHitCounterRenderer
//! Destruction
//!
//------------------------------------------------------------------------------------------
SpecialHitCounterRenderer::~SpecialHitCounterRenderer( void )
{
}

bool SpecialHitCounterRenderer::Initialise( void )
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
//!	SpecialHitCounterRenderer::RenderStyleBar
//!
//------------------------------------------------------------------------------------------
bool SpecialHitCounterRenderer::Render( void)
{
	if (! m_pobHitCounter )
		return true;

#ifdef _DEBUG
	g_VisualDebug->Printf2D( 5.0f, 420.0f, DC_RED, 0, "Overall Alpha %f\n", m_fOverallAlpha );
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

	CPoint obBlobPos = obPosition;


	float fSuperStyleProgression = 0.0f;
	//float fFadeFactor = 1.0f;

	fSuperStyleProgression = 
		(float)(m_fStyleThisFrame - m_pobHitCounter->GetCurrentSuperStyleThreshold() ) / 
		(float)(m_pobHitCounter->GetNextSuperStyleThreshold() - m_pobHitCounter->GetCurrentSuperStyleThreshold() );	


	if (m_pobHitCounter->GetCurrentHitLevel() == HL_SPECIAL )
	{
		fSuperStyleProgression = 1.0f;
	}

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
	m_obStyleBaseSprite.SetColour( CVector(1.0f, 1.0f, 1.0f, m_pobDef->m_fAlphaForStyleBar * m_fOverallAlpha) );
	m_obStyleBaseSprite.Render();

	// Bar
	CPoint obBarPos = obBlobPos;
	float fBarProgression = m_pobDef->m_fBarMin + fSuperStyleProgression * (m_pobDef->m_fBarMax - m_pobDef->m_fBarMin);
	obBarPos.X() -= 0.5f * fSSBarWidth * (1.0f - fBarProgression) ;

	m_obStyleBarSprite.SetPosition( obBarPos );
	m_obStyleBarSprite.SetHeight(fSSBarHeight);
	m_obStyleBarSprite.SetWidth(fSSBarWidth*fBarProgression);
	m_obStyleBarSprite.SetUV(0.0f,0.0f,1.0f,fBarProgression);
	m_obStyleBarSprite.SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaForStyleBar * m_fOverallAlpha) );
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
			m_obStyleGlowSprite.SetColour( CVector(1.0,1.0,1.0,m_fBlobAlpha * CMaths::SmoothStep(1.0f - m_fStyleBlendTime/m_pobDef->m_fGlowGrowTime ) * m_fOverallAlpha ) );
		else
			m_obStyleGlowSprite.SetColour( CVector(1.0,1.0,1.0,m_fBlobAlpha * m_fOverallAlpha) );
		
		// Need to lerp it otherwise it gets lost
		m_obStyleGlowSprite.Render();
		
		// Move along screen for next blob
		obBlobPos.X() += fBlobXDelta;
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
//!	SpecialHitCounterRenderer::Update
//! Update our current settings
//!
//------------------------------------------------------------------------------------------
bool SpecialHitCounterRenderer::Update( float fTimeStep )
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
		
	// Update alpha, hdr and pulse deltas
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

	// Update style blend
	m_fStyleThisFrame = m_fLastStylePoints;
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

		m_fStyleThisFrame = m_fLastStylePoints + (m_fAimStylePoints - m_fLastStylePoints)*CMaths::SmoothStep(1.0f - m_fBarBlendTime/m_pobDef->m_fBarGrowTime);
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

		
		// Send my query to the entity manager
		CEntityManager::Get().FindEntitiesByType( obQuery, iEntTypeMask );
		
		bool bDoHint = false;

		// For every hint entity
		for ( ntstd::Vector<CEntity*>::iterator entIt = m_pobHitCounter->m_aobEntList.begin();
				entIt != m_pobHitCounter->m_aobEntList.end(); entIt++ )
		{
			// For every result
			for ( QueryResultsContainerType::iterator resultIt = obQuery.GetResults().begin();
					resultIt != obQuery.GetResults().end(); resultIt++ )
			{
				if ( (*resultIt) == (*entIt) ) 
				{
					bDoHint = true;
					break;
				}
			}

			if ( bDoHint )
				break;
		}

		if ( bDoHint )
		{
			//CHud::Get().GetCombatHUDElements()->m_pobButtonHinter->SetHintForButton(AB_GRAB, true);
			CHud::Get().SetGrabHint( true );
			m_bHintActive = false;
		}
	}

	if (m_eLastHitLevel != eCurrentHitLevel) 
	{
		// If something's changed, start the HDR glow
		//m_fHDR = m_pobDef->m_fHDRForStyleBarHigh;
		
		// Play a sound
		if ( CHud::Exists() && CHud::Get().GetHudSoundManager() )
		{
			CHud::Get().GetHudSoundManager()->PlaySound( CHashedString("StyleLevelSpecial") );
		}

		// Is a new non special level
		if (eCurrentHitLevel != HL_SPECIAL)
		{
			// Oh we've used them all up
			if (eCurrentHitLevel == HL_ZERO)
			{
				m_fStyleFadeTime = m_pobDef->m_fGlowFadeTime;

				m_bHintActive = false;
				//CHud::Get().GetCombatHUDElements()->m_pobButtonHinter->SetHintForButton(AB_GRAB, m_bHintActive);
				CHud::Get().SetGrabHint( false );
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
//!	SpecialHitCounterRenderer::BarPosForStyle( int iStyle )
//! Give the screen position for a specific style point value
//! Helpful for placing the style even labels correctly
//!
//------------------------------------------------------------------------------------------
float SpecialHitCounterRenderer::BarPosForStyle( int iStyle )
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

	if ( fStyleThisFrame > (float)m_pobHitCounter->GetSuperStyleThreshold( HL_SPECIAL ) )
	{
		fStyleThisFrame = (float)m_pobHitCounter->GetSuperStyleThreshold( HL_SPECIAL );
	}

	float fCurrentEffectiveThreshold = (float)m_pobHitCounter->GetSuperStyleThreshold( HL_ZERO );
	float fNextEffectiveThreshold	 = (float)m_pobHitCounter->GetSuperStyleThreshold( HL_SPECIAL );

	float fSuperStyleProgression = 0.0f;
	fSuperStyleProgression = (fStyleThisFrame - fCurrentEffectiveThreshold ) / 
								(fNextEffectiveThreshold - fCurrentEffectiveThreshold );

	// Bar
	CPoint obBarPos (m_fTopLeftX, m_fTopLeftY, 0.0f);
	float fStyleProgression = 0.0f;
	float fBarStart = 0.0f;
	float fBarProgression = 0.0f;

	float fBarPos = 0.0f;

	fStyleProgression = fSuperStyleProgression;

	fBarStart = m_pobDef->m_fBarMin;
	fBarProgression = m_pobDef->m_fBarMin + fStyleProgression * (m_pobDef->m_fBarMax - m_pobDef->m_fBarMin);

	fBarPos = m_fTopLeftX - (0.5f * fSSBarWidth) + ( fBarProgression * fSSBarWidth );

	fBarPos /= fBBWidth;

	return fBarPos;
}

/***************************************************************************************************
*
*	FUNCTION		SpecialHitCounterRenderer::UpdateActive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void SpecialHitCounterRenderer::UpdateActive( float fTimestep )
{
	CBlendableHudUnit::UpdateActive ( fTimestep );

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

		if ( pobCharacter->IsArcher() /*|| ( pobCharacter->IsHero() && !pobCharacter->ToHero()->HasHeavenlySword() )*/ )
		{
			BeginExit();
		}
	}

	if ( NSManager::Get().IsNinjaSequencePlaying() || m_pobHitCounter->GetCurrentStyleProgressionLevel() != HL_SPECIAL 
		|| m_pobHitCounter->GetBossMode() || MoviePlayer::Get().IsActive() )
	{
		BeginExit();	
	}
}

/***************************************************************************************************
*
*	FUNCTION		SpecialHitCounterRenderer::UpdateInactive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void SpecialHitCounterRenderer::UpdateInactive( float fTimestep )
{
	CBlendableHudUnit::UpdateInactive ( fTimestep );

	CEntity* pobEntity = CEntityManager::Get().GetPlayer();
	ntAssert ( pobEntity && pobEntity->IsPlayer() );
	Player* pobCharacter = pobEntity->ToPlayer();

	if ( pobCharacter->IsArcher()  )
	{
		return;
	}
		
	if ( !NSManager::Get().IsNinjaSequencePlaying() && m_pobHitCounter->GetCurrentStyleProgressionLevel() == HL_SPECIAL 
		&& !m_pobHitCounter->GetBossMode() && !MoviePlayer::Get().IsActive() )
	{
		if ( pobCharacter->IsHero() && pobCharacter->GetAwarenessComponent()->IsAwareOfEnemies() )
		{
			BeginEnter();	
			return;
		}

		if (StyleManager::Exists() && StyleManager::Get().HasRenderableEvent( ) )
		{
			BeginEnter();
			return;
		}
	}	
}

/***************************************************************************************************
*
*	FUNCTION		SpecialHitCounterRenderer::BeginExit
*
*	DESCRIPTION		Kicks off the exit state for the unit.
*
***************************************************************************************************/
bool SpecialHitCounterRenderer::BeginExit( bool bForce )
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
