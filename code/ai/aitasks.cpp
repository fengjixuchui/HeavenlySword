/***************************************************************************************************
*
*	DESCRIPTION		File defining extended task functionality.
*
*	NOTES
*
***************************************************************************************************/

#include "ai/aitasks.h"
#include "ai/ainavnodes.h"
#include "ai/aiformation.h"
#include "ai/ainavgraphmanager.h"
#include "ai/ainavfollowpoint.h"
#include "ai/aistatemachine.h"
#include "ai/aiformationcomponent.h"

#include "game/attacks.h"
#include "game/entityinfo.h"
#include "game/aicomponent.h"
#include "game/entitymanager.h"
#include "game/messagehandler.h"
#include "game/luahelp.h"
#include "game/luaexptypes.h"

#ifdef PLATFORM_PC
#include "input/inputhardware.h"
#endif

#include "objectdatabase/dataobject.h"

/***************************************************************************************************
*
*	FUNCTION		CAIWalkToPointTask::OnFirstUpdate
*
*	DESCRIPTION		
*
*	OWNER			GavB, 16/02/05
*
***************************************************************************************************/

void CAIWalkToPointTask::OnFirstUpdate( CAIComponent* pobParent )
{
	CPoint	obEntityPos	= pobParent->GetParent()->GetPosition();

	if (!pobParent->GetFollowPoint().PosSet())
	{
		pobParent->GetFollowPoint().SetPos( obEntityPos );
	}

	// if the action destination has been set this frame, then we should calculate a path
	// KillPath is safe to call even when no path has been allocated
	pobParent->KillPath();
	pobParent->MakePathToDest( &m_obDestPoint );
	
	pobParent->GetFollowPoint().SetPos( obEntityPos );
	pobParent->GetFollowPoint().Update( 10.0f/60.0f );
}

/***************************************************************************************************
*
*	FUNCTION		CAIWalkToPointTask::Update
*
*	DESCRIPTION		
*
*	OWNER			GavB, 16/02/05
*
***************************************************************************************************/

