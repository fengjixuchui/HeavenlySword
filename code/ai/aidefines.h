/***************************************************************************************************
*
*	DESCRIPTION		// sorry paul, had to move some enums to here, because of cyclic include dependancies.
*
*	NOTES
*
***************************************************************************************************/

#ifndef _AI_DEFINES_H
#define _AI_DEFINES_H

// horrible, it should logically be owned by AIStates but that stops us forward declaring it,
// so we have to use this rubbish name kludge instead
enum CAIState_ACTION
{
	// these actions use the pathfinder	
	ACTION_WALK,
	ACTION_RUN,

	ACTION_INVESTIGATE_WALK,
	ACTION_PATROL_WALK,

	ACTION_INTERACTING_WALK,

	// this action may use the pathfinder...
	ACTION_STRAFE,
	ACTION_SHUFFLE,

	// these actions have limited movement and are anim based
	ACTION_PATROL_IDLE,
	ACTION_PATROL_LOOK,
	ACTION_PATROL_SPOTTED,
	ACTION_PATROL_ALERT,
	ACTION_PATROL_ALERTED,
	ACTION_SPOTTED_FIRST,
	ACTION_SPOTTED_INFORMED,

	ACTION_INVESTIGATE_LOOK,
	ACTION_INVESTIGATE_SHRUG,

	ACTION_TAUNT,

	ACTION_SCRIPTANIM,

	// Temp formation stuff
	ACTION_INFORMATION_ANIM,

	// the attack action uses the scripted attack behaviour assigned to this AI
	ACTION_ATTACK,

	ACTION_AIM_BALLISTA,
	ACTION_WHACK_A_MOLE_CROUCH,
	ACTION_IDLE_AIMING_CROSSBOWMAN,

	ACTION_DUCK_LONG_LEFT,

	ACTION_NONE
};

// Extension of the action, giving a concept of styles: 
//	"walking"  
//		normal,
//		cool,
//		heavy,
//		

enum CAIState_ACTIONSTYLE
{
	// The normal style - and the state reset to after any Action change
	AS_NORMAL,

	// Used when the entity is aggressive
	AS_AGGRESSIVE,
};

// Extension of the action, giving a concept of using state: 
//	

//enum AI_ACTION_USING

#endif
