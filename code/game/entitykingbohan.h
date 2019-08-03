#ifndef	_ENTITY_KINGBOHAN_H
#define	_ENTITY_KINGBOHAN_H

#include "entityboss.h"
#include "entityprojectile.h"

#define KING_NUM_RETREAT_POINTS 5
#define KING_NUM_LIGHTNINGBOLTS_1 3
#define KING_NUM_LIGHTNINGBOLTS_2 6
#define KING_NUM_LIGHTNINGBOLTS_3_LIGHT 3
#define KING_NUM_LIGHTNINGBOLTS_3_HEAVY 6
#define KING_STORE_CURRENT_SPECIAL_NAME				//Uncomment this to keep storage of which special we're in (for debug-purposes).

class KingBohan : public Boss
{
	HAS_INTERFACE(KingBohan);
public:
	KingBohan();
	~KingBohan();
	void OnPostPostConstruct();
	bool CanStartAnAttack();
	bool CanStartALinkedAttack();
	void UpdateBossSpecifics(float fTimeDelta);
	void DebugRenderBossSpecifics( CPoint& obScreenLocation, float fXOffset, float fYOffset );
	Boss* GetDemon() const { return m_pobDemon; }
	void SetHitWhileStunned(bool bIsHitWhileStunned) { m_bHitWhileStunned = bIsHitWhileStunned; }
	bool IsHitWhileStunned() const { return m_bHitWhileStunned; }
	bool NeedsHitWhileStunnedNotification() const { return m_bNotifyIfHitStunned; }
	bool IsInSpecialAttack() const { return m_bInSpecialAttack; }
	void SetInSpecialAttack(bool bInSpecialAttack, const char* pcAttackName);
	bool HasAttachedDemon();
	bool IsPlayerInRetreatPointRadius() { return m_bPlayerInRetreatPointRadiusRange; }
#ifdef KING_STORE_CURRENT_SPECIAL_NAME
	const char* GetCurrentSpecialName() { return m_obInSpecialName.c_str(); }
#else
	const char* GetCurrentSpecialName() { return "NotStoredInThisBuild (See defines in header)"; }
#endif

	virtual void GoToPhase(unsigned int iPhaseNumber);
private:
	Boss* m_pobDemon;
	CPoint m_aobRetreatPoints[KING_NUM_RETREAT_POINTS];
	float m_aobRetreatPointDistances[KING_NUM_RETREAT_POINTS];
	int m_aobRetreatHealths[KING_NUM_RETREAT_POINTS];
	int m_iCurrentRetreatPoint;
	bool m_bTooFarFromCurrentRetreatPoint;
	bool m_bRetreatingOnHealthBasis;
	bool m_bInSpecialAttack;
	bool m_bPlayerInRetreatPointRadiusRange;
	bool m_bHitWhileStunned;
	bool m_bNotifyIfHitStunned;	//This flag determines whether or not we need to be notified of a hit on the king (set to true while stunned).

#ifdef KING_STORE_CURRENT_SPECIAL_NAME
	ntstd::String m_obInSpecialName;	//This is just for my debugging purposes and needn't be in the final thing.
#endif

	friend class RandomKingMovementSelector;
	friend class KingRetreatMovementSelector;
	friend class RandomKingAttackSelector;
	friend class DistanceSuitabilityKingAttackSelector;
	friend class KingDetachDemonAttackSelector;
	friend class KingRetreatToPointMovement;
	friend class KingWingAttack;
	friend class KingLightningAttack1;
	friend class KingLightningAttack2;
	friend class KingLightningAttack3;
};


//==================== TRANSITIONING MOVEMENTS ===================

