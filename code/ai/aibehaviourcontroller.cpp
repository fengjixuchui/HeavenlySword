//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aibehaviourcontroller.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                         
                                                                                         
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "game/aicomponent.h"
#include "game/entitymanager.h"

#include "ai/aistates.h"
#include "ai/aibehaviourmanager.h"

enum eBEHAVIOUR_ACTION
{
	BEHAVIOUR_ACTION_PUSH,
	BEHAVIOUR_ACTION_POP,
	BEHAVIOUR_ACTION_REPLACE,
};
/* GCC says this is unused.
static void
DoBehaviourAction(
	eBEHAVIOUR_ACTION				eAction,
	CAIBehaviourManager::eBEHAVIOUR	eNewBehaviour,
	CAIBehaviourManager*			pobManager,
	CEntity*						pTarget = 0)
{
	if (eAction == BEHAVIOUR_ACTION_PUSH || eAction == BEHAVIOUR_ACTION_REPLACE)
	{
		CAIStateMachine*	pobNewBehaviour(
			CAIBehaviourManager::CreateBehaviour( eNewBehaviour, pobManager->GetParent(), pTarget ) );
		ntAssert( pobNewBehaviour );

		if (eAction == BEHAVIOUR_ACTION_PUSH)
		{
			pobManager->Push( *pobNewBehaviour );
		}
		else
		{
			pobManager->Replace( *pobNewBehaviour );
		}
	}
	else if (eAction == BEHAVIOUR_ACTION_POP)
	{
		pobManager->Pop();
	}
}
*/
#define PUSH(behaviour)				DoBehaviourAction( BEHAVIOUR_ACTION_PUSH, behaviour, pobManager, 0 ) 
#define PUSH2(behaviour, target)	DoBehaviourAction( BEHAVIOUR_ACTION_PUSH, behaviour, pobManager, target )
#define POP()						DoBehaviourAction( BEHAVIOUR_ACTION_POP, CAIBehaviourManager::IDLE, pobManager, 0 ) 
#define REPLACE(behaviour)			DoBehaviourAction( BEHAVIOUR_ACTION_REPLACE, behaviour, pobManager, 0 ) 
#define REPLACE2(behaviour, target)	DoBehaviourAction( BEHAVIOUR_ACTION_REPLACE, behaviour, pobManager, target )





bool
AIMeleeBehaviourController::ProcessMessage( BEHAVIOUR_MESSAGE eMessage, CAIBehaviourManager* pobManager )
{
	// DEPRECATED
	UNUSED( eMessage );
	UNUSED( pobManager );

	ntPrintf( "OH NOES! DEPRECATION! AIMeleeBehaviourController\n" );

	/*
	switch (eMessage)
	{
	case ATTACK_INCOMING:
		switch (pobManager->CurrentBehaviourType())
		{
		case CAIBehaviourManager::CHASE:
		case CAIBehaviourManager::FORMATION:
			PUSH2( CAIBehaviourManager::LOCKON, CEntityManager::Get().GetPlayer() );
			break;
	
		case CAIBehaviourManager::INVESTIGATE:
			REPLACE( CAIBehaviourManager::CHASE );
			PUSH2( CAIBehaviourManager::LOCKON, CEntityManager::Get().GetPlayer() );
			break;
	
		case CAIBehaviourManager::LOCKON:
			break;

		default:
			PUSH( CAIBehaviourManager::CHASE );
			PUSH2( CAIBehaviourManager::LOCKON, CEntityManager::Get().GetPlayer() );
			break;
		}

		break;
		
	case ATTACK_TOFARFROMTARGET:
		REPLACE( CAIBehaviourManager::CHASE );
		break;
		
	case COMBAT_NOATTACKQUEUED:
		POP();
		break;
		
	case CHASE_LOSTTARGET:
		REPLACE( CAIBehaviourManager::INVESTIGATE );
		break;
	case CHASE_INATTACKRANGE:
		PUSH2( CAIBehaviourManager::LOCKON, CEntityManager::Get().GetPlayer() );
		break;
		
	case COVER_FINISHEDHIDING:
		POP();
		break;

	case INVESTIGATE_FOUNDNOTHING:
		POP();
		break;
	case INVESTIGATE_FOUNDTARGET:
		REPLACE( CAIBehaviourManager::CHASE );
		break;

	case PATROL_SEENSOMETHING:
		PUSH( CAIBehaviourManager::INVESTIGATE );
		break;
	case PATROL_SEENENEMY:
		PUSH( CAIBehaviourManager::CHASE );
		break;

	case LOCKON_LOCKOFF:
		POP();
		break;

	default:
		return false;
		break;
	}
	*/
	return true;
}







