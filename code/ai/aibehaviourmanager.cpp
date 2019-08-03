//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aibehaviourmanager.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------

#include "ai/aibehaviourmanager.h"
#include "ai/aistatemachine.h"

// -- #include "ai/aiidle.h"
// -- #include "ai/aipatrol.h"
// -- #include "ai/aiinvestigate.h"
// -- #include "ai/aichase.h"
// -- #include "ai/aicover.h"
#include "ai/aibehaviour_patrol.h"			// Dario
#include "ai/aibehaviour_chase.h"			// Dario
#include "ai/aibehaviour_idle.h"			// Dario
#include "ai/aibehaviour_gotonode.h"		// Dario
#include "ai/aibehaviour_attacking.h"		// Dario
#include "ai/aibehaviour_followleader.h"	// Dario
#include "ai/aibehaviour_playanim.h"		// Dario
#include "ai/aibehaviour_faceentity.h"		// Dario
#include "ai/aibehaviour_gotolocatornode.h" // Dario
#include "ai/aibehaviour_gotoentity.h"		// Dario
#include "ai/aibehaviour_useobject.h"		// Dario
#include "ai/aibehaviour_formation.h"		// Dario
#include "ai/aibehaviour_investigate.h"		// Dario
#include "ai/aibehaviour_followpathcover.h"	// Dario
#include "ai/aibehaviour_rangedynamiccover.h"// Dario
#include "ai/aibehaviour_initialreaction.h"	// Dario
#include "ai/aibehaviour_whackamole.h"		// Dario
#include "ai/aibehaviour_goaroundvolumes.h" // Dario
//#include "ai/aibehaviour_attack.h"
#include "ai/aibehaviour_combat.h"
#include "ai/aibehaviour_lockon.h"
#include "ai/aibehaviour_lua.h"
// -- #include "ai/airandomwalk.h"
// -- #include "ai/aiwalktopoint.h"
// -- #include "ai/aifollow.h"
// -- #include "ai/aiwander.h"
// -- #include "ai/aiwalktolocator.h"
// -- #include "ai/aiplayanim.h"
// -- #include "ai/aiuseobject.h"
// -- #include "ai/aifaceentity.h"
// -- #include "ai/airanged.h"

