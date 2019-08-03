#ifndef	_ENTITY_WGEN_H
#define	_ENTITY_WGEN_H

#include "entityboss.h"
#include "editable/enumlist.h"

class WaterInstance;
struct WaveEmitter;
struct WaveDma;
class CombatPhysicsStrikeVolume;

class WaterGeneralSwimToPoint
{
	HAS_INTERFACE( WaterGeneralSwimToPoint );
public:
	CPoint m_obPoint;
	float m_fRadius;
};

struct WaveFinishingData
{
	WaveEmitter* m_pobWaveEmitter;
	WaveDma* m_pobWaveDma;
	float m_fWaveSpeed;
	
	float m_fSpeedReductionRate;
	float m_fWidthReductionRate;
	float m_fHeightReductionRate;
	float m_fDepthReductionRate;
};

class WaterGeneral : public Boss
{
	HAS_INTERFACE(WaterGeneral);
public:
	WaterGeneral();
	~WaterGeneral();
	void OnPostPostConstruct();
	bool CanStartAnAttack();
	bool CanStartALinkedAttack();
	int GetNumberOfSwimToPoints() { return m_obSwimToPoints.size(); };
	CPoint GetSwimToPoint(int i);
	float GetSwimToPointRadius(int i);
	int GetNumberOfWaterfallPoints() { return 4; };
	CPoint GetWaterfallPoint(int i) { return m_aobWaveJumpSpecialStartPoints[i]; };
	CDirection GetWaterfallNormal(int i) { return m_aobWaveJumpSpecialStartNormals[i]; };
	void UpdateBossSpecifics(float fTimeDelta);
	void DebugRenderBossSpecifics( CPoint& obScreenLocation, float fXOffset, float fYOffset );

	void FinishWave(WaveEmitter* pobEmitter, WaveDma* pobWaveDma, float fWaveSpeed);
private:
	CPoint m_aobWaveJumpSpecialStartPoints[4];
	CDirection m_aobWaveJumpSpecialStartNormals[4];

	ntstd::List<WaterGeneralSwimToPoint*> m_obSwimToPoints;

	ntstd::Vector<WaveFinishingData*> m_apobWaves;
};

//------------------------------------------------------------------------------------------
//!
//!	WaterGeneralSwimAwayTransitioningMovement
//! Class for moving a boss relative to a player
//!
//------------------------------------------------------------------------------------------
class WaterGeneralSwimAwayTransitioningMovement : public BossTransitioningMovement
{
	HAS_INTERFACE( WaterGeneralSwimAwayTransitioningMovement );
public:
	WaterGeneralSwimAwayTransitioningMovement() { m_fPlayerTooCloseToSwimToPointThreshold = 5.0f; m_bDone = false; };
	virtual BossMovementType GetMovementType() { return BMT_TRANSITIONING; };
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone() { m_bDone = true; };
	virtual ~WaterGeneralSwimAwayTransitioningMovement() {};
protected:
	bool m_bDone;

	// Exposed:
	float m_fPlayerTooCloseToSwimToPointThreshold;
	CHashedString m_obSwimStartAnim;
	CHashedString m_obSwimCycleAnim;
	CHashedString m_obSwimStopAnim;
};

//------------------------------------------------------------------------------------------
//!
//!	WaterGeneralSwimThroughPointsMovement
//! Class for moving a boss relative to a player
//!
//------------------------------------------------------------------------------------------
class WaterGeneralSwimThroughPointsMovement : public BossTransitioningMovement
{
	HAS_INTERFACE( WaterGeneralSwimThroughPointsMovement );
public:
	WaterGeneralSwimThroughPointsMovement() { m_bDone = false; m_fProximityThreshold = 1.0f; m_fSpeed = 1.0f; m_fTimeToSpendOnEachPoint = 2.5f; m_fMaxRotationPerSecond = 90.0f; m_iNumPoints = 10; m_fMaxDirectionChange = 45.0f; m_fSlowForTurnScalar = 0.6f; m_fMaxSpeedChange = 0.15f; m_fLookAhead = 7.0f; m_bEndAlignToPlayer = false; };
	virtual BossMovementType GetMovementType() { return BMT_TRANSITIONING; };
	virtual BossMovement* Initialise(Boss* pobBoss, CEntity* pobPlayer);
	void SetEndPoint(CPoint& obPoint);
	void SetEndAlignToZ(CDirection& obAlignTo);
	virtual BossMovement* DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyMovementDone();
	bool IsDone() { return m_bDone; };
	virtual ~WaterGeneralSwimThroughPointsMovement() {};
protected:
	bool m_bDone;
	CPoint m_aobPathPoints[25];
	int m_iCurrentPoint;
	float m_fTimeOnCurrentPoint;
	bool m_bEndStarted;
	bool m_bEndAlignToZSet;
	CDirection m_obEndAlignToZ;
	bool m_bDone180OnCurrentPoint;

