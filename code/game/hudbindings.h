/***************************************************************************************************
*
*	FILE			tutorialbindings.h
*
*	DESCRIPTION		
*
***************************************************************************************************/

#ifndef _HUDBINDINGS_H
#define _HUDBINDINGS_H

#include "game/luaglobal.h"
#include "game/luardebug.h"
#include "game/luahelp.h"

class CHudUnit;

/***************************************************************************************************
*
*	CLASS			HudBindings
*
*	DESCRIPTION		
*
***************************************************************************************************/

class HudBindings
{
public:
	static void HUD_BossHealthBar(void);
	static void HUD_BossEntity(CEntity* pobBoss);
	static void Lua_ActivateSpecialStyleLevel( void );
	static void Lua_DeactivateSpecialStyleLevel( void );
	static void Lua_RegisterHintEntity( CEntity* pobEnt );
	static void Lua_ClearHintEntities( void );
	static void Lua_SetSpecialStyleThreshold( int iThreshold );
	static void Lua_ResetSpecialStyleThreshold( void );

	static void Lua_RegisterInteractionHintEntity( CEntity* pobEnt );
	static void Lua_RemoveInteractionHintEntity( CEntity* pobEnt );
	static void Lua_ClearInteractionHintEntities( void );

	static int  AddObjective(const char* pcText);
	static int	AddTimedObjective(const char* pcText, float fTime, CEntity* m_pobCallbackEntity);
	static int  AddStatusObjective(const char* pcText);
	static int  AddImageObjective(const char* pcText);
	static void RemoveObjective(int iObjectiveID);
	static void PassedObjective(int iObjectiveID);
	static void FailedObjective(int iObjectiveID);
	static void BodycountCallback( int iTarget, CEntity* pobCallbackEnt );

	static void CreateMessageBox(const char* pcText);
	static void RemoveMessageBox();
	static void DelayedRemoveMessageBox( float fDelayTime );

	static void SetFailureMessageID(const char* pcText);
	static void NotifyFailure( );
	static void ChapterComplete(int iChapter, bool bUnlockNext );
	static void SetCodePause( bool bPaused );
	static void SetData( CHashedString obKey, int iData );
	static int  GetData( CHashedString obKey );

	static void CreateElement( CHashedString obKey );
	static void RemoveElement( CHashedString obKey );
	static void ExitElement( CHashedString obKey );

	static void RegisterStyleEvent ( CHashedString obEvent, int iValue = 0);

	static void Lua_SetPrologMode( bool bProlog );
	static void Lua_SetLifeClock( int iDays, int iHours, int iMins, float fSecs );
	static void Lua_IncrementLifeClock( int iDays, int iHours, int iMins, float fSecs );
	static void Lua_DecrementLifeClock( int iDays, int iHours, int iMins, float fSecs );
	static void Lua_SetLifeClockScalar( float fScalar );
	static float Lua_GetLifeClockScalar( void );
	static void Lua_LifeClockDrain( float fSecs );

	static void Register();
};


#endif // _HUDBINDINGS_H
