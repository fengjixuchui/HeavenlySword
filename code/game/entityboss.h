#ifndef	_ENTITY_BOSS_H
#define	_ENTITY_BOSS_H

#define DEBUG_HORRIBLE_REACTSTATE_TIMEOUT	//Uncomment to allow 20-second no-hit react-state timeout (auto-recovers).

#include "entitycharacter.h"
#include "movement.h"
#include "eventlog.h"
#include "editable/enumlist.h"

class CStrike;
class Boss;
class Player;
class CAttackLink;
class CAttackData;
class BossAttackPhase;
class BossStateSwitchParameters;
class CombatPhysicsStrikeVolume;

#define DEBUG_SHIFT_AMOUNT 15.0f

#define BOSS_RAND_F drandf
#define BOSS_RAND drand

class BossSimpleNavigationAvoidanceArea
{
	HAS_INTERFACE(BossSimpleNavigationAvoidanceArea);
public:
	BossSimpleNavigationAvoidanceArea() 
		: m_obCentre( CONSTRUCT_CLEAR ),
		m_fRadius( 1.0f )
	{}

	CPoint m_obCentre;
	float m_fRadius;
};

class BossSimpleNavigationManager
{
public:
	BossSimpleNavigationManager(ntstd::List<BossSimpleNavigationAvoidanceArea*>& obAvoidanceAreas);
	bool IsPointInAvoidanceArea(CPoint& obPoint);
	bool AugmentPath(CPoint* paobPoints, int iNumPoints); // Go through all points making sure none are in avoidance areas
	bool AugmentVector(const CPoint& obCurrentPosition, CDirection& obTravelVector, float& fSpeed, float fLookAhead, bool bChangeSpeed); // Check to see if vector and look ahead takes us into an avoidance, and augment the vector to avoid
	void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	void SetAvoidanceAreas(ntstd::List<BossSimpleNavigationAvoidanceArea*>& obAvoidanceAreas) { m_obAvoidanceAreas = obAvoidanceAreas; };
	~BossSimpleNavigationManager();
private:
	ntstd::List<BossSimpleNavigationAvoidanceArea*>& m_obAvoidanceAreas;
};

//------------------------------------------------------------------------------------------
//!
//!	BossMovement
//! Pseudo-abstract class for moving a boss
//!
//------------------------------------------------------------------------------------------
class BossMovement
{
	HAS_INTERFACE( BossMovement );
public:
	enum BossMovementType { BMT_MOVING, BMT_TRANSITIONING, BMT_COUNT };

	BossMovement() { m_fMovementProbability = 1.0f; m_fSpeed = 1.0f;  m_fTimeInMovement = 0.0f; m_bCanBeInterruptedToAttack = false; m_bVulnerableDuring = true; };
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); m_fTimeInMovement = 0.0f; return 0; };
	virtual BossMovement* Reinitialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED( pobBoss ); UNUSED( pobPlayer ); ntAssert(0); return 0; };
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) { ntAssert(0); UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); return 0; };
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset) { UNUSED( obScreenLocation ); UNUSED( fXOffset ); UNUSED( fYOffset ); };
	virtual BossMovementType GetMovementType() { return BMT_COUNT; };
	virtual void NotifyMovementDone() {};
	bool GetCanBeInterruptedToAttack() { return m_bCanBeInterruptedToAttack; };
	bool GetVulnerableDuring() { return m_bVulnerableDuring; };
	float GetMovementProbability() { return m_fMovementProbability; };
	virtual ~BossMovement() {};
protected:
	float m_fMovementProbability;

	float m_fTimeInMovement;

	// Exposed:
	float m_fSpeed;
	bool m_bCanBeInterruptedToAttack;
	bool m_bVulnerableDuring;
};

class BossWalkingMovement : public BossMovement
{
	HAS_INTERFACE(BossWalkingMovement)
public:
	BossWalkingMovement() { m_fTimeToDoMovementExclusively = 0.0f; m_fTimeToDoMovementExclusivelyAdjust = 0.0f; m_fTimeToDoMovementExclusivelyThisTime = 0.0f; };
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* Reinitialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
protected:
	float m_fTimeToDoMovementExclusivelyThisTime;
	
	// Exposed:
	CHashedString m_obWalkingController;
	float m_fTimeToDoMovementExclusively, m_fTimeToDoMovementExclusivelyAdjust;
};

class BossTransitioningMovement : public BossMovement
{
	HAS_INTERFACE(BossTransitioningMovement)
public:
	BossTransitioningMovement() : BossMovement() { m_pobAllowEntityCollisionOnCompletion = 0; m_bAlignToPlayerAtEnd = false; };
	virtual BossMovement* Reinitialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);

protected:
	CEntity* m_pobAllowEntityCollisionOnCompletion;
	bool m_bAlignToPlayerAtEnd;
};

//------------------------------------------------------------------------------------------
//!
//!	BossPlayerRelativeVectorMovement
//! Class for moving a boss relative to a player
//!
//------------------------------------------------------------------------------------------
class BossPlayerRelativeVectorMovement : public BossWalkingMovement
{
	HAS_INTERFACE( BossPlayerRelativeVectorMovement );
public:
	BossPlayerRelativeVectorMovement() { m_obVector = CDirection( CONSTRUCT_CLEAR ); m_fStopDistance = 1.0f; };
	virtual BossMovementType GetMovementType() { return BMT_MOVING; };
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~BossPlayerRelativeVectorMovement() {};
protected:
	CDirection m_obVector; // 0,0,1 is towards player
	float m_fStopDistance;
};

//------------------------------------------------------------------------------------------
//!
//!	BossPlayerRelativeTransitioningMovement
//! Class for moving a boss relative to a player
//!
//------------------------------------------------------------------------------------------
class BossPlayerRelativeTransitioningMovement : public BossTransitioningMovement
{
	HAS_INTERFACE( BossPlayerRelativeTransitioningMovement );
public:
	BossPlayerRelativeTransitioningMovement() { m_obVector = CDirection( CONSTRUCT_CLEAR ); m_fCycleTime = 1.0f; m_bDone = false; };
	virtual BossMovementType GetMovementType() { return BMT_TRANSITIONING; };
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone() { m_bDone = true; };
	virtual ~BossPlayerRelativeTransitioningMovement() {};
protected:
	bool m_bDone;
	CDirection m_obMoveDirection;

