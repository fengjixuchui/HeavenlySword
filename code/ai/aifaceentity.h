//------------------------------------------------------------------------------------------
//!
//!	\file aifaceentity.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIFACEENTITY_H
#define _AIFACEENTITY_H

#include "aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIFaceEntity
//!
//------------------------------------------------------------------------------------------


class AIFaceEntity : public CAIStateMachine
{
public:

	AIFaceEntity( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIFaceEntity( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::FACEENTITY;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_FINDENTITY,
		STATE_TURNTOFACE,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			FindThing( const char* substring, bool& thingFound, CPoint& thingPos );
	bool			AtTargetPos( const CPoint& target, const float threshold ) const;

	float			m_fTimer;
	CEntity*		m_pobFacingEntity;
	CPoint			m_obEntityPos;
	bool			m_bEntityFound;
	CDirection		m_obFacing;
};

#endif