//------------------------------------------------------------------------------------------
//!
//!	KingRetreatToPointMovement
//! Class for moving the king in a retreat through certain points (to lead the battle).
//!
//------------------------------------------------------------------------------------------
class KingRetreatToPointMovement : public BossTransitioningMovement
{
	HAS_INTERFACE( KingRetreatToPointMovement );
public:
	KingRetreatToPointMovement();
	virtual BossMovementType GetMovementType() { return BMT_TRANSITIONING; };
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone();
	virtual ~KingRetreatToPointMovement() {};
protected:
	Boss* m_pobBoss;
	bool m_bDone;
	bool m_bReachedTarget;

	//Exposed:
	CHashedString m_obRetreatStartAnim;
	CHashedString m_obRetreatCycleAnim;
	CHashedString m_obRetreatStopAnim;
};


//==================== MOVEMENT SELECTORS ===================

class KingRetreatMovementSelector : public BossMovementSelector
{
	HAS_INTERFACE(KingRetreatMovementSelector);
public:
	KingRetreatMovementSelector();
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* GetSelectedMovement();
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~KingRetreatMovementSelector() {};
private:
	Boss* m_pobBoss;
	//Exposed.
	bool m_bIsAttachedToDemon;
};


class KingTauntMovementSelector : public BossTauntMovementSelector
{
	HAS_INTERFACE(KingTauntMovementSelector);
public:
	KingTauntMovementSelector();
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);	//Different priority checks (includes demon-factor).
	virtual ~KingTauntMovementSelector() {};
private:
	bool m_bAttachedToDemon;		//Only return this selector as max-priority if the current attached-state matches this.
};


class KingMovementSelector : public BossMovementSelector
{
	HAS_INTERFACE(KingMovementSelector)
public:
	KingMovementSelector() : BossMovementSelector() { m_bIsAttachedToDemon = false; };
protected:
	bool m_bIsAttachedToDemon;
	bool m_bOnlyIfPlayerIsInRangeOfRetreat;
};


class RandomKingMovementSelector : public KingMovementSelector
{
	HAS_INTERFACE( RandomKingMovementSelector );
public:
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* GetSelectedMovement();
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
private:
	int m_iSelectedMovement;
};


//==================== ATTACK SELECTORS ===================

class KingAttackSelector : public BossAttackSelector
{
	HAS_INTERFACE(KingAttackSelector)
public:
	KingAttackSelector() : BossAttackSelector() { m_bIsAttachedToDemon = false; };
protected:
	bool m_bIsAttachedToDemon;
	bool m_bOnlyIfPlayerIsInRangeOfRetreat;
	bool m_bOnlyIfPlayerIsOutsideRangeOfRetreat;
};


class RandomKingAttackSelector : public KingAttackSelector
{
	HAS_INTERFACE(RandomKingAttackSelector)
public:
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_RANDOM; }
};


class DistanceSuitabilityKingAttackSelector : public KingAttackSelector
{
	HAS_INTERFACE(DistanceSuitabilityKingAttackSelector)
public:
	DistanceSuitabilityKingAttackSelector() { m_fTimeSinceLastSelection = 0.0f; m_fTimeBeforeRepeatingSelection = 0.0f; }
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_DISTANCE; }

private:
	BossAttack *m_pobLastSelection, *m_pobSelectedAttack;
	float m_fTimeSinceLastSelection;
	//Exposed.
	float m_fTimeBeforeRepeatingSelection;
};


class KingDetachDemonAttackSelector : public KingAttackSelector
{
	HAS_INTERFACE(KingDetachDemonAttackSelector)
public:
	KingDetachDemonAttackSelector();
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_KINGDEMONDETACH; }
private:
	bool m_bRandomTimeGenerated;
	float m_fTimeTillDetach;

	//Exposed.
	float m_fMinTime;
	float m_fMaxTime;
};


class KingAttachToDemonAttackSelector : public KingAttackSelector
{
	HAS_INTERFACE(KingAttachToDemonAttackSelector)
public:
	KingAttachToDemonAttackSelector();
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_KINGDEMONATTACH; }
private:
	bool m_bRandomTimeGenerated;
	float m_fTimeTillAttach;

	//Exposed.
	float m_fMinTime;
	float m_fMaxTime;
};


