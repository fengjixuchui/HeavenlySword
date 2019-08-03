/***************************************************************************************************
*
*	DESCRIPTION		The main manager of the flow of the game
*
*	NOTES			Looks after front end screens and the management of levels
*
***************************************************************************************************/

// Includes
#include "hud/hudmanager.h"
#include "hud/blendablehudunit.h"
#include "hud/objectivemanager.h"
#include "hud/buttonhinter.h"
#include "hud/stylebar.h"
#include "hud/stylelabel.h"
#include "hud/specialstylebar.h"
#include "hud/healthbar.h"
#include "hud/lifeclock.h"
#include "hud/messagebox.h"
#include "hud/messagedata.h"
#include "hud/failurehud.h"
#include "hud/hudsound.h"
#include "hud/gamecounter.h"

#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/shellconfig.h"
#include "game/shellmain.h"
#include "objectdatabase/dataobject.h"

#include "game/nsmanager.h"
#include "gui/guimanager.h"
#include "input/inputhardware.h"
#include "physics/triggervolume.h"

// ---- Forward references ----

class Character;

// ---- Interfaces ----

START_STD_INTERFACE (HUDParams)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_obNewCheckpointFont, "", NewCheckpointFont)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fNewCheckpointPosX, 0.5f, NewCheckpointPosX)
	PUBLISH_VAR_WITH_DEFAULT_AS (m_fNewCheckpointPosY, 0.5f, NewCheckpointPosY)

	PUBLISH_PTR_AS	( m_pobObjectiveManDef, ObjectiveManagerParams);

END_STD_INTERFACE

START_STD_INTERFACE (CHudConfig)
	PUBLISH_PTR_CONTAINER_AS (m_aobHudRenderDefList, HudRenderDefList)
	PUBLISH_PTR_AS (m_pobHudParams, HudParams)
	PUBLISH_PTR_AS (m_pobMessageBoxDef, MessageBoxDef)
	PUBLISH_PTR_AS (m_pobMessageBoxRenderDef, MessageBoxRenderDef)
	
	DECLARE_POSTCONSTRUCT_CALLBACK		(	PostConstruct )
END_STD_INTERFACE


// --- Force Link PS3 stuff --- //

extern void ForceLinkFunctionButtonHinter();
extern void ForceLinkFunctionHealthBar();
extern void ForceLinkFunctionHUDText();
extern void ForceLinkFunctionHUDImage();
extern void ForceLinkFunctionHUDUnit();
extern void ForceLinkFunctionBlendableHUDUnit();
extern void ForceLinkFunctionLifeClock();
extern void ForceLinkFunctionStyleBar();
extern void ForceLinkFunctionStyleLabel();
extern void ForceLinkFunctionSpecialStyleBar();
extern void ForceLinkFunctionObjectiveManager();
extern void ForceLinkFunctionHUDTimer();
extern void ForceLinkFunctionCMessageBox();
extern void ForceLinkFunctionGameCounterRender();
extern void ForceLinkFunctionFailureHud();
extern void ForceLinkFunctionHudSound();
extern void ForceLinkFunctionBossStyleBar();

void ForceLinkFunctionHUD()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionHUD() !ATTN!\n");
	ForceLinkFunctionButtonHinter();
	ForceLinkFunctionHealthBar();
	ForceLinkFunctionHUDText();
	ForceLinkFunctionHUDImage();
	ForceLinkFunctionHUDUnit();
	ForceLinkFunctionBlendableHUDUnit();
	ForceLinkFunctionLifeClock();
	ForceLinkFunctionStyleBar();
	ForceLinkFunctionStyleLabel();
	ForceLinkFunctionSpecialStyleBar();
	ForceLinkFunctionObjectiveManager();
	ForceLinkFunctionHUDTimer();
	ForceLinkFunctionCMessageBox();
	ForceLinkFunctionGameCounterRender();
	ForceLinkFunctionFailureHud();
	ForceLinkFunctionHudSound();
	ForceLinkFunctionBossStyleBar();
}

