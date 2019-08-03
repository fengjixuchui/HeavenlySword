//--------------------------------------------------
//!
//!	\file game/entityprojectile.h
//!	Definition of the projectile entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_PROJECTILE_H
#define	_ENTITY_PROJECTILE_H

#include "game/entity.h"
#include "game/entity.inl"
#include "editable/enumlist.h"

#include "fsm.h"

class Object_Ranged_Weapon;	//Forward declare this so the firing weapon can pass itself in.
class Interactable_TurretWeapon;

//--------------------------------------------------
//!
//! Struct Projectile_Attributes
//! XML-Exposed attribute class for the projectiles
//!
//--------------------------------------------------
class HitAreas_Attributes
{
	// Declare dataobject interface
	HAS_INTERFACE(HitAreas_Attributes)
	
public:
	CHashedString  m_Head_AttackData; 
	CHashedString  m_Spine_AttackData;
	CHashedString  m_Pelvis_AttackData;
	CHashedString  m_L_UpperArm_AttackData;
	CHashedString  m_L_ForeArm_AttackData;
	CHashedString  m_R_UpperArm_AttackData;
	CHashedString  m_R_ForeArm_AttackData;
	CHashedString  m_L_Thigh_AttackData;
	CHashedString  m_L_Calf_AttackData;
	CHashedString  m_R_Thigh_AttackData;
	CHashedString  m_R_Calf_AttackData;
};

//--------------------------------------------------
//!
//! Struct Projectile_Attributes
//! XML-Exposed attribute class for the projectiles
//!
//--------------------------------------------------
class Projectile_Attributes
{
	// Declare dataobject interface
	HAS_INTERFACE(Projectile_Attributes)

public:
	
	Projectile_Attributes() : 
		m_SoundVolume(-1.0f),
		m_SoundRadius(-1.0f),
		m_IsVolumeConstant(true),
		m_LoopingSoundRange(0.0f),
		m_PassbySoundRange(0.0f)
	{}

	PROJECTILE_TYPE	m_eType;
	ntstd::String	m_Clump;
	CHashedString	m_Properties;
	CHashedString	m_PropertiesFullyCharged;
	CHashedString	m_AttackData;
	CHashedString	m_AttackDataFullyCharged;
	CHashedString	m_AOEAttackData;	//Area of effect attack data (stike-data for if the projectile explodes etc)

	//Attack data for different hit areas. If they are NULL, m_AttackData is used
	CHashedString m_NonPowered_Front_HitAreas;
	CHashedString m_NonPowered_Back_HitAreas;
	CHashedString m_Powered_Front_HitAreas;
	CHashedString m_Powered_Back_HitAreas;

	ntstd::List<CHashedString>		m_aobPfxOnSpawnAttached;
	ntstd::List<CHashedString>		m_aobPfxOnIgniteAttached;
	ntstd::List<CHashedString>		m_aobPfxOnImpactAttached;

	ntstd::List<CHashedString>		m_aobPfxOnSpawnStatic;
	ntstd::List<CHashedString>		m_aobPfxOnIgniteStatic;
	ntstd::List<CHashedString>		m_aobPfxOnImpactStatic;
	ntstd::List<CHashedString>		m_aobPfxOnDestroyStatic;
	
	
	float			m_AreaOfEffectRadius;	//The size of the AOE strike.
	CHashedString	m_CameraDef;
	CHashedString	m_CameraDefFullyCharged;
	CPoint			m_TrackedEntityOffset;
	float			m_TrackingSpeed;

	// AI related sound parameters
	float			m_SoundVolume;
	float			m_SoundRadius;
	bool			m_IsVolumeConstant;
	// Projectile audio control
	CKeyString		m_LoopingSoundCue;
	CKeyString		m_LoopingSoundBank;
	float			m_LoopingSoundRange;
	CKeyString		m_PassbySoundCue;
	CKeyString		m_PassbySoundBank;
	float			m_PassbySoundRange;
	CKeyString		m_ImpactSoundCue;
	CKeyString		m_ImpactSoundBank;
	CKeyString		m_FireSoundCue;
	CKeyString		m_FireSoundBank;

	CHashedString	m_obPhysicsSoundDef;			// Sound definitions for the physics stuff
};

enum E_SHOOTING_ACCURACY
{
	SHOOT_NON_APPLICABLE = 0,
	SHOOT_FAR_WITH_OFFSET,		// Shoots with tracking, aiming with offset to the original pos. of the target
	SHOOT_CLOSE_WITH_OFFSET,	// Shoots with tracking, aiming with offset to the current pos. of the target
	SHOOT_ACCURATELY			// Shoots with tracking, aiming to the original pos. of the target
};

