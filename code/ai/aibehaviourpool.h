#ifndef _AIBEHAVIOURPOOL_H
#define _AIBEHAVIOURPOOL_H

// forward declarations
class CAIComponent;
class CAIStateMachine;
class CAIComponent;
class CAIComponent;
class AIBehaviourController;

#include "game/luaglobal.h"


struct DeleteListEntry
{
	CAIStateMachine*	pState;
	int					delay;
};

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIBehaviourPool::AIBehaviourPool
//!                                                                                         
//! Looks after the C++ behaviour instances created by LUA
//!                                                                                         
//------------------------------------------------------------------------------------------


class AIBehaviourPool : public Singleton<AIBehaviourPool>
{
public:

	AIBehaviourPool();
	~AIBehaviourPool( void );

	HAS_LUA_INTERFACE()

	void Update( const float fTimeChange );
	void LevelUnload();

	CAIStateMachine*	CreateBehaviour( AI*, const char* pcName );
	void				DeleteBehaviour_LUA( CAIStateMachine* );

	// Return the current behaviour for the entity
	CAIStateMachine*	GetCurrentBehaviour( CEntity* ) const ;

	void SetActive( CAIStateMachine* pobState, bool bActive );

private:
	void				DeleteBehaviour( CAIStateMachine*, int );

	typedef ntstd::List<CAIStateMachine*, Mem::MC_AI> AIStateMachineList;
	typedef ntstd::List<DeleteListEntry*, Mem::MC_AI> DeleteListEntryList;
	// the list of state machines
	AIStateMachineList	m_obStateMachines;
	AIStateMachineList	m_obActiveStateMachines;
	//AIStateMachineList	m_obDeleteList;	
	DeleteListEntryList	m_obDeleteList;	
	bool				m_bLevelHasBeenUnloaded;
};

LV_DECLARE_USERDATA(AIBehaviourPool);

#endif