#include "game/aicomponent.h"

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIBehaviourManager::CAIBehaviourManager
//! Construction
//!                                                                                         
//------------------------------------------------------------------------------------------
CAIBehaviourManager::CAIBehaviourManager( CAIComponent&	obParent, AIBehaviourController* pobController ) :
	m_iStackDepth( -1 ),
	m_pobToDelete( NULL ),
	m_pobParent( &obParent ),
	m_pobController( pobController )
{
	ntAssert(m_pobController);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIBehaviourManager::CAIBehaviourManager
//! Construction - Lua Version
//!                                                                                         
//------------------------------------------------------------------------------------------
CAIBehaviourManager::CAIBehaviourManager(CAIComponent& parent)
	: m_iStackDepth(-1),
	m_pobToDelete(0),
	m_pobParent(&parent),
	m_pobController(0)
{
	m_obLuaController = CLuaGlobal::Get().State().GetGlobals()[CHashedString(parent.GetDefinition()->m_obAIBehaviourSet)];
	ntAssert(m_obLuaController.IsTable());
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIBehaviourManager::~CAIBehaviourManager
//! Destruction
//!                                                                                         
//------------------------------------------------------------------------------------------
CAIBehaviourManager::~CAIBehaviourManager()
{
	NT_DELETE_CHUNK( Mem::MC_AI, m_pobToDelete );
	m_pobToDelete = NULL;

	while(m_iStackDepth >= 0)
	{
		m_pobToDelete = m_aStateMachineStack[m_iStackDepth];
		m_aStateMachineStack[m_iStackDepth--] = NULL;
		NT_DELETE_CHUNK( Mem::MC_AI, m_pobToDelete );
		m_pobToDelete = NULL;
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIBehaviourManager::Update
//! Update the Behaviour Manager
//!                                                                                         
//------------------------------------------------------------------------------------------
void CAIBehaviourManager::Update( const float fTimeChange )
{
	// If the stack depth is less than 0, then the stack is not in use so just return now.
	if( m_iStackDepth < 0 )
		return;

	ntAssert( m_iStackDepth >= 0);
	ntAssert( m_aStateMachineStack[m_iStackDepth] );
	ntAssert( m_pobToDelete != m_aStateMachineStack[m_iStackDepth] );

	if (m_pobToDelete)
	{
		NT_DELETE_CHUNK( Mem::MC_AI, m_pobToDelete );
		m_pobToDelete = NULL;
	}

	// update the FSM on top of the stack
	m_aStateMachineStack[m_iStackDepth]->Update( fTimeChange );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIBehaviourManager::Update
//! Update the Behaviour Manager
//!                                                                                         
//------------------------------------------------------------------------------------------
void CAIBehaviourManager::SendMsg( BEHAVIOUR_MESSAGE eMessage )
{
	if(m_pobController)
		m_pobController->ProcessMessage( eMessage, this );
	else
	{
		NinjaLua::LuaObject handler = m_obLuaController[eMessage];

		if(handler.IsTable())
			ApplyLuaTable(handler);
		else if(handler.IsFunction())
		{
			NinjaLua::LuaFunctionRet<NinjaLua::LuaObject> fn(handler);
			NinjaLua::LuaObject table = fn();
			ApplyLuaTable( table );
		}
		else
			ntAssert(false);
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIBehaviourManager::SendMsgEx
//! Send a message to the Lua controller
//!                                                                                         
//------------------------------------------------------------------------------------------
void CAIBehaviourManager::SendMsgEx( const char* pcMsg )
{
	// If we're not in a lua behaviour - then just return (old and need changing)
	if( CurrentBehaviourType() != LUA )
		return;

	if(m_pobController)
	{
		m_pobController->ProcessMessage( pcMsg );
	}

}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIBehaviourManager::CurrentBehaviourType
//! Return the behaviour currently at the top of the stack.
//!                                                                                         
//------------------------------------------------------------------------------------------
CAIBehaviourManager::eBEHAVIOUR	CAIBehaviourManager::CurrentBehaviourType()
{
	return m_aStateMachineStack[m_iStackDepth]->GetType();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIBehaviourManager::Push
//! Push a new behaviour onto the top of the stack.
//!                                                                                         
//------------------------------------------------------------------------------------------
void CAIBehaviourManager::Push( CAIStateMachine& obNewState)
{
	// Call Exit for the currently active state.
	if(m_iStackDepth >= 0)
		m_aStateMachineStack[m_iStackDepth]->States(EVENT_EXIT, m_aStateMachineStack[m_iStackDepth]->GetCurrentState(), 0.0f);

	// Push on the new state.
	m_aStateMachineStack[++m_iStackDepth] = &obNewState;
	ntPrintf( "pushed new behaviour to stack depth %d\n", m_iStackDepth );
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIBehaviourManager::Replace
//! Replace the behaviour currently at the top of a stack.
//!                                                                                         
//------------------------------------------------------------------------------------------
void CAIBehaviourManager::Replace( CAIStateMachine& obNewState)
{
	Pop();
	Push( obNewState );
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIBehaviourManager::Pop
//! Pop a behaviour off the top of the stack.
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CAIBehaviourManager::Pop( void )
{
	// If the 
	if( m_iStackDepth < 0 )
		return false;

	ntAssert( m_iStackDepth >= 0 );

	// Call Exit for the currently active state.
	m_aStateMachineStack[m_iStackDepth]->States(EVENT_EXIT, m_aStateMachineStack[m_iStackDepth]->GetCurrentState(), 0.0f);

	m_pobToDelete = m_aStateMachineStack[m_iStackDepth];
	m_aStateMachineStack[m_iStackDepth--] = NULL;
	ntPrintf( "popped behaviour, stack depth now %d\n", m_iStackDepth );

	// Call Enter for the newly active state.
	if(m_iStackDepth >= 0)
		m_aStateMachineStack[m_iStackDepth]->States(EVENT_ENTER, m_aStateMachineStack[m_iStackDepth]->GetCurrentState(), 0.0f);

	return m_iStackDepth >= 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CAIBehaviourManager::CreateBehaviour
//! Class Factory Lite.
//!                                                                                         
//------------------------------------------------------------------------------------------
CAIStateMachine* CAIBehaviourManager::CreateBehaviour( eBEHAVIOUR eBehaviour, CAIComponent*	pobComponent, AI* pTarg )
{
	ntAssert( pobComponent );
	AI* pobEnt = pobComponent->GetParent();

	CAIStateMachine*	pobDefaultBehaviour( NULL );

	switch( eBehaviour )
	{
	//case IDLE:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIIdle( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "IDLE" );
	//			break;
	//	  }
  	case IDLE_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIIdleBehaviour( pobEnt );
				NAME_BEHAVIOUR( pobDefaultBehaviour, "IDLE_DARIO" );
				break;
		  }
	//case PATROL:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIPatrol( pobEnt );              
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "PATROL" );
	//			break;
	//	  }
	case PATROL_DARIO:
		{
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIPatrolBehaviour( pobEnt );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "PATROL_DARIO" );
				break;
		}
	//case INVESTIGATE:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIInvestigate( pobEnt );              
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "INVESTIGATE" );
	//			break;
	//	  }
	case INVESTIGATE_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIInvestigateBehaviour( pobEnt );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "INVESTIGATE_DARIO" );
				break;
		  }
	case FOLLOWPATHCOVER_DARIO:
		{
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIFollowPathCoverBehaviour( pobEnt );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "FOLLOWPATHCOVER_DARIO" );
				break;
		}
	case RANGEDYNAMICCOVER_DARIO:
		{
			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIRangeDynamicCoverBehaviour( pobEnt );              
			  break;
		} 
    case GOTONODE_DARIO:
		{
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIGoToNodeBehaviour( pobEnt );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "GOTONODE_DARIO" );
				break;
		}
	case GOTOENTITY_DARIO:
		{
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIGoToEntityBehaviour( pobEnt );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "GOTOENTITY_DARIO" );
				break;
		}
	case GOTOLOCATORNODE_DARIO:
		{
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIGoToLocatorNodeBehaviour( pobEnt );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "GOTOLOCATORNODE_DARIO" );
				break;
		}
	//case COVER:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAICover( pobEnt );              
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "COVER" );
	//			break;
	//	  }
	//case CHASE:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIChase( pobEnt );              
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "CHASE" );
	//			break;
	//	  }
	case CHASEPLAYER_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIChaseBehaviour( pobEnt );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "CHASEPLAYER_DARIO" );
				break;
		  }
	//case ATTACK:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIBehaviour_Attack( pobEnt );              
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "ATTACK" );
	//			break;
	//	  }
  	case ATTACK_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIAttackBehaviour( pobEnt );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "ATTACK_DARIO" );
				break;
		  }
	case FORMATION_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIBehaviour_Formation( pobEnt );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "FORMATION_DARIO" );
				break;
		  }
    case FOLLOWLEADER_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIFollowLeaderBehaviour( pobEnt );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "FOLLOWLEADER_DARIO" );
				break;
		  }
	case COMBAT:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIBehaviour_Combat( pobEnt );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "COMBAT" );
				break;
		  }
	case LOCKON:
		  {
				ntAssert(pTarg);
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIBehaviour_LockOn( pobEnt, *pTarg );              
				NAME_BEHAVIOUR( pobDefaultBehaviour, "LOCKON" );
				break;
		  }
	case LUA:
		{
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIBehaviour_Lua( pobEnt );
				NAME_BEHAVIOUR( pobDefaultBehaviour, "LUA" );
				break;
		}
	//case RANDOMWALK:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIRandomWalk( pobEnt );              
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "RANDOMWALK" );
	//			break;
	//	  }
	//case FOLLOW:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIFollow( pobEnt );              
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "FOLLOW" );
	//			break;
	//	  }
	//case WALKTOPOINT:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIWalkToPoint( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "WALKTOPOINT" );
	//			break;
	//	  }
	//case WANDER:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIWander( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "WANDER" );
	//			break;
	//	  }
	//case WALKTOLOCATOR:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIWalkToLocator( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "WALKTOLOCATOR" );
	//			break;
	//	  }
	//case PLAYANIM:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIPlayAnim( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "PLAYANIM" );
	//			break;
	//	  }
	case PLAYANIM_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIPlayAnimBehaviour( pobEnt );
				NAME_BEHAVIOUR( pobDefaultBehaviour, "PLAYANIM_DARIO" );
				break;
		  }
	//case USEOBJECT:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIUseObject( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "USEOBJECT" );
	//			break;
	//	  }
	case USEOBJECT_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIUseObjectBehaviour( pobEnt );
				NAME_BEHAVIOUR( pobDefaultBehaviour, "USEOBJECT_DARIO" );
				break;
		  }
	//case FACEENTITY:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIFaceEntity( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "FACEENTITY" );
	//			break;
	//	  }
	case FACEENTITY_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIFaceEntityBehaviour( pobEnt );
				NAME_BEHAVIOUR( pobDefaultBehaviour, "FACEENTITY_DARIO" );
				break;
		  }
  	case INITIALREACTION_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIInitialReactionBehaviour( pobEnt );
				NAME_BEHAVIOUR( pobDefaultBehaviour, "INITIALREACTION_DARIO" );
				break;
		  }
    case WHACKAMOLE_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIWhackAMoleBehaviour( pobEnt );
				NAME_BEHAVIOUR( pobDefaultBehaviour, "WHACKAMOLE_DARIO" );
				break;
		  }
	case GOAROUNDVOLUMES_DARIO:
		  {
				pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) CAIGoAroundVolumesBehaviour( pobEnt );
				NAME_BEHAVIOUR( pobDefaultBehaviour, "GOAROUNDVOLUMES_DARIO" );
				break;
		  }
	//case RANGED:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIRanged( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "RANGED" );
	//			break;
	//	  }
	//case FINDCOVER:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIFindCover( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "FINDCOVER" );
	//			break;
	//	  }		  
	//case OPENFIRE:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIOpenFire( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "OPENFIRE" );
	//			break;
	//	  }		  
	//case HOLDFIRE:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIHoldFire( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "HOLDFIRE" );
	//			break;
	//	  }		  
	//case BALLISTA:
	//	  {
	//			pobDefaultBehaviour = NT_NEW_CHUNK( Mem::MC_AI ) AIBallista( pobEnt );
	//			NAME_BEHAVIOUR( pobDefaultBehaviour, "BALLISTA" );
	//			break;
	//	  }

	default:
		  {
              ntAssert( !"attempting to create undefined behaviour in AIBehaviourManager" );
		  }
		  
	}

	ntAssert(pobDefaultBehaviour);
	pobDefaultBehaviour->Initialize();

	return pobDefaultBehaviour;
}

