/***************************************************************************************************
*
*	DESCRIPTION		Simple state machine for our AIs
*
*	NOTES
*
***************************************************************************************************/

#ifndef _AI_STATES_H
#define _AI_STATES_H

#include "ai/aidefines.h"
#include "editable/enumlist.h"
#include "game/aicomponent.h"

// Forward declarations
class CAIBehaviourManager;
class AIBehaviourController;
class CAIStateCMD;
class CPointSmoother;

/***************************************************************************************************
*
*	CLASS			AIStates
*
*	DESCRIPTION		Simple mock FSM for AI control
*
***************************************************************************************************/
class AIStates
{
public:
	enum eSTATE
	{
		STATE_START,

		// passive / no motion
		IDLE = STATE_START,	//[Script]

		// combat stuff
		ATTACK_SELECTION,
		ATTACK_PREPARATION,
		COUNTER_SELECTION,
		COUNTER_PREPARATION,
		ATTACKING,
		BLOCKING,

		DISABLED,
		INVALID,		
	};

	AIStates( CAIComponent* pobParent );
	~AIStates( void );

	void	Update( float fTimeChange );
	void	DebugRender( void ) const;

	eSTATE	GetCurrState( void ) const { return m_eCurrState; }

	void	FakeAlertMsgReceive( const CHashedString&, float, float) {;}

	void	SetFormationTargetPos( const CPoint& ) {;}

	CPoint	GetFormationTargetPos( void ) const { return CPoint(CONSTRUCT_CLEAR); }

	void	FirstFrameInit( void );

	void	SetDisabled( bool bDisabled )	{ m_bDisabled = bDisabled; }
	bool	GetDisabled( void ) const		{ return m_bDisabled; }

	bool	IsDebugSelected( void ) const			{ return (m_iCurrentID == m_iUserID); }

	// NB this is likely to expand to other stuff like distances and all that. Which is why we have a function.
	bool	IsReadyForFormationAttack( void ) const { return false; }

	// XXX: the action interface is directly embedded in this class, but ultimately it should:
	// a) be a new class
	// b) belong to CAIComponent rather than AIStates
	void				SetAction( const CAIState_ACTION eAction );
	void				SetActionStyle( const CAIState_ACTIONSTYLE eStyle );
	void				SetActionUsing( AI_ACTION_USING eUsing );
	void				SetActionDest( const CPoint& obDest );
	void				SetActionFacing(const CDirection& dirFacing);
	void				SetActionMoveSpeed(float fSpeed);
	CAIState_ACTION		GetAction( void ) const 					 			{ return m_eAction; }
	CAIState_ACTIONSTYLE GetActionStyle(void) const								{ return m_eActionStyle; }
	AI_ACTION_USING GetActionUsing(void) const							{ return m_eActionUsing; }
	const CPoint&		GetActionDest( void ) const					 			{ return m_obActionDest; }
	const CDirection&	GetActionFacing() const					     			{ntAssert(m_bActionFacingSet); return m_dirActionFacing;}


	CAIBehaviourManager* GetBehaviourManager() {return m_pobBehaviours;}

private:

	// updates VARIABLES ONLY for ALL states so we can evaluate them at any time..
	void	UpdatePersistantStateVars( float fTimeChange );

	// internal methods all use m_eCurrState
	void	InitialiseCurrState( void );
	void	UpdateCurrState( float fTimeChange );
	void	DebugRenderCurrState( void ) const;
	void	ShutdownCurrState( eSTATE eNextState );

	// is this state viable to move to?
	bool	CanBeTriggered( eSTATE eTestState );

	// how interesting is this state to move to? (0.0f->1.0f)
	eSTATE	GetMostInteresting( void );

	CAIComponent*		m_pobParent;
	eSTATE				m_eCurrState;
	eSTATE				m_eLastState;

	bool				m_bReadyToUse;
	bool				m_bStateInitialised;
	bool				m_bFirstStateFrame;
	float				m_fElapsedStateTime;
	mutable bool		m_bStateSpin;

	// CAIStateWorldInfo	m_obWorldInfo;

	// indiviuals state variables.
	//----------------------------------
	// USER
	static int		m_iAllocedIDs;
	static int		m_iCurrentID;
	int				m_iUserID;
	bool			m_bSlaved;
	bool			m_bStrafeSlaved;

	// DISABLED
	bool			m_bDisabled;

	// In order to maintain compatibility with attack selection mechanism,
	// embed the new state system in the old. This is, of course, an extremely
	// temporary measure, and ultimately the behaviour manager should replace
	// CAIStates altogether
	CAIBehaviourManager*		m_pobBehaviours;
	AIBehaviourController*		m_pobBehaviourController;

	// action and destination set by the state machine
	CAIState_ACTION		m_eAction;
	CAIState_ACTIONSTYLE m_eActionStyle;
	AI_ACTION_USING m_eActionUsing;
    CPoint				m_obActionDest;
	CPoint				m_obLastActionDest;
	bool				m_bActionDestSet;
	CDirection			m_dirActionFacing;
	float				m_fMovementSpeed;
	bool				m_bActionFacingSet;
	bool				m_bLastFramePos;
    CPoint				m_obLastFramePos;
	CPointSmoother*		m_pRepulsionSmoother;


	// in the absence of a look around anim, we fudge things using the basic
	// movement controller
	float	m_fLookAroundTimer;
	CPoint	m_obLookAroundDir;
};


#endif // _AI_STATES_H
