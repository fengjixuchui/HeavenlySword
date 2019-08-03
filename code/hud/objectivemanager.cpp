//------------------------------------------------------------------------------------------
//!
//!	\file hud.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "hud/objectivemanager.h"
#include "objectdatabase/dataobject.h"
#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gui/guimanager.h"
#include "hud/hudmanager.h"
#include "hud/hudsound.h"
#include "game/messagehandler.h"
#include "gfx/texturemanager.h"

#include "game/nsmanager.h"

// ---- Forward references ----

// ---- Interfaces ----
START_STD_INTERFACE( ObjectiveManagerDef )
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_fMessageBoxX, 0.5f, MessageBoxX )
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_fMessageBoxY, 0.75f, MessageBoxY )
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_fMessageBoxMinWidth, 0.0f, MessageBoxMinWidth )
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_fMessageBoxMinHeight, 0.0f, MessageBoxMinHeight )
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_fMessageBoxMaxWidth, 0.6f, MessageBoxMaxWidth )
	PUBLISH_VAR_WITH_DEFAULT_AS			( m_fMessageBoxMaxHeight, 0.6f, MessageBoxMaxHeight )
END_STD_INTERFACE

START_STD_INTERFACE( ObjectiveManagerRenderDef )
	PUBLISH_VAR_AS			( m_obFont, Font )
	PUBLISH_VAR_AS			( m_fTimeToRenderNew, TimeToRenderNew )
	PUBLISH_VAR_AS			( m_fTimeToRenderWhenDone, TimeToRenderWhenDone )
	PUBLISH_VAR_AS			( m_fTopLeftX, TopLeftX )
	PUBLISH_VAR_AS			( m_fTopLeftY, TopLeftY )
	PUBLISH_VAR_AS			( m_fTimerTopLeftX, TimerTopLeftX )
	PUBLISH_VAR_AS			( m_fTimerTopLeftY, TimerTopLeftY )
	PUBLISH_VAR_AS			( m_fStatusTopLeftX, StatusTopLeftX )
	PUBLISH_VAR_AS			( m_fStatusTopLeftY, StatusTopLeftY )
	PUBLISH_VAR_AS			( m_fTimerLineSpace, TimerLineSpace )
	PUBLISH_VAR_AS			(m_fOverallScale, OverallScale)
	PUBLISH_PTR_AS			(m_pobTimerRenderDef, TimerRenderDefinition)

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_obBackgroundImage, "hud/timer_base_colour_alpha_npow2_nomip.dds", BackgroundImage )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_obBackgroundGlow, "hud/timer_glow_colour_alpha.dds", BackgroundGlow )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fTimerBackgroundTopLeftX, 0.5f, TimerBackgroundTopLeftX )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fTimerBackgroundTopLeftY, 0.5f, TimerBackgroundTopLeftY )

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fTimerBackgroundWidth, 0.2166f, TimerBackgroundWidth )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fTimerBackgroundHeight, 0.1344f, TimerBackgroundHeight )

	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fTimerBackgroundGlowWidth, 0.1344f, TimerBackgroundGlowWidth)
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_fTimerBackgroundGlowHeight, 0.2166f, TimerBackgroundGlowHeight)

	PUBLISH_VAR_WITH_DEFAULT_AS (m_obLabelTextID, "HUD_INFO_TIMER", LabelTextID)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fTextPosX, 0.0f, TextPosX)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fTextPosY, 0.0f, TextPosY)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obLabelFont, "Body", LabelFont)

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fTimeForGlow, 30.0f, TimeForGlow )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fAlphaForGlowHigh, 1.2f, AlphaForGlowHigh )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fAlphaForGlowLow, 0.4f, AlphaForGlowLow )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fTimeForSecondGlow, 10.0f, TimeForSecondGlow )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fAlphaForSecondGlowHigh, 0.8f, AlphaForSecondGlowHigh )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fAlphaForSecondGlowLow, 0.4f, AlphaForSecondGlowLow )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fGlowDelta, -4.0f, GlowDelta )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fScalarForGlowHigh, 1.6f, ScalarForGlowHigh )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fScalarForGlowLow, 1.0f, ScalarForGlowLow )

	PUBLISH_GLOBAL_ENUM_AS			(m_eBlendMode, BlendMode, EFFECT_BLENDMODE)

	DECLARE_POSTCONSTRUCT_CALLBACK ( PostConstruct )
