//--------------------------------------------------
//!
//!	\file aiwander.h
//!	Wander behaviour.
//!
//--------------------------------------------------

#ifndef _AIWANDER_H_
#define _AIWANDER_H_

#include "aipatrol.h"

//--------------------------------------------------
//!
//!	Wander behaviour class.
//!
//--------------------------------------------------

class AIWander : public CAIPatrol
{
public:

	AIWander( AI* pobEnt ) : CAIPatrol(pobEnt) {};
	~AIWander( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::WANDER;}


protected:

	virtual void	Initialise( void );
	virtual void	WalkInitialise( void );
	virtual void	Walk( void );

};




#endif // end of _AIWANDER_H_
