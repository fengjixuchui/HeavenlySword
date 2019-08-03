//--------------------------------------------------
//!
//!	\file game/entityinteractableturretweapon.h
//!	Definition of the Interactable Turret Weapon
//!
//--------------------------------------------------

#ifndef	_ENTITY_INTERACTABLE_TURRET_WEAPON_H
#define	_ENTITY_INTERACTABLE_TURRET_WEAPON_H

#include "game/entityinteractable.h"
#include "game/turretcontroller.h"

#include "fsm.h"

class Projectile_Attributes;
class AftertouchControlParameters;
class Object_Projectile;

class Att_Turret_Weapon
{
	// Declare dataobject interface
	HAS_INTERFACE(Att_Turret_Weapon)

public:

	Att_Turret_Weapon::Att_Turret_Weapon()
	:	m_pProjectileAttribs(0)
	,	m_pobAftertouchParams(0)
	{;};

	CHashedString					m_sAnimMount;
	CHashedString					m_sAnimDismount;
	CHashedString					m_sAnimRecoil;
	CHashedString					m_sAnimReload;
	CHashedString					m_sMovementController;
	CHashedString					m_sDrivingSeat;
	CHashedString					m_sLaunchTransform;
	Projectile_Attributes*			m_pProjectileAttribs;
	AftertouchControlParameters*	m_pobAftertouchParams;
	CPoint							m_ptTranslationOffset;
	CPoint							m_ptTurrentCam;
	bool							m_bCanDoAftertouch;
	float							m_fShootDelay;
	float							m_fReloadDelay;
	float							m_fReadyDelay;
	float							m_fRecoilTime;
	float							m_fReloadTime;
};

//--------------------------------------------------
//!
//! Class Interactable_TurretWeapon.
//! Base interactable turret weapon
//!
//--------------------------------------------------
class Interactable_TurretWeapon : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_TurretWeapon)

public:
	Interactable_TurretWeapon();
	~Interactable_TurretWeapon();
	void OnPostConstruct();

	// From CEntity
	virtual void OnLevelStart();

public:
	void        SetOperator(Character* pOperator) {m_pOther = pOperator;}
	Character*  GetOperator()                     {return m_pOther;}

	bool        IsExternallyDisabled() const      {return m_bExternallyDisabled;}
	void        SetExternallyDisabled(bool b)     {m_bExternallyDisabled = b;}

	void		EnableProjectileSteering()  {;}
	void		DisableProjectileSteering() {;}

	CHashedString GetMountAnim() const          {return m_pobSharedAttributes ? m_pobSharedAttributes->m_sAnimMount : 0;}
	CHashedString GetDismountAnim() const       {return m_pobSharedAttributes ? m_pobSharedAttributes->m_sAnimDismount : 0;}
	CHashedString GetRecoilAnim() const			{return m_pobSharedAttributes ? m_pobSharedAttributes->m_sAnimRecoil : 0;}
	CHashedString GetReloadAnim() const			{return m_pobSharedAttributes ? m_pobSharedAttributes->m_sAnimReload : 0;}
	CHashedString GetMovementController() const {return m_pobSharedAttributes ? m_pobSharedAttributes->m_sMovementController : 0;}
	CHashedString GetDrivingSeat() const        {return m_pobSharedAttributes ? m_pobSharedAttributes->m_sDrivingSeat : 0;}

	CHashedString GetLaunchTransform() const	{return m_pobSharedAttributes ? m_pobSharedAttributes->m_sLaunchTransform : 0;};

	bool CanDoAftertouch() const				{return m_pobSharedAttributes ? m_pobSharedAttributes->m_bCanDoAftertouch : true; }


	Projectile_Attributes*		GetProjectileAttribs(void)	const	{ return m_pobSharedAttributes ? m_pobSharedAttributes->m_pProjectileAttribs : 0; }
	CPoint						GetTranslationOffset(void)	const	{ return m_pobSharedAttributes ? m_pobSharedAttributes->m_ptTranslationOffset : CPoint(CONSTRUCT_CLEAR); }

	Att_Turret_Weapon*		GetSharedAttributes() { return m_pobSharedAttributes; };

	virtual void	UpdateDerivedComponents( float );

	void			RegisterTurretController ( TurretController* pobController ) { m_pobTurretController = pobController; };

	void			OnFire( void );
	void			OnReload( void );

public:
	int						iCamID;
	Character*				m_pOther;
	Object_Projectile*		m_pActiveCannonBall;
	bool					m_bExternallyDisabled;	
	bool					m_bReloadRequested;
	
	float					m_fMaxYaw;
	
	TURRET_STATE			m_eTurretState;

	TurretController*		m_pobTurretController;

private:
	Att_Turret_Weapon*		m_pobSharedAttributes;

	// The animation container
	CHashedString	m_AnimationContainer;	
};


#endif // _ENTITY_INTERACTABLE_TURRET_WEAPON_H