END_STD_INTERFACE

void ForceLinkFunctionObjectiveManager()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionObjectiveManager() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
//!
//!	Objective::Objective
//!
//------------------------------------------------------------------------------------------
Objective::Objective(ntstd::String obText, int iPriority, int iID)
:	m_obObjectiveText( obText )
	,	m_bDone (false)
	,	m_bNew (true)
	,	m_iPriority( iPriority )
	,	m_iID( iID )
	,	m_pobCallbackEnt ( 0 )
	,	m_pobTextRenderer ( 0 )
	,	m_eType ( OT_SIMPLE )
	,	m_eTimerState ( TOS_INITIAL )
	,	m_pobTimerRenderer ( 0 )
	,	m_pobImageRenderer ( 0 )
{
}

//------------------------------------------------------------------------------------------
//!
//!	Objective::Initialise 
//!
//------------------------------------------------------------------------------------------
bool Objective::Initialise (ntstd::String obFont)
{
	if ( !ntStr::IsNull( m_obObjectiveText ) )
	{
		m_obTextRenderDef.m_obStringTextID = m_obObjectiveText;
		m_obTextRenderDef.m_obFontName = obFont;
		m_obTextRenderDef.m_fTopLeftX = 0.5f;
		m_obTextRenderDef.m_fTopLeftY = 0.25f;

		m_obTextRenderDef.m_fBlendInTime  = 0.25f;
		m_obTextRenderDef.m_fBlendOutTime = 0.25f;

		m_pobTextRenderer = (HudTextRenderer*)m_obTextRenderDef.CreateInstance();
		m_pobTextRenderer->Initialise();
		m_pobTextRenderer->BeginEnter();
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	Objective::InitialiseTimer (TimerRenderDef* pobTimerRenderDef) 
//!
//------------------------------------------------------------------------------------------
bool Objective::InitialiseTimer (TimerRenderDef* pobTimerRenderDef)
{
	if ( pobTimerRenderDef )
	{
		m_pobTimerRenderer = (TimerRenderer*)pobTimerRenderDef->CreateInstance();

		m_pobTimerRenderer->SetTime( m_fTimer, true );
		m_pobTimerRenderer->Initialise();
	}
	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	Objective::InitialiseImage ( CHashedString obImageDefName)
//!
//------------------------------------------------------------------------------------------
bool Objective::InitialiseImage ( CHashedString obImageDefName)
{
	HudImageRenderDef* pobImageDef = ObjectDatabase::Get().GetPointerFromName<HudImageRenderDef*>( obImageDefName );
	if (pobImageDef)
	{
		m_pobImageRenderer = (HudImageRenderer*)pobImageDef->CreateInstance();
		m_pobImageRenderer->Initialise();
	}

	return true;
}


void Objective::Passed( void )
{
	// Play a sound
	if ( CHud::Exists() && CHud::Get().GetHudSoundManager() )
	{
		CHud::Get().GetHudSoundManager()->PlaySound( CHashedString("CompleteObjective") );
	}

	m_bDone = true;
}

void Objective::Failed( void )
{
	// Play a sound
	if ( CHud::Exists() && CHud::Get().GetHudSoundManager() )
	{
		CHud::Get().GetHudSoundManager()->PlaySound( CHashedString("FailedObjective") );
	}

	m_bDone = true;
}

//------------------------------------------------------------------------------------------
//!
//!	Objective::~Objective
//!
//------------------------------------------------------------------------------------------
Objective::~Objective( void )
{
	if (m_pobTextRenderer)
		NT_DELETE_CHUNK ( Mem::MC_MISC, m_pobTextRenderer);
	if (m_pobTimerRenderer)
		NT_DELETE_CHUNK ( Mem::MC_MISC, m_pobTimerRenderer);
	if (m_pobImageRenderer)
		NT_DELETE_CHUNK ( Mem::MC_MISC, m_pobImageRenderer);
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerDef::ObjectiveManagerDef
//!
//------------------------------------------------------------------------------------------
ObjectiveManagerDef::ObjectiveManagerDef()
{
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerDef::~ObjectiveManagerDef
//!
//------------------------------------------------------------------------------------------
ObjectiveManagerDef::~ObjectiveManagerDef()
{
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManager::ObjectiveManager
//!
//------------------------------------------------------------------------------------------
ObjectiveManager::ObjectiveManager()
:	m_iObjectivesSoFar( 0 )
,	m_pobDef ( 0 )
,	m_bNSActive ( false )
{
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManager::~ObjectiveManager
//!
//------------------------------------------------------------------------------------------
ObjectiveManager::~ObjectiveManager()
{
	ntstd::List< Objective* >::iterator obEnd = m_obObjectiveList.end();
	for( ntstd::List< Objective* >::iterator obIt = m_obObjectiveList.begin(); obIt != obEnd; ++obIt )
	{
		NT_DELETE( *(obIt) );
	}
	m_obObjectiveList.clear();
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManager::RegisterDef
//!
//------------------------------------------------------------------------------------------
void ObjectiveManager::RegisterDef(ObjectiveManagerDef* pobDef )
{
	m_pobDef = pobDef;
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManager::AddObjective
//!
//------------------------------------------------------------------------------------------
int ObjectiveManager::AddObjective(const char* pcText, int iPriority)
{
	Objective* pobNewOb = NT_NEW Objective(ntstd::String(pcText),iPriority,m_iObjectivesSoFar);
	m_iObjectivesSoFar++;
	m_obObjectiveList.push_back(pobNewOb);

	
	// Armydemo hack - doing this here as it will invalidate the renderable itterator if done from
	// the update/render functions.  Proper soultion is for the objective manager to own/update
	// its own child elements.
	// ... And not during NS
	if ( CHud::Exists() && !m_bNSActive )
	{
		if ( m_pobDef )
		{
			CHud::Get().CreateMessageBox( ntstd::String(pcText), m_pobDef->m_fMessageBoxX, m_pobDef->m_fMessageBoxY, 
												m_pobDef->m_fMessageBoxMinWidth, m_pobDef->m_fMessageBoxMinHeight,
												m_pobDef->m_fMessageBoxMaxWidth, m_pobDef->m_fMessageBoxMaxHeight );
		}
		else
		{
			CHud::Get().CreateMessageBox( ntstd::String(pcText), 0.5f, 0.65f, 0.0f, 0.0f, 0.75f, 0.4f );
		}
	}
	return pobNewOb->m_iID;
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManager::AddTimedObjective
//!
//------------------------------------------------------------------------------------------
int ObjectiveManager::AddTimedObjective(const char* pcText, float fTime, CEntity* pobCallbackEnt, int iPriority)
{
	Objective* pobNewOb = NT_NEW Objective(ntstd::String(pcText),iPriority,m_iObjectivesSoFar);
	pobNewOb->m_fTimer = fTime;
	pobNewOb->m_eType = Objective::OT_TIMER;
	pobNewOb->m_pobCallbackEnt = pobCallbackEnt;
	m_iObjectivesSoFar++;
	m_obObjectiveList.push_back(pobNewOb);

	
	// Armydemo hack - doing this here as it will invalidate the renderable itterator if done from
	// the update/render functions.  Proper soultion is for the objective manager to own/update
	// its own child elements.
	// ... And not during NS
	if ( CHud::Exists() && !m_bNSActive )
	{
		if ( m_pobDef )
		{
			CHud::Get().CreateMessageBox( ntstd::String(pcText), m_pobDef->m_fMessageBoxX, m_pobDef->m_fMessageBoxY, 
												m_pobDef->m_fMessageBoxMinWidth, m_pobDef->m_fMessageBoxMinHeight,
												m_pobDef->m_fMessageBoxMaxWidth, m_pobDef->m_fMessageBoxMaxHeight );
		}
		else
		{
			CHud::Get().CreateMessageBox( ntstd::String(pcText), 0.5f, 0.65f, 0.0f, 0.0f, 0.75f, 0.4f );
		}
	}
	return pobNewOb->m_iID;
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManager::AddStatusObjective
//!
//------------------------------------------------------------------------------------------
int ObjectiveManager::AddStatusObjective(const char* pcText, int iPriority)
{
	Objective* pobNewOb = NT_NEW Objective(ntstd::String(pcText),iPriority,m_iObjectivesSoFar);
	pobNewOb->m_eType = Objective::OT_STATUS;
	m_iObjectivesSoFar++;
	m_obObjectiveList.push_back(pobNewOb);

	// Armydemo hack - doing this here as it will invalidate the renderable itterator if done from
	// the update/render functions.  Proper soultion is for the objective manager to own/update
	// its own child elements.
	// ... And not during NS
	if ( CHud::Exists() && !m_bNSActive )
	{
		if ( m_pobDef )
		{
			CHud::Get().CreateMessageBox( ntstd::String(pcText), m_pobDef->m_fMessageBoxX, m_pobDef->m_fMessageBoxY, 
												m_pobDef->m_fMessageBoxMinWidth, m_pobDef->m_fMessageBoxMinHeight,
												m_pobDef->m_fMessageBoxMaxWidth, m_pobDef->m_fMessageBoxMaxHeight );
		}
		else
		{
			CHud::Get().CreateMessageBox( ntstd::String(pcText), 0.5f, 0.65f, 0.0f, 0.0f, 0.75f, 0.4f );
		}
	}
	return pobNewOb->m_iID;
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManager::AddImageObjective
//!
//------------------------------------------------------------------------------------------
int ObjectiveManager::AddImageObjective(const char* pcText, int iPriority)
{
	Objective* pobNewOb = NT_NEW Objective(ntstd::String(pcText),iPriority,m_iObjectivesSoFar);
	pobNewOb->m_eType = Objective::OT_IMAGE;
	m_iObjectivesSoFar++;

	pobNewOb->InitialiseImage( CHashedString( pcText ) );

	m_obObjectiveList.push_back(pobNewOb);
	return pobNewOb->m_iID;
}

void ObjectiveManager::RemoveObjective( int iObjectiveID)
{
	Objective* pobRemove = GetObjective(iObjectiveID);

	if (pobRemove)
	{
		m_obObjectiveList.remove( pobRemove );
		NT_DELETE( pobRemove );
	}
}

void ObjectiveManager::PassedObjective( int iObjectiveID)
{
	Objective* pobPass = GetObjective(iObjectiveID);

	if (pobPass)
	{
		pobPass->Passed();
	}
}

void ObjectiveManager::FailedObjective( int iObjectiveID)
{
	Objective* pobFail = GetObjective(iObjectiveID);

	if (pobFail)
	{
		pobFail->Failed();
	}
}

Objective* ObjectiveManager::GetObjective(int iID)
{
	ntstd::List< Objective* >::iterator obEnd = m_obObjectiveList.end();
	for( ntstd::List< Objective* >::iterator obIt = m_obObjectiveList.begin(); obIt != obEnd; ++obIt )
	{
		// If we have the required objective
		if ( (*obIt)->m_iID == iID)
		{
			return (*obIt);
		}	
	}
	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerRenderDef::ObjectiveManagerRenderDef
//!
//------------------------------------------------------------------------------------------
ObjectiveManagerRenderDef::ObjectiveManagerRenderDef()
:	m_fTimeToRenderNew( 3.0f )
,	m_fTimeToRenderWhenDone( 5.0f )
,	m_fTopLeftX( 0.5f )
,	m_fTopLeftY( 0.5f )
,	m_fTimerTopLeftX( 0.8f )
,	m_fTimerTopLeftY( 0.1f )
{

}

CHudUnit* ObjectiveManagerRenderDef::CreateInstance( void ) const
{
	return NT_NEW_CHUNK (Mem::MC_MISC) ObjectiveManagerRenderer( (ObjectiveManagerRenderDef*)this );
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerRenderDef::PostConstruct
//! PostConstruct
//!
//------------------------------------------------------------------------------------------
void ObjectiveManagerRenderDef::PostConstruct( void )
{
	if ( !ntStr::IsNull ( m_obBackgroundImage ) )
		TextureManager::Get().LoadTexture_Neutral( m_obBackgroundImage.GetString() );

	if ( !ntStr::IsNull ( m_obBackgroundGlow ) )
		TextureManager::Get().LoadTexture_Neutral( m_obBackgroundGlow.GetString() );
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerRenderDef::~ObjectiveManagerRenderDef
//!
//------------------------------------------------------------------------------------------
ObjectiveManagerRenderDef::~ObjectiveManagerRenderDef()
{
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerRenderer::Initialise
//!
//------------------------------------------------------------------------------------------
ObjectiveManagerRenderer::~ObjectiveManagerRenderer()
{
	if (m_pobTextRenderer)
		NT_DELETE_CHUNK ( Mem::MC_MISC, m_pobTextRenderer);
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerRenderer::Initialise
//!
//------------------------------------------------------------------------------------------
bool ObjectiveManagerRenderer::Initialise( void )
{
	ntAssert( m_pobRenderDef );

	m_pobObjectiveManager = CHud::Get().GetObjectiveManager();

	if ( !ntStr::IsNull ( m_pobRenderDef->m_obBackgroundImage ) )
	{
		m_obTimerBackground.SetTexture( m_pobRenderDef->m_obBackgroundImage );
	}

	if ( !ntStr::IsNull ( m_pobRenderDef->m_obBackgroundGlow ) )
	{
		m_obTimerGlow.SetTexture( m_pobRenderDef->m_obBackgroundGlow );
	}

	m_fGlowAlpha = 0.0f;

	if ( !ntStr::IsNull( m_pobRenderDef->m_obLabelTextID ) )
	{
		m_obTextRenderDef.m_obStringTextID = m_pobRenderDef->m_obLabelTextID;
		m_obTextRenderDef.m_obFontName = m_pobRenderDef->m_obLabelFont;
		m_obTextRenderDef.m_fTopLeftX = m_pobRenderDef->m_fTimerBackgroundTopLeftX + m_pobRenderDef->m_fTextPosX;
		m_obTextRenderDef.m_fTopLeftY = m_pobRenderDef->m_fTimerBackgroundTopLeftY + m_pobRenderDef->m_fTextPosY;

		m_pobTextRenderer = (HudTextRenderer*)m_obTextRenderDef.CreateInstance();
		m_pobTextRenderer->Initialise();
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerRenderer::Update
//!
//------------------------------------------------------------------------------------------
bool ObjectiveManagerRenderer::Update( float fTimeChange )
{
	ntAssert ( m_pobObjectiveManager );

	m_fTimeChange = fTimeChange;

	if ( m_pobObjectiveManager->m_bNSActive )
	{
		if (! NSManager::Get().IsNinjaSequencePlaying() )
		{
			// NS just ended
			m_pobObjectiveManager->m_bNSActive = false;
		}
	}
	else
	{
		if ( NSManager::Get().IsNinjaSequencePlaying() )
		{
			// NS just started
			m_pobObjectiveManager->m_bNSActive = true;

			// Clear any active message box
			CHud::Get().RemoveMessageBox( );
		}
	}

	return true;
}


//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerRenderer::Render
//!
//------------------------------------------------------------------------------------------
bool ObjectiveManagerRenderer::Render( void )
{
	ntAssert ( m_pobObjectiveManager );
	ntAssert ( m_pobRenderDef );

	if ( NSManager::Get().IsNinjaSequencePlaying() )
		return false;

	// Scale our heights and positions accordingly
	float fBBHeight = CGuiManager::Get().BBHeight();
	//float fBBWidth = CGuiManager::Get().BBWidth();

	//int Lines = 0;
	int TimerLines = 0;
	//int StatusLines = 0;
	//const int LineHeight = 20;
	//float fLineOffset = 0.0f;

	//float fTextTopLeftX = m_pobRenderDef->m_fTopLeftX;
	//float fTextTopLeftY = m_pobRenderDef->m_fTopLeftY;

	ntstd::List< Objective* >::iterator obEnd = m_pobObjectiveManager->m_obObjectiveList.end();
	for( ntstd::List< Objective* >::iterator obIt = m_pobObjectiveManager->m_obObjectiveList.begin(); obIt != obEnd; ++obIt )
	{
		switch ( (*obIt)->m_eType )
		{
			// Simple objective displayed for m_pobRenderDef->m_fTimeToRenderNew seconds
			case Objective::OT_SIMPLE:
			{
				if ( (*obIt)->IsNew() )
				{
					CHud::Get().RemoveMessageBox( m_pobRenderDef->m_fTimeToRenderNew );

					/*(*obIt)->Initialise( m_pobRenderDef->m_obFont );*/
					(*obIt)->m_fRenderTime = m_pobRenderDef->m_fTimeToRenderNew;	
					(*obIt)->m_bNew = false;
				}
			
				if ((*obIt)->m_fRenderTime >= 0.0f )
				{
					(*obIt)->m_fRenderTime -= m_fTimeChange;
					/*if ( (*obIt)->m_pobTextRenderer )
					{
						(*obIt)->m_pobTextRenderer->SetPosition ( fTextTopLeftX, fTextTopLeftY + fLineOffset );
						fLineOffset += (*obIt)->m_pobTextRenderer->RenderHeight()/fBBHeight;
						(*obIt)->m_pobTextRenderer->Update(m_fTimeChange);
						(*obIt)->m_pobTextRenderer->Render();
					}
					Lines++;*/
				}

				// Remove me if done
				if ( (*obIt)->m_fRenderTime < 0.0f  )
				{
					m_pobObjectiveManager->RemoveObjective( (*obIt)->m_iID );
					return false;
				}
				break;
			}

			// Timed objective displays for m_pobRenderDef->m_fTimeToRenderNew in the centre of screen
			// then moves to edge of screen
			case Objective::OT_TIMER:
			{
				if ( (*obIt)->IsNew() )
				{
					CHud::Get().RemoveMessageBox( m_pobRenderDef->m_fTimeToRenderNew );

					/*(*obIt)->Initialise( m_pobRenderDef->m_obFont );*/
					(*obIt)->InitialiseTimer( m_pobRenderDef->m_pobTimerRenderDef );
					(*obIt)->m_fRenderTime = m_pobRenderDef->m_fTimeToRenderNew;
					(*obIt)->m_bNew = false;

					if ((*obIt)->m_pobTimerRenderer)
					{
						(*obIt)->m_pobTimerRenderer->BeginEnter();
					}

					// Play a sound
					if ( CHud::Exists() && CHud::Get().GetHudSoundManager() )
					{
						CHud::Get().GetHudSoundManager()->PlaySound( CHashedString("NewTimerObjective") );
					}

					// Text 
					if ( m_pobTextRenderer)
					{
						m_pobTextRenderer->BeginEnter();
					}
				}
			
				if ( (*obIt)->m_fTimer >= 0.0f && !(*obIt)->IsDone())
					(*obIt)->m_fTimer -= m_fTimeChange;

				if ((*obIt)->m_pobTimerRenderer)
				{
					// Use this code if we need timers that loose or gain lots of
					// time due to external events
					//(*obIt)->m_pobTimerRenderer->Update( m_fTimeChange );
					//(*obIt)->m_pobTimerRenderer->SetTime( (*obIt)->m_fTimer );

					// Or just blat on our current time
					(*obIt)->m_pobTimerRenderer->SetTime( (*obIt)->m_fTimer, true );
				}
				
				// Has the timer finsihed?
				if ( (*obIt)->m_fTimer < 0.0f && !(*obIt)->IsDone() )
				{
					(*obIt)->m_fTimer = 0.0f;
					(*obIt)->m_fRenderTime = m_pobRenderDef->m_fTimeToRenderWhenDone;	

					CLuaGlobal::CallLuaFunc("OnTimerLowEnd");

					if ( (*obIt)->m_pobCallbackEnt )
					{
						// Set a message for the arm to auto reset
						Message msgTimeOver(msg_time_complete);
						(*obIt)->m_pobCallbackEnt->GetMessageHandler()->QueueMessage( msgTimeOver );	
					}

					(*obIt)->Failed();
				}
			
				// Display the timer for a while after its done
				if ( (*obIt)->IsDone() )
				{
					(*obIt)->m_fRenderTime -= m_fTimeChange;	
					if ( (*obIt)->m_fRenderTime < 0.0f  )
					{
						m_pobObjectiveManager->RemoveObjective( (*obIt)->m_iID );
						return false;
					}
				}

				// Display in center if new
				if (!(*obIt)->IsDone() && (*obIt)->m_fRenderTime >= 0.0f )
				{
					(*obIt)->m_fRenderTime -= m_fTimeChange;
					
					/*if ( (*obIt)->m_pobTextRenderer )
					{
						(*obIt)->m_pobTextRenderer->SetPosition ( fTextTopLeftX, fTextTopLeftY + fLineOffset );
						fLineOffset += (*obIt)->m_pobTextRenderer->RenderHeight()/fBBHeight;
						(*obIt)->m_pobTextRenderer->Update(m_fTimeChange);
						(*obIt)->m_pobTextRenderer->Render();
					}
					Lines++;*/
				}
				// Otherwise display off to one side
				else
				{
					//g_VisualDebug->Printf2D( m_pobRenderDef->m_fTimerTopLeftX*fBBWidth, m_pobRenderDef->m_fTimerTopLeftY*fBBHeight+(TimerLines*LineHeight), DC_WHITE, 0, (*obIt)->GetText() );
					/*if ( (*obIt)->m_pobTextRenderer )
					{
						(*obIt)->m_pobTextRenderer->Update(m_fTimeChange);
						(*obIt)->m_pobTextRenderer->Render();
					}*/
					TimerLines++;
				}

				CPoint Pos(CONSTRUCT_CLEAR);
				Pos.X() = m_pobRenderDef->m_fTimerTopLeftX;
				Pos.Y() = m_pobRenderDef->m_fTimerTopLeftY;

				if ((*obIt)->m_pobTimerRenderer)
				{
					TimerBackgroundRender( (*obIt) , m_fTimeChange );
					(*obIt)->m_pobTimerRenderer->Update(m_fTimeChange);
					(*obIt)->m_pobTimerRenderer->SetPosition( Pos );
					(*obIt)->m_pobTimerRenderer->Render();
				}

				// Text 
				if ( m_pobTextRenderer)
				{
					// Position - for testing only
					m_pobTextRenderer->SetPosition( m_pobRenderDef->m_fTimerTopLeftX + m_pobRenderDef->m_fTextPosX, 
													m_pobRenderDef->m_fTimerTopLeftY + m_pobRenderDef->m_fTextPosY );
					
					m_pobTextRenderer->Update( m_fTimeChange );
					m_pobTextRenderer->Render();
				}

				break;
			}

			// Status message displays until removed
			case Objective::OT_STATUS:
			{
				if ( (*obIt)->IsNew() )
				{
					//(*obIt)->Initialise( m_pobRenderDef->m_obFont );
					//(*obIt)->m_fRenderTime = m_pobRenderDef->m_fTimeToRenderNew;	
					(*obIt)->m_bNew = false;
				}

				/*if ( (*obIt)->m_pobTextRenderer )
				{
					(*obIt)->m_pobTextRenderer->SetPosition ( fTextTopLeftX, fTextTopLeftY + fLineOffset );
					fLineOffset += (*obIt)->m_pobTextRenderer->RenderHeight()/fBBHeight;	
					(*obIt)->m_pobTextRenderer->Update(m_fTimeChange);
					(*obIt)->m_pobTextRenderer->Render();
				}
				StatusLines++;*/
				break;
			}

			// Image Objective
			case Objective::OT_IMAGE:
			{
				if ( (*obIt)->m_pobImageRenderer )
				{
					if ( (*obIt)->IsNew() )
					{
						(*obIt)->m_pobImageRenderer->BeginEnter();
						(*obIt)->m_bNew = false;
					}

					//(*obIt)->m_pobTextRenderer->SetPosition ( fTextTopLeftX, fTextTopLeftY + fLineOffset );
					//fLineOffset += (*obIt)->m_pobTextRenderer->RenderHeight()/fBBHeight;	
					(*obIt)->m_pobImageRenderer->Update(m_fTimeChange);
					(*obIt)->m_pobImageRenderer->Render();
				}
				break;
			}

			default:
				break;

		}	// switch (*obIt)->m_eType
	}

	UNUSED (fBBHeight);
	return true;
}

void ObjectiveManagerRenderer::TimerBackgroundRender ( Objective* pobObjective, float fTimeStep )
{
	// Scale our heights and positions accordingly
	float fBBHeight = CGuiManager::Get().BBHeight();
	float fBBWidth = CGuiManager::Get().BBWidth();

	UNUSED ( fTimeStep );

	// Update alpha deltas
	/*if ( pobObjective->m_fTimer <= (m_pobRenderDef->m_fTimeForSecondGlow+1.0f))
	{
		if ( pobObjective->m_eTimerState == Objective::TOS_FIRST_WARNING )
		{
			// Play a sound
			if ( CHud::Exists() && CHud::Get().GetHudSoundManager() )
			{
				CHud::Get().GetHudSoundManager()->PlaySound( CHashedString("TimerObjectiveVeryLow") );
			}
			pobObjective->m_eTimerState = Objective::TOS_SECOND_WARNING;
		}

	
	}
	else*/ if ( pobObjective->m_fTimer <= (m_pobRenderDef->m_fTimeForGlow+1.0f))
	{
		if ( pobObjective->m_eTimerState == Objective::TOS_INITIAL )
		{
			CLuaGlobal::CallLuaFunc("OnTimerLowStart");

			// Play a sound
			if ( CHud::Exists() && CHud::Get().GetHudSoundManager() )
			{
				CHud::Get().GetHudSoundManager()->PlaySound( CHashedString("TimerObjectiveLow") );
			}
			pobObjective->m_eTimerState = Objective::TOS_FIRST_WARNING;
		}

		float fGlowTime = pobObjective->m_fTimer - floor(pobObjective->m_fTimer);

		m_fGlowAlpha = m_pobRenderDef->m_fAlphaForGlowLow + CMaths::SmoothStep(fGlowTime) * ( m_pobRenderDef->m_fAlphaForGlowHigh - m_pobRenderDef->m_fAlphaForGlowLow);
		m_fGlowScalar = m_pobRenderDef->m_fScalarForGlowLow + CMaths::SmoothStep(fGlowTime) * ( m_pobRenderDef->m_fScalarForGlowHigh - m_pobRenderDef->m_fScalarForGlowLow);
	}
	else
	{
		m_fGlowAlpha = 0.0f;
	}

	if ( !ntStr::IsNull ( m_pobRenderDef->m_obBackgroundImage ) )
	{
		RenderStateBlock::SetBlendMode(m_pobRenderDef->m_eBlendMode);

		m_obTimerBackground.SetPosition( CPoint( m_pobRenderDef->m_fTimerBackgroundTopLeftX * fBBWidth, m_pobRenderDef->m_fTimerBackgroundTopLeftY * fBBHeight, 0.0f ) );
		m_obTimerBackground.SetWidth( m_pobRenderDef->m_fTimerBackgroundWidth * fBBWidth );
		m_obTimerBackground.SetHeight( m_pobRenderDef->m_fTimerBackgroundHeight * fBBHeight );
		m_obTimerBackground.SetColour( CVector(1.0f, 1.0f, 1.0f, 1.0f-m_fGlowAlpha) );
		m_obTimerBackground.Render();
	}
	
	if ( !ntStr::IsNull ( m_pobRenderDef->m_obBackgroundGlow ) )
	{
		RenderStateBlock::SetBlendMode(m_pobRenderDef->m_eBlendMode);

		m_obTimerGlow.SetPosition( CPoint( m_pobRenderDef->m_fTimerBackgroundTopLeftX * fBBWidth, m_pobRenderDef->m_fTimerBackgroundTopLeftY * fBBHeight, 0.0f ) );
		m_obTimerGlow.SetWidth( m_pobRenderDef->m_fTimerBackgroundGlowWidth * m_fGlowScalar * fBBWidth );
		m_obTimerGlow.SetHeight( m_pobRenderDef->m_fTimerBackgroundGlowHeight * m_fGlowScalar * fBBHeight );
		m_obTimerGlow.SetColour( CVector(1.0f, 1.0f, 1.0f, m_fGlowAlpha) );
		m_obTimerGlow.Render();
	}

}
