#ifndef	_ENTITY_AGEN_H
#define	_ENTITY_AGEN_H

#include "entityboss.h"
#include "entityprojectile.h"

class Interactable_Thrown;

class AerialGeneral : public Boss
{
	HAS_INTERFACE(AerialGeneral);

	#define AGEN_DOPPELGANGER_COUNT 30
public:
	AerialGeneral();
	~AerialGeneral();
	void OnPostPostConstruct();
	bool CanStartAnAttack();
	bool CanStartALinkedAttack();
	void UpdateBossSpecifics(float fTimeDelta);
	void DebugRenderBossSpecifics( CPoint& obScreenLocation, float fXOffset, float fYOffset );
	bool IsInAir() { return m_bInAir; };
	void SetIsInAir(bool bAir) { m_bInAir = bAir; };
	void NotifyBoomerangSwordCountered(CEntity* pobCounterer);
	void SetIsDoppelganger(int iDoppelganger) { m_iDoppelganger = iDoppelganger; };
	int GetIsDoppelganger() { return m_iDoppelganger; };

	void WakeUpDoppelganger( bool bDoSplit = true, bool bForce = false );
	const CPoint& GetArenaCentre() { return m_obArenaCentre; };
	float GetArenaRadius() { return m_fArenaRadius; };
	void SetIsDoppelgangerPaused(bool bPause) { m_bDoppelgangerPaused = bPause; };
	bool GetIsDoppelgangerPaused() { return m_bDoppelgangerPaused; };

	void UnholsterSword(bool bRightHand, bool bPowerSword);
	void HolsterSword(bool bRightHand, bool bPowerSword);

	void SetSplitting(bool bSplitting) { m_bSplitting = bSplitting; };
	bool GetSplitting() { return m_bSplitting; };

	AerialGeneral* GetNextDoppelgangerToWakeUp();
private:
	bool m_bSplitting;
	bool m_bInAir;

	CHashedString m_aobDoppelgangerAGenEntityNames[AGEN_DOPPELGANGER_COUNT];
	AerialGeneral* m_apobDoppelgangers[AGEN_DOPPELGANGER_COUNT];
	CHashedString m_obAGenSplitAnim, m_obDoppelgangerSplitAnim;
	CHashedString m_obAllDoppelgangersActiveMessage, m_obAllDoppelgangersActiveMessageEntityName;
	
	//The in-hand swords.
	CEntity* CreateInHandSword(bool bRightHand, bool bPowerSword, const char* pcName);
	CEntity* m_pLeftSword;
	CEntity* m_pRightSword;
	CEntity* m_pLeftPowerSword;
	CEntity* m_pRightPowerSword;

	bool m_bDoppelgangerPaused;

	//Exposed.
	int m_iRightHandSwordBackTransform1;
	int m_iRightHandSwordBackTransform2;
	int m_iLeftHandSwordBackTransform1;
	int m_iLeftHandSwordBackTransform2;

    int m_iDoppelganger;

	ntstd::Vector<CEntity*> m_obBackSwords;

	CPoint m_obArenaCentre;
	float m_fArenaRadius;

	int m_iNumberOfDoppelgangersTillEnd;
};

struct DoppelgangerAttackingSlot
{
	AerialGeneral* m_pobAGen;
	float m_fTimeAttackingSoFar;
	float m_fTimeAllowedAttacking;
};

class DoppelgangerManager : public Singleton<DoppelgangerManager>
{
public:
	HAS_LUA_INTERFACE()

	DoppelgangerManager();
	void Update( float fTimeDelta );
	void RegisterDoppelganger( CEntity* pobAGen );
	void RegisterMaster( CEntity* pobAGen );
	void SetActive( bool bActive ) { m_bActive = bActive; };
	bool IsActive() { return m_bActive; };
	bool CanIAttack( CEntity* pobAGen );
	void SetMaxNumberOfAttackers( int iAttackers );
	void SetAttackerSwitchFrequency( float fMaxFrequency, float fMaxFrequencyAdjust ) { m_fMaxFrequency = fMaxFrequency; m_fMaxFrequencyAdjust = fMaxFrequencyAdjust; };
	AerialGeneral* GetMaster() { return m_pobMaster; };
	float GetArenaFormationDistance( AerialGeneral* pobAGen );
	void SetArenaFormationStartDistance( float fArenaFormationStartDistance ) { m_fArenaFormationStartDistance = fArenaFormationStartDistance; };
	float GetTimeSinceLastWakeUp() { return m_fTimeSinceLastWakeUp; };
	void ResetTimeSinceLastWakeUp() { m_fTimeSinceLastWakeUp = 0.0f; };
private:
	float m_fTimeSinceLastWakeUp;
	float m_fArenaFormationStartDistance;
	AerialGeneral* m_pobMaster;
	bool m_bFirstFrame;
	int m_iAttackers;
	bool m_bActive;
	float m_fMaxFrequency, m_fMaxFrequencyAdjust;

