//------------------------------------------------------------------------------------------
//!	@file collisioneffecthandler.h
//!	@author Chip Bell (SCEE), Harvey Cotton
//!	@date 31.08.06
//!
//!	@brief Declaration of the CollisionEffectHandler and related classes.
//------------------------------------------------------------------------------------------


#ifndef _COLLISIONEFFECTHANDLER_H_
#define _COLLISIONEFFECTHANDLER_H_


#include "audio/collisioneffectmanager.h"
#include "core/keystring.h"


#ifdef _COLLISION_EFFECT_DEBUG
#include "core/vecmath.h"

class CollisionEffectDef;
#endif // _COLLISION_EFFECT_DEBUG


//------------------------------------------------------------------------------------------
//!	@class CollisionEffectFilterDef
//!	@brief Defines parameters required for entity collision effect handling.
//------------------------------------------------------------------------------------------
class CollisionEffectFilterDef
{
public:
	CollisionEffectFilterDef(void);

	CollisionEffectFilterDef(
		unsigned int uiMaterialClass,
		unsigned int uiMassClass,
		float fBounceMinProjVel,
		float fBounceMinInterval,
		float fCrashMinProjVel,
		float fCrashMinInterval,
		float fSlideMinProjVel,
		float fSlideTimeout,
		float fRollMinAngVel,
		float fRollTimeout,
		bool bEnableBounce,
		bool bEnableCrash,
		bool bEnableSlide,
		bool bEnableRoll,
		bool bEnableDebug);

	~CollisionEffectFilterDef(void);

	void PostConstruct(void);
	bool EditorChangeValue(CallBackParameter obItem, CallBackParameter obValue);

	// Serialised data...
	uint64_t m_uiMaterialType;			//!< Base material classification - this is used as a fallback if a surface on this object does not have a material
	unsigned int m_uiMassType;			//!< Base mass classification

	float m_fBounceMinProjVel;			//!< Min projected velocity for bounce effect
	float m_fBounceMinInterval;			//!< Min time between bounce effects

	float m_fCrashMinProjVel;			//!< Min projected velocity for crash effect
	float m_fCrashMinInterval;			//!< Min time between crash effects

	float m_fSlideMinProjVel;			//!< Min projected velocity for slide effect
	float m_fSlideTimeout;				//!< Duration after which slide effect lapses

	float m_fRollMinAngVel;				//!< Min projected (angular) velocity for roll effect
	float m_fRollTimeout;				//!< Duration after which roll effect lapses

	bool  m_bEnableBounce;				//!< Toggles bounce effects
	bool  m_bEnableCrash;				//!< Toggles crash effects
	bool  m_bEnableSlide;				//!< Toggles slide effects
	bool  m_bEnableRoll;				//!< Toggles roll effects

	bool  m_bEnableDebug;				//!< Toggles debug

	ntstd::String m_obMaterialType;		//!< String represenation for m_uiMaterialType
	CHashedString m_obMassType;			//!< String representation for m_uiMassType

#ifdef _COLLISION_EFFECT_DEBUG
protected:
	void DebugIntegrityCheck(void);
#endif // _COLLISION_EFFECT_DEBUG
};




//------------------------------------------------------------------------------------------
//!	@class CollisionEffectHandler
//!	@brief Handles collision effects for an entity.
//!	Deals with bounce, crash, slid and roll collisions for an entity triggering and
//!	maintains the appropriate effects as required.
//------------------------------------------------------------------------------------------
class CollisionEffectHandler
{
public:
	//--------------------------------------------------------------------------------------
	//!	@class CollisionEffectHandlerEvent
	//!	@brief Reports information about an interaction involving this collision effect
	//!	handler and another.
	//--------------------------------------------------------------------------------------
	class CollisionEffectHandlerEvent
	{
	public:
		//!	Default ctor. Intialises collision effect handler event parameters.
		CollisionEffectHandlerEvent()
		:
			m_eType(CollisionEffectManager::ANY_COLLISION),
			m_fRelevantVelocity(0.0f),
			m_uiCustomMaterial1(CollisionEffectManager::INVALID_MATERIAL),
			m_uiCustomMaterial2(CollisionEffectManager::INVALID_MATERIAL),
			m_uiCustomMass1(CollisionEffectManager::INVALID_MASS),
			m_uiCustomMass2(CollisionEffectManager::INVALID_MASS)
		{}

