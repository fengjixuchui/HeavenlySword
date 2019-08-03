//------------------------------------------------------------------------------------------
//!
//!	\file aiwalktolocator.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIWALKTOLOCATOR_H
#define _AIWALKTOLOCATOR_H

#include "aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIWalkToLocator
//!
//------------------------------------------------------------------------------------------


class AIWalkToLocator : public CAIStateMachine
{
public:

	AIWalkToLocator( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIWalkToLocator( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::WALKTOLOCATOR;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_FINDLOCATOR,
		STATE_GOTOLOCATOR,
		STATE_FINDFACINGLOCATOR,
		STATE_TURNTOFACE,
		STATE_LOCATORUNREACHABLE,
		STATE_STOPANDNOTIFY,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			FindThing( const char* substring, bool& thingFound, CPoint& thingPos );
	bool			AtTargetPos( const CPoint& target, const float threshold ) const;

	float			m_fTimer;
	CPoint			m_obLocatorPos;
	bool			m_bLocatorFound;
	CPoint			m_obFacingLocatorPos;
	bool			m_bFacingLocatorFound;
	CDirection		m_obFacing;
};

#endif
