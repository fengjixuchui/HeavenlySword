/***************************************************************************************************
*
*	FILE			hudbindings.cpp
*
*	DESCRIPTION		
*
***************************************************************************************************/


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "game/hudbindings.h"

#include "game/combatstyle.h"
#include "hud/hudmanager.h"
#include "hud/messagedata.h"
#include "hud/hudunit.h"
#include "hud/failurehud.h"
#include "hud/healthbar.h"
#include "hud/stylebar.h"
#include "hud/objectivemanager.h"
#include "hud/gamecounter.h"
#include "hud/buttonhinter.h"
#include "gui/guimanager.h"
#include "gui/tutorial.h"
#include "gui/guinotify.h"
#include "game/entity.h"
#include "game/shelllevel.h"
#include "game/shellmain.h"

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::HUD_BossHealthBar()
// DESCRIPTION: Let the Hud manager know that Boss health bar should be displayed
//-------------------------------------------------------------------------------------------------
void HudBindings::HUD_BossHealthBar(void)
{
	lua_bind_warn_msg(("Bind Function 'HUD_BossHealthBar' obsolete\n"));		
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::HUD_BossEntity()
// DESCRIPTION: Let the Hud manager know the Boss entity
//-------------------------------------------------------------------------------------------------
void HudBindings::HUD_BossEntity(CEntity* pobBoss)
{
	UNUSED ( pobBoss );
	lua_bind_warn_msg(("Bind Function 'HUD_BossEntity' obsolete\n"));
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_ActivateSpecialStyleLevel( )
// DESCRIPTION: Activate the special style level
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_ActivateSpecialStyleLevel( void )
{
	if (StyleManager::Exists() && StyleManager::Get().GetHitCounter() )
	{
		StyleManager::Get().GetHitCounter()->ActivateSpecialStyleLevel();
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_DeactivateSpecialStyleLevel( )
// DESCRIPTION: Activate the special style level
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_DeactivateSpecialStyleLevel( void )
{
	if (StyleManager::Exists() && StyleManager::Get().GetHitCounter() )
	{
		StyleManager::Get().GetHitCounter()->DeactivateSpecialStyleLevel();
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_SetSpecialStyleThreshold( )
// DESCRIPTION: Set the special threshold
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_SetSpecialStyleThreshold( int iThreshold )
{
	if (StyleManager::Exists() && StyleManager::Get().GetHitCounter() )
	{
		StyleManager::Get().GetHitCounter()->SetSpecialStyleThreshold( iThreshold );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_ResetSpecialStyleThreshold( )
// DESCRIPTION: Reset the special threshold to hero_style.xml set default
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_ResetSpecialStyleThreshold( void )
{
	if (StyleManager::Exists() && StyleManager::Get().GetHitCounter() )
	{
		StyleManager::Get().GetHitCounter()->ResetSpecialStyleThreshold();
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_RegisterHintEntity( )
// DESCRIPTION: Register an entity for the special bar to hint for
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_RegisterHintEntity( CEntity* pobEnt )
{
	if (StyleManager::Exists() && StyleManager::Get().GetHitCounter() )
	{
		StyleManager::Get().GetHitCounter()->RegisterHintEntity( pobEnt );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_ClearHintEntities( )
// DESCRIPTION: Reset the special bar hint entity list
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_ClearHintEntities( void )
{
	if (StyleManager::Exists() && StyleManager::Get().GetHitCounter() )
	{
		StyleManager::Get().GetHitCounter()->ClearHintEntities();
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_RegisterInteractionHintEntity( )
// DESCRIPTION: Register an entity for the special bar to hint for
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_RegisterInteractionHintEntity( CEntity* pobEnt )
{
	if (CHud::Exists() && CHud::Get().GetButtonHinter() )
	{
		CHud::Get().GetButtonHinter()->RegisterHintEntity( pobEnt );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_RegisterInteractionHintEntity( )
// DESCRIPTION: Register an entity for the special bar to hint for
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_RemoveInteractionHintEntity( CEntity* pobEnt )
{
	if (CHud::Exists() && CHud::Get().GetButtonHinter() )
	{
		CHud::Get().GetButtonHinter()->RemoveHintEntity( pobEnt );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_ClearInteractionHintEntities( )
// DESCRIPTION: Reset the special bar hint entity list
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_ClearInteractionHintEntities( void )
{
	if (CHud::Exists() && CHud::Get().GetButtonHinter() )
	{
		CHud::Get().GetButtonHinter()->ClearHintEntities();
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::AddObjective(text)
// DESCRIPTION: Add a new objective to the HUD objective manager
//				Objective remains on screen for ObjectiveRenderDef->m_fTimeRenderNew seconds
//-------------------------------------------------------------------------------------------------
int HudBindings::AddObjective(const char* pcText)
{
	// Check we have an input
	if ( !pcText )
		return -1;
	
	// Check we have an objective manager
	if ( !CHud::Exists() || !CHud::Get().GetObjectiveManager() )
		return -1;
	
	// Add the objective
	return CHud::Get().GetObjectiveManager()->AddObjective(pcText);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::AddTimedObjective(text, time)
// DESCRIPTION: Add a new timed objective to the HUD objective manager
//-------------------------------------------------------------------------------------------------
int HudBindings::AddTimedObjective(const char* pcText, float fTime, CEntity* m_pobCallbackEntity)
{
	// Check we have an input
	if ( !pcText )
		return -1;
	
	// Check we have an objective manager
	if ( !CHud::Exists() || !CHud::Get().GetObjectiveManager() )
		return -1;
	
	// Add the objective
	return CHud::Get().GetObjectiveManager()->AddTimedObjective(pcText, fTime, m_pobCallbackEntity);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Objective_AddStatusObjective(text)
// DESCRIPTION: Add a new status objective to the HUD objective manager
//				Remains on screen untill removed by Objective_RemoveObjective
//-------------------------------------------------------------------------------------------------
int HudBindings::AddStatusObjective(const char* pcText)
{
	
	// Check we have an input
	if ( !pcText )
		return -1;
	
	// Check we have an objective manager
	if ( !CHud::Exists() || !CHud::Get().GetObjectiveManager() )
		return -1;
	
	// Add the objective
	return CHud::Get().GetObjectiveManager()->AddStatusObjective(pcText);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Objective_AddImageObjective(text)
// DESCRIPTION: Add a new image objective to the HUD objective manager
//-------------------------------------------------------------------------------------------------
int HudBindings::AddImageObjective(const char* pcText)
{
	// Check we have an input
	if ( !pcText )
		return -1;
	
	// Check we have an objective manager
	if ( !CHud::Exists() || !CHud::Get().GetObjectiveManager() )
		return -1;

	// Add the objective
	return CHud::Get().GetObjectiveManager()->AddImageObjective(pcText);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::RemoveObjective(id)
// DESCRIPTION: Add a new timed objective to the HUD objective manager
//-------------------------------------------------------------------------------------------------
void HudBindings::RemoveObjective(int iObjectiveID)
{
	// Check we have an objective manager
	if ( !CHud::Exists() || !CHud::Get().GetObjectiveManager() )
		return;

	// Remove the objective
	CHud::Get().GetObjectiveManager()->RemoveObjective(iObjectiveID);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::PassedObjective(id)
// DESCRIPTION: Add a new timed objective to the HUD objective manager
//-------------------------------------------------------------------------------------------------
void HudBindings::PassedObjective(int iObjectiveID)
{
	// Check we have an objective manager
	if ( !CHud::Exists() || !CHud::Get().GetObjectiveManager() )
		return;

	// Remove the objective
	CHud::Get().GetObjectiveManager()->PassedObjective(iObjectiveID);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::FailedObjective(id)
// DESCRIPTION: Add a new timed objective to the HUD objective manager
//-------------------------------------------------------------------------------------------------
void HudBindings::FailedObjective(int iObjectiveID)
{
	// Check we have an objective manager
	if ( !CHud::Exists() || !CHud::Get().GetObjectiveManager() )
		return;

	// Remove the objective
	CHud::Get().GetObjectiveManager()->FailedObjective(iObjectiveID);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::BodycountCallback( targetKills, callbackEntity)
// DESCRIPTION: Setup a callback to monitor the kill counter
//-------------------------------------------------------------------------------------------------
void HudBindings::BodycountCallback( int iTarget, CEntity* pobCallbackEnt )
{
	// Check we have a body count widget
	if ( !CHud::Exists() || !CHud::Get().GetBodycount() )
		return;

	// Setup the callback
	CHud::Get().GetBodycount()->BodycountCallback ( iTarget, pobCallbackEnt );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::CreateMessageBox(id)
// DESCRIPTION: Add a new message box
//-------------------------------------------------------------------------------------------------
void HudBindings::CreateMessageBox(const char* pcText)
{
	if ( !CHud::Exists() )
		return;

	ntstd::Vector<ntstd::String> obStringList;
	obStringList.push_back( ntstd::String( pcText ) );
	
	CHud::Get().CreateMessageBox( obStringList, 0.5f, 0.5f );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::RemoveMessageBox()
// DESCRIPTION: Remove the active message box
//-------------------------------------------------------------------------------------------------
void HudBindings::RemoveMessageBox()
{
	if ( !CHud::Exists() )
		return;

	CHud::Get().RemoveMessageBox( );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::DelayedRemoveMessageBox(float fDelayTime)
// DESCRIPTION: Remove the active message box
//-------------------------------------------------------------------------------------------------
void HudBindings::DelayedRemoveMessageBox( float fDelayTime )
{
	if ( !CHud::Exists() )
		return;

	CHud::Get().RemoveMessageBox( fDelayTime );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::SetFailureMessageID(const char* pcText)
// DESCRIPTION: Set the failure message ID
//-------------------------------------------------------------------------------------------------
void HudBindings::SetFailureMessageID(const char* pcText)
{
	if ( CHud::Exists() && CHud::Get().GetFailureHud() )
	{
		CHud::Get().GetFailureHud()->SetFailureStringID( ntstd::String ( pcText ) );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::NotifyFailure()
// DESCRIPTION: Trigger the failure HUD/screen
//-------------------------------------------------------------------------------------------------
void HudBindings::NotifyFailure( )
{
	if ( CHud::Exists() && CHud::Get().GetFailureHud() )
	{
		CHud::Get().GetFailureHud()->NotifyFailure();
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::NotifyFailure()
// DESCRIPTION: Trigger the failure HUD/screen
//-------------------------------------------------------------------------------------------------
void HudBindings::ChapterComplete(int iChapter, bool bUnlockNext )
{
	// Activate last checkpoint in the chapter from code.
	ShellLevel* pPlayingLevel = ShellMain::Get().GetCurrRunningLevel();
	ntAssert( pPlayingLevel );

	pPlayingLevel->ActivateChapterCompleteCheckpoint();

	CGuiNotify::ChapterComplete( iChapter, bUnlockNext );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::SetCodePause( bool bPause )
// DESCRIPTION: Set a code pause from the script environment. Used for pausing level during FMV
//-------------------------------------------------------------------------------------------------
void HudBindings::SetCodePause( bool bPaused )
{
	// this internal pause will not result in the normal pause screens etc
	ShellMain::Get().RequestPauseToggle( ShellMain::PM_INTERNAL, bPaused );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::SetData( CHashedString obKey, int iData )
// DESCRIPTION: Bind to set game data that can be presented in a string
//-------------------------------------------------------------------------------------------------
void HudBindings::SetData( CHashedString obKey, int iData )
{
	MessageDataManager* pobMsgDataMan = CHud::Get().GetMessageDataManager();
	ntAssert( pobMsgDataMan );
	
	pobMsgDataMan->SetValue( obKey, iData );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::GetData( CHashedString obKey )
// DESCRIPTION: Bind to set game data that can be presented in a string
//-------------------------------------------------------------------------------------------------
int  HudBindings::GetData( CHashedString obKey )
{
	MessageDataManager* pobMsgDataMan = CHud::Get().GetMessageDataManager();
	ntAssert( pobMsgDataMan );
	
	int iReturnValue;
	pobMsgDataMan->GetValue( obKey, iReturnValue );

	return iReturnValue;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::CreateElement( bool bPause )
// DESCRIPTION: Bind to create a HUD element
//-------------------------------------------------------------------------------------------------
void HudBindings::CreateElement( CHashedString obKey )
{
	if (  CHud::Exists() )
	{
		CHud::Get().CreateHudElement( obKey );
	}
}	

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::RemoveElement( bool bPause )
// DESCRIPTION: Bind to remove a HUD element (previously created by the script)
//-------------------------------------------------------------------------------------------------
void HudBindings::RemoveElement( CHashedString obKey )
{
	if (  CHud::Exists() )
	{
		CHud::Get().RemoveHudElement( obKey );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::RemoveElement( bool bPause )
// DESCRIPTION: Bind to remove a HUD element (previously created by the script)
//-------------------------------------------------------------------------------------------------
void HudBindings::ExitElement( CHashedString obKey )
{
	if (  CHud::Exists() )
	{
		CHud::Get().ExitHudElement( obKey );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: HudBindings::RegisterStyleEvent( luaobject Properties )

//-------------------------------------------------------------------------------------------------
void HudBindings::RegisterStyleEvent ( CHashedString obEvent, int iValue )
{
	if ( iValue != 0 )
	{
		// Use specified value
		StyleManager::Get().RegisterStyleEvent( obEvent, iValue, STYLE_TYPE_MISC);
	}
	else 
	{
		// Use default value
		StyleManager::Get().RegisterStyleEvent( obEvent, STYLE_TYPE_MISC);
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_SetPrologMode( bool bProlog )
// DESCRIPTION: Switch mechanics on LC and Style bar for ch0
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_SetPrologMode( bool bProlog )
{
	if (StyleManager::Exists() )
	{
		StyleManager::Get().SetPrologMode( bProlog );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_SetLifeClock( Days, Hours, Mins, Secs )
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_SetLifeClock( int iDays, int iHours, int iMins, float fSecs )
{
	if (StyleManager::Exists() && StyleManager::Get().GetLifeClock() )
	{

		double dSeconds = fSecs;
		dSeconds += iMins * 60.0;
		dSeconds += iHours   * 3600.0;
		dSeconds += iDays    * 86400.0;

		StyleManager::Get().GetLifeClock()->SetTotalInSeconds( dSeconds );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_IncrementLifeClock( Days, Hours, Mins, Secs )
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_IncrementLifeClock( int iDays, int iHours, int iMins, float fSecs )
{
	if (StyleManager::Exists() && StyleManager::Get().GetLifeClock() )
	{

		float fSeconds = fSecs;
		fSeconds += iMins * 60.0f;
		fSeconds += iHours   * 3600.0f;
		fSeconds += iDays    * 86400.0f;

		StyleManager::Get().GetLifeClock()->IncrementLifeClock( fSeconds );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_DecrementLifeClock( Days, Hours, Mins, Secs )
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_DecrementLifeClock( int iDays, int iHours, int iMins, float fSecs )
{
	if (StyleManager::Exists() && StyleManager::Get().GetLifeClock() )
	{
		float fSeconds = fSecs;
		fSeconds += iMins * 60.0f;
		fSeconds += iHours   * 3600.0f;
		fSeconds += iDays    * 86400.0f;

		StyleManager::Get().GetLifeClock()->DecrementLifeClock( fSeconds );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_SetLifeClockScalar( Scalar )
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_SetLifeClockScalar( float fScalar )
{
	if (StyleManager::Exists() && StyleManager::Get().GetLifeClock() )
	{
		StyleManager::Get().GetLifeClock()->SetScalar( fScalar );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_GetLifeClockScalar(  )
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
float	HudBindings::Lua_GetLifeClockScalar( void )
{
	if (StyleManager::Exists() && StyleManager::Get().GetLifeClock() )
	{
		return StyleManager::Get().GetLifeClock()->GetScalar( );
	}
	return 0.0f;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    HudBindings::Lua_LifeClockDrain( Secs )
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
void HudBindings::Lua_LifeClockDrain( float fSecs )
{
	if (StyleManager::Exists() && StyleManager::Get().GetLifeClock() )
	{
		double dLCRemain = StyleManager::Get().GetLifeClock()->GetTotalInSeconds( );

		float fScale = (float)dLCRemain / fSecs;
		StyleManager::Get().GetLifeClock()->SetScalar( fScale );
	}
}

/***************************************************************************************************
*
*	FUCNTIION		HudBindings::HUDBindings::Register
*
*	DESCRIPTION		
*
***************************************************************************************************/
void HudBindings::Register()
{
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	obGlobals.Set("HUD", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obGlobals["HUD"].Register("BossHealthBar",					HudBindings::HUD_BossHealthBar);
	obGlobals["HUD"].Register("BossEntity",						HudBindings::HUD_BossEntity);
	obGlobals["HUD"].Register("ActivateSpecialStyleLevel",		HudBindings::Lua_ActivateSpecialStyleLevel);
	obGlobals["HUD"].Register("DeactivateSpecialStyleLevel",	HudBindings::Lua_DeactivateSpecialStyleLevel);
	obGlobals["HUD"].Register("SetSpecialStyleThreshold",		HudBindings::Lua_SetSpecialStyleThreshold);
	obGlobals["HUD"].Register("ResetSpecialStyleThreshold",		HudBindings::Lua_ResetSpecialStyleThreshold);
	obGlobals["HUD"].Register("RegisterHintEntity",				HudBindings::Lua_RegisterHintEntity);
	obGlobals["HUD"].Register("ClearHintEntities",				HudBindings::Lua_ClearHintEntities);
	obGlobals["HUD"].Register("CreateElement",					HudBindings::CreateElement);
	obGlobals["HUD"].Register("RemoveElement",					HudBindings::RemoveElement);
	obGlobals["HUD"].Register("ExitElement",					HudBindings::ExitElement);
	obGlobals["HUD"].Register("RegisterInteractionHintEntity",	HudBindings::Lua_RegisterInteractionHintEntity);
	obGlobals["HUD"].Register("RemoveInteractionHintEntity",	HudBindings::Lua_RemoveInteractionHintEntity);
	obGlobals["HUD"].Register("ClearInteractionHintEntities",	HudBindings::Lua_ClearInteractionHintEntities);
	
	obGlobals.Set("Objective", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obGlobals["Objective"].Register("AddObjective",			HudBindings::AddObjective);
	obGlobals["Objective"].Register("AddTimedObjective",	HudBindings::AddTimedObjective);
	obGlobals["Objective"].Register("AddStatusObjective",	HudBindings::AddStatusObjective);
	obGlobals["Objective"].Register("RemoveObjective",		HudBindings::RemoveObjective);
	obGlobals["Objective"].Register("PassedObjective",		HudBindings::PassedObjective);
	obGlobals["Objective"].Register("FailedObjective",		HudBindings::FailedObjective);
	obGlobals["Objective"].Register("AddImageObjective",	HudBindings::AddImageObjective);
	obGlobals["Objective"].Register("BodycountCallback",	HudBindings::BodycountCallback);
	
	obGlobals.Set("MessageBox", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obGlobals["MessageBox"].Register("Create",			HudBindings::CreateMessageBox);
	obGlobals["MessageBox"].Register("Remove",			HudBindings::RemoveMessageBox);
	obGlobals["MessageBox"].Register("DelayedRemove",	HudBindings::DelayedRemoveMessageBox);

	obGlobals.Set("Game", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obGlobals["Game"].Register("SetFailureMessageID",	HudBindings::SetFailureMessageID);
	obGlobals["Game"].Register("NotifyFailure",			HudBindings::NotifyFailure);
	obGlobals["Game"].Register("ChapterComplete",		HudBindings::ChapterComplete);
	obGlobals["Game"].Register("SetCodePause",			HudBindings::SetCodePause);
	obGlobals["Game"].Register("GetData",				HudBindings::GetData);
	obGlobals["Game"].Register("SetData",				HudBindings::SetData);
	obGlobals["Game"].Register("RegisterStyleEvent",	HudBindings::RegisterStyleEvent);
	obGlobals["Game"].Register("SetPrologMode",			HudBindings::Lua_SetPrologMode);
	obGlobals["Game"].Register("SetLifeClock",			HudBindings::Lua_SetLifeClock);
	obGlobals["Game"].Register("IncrementLifeClock",	HudBindings::Lua_IncrementLifeClock);
	obGlobals["Game"].Register("DecrementLifeClock",	HudBindings::Lua_DecrementLifeClock);

	obGlobals["Game"].Register("SetLifeClockScalar",	HudBindings::Lua_SetLifeClockScalar);
	obGlobals["Game"].Register("GetLifeClockScalar",	HudBindings::Lua_GetLifeClockScalar);
	obGlobals["Game"].Register("LifeClockDrain",		HudBindings::Lua_LifeClockDrain);
}
