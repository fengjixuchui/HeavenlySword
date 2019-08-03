//------------------------------------------------------------------------------------------
//!
//!	\file aibehaviour_formation.h
//!
//------------------------------------------------------------------------------------------

#ifndef AIBEHAVIOUR_FORMATION_INC
#define AIBEHAVIOUR_FORMATION_INC

/////////////////////////////////////
// Includes required for inheritance 
/////////////////////////////////////
#include "ai/aistatemachine.h"

class AIFormation;
class AIFormationSlot;
class AIFormationSlot_Circle;

class AIBehaviour_Formation : public CAIStateMachine
{
public:

	AIBehaviour_Formation(AI* pobEnt);
	~AIBehaviour_Formation();

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::FORMATION_DARIO;}

// State Enumeration
private:
	enum STATES
	{
		STATE_INITIALISE,
		STATE_FORMATION_MOVE_COMMAND,
		STATE_FORMATION_MOVING_TO_POINT,
		STATE_FORMATION_POINT_REACHED,
		STATE_FORMATION_ATTACKING,
		STATE_FORMATION_PLAY_INFO_ANIM,
		STATE_WAIT_FOR_CS_STANDARD,
	};

// State Functions
private:
	virtual bool States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

// Members
private:
	AIFormation*			m_pFormation;
	AIFormationSlot*		m_pSlot;
	unsigned int			m_ePreviousState;
	uint32_t				m_uiCtrlID;
	float					m_fTimer;
	float					m_fSlotTolerance;
	bool					m_bBeenInFormation;
};

#endif //AIBEHAVIOUR_FORMATION_INC