	// Exposed:
	CDirection m_obVector; // 0,0,1 is towards player	
	CHashedString m_obStartMove, m_obCycleMove, m_obEndMove;
	float m_fCycleTime;
};

class BossTauntMovement : public BossTransitioningMovement
{
	HAS_INTERFACE( BossTauntMovement );
public:
	BossTauntMovement();
	virtual BossMovementType GetMovementType() { return BMT_TRANSITIONING; };
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone() { m_bDone = true; }
	virtual ~BossTauntMovement() {};
protected:
	Boss* m_pobBoss;
	bool m_bDone;

	//Exposed
	CHashedString m_obTauntAnim;
	bool m_bGravity;
	bool m_bUseSimpleTransition;
};


//------------------------------------------------------------------------------------------
//!
//!	BossMovementSelector
//! Pseudo-abstract class for selecting how to move a boss
//!
//------------------------------------------------------------------------------------------
class BossMovementSelector
{
	HAS_INTERFACE( BossMovementSelector );
public:
	BossMovementSelector() { m_fMinPriority = m_fMaxPriority = 0.5f; };
	virtual BossMovement* GetSelectedMovement() { return 0; };
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) { ntAssert(0); UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); return 0.0f; }; // Will return a float (0.0 - 1.0) representing how important it is that this selector select the next attack
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset) { UNUSED( obScreenLocation ); UNUSED( fXOffset ); UNUSED( fYOffset ); };
	BossMovement* ChooseRandomMovementWithWeighting(ntstd::List<BossMovement*>* pobAlternateList = 0);
	int GetNumberOfMovements() { return m_obMovements.size(); };
	virtual ~BossMovementSelector() {};
protected:
	ntstd::List<BossMovement*> m_obMovements;
	float m_fMinPriority, m_fMaxPriority;
};

//------------------------------------------------------------------------------------------
//!
//!	GeneralBossMovementSelector
//! Stuff for a general selector that can be built up using condition objects
//!
//------------------------------------------------------------------------------------------
class GeneralBossMovementSelectorCondition
{
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); ntError(0); };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) { UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); ntError(0); return false; };
	virtual ~GeneralBossMovementSelectorCondition() {};
};

class GeneralBossMovementSelectorPlayerCombatStateCondition : public GeneralBossMovementSelectorCondition
{
	HAS_INTERFACE( GeneralBossMovementSelectorPlayerCombatStateCondition );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossMovementSelectorPlayerCombatStateCondition() {};
private:
	COMBAT_STATE m_eRequiredCombatState;
	bool m_bNegateCheck;
};

class GeneralBossMovementSelectorAGenInAir : public GeneralBossMovementSelectorCondition
{
	HAS_INTERFACE( GeneralBossMovementSelectorAGenInAir );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossMovementSelectorAGenInAir() {};
private:
	bool m_bRequiredInAirState;
};

class GeneralBossMovementSelectorPlayerDistanceCondition : public GeneralBossMovementSelectorCondition
{
	HAS_INTERFACE( GeneralBossMovementSelectorPlayerDistanceCondition );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };;
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossMovementSelectorPlayerDistanceCondition() {};
private:
	float m_fInnerDistance, m_fOuterDistance;
};

class GeneralBossMovementSelectorPlayerTimeSinceLastAttackCondition : public GeneralBossMovementSelectorCondition
{
	HAS_INTERFACE( GeneralBossMovementSelectorPlayerTimeSinceLastAttackCondition );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };;
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossMovementSelectorPlayerTimeSinceLastAttackCondition() {};
private:
	float m_fGracePeriod, m_fGracePeriodAdjust;
};

class GeneralBossMovementSelectorBossTimeSinceLastAttackCondition : public GeneralBossMovementSelectorCondition
{
	HAS_INTERFACE( GeneralBossMovementSelectorBossTimeSinceLastAttackCondition );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };;
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossMovementSelectorBossTimeSinceLastAttackCondition() {};
private:
	float m_fGracePeriod, m_fGracePeriodAdjust;
};

class GeneralBossMovementSelector : public BossMovementSelector
{
	HAS_INTERFACE( GeneralBossMovementSelector );
public:
	GeneralBossMovementSelector() {};
	virtual BossMovement* GetSelectedMovement();
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer); 
private:
	ntstd::List<GeneralBossMovementSelectorCondition*> m_obConditions; // List of conditions
};

//------------------------------------------------------------------------------------------
//!
//!	BossTauntMovementSelector
//!
//------------------------------------------------------------------------------------------
class BossTauntMovementSelector : public BossMovementSelector
{
	HAS_INTERFACE(BossTauntMovementSelector);
public:
	BossTauntMovementSelector();
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* GetSelectedMovement();
	virtual ~BossTauntMovementSelector() {};
private:
protected:
	Boss* m_pobBoss;
	//Exposed.
	float m_fTimeSinceAttackBeforeTaunt;
	float m_fMinDistanceToPlayer;	//Don't taunt if the player is within this range (too likely to be attacked, or should be attacking).
	float m_fMaxDistanceToPlayer;	//Don't taunt if the player is so far away they won't see it!
};

//------------------------------------------------------------------------------------------
//!
//!	SingleBossMovementSelector
//! Class for selecting one boss movement
//!
//------------------------------------------------------------------------------------------
class SingleBossMovementSelector : public BossMovementSelector
{
	HAS_INTERFACE( SingleBossMovementSelector );
public:
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) { UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); return ( m_fMinPriority + m_fMaxPriority ) * 0.5f; };
	
	virtual BossMovement* GetSelectedMovement() 
	{
		if (m_obMovements.size() > 0)
			return (*m_obMovements.begin()); 
		else
			return 0; 
	};
};

//------------------------------------------------------------------------------------------
//!
//!	RandomBossMovementSelector
//! Class for selecting boss movements at random - only good if the movements have some time
//! of exclusivity.
//!
//------------------------------------------------------------------------------------------
class RandomBossMovementSelector : public BossMovementSelector
{
	HAS_INTERFACE( RandomBossMovementSelector );
public:
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) 
	{ 
		UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); 

		return ( m_fMinPriority + m_fMaxPriority ) * 0.5f; 
	};

	virtual BossMovement* GetSelectedMovement() 
	{
		return ChooseRandomMovementWithWeighting(); 
	};
};

