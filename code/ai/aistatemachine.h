/***************************************************************************************************
*
*	DESCRIPTION		aistatemachine.h - Macro based FSM language and supporting class
*
*	NOTES			State machine macro language and associated class based on chapter 6.5 + 6.6 of 
*					AI Games Programming Wisdom. Portions Copyright (C) Steve Rabin, 2001 
*
***************************************************************************************************/

#ifndef _AI_STATEMACHINE_H
#define _AI_STATEMACHINE_H

#include "ai/aibehaviourmanager.h"
#include "ai/aitask.h"
#include "game/attacks.h"

#ifndef GAME_SHELLCONFIG_H
#include "game/shellconfig.h"
#endif

// forward declarations
class CAIMovement;
class CAIComponent;
struct lua_State;

// events that can occur within a state
enum STATE_MACHINE_EVENT
{
	EVENT_UPDATE,
	EVENT_ENTER,
	EVENT_EXIT
};

// State Printing Macro (Debug)

#define PRINT_STATE(x) if (g_ShellOptions->m_bBehaviourDebug) ntPrintf("Entity: [%s] enter into: " #x "\n", this->m_pobEnt->GetName().c_str());

//State Machine Language Macros (put these keywords in the file USERTYPE.DAT in the same directory as MSDEV.EXE)
#define BeginStateMachine		if( iState < 0 ) { if(0) {
#define EndStateMachine			return( true ); } } else { ntAssert(0); return( false ); }  return( false );
#define AIState(a)				return( true ); } } else if( a == iState ) { RenderDebugInfo(#a); if(0) {
#define OnEvent(a)				return( true ); } else if( a == eEvent ) {
#define OnEnter					OnEvent( EVENT_ENTER )
#define OnUpdate				OnEvent( EVENT_UPDATE )
#define OnExit					OnEvent( EVENT_EXIT )

#define NAME_BEHAVIOUR(a,b)		(a)->SetBehaviourName( b )

/***************************************************************************************************
*
*	CLASS			CAIStateMachine
*
*	DESCRIPTION		Finite state machine class
*
*	NOTES			
*
***************************************************************************************************/

class CAIStateMachine
{
public:

	enum UPDATE_FLAGS
	{
		FORCE_UPDATE_WHEN_AI_DISABLED_F			=	1 << 0,
		FORCE_UPDATE_WHEN_DEAD_F				=	1 << 1,
		FORCE_UPDATE_WHEN_PAUSED_F				=	1 << 2,
		FORCE_UPDATE_WHEN_COMBAT_RECOVERING_F	=	1 << 3,
	};

public:
	CAIStateMachine( AI* );
	virtual ~CAIStateMachine( void );

	HAS_LUA_INTERFACE()

	void	Initialize( void );
	void	Update( const float fTimeChange );
	void	SetState( unsigned int newState );
	bool	ShouldUpdate( uint ) const;

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() = 0;
	unsigned int GetCurrentState() const {return m_uiCurrentState;}

	// Debug Functions.
	void	EnableDebugInfo(bool b) {m_bRenderDebugInfo = b;}
	void	RenderDebugInfo(const char* pcInfo) const;

	// Add a task to the state machine
	void AddTask( const char* pcName, NinjaLua::LuaObject obArgs );

	// To avoid undesireable lose of mov. controllers
	bool IsSaveToPushAController ( void ) const { return (m_pobEnt->GetAttackComponent()->AI_Access_IsInCSStandard()); }

	// 
	void			SetReturnToState ( unsigned int uiRS )	{ m_uiReturnToState = uiRS; }
	unsigned int	GetReturnToState ( void	)				{ return m_uiReturnToState; }

	// Set the name of the behaviour (debug)
	void SetBehaviourName( const char* pcName ) { m_BehaviourName = pcName; }

	const ntstd::String& GetBehaviourName(void) const { return m_BehaviourName; } 
	const ntstd::String& GetBehaviourState(void) const { return m_BehaviourState; } 

	const CEntity* GetOwnerEntity(void) const { return m_pobEnt; }
	CAIComponent* GetOwnersAIComponent(void) const { return m_pobAIComp; }

	// Return flags for the behaviour 
	uint	GetUpdateFlags		() const		{ return m_UpdateFlags; }

	// Access the active state of the state machine
	bool	Active				() const		{ return m_bActive; }
	void	Active				(bool bValue)	{ m_bActive = bValue; }

protected:
	bool	EntityPaused() const;
	bool	EntityDead() const;
	bool	EntityAIDisabled() const;
	bool	EntityAIRecovering() const;


protected:
	AI*				m_pobEnt;
	CAIComponent*	m_pobAIComp;
	CAIMovement*	m_pMov;
	uint			m_UpdateFlags;
	bool			m_bActive;

	friend class CAIBehaviourManager;

private:

	unsigned int	m_uiCurrentState;
	unsigned int	m_uiNextState;
	bool			m_bStateChange;
	unsigned int	m_uiReturnToState;

	void			Process( STATE_MACHINE_EVENT eEvent, float fTimeChange );
	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange ) = 0;

	// Debugging.
	static bool		m_bRenderDebugInfo;

	// 
	ntstd::String			m_BehaviourName;
	mutable ntstd::String	m_BehaviourState;

	// Tasks assoicated with the state machine
	typedef ntstd::List<CAITask*, Mem::MC_AI> AITaskList;
	AITaskList			m_obTaskList;
};

LV_DECLARE_USERDATA(CAIStateMachine);


/***************************************************************************************************
*
*	CLASS			CAISpread
*
*	DESCRIPTION		"Spread Out!" behaviour FSM
*
*	NOTES			
*
***************************************************************************************************/

class CAISpread : public CAIStateMachine
{
public:

	CAISpread( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	virtual ~CAISpread( void ) {};


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_ALERT,
		STATE_RUN,
		STATE_WAIT,
	};

	virtual bool States( const STATE_MACHINE_EVENT eEvent, const int iState );

	unsigned int m_iTimer;

};



/***************************************************************************************************
*
*	CLASS			CAIAmbush
*
*	DESCRIPTION		Ambush behaviour FSM
*
*	NOTES			
*
***************************************************************************************************/

class CAIAmbush : public CAIStateMachine
{
public:

	CAIAmbush( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	virtual ~CAIAmbush( void ) {};


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_HIDDEN,
		STATE_HIDE,
		STATE_ALERT,
		STATE_SEQUENCE,
	};

	virtual bool States( const STATE_MACHINE_EVENT eEvent, const int iState );

	unsigned int m_iTimer;

};




#endif	// _AI_STATEMACHINE_H