	float m_fTimeToSpendOnEachPoint;
	float m_fSpeed;
	float m_fProximityThreshold;
	float m_fMaxRotationPerSecond,	m_fMaxDirectionChange, m_fSlowForTurnScalar, m_fMaxSpeedChange;
	float m_fLookAhead;
	CHashedString m_obSwimStartAnim;
	CHashedString m_obSwimCycleAnim;
	CHashedString m_obSwimCycleLeanLeftAnim;
	CHashedString m_obSwimCycleLeanRightAnim;
	CHashedString m_obSwim180Anim;
	CHashedString m_obSwimStopAnim;
	int m_iNumPoints;
	bool m_bEndAlignToPlayer;
	float m_f180TurnAngleTrigger;
};

class WaterGeneralSingleWaveLashSpecial : public BossSpecialAttack
{
	HAS_INTERFACE(WaterGeneralSingleWaveLashSpecial);

	#define MAX_WAVE_LASH_EMITTERS 7
public:
	WaterGeneralSingleWaveLashSpecial();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual bool IsVulnerableTo(CStrike* pobStrike);
	virtual bool IsVulnerableTo(const CAttackData* pobAttackData);
	virtual void NotifyAttackFinished();
	virtual void NotifyAttackAutoLinked();
	virtual BossAttack* NotifyAttackInterrupted() { return 0; };
	virtual void NotifyMovementDone();
	virtual void NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	virtual void NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual ~WaterGeneralSingleWaveLashSpecial() {};
private:
	int m_iLashesRemaining;
	bool m_bStartLashing;

	WaterInstance* m_pobWaterInstance;
	int m_iNextWaveEmmiterToUse;
	CombatPhysicsStrikeVolume* m_apobWaveEmitterStrikeVolumes[MAX_WAVE_LASH_EMITTERS];
	WaveEmitter* m_apobWaveEmitters[MAX_WAVE_LASH_EMITTERS];
	WaveDma* m_apobWaveDmas[MAX_WAVE_LASH_EMITTERS];
	float m_afWaveDisplacementsLastFrame[MAX_WAVE_LASH_EMITTERS];

	// Exposed:
	int m_iMaxNumberOfLashes;
	int m_iMaxNumberOfLashesAdjust;
	const CAttackLink* m_pobLashStart;
	const CAttackLink* m_pobLashCycle;
	const CAttackLink* m_pobLashEnd;
	WaterGeneralSwimThroughPointsMovement* m_pobSwimmyMovement;
	float m_fPlayerTooCloseToSwimToPointThreshold;

	CHashedString m_obWaterInstanceName;
	CHashedString m_aobWaveEmitterNames[MAX_WAVE_LASH_EMITTERS];

	CHashedString m_obWaveJumpClusterStructureName;
	float m_fInnerDistanceFromWaveToSwapInWaveJumpCluster;
	float m_fOuterDistanceFromWaveToSwapInWaveJumpCluster;
	VIRTUAL_BUTTON_TYPE m_eWaveJumpButtonHint;
};

class WaterGeneralWaveJumpSpecial : public BossSpecialAttack
{
	HAS_INTERFACE(WaterGeneralWaveJumpSpecial);

	#define MAX_WAVE_JUMP_EMITTERS 3
public:
	WaterGeneralWaveJumpSpecial();
	virtual BossAttack* Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer);
	virtual void NotifyAttackFinished();
	virtual void NotifyMovementDone();
	virtual void NotifyCreatedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	virtual void NotifyRemovedStrikeVolume(CombatPhysicsStrikeVolume* pobVol);
	virtual bool Initialise(Boss* pobBoss, CEntity* pobPlayer);
	virtual void DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset);
	virtual bool IsVulnerableTo(CStrike* pobStrike) { UNUSED(pobStrike); return m_pobSwimmyMovement->IsDone(); };
	virtual bool IsVulnerableTo(const CAttackData* pobAttackData) { UNUSED(pobAttackData); return m_pobSwimmyMovement->IsDone(); };
	virtual ~WaterGeneralWaveJumpSpecial() {};
private:
	bool m_bStartAttack, m_bClimbStarted, m_bAttackStarted;

	WaterInstance* m_pobWaterInstance;
	int m_iNextWaveEmmiterToUse;
	CombatPhysicsStrikeVolume* m_apobWaveEmitterStrikeVolumes[MAX_WAVE_JUMP_EMITTERS];
	WaveEmitter* m_apobWaveEmitters[MAX_WAVE_JUMP_EMITTERS];
	WaveDma* m_apobWaveDmas[MAX_WAVE_JUMP_EMITTERS];

	// Exposed:
	WaterGeneralSwimThroughPointsMovement* m_pobSwimmyMovement;
	CHashedString m_obWaterfallClimbAnim;
	CHashedString m_obWaterfallJumpAnim;

	const CAttackLink* m_pobLandAttack;

	/*CHashedString m_obWaveJumpClusterStructureName;
	float m_fInnerDistanceFromWaveToSwapInWaveJumpCluster;
	float m_fOuterDistanceFromWaveToSwapInWaveJumpCluster;
	VIRTUAL_BUTTON_TYPE m_eWaveJumpButtonHint;*/
	
	CHashedString m_obWaterInstanceName;
	CHashedString m_aobWaveEmitterNames[MAX_WAVE_JUMP_EMITTERS];
};

#endif	// _ENTITY_WGEN_H
