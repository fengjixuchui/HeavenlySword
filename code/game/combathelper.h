//----------------------------------------------------------------------------------------------------
//!
//!	\file combathelper.h
//!	Combat Helper functions, basically the same as the bind functions without the LUA, and can
//! be called from anywhere that includes this file
//!
//----------------------------------------------------------------------------------------------------

#ifndef _COMBATHELPER_H
#define _COMBATHELPER_H

class CEntity;

class CombatHelper
{
public:
	static bool Combat_GenerateStrike( CEntity* pobObject, CEntity* pobAttacker, CEntity* pobTarget, const CHashedString& pcAttackDef, int iHitArea = -1);
};

#endif //_COMBATHELPER_H
