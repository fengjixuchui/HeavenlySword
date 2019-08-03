//------------------------------------------------------------------------------------------
//!
//!	\file specialstylebar.cpp
//!
//------------------------------------------------------------------------------------------


// Necessary includes
#include "hud/healthbar.h"
#include "hud/hudmanager.h"
#include "game/combatstyle.h"
#include "objectdatabase/dataobject.h"
#include "game/entitymanager.h"
#include "game/nsmanager.h"
#include "movies/movieinstance.h"
#include "movies/movieplayer.h"
#include "game/shellconfig.h"

#include "gui/guimanager.h"

// ---- Forward references ----

// ---- Interfaces ----
START_STD_INTERFACE( HealthBarRenderDef )
	DEFINE_INTERFACE_INHERITANCE(CBlendableHudUnitDef)
	COPY_INTERFACE_FROM(CBlendableHudUnitDef)

	PUBLISH_VAR_AS( m_fTopLeftX, TopLeftX )
	PUBLISH_VAR_AS( m_fTopLeftY, TopLeftY )
	PUBLISH_VAR_AS( m_fScale, OverallScale )
	PUBLISH_VAR_AS( m_fBackgroundHeight, BackgroundHeight )
	PUBLISH_VAR_AS( m_fBackgroundWidth, BackgroundWidth )
	PUBLISH_VAR_AS( m_fBarMin, BarMin )
	PUBLISH_VAR_AS( m_fBarMax, BarMax )
	PUBLISH_VAR_AS( m_fBarFadeTime, BarFadeTime )
	PUBLISH_VAR_AS( m_fBarGrowTime, BarGrowTime )
	PUBLISH_VAR_AS( m_fAlphaForHealthBar, AlphaForHealthBar )
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obBaseTexture, "hud/ssbossbase_colour_alpha.dds", BaseTexture)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obBarTexture, "hud/ssbossbar_colour_alpha.dds", BarTexture)
	PUBLISH_GLOBAL_ENUM_AS			(m_eBlendMode, BlendMode, EFFECT_BLENDMODE)
END_STD_INTERFACE

void ForceLinkFunctionHealthBar()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionHealthBar() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
//!
//!	HealthBarRenderDef::HealthBarRenderDef
//! Construction
//!
//------------------------------------------------------------------------------------------
HealthBarRenderDef::HealthBarRenderDef( )
:	m_fTopLeftX( 0.1f )
,	m_fTopLeftY( 0.9f )
,	m_fScale( 0.75f )
,	m_fBackgroundHeight( 128.0f )
,	m_fBackgroundWidth( 512.0f )
{	
	m_eBlendMode = EBM_LERP;
}


//------------------------------------------------------------------------------------------
//!
//!	HealthBarRenderDef::~HealthBarRenderDef
//! Destruction
//!
//------------------------------------------------------------------------------------------
HealthBarRenderDef::~HealthBarRenderDef( void )
{
}

//------------------------------------------------------------------------------------------
//!
//!	HealthBarRenderDef::CreateInstance
//! Create instance of renderer
//!
//------------------------------------------------------------------------------------------

CHudUnit* HealthBarRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) HealthBarRenderer( (HealthBarRenderDef*)this );
}

//------------------------------------------------------------------------------------------
//!
//!	HealthBarRenderer::HealthBarRenderer
//! Construction
//!
//------------------------------------------------------------------------------------------
HealthBarRenderer::HealthBarRenderer( HealthBarRenderDef*  pobRenderDef )
:	CBlendableHudUnit ( pobRenderDef )
,	m_pobDef ( pobRenderDef )
,	m_fHealthBlendTime ( 0.0f )
//,	m_fHealthFadeTime ( 0.0f )
,	m_fLastHealth ( 0.0f )
,	m_fAimHealth ( 0.0f )
,	m_fHealthThisFrame ( 0.0f )
,	m_pobCharacter ( 0 )
{
}


//------------------------------------------------------------------------------------------
//!
//!	HealthBarRenderer::~HealthBarRenderer
//! Destruction
//!
//------------------------------------------------------------------------------------------
HealthBarRenderer::~HealthBarRenderer( void )
{
}

