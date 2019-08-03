//------------------------------------------------------------------------------------------
//!
//!	\file aigroupmove.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIGROUPMOVE_H
#define _AIGROUPMOVE_H

#include "ai/aistatemachine.h"

class CAINavNodeAABB;

//------------------------------------------------------------------------------------------
//!
//!	AIGroupMove
//!
//------------------------------------------------------------------------------------------


class AIGroupMove : public CAIStateMachine
{
public:

	AIGroupMove( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIGroupMove( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::GETWEAPON;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_GETTARGET,
		STATE_GOTOTARGET,
		STATE_WAITFORGROUP,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	bool			InTargetNode() const;
	void			SelectTargetNode();
	void			SetupGroupPosition();
	bool			TooFarAhead() const;
	float			DistFromGroupCentre() const;

	float					m_fTimer;
	CPoint					m_obTargetNodeCentre;
	const CAINavNodeAABB*	m_pobTargetNode;
	int						m_iTripCycle;
	CPoint					m_obStartPoint;
};

#endif //_AIGETWEAPON_H
