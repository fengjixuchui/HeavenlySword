/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "Physics/world.h"

#include "ai/aistatemachine.h"
#include "game/aicomponent.h"
#include "input/inputhardware.h"
#include "core/visualdebugger.h"
#include "game/entityinfo.h"

// Statics
bool CAIStateMachine::m_bRenderDebugInfo = false;

/***************************************************************************************************
* Start exposing the element to Lua
***************************************************************************************************/
LUA_EXPOSED_START(CAIStateMachine)
	LUA_EXPOSED_METHOD(AddTask,		AddTask, "", "", "") 
LUA_EXPOSED_END(CAIStateMachine)


/***************************************************************************************************
*
*	FUNCTION		CAIStateMachine constructor
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAIStateMachine::CAIStateMachine( AI* pobEnt ) :
	m_pobEnt( pobEnt ),
	m_pobAIComp( 0 ),
	m_pMov( 0 ),
	m_UpdateFlags( 0 ),
	m_bActive( false ),
	m_uiCurrentState( 0 ),
	m_uiNextState( false ),
	m_bStateChange( false ),
	m_uiReturnToState ( 0 )
{
	ATTACH_LUA_INTERFACE(CAIStateMachine);
	m_pobAIComp = pobEnt->GetAIComponent();
	ntAssert( m_pobAIComp );
	m_pMov = m_pobAIComp->GetCAIMovement();
}


/***************************************************************************************************
*
*	FUNCTION		CAIStateMachine::~CAIStateMachine
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAIStateMachine::~CAIStateMachine( void )
{
	// Remove all the tasks. 
	while( !m_obTaskList.empty() )
	{
		CAITask* pDelete = m_obTaskList.front();
		m_obTaskList.pop_front();
		NT_DELETE_CHUNK( Mem::MC_AI, pDelete );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAIStateMachine::Initialize
*
*	DESCRIPTION		Performs initialisation that can't be done from within the constructor. 
*
*	NOTES			The event EVENT_ENTER is processed by the state machine, but because
*					CAIStateMachine is an abstract base class, the derived class will not have been
*					constructed when the base constructor is called. Hence, the event cannot be
*					processed until after the the class instance is constructed.
*
***************************************************************************************************/

void
CAIStateMachine::Initialize( void )
{
	Process( EVENT_ENTER, 0.0f );
}


/***************************************************************************************************
*
*	FUNCTION		CAIStateMachine::Update
*
*	DESCRIPTION		The per frame state machine update function
*
***************************************************************************************************/

void CAIStateMachine::Update( const float fTimeChange )
{
	// Sanity check
	ntAssert( ShouldUpdate( GetUpdateFlags() ) );

	/*
	// Update the tasks associated with the state
	for( AITaskList::iterator obIt = m_obTaskList.begin(); obIt != m_obTaskList.end(); ++obIt )
	{
		(*obIt)->Update( fTimeChange, m_pobAIComp );
	}
	*/

	Process( EVENT_UPDATE, fTimeChange );
}


//------------------------------------------------------------------------------------------
//!  protected constant  EntityPaused
//!
//!  @return bool Returns true if the owner entity is paused
//!
//!  @author GavB @date 17/08/2006
//------------------------------------------------------------------------------------------
bool CAIStateMachine::EntityPaused() const
{
	return m_pobEnt->IsPaused();
}

//------------------------------------------------------------------------------------------
//!  protected constant  EntityDead
//!
//!  @return bool true if the entity is dead
//!
//!  @author GavB @date 17/08/2006
//------------------------------------------------------------------------------------------
bool CAIStateMachine::EntityDead() const
{
	return m_pobEnt->ToCharacter()->IsDead();
}

//------------------------------------------------------------------------------------------
//!  protected constant  EntityAIDisabled
//!
//!  @return bool true if the entity should not update any AI code
//!
//!  @author GavB @date 17/08/2006
//------------------------------------------------------------------------------------------
bool CAIStateMachine::EntityAIDisabled() const
{
	return (m_pobEnt->GetAIComponent() && m_pobEnt->GetAIComponent()->GetDisabled());
}

