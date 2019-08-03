/***************************************************************************************************
*
*	FILE			tutorialbindings.h
*
*	DESCRIPTION		
*
***************************************************************************************************/

#ifndef _TUTORIALBINDINGS_H
#define _TUTORIALBINDINGS_H

#include "game/luaglobal.h"
#include "game/luardebug.h"
#include "game/luahelp.h"

/***************************************************************************************************
*
*	CLASS			TutorialBindings
*
*	DESCRIPTION		
*
***************************************************************************************************/

class TutorialBindings
{
public:
	static void ScreenMessage(const char* pcTextID, const char* pcFont, float fDuration);
	static void ScreenMessagePosition(float fX, float fY);
	static void RemoveMessage(void);
	static void WatchEvent(CHashedString obEvent, CEntity* pobEnt, CHashedString obMsg);
	static void WatchSpecificMove( CHashedString obMove, bool bHit, CEntity* pobReplyEnt, CHashedString obMsg);
	static void WatchEventAI(CEntity* pobAIEnt, CHashedString obEvent, CEntity* pobReplyEnt, CHashedString obMsg);
	static void WatchSpecificMoveAI(CEntity* pobAIEnt, CHashedString obMove, bool bHit, CEntity* pobReplyEnt, CHashedString obMsg);
	
	static void WatchAttackWindow(CHashedString obWindow, CEntity* pobReplyEnt, CHashedString obEnterMsg, CHashedString obExitMsg );
	static void WatchAttackWindowAI(CEntity* pobAIEnt, CHashedString obWindow, CEntity* pobReplyEnt, CHashedString obEnterMsg, CHashedString obExitMsg );

	static void ClearWatches( void);

	static void	AddTimeScalar ( float fTimeScalar, float fBlendTime );		
	static void	ClearTimeScalar ( float fBlendTime );
	static void	TimeScalarCallback ( CEntity* pobReplyEnt, CHashedString obMsg );

	static void Register();
};


#endif // _TUTORIALBINDINGS_H