//--------------------------------------------------
//!
//! Struct Projectile_Data
//! Per-instance data
//!
//--------------------------------------------------
struct Projectile_Data
{
	CEntity* pAttacker;
	CEntity* pOriginator;
	CEntity* pTarget;
	CPoint vPosition;
	CDirection vDirection;
	CHashedString ProjectileProperties;
	CHashedString ProjectileAttackData;
	CHashedString ProjectileAttackDataFullyCharged;
	CHashedString ProjectileAttackDataAOE;	//Area of effect attack data (stike-data for if the projectile explodes etc)

	// Attack data for diffrent body parts
	CHashedString ProjectileNonPoweredFrontHitAreas;
	CHashedString ProjectileNonPoweredBackHitAreas;
	CHashedString ProjectilePoweredFrontHitAreas;
	CHashedString ProjectilePoweredBackHitAreas;

	float AreaOfEffectRadius;
	CHashedString ProjectileCameraDef;
	//bool bFirstPersonAim;
	bool bDestroyOnImpact;
	CPoint vTrackedEntityOffset;
	float fTrackingSpeed;
	float Gravity;
	float SoundVolume;
	float SoundRadius;
	float IsVolumeConstant;
	// Projectile audio control
	CKeyString LoopingSoundCue;
	CKeyString LoopingSoundBank;
	float LoopingSoundRange;
	CKeyString PassbySoundCue;
	CKeyString PassbySoundBank;
	float PassbySoundRange;
	CKeyString ImpactSoundCue;
	CKeyString ImpactSoundBank;
	CKeyString FireSoundCue;
	CKeyString FireSoundBank;
	//float	m_fAccuracy;
	//bool	m_bAlwaysMiss;
	E_SHOOTING_ACCURACY	m_eWayOfShooting;
};


//--------------------------------------------------
//!
//! Class Object_Projectile.
//! Projectile object.
//!
//--------------------------------------------------
class Object_Projectile : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_Projectile)

