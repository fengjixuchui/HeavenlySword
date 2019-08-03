
/***************************************************************************************************
*
*	CLASS			CAIInvestigate
*
*	DESCRIPTION		Investigate behaviour FSM
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _AI_INVESTIGATE_H
#define _AI_INVESTIGATE_H

#include "ai/aistatemachine.h"

class CAIInvestigate : public CAIStateMachine
{
public:

	CAIInvestigate( AI* pobEnt ) : CAIStateMachine(pobEnt), m_bFromChase( false ) {};
	~CAIInvestigate( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::INVESTIGATE;}

	void SetFromChase( const bool bFromChase )	{ m_bFromChase = bFromChase; };

private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_ALERTING,
		STATE_ALERTED,
		STATE_APPROACH,
		STATE_LOOK,
		STATE_SHRUG,
		STATE_ENEMYSEEN,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange  );
	void			GenerateSearchPos( void );
	bool			AtDest( void ) const;

	float			m_fTimer;			// measures time in a given state
	unsigned int	m_iSearchTimer;		// measures total time spent searching for the player
	CPoint			m_obDest;
	bool			m_bCheckPlayerPos;
	bool			m_bFromChase;

};

#endif // _AI_INVESTIGATE_H