//------------------------------------------------------------------------------------------
//!
//!	VulnerableToIncomingAttackBossMovementSelector
//! Class for selecting one boss movement when we're about to be hit
//!
//------------------------------------------------------------------------------------------
class VulnerableToIncomingAttackBossMovementSelector : public BossMovementSelector
{
	HAS_INTERFACE( VulnerableToIncomingAttackBossMovementSelector );
public:
	VulnerableToIncomingAttackBossMovementSelector() { m_fProbabilityOfSelecting = 1.0f; };
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* GetSelectedMovement();
private:
	float m_fProbabilityOfSelecting;
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttack
//! Pseudo-abstract class for attacks
//!
//------------------------------------------------------------------------------------------
class BossAttack
{
	HAS_INTERFACE(BossAttack);
public:
	enum BossAttackType { BAT_MELEE, BAT_SPECIAL, BAT_COUNT };

	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) { ntAssert(0); UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); return false; };
	virtual BossAttack* NotifyAttackInterrupted() { return 0; }
	virtual void NotifyAttackFinished() {};
	virtual void NotifyAttackAutoLinked() {};
	virtual void NotifyAttackStarted() {};
	virtual void NotifyInteractionWith(CEntity* pobEntity) { UNUSED( pobEntity ); };
	virtual void NotifyMovementDone() {};
	virtual void NotifyGotStruck() {};
	virtual void NotifyIsInFailedStrikeRecovery(bool bFailedStrike) { UNUSED(bFailedStrike); };
	virtual void NotifyVulnerableToIncomingStrike(CStrike* pobStrike) { UNUSED(pobStrike); };
	virtual void NotifyInvulnerableToIncomingStrike(CStrike* pobStrike) { UNUSED(pobStrike); };
	virtual void NotifyWillBlockIncomingStrike(CStrike* pobStrike) { UNUSED(pobStrike); };
	virtual void NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol) { UNUSED(pobVol); };
	virtual void NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol) { UNUSED(pobVol); };
	virtual void NotifyProjectileCountered(Object_Projectile* /*pobProj*/) {};
	virtual void NotifyPlayerInteracting(bool /*bState*/) {};
	virtual void NotifyPlayerInteractionAction() { };
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); ntAssert(0); return false; };
	virtual float GetMaxDistance() { return 0.0f; };
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset) { UNUSED( obScreenLocation ); UNUSED( fXOffset ); UNUSED( fYOffset ); };
	virtual BossAttackType GetType() { ntAssert(0); return BAT_COUNT; }; 
	virtual bool IsVulnerableTo(CStrike* pobStrike) { ntAssert(0); UNUSED( pobStrike ); return false; };
	virtual bool IsVulnerableTo(const CAttackData* pobAttackData) { ntAssert(0); UNUSED( pobAttackData ); return false; };
	virtual ~BossAttack() {};
	float GetAttackProbability() { return m_fAttackProbability; }
	bool IsInvulnerableDuringRecovery() const { return m_bInvulnerableDuringRecovery; }
	bool IsUnInterruptableDuringRecovery() const { return m_bUnInterruptableDuringRecovery; }
protected:
	float m_fAttackProbability;
	bool m_bInvulnerableDuringRecovery;
	bool m_bUnInterruptableDuringRecovery;
};

//------------------------------------------------------------------------------------------
//!
//!	BossMeleeAttack
//! Wrapper class for the start of an attack tree which then can be linked down with certain probability
//!
//------------------------------------------------------------------------------------------
class BossMeleeAttack : public BossAttack
{
	HAS_INTERFACE(BossMeleeAttack);
public:
	BossMeleeAttack();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackStarted();
	virtual void NotifyIsInFailedStrikeRecovery(bool bFailedStrike) { m_bInFailedStrikeRecovery = bFailedStrike; };
	virtual float GetMaxDistance();
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual BossAttackType GetType() { ntAssert(0); return BAT_MELEE; };
	virtual bool IsVulnerableTo(CStrike* pobStrike);
	virtual bool IsVulnerableTo(const CAttackData* pobAttackData);
	virtual ~BossMeleeAttack() {};
protected:
	bool m_bFinished;
	const CAttackLink* m_pobCurrentAttackLink;
	bool m_bDecidedOnLink;
	bool m_bInFailedStrikeRecovery;
	int m_iDepth;

	// Exposed:
	const CAttackLink* m_pobRootAttackLink;
	bool m_bVulnerableDuring;
	bool m_bVulnerableDuringFailedStrikeRecovery;
	ntstd::String m_obProbabilityString;
};

//------------------------------------------------------------------------------------------
//!
//!	BossSpecialAttack
//! Pseudo-abstract class for attacks that require frame by frame management of some sort
//!
//------------------------------------------------------------------------------------------
class BossSpecialAttack : public BossAttack
{
	HAS_INTERFACE(BossSpecialAttack)
public:
	BossSpecialAttack();
	virtual BossAttackType GetType() { ntAssert(0); return BAT_SPECIAL; };
	virtual bool IsVulnerableTo(CStrike* pobStrike) { UNUSED( pobStrike ); return false; };
	virtual bool IsVulnerableTo(const CAttackData* pobAttackData) { UNUSED( pobAttackData ); return false; };
protected:
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttackSelector
//! Pseudo-abstract class for selecting attacks
//!
//------------------------------------------------------------------------------------------
class BossAttackSelector
{
	HAS_INTERFACE(BossAttackSelector)
public:
	enum BossAttackSelectorType { BAST_STANDARD, BAST_RANDOM, BAST_DISTANCE, BAST_UNDERATTACK, BAST_HEALTH, BAST_GGENSMASH, BAST_KINGDEMONDETACH, BAST_KINGDEMONATTACH, BAST_DEMONTENTACLE, BAST_GENERAL, BAST_COUNT };

	BossAttackSelector() :
		m_obAttacks(),
		m_fMaxPriority( 0.6f ),
		m_fMinPriority( 0.4f )
	{};
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) { ntAssert(0); UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); return 0.0f; }; // Will return a float (0.0 - 1.0) representing how important it is that this selector select the next attack
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer) { ntAssert(0); UNUSED(pobBoss); UNUSED(pobPlayer); return 0; }; // Will select the next attack for the boss
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset) { UNUSED( obScreenLocation ); UNUSED( fXOffset ); UNUSED( fYOffset ); };
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };
	virtual BossAttackSelectorType GetSelectorType() { return BAST_STANDARD; }
	virtual ~BossAttackSelector() {};
	BossAttack* ChooseRandomAttackWithWeighting(Boss* pobBoss, CEntity* pobPlayer, ntstd::List<BossAttack*>* pobAlternateList = 0);
