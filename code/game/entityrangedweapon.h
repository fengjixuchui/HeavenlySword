//--------------------------------------------------
//!
//!	\file game/entityrangedweapon.h
//!	Definition of the ranged weapon object
//!
//--------------------------------------------------

#ifndef	_ENTITY_RANGEDWEAPON_H
#define	_ENTITY_RANGEDWEAPON_H

#include "game/entityinteractable.h"
//#include "game/entityprojectile.h"
class Object_Projectile;	//Forward declare this.
class Projectile_Attributes;
class AftertouchControlParameters;
class CoolCam_AfterTouchDef;
class CoolCam_ChaseDef;
class CoolCam_AimDef;

#include "fsm.h"

//--------------------------------------------------
//!
//! Class Att_RangedWeapon.
//! Interactable definition params
//!
//--------------------------------------------------

class Att_RangedWeapon
{
	// Declare dataobject interface
	HAS_INTERFACE(Att_RangedWeapon)

public:
	float m_fMass;
	CPoint m_obCenterOfMass;
	float m_fRestitution;
	float m_fFriction;
	float m_fLinearDamping;
	float m_fAngularDamping;
	float m_fMaxLinearVelocity;
	float m_fMaxAngularVelocity;
	CHashedString m_obPhysicsSoundDef;
	
	float m_fImpactThreshold;
	int m_iHitCount;
	bool m_bDamageOnChar;
	bool m_bDamageOnEnv;
	CHashedString m_obThrownAttackData;		

	int m_iAmmo;			
	bool m_bReloadAfterShot;

	
	Projectile_Attributes* m_pobProjectileDef;
	CPoint m_obTranslationOffset;
	float m_fThrowTime;						
	float m_fDropTime;
	CPoint m_obThrowVelocity;
	CPoint m_obThrowAngularVelocity;
	CPoint m_obDropVelocity;

	bool	m_bAftertouchOnThrow;
	float	m_fAfterTouchWaitTime;

	CHashedString m_obAnimObjectShoot;
	CHashedString m_obAnimObjectReload;
	CHashedString m_obAnimPlayerMoveTo;
	CHashedString m_obAnimPlayerRunTo;
	CHashedString m_obAnimPlayerPickup;
	CHashedString m_obAnimPlayerRunPickup;
	CHashedString m_obAnimPlayerShoot;
	CHashedString m_obAnimPlayerReload;
	CHashedString m_obAnimPlayerThrow;
	CHashedString m_obAnimPlayerDrop;

	CHashedString m_obAnimIdleToAim;
	CHashedString m_obAnimAimToIdle;

	MovementControllerDef* m_pobPlayerHoldingMovement;
	MovementControllerDef* m_pobPlayerAimingMovement;
	MovementControllerDef* m_pobPlayerThrowMovement;
	MovementControllerDef* m_pobPlayerShootMovement;

	AftertouchControlParameters* m_pobAftertouchProperties;
	CoolCam_AfterTouchDef* m_pobAftertouchCamProperties;
	CoolCam_ChaseDef* m_pobChasecamProperties;
	CoolCam_AimDef*  m_pobAimcamProperties;

	CHashedString m_obPfxImpact;
	CHashedString m_obPfxDestroyed;
	CHashedString m_obSfxDestroyed;
	CHashedString m_obOnAftertouchStart;
	CHashedString m_obOnAftertouchEnd;
	RANGED_WEAPON_TYPE m_eRangedWeaponType;