public:
	
	// Constructor
	Object_Projectile();

	// Destructor
	~Object_Projectile();

	// Post Construct
	void OnPostConstruct();

	// Externally destroy the projectile... Early out from bazooka aftertouch, ranged deflection etc...
	void Destroy();

	// Creation functions when fired from weapons
	static	Object_Projectile* CreateCrossbowBolt(Object_Ranged_Weapon* pRangedWeapon, const Projectile_Attributes* const pProjAttrs, CEntity* targetEnt = NULL, bool bDestroyOnImpact = false);
	static	Object_Projectile* CreateBazookaRockets(Object_Ranged_Weapon* pRangedWeapon, const Projectile_Attributes* const pProjAttrs, CEntity* targetEnt = NULL, bool bDestroyOnImpact = false);
	static	Object_Projectile* CreateCannonBall(Interactable_TurretWeapon* pInteractable, const Projectile_Attributes* const pProjAttrs, CEntity* pTargetEnt = NULL, bool bDestroyOnImpact = false);

	// Temp create - for non C++ crossbows. 
	static	Object_Projectile* CreateCrossbowBolt(CEntity* pRangedWeapon, CEntity* pOther, const Projectile_Attributes* const pProjAttrs, CEntity* targetEnt = NULL, bool bDestroyOnImpact = false, const CDirection* pdirShot = NULL, float fCharge = 0.0f);

	// For the AGen bosses swords
	static Object_Projectile* CreateAGenSword(CEntity* pRangedWeapon, CEntity* pOther, const Projectile_Attributes* const pProjAttrs, CEntity* targetEnt, CDirection* pobDirection = 0);

	// For the King's lightning balls.
	static Object_Projectile* CreateKingLightningBall(CEntity* pKing, CEntity* pOther, const Projectile_Attributes* const pProjAttrs, CEntity* targetEnt, bool bCounterable = false, bool bGoToOrbit = false, bool bLarge = false);
	// For the King's wing-attack, firing crow-type projectiles at you.
	static Object_Projectile* CreateKingWingAttackCrow(CEntity* pKing, CEntity* pOther, const Projectile_Attributes* const pProjAttrs, CEntity* targetEnt, const CPoint &obOffset, bool bRandomTargetOffset = true);

	// Spawn functions when created without a weapon
	static	Object_Projectile* SpawnTrackedProjectile(const Projectile_Attributes* const pProjAttrs, const CPoint& obOrigin, CEntity* pobTarget, bool bDestroyOnImpact = false);
	static	Object_Projectile* SpawnAimedProjectile(const Projectile_Attributes* const pProjAttrs, const CPoint& obOrigin, const CPoint& obTargetPos, bool bDestroyOnImpact = false);

	// Crossbow Bolt event functions
	void	CrossbowOnSpawn( void );
	void	CrossbowIgniteEffects( void );
	void	CrossbowOnImpact( const Message& msg); 
	void	CrossbowOnDestroy( void );
	void	CrossbowOnStrike( CEntity* pStriker );

	// Crossbow Bolt event functions
	void	AGenSwordOnSpawn( void );
	void	AGenSwordOnImpact( CEntity* pCollidee, const CPoint& obCollisionPoint );
	void	AGenSwordOnDestroy( void );
	void	AGenSwordSetStickIntoStaticCollision( bool bStick ) { m_bStickIntoStaticCollision = bStick; };
	void	AGenSwordDestroy();
	void	AGenSwordBringBack();
	void	AGenSwordSetBringBackOnImpact(bool bBack) { m_bAGenSwordBringBackOnImpact = bBack; };
	bool	AGenSwordIsMoving();
	bool	AGenSwordHasMissedTarget();
	void	AGenSwordOnMissedCounter( CEntity* );
	void	AGenSwordFreeze();
	bool	AGenSwordIsFrozen();
	void	AGenSwordBringBackStraight();

	// King lightning ball event functions.
	void	KingLightningBallOnSpawn( bool bLarge = false );
	void	KingLightningBallOnImpact( CEntity* pCollidee, const CPoint& obCollisionPoint );
	void	KingLightningBallOnDestroy( void );
	void	KingLightningBallDestroy();
	void	KingLightningBallBringBack();
	bool	KingLightningBallIsMoving();
	void	KingLightningBallLaunchFromOrbit();

	// King wing attack crows.
	void	KingWingAttackCrowOnSpawn( void );
	void	KingWingAttackCrowOnImpact( CEntity* pCollidee, const CPoint& obCollisionPoint );
	void	KingWingAttackCrowOnDestroy( void );
	void	KingWingAttackCrowDestroy();
	void	KingWingAttackCrowHitRanged();

	// Bazooka Rocket event functions
	void	BazookaOnSpawn( void );
	void	BazookaIgnition( void );
	void	BazookaOnImpact( CEntity* pCollidee );
	void	BazookaOnStrike( CEntity* pStriker );
	void	BazookaOnDestroy( CEntity* pHeroDeflect = NULL );

	// TGS HACKS - TODO
	void	BazookaDoAftertouchDrop( float fEarliestDropOffTime ); // Moves the drop time forward if user exited aftertouch after the threshold time
	float	BazookaGetDropOffTime( void );
	void	TGSDestroyAllProjectiles( void );

	// Cannon ball run methods
	void	CannonBallOnSpawn( void );
	void	CannonBallOnImpact( CEntity* pCollidee );
	void	CannonBallOnDestroy( void );
	
	// If projectile gets countered - just giving them all some implementation for completeness - DGF
	void	CannonBallOnCountered( CEntity* pobCounterer );
	void	BazookaOnCountered( CEntity* pobCounterer );
	void	AGenSwordOnCountered( CEntity* pobCounterer );
	void	KingLightningBallOnCountered( CEntity* pobCounterer );
	void	KingLightningBallOnMissedCounter( CEntity* pobCounterer );
	void	CrossbowOnCountered( CEntity* pobCounterer );

	// Dummy Bazooka Rocket event functions
	void	DummyBazookaOnSpawn( void );
	void	DummyBazookaOnImpact( void );

	// Projectile audio control
	void UpdateProjectileAudio(void);
	void EnableProjectileAudio(bool bEnable);
	void ProjectileImpactAudio(CEntity* pobCollidee);
	void ProjectileFireAudio();

	// Allow setting of the charge (archer only) 
	void	SetCharge( float fValue )	{ m_fCharge = fValue; }
	float	GetCharge( ) const			{ return m_fCharge; }
	float	GetAttackScalar( ) const	{ return m_fCharge; }


	CEntity* GetShooter( void ) const	{ return m_ProjectileData.pAttacker; };
	CEntity* GetWeapon( void ) const	{ return m_ProjectileData.pOriginator; };
	CEntity* GetTarget( void ) const	{ return m_ProjectileData.pTarget; };

	// Type of projectile this is
	PROJECTILE_TYPE			m_eProjectileType;

	// Projectile Data
	Projectile_Data			m_ProjectileData;

	const Projectile_Attributes*  m_pobSharedAttributes;

	// Misc data
	int						m_iCameraHandle;	// Camera handle
	bool					m_bInAftertouch;	// In Aftertouch?
	bool					m_bOnFire;			// Is Projectile on fire?  Used by crossbow bolts.
	int						m_iProjectileIndex;	// Used for distance drop off on bazooka rockets.
	bool					m_bAlreadyExploded;	// HACK: Stop the bazooka exploding twice.

	// Projectile audio control
	unsigned int			m_uiLoopingSoundId;	// Sound system looping sound identifier, non-zero indicates sound is playing
	int						m_iLoopingSoundArg;	// If non-zero, passed to the looping sound FMOD event as the rocketnumber parameter
	unsigned int			m_uiPassbySoundId;	// Sound system passby sound identifier, non-zero indicates sound is playing
	bool					m_bProjectileAudio;	// Indicates projectile audio enabled/disabled

	bool					m_bHasBeenCountered;
	bool					m_bFailedToCounterFlagged;
	bool					m_bIsCounterable;
	bool					m_bIsOrbitingOriginator;	//Clearly king-specific!
	float					m_fOrbitAngle;				//We spin on x/z or y/z using this angle (specified in radians)
	bool					m_bLarge;					//King-specific again, large or small lightningballs?
	bool					m_bEffectOnDestroy;

	float					m_fTimeSinceSpawn;	// So that we can remove king-spawned crow projectiles on two time-based conditions.

	// Currently put in AGenImpact only - DGF
	void					SetGenerateNoStrikes(bool bSet) { m_bGenerateNoStrikes = bSet; };
