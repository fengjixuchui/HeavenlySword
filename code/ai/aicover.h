/***************************************************************************************************
*
*	CLASS			CAICover
*
*	DESCRIPTION		"Take Cover!" behaviour FSM
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _AI_COVER_H
#define _AI_COVER_H

#include "ai/aistatemachine.h"

class CAICover : public CAIStateMachine
{
public:

	CAICover( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~CAICover( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::COVER;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_ALERT,
		STATE_RUN,
		STATE_HIDE,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	bool			GenerateCoverPoint();
	bool			InCover() const;

	float			m_fTimer;
	CPoint			m_obCoverPos;

};

#endif // _AI_COVER_H