		CollisionEffectHandler* m_pobOther;				//!< Other body
		CollisionEffectManager::COLLISION_TYPE m_eType;	//!< Collision type
		float m_fRelevantVelocity;						//!< Projected or angular velocity (should always be positive!)
		CPoint m_obCollisionPosition;					//!< Origin of interaction

		// Custom material and mass setting override those available from the other body's filter defintion
		uint64_t m_uiCustomMaterial1;				//!< Specific point material 1 (overrides base setting on filter definition)
		uint64_t m_uiCustomMaterial2;				//!< Specific point material 2 (overrides base setting on filter definition)
		unsigned int m_uiCustomMass1;					//!< Specific point mass 1 (overrides base setting on filter definition)
		unsigned int m_uiCustomMass2;					//!< Specific point mass 2 (overrides base setting on filter definition)
	};

	static void Initialise(void);
	static void Deinitialise(void);

	CollisionEffectHandler(void);
	~CollisionEffectHandler(void);

	void SetDefName(const CHashedString& obDefName, bool bKeepLooking);
	CHashedString GetDefName(void);
	CollisionEffectFilterDef* GetDef(void);

	bool IsValid(void);

	void Update (float fTimeDelta);

	bool ProcessEvent(const CollisionEffectHandlerEvent& obEvent);


protected:
	bool LookupDef(void);
	void SetDef(CollisionEffectFilterDef* pobDef);
	CollisionEffectDef* GetEffectDef(const CollisionEffectHandlerEvent& obEvent);
	bool MaintainContinuousEffect(const CollisionEffectHandlerEvent& obEvent);
	void StopContinuousEffect(void);
	bool ContinuousEffectActive(void);

	//!	Retrieves current impact partner.
	//! @return Collision effect handler for any current impact collision entity partner. May be null.
	CollisionEffectHandler* GetCurrentImpactPartner(void) {return m_pobCurImpactPartner;}

	//!	Retrieves current continuous partner.
	//! @return Collision effect handler for any current continuous collision entity partner. May be null.
	CollisionEffectHandler* GetCurrentContinuousPartner(void) {return m_pobCurContinuousPartner;}


private:
	static const CollisionEffectFilterDef* s_pobSURROGATE;			//!< Stand in filter definition for environments etc.

	CHashedString m_obCollisionFilterDefName;						//!< Name of associated collision filter definition
	CollisionEffectFilterDef* m_pobCollisionFilterDef;				//!< Associated collision filter definition
	CollisionEffectHandler* m_pobCurImpactPartner;					//!< Handler for the last entity involved in an impact collision with this one
	CollisionEffectHandler* m_pobCurContinuousPartner;				//!< Handler for the last entity involved in a continuous collision with this one
	float m_fImpactDelay;											//!< Remaining time until impact minimum interval expires (applies to bounce and crash sounds)
	float m_fContinuousTimeout;										//!< Remaining time until current continuous sound expires (applies to rolls and slides)
	unsigned int m_uiContinuousSoundId;								//!< Rolling or sliding sound identifier
	CollisionEffectManager::COLLISION_TYPE	m_eContinuousCollision;	//!< Collision type (e.g. rolling or sliding) associated with current continuous sound. Should only ever be SLIDE or ROLL, with ANY_COLLISION representing none.


#ifdef _COLLISION_EFFECT_DEBUG
protected:
	void DebugRefreshInfo(const CollisionEffectHandlerEvent* pobEvent, const CollisionEffectDef* pobDef);
	void DebugResetInfo(void);

private:
	static const int s_iDebugInfoSize;
	char* m_pcDebugImpactInfo;
	char* m_pcDebugContinuousInfo;
	float m_fDebugImpactInfoTimeout;
	float m_fDebugContinuousInfoTimeout;
	CPoint m_obDebugImpactInfoPos;
	CPoint m_obDebugContinuousInfoPos;
	uint32_t m_uiDebugImpactInfoCol;
	uint32_t m_uiDebugContinuousInfoCol;
#endif // _COLLISION_EFFECT_DEBUG
};


#endif // _COLLISIONEFFECTHANDLER_H_
