/***************************************************************************************************
*
*	FILE			tutorialbindings.cpp
*
*	DESCRIPTION		
*
***************************************************************************************************/


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "game/tutorialbindings.h"

#include "gui/guimanager.h"
#include "gui/tutorial.h"
#include "game/entity.h"

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    TutorialBindings::ScreenMessage( TextID, Font, Duration)
// DESCRIPTION: Display message ID (TextID) on screen with Font for Duration seconds
//-------------------------------------------------------------------------------------------------
void TutorialBindings::ScreenMessage(const char* pcTextID, const char* pcFont, float fDuration)
{
	if ( !CTutorialManager::Exists() )
		return;

	CTutorialManager::Get().AddScreenMessage( pcTextID, pcFont, fDuration);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    TutorialBindings::ScreenMessagePosition( TextID, Font, Duration)
// DESCRIPTION: Change the position for the screen messages
//-------------------------------------------------------------------------------------------------
void TutorialBindings::ScreenMessagePosition(float fX, float fY)
{
	if ( !CTutorialManager::Exists() )
		return;

	CTutorialManager::Get().ScreenMessagePosition( fX, fY );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    TutorialBindings::RemoveMessage( )
// DESCRIPTION: Remove message from screen
//-------------------------------------------------------------------------------------------------
void TutorialBindings::RemoveMessage()
{
	if ( !CTutorialManager::Exists() )
		return;

	CTutorialManager::Get().RemoveScreenMessage( );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Tutorial_WatchEvent( Event, Entity, Message )
// DESCRIPTION: Add an Event watch in the tutorial system.  On success reply to Entity with Message
//-------------------------------------------------------------------------------------------------
void TutorialBindings::WatchEvent( CHashedString obEvent, CEntity* pobReplyEnt, CHashedString obMsg)
{
	if ( !CTutorialManager::Exists() )
		return;

	CTutorialManager::Get().AddWatch(obEvent, pobReplyEnt, obMsg);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    WatchSpecificMove( Move, Hit, Entity, Message )
// DESCRIPTION: Add an Event watch in the tutorial system.  On success reply to Entity with Message
//-------------------------------------------------------------------------------------------------
void TutorialBindings::WatchSpecificMove( CHashedString obMove, bool bHit, CEntity* pobReplyEnt, CHashedString obMsg)
{
	if ( !CTutorialManager::Exists() )
		return;

	if ( bHit )
	{
		CTutorialManager::Get().AddWatch( CHashedString("SPECIFIC_STRIKE_HIT"), pobReplyEnt, obMsg, (void*)&obMove );
	}
	else
	{
		CTutorialManager::Get().AddWatch( CHashedString("SPECIFIC_STRIKE"), pobReplyEnt, obMsg, (void*)&obMove );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Tutorial_WatchEventAI( AI, Event, Entity, Message )
// DESCRIPTION: Add an Event watch in the tutorial system.  On success reply to Entity with Message
//-------------------------------------------------------------------------------------------------
void TutorialBindings::WatchEventAI(CEntity* pobAIEnt, CHashedString obEvent, CEntity* pobReplyEnt, CHashedString obMsg)
{
	if ( !CTutorialManager::Exists() )
		return;

	CTutorialManager::Get().AddWatch(obEvent, pobReplyEnt, obMsg, 0, pobAIEnt);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    WatchSpecificMove( AI, Move, Hit, Entity, Message )
// DESCRIPTION: Add an Event watch in the tutorial system.  On success reply to Entity with Message
//-------------------------------------------------------------------------------------------------
void TutorialBindings::WatchSpecificMoveAI(CEntity* pobAIEnt, CHashedString obMove, bool bHit, CEntity* pobReplyEnt, CHashedString obMsg)
{
	if ( !CTutorialManager::Exists() )
		return;

	if ( bHit )
	{
		CTutorialManager::Get().AddWatch( CHashedString("SPECIFIC_STRIKE_HIT"), pobReplyEnt, obMsg, (void*)&obMove, pobAIEnt );
	}
	else
	{
		CTutorialManager::Get().AddWatch( CHashedString("SPECIFIC_STRIKE"), pobReplyEnt, obMsg, (void*)&obMove, pobAIEnt );
	}
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    WatchAttackWindow( Window, ReplyEntity, EnterMessage, ExitMessage )
// DESCRIPTION: Add an Event watch in the tutorial system.  On success reply to Entity with Message
//-------------------------------------------------------------------------------------------------
void TutorialBindings::WatchAttackWindow(CHashedString obWindow, CEntity* pobReplyEnt, CHashedString obEnterMsg, CHashedString obExitMsg )
{
	if ( !CTutorialManager::Exists() )
		return;

	CTutorialManager::Get().AddAttackWindowWatch( 0, obWindow, pobReplyEnt, obEnterMsg, obExitMsg );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    WatchAttackWindowAI( AI, Window, ReplyEntity, EnterMessage, ExitMessage )
// DESCRIPTION: Add an Event watch in the tutorial system.  On success reply to Entity with Message
//-------------------------------------------------------------------------------------------------
void TutorialBindings::WatchAttackWindowAI(CEntity* pobAIEnt, CHashedString obWindow, CEntity* pobReplyEnt, CHashedString obEnterMsg, CHashedString obExitMsg )
{
	if ( !CTutorialManager::Exists() )
		return;

	CTutorialManager::Get().AddAttackWindowWatch( pobAIEnt, obWindow, pobReplyEnt, obEnterMsg, obExitMsg );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Tutorial_ClearWatches( )
// DESCRIPTION: Remove any remaining watches
//-------------------------------------------------------------------------------------------------
void TutorialBindings::ClearWatches( void )
{
	if ( !CTutorialManager::Exists() )
		return;

	CTutorialManager::Get().ClearWatches();
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    AddTimeScalar( )
// DESCRIPTION: Add a tutorial time scalar
//-------------------------------------------------------------------------------------------------
void	TutorialBindings::AddTimeScalar ( float fTimeScalar, float fBlendTime )
{
	if ( !CTutorialManager::Exists() )
		return;

	CTutorialManager::Get().AddTimeScalar ( fTimeScalar, fBlendTime );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    ClearTimeScalar( )
// DESCRIPTION: Remove the tutorial time scalar
//-------------------------------------------------------------------------------------------------
void	TutorialBindings::ClearTimeScalar ( float fBlendTime )
{
	if ( !CTutorialManager::Exists() )
		return;

	CTutorialManager::Get().ClearTimeScalar ( fBlendTime );
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    TimeScalarCallback( )
// DESCRIPTION: Add a callback msg when the tutorial time scalar is done
//-------------------------------------------------------------------------------------------------
void	TutorialBindings::TimeScalarCallback ( CEntity* pobReplyEnt, CHashedString obMsg )
{
	if ( !CTutorialManager::Exists() )
		return;

	CTutorialManager::Get().TimeScalarCallback ( pobReplyEnt, obMsg );
}

/***************************************************************************************************
*
*	FUCNTIION		DebugBindings::Register
*
*	DESCRIPTION		
*
***************************************************************************************************/
void TutorialBindings::Register()
{
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	// New Way - Register Debug functions in a debug library/table
	obGlobals.Set("Tutorial", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obGlobals["Tutorial"].Register("ScreenMessage",			TutorialBindings::ScreenMessage);
	obGlobals["Tutorial"].Register("ScreenMessagePosition",	TutorialBindings::ScreenMessagePosition);
	obGlobals["Tutorial"].Register("RemoveMessage",			TutorialBindings::RemoveMessage);
	obGlobals["Tutorial"].Register("WatchEvent",			TutorialBindings::WatchEvent);
	obGlobals["Tutorial"].Register("WatchSpecificMove",		TutorialBindings::WatchSpecificMove);
	obGlobals["Tutorial"].Register("WatchEventAI",			TutorialBindings::WatchEventAI);
	obGlobals["Tutorial"].Register("WatchSpecificMoveAI",	TutorialBindings::WatchSpecificMoveAI);
	obGlobals["Tutorial"].Register("WatchAttackWindow",		TutorialBindings::WatchAttackWindow);
	obGlobals["Tutorial"].Register("WatchAttackWindowAI",	TutorialBindings::WatchAttackWindowAI);

	obGlobals["Tutorial"].Register("ClearWatches",			TutorialBindings::ClearWatches);

	obGlobals["Tutorial"].Register("AddTimeScalar",			TutorialBindings::AddTimeScalar);
	obGlobals["Tutorial"].Register("ClearTimeScalar",		TutorialBindings::ClearTimeScalar);
	obGlobals["Tutorial"].Register("TimeScalarCallback",	TutorialBindings::TimeScalarCallback);
}
