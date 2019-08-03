//--------------------------------------------------
//!
//!	\file aipatrol.h
//!	Patrol behaviour.
//!
//--------------------------------------------------

#ifndef _AIPATROL_H_
#define _AIPATROL_H_

#include "ai/aistatemachine.h"
//--------------------------------------------------
//!
//!	Patrol behaviour class.
//!
//--------------------------------------------------

class CAIPatrol : public CAIStateMachine
{
public:

	CAIPatrol( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAIPatrol( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::PATROL;}


protected:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_WALK,
		STATE_IDLE,
		STATE_LOOK,
		STATE_SPOTTED,
		STATE_ALERT,
		STATE_ALERTED,
		STATE_SEENSOMETHING,
		STATE_RETURN,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	virtual void	Initialise( void );
	virtual void	WalkInitialise( void );
	virtual void	Walk( void );

	float			m_fTimer;
	CPoint			m_obDest;
	int				m_iPathNum;
	int				m_iStartNodeNum;
	int				m_iCurrentNodeNum;

};

#endif // _AI_PATROL_H