typedef struct	BehaviourStringTuple_s
{
	char*							strName;
	CAIBehaviourManager::eBEHAVIOUR	eBehaviour;

} BehaviourStringTuple;

static BehaviourStringTuple	aBehaviourStringTable[] =
{
//	{	"Idle",				CAIBehaviourManager::IDLE	},
	{	"IdleDario",		CAIBehaviourManager::IDLE_DARIO	},
//	{	"Patrol",			CAIBehaviourManager::PATROL	},
	{	"PatrolDario",		CAIBehaviourManager::PATROL_DARIO	},
	{	"FollowPathCoverDario",		CAIBehaviourManager::FOLLOWPATHCOVER_DARIO	},
	{	"RangeDynCoverDario",		CAIBehaviourManager::RANGEDYNAMICCOVER_DARIO},
	{	"GoToNodeDario",	CAIBehaviourManager::GOTONODE_DARIO	},
	{	"GoToEntityDario",	CAIBehaviourManager::GOTOENTITY_DARIO	},
	{	"GoToLocatorNodeDario",	CAIBehaviourManager::GOTOLOCATORNODE_DARIO	},
	{	"FollowLeaderDario",CAIBehaviourManager::FOLLOWLEADER_DARIO	},
//	{	"Investigate",		CAIBehaviourManager::INVESTIGATE	},
	{	"InvestigateDario",	CAIBehaviourManager::INVESTIGATE_DARIO },
//	{	"Cover",			CAIBehaviourManager::COVER	},
//	{	"Chase",			CAIBehaviourManager::CHASE	},
	{	"ChaseDario",		CAIBehaviourManager::CHASEPLAYER_DARIO },
//	{	"Attack",			CAIBehaviourManager::ATTACK	},
	{	"AttackDario",		CAIBehaviourManager::ATTACK_DARIO },
	{	"Combat",			CAIBehaviourManager::COMBAT	},
	{	"Lockon",			CAIBehaviourManager::LOCKON	},
	{	"Lua",				CAIBehaviourManager::LUA	},
	{	"Formation",		CAIBehaviourManager::FORMATION	},
	{	"FormationDario",	CAIBehaviourManager::FORMATION_DARIO },
//	{	"Wander",			CAIBehaviourManager::WANDER	},
//	{	"WalkToLocator",	CAIBehaviourManager::WALKTOLOCATOR	},
//	{	"PlayAnim",			CAIBehaviourManager::PLAYANIM	},
	{	"PlayAnimDario",	CAIBehaviourManager::PLAYANIM_DARIO	},
//	{	"UseObject",		CAIBehaviourManager::USEOBJECT	},
	{	"UseObjectDario",	CAIBehaviourManager::USEOBJECT_DARIO },
//	{	"FaceEntity",		CAIBehaviourManager::FACEENTITY	},
	{	"FaceEntityDario",	CAIBehaviourManager::FACEENTITY_DARIO },
	{	"InitialReactionDario", CAIBehaviourManager::INITIALREACTION_DARIO },
	{	"WhackAMoleDario", CAIBehaviourManager::WHACKAMOLE_DARIO },
	{	"GoAroundVolumesDario", CAIBehaviourManager::GOAROUNDVOLUMES_DARIO },
//	{	"Ranged",			CAIBehaviourManager::RANGED	},
//	{	"FindCover",		CAIBehaviourManager::FINDCOVER	},
//	{	"OpenFire",			CAIBehaviourManager::OPENFIRE	},
//	{	"HoldFire",			CAIBehaviourManager::HOLDFIRE	},
//	{	"Ballista",			CAIBehaviourManager::BALLISTA	},

	// this must be the last entry in the table
	{	"",				CAIBehaviourManager::NONE	}
};

