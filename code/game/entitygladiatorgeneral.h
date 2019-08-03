#ifndef	_ENTITY_GGEN_H
#define	_ENTITY_GGEN_H

#include "entityboss.h"

#define GGEN_NUM_DEFENSIVE_SPIN_TARGET_POINTS 4
#define GGEN_NUM_PILLARS 5
#define GGEN_ATTACK_TIMEOUT_SAFETY //Uncomment this to add a time-out based safety net to the attacks (in-case certain messages are never recieved)

class GladiatorGeneral : public Boss
{
	HAS_INTERFACE(GladiatorGeneral);
public:
	GladiatorGeneral();
	~GladiatorGeneral();
	void OnPostPostConstruct();
	bool CanStartAnAttack();
	bool CanStartALinkedAttack();
	void UpdateBossSpecifics(float fTimeDelta);
	void DebugRenderBossSpecifics( CPoint& obScreenLocation, float fXOffset, float fYOffset ) { UNUSED(obScreenLocation); UNUSED(fXOffset); UNUSED(fYOffset); };
	void BreakShell_1();
	void BreakShell_2();
	void BreakShell_3();
	int GetSmashAttackPhase() { return m_iSmashAttackPhaseNumber; }
	void SetShellbreakVulnerability(bool bVulnerableToShellBreak);
	void StopNotificationWithPillars();
	void NotifyJustFinishedStomp() { m_bJustFinishedStomp = true; }
	bool IsInSpecialAttack() { return m_bInSpecialAttack; }
	void SetInSpecialAttack(bool bInSpecialAttack) { m_bInSpecialAttack = bInSpecialAttack; }
	void SetHitWhileStunned(bool bIsHitWhileStunned) { m_bHitWhileStunned = bIsHitWhileStunned; }
	bool IsHitWhileStunned() { return m_bHitWhileStunned; }
	bool NeedsHitWhileStunnedNotification() { return m_bNotifyIfHitStunned; }
	
private:
	void CreateShellPiece(const char* pcName, const char* pcClump, const char* pcTransformName, bool bThrowable, bool bVisible, const CPoint &obOffset);
	void DetachShellPiece(const char* pcName, float fXDir, float fYDir, float fZDir, float fSpeed, const char* pcShow = NULL, const char* pcDetach = NULL);
	void CreateVulnerabilityParticleEmitter();
	void ShowVulnerabilityZoneMarker(bool bShow);
	CPoint m_aobDefensiveSpinTargetPoints[GGEN_NUM_DEFENSIVE_SPIN_TARGET_POINTS];	//Points are accessed directly by defensive spin.
	ntstd::List<CEntity*> m_obPillars;
	ntstd::List<CEntity*> m_obShellPieces;
	int m_iMinHealthTillShellBreak1;
	int m_iMinHealthTillShellBreak2;
	int m_iMinHealthTillShellBreak3;
	int m_iSmashAttackPhaseNumber;
	bool m_bVulnerableToShellBreakSpecials;
	bool m_bJustFinishedStomp;
	bool m_bInSpecialAttack;
	CHashedString m_aobPillarNames[GGEN_NUM_PILLARS];
	bool m_bHitWhileStunned;
	bool m_bNotifyIfHitStunned;	//This flag determines whether or not we need to be notified of a hit on the king (set to true while stunned).
	bool m_bEmitterIsActive;

	CEntity* m_pobParticleEmitterEntity;	//A little emitter for showing the player where to go when the GGen is vulnerable to shell-break.

	friend class GladiatorGeneralRollSpecial;
	friend class GladiatorGeneralDefensiveSpinSpecial;
	friend class GladiatorGeneralSmashAttackSpecial;
	friend class GGenSmashAttackSelector;
	friend class GladiatorGeneralStompSpecial;
};

class GladiatorGeneralRollSpecial : public BossSpecialAttack
{
	HAS_INTERFACE(GladiatorGeneralRollSpecial);
public:
	GladiatorGeneralRollSpecial();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual void NotifyInteractionWith(CEntity* pobEntity);
	virtual BossAttack* NotifyAttackInterrupted();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual bool IsVulnerableTo(CStrike* pobStrike);
	virtual bool IsVulnerableTo(const CAttackData* pobAttackData);
	virtual ~GladiatorGeneralRollSpecial() {};
private:
	int m_iRollsRemaining;
	CEntity* m_pobInteractionWith;
	bool m_bHitPillar;
	CPoint m_obHitPillarPosition;
	bool m_bHeadingTowardsPillar;
	bool m_bAttackEndTriggered;
	const Boss* m_pobBoss;

#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	float m_fAttackTimeoutTimer;
#endif

