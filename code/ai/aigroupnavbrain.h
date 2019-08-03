//------------------------------------------------------------------------------------------
//!
//!	\file aigroupnavbrain.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIGROUPNAVBRAIN_H
#define _AIGROUPNAVBRAIN_H


//------------------------------------------------------------------------------------------
//!
//!	AIGroupNavBrain
//!
//------------------------------------------------------------------------------------------


class AIGroupNavBrain
{
public:

	AIGroupNavBrain( CEntity* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIGroupNavBrain( void ) {};


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_ATTACK,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	bool			TooFarFromPlayer( void ) const;

	float			m_fTimer;

};

#endif //_AIGROUPNAVBRAIN_H