	Att_RangedWeapon()
	:     m_fMass (1.0f)
	,     m_obCenterOfMass( CONSTRUCT_CLEAR )
	,     m_fRestitution( 0.4f )
	,     m_fFriction(0.5f)
	,     m_fLinearDamping(0.0f)
	,     m_fAngularDamping(0.05f)
	,     m_fMaxLinearVelocity(200.0f)
	,     m_fMaxAngularVelocity(100.0f)
	,     m_obPhysicsSoundDef("")
	,     m_fImpactThreshold(0.0f)
	,     m_iHitCount( 0 )
	,     m_bDamageOnChar(false)
	,     m_bDamageOnEnv(false)
	,     m_obThrownAttackData("")
	,     m_iAmmo(0)	
	,     m_bReloadAfterShot(false)
	,     m_pobProjectileDef( 0 )
	,     m_obTranslationOffset( CONSTRUCT_CLEAR )
	,     m_fThrowTime(0.0f)
	,     m_fDropTime(0.0f)
	,     m_obThrowVelocity( CONSTRUCT_CLEAR )
	,     m_obThrowAngularVelocity( CONSTRUCT_CLEAR )
	,     m_obDropVelocity( CONSTRUCT_CLEAR )
	,     m_bAftertouchOnThrow(false)
	,	  m_fAfterTouchWaitTime(0.15f)
	,     m_obAnimObjectShoot("")
	,     m_obAnimObjectReload("")
	,     m_obAnimPlayerMoveTo("")
	,     m_obAnimPlayerRunTo("")
	,     m_obAnimPlayerPickup("")
	,     m_obAnimPlayerRunPickup("")
	,     m_obAnimPlayerShoot("")
	,     m_obAnimPlayerReload("")
	,     m_obAnimPlayerThrow("")
	,     m_obAnimPlayerDrop("")
	,     m_pobPlayerHoldingMovement( 0 )
	,     m_pobPlayerAimingMovement( 0 )
	,     m_pobPlayerThrowMovement( 0 )
	,     m_pobPlayerShootMovement( 0 )
	,     m_pobAftertouchProperties( 0 )
	,     m_pobAftertouchCamProperties( 0 )
	,     m_pobChasecamProperties( 0 )
	,     m_pobAimcamProperties( 0 )
	,     m_obPfxImpact("")
	,     m_obPfxDestroyed("")
	,     m_obSfxDestroyed("")
	,     m_obOnAftertouchStart("")
	,     m_obOnAftertouchEnd("")
	,	  m_eRangedWeaponType(RWT_NONE)
	{}
};

//--------------------------------------------------
//!
//! Class Object_Ranged_Weapon.
//! Ragdoll object.
//!
//--------------------------------------------------
class Object_Ranged_Weapon : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_Ranged_Weapon)

public:
	// Constructor
	Object_Ranged_Weapon();

	// Destructor
	~Object_Ranged_Weapon();

	// Post Construct
	void OnPostConstruct();

	Att_RangedWeapon* GetSharedAttributes(void) { return m_pSharedAttributes; }
	Projectile_Attributes* GetProjectileAttributes(void) { return m_pSharedAttributes->m_pobProjectileDef; }
	CPoint	GetTranslationOffset(void) { return m_pSharedAttributes->m_obTranslationOffset; }
	
	void PlayAnim( CHashedString anim );	

	Character*	m_pOther;
	int			m_iProjectileCount;
	int			m_iAmmo;
	int			m_iThrownCameraHandle;
	bool		m_bAiming;
	bool		m_bAimingEnableRequest;
	bool		m_bDisableAiming;
	bool		m_bShotFired;
	bool		m_bInAftertouch;
	bool		m_bMovementDone;
	bool		m_bAtRest;

	ntstd::Vector<Object_Projectile*> Projectile;
	ntstd::Vector<ntstd::String> ProjectileName;

	//Overload CEntity::IsRangedWeapon() for this class (so we can cast to Object_Ranged_Weapon for query and any other
	//weapon-type-specific info can go here in this class.
	virtual bool IsRangedWeapon() const { return true; }
	RANGED_WEAPON_TYPE GetRangedWeaponType()
	{
		ntError_p(m_pSharedAttributes, ("Ranged weapon with no attributes") );
		return m_pSharedAttributes->m_eRangedWeaponType;
	}

	void SetHeldMatrix(const CMatrix &obHeldMatrix) { m_obHeldMatrix = obHeldMatrix; }
	CMatrix GetHeldMatrix() { return m_obHeldMatrix; }

protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	CKeyString	m_InitialState;

	// Animation container
	CHashedString	m_AnimationContainer;

	Att_RangedWeapon* m_pSharedAttributes;
private:
	CMatrix m_obHeldMatrix;
};


#endif // _ENTITY_RANGEDWEAPON_H