	// Exposed:
	float m_fPillarGlanceAngle;
	int m_iMaxNumberOfTrackingRolls;
	int m_iMaxNumberOfTrackingRollsAdjust;
	const CAttackLink* m_pobRollStart; // Going into roll from fight pose
	const CAttackLink* m_pobRollTrack; // Loop this rand times folowing player then do the attack
	const CAttackLink* m_pobRollAttack; // ... the attack
	const CAttackLink* m_pobRollHitPillar; // Do this if we hit a pillar
	const CAttackLink* m_pobRollGlancePillarLeft;	//Do this if we're at an angle where we'd glance off to the left.
	const CAttackLink* m_pobRollGlancePillarRight;	//Do this if we're at an angle where we'd glance off to the right.
	const CAttackLink* m_pobFromStunnedRecoil;		//Do this if we're hit while stunned from hitting the pillar.
	float m_fDamageOnHitPillar;						//How much damage do we want the GGen to take upon hitting a pillar?
	float m_fDamageOnGlanceOffPillar;				//How much damage do we want the GGen to take if he glances off the side of a pillar?
};

class GladiatorGeneralDefensiveSpinSpecial : public BossSpecialAttack
{
	HAS_INTERFACE(GladiatorGeneralDefensiveSpinSpecial);
public:
	GladiatorGeneralDefensiveSpinSpecial();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual BossAttack* NotifyAttackInterrupted();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual ~GladiatorGeneralDefensiveSpinSpecial() {};
private:
	int m_iRollsRemaining;
	int m_iCurrentChosenPoint;
	bool m_bLeaping;
	bool m_bAttackEndTriggered;
	Boss* m_pobBoss;

#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	float m_fAttackTimeoutTimer;
#endif

	// Exposed:
	int m_iMaxNumberOfRolls;
	int m_iMaxNumberOfRollsAdjust;
	const CAttackLink* m_pobRollStart;
	const CAttackLink* m_pobRoll;
	const CAttackLink* m_pobRollLeap;
};

class GladiatorGeneralStompSpecial : public BossSpecialAttack
{
	HAS_INTERFACE(GladiatorGeneralStompSpecial);
public:
	GladiatorGeneralStompSpecial();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual ~GladiatorGeneralStompSpecial() {};
private:
	int m_iStompsRemaining;
	int m_iLastUsedStompIndex;

#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	float m_fAttackTimeoutTimer;
#endif

	// Exposed:
	int m_iMaxNumberOfStomps;
	int m_iMaxNumberOfStompsAdjust;
	const CAttackLink* m_pobStomp[4];
	float m_fMinStompedNearDistance;
	float m_fMaxStompedNearDistance;
	CHashedString m_obStompedNearPlayerAttackData;
};


//------------------------------------------------------------------------------------------
//!
//!	GGenSmashAttackSelector
//! Class for selecting the gladiator general's smash-attack.
//!
//------------------------------------------------------------------------------------------
class GGenSmashAttackSelector : public BossAttackSelector
{
	HAS_INTERFACE(GGenSmashAttackSelector)
public:
	GGenSmashAttackSelector();
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_GGENSMASH; }
private:
	float m_fMinDistance;
	float m_fMaxDistance;
	float m_fMinSafeDistance;
};


class GladiatorGeneralSmashAttackSpecial : public BossSpecialAttack
{
	HAS_INTERFACE(GladiatorGeneralSmashAttackSpecial);
public:
	GladiatorGeneralSmashAttackSpecial();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* NotifyAttackInterrupted();
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual ~GladiatorGeneralSmashAttackSpecial() {};
	virtual bool IsVulnerableTo(CStrike* pobStrike);
	virtual bool IsVulnerableTo(const CAttackData* pobAttackData);
	virtual void NotifyIsInFailedStrikeRecovery(bool bFailedStrike);
private:
	const Boss* m_pobBoss;
	const CAttackLink* m_pobSmashAttack1;
	const CAttackLink* m_pobSmashAttack2;
	const CAttackLink* m_pobSmashAttack3;
	bool m_bFinished;
	bool m_bVulnerableTo;
};

#endif	//_ENTITY_GGEN_H