//------------------------------------------------------------------------------------------
//!
//!	CHudConfig::PostConstruct
//!
//------------------------------------------------------------------------------------------
void CHudConfig::PostConstruct( void )
{
	if ( CHud::Exists() )
	{
		CHud::Get().SetConfig ( this );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::CHud
//! Construction
//!
//------------------------------------------------------------------------------------------
CHud::CHud () 
	:	m_obIncOverlayResourceManager(32)
{
	m_pobHudConfig=0;
	//m_fOverallAlpha=1.0f;

	m_pobCombatHUDElements = NT_NEW CombatHUDElements();
	
	m_pobCombatHUDElements->m_pobButtonHinter = 0;
	m_pobCombatHUDElements->m_pobObjectiveManager = 0;
	m_pobCombatHUDElements->m_pobFailureHud = 0;
	m_pobCombatHUDElements->m_pobBodyCount = 0;

	m_pobCombatEventLog = NT_NEW CombatEventLog();
	m_bCombatLogRegistered = false;

	m_pobMessageDataManager = NT_NEW_CHUNK (Mem::MC_MISC) MessageDataManager();

	m_pobHudSoundManager	= NT_NEW_CHUNK (Mem::MC_MISC) HudSoundManager();

	m_pobMessageBoxRenderer = 0;
	m_pobMessageRenderer = 0;

	m_fMessageBoxRemoveTime = 0.0f;
	m_fMessageRemoveTime = 0.0f;
	m_fDelayedMessageTime = 0.0f;

	m_bGrabHint = false;
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::CHud
//! Destruction
//!
//------------------------------------------------------------------------------------------
CHud::~CHud ()
{
	if ( m_pobCombatHUDElements )
		NT_DELETE( m_pobCombatHUDElements );

	if ( m_pobCombatEventLog )
		NT_DELETE( m_pobCombatEventLog );

	if ( m_pobMessageDataManager )
		NT_DELETE_CHUNK (Mem::MC_MISC, m_pobMessageDataManager);

	if ( m_pobHudSoundManager )
		NT_DELETE_CHUNK (Mem::MC_MISC, m_pobHudSoundManager);
}

void CHud::Reset()
{
	if (m_pobCombatHUDElements->m_pobButtonHinter)
	{
		NT_DELETE( m_pobCombatHUDElements->m_pobButtonHinter );
		m_pobCombatHUDElements->m_pobButtonHinter = 0;
	}

	if (m_pobCombatHUDElements->m_pobObjectiveManager)
	{
		NT_DELETE( m_pobCombatHUDElements->m_pobObjectiveManager );
		m_pobCombatHUDElements->m_pobObjectiveManager = 0;
	}

	if (m_pobCombatHUDElements->m_pobFailureHud)
	{
		NT_DELETE( m_pobCombatHUDElements->m_pobFailureHud );
		m_pobCombatHUDElements->m_pobFailureHud = 0;
	}

	if (m_pobCombatHUDElements->m_pobBodyCount)
	{
		NT_DELETE( m_pobCombatHUDElements->m_pobBodyCount );
		m_pobCombatHUDElements->m_pobBodyCount = 0;
	}

	if ( m_pobMessageDataManager )
		m_pobMessageDataManager->Reset();

	if ( m_pobHudSoundManager )
		m_pobHudSoundManager->Reset();

	for (HudRenderIter obIt = m_aobHudRenderList.begin(); 
			obIt != m_aobHudRenderList.end(); ++obIt )
	{
		NT_DELETE_CHUNK (Mem::MC_MISC, *obIt);
	}
	m_aobHudRenderList.clear();

	m_obElementMap.clear();

	m_pobHudConfig = 0;

	m_pobMessageBoxRenderer = 0;
};


//------------------------------------------------------------------------------------------
//!
//!	CHudConfig::SetConfig
//!
//------------------------------------------------------------------------------------------
void CHud::SetConfig (CHudConfig* pobHudConfig)
{
	ntAssert ( pobHudConfig );
	m_pobHudConfig = pobHudConfig;
}


void CHud::Initialise ( void )
{
	if (g_ShellOptions->m_bUseHUD )
	{
		// Set up the debug camera sprite
		CVector obCol;
		obCol.SetFromNTColor(0xb0ffffff);
		m_obDebugCameraSprite.SetColour(obCol.GetNTColor());
		m_obDebugCameraSprite.SetTexture("hud\\camera_mono_nomip.dds");

		if ( m_pobHudConfig )
		{
			ntError(m_pobHudConfig);
			ntError( m_pobCombatHUDElements != NULL );
			
			m_pobCombatHUDElements->m_pobButtonHinter = NT_NEW ButtonHinter();
			m_pobCombatHUDElements->m_pobObjectiveManager = NT_NEW ObjectiveManager( );

			if ( m_pobHudConfig->m_pobHudParams && m_pobHudConfig->m_pobHudParams->m_pobObjectiveManDef )
			{
				m_pobCombatHUDElements->m_pobObjectiveManager->RegisterDef( m_pobHudConfig->m_pobHudParams->m_pobObjectiveManDef );
			}

			m_pobCombatHUDElements->m_pobFailureHud = NT_NEW FailureHud( );
			m_pobCombatHUDElements->m_pobBodyCount = NT_NEW BodyCount( );

			for (HudDefIter obIt = m_pobHudConfig->m_aobHudRenderDefList.begin(); 
						obIt != m_pobHudConfig->m_aobHudRenderDefList.end(); ++obIt )
			{
				if ( (*obIt) )
				{
					CreateHudElement((*obIt));
				}
			}

			// Plug in our log if we can
			if (!m_bCombatLogRegistered && CEntityManager::Get().GetPlayer() && CEntityManager::Get().GetPlayer()->GetAttackComponent())
			{
				// Plug into players combat event log manager so we get events we care about
				CEntityManager::Get().GetPlayer()->GetAttackComponent()->RegisterCombatEventLog(m_pobCombatEventLog);
				m_bCombatLogRegistered = true;
			}
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CHud::CreateHudElement
//!
//------------------------------------------------------------------------------------------
CHudUnit* CHud::CreateHudElement( const CHudUnitDef* pobRenderDef )
{
	CHudUnit* pobNewUnit = pobRenderDef->CreateInstance();
	pobNewUnit->Initialise();
	m_aobHudRenderList.push_back( pobNewUnit );

	m_obElementMap[pobRenderDef] = pobNewUnit;

	return pobNewUnit;
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::RemoveHudElement
//!
//------------------------------------------------------------------------------------------
bool CHud::RemoveHudElement(  CHudUnit* pobUnitToSearchFor )
{
	HudRenderIter unit_it =
	    ntstd::find(m_aobHudRenderList.begin(), 
					m_aobHudRenderList.end(), 
					pobUnitToSearchFor);

   if (unit_it!=m_aobHudRenderList.end())
   {
	   CHudUnit* const pobFoundUnit = *unit_it;
	   m_aobHudRenderList.erase( unit_it );
	   NT_DELETE_CHUNK (Mem::MC_MISC,  pobFoundUnit );
	   return true;
   }

   return false;
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::CreateHudElement
//!
//------------------------------------------------------------------------------------------
CHudUnit* 	 CHud::CreateHudElement( CHashedString pobRenderDefName )
{
	// Find def from object data base
	CHudUnitDef* pobDef = ObjectDatabase::Get().GetPointerFromName<CHudUnitDef*>(pobRenderDefName);

	// Ask HUD manager to create element
	CHudUnit* pobUnit = 0;
	
	if ( pobDef)
	{
		pobUnit = CHud::Get().CreateHudElement( pobDef );
	}

	// Hand a refrence back to the lua so it can do a remove HUD element
	return pobUnit;
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::RemoveHudElement
//!
//------------------------------------------------------------------------------------------
bool		 CHud::RemoveHudElement( CHashedString pobRenderDefName )
{
	// Find def from object data base
	CHudUnitDef* pobDef = ObjectDatabase::Get().GetPointerFromName<CHudUnitDef*>(pobRenderDefName);

	if ( pobDef )
	{
		// Find the render from the map
		ntstd::Map<const CHudUnitDef*, CHudUnit*>::iterator obIt = m_obElementMap.find ( pobDef );

		if ( obIt != m_obElementMap.end() )
		{
			// Remove the HUD element
			CHud::Get().RemoveHudElement( obIt->second );

			// remove the refrence
			m_obElementMap.erase( obIt );
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::ExitHudElement
//!
//------------------------------------------------------------------------------------------
bool		 CHud::ExitHudElement( CHashedString pobRenderDefName )
{
	// Find def from object data base
	CHudUnitDef* pobDef = ObjectDatabase::Get().GetPointerFromName<CHudUnitDef*>(pobRenderDefName);

	if ( pobDef )
	{
		// Find the render from the map
		ntstd::Map<const CHudUnitDef*, CHudUnit*>::iterator obIt = m_obElementMap.find ( pobDef );

		if ( obIt != m_obElementMap.end() )
		{
			// Flag element to remove itself on exit
			obIt->second->RemoveOnExit();

			// Request the element starts an exit
			obIt->second->BeginExit();

			// remove the refrence
			m_obElementMap.erase( obIt );
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::Render
//!
//------------------------------------------------------------------------------------------
void CHud::Render( const float fTimeChange )
{
	if (g_ShellOptions->m_bUseHUD == false)
		return;

#ifndef _GOLD_MASTER
	// Render the debug camera sprite
	if (CInputHardware::Get().GetPadContext() == PAD_CONTEXT_DEBUG)
	{
		// Debug camera
		static const float fDbgCamSize = 32.0f;
		static const float fAmplitude = 15.0f;
		static const float fWaveLength = 4.0f;

		// Update size with a heartbeat style waveform
		float fVal = (float)CTimer::Get().GetGameTime()*fWaveLength;
		float fInt;
		fVal = 1.0f -modf(fVal/PI, &fInt);
		// fVal is in range 0 to 1
		fVal = (1.0f-fVal*fVal*fVal*fVal*fVal*fVal); // Create a sharply rising ramp that tails off,
		fVal = sin (fVal*PI)*fAmplitude; // Scale normalised curve by PI and feed through Sin
		m_obDebugCameraSprite.SetWidth(fDbgCamSize + fVal);
		m_obDebugCameraSprite.SetHeight(fDbgCamSize + fVal);
		m_obDebugCameraSprite.SetPosition(CPoint(	g_VisualDebug->GetDebugDisplayWidth()/2.0f - fDbgCamSize/2.0f,
													g_VisualDebug->GetDebugDisplayHeight() - fDbgCamSize - 44.0f, 0.0f));

		// Render it
		RenderStateBlock::SetBlendMode( EBM_LERP);
		m_obDebugCameraSprite.Render();
	}	
#endif

	// don't render the HUD during ninja sequences
	if	(
		( NSManager::Get().IsNinjaSequencePlaying() == false ) &&
		( ShellMain::Get().IsPausedAny() == false )
		)
	{	
		if (!m_pobHudConfig)
			return;

		CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
		if (pobPlayer && pobPlayer->GetAttackComponent())
		{
			for (int i = 0; i < (int)AB_NUM; i++)
			{
				if ( (VIRTUAL_BUTTON_TYPE)i ==  AB_ACTION )
				{
					bool bHint;
					// Update action button hint from trigger volume manager 
					bHint = CTriggerVolumeManager::Get().m_bNeedsTriggerButtonRender;

					// Update button hint from combat system
					bHint = bHint || pobPlayer->GetAttackComponent()->GetNeedsHintForButton((VIRTUAL_BUTTON_TYPE)i);
					m_pobCombatHUDElements->m_pobButtonHinter->SetHintForButton((VIRTUAL_BUTTON_TYPE)i,bHint);
				}
				else if ( (VIRTUAL_BUTTON_TYPE)i ==  AB_GRAB )
				{
					bool bHint;
		
					// Update button hint from combat system
					bHint = m_bGrabHint || pobPlayer->GetAttackComponent()->GetNeedsHintForButton((VIRTUAL_BUTTON_TYPE)i);
					m_pobCombatHUDElements->m_pobButtonHinter->SetHintForButton((VIRTUAL_BUTTON_TYPE)i,bHint);
				}
				else
				{
					// Update button hint from combat system
					m_pobCombatHUDElements->m_pobButtonHinter->SetHintForButton((VIRTUAL_BUTTON_TYPE)i,pobPlayer->GetAttackComponent()->GetNeedsHintForButton((VIRTUAL_BUTTON_TYPE)i));
				}
			}
		}
	}

	// Look at combat events we care about
	// Done with them now, so clear out ready for the next update
	m_pobCombatEventLog->ClearEvents();

	// Definatly no HUD when Gui pause is active
	if( ShellMain::Get().IsPausedAny() == false )
	{
		// Update any owned objects
		if (m_pobCombatHUDElements )
		{
			if (m_pobCombatHUDElements->m_pobBodyCount)
		{
			m_pobCombatHUDElements->m_pobBodyCount->Update();
		}

			if (m_pobCombatHUDElements->m_pobButtonHinter)
			{
				m_pobCombatHUDElements->m_pobButtonHinter->Update();
			}
		}

		// Render everything
		for (HudRenderIter obIt = m_aobHudRenderList.begin(); 
					obIt != m_aobHudRenderList.end(); )
		{
			CHudUnit* const pobUnit = *obIt;

			if ( pobUnit->Update(fTimeChange) )
			{
				if (!pobUnit->IsDeactive())
				{
					pobUnit->Render();
				}

				obIt++;
			}
			else
			{
				NT_DELETE_CHUNK (Mem::MC_MISC, pobUnit);
				obIt = m_aobHudRenderList.erase( obIt );
			}
		}

		// Timer for message box removal 
		if ( m_fMessageBoxRemoveTime > 0.0f)
		{
			// Message boxes remove in real time as they may be used over pause/gui screens
			// ... Scratch that, they should be in game time so they can be paused on screen
			m_fMessageBoxRemoveTime -= CTimer::Get().GetGameTimeChange();

			if ( m_fMessageBoxRemoveTime < 0.0f )
			{
				RemoveMessageBox();
			}	
		}

		// Timer for message removal 
		if ( m_fMessageRemoveTime > 0.0f)
		{
			m_fMessageRemoveTime -= CTimer::Get().GetGameTimeChange();

			if ( m_fMessageRemoveTime < 0.0f )
			{
				RemoveMessage();
			}	
		}

		// Timer for delayed message trigger
		if ( m_fDelayedMessageTime > 0.0f)
		{
			m_fDelayedMessageTime -= CTimer::Get().GetGameTimeChange();

			if ( m_fDelayedMessageTime < 0.0f )
			{
				if (m_pobHudConfig && m_pobHudConfig->m_pobHudParams )
				{
					CreateMessage( m_obDelayedMessageID, m_pobHudConfig->m_pobHudParams->m_fNewCheckpointPosX,
												m_pobHudConfig->m_pobHudParams->m_fNewCheckpointPosY );
					RemoveMessage( 5.0f );
				}
			}	
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::CreateMessageBox
//!
//------------------------------------------------------------------------------------------
void CHud::CreateMessageBox( ntstd::String obStringId, float fPosX, float fPosY, 
									float fMinWidth, float fMinHeight, 
									float fMaxWidth, float fMaxHeight )
{
	ntstd::Vector<ntstd::String> obStringList;
	obStringList.push_back(obStringId);

	CreateMessageBox ( obStringList, fPosX, fPosY, fMinWidth, fMinHeight, fMaxWidth, fMaxHeight );
}

void CHud::CreateMessageBox( ntstd::Vector<ntstd::String>& obStringList, float fPosX, float fPosY, 
									float fMinWidth, float fMinHeight, 
									float fMaxWidth, float fMaxHeight )
{
	if ( m_pobMessageBoxRenderer )
	{
		RemoveHudElement ( m_pobMessageBoxRenderer );
		m_pobMessageBoxRenderer = 0;
	}

	// Create concreate Message box, with defaults if available
	// scee.sbashow : m_pobMessageBoxRenderer now owns this, and deletes it when it is removed
	//					this was to stop any MessageBoxRenderer instance from referring to their 
	//					message box when it was being deleted in here previously.
	//					
	 if ( m_pobHudConfig && m_pobHudConfig->m_pobMessageBoxDef )
	 {
		 CMessageBox * const pobMessageBox = 
			 NT_NEW_CHUNK (Mem::MC_MISC) CMessageBox( m_pobHudConfig->m_pobMessageBoxDef );

		// Set parameters
		pobMessageBox->m_fPositionX = fPosX;
		pobMessageBox->m_fPositionY = fPosY;
	
		if ( fMinWidth != 0.0f )
			pobMessageBox->m_fBoxMinWidth = fMinWidth;
	
		if ( fMinHeight != 0.0f )
			pobMessageBox->m_fBoxMinHeight = fMinHeight;
	
		if ( fMaxWidth != 0.0f )
			pobMessageBox->m_fBoxMaxWidth = fMaxWidth;
	
		if ( fMaxHeight != 0.0f )
			pobMessageBox->m_fBoxMaxHeight = fMaxHeight;
	
		for ( ntstd::Vector<ntstd::String>::iterator obIt = obStringList.begin();
							obIt != obStringList.end(); obIt++)
		{
			// Create a text render def
			HudTextRenderDef* pobNewTextRenderDef =  NT_NEW_CHUNK (Mem::MC_MISC) HudTextRenderDef( );
			pobNewTextRenderDef->m_obStringTextID = (*obIt);
	
			pobMessageBox->m_aobHudTextDefList.push_back( pobNewTextRenderDef );
		}
	
		m_pobMessageBoxRenderer = (CMessageBoxRenderer*)CreateHudElement ( m_pobHudConfig->m_pobMessageBoxRenderDef );
		m_pobMessageBoxRenderer->Initialise ( CSharedPtr<CMessageBox,Mem::MC_MISC>(pobMessageBox) );
		m_pobMessageBoxRenderer->BeginEnter();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CHud::RemoveMessageBox
//!	Remove the message box now
//!
//------------------------------------------------------------------------------------------
void CHud::RemoveMessageBox( void )
{
	if ( m_pobMessageBoxRenderer )
	{
		m_fMessageBoxRemoveTime = 0.0f;
		m_pobMessageBoxRenderer->BeginExit(true);

		// scee.sbashow : the hud manager will delete this when 
		//		it has become deactivated after the exit fade.
		m_pobMessageBoxRenderer = 0;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::RemoveMessageBox( fDelayTime )
//! Remove the message box after fDelayTime passed
//!
//------------------------------------------------------------------------------------------
void CHud::RemoveMessageBox( float fDelayTime )
{
	if ( fDelayTime < 0.0f)
		fDelayTime = 0.0f;

	m_fMessageBoxRemoveTime = fDelayTime;
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::CreateMessage
//!
//------------------------------------------------------------------------------------------
void CHud::CreateMessage( ntstd::String obStringId, float fPosX, float fPosY)
{
	if ( m_pobMessageRenderer )
	{
		RemoveHudElement ( m_pobMessageRenderer );
		m_pobMessageRenderer = 0;
	}

	// Create concreate Message	
	if ( m_pobHudConfig && m_pobHudConfig->m_pobHudParams && !ntStr::IsNull( obStringId ) )
	{
		m_obMessageDef.m_obStringTextID = obStringId;
		m_obMessageDef.m_obFontName = m_pobHudConfig->m_pobHudParams->m_obNewCheckpointFont;
		m_obMessageDef.m_fTopLeftX = fPosX;
		m_obMessageDef.m_fTopLeftY = fPosY;
		m_obMessageDef.m_fBlendInTime = 0.25f;
		m_obMessageDef.m_fBlendOutTime = 0.25f;
		m_pobMessageRenderer = (HudTextRenderer*)CreateHudElement( &m_obMessageDef );
		m_pobMessageRenderer->BeginEnter( true );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::RemoveMessage
//!	Remove the message now
//!
//------------------------------------------------------------------------------------------
void CHud::RemoveMessage( void )
{
	if ( m_pobMessageRenderer )
	{
		m_fMessageRemoveTime = 0.0f;
		m_pobMessageRenderer->BeginExit(true);

		// scee.sbashow : the hud manager will delete this when 
		//		it has become deactivated after the exit fade.
		m_pobMessageRenderer = 0;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::RemoveMessage( fDelayTime )
//! Remove the message after fDelayTime passed
//!
//------------------------------------------------------------------------------------------
void CHud::RemoveMessage( float fDelayTime )
{
	if ( fDelayTime < 0.0f)
		fDelayTime = 0.0f;

	m_fMessageRemoveTime = fDelayTime;
}

//------------------------------------------------------------------------------------------
//!
//!	CHud::CreateDelayedMessage( ntstd::String obStringId, fDelayTime )
//! Kludgy code to place a delayed message for the new checkpoint once the 
//!	previous checkpoints summary screen has finnished.  Doing it this way as 
//! there is not update on the checkpoint code to create the message after the 
//! summary screen has been done.
//!
//------------------------------------------------------------------------------------------
void CHud::CreateDelayedMessage( ntstd::String obStringId, float fDelayTime)
{
	m_obDelayedMessageID  = obStringId;
	m_fDelayedMessageTime = fDelayTime;
}
