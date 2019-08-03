//------------------------------------------------------------------------------------------
//!
//!	\file lifeclock.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "hud/lifeclock.h"
#include "objectdatabase/dataobject.h"
#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gui/guimanager.h"
#include "game/combatstyle.h"
#include "game/nsmanager.h"
#include "movies/movieinstance.h"
#include "movies/movieplayer.h"

#ifdef _DEBUG
#	include "core/visualdebugger.h"
#endif

// ---- Forward references ----



// ---- Interfaces ----
START_CHUNKED_INTERFACE( LifeClockRenderDef, Mem::MC_MISC )
	DEFINE_INTERFACE_INHERITANCE(CBlendableHudUnitDef)
	COPY_INTERFACE_FROM(CBlendableHudUnitDef)

	PUBLISH_VAR_AS		(m_fTopLeftX, TopLeftX)
	PUBLISH_VAR_AS		(m_fTopLeftY, TopLeftY)
	PUBLISH_VAR_AS		(m_fOverallScale, OverallScale)

	PUBLISH_VAR_AS		(m_fFadeSpeed, FadeSpeed)

	PUBLISH_VAR_AS		(m_obTimeToLivePos, TimeToLivePos)
	PUBLISH_VAR_AS		(m_fTimeToLiveWidth, TimeToLiveWidth)
	PUBLISH_VAR_AS		(m_fTimeToLiveHeight, TimeToLiveHeight)
	PUBLISH_VAR_AS		(m_obTimeToLiveColour, TimeToLiveColour)
	PUBLISH_VAR_AS		(m_obTimeToLiveGlowColour, TimeToLiveGlowColour)
	
	PUBLISH_GLOBAL_ENUM_AS		(m_eTimeToLiveBlendmode, TimeToLiveBlendmode, EFFECT_BLENDMODE)
	PUBLISH_GLOBAL_ENUM_AS		(m_eTimeToLiveGlowBlendmode, TimeToLiveGlowBlendmode, EFFECT_BLENDMODE)
	PUBLISH_VAR_AS		(m_obTimerPos, TimerPos)

	PUBLISH_PTR_AS		(m_pobTimerRenderDef, TimerRenderDefinition)
	
END_STD_INTERFACE

void ForceLinkFunctionLifeClock()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionLifeClock() !ATTN!\n");
}

///////////////////////////////////////////
//
// LifeclockRenderDef
//
///////////////////////////////////////////
LifeClockRenderDef::LifeClockRenderDef()
: m_obTimerPos ( CONSTRUCT_CLEAR )
{
	// FIX ME expose these strings

	// Set textures for all sprites
	m_aobTimeToLive.SetTexture("hud\\T2L_colour_alpha_nomip.bmp");
	
	m_fTopLeftX = 0.5f;
	m_fTopLeftY = 0.5f;

	m_fOverallScale = 1.0f;
}

CHudUnit* LifeClockRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) LifeClockRenderer( (LifeClockRenderDef*)this );
}


///////////////////////////////////////////
//
// LifeclockRenderer
//
///////////////////////////////////////////
LifeClockRenderer::LifeClockRenderer( LifeClockRenderDef*  pobRenderDef) 
:	CBlendableHudUnit ( pobRenderDef )
,	m_pobRenderDef ( pobRenderDef )
,	m_pobLifeClock ( 0 )
,	m_bDrainingLastFrame ( false )
{};

LifeClockRenderer::~LifeClockRenderer()
{
	if ( m_pobTimerRenderer )
		NT_DELETE_CHUNK ( Mem::MC_MISC, m_pobTimerRenderer );
}

bool LifeClockRenderer::Initialise( void )
{
	// Done per frame in old HUD.  Could move to Update() if this is still required per frame
	if ( StyleManager::Exists() )
	{
		ntAssert( StyleManager::Get().GetLifeClock() );
		m_pobLifeClock = StyleManager::Get().GetLifeClock();
	}

	ntAssert( m_pobRenderDef );
	ntAssert( m_pobRenderDef->m_pobTimerRenderDef );

	m_pobTimerRenderer = (TimerRenderer*)m_pobRenderDef->m_pobTimerRenderDef->CreateInstance( );
	if (! m_pobTimerRenderer )
	{
		return false;
	}

	m_pobTimerRenderer->Initialise();
	if ( m_pobLifeClock )
		m_pobTimerRenderer->SetTime( m_pobLifeClock->GetTotalInSeconds(), true );

	return true;
}

bool LifeClockRenderer::Update( float fTimeStep )
{
	if (! m_pobLifeClock )
		return true;

	if (! m_pobTimerRenderer )
		return true;

	// Update base unit
	CBlendableHudUnit::Update( fTimeStep );

	// Set the time from the lifeclock
	m_pobTimerRenderer->SetTime( m_pobLifeClock->GetTotalInSeconds() );

	// Pass in a time step for blends incase we have done a large jump
	m_pobTimerRenderer->TimerUpdate( fTimeStep, m_pobLifeClock->GetScalar() );

	return true;
}