protected: 
	ntstd::List<BossAttack*> m_obAttacks; // List of attacks to select from
	float m_fMaxPriority, m_fMinPriority;
};

//------------------------------------------------------------------------------------------
//!
//!	GeneralBossAttackSelector
//! Stuff for a general selector that can be built up using condition objects
//!
//------------------------------------------------------------------------------------------
class GeneralBossAttackSelectorCondition
{
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); ntError(0); };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) { UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); ntError(0); return false; };
	virtual void Reset() {};
	virtual ~GeneralBossAttackSelectorCondition() {};
};

class GeneralBossAttackSelectorAGenInAir : public GeneralBossAttackSelectorCondition
{
	HAS_INTERFACE( GeneralBossAttackSelectorAGenInAir );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossAttackSelectorAGenInAir() {};
private:
	bool m_bRequiredInAirState;
};

class GeneralBossAttackSelectorPlayerCombatStateCondition : public GeneralBossAttackSelectorCondition
{
	HAS_INTERFACE( GeneralBossAttackSelectorPlayerCombatStateCondition );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossAttackSelectorPlayerCombatStateCondition() {};
private:
	COMBAT_STATE m_eRequiredCombatState;
	bool m_bNegateCheck;
};

class GeneralBossAttackSelectorPlayerDistanceCondition : public GeneralBossAttackSelectorCondition
{
	HAS_INTERFACE( GeneralBossAttackSelectorPlayerDistanceCondition );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };;
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossAttackSelectorPlayerDistanceCondition() {};
private:
	float m_fInnerDistance, m_fOuterDistance;
};

class GeneralBossAttackSelectorPlayerTimeSinceLastAttackCondition : public GeneralBossAttackSelectorCondition
{
	HAS_INTERFACE( GeneralBossAttackSelectorPlayerTimeSinceLastAttackCondition );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };;
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossAttackSelectorPlayerTimeSinceLastAttackCondition() {};
private:
	float m_fGracePeriod, m_fGracePeriodAdjust;
};

class GeneralBossAttackSelectorBossTimeSinceLastAttackCondition : public GeneralBossAttackSelectorCondition
{
	HAS_INTERFACE( GeneralBossAttackSelectorBossTimeSinceLastAttackCondition );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };;
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossAttackSelectorBossTimeSinceLastAttackCondition() {};
private:
	float m_fGracePeriod, m_fGracePeriodAdjust;
};

class GeneralBossAttackSelectorBossDistanceFromStaticGeometryBox : public GeneralBossAttackSelectorCondition
{
	HAS_INTERFACE(GeneralBossAttackSelectorBossDistanceFromStaticGeometryBox );
public:
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~GeneralBossAttackSelectorBossDistanceFromStaticGeometryBox() {};
private:
	CPoint m_obOffset;
	float m_fRotationY;
	float m_fWidth;
	float m_fLength;
};

class GeneralBossAttackSelectorMinTimeSinceLastSelected : public GeneralBossAttackSelectorCondition
{
	HAS_INTERFACE(GeneralBossAttackSelectorMinTimeSinceLastSelected);
public:
	GeneralBossAttackSelectorMinTimeSinceLastSelected() { m_fTimeSinceLastSelected = 0.0f; m_fMinTimeBetweenSelection = 5.0f; }
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer);};
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void Reset();
	virtual ~GeneralBossAttackSelectorMinTimeSinceLastSelected() {};
private:
	float m_fTimeSinceLastSelected;
	float m_fMinTimeBetweenSelection;

	//Exposed
	float m_fMinTimeBetweenSelectionLower;
	float m_fMinTimeBetweenSelectionUpper;
};


class GeneralBossAttackSelector : public BossAttackSelector
{
	HAS_INTERFACE( GeneralBossAttackSelector );
public:
	GeneralBossAttackSelector() {};
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer); 
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttackSelectorType GetSelectorType() { return BAST_STANDARD; }
private:
	ntstd::List<GeneralBossAttackSelectorCondition*> m_obConditions; // List of conditions
};

//------------------------------------------------------------------------------------------
//!
//!	RandomBossAttackSelector
//! Class for selecting special attacks randomly
//!
//------------------------------------------------------------------------------------------
class RandomBossAttackSelector : public BossAttackSelector
{
	HAS_INTERFACE(RandomBossAttackSelector)
public:
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_RANDOM; }
};

//------------------------------------------------------------------------------------------
//!
//!	HealthBossAttackSelector
//! Class for selecting attacks according to health lost over time
//!
//------------------------------------------------------------------------------------------
class HealthBossAttackSelector : public BossAttackSelector
{
	HAS_INTERFACE(HealthBossAttackSelector)
public:
	HealthBossAttackSelector();
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_HEALTH; }
private:
	bool m_bFirstUpdate;
	float m_fTimeOfLastSample;
	float m_afHealthSamples[2];

	// Exposed:
	float m_fHealthLostThreshold;
	float m_fHealthSampleRate;
};

//------------------------------------------------------------------------------------------
//!
//!	UnderAttackBossAttackSelector
//! Class for selecting melee attacks when under attack
//!
//------------------------------------------------------------------------------------------
class UnderAttackBossAttackSelector : public BossAttackSelector
{
	HAS_INTERFACE(UnderAttackBossAttackSelector)
public:
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_UNDERATTACK; }
};

//------------------------------------------------------------------------------------------
//!
//!	DistanceSuitabilityBossAttackSelector
//! Class for selecting attacks according to whether they can reach player
//!	Will switch boss to a moving mode if no attack is selectable
//!
//------------------------------------------------------------------------------------------
class DistanceSuitabilityBossAttackSelector : public BossAttackSelector
{
	HAS_INTERFACE(DistanceSuitabilityBossAttackSelector)
public:
	virtual float GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual BossAttack* BeginAttack(Boss* pobBoss, CEntity* pobPlayer);
	BossAttackSelectorType GetSelectorType() { return BAST_DISTANCE; }

private:
	ntstd::List<BossAttack*> m_obInRangeAttacks;
	BossAttack *m_pobLastSelection, *m_pobSelectedAttack;
};


