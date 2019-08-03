//! -------------------------------------------
//! airangedtargeting.h
//!
//! AI Ranged Targeting
//!
//! Author: Gavin Costello
//!
//!--------------------------------------------

#ifndef _AIRANGEDTARGETING_H
#define _AIRANGEDTARGETING_H

//Uncomment this to only update the aim every nth seconds (though tracking etc is done per-frame still).
//#define AI_RANGEDTARGETING_PERIODIC_AIM_UPDATE

// Forward declaration of classes and structures
class CAIComponent;
class CEntity;
class AI;
class Character;

//--------------------------------------------------
//!
//! CAIRangedTargetingModifiableByLuaParams
//!
//! Stores all of the parameters for the ranged-aiming that can be set from LUA scripts.
//! These parameters are then used during CAIRangedTargeting::Update().
//!
//--------------------------------------------------
struct CAIRangedTargetingModifiableByLuaParams
{
public:
	//Relative sizes for the targeting reticule.
	float m_fMaxExpansion;
	float m_fMaxExpansionDistance;
	float m_fMinContraction;

	//The targeting reticule, after increasing, will stay at max-size for m_fContractionCooldownTime, and then decrease back to
	//m_fMinContraction over the next m_fContractionTime seconds (unless interrupted).
	float m_fContractionCooldownTime;
	float m_fContractionTime;

	//For the movement-related expansion-events.
	float m_fTimeRunningContinuouslyForExpansionEvent;
	float m_fMinTimeRunningBeforeStopForExpansionEvent;

	CAIRangedTargetingModifiableByLuaParams() :
	  m_fMaxExpansion(1.5f),
	  m_fMaxExpansionDistance(40.0f),
	  m_fMinContraction(0.5f),
	  m_fContractionCooldownTime(5.0f),
	  m_fContractionTime(6.0f),
	  m_fTimeRunningContinuouslyForExpansionEvent(6.0f),
	  m_fMinTimeRunningBeforeStopForExpansionEvent(3.0f)
	{}
};


//--------------------------------------------------
//!
//! CAIRangedTargeting
//!
//! Handles ranged-AI aiming.
//!
//--------------------------------------------------
class CAIRangedTargeting
{
protected:
private:
	//All params that can be set from LUA scripts.
	CAIRangedTargetingModifiableByLuaParams m_obParams;

	//The overall target-point that we should be aiming for.
	CPoint m_obTargetPoint;

	//Our parent AI so that we can grab information such as target during update.
	AI* m_pParentEntity;

	//Current-state variables.
	float m_fCurrentTimeTillCooldownBegins;
	float m_fCurrentExpansion;
	float m_fCurrentExpansionScalar;
#ifdef AI_RANGEDTARGETING_PERIODIC_AIM_UPDATE
	float m_fCurrentTimeSinceLastAimUpdate;	//We won't update our actual aim position too often.
#endif
	float m_fLastAmountOfTimeTargetIsMovingContinuously;
	float m_fAmountOfTimeTargetIsMovingContinuously;
	bool m_bInFrustumLastFrame;
	bool m_bArcherTargetAimingLastFrame;

	bool CheckForExpansionEvents(const Character* pTargetEntity);
	void ForceReticuleExpansion();

public:
	CAIRangedTargeting();
	~CAIRangedTargeting();

	const CPoint &GetTargetPoint() { return m_obTargetPoint; }
	void SetParent(CEntity* pParentEntity);
	void SetRangedTargetingParam(unsigned int uiParam, float fValue);

	void Update(float fTimeChange);
};

#endif //_AIRANGEDTARGETING_H