CAIStateMachine* CAIBehaviourManager::CreateBehaviour( const char* strBehaviour, CAIComponent*	pobComponent, AI* pTarg )
{
	int i = 0;

	while (aBehaviourStringTable[i].eBehaviour != CAIBehaviourManager::NONE)
	{
		if (stricmp( aBehaviourStringTable[i].strName, strBehaviour ) == 0)
		{
			return CreateBehaviour( aBehaviourStringTable[i].eBehaviour, pobComponent, pTarg );
		}
		++i;
	}
	return NULL;
}

// This method really sucks, please make it suck less. - JML
void CAIBehaviourManager::ApplyLuaTable(NinjaLua::LuaObject& table)
{
	for(int i = 1; table[i].IsTable(); i++)
	{
		NinjaLua::LuaObject ob = table[i];

		// Get the action
		ntAssert(ob[1].IsInteger());
		enum ACTION {POP=1, PUSH=2, REPLACE=3};
		ACTION eAction = (ACTION)ob[1].GetInteger();

		if(eAction == POP)
		{
			Pop();
			continue;
		}

		// Get the required behaviour
		ntAssert(ob[2].IsInteger());
		CAIBehaviourManager::eBEHAVIOUR eBehaviour = (CAIBehaviourManager::eBEHAVIOUR)ob[2].GetInteger();

		// Change this so it works with tables rather than entity pointers...
		AI* pEnt;
		if(ob[3].IsLightUserData())
		{
			pEnt = (AI*)ob[3].GetLightUserData();
			if(!pEnt->IsAI())
				pEnt = 0;
		}
		else
			pEnt = 0;
		
		// Create the behaviour
		CAIStateMachine* pNewBehaviour = CreateBehaviour(eBehaviour, GetParent(), pEnt);

		// Push or replace it as necessary
		if(eAction == PUSH)
			Push(*pNewBehaviour);
		else // REPLACE
			Replace(*pNewBehaviour);
	}
}