//------------------------------------------------------------------------------------------
//!
//!	BossAttackPhaseEndCondition
//! Pseudo-abstract class for conditions upon which to end the attack phase
//!
//------------------------------------------------------------------------------------------
class BossAttackPhaseEndCondition
{
	HAS_INTERFACE(BossAttackPhaseEndCondition)
public:
	BossAttackPhaseEndCondition() { m_bNotCondition = false; };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer); // Return whether this condition is satisfied or not
	virtual ~BossAttackPhaseEndCondition() {};
protected:
	bool m_bNotCondition;
};

//------------------------------------------------------------------------------------------
//!
//!	HealthBossAttackPhaseEndCondition
//! Class to decide if a health threshold has been reached
//!
//------------------------------------------------------------------------------------------
class HealthBossAttackPhaseEndCondition : public BossAttackPhaseEndCondition
{
	HAS_INTERFACE(HealthBossAttackPhaseEndCondition)
public:
	HealthBossAttackPhaseEndCondition() : BossAttackPhaseEndCondition() { m_fHealthThreshold = 0.0f; };
	virtual bool IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual ~HealthBossAttackPhaseEndCondition() {};
private:
	float m_fHealthThreshold;
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttackPhaseEndTransition
//! Pseudo-abstract class to perform some ending transition to an attack phase
//!
//------------------------------------------------------------------------------------------
class BossAttackPhaseEndTransition
{
	HAS_INTERFACE(BossAttackPhaseEndTransition)
public:
	BossAttackPhaseEndTransition() {};
	BossAttackPhase* GetToPhase();
	virtual bool BeginEndTransition(Boss* pobBoss, CEntity* pobPlayer) { UNUSED(pobBoss); UNUSED(pobPlayer); return false; };
	virtual bool Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) { ntAssert(0); UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); return false; };
	virtual void NotifyAttackFinished() {};
	virtual void NotifyAttackAutoLinked() {};
	virtual void NotifyAttackStarted() {};
	virtual void NotifyInteractionWith(CEntity* pobEntity) { UNUSED( pobEntity ); };
	virtual void NotifyMovementDone() {};
	virtual void NotifyGotStruck() {};
	virtual void NotifyIsInFailedStrikeRecovery(bool bFailedStrike) { UNUSED(bFailedStrike); };
	virtual void NotifyVulnerableToIncomingStrike(CStrike* pobStrike) { UNUSED(pobStrike); };
	virtual void NotifyInvulnerableToIncomingStrike(CStrike* pobStrike) { UNUSED(pobStrike); };
	virtual void NotifyWillBlockIncomingStrike(CStrike* pobStrike) { UNUSED(pobStrike); };
	virtual void NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol) { UNUSED(pobVol); };
	virtual void NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol) { UNUSED(pobVol); };
	virtual void NotifyProjectileCountered(Object_Projectile* /*pobProj*/) {};
	virtual void NotifyPlayerInteracting(bool /*bState*/) {};
	virtual void NotifyPlayerInteractionAction() {};
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset) { UNUSED( obScreenLocation ); UNUSED( fXOffset ); UNUSED( fYOffset ); };
	virtual ~BossAttackPhaseEndTransition() {};
protected:
	BossAttackPhase* m_pobToPhase;
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttackPhaseEndTransitionSpecialAttacking
//! Class to perform an attacking ending transition
//!
//------------------------------------------------------------------------------------------
class BossAttackPhaseEndTransitionSpecialAttacking : public BossAttackPhaseEndTransition
{
	HAS_INTERFACE(BossAttackPhaseEndTransitionSpecialAttacking)
public:
	BossSpecialAttack* GetBossSpecialAttack();
	virtual bool BeginEndTransition(Boss* pobBoss, CEntity* pobPlayer);
	virtual bool Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished() { m_pobEndAttack->NotifyAttackFinished(); };
	virtual void NotifyAttackAutoLinked() { m_pobEndAttack->NotifyAttackAutoLinked(); };
	virtual void NotifyAttackStarted() { m_pobEndAttack->NotifyAttackStarted(); };
	virtual void NotifyInteractionWith(CEntity* pobEntity) { m_pobEndAttack->NotifyInteractionWith(pobEntity); };
	virtual void NotifyMovementDone() { m_pobEndAttack->NotifyMovementDone(); };
	virtual void NotifyGotStruck() { m_pobEndAttack->NotifyGotStruck(); };
	virtual void NotifyIsInFailedStrikeRecovery(bool bFailedStrike) { m_pobEndAttack->NotifyIsInFailedStrikeRecovery(bFailedStrike); };
	virtual void NotifyVulnerableToIncomingStrike(CStrike* pobStrike) { m_pobEndAttack->NotifyVulnerableToIncomingStrike(pobStrike); };
	virtual void NotifyInvulnerableToIncomingStrike(CStrike* pobStrike) { m_pobEndAttack->NotifyInvulnerableToIncomingStrike(pobStrike); };
	virtual void NotifyWillBlockIncomingStrike(CStrike* pobStrike) { m_pobEndAttack->NotifyWillBlockIncomingStrike(pobStrike); };
	virtual void NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	virtual void NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	virtual void NotifyProjectileCountered(Object_Projectile* pobProj) { m_pobEndAttack->NotifyProjectileCountered(pobProj); };
	virtual void NotifyPlayerInteractionAction() { m_pobEndAttack->NotifyPlayerInteractionAction(); };
	virtual void NotifyPlayerInteracting(bool bState) { m_pobEndAttack->NotifyPlayerInteracting(bState); };
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual ~BossAttackPhaseEndTransitionSpecialAttacking() {};
private:
	BossSpecialAttack* m_pobEndAttack;
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttackPhaseEndTransitionAnimated
//! Class to perform an animated ending transition
//!
//------------------------------------------------------------------------------------------
class BossAttackPhaseEndTransitionAnimated : public BossAttackPhaseEndTransition
{
	HAS_INTERFACE(BossAttackPhaseEndTransitionAnimated)
public:
	BossAttackPhaseEndTransitionAnimated() { m_bFinished = false; };
	CHashedString GetAnimName();
	virtual bool BeginEndTransition(Boss* pobBoss, CEntity* pobPlayer);
	virtual bool Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual void NotifyMovementDone();
	virtual ~BossAttackPhaseEndTransitionAnimated() {};
private:
	CHashedString m_obEndAnimName;
	bool m_bFinished;
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttackPhaseEndTransitionWithMovement
//!
//------------------------------------------------------------------------------------------
class BossAttackPhaseEndTransitionWithMovement : public BossAttackPhaseEndTransition
{
	HAS_INTERFACE(BossAttackPhaseEndTransitionWithMovement)
public:
	BossAttackPhaseEndTransitionWithMovement() { m_bFinished = false; };
	CHashedString GetAnimName();
	virtual bool BeginEndTransition(Boss* pobBoss, CEntity* pobPlayer);
	virtual bool Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual void NotifyMovementDone();
	virtual ~BossAttackPhaseEndTransitionWithMovement() {};
private:
	BossMovement* m_pobMovementToDo;
	bool m_bFinished;
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttackPhaseEndTransitionAnimated
//! Class to perform an animated ending transition - NOT IMPLEMENTED YET
//!
//------------------------------------------------------------------------------------------
class BossAttackPhaseEndTransitionNinjaSequence : public BossAttackPhaseEndTransition
{
	HAS_INTERFACE(BossAttackPhaseEndTransitionNinjaSequence)
public:
	BossAttackPhaseEndTransitionNinjaSequence() { m_bFinished = false; };
	virtual bool BeginEndTransition(Boss* pobBoss, CEntity* pobPlayer);
	virtual bool Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual ~BossAttackPhaseEndTransitionNinjaSequence() {};
private:
	bool m_bFinished;
	CEntity* m_pobNinjaSequenceEntity;
	bool m_bFirstFrame;
	