//==================== SELECTOR CONDITIONS (for generic attack/movement selectors) ===================

class GeneralBossAttackSelectorKingHasDemonAttached : public GeneralBossAttackSelectorCondition
{
	HAS_INTERFACE(GeneralBossAttackSelectorKingHasDemonAttached);
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossAttackSelectorKingHasDemonAttached() {};
private:
	bool m_bDesiredAttachedState;	//So this can be selected only if attached, or only if not attached.
};

class GeneralBossMovementSelectorKingHasDemonAttached : public GeneralBossMovementSelectorCondition
{
	HAS_INTERFACE( GeneralBossMovementSelectorKingHasDemonAttached );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossMovementSelectorKingHasDemonAttached() {};
private:
	bool m_bDesiredAttachedState;	//So this can be selected only if attached, or only if not attached.
};


class GeneralBossAttackSelectorKingPlayerInRetreatRadius : public GeneralBossAttackSelectorCondition
{
	HAS_INTERFACE(GeneralBossAttackSelectorKingPlayerInRetreatRadius);
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossAttackSelectorKingPlayerInRetreatRadius() {};
private:
	bool m_bDesiredInRadiusState;	//So this can be selected only if the player is inside or outside of the retreat radius.
};


class GeneralBossMovementSelectorKingPlayerInRetreatRadius : public GeneralBossMovementSelectorCondition
{
	HAS_INTERFACE(GeneralBossMovementSelectorKingPlayerInRetreatRadius);
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossMovementSelectorKingPlayerInRetreatRadius() {};
private:
	bool m_bDesiredInRadiusState;	//So this can be selected only if the player is inside or outside of the retreat radius.
};


//==================== SPECIAL ATTACKS ===================


class KingWingAttack : public BossSpecialAttack
{
	HAS_INTERFACE(KingWingAttack);
public:
	KingWingAttack();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);

	virtual ~KingWingAttack() {};
private:
	Boss* m_pobBoss;
	float m_fCrowSpawnDelayCounter;
	int m_iWavesRemaining;
	int m_iLinksProcessed;
	bool m_bFlaggedFinished;
	bool m_bSpawnCrows;

	//Exposed
	float m_fCrowSpawnXRange;
	float m_fCrowSpawnYRange;
	float m_fCrowSpawnZRange;
	float m_fCrowSpawnDelay;
	int m_iMaxNumberOfWaves;
	int m_iMaxNumberOfWavesAdjust;
	int m_iMaxCrowsPerSpawn;
	int m_iMinCrowsPerSpawn;
	
	const CAttackLink* m_pobAttackStart;	//Going to attack from fight pose.
	const CAttackLink* m_pobAttackLoop;		//The continuous flapping of the wings.
	const CAttackLink* m_pobAttackEnd;		//When we're done, use this to get back, then 2fs to normality.

	Projectile_Attributes* m_pobProjectileAttributes; //Projectile-Attributes for the crow.
};


class KingLightningAttack1 : public BossSpecialAttack
{
	HAS_INTERFACE(KingLightningAttack1);
public:
	KingLightningAttack1();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual BossAttack* NotifyAttackInterrupted();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual bool IsVulnerableTo(CStrike* pobStrike);
	virtual bool IsVulnerableTo(const CAttackData* pobAttackData);
	virtual ~KingLightningAttack1() {};
private:
	Object_Projectile* m_pobLightningBalls[KING_NUM_LIGHTNINGBOLTS_1];
	CHashedString m_obProjectileNames[KING_NUM_LIGHTNINGBOLTS_1];
	Boss* m_pobBoss;
	float m_fBoltSpawnDelayCounter;
	float m_fTimeIdleInAir;
	int m_iBoltsFired;
	int m_iBoltsDeflected;
	int m_iDazedLoopCount;
	bool m_bFlaggedFinished;
	bool m_bReadyToFire;
	bool m_bRecoverySet;
	bool m_bStunned;

