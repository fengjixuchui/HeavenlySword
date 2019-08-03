//------------------------------------------------------------------------------------------
//!
//!	\file airanged.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIRANGED_H
#define _AIRANGED_H

#include "ai/aistatemachine.h"

//------------------------------------------------------------------------------------------
//!
//!	AIRanged
//!
//!	Behaviour class for ranged combat soldiers
//------------------------------------------------------------------------------------------

class AIRanged : public CAIStateMachine
{
public:

	AIRanged( AI* pobEnt ) : CAIStateMachine(pobEnt), m_iCoverPointId(-1) {};
	~AIRanged( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::RANGED;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_LOCATECOVER,
		STATE_GOTOCOVER,
		STATE_PAUSE,
		STATE_MOVETOFIREPOINT,
		STATE_ATTACK,
		STATE_RETURNTOCOVER,
		STATE_RELOAD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			FindCover();
	void			PickupWeapon();
	void			FireWeapon();
	bool			CheckFallback( float threshold ) const;

	float			m_fTimer;
	CPoint			m_obCoverPosition;
	CPoint			m_obFirePosition;
	bool			m_bCoverFound;
	int				m_iCoverPointId;
};



//------------------------------------------------------------------------------------------
//!
//!	AIRanged subclasses
//!
//!	Ranged behaviour split down into small behaviours to allow for greater scripting
//! flexibility
//!
//! Names should all be self explanatory
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//!
//!	AIFindCover
//!
//------------------------------------------------------------------------------------------

class AIFindCover : public CAIStateMachine
{
public:

	AIFindCover( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIFindCover( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::FINDCOVER;}

private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_LOCATECOVER,
		STATE_GOTOCOVER
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			FindCover();

	float			m_fTimer;
	bool			m_bCoverFound;
	CPoint			m_obCoverPosition;
};

//------------------------------------------------------------------------------------------
//!
//!	AIOpenFire
//!
//------------------------------------------------------------------------------------------

class AIOpenFire : public CAIStateMachine
{
public:

	AIOpenFire( AI* pobEnt ) : CAIStateMachine(pobEnt) {};
	~AIOpenFire( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::OPENFIRE;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_PAUSE,
		STATE_MOVETOFIREPOINT,
		STATE_ATTACK,
		STATE_RETURNTOCOVER,
		STATE_RELOAD,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			FireWeapon();

	float			m_fTimer;
	CPoint			m_obCoverPosition;
	CPoint			m_obFirePosition;
	bool			m_bUsingCover;
	bool			m_bHasFirePoint;
};

//------------------------------------------------------------------------------------------
//!
//!	AIHoldFire
//!
//------------------------------------------------------------------------------------------

class AIHoldFire : public CAIStateMachine
{
public:

	AIHoldFire( AI* pobEnt ) : CAIStateMachine(pobEnt), m_iCoverPointId(-1) {};
	~AIHoldFire( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::HOLDFIRE;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_PAUSE,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );

	float			m_fTimer;
	int				m_iCoverPointId;
};



// bind functions to go on component

bool IsEntityInCover();
void SpecifyTarget( char* targetName );


//------------------------------------------------------------------------------------------
//!
//!	AIBallista
//!
//!	Behaviour class for soldiers using the ballista
//------------------------------------------------------------------------------------------

class AIBallista : public CAIStateMachine
{
public:

	AIBallista( AI* pobEnt ) : CAIStateMachine(pobEnt), m_bTargetFound(false), m_obTargetPos(CONSTRUCT_CLEAR), m_pobTarget(NULL) { m_iNum = m_iCount++; };
	~AIBallista( void ) {};

	virtual CAIBehaviourManager::eBEHAVIOUR GetType() {return CAIBehaviourManager::BALLISTA;}


private:

	enum STATES
	{
		STATE_INITIALISE,
		STATE_ACQUIRETARGET,
		STATE_WATCHTARGET,
		STATE_FIRE,
		STATE_PAUSE,
	};

	virtual bool	States( const STATE_MACHINE_EVENT eEvent, const int iState, const float fTimeChange );
	void			FireWeapon();
	void			FindTarget();
	void			UpdateTargetPos();
	void			UpdateAngleDifferences();
	
	float			GetYawAngle( const CDirection& obVec1, const CDirection& obVec2 );
	float			GetPitchAngle( const CDirection& obVec1, const CDirection& obVec2, const CMatrix& worldMatrix );

	float			AngleDiffToInput( float angle, const float minInput, const float maxInput );
	void			SetTrackingAction( const float yawDiff, const float pitchDiff );

	float			m_fTimer;
	bool			m_bTargetFound;
	CPoint			m_obTargetPos;
	CEntity*		m_pobTarget;

	float			m_fYawDiff;
	float			m_fPitchDiff;

	static int		m_iCount;
	int				m_iNum;
};




#endif //_AIRANGED_H