bool
AIRangedBehaviourController::ProcessMessage( BEHAVIOUR_MESSAGE eMessage, CAIBehaviourManager* pobManager )
{
	// DEPRECATED
	UNUSED( eMessage );
	UNUSED( pobManager );

	ntPrintf( "OH NOES! DEPRECATION! AIRangedBehaviourController\n" );
	
	/*
	switch (eMessage)
	{
	case ATTACK_TOFARFROMTARGET:
		REPLACE( CAIBehaviourManager::CHASE );
		break;
		
	case COMBAT_NOATTACKQUEUED:
		POP();
		break;
		
	case CHASE_LOSTTARGET:
		REPLACE( CAIBehaviourManager::INVESTIGATE );
		break;
	case CHASE_INATTACKRANGE:
		REPLACE( CAIBehaviourManager::ATTACK );
		break;
		
	case COVER_FINISHEDHIDING:
		POP();
		break;

	case INVESTIGATE_FOUNDNOTHING:
		POP();
		break;
	case INVESTIGATE_FOUNDTARGET:
		REPLACE( CAIBehaviourManager::CHASE );
		break;

	case PATROL_SEENSOMETHING:
		PUSH( CAIBehaviourManager::INVESTIGATE );
		break;
	case PATROL_SEENENEMY:
		PUSH( CAIBehaviourManager::CHASE );
		break;

	case LOCKON_LOCKOFF:
		POP();
		break;

	default:
		return false;
		break;
	}
	*/
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		AILuaBehaviourController::AILuaBehaviourController
*
*	DESCRIPTION		AILuaBehaviourController cosntructor
*
***************************************************************************************************/

AILuaBehaviourController::AILuaBehaviourController(const CAIComponent& obParent) : 
	m_obParent( obParent )
{
	// Make sure that the Lua script called is aware of the Entity it's attached to
	CEntity* pOldTarg = CLuaGlobal::Get().GetTarg();
	CLuaGlobal::Get().SetTarg(obParent.GetParent());

	NinjaLua::LuaObject funcObject = CLuaGlobal::Get().State().GetGlobals()["AIMakeController"];
	NinjaLua::LuaFunctionRet<NinjaLua::LuaObject> AIMakeController(funcObject);
	NinjaLua::LuaObject obNewController = AIMakeController(CLuaGlobal::Get().State().GetGlobals()["AIController_Test"]);
	CLuaGlobal::Get().State().GetGlobals().Set( "newController", obNewController );

	// revert back to the old target.. 
	CLuaGlobal::Get().SetTarg(pOldTarg);
}


/***************************************************************************************************
*
*	FUNCTION		AILuaBehaviourController::~AILuaBehaviourController
*
*	DESCRIPTION		AILuaBehaviourController destructor - currently does nothing
*
***************************************************************************************************/

AILuaBehaviourController::~AILuaBehaviourController()
{
//	m_obLuaController.AssignNil( &CLuaGlobal::Get().GetState() );
}


/***************************************************************************************************
*
*	FUNCTION		AILuaBehaviourController::ProcessMessage
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool AILuaBehaviourController::ProcessMessage( const char* pcMessage )
{
	NinjaLua::LuaObject obLuaController = CLuaGlobal::Get().State().GetGlobals()["newController"];

	ntAssert(obLuaController.IsTable());

	NinjaLua::LuaObject handler = obLuaController["CallEvent"];

	if(!handler.IsFunction())
		return false;

	CEntity* pOldTarg = CLuaGlobal::Get().GetTarg();
	CLuaGlobal::Get().SetTarg(m_obParent.GetParent());

	NinjaLua::LuaFunction fn(handler);
	fn(obLuaController, pcMessage);

	CLuaGlobal::Get().SetTarg(pOldTarg);

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		AILuaBehaviourController::ProcessMessage
*
*	DESCRIPTION		This is a wrapper method directing message enums to the method above
*
***************************************************************************************************/

bool AILuaBehaviourController::ProcessMessage( BEHAVIOUR_MESSAGE eBMsg, CAIBehaviourManager* )
{
	char *pacMessageInterpreter[BEHAVIOUR_MESSAGE_MAX] = 
	{
		"ATTACK_INCOMING",
		"ATTACK_TOFARFROMTARGET",
		"ATTACK_IN_SHOOTING_RANGE",
		"ATTACK_IN_MELE_RANGE",
		"ATTACK_LOSTTARGET",
		"COMBAT_NOATTACKQUEUED",
		"CHASE_LOSTTARGET",
		"CHASE_INATTACKRANGE",
		"COVER_FINISHEDHIDING",
		"INVESTIGATE_FOUNDNOTHING",
		"INVESTIGATE_FOUNDTARGET",
		"SEENENEMY",
		"PATROL_SEENSOMETHING",
		"PATROL_SEENENEMY",
		"ENEMY_DEAD",
		"PATH_NOT_FOUND",
		"MSG_MOVING_TO_COVER_POINT",
		"COVER_POINT_REACHED",
		"DESTINATION_REACHED",
		"DESTINATION_UNREACHABLE",
		"TIME_OUT",
		"LOCKON_LOCKOFF",
		"ANIM_STARTED",
		"ANIM_FAILED",
		"ANIM_COMPLETE",
		"ANIM_LOOP_COMPLETED",
		"LOCATOR_REACHED",
		"LOCATOR_UNREACHABLE",
		"USING_OBJECT",
		"OBJECT_USED",
		"OBJECT_UNREACHABLE",
		"FACING_ENTITY",
	};

	return ProcessMessage( pacMessageInterpreter[eBMsg] );
}

