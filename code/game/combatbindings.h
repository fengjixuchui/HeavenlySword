/***************************************************************************************************
*
*	FILE			combatbindings.h
*
*	DESCRIPTION		Exposes combat related functionality to the scripting environment
*
***************************************************************************************************/

#ifndef _COMBATBINDINGS_H
#define _COMBATBINDINGS_H

/***************************************************************************************************
*
*	CLASS			CCombatBindings
*
*	DESCRIPTION		A Static class to expose an interface to the combat system, making it available
*					to the scripting environment
*
***************************************************************************************************/

class CCombatBindings
{
public:

	// Register the exposed functions with the scripting environment
	static void Register( void );
};

#endif // _COMBATBINDINGS_H
