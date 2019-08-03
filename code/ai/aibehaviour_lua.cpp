//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aibehaviour_lua.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------
#include "ai/aibehaviour_lua.h"
#include "game/aicomponent.h"
#include "input/inputhardware.h"


bool AIBehaviour_Lua::States(const STATE_MACHINE_EVENT eEvent, const int iState, const float)
{

BeginStateMachine
	
	/////////////////////////////////////
	//  Move to Formation
	/////////////////////////////////////
	AIState(0)
		OnEnter
			
		OnUpdate
			
		OnExit
		
EndStateMachine
	
	return false;
}
