#ifndef	_ENTITY_DEMON_H
#define	_ENTITY_DEMON_H

#include "entityboss.h"

#define DO_DEMON_EFFECT	//Used to toggle demon particle effects (for dev purposes and until we have a better effect!)
#define DEMON_TRANSITION_BACKUP_HAX	//Used to toggle on/off the "horrible" hack that forces the end of the transitions after it's been too long.
//#define DEMON_CAN_DETACH	//Old code where the demon used to be able to detach and turn into a cloud-form. Uncomment this to re-enable.

#define NUM_DEMON_NODES 73
#define NUM_TENTACLE_NODES 12


//Define our demon-tentacle as a new entity type (stored here because it won't be used anywhere else!)
class DemonTentacle : public CEntity
{
	HAS_INTERFACE(DemonTentacle);
public:
	DemonTentacle();
	~DemonTentacle();

	// Post Construct
	void OnPostConstruct();

	void PlayTentacleAnim(CHashedString obAnimName, bool bHideWhenAnimDone, bool bIsStrikeAnim);
	void ToggleTentacleEffect(bool bEffectOn);

	bool m_bHideOnAnimDone;
	bool m_bStrikeAnimComplete;
	bool m_bIsStrikeAnim;	//So we can toggle our bStrikeAnimComplete flag.
protected:
private:
	bool m_bIsEffectOn;	//So that we don't try to turn if on/off twice in a row.
	int m_iNodeEffectID[NUM_DEMON_NODES];
	int m_iTentacleStrikeWarningEffect;

	friend class Demon;
	friend class DemonTentacleAttack;
	friend class DemonDescendAndAttachMovement;
};


class Demon : public Boss
{
	HAS_INTERFACE(Demon);
public:
	Demon();
	~Demon();
	void OnPostPostConstruct();
	bool CanStartAnAttack();
	bool CanStartALinkedAttack();
	void UpdateBossSpecifics(float fTimeDelta);
	void DebugRenderBossSpecifics( CPoint& obScreenLocation, float fXOffset, float fYOffset ) { UNUSED(obScreenLocation); UNUSED(fXOffset); UNUSED(fYOffset); };
	void PlayAnimation(CHashedString obAnimName, bool bLooping = false);
	bool IsAttachedToKing() { return m_bAttachedToKing; }
	bool HasJustAttachedToKing() { return m_bJustReattachedToKing; }
	void SetKingPointer(Boss* pobKing);
	float GetTimeSinceLastAttachStateChange() { return m_fTimeSinceAttachStateChange; }

	//Various effect switches (because one effect doesn't suit all moves).
	//NOTE: The reason I keep these seperate instead of having one 'SetDemonEffect' function with a CHashedString parameter
	//for the particle-def name is that it's quite likely that we'll be doing per-node allocations (or at least per-node-list allocations)
	//which will be different for each effect. So while (for the meantime) it looks like they're all the same with just a different
	//particle-def string, in time this will change and each will be much more specialised.
	void RemoveDemonEffects();						//Clean-up any allocated particle effects.
	void SetDemonEffectStandard();					//For standard movement
	void SetDemonEffectWingAttack();				//For the "windbreaker" attack.
	void SetDemonEffectRetreat();					//Slightly larger effect for larger retreat wings.
	void SetDemonEffectStunned();					//For when the king had all three lightning balls returned.
	void SetDemonEffectCloud();						//For the detached demon in cloud-form.
	void SetDemonNodeEffectCloud(int iNode);		//For changing a single "attack-notify" cloud node back into a cloud.
	void SetDemonNodeEffectCloudAttack(int iNode);	//For changing a single cloud node into an "attack-notify" cloud node.
	bool IsInMoveTransition() { return m_bInMoveTransition; }
	Boss* GetKing() { return m_pobKing; }
	DemonTentacle* GetTentacle() { return m_pobTentacle; }