	// Exposed:
	CHashedString m_obNinjaSequenceEntityName;
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttackPhaseStartTransition
//! Pseudo-abstract class to perform some start transition to an attack phase
//!
//------------------------------------------------------------------------------------------
class BossAttackPhaseStartTransition
{
	HAS_INTERFACE(BossAttackPhaseStartTransition)
public:
	BossAttackPhaseStartTransition() {};
	virtual void BeginStartTransition(Boss* pobBoss, CEntity* pobPlayer) { ntAssert(0); UNUSED(pobBoss); UNUSED( pobPlayer ); };
	virtual bool Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) { ntAssert(0); UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); return false; };
	virtual void NotifyAttackFinished() {};
	virtual void NotifyAttackAutoLinked() {};
	virtual void NotifyAttackStarted() {};
	virtual void NotifyInteractionWith(CEntity* pobEntity) { UNUSED( pobEntity ); };
	virtual void NotifyMovementDone() {};
	virtual void NotifyGotStruck() {};
	virtual void NotifyIsInFailedStrikeRecovery(bool bFailedStrike) { UNUSED(bFailedStrike); };
	virtual void NotifyVulnerableToIncomingStrike(CStrike* pobStrike) { UNUSED(pobStrike); };
	virtual void NotifyInvulnerableToIncomingStrike(CStrike* pobStrike) { UNUSED(pobStrike); };
	virtual void NotifyWillBlockIncomingStrike(CStrike* pobStrike) { UNUSED(pobStrike); };
	virtual void NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol) { UNUSED(pobVol); };
	virtual void NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol) { UNUSED(pobVol); };
	virtual void NotifyProjectileCountered(Object_Projectile* /*pobProj*/) {};
	virtual void NotifyPlayerInteracting(bool /*bState*/) {};
	virtual void NotifyPlayerInteractionAction() {};
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset) { UNUSED( obScreenLocation ); UNUSED( fXOffset ); UNUSED( fYOffset ); };
	virtual ~BossAttackPhaseStartTransition() { };
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttackPhaseStartTransitionSpecialAttacking
//! Class to perform an attacking ending transition
//!
//------------------------------------------------------------------------------------------
class BossAttackPhaseStartTransitionSpecialAttacking : public BossAttackPhaseStartTransition
{
	HAS_INTERFACE(BossAttackPhaseStartTransitionSpecialAttacking)
public:
	BossSpecialAttack* GetBossSpecialAttack();
	virtual void BeginStartTransition(Boss* pobBoss, CEntity* pobPlayer);
	virtual bool Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished() { m_pobStartAttack->NotifyAttackFinished(); };
	virtual void NotifyAttackAutoLinked() { m_pobStartAttack->NotifyAttackAutoLinked(); };
	virtual void NotifyAttackStarted() { m_pobStartAttack->NotifyAttackStarted(); };
	virtual void NotifyInteractionWith(CEntity* pobEntity) { m_pobStartAttack->NotifyInteractionWith(pobEntity); };
	virtual void NotifyMovementDone() { m_pobStartAttack->NotifyMovementDone(); };
	virtual void NotifyGotStruck() { m_pobStartAttack->NotifyGotStruck(); };
	virtual void NotifyIsInFailedStrikeRecovery(bool bFailedStrike) { m_pobStartAttack->NotifyIsInFailedStrikeRecovery(bFailedStrike); };
	virtual void NotifyVulnerableToIncomingStrike(CStrike* pobStrike) { m_pobStartAttack->NotifyVulnerableToIncomingStrike(pobStrike); };
	virtual void NotifyInvulnerableToIncomingStrike(CStrike* pobStrike) { m_pobStartAttack->NotifyInvulnerableToIncomingStrike(pobStrike); };
	virtual void NotifyWillBlockIncomingStrike(CStrike* pobStrike) { m_pobStartAttack->NotifyWillBlockIncomingStrike(pobStrike); };
	virtual void NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	virtual void NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	virtual void NotifyProjectileCountered(Object_Projectile* pobProj) { m_pobStartAttack->NotifyProjectileCountered(pobProj); };
	virtual void NotifyPlayerInteracting(bool bState) { m_pobStartAttack->NotifyPlayerInteracting(bState); };
	virtual void NotifyPlayerInteractionAction() { m_pobStartAttack->NotifyPlayerInteractionAction(); };
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual ~BossAttackPhaseStartTransitionSpecialAttacking() {};
private:
	BossSpecialAttack* m_pobStartAttack;
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttackPhaseStartTransitionAnimated
//! Class to perform an animated ending transition - NOT IMPLEMENTED YET
//!
//------------------------------------------------------------------------------------------
class BossAttackPhaseStartTransitionAnimated : public BossAttackPhaseStartTransition
{
	HAS_INTERFACE(BossAttackPhaseStartTransitionAnimated)
public:
	BossAttackPhaseStartTransitionAnimated() { m_bFinished = false; };
	CHashedString GetAnimName();
	virtual void BeginStartTransition(Boss* pobBoss, CEntity* pobPlayer);
	virtual bool Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual void NotifyMovementDone();
	virtual ~BossAttackPhaseStartTransitionAnimated() {};
private:
	CHashedString m_obStartAnimName;
	bool m_bFinished;
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttackPhaseState
//! Class to manage an attack phase state
//!
//------------------------------------------------------------------------------------------
class BossAttackPhaseState
{
	HAS_INTERFACE( BossAttackPhaseState )
public:
	enum BossAttackPhaseStateMode { BAPSM_MOVING, BAPSM_ATTACKING };