	ntstd::Vector< AerialGeneral* > m_apobNonAttackers;
	ntstd::Vector< DoppelgangerAttackingSlot* > m_apobAttackers;
};

class AerialGeneralIntoAirTransitionMovement : public BossTransitioningMovement
{
	HAS_INTERFACE(AerialGeneralIntoAirTransitionMovement)
public:
	AerialGeneralIntoAirTransitionMovement() { m_bDone = false; m_fHoverHeight = 1.0f; };
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone() { m_bDone = true; };
private:
	bool m_bDone;

	CHashedString m_obIntoAirAnim;
	float m_fHoverHeight;
};

class AerialGeneralOntoGroundTransitionMovement : public BossTransitioningMovement
{
	HAS_INTERFACE(AerialGeneralOntoGroundTransitionMovement)
public:
	AerialGeneralOntoGroundTransitionMovement() { m_bDone = false; };
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone() { m_bDone = true; };
private:
	bool m_bDone;

	CHashedString m_obFallStartAnim, m_obFallCycleAnim, m_obHitGroundAnim;
};

class AerialGeneralAttackSelector : public BossAttackSelector
{
	HAS_INTERFACE(AerialGeneralAttackSelector)
public:
	AerialGeneralAttackSelector() : BossAttackSelector() { m_bInAir = false; };
protected:
	bool m_bInAir;
};

//------------------------------------------------------------------------------------------
//!
//!	RandomAerialGeneralInAirAttackSelector
//! Class for selecting special attacks randomly
//!
//------------------------------------------------------------------------------------------
class RandomAerialGeneralAttackSelector : public AerialGeneralAttackSelector
{
	HAS_INTERFACE(RandomAerialGeneralAttackSelector)
public:
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_RANDOM; }
};

class AerialGeneralMovementSelector : public BossMovementSelector
{
	HAS_INTERFACE(AerialGeneralMovementSelector)
public:
	AerialGeneralMovementSelector() : BossMovementSelector() { m_bInAir = false; };
protected:
	bool m_bInAir;
};
//------------------------------------------------------------------------------------------
//!
//!	RandomAerialGeneralInAirMovementSelector
//!
//------------------------------------------------------------------------------------------
class RandomAerialGeneralMovementSelector : public AerialGeneralMovementSelector
{
	HAS_INTERFACE( RandomAerialGeneralMovementSelector );
public:
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* GetSelectedMovement();
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
private:
	int m_iSelectedMovement;
};

//------------------------------------------------------------------------------------------
//!
//!	DistanceSuitabilityAerialGeneralAttackSelector
//!
//------------------------------------------------------------------------------------------
class DistanceSuitabilityAerialGeneralAttackSelector : public AerialGeneralAttackSelector
{
	HAS_INTERFACE(DistanceSuitabilityAerialGeneralAttackSelector)
public:
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_DISTANCE; }

private:
	ntstd::List<BossAttack*> m_obInRangeAttacks;
	BossAttack *m_pobLastSelection, *m_pobSelectedAttack;
};

class AerialGeneralBoomerangSpecialAttack : public BossSpecialAttack
{
	HAS_INTERFACE(AerialGeneralBoomerangSpecialAttack)
public:
	AerialGeneralBoomerangSpecialAttack();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone();
	virtual void NotifyProjectileCountered(Object_Projectile* pobProj);
	virtual void NotifyPlayerInteracting(bool bState);
	virtual void NotifyPlayerInteractionAction();
	virtual BossAttack* NotifyAttackInterrupted();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual bool IsVulnerableTo(CStrike* pobStrike);
	virtual bool IsVulnerableTo(const CAttackData* pobAttackData);
	virtual ~AerialGeneralBoomerangSpecialAttack() {};
private:
	bool m_bDone;
	Object_Projectile* m_pobSword;
	Interactable_Thrown* m_pobCounteredSword;
	bool m_bSwordCreatedForThisStrikeWindow;
	bool m_bMovementInBetweenInitialised;
	bool m_bSwordComingBack;
	float m_fLastSwordToAGenDistance;
	bool m_bDoneAttack;
	bool m_bCatching;
	bool m_bBeingSyncCountered, m_bBeingThrownCountered;
	bool m_bNotifyNotInAir;
	CPoint m_obTargetPoint;
	CAttackLink* m_pobAttackToUse;
	bool m_bGoading, m_bDiving, m_bStartDiving, m_bGotStruck;
	BossMovement* m_pobDive;