	//Exposed.
	float m_fTimeDelayOnBoltSpawn[KING_NUM_LIGHTNINGBOLTS_1];
	float m_fMaxTimeLightningIdle;
	int m_iNumDazedLoops;
	float m_fDamagePerReturnedBolt;

	const CAttackLink* m_pobAttackStart;			//Going to attack from fight pose.
	const CAttackLink* m_pobAttackFire;				//One shot fired each time it loops.
	const CAttackLink* m_pobAttackEnd;				//When we're done, use this to get back, then to 2fs to normality.
	const CAttackLink* m_pobBoltsReturnedAndHit;	//If the player returns all three bolts.
	const CAttackLink* m_pobDazedRecovery;			//After dazed-cycle has cycled enough, force into this.
	const CAttackLink* m_pobDazedRecoil;			//When hit while dazed.
	CHashedString m_obDeflectEffects[KING_NUM_LIGHTNINGBOLTS_1 * 3];

	Projectile_Attributes* m_pobProjectileAttributes;	//Projectile-Attributes for the lightning balls it'll fire.
};


class KingLightningAttack2 : public BossSpecialAttack
{
	HAS_INTERFACE(KingLightningAttack2);
public:
	KingLightningAttack2();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual BossAttack* NotifyAttackInterrupted();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual bool IsVulnerableTo(CStrike* pobStrike);
	virtual bool IsVulnerableTo(const CAttackData* pobAttackData);
	virtual ~KingLightningAttack2() {};
private:
	Boss* m_pobBoss;
	float m_fBoltSpawnDelayCounter;
	float m_fBoltFireDelayCounter;
	float m_fTimeIdleInAir;
	int m_iBoltsSpawned;
	int m_iBoltsFired;
	int m_iBoltsDeflected;
	int m_iDazedLoopCount;
	bool m_bFlaggedFinished;
	bool m_bReadyToFire;
	bool m_bRecoverySet;
	bool m_bStunned;

	//Exposed.
	float m_fTimeDelayOnBoltSpawn[KING_NUM_LIGHTNINGBOLTS_2];
	float m_fMaxTimeLightningIdle;
	int m_iNumDazedLoops;
	float m_fDamagePerReturnedBolt;

	const CAttackLink* m_pobAttackStart;			//Going to attack from fight pose.
	const CAttackLink* m_pobAttackFire;				//One shot fired each time it loops.
	const CAttackLink* m_pobAttackEnd;				//When we're done, use this to get back, then to 2fs to normality.
	const CAttackLink* m_pobBoltsReturnedAndHit;	//If the player returns all three bolts.
	const CAttackLink* m_pobDazedRecovery;			//After dazed-cycle has cycled enough, force into this.
	const CAttackLink* m_pobDazedRecoil;			//When hit while dazed.
	const CAttackLink* m_pobHitInAirRecoil;			//When hit by a lightning ball but not the final one (so not going into stunned state)

	Projectile_Attributes* m_pobProjectileAttributes;	//Projectile-Attributes for the lightning balls it'll fire (unblockable).
	Object_Projectile* m_pobLightningBalls[KING_NUM_LIGHTNINGBOLTS_2];
	CHashedString m_obProjectileNames[KING_NUM_LIGHTNINGBOLTS_2];
	CHashedString m_obDeflectEffects[KING_NUM_LIGHTNINGBOLTS_2 * 3];	//Up to three effects per hit.
};