	BossAttackPhaseState();
	~BossAttackPhaseState();

	void Initialise(Boss* pobBoss, CEntity* pobPlayer);
	float GetTimeToSpendIn() { return m_fTimeToSpendIn; };
	float GetTimeSpentIn() { return m_fTimeSpentIn; };
	float GetProbabilityOfSwitchingTo() { return m_fProbabilityOfSwitchingToState; };
	BossAttackPhaseState* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	bool IsAttacking() { return m_pobCurrentAttack != 0; };

	// Notifications
	void NotifyAttackFinished(Boss* pobBoss);
	void NotifyAttackStarted(Boss* pobBoss);
	void NotifyAttackAutoLinked(Boss* pobBoss);
	void NotifyAttackInterrupted(Boss* pobBoss);
	void NotifyInteractionWith(CEntity* pobEntity);
	void NotifyMovementDone(Boss* pobBoss);
	void NotifyGotStruck(Boss* pobBoss);
	void NotifyIsInFailedStrikeRecovery(bool bFailedStrike);
	void NotifyVulnerableToIncomingStrike(CStrike* pobStrike);
	void NotifyDeflected(Boss* pobBoss);
	void NotifyRecovered(Boss* pobBoss);
	void NotifyInvulnerableToIncomingStrike(CStrike* pobStrike);
	void NotifyWillBlockIncomingStrike(CStrike* pobStrike);
	void NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	void NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	void NotifyProjectileCountered(Object_Projectile* pobProj);
	void NotifyPlayerInteracting(bool bState);
	void NotifyPlayerInteractionAction();

	bool IsVulnerableTo(CStrike* pobStrike);
	bool IsVulnerableTo(const CAttackData* pobAttackData);
	bool ShouldBlock(CStrike* pobStrike);

	void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);

	const BossAttack* GetCurrentAttack() const { return m_pobCurrentAttack; }
private:
	BossAttackPhaseStateMode m_eCurrentMode;

	BossAttack* m_pobCurrentAttack;
	// Need 2 movement vars so we can keep track of which one was last active and whether it's done yet
	BossMovement* m_pobCurrentMovement;
	BossMovement* m_pobCurrentMovementNotDone;

	float m_fTimeSpentIn;
	float m_fTimeToSpendIn;

	bool m_bNeedsDodge, m_bIsDodging;
	bool m_bGotStruck;
	ntstd::List<float> m_obTimesToDecrement;
	int m_iNumberOfStrikesInLastSeconds;
	float m_fTimeToBeVulnerableAgain;
	bool m_bInvulnerableBecauseOfStrikeCountTrigger;

	float m_fLastWinningPriority;
	BossAttackSelector* m_pobLastWinningAttackSelector;

	// Exposed:
	ntstd::List<BossAttackSelector*> m_obAttackSelectors;
	ntstd::List<BossMovementSelector*> m_obMovementSelectors;

	float m_fMaxTimeInState;
	float m_fMaxTimeInStateAdjust;
	float m_fProbabilityOfSwitchingToState;
	bool m_bVulnerableWhileNotAttacking;
	float m_fTimeToRememberStrikes;
	float m_fTimeToBeInvulnerable;
	int m_iNumberOfStrikesTillInvulnerable;

	ntstd::String m_obRecoilingDodgeProbabilityString, m_obDeflectingDodgeProbabilityString;
	BossMovementSelector* m_pobDodgeMovementSelector;
	BossAttackSelector* m_pobDodgeAttackSelector;
	float m_fProbabilityOfDodgeMovement;

	float m_fSpeedBlockProbability;
	float m_fPowerBlockProbability;
	float m_fRangeBlockProbability;
};

//------------------------------------------------------------------------------------------
//!
//!	BossAttackPhase
//! Class to manage an attack phase
//!
//------------------------------------------------------------------------------------------
class BossAttackPhase
{
	HAS_INTERFACE(BossAttackPhase)
public:
	BossAttackPhase();
	virtual ~BossAttackPhase();	

	BossAttackPhase* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	BossAttackPhaseEndTransition* GetEndTransition();

	enum BossAttackPhaseMode
	{
		BAPM_START_PHASE_TRANSITION, // Starting this phase
		BAPM_IN_ATTACK_STATE, // We're in a state
		BAPM_END_PHASE_TRANSITION, // Ending this phase
		BAPM_COUNT
	};

	// Notifications
	void NotifyAttackFinished(Boss* pobBoss);
	void NotifyAttackStarted(Boss* pobBoss);
	void NotifyAttackAutoLinked(Boss* pobBoss);
	void NotifyAttackInterrupted(Boss* pobBoss);
	void NotifyInteractionWith(CEntity* pobEntity);
	void NotifyMovementDone(Boss* pobBoss);
	virtual void NotifyGotStruck(Boss* pobBoss); // Virtualised for AGen doppelganger spawning
	void NotifyIsInFailedStrikeRecovery(bool bFailedStrike);
	void NotifyVulnerableToIncomingStrike(CStrike* pobStrike);
	void NotifyDeflected(Boss* pobBoss);
	void NotifyRecovered(Boss* pobBoss);
	void NotifySuperSafetyTransitionDone(Boss* pobBoss);
	void NotifyInvulnerableToIncomingStrike(CStrike* pobStrike);
	void NotifyWillBlockIncomingStrike(CStrike* pobStrike);
	void NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	void NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	void NotifySuperStyleDone(Boss* pobBoss);
	void NotifyProjectileCountered(Object_Projectile* pobProj);
	void NotifyPlayerInteracting(bool bState);
	void NotifyPlayerInteractionAction();

	void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);

	bool IsVulnerableTo(CStrike* pobStrike);
	bool IsVulnerableTo(const CAttackData* pobAttackData);
	bool ShouldBlock(CStrike* pobStrike);