	// Exposed:
	BossMovement* m_pobBeginningMovement;
	CAttackLink* m_pobThrowAttackLeft;
	CAttackLink* m_pobThrowAttackRight;
	BossMovement* m_pobMovementInBetween;
	CAttackLink* m_pobCatchAttack;
	Projectile_Attributes* m_pobProjectileAttributes;
	float m_fCatchDistance;
	BossMovement *m_pobGoadingMovement, *m_pobDiveLeftMovement, *m_pobDiveRightMovement;
	float m_fRandomAugmentAngle;
	ntstd::String m_obCounteredSwordSharedAttribs, m_obCounteredSwordClump;
};

class AerialGeneralSwoopMovement : public BossTransitioningMovement
{
	HAS_INTERFACE(AerialGeneralSwoopMovement)
public:
	AerialGeneralSwoopMovement() { m_bDone = false; m_fRandomAugmentAngle = 0.0f; };
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual void NotifyMovementDone() { m_bDone = true; };
private:
	bool m_bDone;
	CPoint m_obSwoopPoint;
	CDirection m_obFromCentreVector;

	CHashedString m_obTurnLeftAnim, m_obTurnRightAnim, m_obSwoopAnim;
	float m_fRandomAugmentAngle;

	float m_fHeightAdjustMinimum, m_fHeightAdjust;

	float m_fToDistanceInFrontOfPlayer;
};

class AerialGeneralMachineGunSpecialAttack : public BossSpecialAttack
{
	HAS_INTERFACE(AerialGeneralMachineGunSpecialAttack)
public:
	AerialGeneralMachineGunSpecialAttack();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual ~AerialGeneralMachineGunSpecialAttack() {};
private:
	bool m_bDone;

	ntstd::Vector<Object_Projectile*> m_apobSwords;
	bool m_bSwordCreatedForThisStrikeWindow;
	bool m_bMovementInBetweenInitialised;
	int m_iRemainingAttacks;	
	bool m_bCatching;
	bool m_bStartedCatchAttack;
	float m_fAttackTimer;
	bool m_bBeginningMovementDone;
	int m_iSwordsThrowSoFarThisAttack;

	// Exposed:
	BossMovement* m_pobBeginningMovement;
	CAttackLink* m_pobThrowAttack;
	BossMovement* m_pobMovementInBetween;
	CAttackLink* m_pobCatchAttack;
	Projectile_Attributes* m_pobProjectileAttributes;
	float m_fStartCatchDistance;
	float m_fSwordDestroyDistance;

	float m_fTimeTillSwordReclaim;
	float m_fTimeBetweenAttacks;
	int m_iNumAttacksToDo;
	int m_iNumAttacksToDoAdjust;
	CDirection m_obDownishVector;
	int m_iNumberOfThrowsTillOnTarget;
};

//------------------------------------------------------------------------------------------
//!
//!	AerialGeneralTeleportingMeleeAttack
//!
//------------------------------------------------------------------------------------------
class AerialGeneralTeleportingMeleeAttack : public BossMeleeAttack
{
	HAS_INTERFACE(AerialGeneralTeleportingMeleeAttack);
public:
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
protected:
	ntstd::String m_obTeleportString;
};

class AerialGeneralDoppelgangerSpawningBossAttackPhase : public BossAttackPhase
{
	HAS_INTERFACE( AerialGeneralDoppelgangerSpawningBossAttackPhase );
public:
	virtual void NotifyGotStruck(Boss* pobBoss);
private:
	
};

class AerialGeneralDoppelgangerSpawningStartTransition : public BossAttackPhaseStartTransition
{
	HAS_INTERFACE( AerialGeneralDoppelgangerSpawningStartTransition );
public:
	virtual void BeginStartTransition(Boss* pobBoss, CEntity* pobPlayer);
	virtual bool Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
private:
	int m_iNumToWakeUp;
};

//------------------------------------------------------------------------------------------
//!
//!	AerialGeneralPlayerRelativeVectorMovement
//! Class for moving an AGen relative to a player, will obey DoppelgangerManager formations
//!
//------------------------------------------------------------------------------------------
class AerialGeneralPlayerRelativeVectorMovement : public BossWalkingMovement
{
	HAS_INTERFACE( AerialGeneralPlayerRelativeVectorMovement );
public:
	AerialGeneralPlayerRelativeVectorMovement() { m_obVector = CDirection( CONSTRUCT_CLEAR ); m_fStopDistance = 2.0f; };
	virtual BossMovementType GetMovementType() { return BMT_MOVING; };
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~AerialGeneralPlayerRelativeVectorMovement() {};
protected:
	CDirection m_obVector; // 0,0,1 is towards player
	float m_fStopDistance;
	float m_fDistanceToFormAroundPlayer;
	float m_fFormationTolerance;
};

LV_DECLARE_USERDATA(DoppelgangerManager);

#endif