class KingLightningAttack3 : public BossSpecialAttack
{
	HAS_INTERFACE(KingLightningAttack3);
public:
	KingLightningAttack3();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual BossAttack* NotifyAttackInterrupted();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual bool IsVulnerableTo(CStrike* pobStrike);
	virtual bool IsVulnerableTo(const CAttackData* pobAttackData);
	virtual ~KingLightningAttack3() {};
private:
	ntstd::Vector<Object_Projectile*> m_vpobProjectilesToFire;	//This vector is used to randomly fire off projectiles.
	Boss* m_pobBoss;
	float m_fBoltSpawnDelayCounter;
	float m_fBoltFireDelayCounter;
	float m_fTimeIdleInAir;
	float m_fTimeToFireNextShot;
	int m_iBoltsSpawned;
	int m_iBoltsFired;
	int m_iBoltsDeflected;
	int m_iDazedLoopCount;
	bool m_bFlaggedFinished;
	bool m_bReadyToFire;
	bool m_bRecoverySet;
	bool m_bStunned;

	//Exposed.
	float m_fTimeDelayOnBoltSpawn[KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY];
	float m_fMaxTimeLightningIdle;
	int m_iNumDazedLoops;
	float m_fDamagePerReturnedBolt;
	float m_fMinTimeBetweenShots;
	float m_fMaxTimeBetweenShots;

	const CAttackLink* m_pobAttackStart;			//Going to attack from fight pose.
	const CAttackLink* m_pobAttackFire;				//One shot fired each time it loops.
	const CAttackLink* m_pobAttackEnd;				//When we're done, use this to get back, then to 2fs to normality.
	const CAttackLink* m_pobBoltsReturnedAndHit;	//If the player returns all three bolts.
	const CAttackLink* m_pobDazedRecovery;			//After dazed-cycle has cycled enough, force into this.
	const CAttackLink* m_pobDazedRecoil;			//When hit while dazed.
	const CAttackLink* m_pobHitInAirRecoil;			//When hit by a lightning ball but not the final one (so not going into stunned state)

	Projectile_Attributes* m_pobProjectileAttributes[2];	//Projectile-Attributes for the lightning balls it'll fire (both small and large).
	Object_Projectile* m_pobLightningBalls[KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY];
	CHashedString m_obProjectileNames[KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY];
	CHashedString m_obDeflectEffects[3 * 3];		//Up to three effects per hit, with 3 different hit levels.
};


class KingSwoopAttack : public BossSpecialAttack
{
	HAS_INTERFACE(KingSwoopAttack);
public:
	KingSwoopAttack();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual BossAttack* NotifyAttackInterrupted();
	virtual ~KingSwoopAttack() {};
private:
	Boss* m_pobBoss;
	bool m_bTrackPlayer;

	//Exposed
	const CAttackLink* m_pobAttack;			//Into links automatically to middle, end, and then 2fs through attack-data.
	float m_fTrackTillDistance;				//Stay tracking the player up until below this distance.
};



//Detach from demon "attack" for allowing the demon to detach and go about it's business.
class KingDetachFromDemon : public BossSpecialAttack
{
	HAS_INTERFACE(KingDetachFromDemon);
public:
	KingDetachFromDemon();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual BossAttack* NotifyAttackInterrupted();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual ~KingDetachFromDemon() {};

private:
	Boss* m_pobBoss;
	bool m_bFlaggedFinished;

	//Exposed.
	const CAttackLink* m_pobStart;
	const CAttackLink* m_pobLoop;
	const CAttackLink* m_pobEnd;
};


//Attach to demon "attack" for allowing the demon to return, attach, and re-synchronise with the king.
class KingAttachToDemon : public BossSpecialAttack
{
	HAS_INTERFACE(KingAttachToDemon);
public:
	KingAttachToDemon();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual BossAttack* NotifyAttackInterrupted();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual ~KingAttachToDemon() {};

private:
	Boss* m_pobBoss;
	bool m_bFlaggedFinished;

	//Exposed.
	const CAttackLink* m_pobStart;
	const CAttackLink* m_pobLoop;
	const CAttackLink* m_pobEnd;
};


#endif	//_ENTITY_KINGBOHAN_H
