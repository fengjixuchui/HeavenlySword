//--------------------------------------------------
//!
//!	\file game/entitycatapultrock.h
//!	Definition of the Catapult rock object  based on Interactable Thrown entity object
//! could be merged back later if the majority of functionality remains the same. T McK
//!
//--------------------------------------------------

#ifndef	_ENTITY_CATAPULT_ROCK_H
#define	_ENTITY_CATAPULT_ROCK_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"

class CoolCam_AfterTouchDef;
class CoolCam_ChaseDef;
class CoolCam_AimDef;
class AftertouchControlParameters;
class Att_Catapult_Rock;
class Object_Catapult;

//--------------------------------------------------
//!
//! Class Att_Ammo
//! Attributes specific to a single firing arm
//!
//--------------------------------------------------
class Att_Ammo
{
	// Declare dataobject interface
	HAS_INTERFACE(Att_Ammo)

public:
	ntstd::String			m_obLaunchTransform;
	CPoint					m_obLaunchTranslation;
	CQuat					m_obLaunchOrientation;
	float					m_fLaunchHeight;
	CPoint					m_obLaunchAngularVelocity;
	float					m_fLaunchTime;
	CPoint					m_obTarget;
	ntstd::String			m_obNameAppend;
	ntstd::String			m_obClump;
	CHashedString			m_obFireAnim;
	CHashedString			m_obReloadAnim;
	float					m_fRespawnTime;
	float					m_fAutoResetTime;

	Att_Ammo()
	:	m_obLaunchTransform ( "ROOT" )
	,	m_obLaunchTranslation ( CONSTRUCT_CLEAR )
	,	m_obLaunchOrientation ( CONSTRUCT_CLEAR )
	//,	m_fLaunchLinearSpeed ( 20.0f )
	,	m_fLaunchHeight ( 10.0f )
	,	m_obLaunchAngularVelocity ( CONSTRUCT_CLEAR )
	,	m_fLaunchTime ( 0.0f )
	,	m_obTarget ( 0.0f, 0.0f, 30.0f )
	,	m_obNameAppend ( "" )
	,	m_obClump ( "" ) 
	,	m_obFireAnim ( "" )
	,	m_obReloadAnim ( "" )
	,	m_fRespawnTime ( 0.0f )
	,	m_fAutoResetTime ( 0.0f )
	{};
};

//--------------------------------------------------
//!
//! Class Att_Catapult_Rock.
//! Attibutes shared by all rocks
//!
//--------------------------------------------------
class Att_Catapult_Rock
{
	// Declare dataobject interface
	HAS_INTERFACE(Att_Catapult_Rock)

public:
	bool							m_bAIAvoid;						// Sets if the AI will try to avoid this object
	float							m_fAIAvoidRadius;				// Sets the radius that the AI will try to avoid the object by

	// TODO (chipb) replace
	// CHashedString					m_obPhysicsSoundDef;			// Sound definitions for the physics stuff

	float							m_fImpactThreshold;				// Magic number to indicate the threshold before an object will break on impact
	bool							m_bDamageOnChar;				// Flag if the object will break in contact with a character
	bool							m_bDamageOnEnv;					// Flag if the object will break in contact with the environment
	bool							m_bCollapseOnDamage;			// Do compound rigid collapse on damage
	bool							m_bRemoveOnDamage;				// Remove mesh on damage
	bool							m_bRebound;						// If the object will rebound for more fun, shields
	bool							m_bInstantKill;					// If the object will kill on impact
	CHashedString					m_obThrownAttackData;			// The definition that defines the strike data
	AftertouchControlParameters*	m_pobAftertouchProperites;		// Aftertouch parameters for the object
	CoolCam_AfterTouchDef*			m_pobAftertouchCamProperties;	// Aftertouch camera paramters
	CHashedString					m_obSfxDestroy;
	CHashedString					m_obSfxCollapse;
	CHashedString					m_obPfxDestroy;
	CHashedString					m_obPfxCollapse;
	ntstd::List<CHashedString>		m_aobPfxLoaded;
	ntstd::List<CHashedString>		m_aobPfxFlight;
	ntstd::List<CHashedString>		m_aobPfxExplosion;
	float							m_fExplosionPush;
	float							m_fExplosionPushDropoff;
	float							m_fExplosionRadius;
	CHashedString					m_obOnAftertouchStart;
	CHashedString					m_obOnAftertouchEnd;
	// Projectile audio control
	CKeyString						m_obLoopingSoundCue;
	CKeyString						m_obLoopingSoundBank;
	float							m_fLoopingSoundRange;
	CKeyString						m_obPassbySoundCue;
	CKeyString						m_obPassbySoundBank;
	float							m_fPassbySoundRange;
	CKeyString						m_obIdleSoundCue;
	CKeyString						m_obIdleSoundBank;
	CKeyString						m_obImpactSoundCue;
	CKeyString						m_obImpactSoundBank;

