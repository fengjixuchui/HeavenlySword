/***************************************************************************************************
*
*	DESCRIPTION		Header file defining extended task functionality.
*
*	NOTES
*
*	OWNER			GavB, 16/02/05
*
***************************************************************************************************/

#ifndef _AI_TASKS_H
#define _AI_TASKS_H

// Necessary includes
#include "ai/aitask.h"
#include "editable/enumlist.h"

// Forward declarations
class CEntity;
namespace NinjaLua { class LuaObject; }

CAITask* CreateNewTask( const char* pcName, NinjaLua::LuaObject obArgs );

/***************************************************************************************************
*
*	CLASS			CAIWalkToPointTask
*
*	DESCRIPTION		
*
*	OWNER			GavB, 16/02/05
*
***************************************************************************************************/

class CAIWalkToPointTask : public CAITask
{
public:

	virtual void OnFirstUpdate( CAIComponent* );
	virtual void Update(float, CAIComponent* );

	void SetDestPoint( const CPoint& obPoint ) { m_obDestPoint = obPoint; }
	CPoint GetDestPoint( void ) const { return m_obDestPoint; }

	void SetMagnitude( float fMagnitude ) { m_fMagnitude = fMagnitude; }
	float GetMagnitude( void ) const { return m_fMagnitude; }

protected:

	// Where to move to
	CPoint m_obDestPoint;

	// The speed in which to get to the dest
	float m_fMagnitude;
};

/***************************************************************************************************
*
*	CLASS			CAIUpdateFormation
*
*	DESCRIPTION		
*
*	OWNER			GavB, 03/03/05
*
***************************************************************************************************/

class CAIUpdateFormation : public CAITask
{
public:
	virtual void Update(float, CAIComponent* );
};

/***************************************************************************************************
*
*	CLASS			CAIUpdateAttackPosition
*
*	DESCRIPTION		
*
*	OWNER			GavB, 03/03/05
*
***************************************************************************************************/

class CAIUpdateAttackPosition : public CAITask
{
public:
	virtual void Update(float, CAIComponent* );

	void SetTarget(CEntity* pTarget ) { m_pTarget = pTarget; }
	void SetUpdateMovement( bool bValue ) { m_bUpdateMovement = bValue; }

	CEntity*	m_pTarget;
	bool		m_bUpdateMovement;
};

/***************************************************************************************************
*
*	CLASS			CAIForwardPlayerCombatStateChanges
*
*	DESCRIPTION		
*
*	OWNER			GavB, 03/03/05
*
***************************************************************************************************/

class CAIForwardPlayerCombatStateChanges : public CAITask
{
public:
	virtual void Update(float, CAIComponent* );

	COMBAT_STATE  m_ePlayerState;

};

/***************************************************************************************************
*
*	CLASS			CAITask_KeyboardCapture
*
*	DESCRIPTION		
*
*	OWNER			GavB, 20/04/05
*
***************************************************************************************************/

class CAITask_KeyboardCapture : public CAITask
{
public:
	virtual void Update(float, CAIComponent* );
};

//--------------------------------------------------
//!
//!  CAICombatTask
//! 
//! 
//!
//-------------------------------------------------

class CAICombatTask : public CAITask
{
	enum MOVEMENT_STATE
	{
		MS_INITIALISE,
		MS_WAITING_FOR_RADIUS,
		MS_MOVING,
	};

public:
	CAICombatTask(void) :
		m_eMovementState(MS_INITIALISE)
	{
	}

	virtual void Update(float, CAIComponent* );

//	void SetTrackPlayer(bool bTrackPlayer) { m_bTrackPlayer = bTrackPlayer; }
//	void SetTarget(CEntity* pTarget ) { m_pTarget = pTarget; }
//	void SetRange(float fRangeSqrd ) { m_fRangeSqrd = fRangeSqrd*fRangeSqrd; }
//	void SetExitRange(float fRangeSqrd ) { m_fExitRangeSqrd = fRangeSqrd*fRangeSqrd; }

//	bool		m_bTrackPlayer;
//	CEntity*	m_pTarget;
//	float		m_fRangeSqrd;
//	float		m_fExitRangeSqrd;
//	CPoint		m_obLastGoodPos;

	// Keeps track of the current movement state of the entity
	MOVEMENT_STATE	m_eMovementState;
};


#endif // _AI_TASKS_H
