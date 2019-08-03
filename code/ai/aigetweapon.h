//------------------------------------------------------------------------------------------
//!
//!	\file aigetweapon.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIGETWEAPON_H
#define _AIGETWEAPON_H

#include "ai/aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIBehaviour_Attack
//!
//------------------------------------------------------------------------------------------


class AIGetWeapon : public CAIStateMachine
{
public:

	AIGetWeapon( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIGetWeapon( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::GETWEAPON;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_LOCATEWEAPON,
		STATE_GOTOWEAPON,
		STATE_GRABATTEMPT,
		STATE_CHECKPICKUP,
		STATE_MOVEAWAY,
		STATE_ATTACK,
		STATE_RELOAD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			FindWeapon();
	bool			AtWeaponPos() const;
	void			PickupWeapon();
	void			FireWeapon();

	float			m_fTimer;
	CPoint			m_obWeaponLocation;
	bool			m_bWeaponFound;
	bool			m_bWeaponHeld;

};

#endif //_AIGETWEAPON_H