	Att_Catapult_Rock()
	:	  m_bAIAvoid (false)
	,     m_fAIAvoidRadius (0.0f)

	// TODO (chipb) replace
	// ,     m_obPhysicsSoundDef("SwordPhysicsSoundDef")

	,     m_fImpactThreshold(7.5f)
	,     m_bDamageOnChar(false)
	,     m_bDamageOnEnv(false)
	,     m_bCollapseOnDamage(false)
	,     m_bRemoveOnDamage(false)
	,     m_bRebound(false)
	,     m_bInstantKill(true)
	,     m_obThrownAttackData("atk_generic_obj_strike")
	,     m_pobAftertouchProperites( 0 )
	,     m_pobAftertouchCamProperties( 0 )
	,     m_obSfxDestroy("")
	,     m_obSfxCollapse("")
	,     m_obPfxDestroy("")
	,     m_obPfxCollapse("")
	,     m_obOnAftertouchStart("aftertouchStartAudio")
	,     m_obOnAftertouchEnd("aftertouchEndAudio")
	,     m_fLoopingSoundRange(0.0f)
	,     m_fPassbySoundRange(0.0f)
	{};
};

typedef ntstd::List<CHashedString>::iterator PFXIter;
typedef ntstd::List<u_int>::iterator UIntIter;

//--------------------------------------------------
//!
//! Class Object_Catapult_Rock.
//! Catapults rock entity type
//!
//--------------------------------------------------
class Object_Catapult_Rock : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_Catapult_Rock)

public:
	// Constructor
	Object_Catapult_Rock();

	// Destructor
	~Object_Catapult_Rock();

	// Post Construct
	void OnPostConstruct();

	static CEntity* ConstructCatapultRockObject(Object_Catapult* pobCatapult, Att_Ammo* pobAmmoAttrs, Att_Catapult_Rock* pobSharedAttrs);
	static CEntity* ConstructBarrageCatapultRockObject( uint32_t uSectorBits, const CPoint& obOrigin, const CPoint& obTarget );

	// Helper function for barrages
	void ForceIntoThrowState( void );
	
	CDirection LaunchVelocity ( const CPoint& obOrigin, const CPoint& obTarget, float fWorldHeight );

	Att_Catapult_Rock* GetSharedAttributes(void) { return m_pSharedAttributes; }
	Object_Catapult*	GetCatapult(void)         { return m_pobCatapult; }

	void Ignition( void );
	void OnLaunch( void );
	void OnDestroy( void );

	// Public Variables (once part of attribute table in the script)
	CEntity*	m_pOther;

	int			m_nThrownCameraHandle;
	bool		m_bMovementDone;
	bool		m_bThrown;

	void		Aftertouch_Power_Off(void);

	CPoint m_obTarget;
	CPoint m_obLaunchTranslation;
	CQuat m_obLaunchOrientation;
	CDirection m_obAngularVelocity;
	CDirection m_obLaunchVelocity;
	float m_fLaunchTime;
	float m_fLaunchHeight;

	// Projectile audio control
	void UpdateProjectileFlightAudio(void);
	void UpdateProjectileIdleAudio(void);
	void EnableProjectileFlightAudio(bool bEnable);
	void EnableProjectileIdleAudio(bool bEnable);
	void ProjectileImpactAudio(CEntity* pobEntity);


	#ifdef _DEBUG
		
		void RenderDebugTarget (CPoint& obTarget);
	#endif // _DEBUG
protected:
	// Object description
	CHashedString	m_Description;

	// Initial State
	CHashedString	m_InitialState;

	// The animation container
	CHashedString	m_AnimationContainer;

	Att_Catapult_Rock* m_pSharedAttributes;	

	// Need to isolate these so only used when spawned on catapult
	Att_Ammo*		m_pAmmoAttributes;

public:
	Object_Catapult*		m_pobCatapult;

protected:
	// Attributes so we dont need to use att_ammo if spawned via barrage

	ntstd::List<u_int> m_aiPfxID;

	// Projectile audio control
	unsigned int m_uiLoopingSoundId;	//!< Sound system looping sound identifier, non-zero indicates sound is playing
	unsigned int m_uiPassbySoundId;		//!< Sound system passby sound identifier, non-zero indicates sound is playing
	unsigned int m_uiIdleSoundId;		//!< Sound system idle sound identifier, non-zero indicates sound is playing
	bool m_bProjectileFlightAudio;		//!< Indicates projectile flight audio enabled/disabled
	bool m_bProjectileIdleAudio;		//!< Indicates projectile idle audio enabled/disabled


	bool m_bAttached;
};


#endif // _ENTITY_CATAPULT_ROCK_H