bool HealthBarRenderer::Initialise( void )
{
	// Get player
	m_pobCharacter = CEntityManager::Get().GetPlayer();
	if ( m_pobCharacter )
	{
		m_fLastHealth =	m_fAimHealth = m_fHealthThisFrame = m_pobCharacter->GetStartHealth();
	}

	// Set sprite textures
	m_obHealthBaseSprite.SetTexture (m_pobDef->m_obBaseTexture);	
	m_obHealthBarSprite.SetTexture (m_pobDef->m_obBarTexture);

	return true;
}
//------------------------------------------------------------------------------------------
//!
//!	HealthBarRenderer::RenderStyleBar
//!
//------------------------------------------------------------------------------------------
bool HealthBarRenderer::Render( void)
{
	// If deactive - 'cos we should render Enter and Exit states too
	if ( IsDeactive() )
		return true;

	if (! m_pobCharacter )
		return true;

	// Scale our heights and positions accordingly
	float fBBHeight = CGuiManager::Get().BBHeight();
	float fBBWidth = CGuiManager::Get().BBWidth();

	m_fTopLeftX = (m_pobDef->m_fTopLeftX) * fBBWidth;
	m_fTopLeftY = (m_pobDef->m_fTopLeftY) * fBBHeight;
	m_fBackgroundHeight = m_pobDef->m_fBackgroundHeight * fBBHeight;
	m_fBackgroundWidth = m_pobDef->m_fBackgroundWidth * fBBWidth;		

	float fSSBarHeight = m_fBackgroundHeight * m_pobDef->m_fScale;
	float fSSBarWidth = m_fBackgroundWidth * m_pobDef->m_fScale;
	
	CPoint obPosition = CPoint(	m_fTopLeftX, m_fTopLeftY, 0.0f);

	RenderStateBlock::SetBlendMode(m_pobDef->m_eBlendMode);

	float fHealthProgression = 0.0f;
	
	fHealthProgression = m_fHealthThisFrame / m_pobCharacter->GetStartHealth();


	float fBarHeight = m_fBackgroundHeight * m_pobDef->m_fScale;
	float fBarWidth = m_fBackgroundWidth * m_pobDef->m_fScale;

	// Background
	m_obHealthBaseSprite.SetPosition( obPosition );
	m_obHealthBaseSprite.SetHeight(fBarHeight);
	m_obHealthBaseSprite.SetWidth(fBarWidth);
	m_obHealthBaseSprite.SetColour( CVector(1.0f, 1.0f, 1.0f, m_pobDef->m_fAlphaForHealthBar * m_fOverallAlpha) );
	m_obHealthBaseSprite.Render();

	// Bar
	CPoint obBarPos = obPosition;
	float fBarProgression = m_pobDef->m_fBarMin + fHealthProgression * (m_pobDef->m_fBarMax - m_pobDef->m_fBarMin);
	obBarPos.X() -= 0.5f * fSSBarWidth * (1.0f - fBarProgression) ;

	m_obHealthBarSprite.SetPosition( obBarPos );
	m_obHealthBarSprite.SetHeight(fSSBarHeight);
	m_obHealthBarSprite.SetWidth(fSSBarWidth*fBarProgression);
	m_obHealthBarSprite.SetUV(0.0f,0.0f,1.0f,fBarProgression);
	m_obHealthBarSprite.SetColour( CVector(1.0f , 1.0f, 1.0f, m_pobDef->m_fAlphaForHealthBar * m_fOverallAlpha) );
	m_obHealthBarSprite.Render();

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	HealthBarRenderer::Update
//! Update our current settings
//!
//------------------------------------------------------------------------------------------
bool HealthBarRenderer::Update( float fTimeStep )
{
	// Get player - every frame as the main player might change!!!
	m_pobCharacter = CEntityManager::Get().GetPlayer();

	if (! m_pobCharacter )
		return true;

	ntAssert( m_pobDef );
	
	// Update base unit
	CBlendableHudUnit::Update( fTimeStep );

	// If not active - 'cos we shouldn't update while blending
	if ( ! IsActive() )
		return true;

	// Update style blend
	m_fHealthThisFrame = m_fLastHealth;
	if (m_fHealthBlendTime > 0.0f)
	{
		m_fHealthBlendTime  -= fTimeStep;
		
		// bounds check
		if (m_fHealthBlendTime <= 0.0f)
		{
			// reset
			m_fHealthBlendTime = 0.0f;
			m_fLastHealth = m_fAimHealth;
		}

		m_fHealthThisFrame = m_fLastHealth + (m_fAimHealth - m_fLastHealth)*CMaths::SmoothStep(1.0f - m_fHealthBlendTime/m_pobDef->m_fBarGrowTime);
	}

	// Update style fade
	/*if (m_fHealthFadeTime > 0.0f)
	{
		m_fHealthFadeTime  -= fTimeStep;
		
		// bounds check
		if (m_fHealthFadeTime <= 0.0f)
			m_fHealthFadeTime = 0.0f;
	}*/

	// Do we do health grow? 
	if ( (m_fAimHealth != m_pobCharacter->GetCurrHealth()) )
	{
		// Start new grow
		if ( m_fHealthBlendTime <= 0.0f)
		{	
			m_fAimHealth = m_pobCharacter->GetCurrHealth();
		}
		// or add extra amount to current grow
		else
		{
			// Need to include the blend done so far so it dosnt look odd
			m_fLastHealth = m_fLastHealth + (m_fAimHealth - m_fLastHealth)*CMaths::SmoothStep(1.0f - m_fHealthBlendTime/m_pobDef->m_fBarGrowTime);
			m_fAimHealth = m_pobCharacter->GetCurrHealth();
		}
		m_fHealthBlendTime = m_pobDef->m_fBarGrowTime;
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		HealthBarRenderer::UpdateActive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void HealthBarRenderer::UpdateActive( float fTimestep )
{
	CBlendableHudUnit::UpdateActive ( fTimestep );

	if ( !g_ShellOptions->m_bUseHeroHealth )
	{
		if ( m_pobCharacter->IsHero() && m_pobCharacter->ToHero()->HasHeavenlySword() )
			BeginExit();
	}

	if ( NSManager::Get().IsNinjaSequencePlaying() || MoviePlayer::Get().IsActive() )
	{
		BeginExit();	
	}
}

/***************************************************************************************************
*
*	FUNCTION		HealthBarRenderer::UpdateInactive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void HealthBarRenderer::UpdateInactive( float fTimestep )
{
	CBlendableHudUnit::UpdateInactive ( fTimestep );
	
	if ( !NSManager::Get().IsNinjaSequencePlaying() && !MoviePlayer::Get().IsActive() )
	{
		if ( !g_ShellOptions->m_bUseHeroHealth )
		{
			if ( m_pobCharacter->IsHero() && m_pobCharacter->ToHero()->HasHeavenlySword() )
				return;
		}

		BeginEnter();	
	}	
}
