#ifndef _AIBEHAVIOURMANAGER_H
#define _AIBEHAVIOURMANAGER_H

// forward declarations
class CAIComponent;
class CAIStateMachine;
class CAIComponent;
class CAIComponent;
// Nasty little test macro
#define AI_BEHAVIOUR_SEND_MSG( MSG ) \
	if( m_pobAIComp->GetBehaviourManager() )	m_pobAIComp->GetBehaviourManager()->SendMsg( MSG ); \
	else										m_pobAIComp->GetParent()->GetMessageHandler()->Receive( CMessageHandler::Make(m_pobAIComp->GetParent(), #MSG ) );

#include "ai/aibehaviourcontroller.h"
#include "game/luaglobal.h"

/***************************************************************************************************
*
*	CLASS			CAIBehaviourManager
*
*	DESCRIPTION		AI behaviour management using a push down automaton
*
*	NOTES			Manages a set of state machines as a PDA, so when you transition to a new state,
*					you can maintain the old state		
*
***************************************************************************************************/

class CAIBehaviourManager
{
public:

	CAIBehaviourManager( CAIComponent& obParent, AIBehaviourController* pobController );
	CAIBehaviourManager(CAIComponent& parent);
	~CAIBehaviourManager( void );

	// accessors
	CAIComponent*	GetParent( void )	{ return m_pobParent; }

	// stack functions
	void	Push( CAIStateMachine& );
	void	Replace( CAIStateMachine& );
	bool	Pop( void );

	void	SetInitialState( CAIStateMachine* pobInitial )	{ ntAssert( pobInitial ); Push( *pobInitial ); }
	void	Update( const float fTimeChange );
	void	SendMsg( BEHAVIOUR_MESSAGE );
	void	SendMsgEx( const char* );

	// Return the current state
	CAIStateMachine* GetCurrentState(void) const { return m_iStackDepth < 0 ? 0 : m_aStateMachineStack[ m_iStackDepth ]; }

	// enumerated type for the various behaviours, allowing object factory style behaviour creation
	enum eBEHAVIOUR
	{
		NONE = -1,
//        IDLE = 0,
		IDLE_DARIO = 0,
	//--	PATROL,
		PATROL_DARIO,
		GOTONODE_DARIO,
		FOLLOWPATHCOVER_DARIO,
		RANGEDYNAMICCOVER_DARIO,
		GOTOLOCATORNODE_DARIO,
		GOTOENTITY_DARIO,
	//--	INVESTIGATE,
		INVESTIGATE_DARIO,
	//--	COVER,
	//--	CHASE,
		CHASEPLAYER_DARIO,
		INITIALREACTION_DARIO,
//		ATTACK,
		ATTACK_DARIO,
		COMBAT,
		FORMATION,
		FORMATION_DARIO,
		GETWEAPON,
		LOCKON,
		LUA,
//--		WALKTOPOINT,
//--		RANDOMWALK,
// --		FOLLOW,
		FOLLOWLEADER_DARIO,
//--		WANDER,
//--		WALKTOLOCATOR,
//--		PLAYANIM,
		PLAYANIM_DARIO,
//--		USEOBJECT,
		USEOBJECT_DARIO,
//--		FACEENTITY,
		FACEENTITY_DARIO,
		WHACKAMOLE_DARIO,
		GOAROUNDVOLUMES_DARIO,
// --		RANGED,
//		FINDCOVER,
//		OPENFIRE,
//		HOLDFIRE,
//		BALLISTA,
	};

	eBEHAVIOUR	CurrentBehaviourType();

	// static functions for behaviour creation by enum and string (mainly for scripting usage)
	static CAIStateMachine*	CreateBehaviour( eBEHAVIOUR, CAIComponent*, class AI* = 0 );
	static CAIStateMachine*	CreateBehaviour( const char*, CAIComponent*, class AI* = 0 );

// Helper Functions
protected:
	void ApplyLuaTable(NinjaLua::LuaObject& table);

private:

	// the stack containing the state machines
	// XXX: replace with Stack type?
	CAIStateMachine*		m_aStateMachineStack[32];
	int						m_iStackDepth;
	CAIStateMachine*		m_pobToDelete;

	// parent component pointer, for introspective queries
	CAIComponent*			m_pobParent;

	// behaviour controller
	AIBehaviourController*	m_pobController;
	NinjaLua::LuaObject      m_obLuaController;

};

#endif