	void DetachFromKingForNS();
	void AttachToKingAfterNS();
private:
	Boss* m_pobKing;
	int m_iNodeEffectID[NUM_DEMON_NODES];
	float m_fTimeSinceAttachStateChange;
	bool m_bAttachedToKing;
	bool m_bJustReattachedToKing;
	bool m_bInMoveTransition;
	DemonTentacle* m_pobTentacle;

	friend class DemonDetachAndAscendMovement;
	friend class DemonDescendAndAttachMovement;
	friend class DemonAttackPhaseMovement;
};


class DemonDetachAndAscendMovement : public BossTransitioningMovement
{
	HAS_INTERFACE( DemonDetachAndAscendMovement );
public:
	DemonDetachAndAscendMovement();
	virtual BossMovementType GetMovementType() { return BMT_TRANSITIONING; }
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone() { m_bDone = true; }
	virtual ~DemonDetachAndAscendMovement() {};
protected:

	Boss* m_pobBoss;
	bool m_bDone;

#ifdef DEMON_TRANSITION_BACKUP_HAX
	float m_fTimeInMovement;
#endif

	//Exposed
	CHashedString m_obAscendStartAnim;
	CHashedString m_obAscendCycleAnim;
	CHashedString m_obAscendStopAnim;
};


class DemonDescendAndAttachMovement : public BossTransitioningMovement
{
	HAS_INTERFACE( DemonDescendAndAttachMovement );
public:
	DemonDescendAndAttachMovement();
	virtual BossMovementType GetMovementType() { return BMT_TRANSITIONING; }
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone();
	virtual ~DemonDescendAndAttachMovement() {};
protected:

	Boss* m_pobBoss;
	bool m_bDone;
	bool m_bStartingAttach;

#ifdef DEMON_TRANSITION_BACKUP_HAX
	float m_fTimeInMovement;
#endif

	//Exposed
	CHashedString m_obDescendStartAnim;
	CHashedString m_obDescendCycleAnim;
	CHashedString m_obDescendStopAnim;
};


class DemonAttackPhaseMovement : public BossTransitioningMovement
{
	HAS_INTERFACE( DemonAttackPhaseMovement );
public:
	DemonAttackPhaseMovement();
	virtual BossMovementType GetMovementType() { return BMT_TRANSITIONING; }
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone();
	virtual ~DemonAttackPhaseMovement() {};
protected:

	Boss* m_pobBoss;
	bool m_bDone;

#ifdef DEMON_TRANSITION_BACKUP_HAX
	float m_fTimeInMovement;
#endif

	//Exposed
	CHashedString m_obMoveStartAnim;
	CHashedString m_obMoveCycleAnim;
	CHashedString m_obMoveStopAnim;
};


class DemonTentacleAttackSelector : public BossAttackSelector
{
	HAS_INTERFACE(DemonTentacleAttackSelector)
public:
	DemonTentacleAttackSelector();
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_DEMONTENTACLE; }
private:
	float m_fMinDistance;
	float m_fMaxDistance;
};


class DemonTentacleAttack : public BossSpecialAttack
{
	HAS_INTERFACE(DemonTentacleAttack);
public:
	DemonTentacleAttack();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual ~DemonTentacleAttack() {};
private:
	Boss* m_pobBoss;
	float m_fStrikeTimerCounter;
	float m_fStrikeGenerateHitCounter;
	float m_fStrikeOverTimerCounter;
	float m_fStrikeNodeCalculateTimer;
	float m_fStrikeNodeCalculateDelay;	//So we don't process the distance from 73 nodes every frame.
	int m_iStrikeNode;
	bool m_bStrikeStarted;
	bool m_bStrikeCompleted;

	//Exposed.
	CHashedString m_obTentacleStrikeData;
	CHashedString m_obTentacleStrikeAnim;
	CHashedString m_obTentacleStrike2fsAnim;
	CHashedString m_obStrikeImpactEffect[3];
	float m_fTimeDelayTillStrike;
	float m_fTimeDelayTillGenerateStrikeTimeout;
	float m_fTimeDelayTillStrikeOver;
	float m_fStrikeRadius;
};


#endif	//_ENTITY_DEMON_H