//------------------------------------------------------------------------------------------
//!  protected constant  EntityAIRecovering
//!
//!  @return bool true if the entity should not update any AI code
//!
//!  @author GavB @date 17/08/2006
//------------------------------------------------------------------------------------------
bool CAIStateMachine::EntityAIRecovering() const
{
	return (m_pobEnt->GetAIComponent() && m_pobEnt->GetAIComponent()->IsRecovering() );
}


//------------------------------------------------------------------------------------------
//!  public constant  ShouldUpdate
//!
//!  @return bool true if the update is ok to be called. 
//!
//!  @author GavB @date 17/08/2006
//------------------------------------------------------------------------------------------
bool CAIStateMachine::ShouldUpdate( uint uiUpdateFlags ) const 
{
	return	(!EntityPaused()		|| (uiUpdateFlags & CAIStateMachine::FORCE_UPDATE_WHEN_PAUSED_F) ) && 
			(!EntityDead()			|| (uiUpdateFlags & CAIStateMachine::FORCE_UPDATE_WHEN_DEAD_F) ) && 
			(!EntityAIRecovering()	|| (uiUpdateFlags & CAIStateMachine::FORCE_UPDATE_WHEN_COMBAT_RECOVERING_F) ) && 
			(!EntityAIDisabled()	|| (uiUpdateFlags & CAIStateMachine::FORCE_UPDATE_WHEN_AI_DISABLED_F) );
}

/***************************************************************************************************
*
*	FUNCTION		CAIStateMachine::Process
*
*	DESCRIPTION		Called when an event occurs (enter state, exit state, update state). The entry
*					point from which the actual state machine code is called
*
***************************************************************************************************/

void
CAIStateMachine::Process( STATE_MACHINE_EVENT event, const float fTimeChange )
{
	//when we've got the messaging system in, this may be how we want things to work
#if 0
	if( States( event, m_uiCurrentState ) == false )
	{	// Current state didn't handle msg, try Global state (-1)
		States( event, -1 );
	}
#else
	// but until then, we send the message to the global state first, and then to the current state
	// if global doesn't trigger a state change

	// are we about to change state anyway?
	bool	bGoingToChange = m_bStateChange;

	// set state change to false so that we can tell if the global state triggers a change 
	m_bStateChange = false;

	States( event, -1, fTimeChange );

	if( !m_bStateChange )
	{
		States( event, m_uiCurrentState, fTimeChange );
	}

	// now, if we were going to change state when we entered the function, set the flag back again
	if (bGoingToChange)
	{
		m_bStateChange = true;
	}
#endif

	// if we get more than 50 state changes in one frame then there's something going wrong
	int safetyCount = 50;
	while( m_bStateChange && (--safetyCount >= 0) )
	{
		ntAssert( safetyCount > 0 && "CAIStateMachine::Process - States are flip-flopping in an infinite loop." );

		m_bStateChange = false;

		// let the last state tidy up after itself
		States( EVENT_EXIT, m_uiCurrentState, fTimeChange );

		// set the new state
		m_uiCurrentState = m_uiNextState;

		// initialise the new state
		States( EVENT_ENTER, m_uiCurrentState, fTimeChange );
	}

}

/***************************************************************************************************
*
*	FUNCTION		CAIStateMachine::SetState
*
*	DESCRIPTION		
*
***************************************************************************************************/

void
CAIStateMachine::SetState( unsigned int newState )
{
	m_bStateChange = true;
	m_uiNextState = newState;
}




//------------------------------------------------------------------------------------------
//!
//!	CAIStateMachine::RenderDebugInfo
//! Display some debug information
//!
//------------------------------------------------------------------------------------------
void CAIStateMachine::RenderDebugInfo(const char* pcInfo) const
{
#ifndef _GOLD_MASTER
	// Set a debug state name
	m_BehaviourState = pcInfo;

	if(!m_bRenderDebugInfo)
		return;

	CPoint pt3D = m_pobEnt->GetPosition();
	pt3D.Y() += 2.0f;

	g_VisualDebug->Printf3D(pt3D, DC_WHITE, 0, pcInfo);
#endif
}