bool LifeClockRenderer::Render( void )
{
	ntAssert ( m_pobRenderDef );
	
	if (! m_pobLifeClock )
		return false;

	// If deactive - 'cos we should render Enter and Exit states too
	if ( IsDeactive() )
		return false;

	CPoint obPos;
	
	float fScreenHeight = CGuiManager::Get().BBHeight();
	float fScreenWidth = CGuiManager::Get().BBWidth();

	float fResolutionScale = fScreenWidth/1280.0f;

	// BEGIN RENDERING
	//	Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );

	// Time to live
	{
		obPos.X() = (m_pobRenderDef->m_obTimeToLivePos.X()+m_pobRenderDef->m_fTopLeftX)*fScreenWidth;
		obPos.Y() = (m_pobRenderDef->m_obTimeToLivePos.Y()+m_pobRenderDef->m_fTopLeftY)*fScreenHeight;
		obPos.Z() = m_pobRenderDef->m_obTimeToLivePos.Z();

		m_pobRenderDef->m_aobTimeToLive.SetPosition(obPos);
		m_pobRenderDef->m_aobTimeToLive.SetWidth  ( m_pobRenderDef->m_fTimeToLiveWidth  * m_pobRenderDef->m_fOverallScale * fResolutionScale);
		m_pobRenderDef->m_aobTimeToLive.SetHeight ( m_pobRenderDef->m_fTimeToLiveHeight * m_pobRenderDef->m_fOverallScale * fResolutionScale);

		CVector obColour(m_pobRenderDef->m_obTimeToLiveColour);	
		obColour.W() *= m_fOverallAlpha;
		m_pobRenderDef->m_aobTimeToLive.SetColour(obColour.GetNTColor());

		RenderStateBlock::SetBlendMode( m_pobRenderDef->m_eTimeToLiveBlendmode );
		m_pobRenderDef->m_aobTimeToLive.Render();
	}

	// Render composite Timer
	if (m_pobTimerRenderer)
	{
		CPoint obTimerPos ( m_pobRenderDef->m_obTimerPos );
		obTimerPos.X() += m_pobRenderDef->m_fTopLeftX;
		obTimerPos.Y() += m_pobRenderDef->m_fTopLeftY;
		m_pobTimerRenderer->SetPosition( obTimerPos );
		m_pobTimerRenderer->Render();

#ifdef _DEBUG
		if ( m_pobTimerRenderer->IsCatchingUp() )
		{
			g_VisualDebug->Printf2D( ( m_pobRenderDef->m_fTopLeftX + 0.1f ) * fScreenWidth, ( m_pobRenderDef->m_fTopLeftY + 0.05f) * fScreenHeight, DC_RED, 0, "Catch up\n");
		}	
		if ( m_pobTimerRenderer->IsDraining() )
		{
			g_VisualDebug->Printf2D( ( m_pobRenderDef->m_fTopLeftX + 0.1f ) * fScreenWidth, ( m_pobRenderDef->m_fTopLeftY + 0.05f) * fScreenHeight, DC_RED, 0, "Drain\n");
		}
#endif // _DEBUG

		if ( m_bDrainingLastFrame != m_pobTimerRenderer->IsDraining() )
		{
			m_bDrainingLastFrame = m_pobTimerRenderer->IsDraining();
			if ( m_bDrainingLastFrame )
			{
				CLuaGlobal::CallLuaFunc("OnLifeclockDrainStart");
			}
			else
			{
				CLuaGlobal::CallLuaFunc("OnLifeclockDrainEnd");
			}
		}
	}
	
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		LifeClockRenderer::UpdateActive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void LifeClockRenderer::UpdateActive( float fTimestep )
{
	CBlendableHudUnit::UpdateActive ( fTimestep );

	CEntity* pobEntity = CEntityManager::Get().GetPlayer();
	ntAssert ( pobEntity && pobEntity->IsPlayer() );
	Player* pobCharacter = pobEntity->ToPlayer();

	if ( pobCharacter->IsArcher() || ( pobCharacter->IsHero() && !pobCharacter->ToHero()->HasHeavenlySword() ) )
	{
		BeginExit();
	}

	if ( NSManager::Get().IsNinjaSequencePlaying() || MoviePlayer::Get().IsActive()
		/*|| !pobCharacter->GetAwarenessComponent()->IsAwareOfEnemies()*/ )
	{
		BeginExit();

		if (m_pobTimerRenderer)
		{
			m_pobTimerRenderer->BeginExit();
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		LifeClockRenderer::UpdateInactive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void LifeClockRenderer::UpdateInactive( float fTimestep )
{
	CBlendableHudUnit::UpdateInactive ( fTimestep );

	CEntity* pobEntity = CEntityManager::Get().GetPlayer();
	ntAssert ( pobEntity && pobEntity->IsPlayer() );
	Player* pobCharacter = pobEntity->ToPlayer();

	if ( pobCharacter->IsArcher() || ( pobCharacter->IsHero() && !pobCharacter->ToHero()->HasHeavenlySword() ) )
	{
		return;
	}
	
	if ( ! NSManager::Get().IsNinjaSequencePlaying() && !MoviePlayer::Get().IsActive() )
	{
		if ( pobCharacter->IsHero() /*&& pobCharacter->GetAwarenessComponent()->IsAwareOfEnemies()*/ )
		{
			BeginEnter();	

			if (m_pobTimerRenderer)
			{
				m_pobTimerRenderer->BeginEnter();
			}
		}
	}
}
