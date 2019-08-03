//--------------------------------------------------
//!
//!	\file game/entitybarrage.h
//!	Definition of the Barrage entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_BARRAGE_H
#define	_ENTITY_BARRAGE_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Object_Barrage.
//! Barrage object
//!
//--------------------------------------------------
class Object_Barrage : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_Barrage)

public:

	enum BARRAGE_TYPE
	{
		BARRAGE_TYPE_PROJECTILE,	// This barrage fires projectiles
		BARRAGE_TYPE_ENTITY,		// This barrage fires entities (catapult rocks...)

		BARRAGE_TYPE_UNKNOWN
	};

	// Constructor
	Object_Barrage();

	// Destructor
	~Object_Barrage();

	// Post Construct
	void OnPostConstruct();

	void SetTarget(CEntity* pEnt) {m_pTargetEntity = pEnt;}

	// Public data so we can access them from the state machine
	int			m_nCurrentFiredNum;

	// Target entity to shoot at (optional)
	CEntity*	m_pTargetEntity;

	// Target area to shoot at
	CPoint		m_obTargetPosition;
	float		m_fTargetRadius;

	// Firing parameters and information
	int			m_nNumShotsToFire;	// 0 = infinite firing
	float		m_fMinShootInterval;
	float		m_fMaxShootInterval;
	bool		m_bCanBeReShot;

	void		FireBarrageShot( void );
	void		ProcessLogic( void );

	bool		m_bCanFire;	// Internal variable used to stop firing while waiting for reuse message

protected:

	BARRAGE_TYPE	m_eBarrageType;

	// Object description
	CKeyString	m_Description;

	// Projectile Def name
	CHashedString	m_ProjectileDef;
	Projectile_Attributes*	m_pProjAttrs;

	// Entity to fire instead of projectile
	CHashedString	m_EntityNameToFire;
};

#endif // _ENTITY_BARRAGE_H
