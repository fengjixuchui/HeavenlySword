//------------------------------------------------------------------------------------------
//!
//!	\file aiuseobject.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIUSEOBJECT_H
#define _AIUSEOBJECT_H

#include "aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIUseObject
//!
//------------------------------------------------------------------------------------------


class AIUseObject : public CAIStateMachine
{
public:

	AIUseObject( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIUseObject( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::WALKTOLOCATOR;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_FINDOBJECT,
		STATE_GOTOOBJECT,
		STATE_USEOBJECT,
		STATE_OBJECTUNREACHABLE,
		STATE_STOPANDNOTIFY,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			FindThing( const char* substring, bool& thingFound, CPoint& thingPos );
	bool			AtTargetPos( const CPoint& target, const float threshold ) const;
	void			UseObject();

	float			m_fTimer;
	CPoint			m_obObjectPos;
	bool			m_bObjectFound;
	bool			m_bObjectUsed;

};

#endif //_AIGETWEAPON_H