	BossAttackPhaseState* GetCurrentBossAttackPhaseState() { return m_pobCurrentState; };

	// Nice helper to have around:
	static const CAttackLink* GetNextAttackLink(const CAttackLink* pobFromLink);
protected:
	// Nice helpers
	bool ShouldLeaveState();
	BossAttackPhaseState* GetNextState();
	void SwitchStateTo(BossAttackPhaseState* pobTo, Boss* pobBoss, CEntity* pobPlayer);
	bool CanSwitchState(Boss* pobBoss);

	// Control variables
	BossAttackPhaseMode m_eCurrentMode; // Starting, in a state, or ending
	BossAttackPhaseState* m_pobCurrentState; // Pointer to a state that holds attacks and stuff
	bool m_bFirstUpdate;

	// Exposed to welder:
	// Lists of attack states
	ntstd::List<BossAttackPhaseState*> m_obAttackPhaseStates;
	// How do we start this phase
	BossAttackPhaseStartTransition* m_pobStartTransition;
	// When and how do we end this phase
	ntstd::List<BossAttackPhaseEndCondition*> m_obEndConditions; // These will be queried every frame to see if this phase is over, all conditions must be satisfied
	BossAttackPhaseEndTransition* m_pobEndTransition; // Do we need to do anything special at the end of the phase

	// For triggering a ninja sequence if we go below a certain health threshold and get involved in a super safety transition
	CHashedString m_obSuperStyleNinjaSequenceEntityName;
	float m_fSuperStyleNinjaSequenceHealthThreshold;

	friend class Boss;
};

class Boss : public Character
{
	// Declare dataobject interface
	HAS_INTERFACE(Boss)

public:
	Boss();
	~Boss();

	enum BossType
	{
		BT_GLADIATOR_GENERAL,
		BT_WATER_GENERAL,
        BT_AERIAL_GENERAL,
		BT_KING_BOHAN,
		BT_DEMON,
		BT_COUNT
	};

	void OnPostConstruct();

	void Update(float fTimeDelta);

	void SetMovementController();
	void SetInitialAttackPhase();
	//Added for direct control over the demon.
	void SetNamedAttackPhase(CHashedString obAttackPhaseName);
	void SetNoAttackPhase();

	CMovementInput* GetBossMovement();
	BossAttackPhase* GetCurrentAttackPhase();

	// Some notifications of combat events, mostly just passed onto active attack phase
	void NotifyUnderAttack(bool bUnderAttack);
	virtual void NotifyIsInFailedStrikeRecovery(bool bFailedStrike);
	void NotifyAttackStarted();
	void NotifyAttackFinished();
	void NotifyAttackAutoLinked();
	void NotifyAttackInterrupted();
	void NotifyMovementDone();
	void NotifyInteractionWith(CEntity* pobEntity);
	void NotifyGotStruck();
	void NotifyVulnerableToIncomingStrike(CStrike* pobStrike);
	void NotifyDeflected();
	void NotifyRecovered();
	void NotifySuperStyleDone();
	void NotifyInvulnerableToIncomingStrike(CStrike* pobStrike);
	void NotifyWillBlockIncomingStrike(CStrike* pobStrike);
	void NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	void NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	void NotifyProjectileCountered(Object_Projectile* pobProj);
	void NotifyPlayerInteracting(bool bState);
	void NotifyPlayerInteractionAction();

	bool IsUnderAttack();
	bool IsVulnerableToIncomingStrike() { return m_bVulnerableToIncomingStrike; };

	// Pseudo-virtual members that each boss type must implement - can't be true virtual cos of serialisation
	bool IsVulnerableTo(CStrike* pobStrike);
	bool IsVulnerableTo(const CAttackData* pobAttackData);
	bool CanStartAnAttack();
	bool CanStartALinkedAttack();
	bool ShouldBlock(CStrike* pobStrike);

	void UpdateBossSpecifics(float fTimeDelta);
	void DebugRenderBossSpecifics( CPoint& obScreenLocation, float fXOffset, float fYOffset );

	void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);

	BossType GetBossType() { return m_eBossType; };

	BossSimpleNavigationManager* GetNavMan() { return m_pobNavMan; };
	
	float GetTimeSinceLastPlayerStrike() { return m_fTimeSinceLastPlayerStrike; }
	void ResetTimeSinceLastPlayerStrike() { m_fTimeSinceLastPlayerStrike = 0.0f; }
    
	float GetTimeSinceBossLastAttacked() { return m_fTimeSinceBossLastAttacked; }
	void ResetTimeSinceBossLastAttacked() { m_fTimeSinceBossLastAttacked = 0.0f; }

	void GoToPhase(CHashedString obName);
	virtual void GoToPhase(unsigned int iPhaseNumber) { UNUSED(iPhaseNumber); }
#ifdef DEBUG_HORRIBLE_REACTSTATE_TIMEOUT
	float GetTimeInReactState() { return m_fTimeInReactState; }
	void ResetTimeInReactState() { m_fTimeInReactState = 0.0f; }
	void IncrementTimeInReactState(float fTimeDelta) { m_fTimeInReactState += fTimeDelta; }
#endif
        
	virtual void ChangeHealth( float fDelta, const char* );
protected:	
	BossType m_eBossType;

	CHashedString m_obDefaultWalkingController;
	CHashedString m_obInitialAttackPhase;

	CMovementInput m_obMovementInput;

	BossAttackPhase* m_pobCurrentAttackPhase;

	bool m_bIsUnderAttack;

	CombatEventLog* m_pobCombatEventLog;

	bool m_bVulnerableToIncomingStrike;

	bool m_bForceRemoveAttackPhase;
	bool m_bAttackPhaseSetManually;
	BossAttackPhase* m_pobForcedCurrentAttackPhase;

	ntstd::List<BossSimpleNavigationAvoidanceArea*> m_obAvoidanceAreas;
	BossSimpleNavigationManager* m_pobNavMan;
	
	float m_fTimeSinceLastPlayerStrike;
	float m_fTimeSinceBossLastAttacked;
#ifdef DEBUG_HORRIBLE_REACTSTATE_TIMEOUT
	float m_fTimeInReactState;	//Timeout baby, YEAH! Shouldn't be needed in final thing, but a safety net sure helps testing!
#endif
};

#endif // _ENTITY_BOSS_H
