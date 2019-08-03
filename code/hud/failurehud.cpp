//------------------------------------------------------------------------------------------
//!
//!	\file failurehud.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "hud/failurehud.h"
#include "objectdatabase/dataobject.h"
#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gui/guimanager.h"
#include "gui/guinotify.h"
#include "game/nsmanager.h"
#include "movies/movieinstance.h"
#include "movies/movieplayer.h"
#include "core/timer.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "game/shellconfig.h"
#include "game/combatstyle.h"

// ---- Forward references ----


// ---- Interfaces ----
START_STD_INTERFACE( FailureHudRenderDef )
	DEFINE_INTERFACE_INHERITANCE(CBlendableHudUnitDef)
	COPY_INTERFACE_FROM(CBlendableHudUnitDef)

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fSlowTimeFactor,	0.25f,	SlowTimeFactor )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fSlowBlendTime,		5.0f,	SlowBlendTime  )

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fMoveOnTime,		5.0f,	MoveOnTime  )

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_obFadeTime,			2.0f,	FadeTime )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_iFadeColour,		0x00000000,	FadeColour  )

END_STD_INTERFACE


void ForceLinkFunctionFailureHud()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionFailureHud() !ATTN!\n");
}

///////////////////////////////////////////
//
// FailureHud
// C'tor/D'tor
//
///////////////////////////////////////////
FailureHud::FailureHud()
:	m_pobFailureHudRenerer ( 0 )
,	m_obFailureStringID ("FAILURE_DEFAULT")
{
}

FailureHud::~FailureHud()
{
	m_pobFailureHudRenerer = 0;
}

///////////////////////////////////////////
//
// FailureHud::SetRenderer
// Register the elements renderer
//
///////////////////////////////////////////
void FailureHud::SetRenderer( FailureHudRenderer* pobFailureHudRenerer )
{
	ntAssert ( pobFailureHudRenerer );
	m_pobFailureHudRenerer = pobFailureHudRenerer;
}

///////////////////////////////////////////
//
// FailureHud::SetFailureStringID
// Change the string for the failure message
//
///////////////////////////////////////////
void FailureHud::SetFailureStringID( ntstd::String obFailureStringID ) 
{ 
	m_obFailureStringID = obFailureStringID; 
}

///////////////////////////////////////////
//
// FailureHud::NotifyFailure
// Notify that a failure has occured
//
///////////////////////////////////////////
void FailureHud::NotifyFailure( void )
{
	if ( m_pobFailureHudRenerer )
	{
		m_pobFailureHudRenerer->BeginEnter( true );
	}
}

void FailureHud::NotifyFailure( ntstd::String obFailureStringID )
{
	SetFailureStringID ( obFailureStringID );
	NotifyFailure();
}

///////////////////////////////////////////
//
// FailureHudRenderDef
//
///////////////////////////////////////////
FailureHudRenderDef::FailureHudRenderDef()
{
}

CHudUnit* FailureHudRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) FailureHudRenderer( (FailureHudRenderDef*)this );
}

///////////////////////////////////////////
//
// FailureHudRenderer
//
///////////////////////////////////////////
bool FailureHudRenderer::Initialise( void )
{
	ntAssert ( m_pobRenderDef );
	ntAssert ( CHud::Exists() );

	if ( CHud::Get().GetFailureHud() )
	{
		m_pobFailureHud = CHud::Get().GetFailureHud();

		// Register ourselves for later use;
		m_pobFailureHud->SetRenderer( this );
	}

	m_fCurrSlowBlendTime = 0.0f;
	m_fCurrMoveOnTime = 0.0f;

	CTimer::Get().ClearFailureTimeScalar();

	return true;
}

bool FailureHudRenderer::Update( float fTimeStep )
{
	ntAssert ( m_pobRenderDef );

	// Update base unit
	CBlendableHudUnit::Update( fTimeStep );

#if 0 //def _DEBUG
	float fTimeScalar = CTimer::Get().GetGameTimeScalar();
	g_VisualDebug->Printf2D( 5.0f, 40.0f, DC_GREEN, 0, "Time Scalar %f\n", fTimeScalar );
#endif // _DEBUG


	if ( m_fCurrSlowBlendTime > 0.0f )
	{
		m_fCurrSlowBlendTime -= CTimer::Get().GetSystemTimeChange();
		
		float fTimeScalarThisFrame = 1.0f - (1.0f - m_pobRenderDef->m_fSlowTimeFactor) *
			CMaths::SmoothStep(1.0f - m_fCurrSlowBlendTime / m_pobRenderDef->m_fSlowBlendTime );
		CTimer::Get().SetFailureTimeScalar( fTimeScalarThisFrame );

		if ( m_fCurrSlowBlendTime <= 0.0f )
		{
			m_fCurrSlowBlendTime = 0.0f;
		}
	}

	if ( m_fCurrMoveOnTime > 0.0f )
	{
		m_fCurrMoveOnTime -= CTimer::Get().GetSystemTimeChange();
		
		
		if ( m_fCurrMoveOnTime <= 0.0f )
		{
			if ( g_ShellOptions->m_eFrontendMode == FRONTEND_FINAL )
			{
				if ( ( !StyleManager::Exists() ) || ( StyleManager::Exists()&& !StyleManager::Get().IsPrologMode() ) )
				{
					// FIX ME, could be fail not always death!!
					CGuiNotify::PlayerDeath();
				}
			}
			else if ( g_ShellOptions->m_eFrontendMode == FRONTEND_TGS )
			{
				CGuiManager::Get().OnComplete();
			}
			else	
			{
				if ( m_pobFailureHud )
					CHud::Get().CreateMessageBox( m_pobFailureHud->m_obFailureStringID, 0.5f, 0.5f);
			}

			m_fCurrMoveOnTime = 0.0f;
		}
	}

	return true;
}

bool FailureHudRenderer::Render( void )
{
	ntAssert ( m_pobRenderDef );

	// If deactive - 'cos we should render Enter and Exit states too
	if ( IsDeactive() )
		return true;
	
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		FailureHudRenderer::UpdateActive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void FailureHudRenderer::UpdateActive( float fTimestep )
{
	CBlendableHudUnit::UpdateActive ( fTimestep );
}

/***************************************************************************************************
*
*	FUNCTION		FailureHudRenderer::UpdateInactive
*
*	DESCRIPTION		
*
***************************************************************************************************/
void FailureHudRenderer::UpdateInactive( float fTimestep )
{
	CBlendableHudUnit::UpdateInactive ( fTimestep );
}


/***************************************************************************************************
*
*	FUNCTION		FailureHudRenderer::BeginEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool FailureHudRenderer::BeginEnter( bool bForce )
{	
	ntAssert ( m_pobRenderDef );

	m_fCurrSlowBlendTime = m_pobRenderDef->m_fSlowBlendTime;
	m_fCurrMoveOnTime = m_pobRenderDef->m_fMoveOnTime;

	/*if(CamMan::Get().GetView(0))
		CamMan::Get().GetView(0)->FadeTo ( m_pobRenderDef->m_iFadeColour, m_pobRenderDef->m_obFadeTime );

	if(CamMan::Get().GetView(1))
		CamMan::Get().GetView(1)->FadeTo ( m_pobRenderDef->m_iFadeColour, m_pobRenderDef->m_obFadeTime );*/

	return CBlendableHudUnit::BeginEnter( bForce );
}

/***************************************************************************************************
*
*	FUNCTION		FailureHudRenderer::BeginEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool FailureHudRenderer::BeginExit( bool bForce )
{	
	return CBlendableHudUnit::BeginExit( bForce );
}