void CAIWalkToPointTask::Update( float fTimeChange, CAIComponent* pobParent )
{
	UNUSED(fTimeChange);

	// get current destination
	CPoint	obEntityPos	= pobParent->GetParent()->GetPosition();
	CPoint	obTarget	= m_obDestPoint;
	CPoint	obHeading	= obTarget - obEntityPos;
	float   fLenSqr		= obHeading.LengthSquared();

	obHeading.Y() = 0;

	// Ensure the correct controller is activated
	pobParent->ActivateController(CAIComponent::MC_WALKING);

	pobParent->GetFollowPoint().SetDistSquared( fLenSqr );

	CDirection	obHeadingDir(obHeading);
	obHeadingDir.Normalise();

	float fClippedLenSqr = ntstd::Min( fLenSqr, 1.0f );

	pobParent->SetMovementDirection( obHeadingDir );
	pobParent->SetMovementMagnitude( m_fMagnitude * fClippedLenSqr );

	// post current position to avoidance manager
	//CAINavGraphManager::Get().GetRepulsion()->UpdatePos( obEntityPos, pobParent->GetAvoidanceID() );
	//CAINavGraphManager::Get().GetRepulsion()->UpdateDir( obHeadingDir, pobParent->GetAvoidanceID() );

	if( fLenSqr < (0.3 * 0.3) )
	{
		ntAssert( false );
		//pobParent->HandleEvent("OnActionComplete");
		ntError( false );

		SetState(AI_TASK_COMPLETED);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAIWalkToPointTask::Update
*
*	DESCRIPTION		
*
*	OWNER			GavB, 03/03/05
*
***************************************************************************************************/

void CAIUpdateFormation::Update( float fTimeChange, CAIComponent* pobParent )
{
	UNUSED(fTimeChange);
	UNUSED(pobParent);
	ntAssert( pobParent );
	ntAssert( pobParent->GetAIFormationComponent() );
}


/***************************************************************************************************
*
*	FUNCTION		CAIUpdateAttackPosition::Update
*
*	DESCRIPTION		
*
*	OWNER			GavB, 03/03/05
*
***************************************************************************************************/

void CAIUpdateAttackPosition::Update( float fTimeChange, CAIComponent* pobParent )
{
	// In this state we can update the combat component
	pobParent->UpdateCombatComponent(fTimeChange, m_bUpdateMovement);
}

/***************************************************************************************************
*
*	FUNCTION		CAIForwardPlayerCombatStateChanges::Update
*
*	DESCRIPTION		
*
*	OWNER			GavB, 31/03/05
*
***************************************************************************************************/

void CAIForwardPlayerCombatStateChanges::Update( float fTimeChange, CAIComponent* pobParent )
{
	UNUSED(fTimeChange);

	CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
	ntAssert( pobPlayer );
	ntAssert( pobPlayer->GetAttackComponent() );

	COMBAT_STATE eState = pobPlayer->GetAttackComponent()->AI_Access_GetState();

	if( m_ePlayerState != eState )
	{
		const char* pcPrefix = "PLAYER_";
		const char* pcEnumName = ObjectDatabase::Get().GetGlobalEnum( "COMBAT_STATE" ).GetName( eState ).c_str();
		static char pcEventName[ MAX_PATH ];

		strcpy( pcEventName, pcPrefix );
		strcat( pcEventName, pcEnumName );

		// ntPrintf("Sending messsage: %s\n", pcEventName );

		// Send the message
		pobParent->GetParent()->GetMessageHandler()->Receive( CMessageHandler::Make(pobParent->GetParent(), pcEventName, (int)eState, (int)m_ePlayerState) );

		// Save the player state
		m_ePlayerState = eState;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAIForwardPlayerCombatStateChanges::Update
*
*	DESCRIPTION		
*a
*	OWNER			GavB, 20/04/05
*
***************************************************************************************************/

void CAITask_KeyboardCapture::Update( float fTimeChange, CAIComponent* pobParent )
{
#ifdef PLATFORM_PC

	UNUSED(fTimeChange);
	CInputKeyboard& obKeyb = CInputHardware::Get().GetKeyboard();

	for( int iKey = 0; iKey < 256; ++iKey )
	{
		if( obKeyb.IsKeyPressed( (KEY_CODE) iKey ) || obKeyb.IsKeyPressed( (KEY_CODE) iKey, KEYM_SHIFT ) )
		{
			char acKey[2] = {obKeyb.KeyCodeToAscii( (KEY_CODE) iKey ) };
			pobParent->GetParent()->GetMessageHandler()->Receive( CMessageHandler::Make(pobParent->GetParent(), "OnKeyPress", (const char*)acKey) );
		}
		else if( obKeyb.IsKeyReleased( (KEY_CODE) iKey ) || obKeyb.IsKeyReleased( (KEY_CODE) iKey, KEYM_SHIFT ) )
		{
			char acKey[2] = {obKeyb.KeyCodeToAscii( (KEY_CODE) iKey ) };
			pobParent->GetParent()->GetMessageHandler()->Receive( CMessageHandler::Make(pobParent->GetParent(), "OnKeyRelease", (const char*)acKey) );
		}
	}

#elif PLATFORM_PS3

	ntError_p( 0, ("Not implemented on playstation 3 yet!") );
	UNUSED( fTimeChange );
	UNUSED( pobParent );

#endif
}

//------------------------------------------------------------------------------------------
//!
//!	CAIStateMachine::AddTask
//! Add a task to the state machine
//!
//------------------------------------------------------------------------------------------

void CAIStateMachine::AddTask( const char* pcName, NinjaLua::LuaObject obArgs )
{
	CAITask* pTask = 0;

	// 
	if( strcmp( pcName, "KeyboardCapture") == 0 )
	{
		pTask = NT_NEW_CHUNK( Mem::MC_AI ) CAITask_KeyboardCapture;
	}
	else if( strcmp( pcName, "FormationUpdate") == 0 )
	{
		pTask = NT_NEW_CHUNK( Mem::MC_AI ) CAIUpdateFormation;
	}
	else if( strcmp( pcName, "CombatUpdate") == 0 )
	{
		CAIUpdateAttackPosition* pobNewTask = NT_NEW_CHUNK( Mem::MC_AI ) CAIUpdateAttackPosition;
		pobNewTask->SetTarget( obArgs.GetOpt<CEntity*>( "Target", CEntityManager::Get().GetPlayer() ) );
		pobNewTask->SetUpdateMovement( obArgs.GetOpt<bool>( "Movement", true ) );
		pTask = pobNewTask;
	}
	else if( strcmp( pcName, "MoveToPoint") == 0 )
	{
		CAIWalkToPointTask* pobNewTask = NT_NEW_CHUNK( Mem::MC_AI ) CAIWalkToPointTask;
		pobNewTask->SetDestPoint( CLuaHelper::PointFromTable(obArgs.Get<NinjaLua::LuaObject>("Point")) );
		pobNewTask->SetMagnitude( obArgs.GetOpt<float>( "Speed", 0.5f ) );
		pTask = pobNewTask;
	}
	else
	{
		ntPrintf("Task could not be added (%s)\n", pcName );
	}

	// If created, add the new task
	if( pTask )
	{
		pTask->SetName( pcName );
		m_obTaskList.push_back( pTask );
	}
}