protected:
	
	// Object description
	CKeyString	m_Description;

	// Initial State
	CKeyString	m_InitialState;

	// Create Projectile Object functions - actually create the objects in the database
	static	Object_Projectile* CreateNewProjectileInDatabase( const Projectile_Attributes* const pProjAttrs );
	void	ConstructCrossbowObject( void );
	void	ConstructAGenSwordObject( void );
	void	ConstructKingLightningBallObject( bool bLarge = false );
	void	ConstructKingWingAttackCrowObject( void );
	void	ConstructBazookaObject( void );
	void	ConstructCannonballObject( void );

	void	ReparentProjectile(CEntity* pobTarget, const CPoint& obCollisionPoint);

	// Particle ID's needed to kill effects
	
	union
	{
		// Bazooka
		struct
		{
			u_int	m_PfxIgnitionID;
			u_int	m_PfxSeederID;
			u_int	m_PfxFlameID;
			u_int	m_PfxSmokeID;
		} m_BazookaPfxIDs;
		
		// Crossbow
		struct
		{
			u_int	m_PfxTrailID;
			u_int	m_PfxOnFireFlameID;
			u_int	m_PfxOnFireSmokeID;
		} m_CrossbowPfxIDs;

		// AGenSword
		struct
		{
			u_int	m_PfxTrailID;
		} m_AGenSwordPfxIDs;

		// King Lightningball.
		struct
		{
			u_int	m_PfxTrailID;
			u_int	m_PfxEmbersID;
		} m_KingLightningBallPfxIDs;

		//King Crow.
		struct
		{
			u_int	m_PfxTrailID;
			u_int	m_PfxDestroy;
		} m_KingCrowPfxIDs;
	};

	ntstd::List<u_int> m_aiPfxID;

	bool m_bStickIntoStaticCollision; // Defaults to false, used only in the AGenSword type
	bool m_bAGenSwordBringBackOnImpact;

	//--------------------------------------------------
	// TGS MEGA SUPER DUPER, MUST BE REMOVED, HAXXXOR.
	//--------------------------------------------------
	CHashedString BazookaChildren[3];
	//--------------------------------------------------
	// TGS MEGA SUPER DUPER, MUST BE REMOVED, HAXXXOR.
	//--------------------------------------------------

	// Whilst only used for the archer - it could be used for other things ( i doubt it thought )
	float m_fCharge;

	bool m_bGenerateNoStrikes;
public:
	// For linked projectiles
	Object_Projectile* m_pNextProjectile;
	Object_Projectile* m_pPrevProjectile;

	typedef ntstd::List<CHashedString>::const_iterator PFXIter;
};

typedef ntstd::List<u_int>::iterator UIntIter;

#endif // _ENTITY_PROJECTILE_H
