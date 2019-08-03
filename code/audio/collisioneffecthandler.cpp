//--------------------------------------------------------------------------------------
//!	@file collisioneffecthandler.cpp
//!	@author Chip Bell (SCEE), Harvey Cotton
//!	@date 31.08.06
//!
//!	@brief Implementation of the CollisionEffectHandler and related classes.
//--------------------------------------------------------------------------------------


#include "audio/collisioneffecthandler.h"

#include <string.h>

#include "audio/audiosystem.h"
#include "objectdatabase/dataobject.h"

#include "physics/physicsmaterial.h"

#ifdef _COLLISION_EFFECT_DEBUG
#include "core/visualdebugger.h"
#include <stdio.h>
#endif // _COLLISION_EFFECT_DEBUG



#define DEFAULT_MATERIAL_TYPE			CollisionEffectManager::ANY_MATERIAL
#define DEFAULT_MATERIAL_TYPE_STR		"ANY_MATERIAL"
#define DEFAULT_MASS_TYPE				CollisionEffectManager::ANY_MASS
#define DEFAULT_MASS_TYPE_STR			"ANY_MASS"

#define DEFAULT_BOUNCE_MIN_PROJ_VEL		0.5f
#define DEFAULT_BOUNCE_MIN_INTERVAL		0.25f

#define DEFAULT_CRASH_MIN_PROJ_VEL		0.5f
#define DEFAULT_CRASH_MIN_INTERVAL		0.25f

#define DEFAULT_SLIDE_MIN_PROJ_VEL		0.25f
#define DEFAULT_SLIDE_TIMEOUT			0.5f

#define DEFAULT_ROLL_MIN_ANG_VEL		0.25f
#define DEFAULT_ROLL_TIMEOUT			0.5f

#define DEFAULT_ENABLE_BOUNCE			true
#define DEFAULT_ENABLE_CRASH			true
#define DEFAULT_ENABLE_SLIDE			true
#define DEFAULT_ENABLE_ROLL				true


#ifndef DEFAULT_ENABLE_DEBUG
#define DEFAULT_ENABLE_DEBUG			false
#else
// Such a common definition may already exist...
#error Identifier collision: DEFAULT_ENABLE_DEBUG already defined.
#endif // DEFAULT_ENABLE_DEBUG


#ifdef _COLLISION_EFFECT_DEBUG

#define DEBUG_REFRESH_INFO(pobEvent, pobDef)	\
	DebugRefreshInfo(pobEvent, pobDef)

#define DEBUG_IMPACT_INFO_TIMEOUT_BOUNCE		3.0f
#define DEBUG_IMPACT_INFO_TIMEOUT_CRASH			4.0f
#define DEBUG_CONTINUOUS_INFO_TIMEOUT_SLIDE		3.0f
#define DEBUG_CONTINUOUS_INFO_TIMEOUT_ROLL		3.0f
#define DEBUG_IMPACT_INFO_COLOUR_BOUNCE			DC_CYAN
#define DEBUG_IMPACT_INFO_COLOUR_CRASH			DC_PURPLE
#define DEBUG_IMPACT_INFO_COLOUR_MISSING		DC_WHITE
#define DEBUG_CONTINUOUS_INFO_COLOUR_SLIDE		DC_YELLOW
#define DEBUG_CONTINUOUS_INFO_COLOUR_ROLL		DC_RED
#define DEBUG_CONTINUOUS_INFO_COLOUR_MISSING	DC_WHITE
#define DEBUG_IMPACT_INFO_COLOUR_SURROGATE		DC_GREY

const int CollisionEffectHandler::s_iDebugInfoSize = 64;
#else

#define DEBUG_REFRESH_INFO(pobEvent, pobDef)	\
	{}

#endif // _COLLISION_EFFECT_DEBUG


const CollisionEffectFilterDef* CollisionEffectHandler::s_pobSURROGATE = 0;


START_STD_INTERFACE						( CollisionEffectFilterDef )
	PUBLISH_VAR_AS						( m_obMaterialType,		MaterialType )
	PUBLISH_VAR_AS						( m_obMassType,			MassType )

	PUBLISH_VAR_AS						( m_fBounceMinProjVel,	BounceMinProjVel )
	PUBLISH_VAR_AS						( m_fBounceMinInterval,	BounceMinInterval )

	PUBLISH_VAR_AS						( m_fCrashMinProjVel,	CrashMinProjVel )
	PUBLISH_VAR_AS						( m_fCrashMinInterval,	CrashMinInterval )

	PUBLISH_VAR_AS						( m_fSlideMinProjVel,	SlideMinProjVel )
	PUBLISH_VAR_AS						( m_fSlideTimeout,		SlideTimeout )

	PUBLISH_VAR_AS						( m_fRollMinAngVel,		RollMinAngVel )
	PUBLISH_VAR_AS						( m_fRollTimeout,		RollTimeout )

	PUBLISH_VAR_AS						( m_bEnableBounce,		EnableBounce )
	PUBLISH_VAR_AS						( m_bEnableCrash,		EnableCrash )
	PUBLISH_VAR_AS						( m_bEnableSlide,		EnableSlide )
	PUBLISH_VAR_AS						( m_bEnableRoll,		EnableRoll )

	PUBLISH_VAR_AS						( m_bEnableDebug,		EnableDebug )

	PUBLISH_VAR_AS						( m_uiMaterialType,		MaterialType_Code )
	PUBLISH_VAR_AS						( m_uiMassType,			MassType_Code )

	DECLARE_POSTCONSTRUCT_CALLBACK		( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK	( EditorChangeValue )
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//	CollisionEffectFilterDef (ctor)
//!	Default constructor. Initialises collision effect filter definition members to their
//!	defaults.
//!	@note Because of the hased string members, it seems filter definitions cannot be constant
//!	static members of a class (critical section violation at runtime).
//!	@sa CollisionEffectHandler::Initialise(void)
//------------------------------------------------------------------------------------------
CollisionEffectFilterDef::CollisionEffectFilterDef(void)
:
	m_uiMaterialType(DEFAULT_MATERIAL_TYPE),
	m_uiMassType(DEFAULT_MASS_TYPE),
	m_fBounceMinProjVel(DEFAULT_BOUNCE_MIN_PROJ_VEL),
	m_fBounceMinInterval(DEFAULT_BOUNCE_MIN_INTERVAL),
	m_fCrashMinProjVel(DEFAULT_CRASH_MIN_PROJ_VEL),
	m_fCrashMinInterval(DEFAULT_CRASH_MIN_INTERVAL),
	m_fSlideMinProjVel(DEFAULT_SLIDE_MIN_PROJ_VEL),
	m_fSlideTimeout(DEFAULT_SLIDE_TIMEOUT),
	m_fRollMinAngVel(DEFAULT_ROLL_MIN_ANG_VEL),
	m_fRollTimeout(DEFAULT_ROLL_TIMEOUT),
	m_bEnableBounce(DEFAULT_ENABLE_BOUNCE),
	m_bEnableCrash(DEFAULT_ENABLE_CRASH),
	m_bEnableSlide(DEFAULT_ENABLE_SLIDE),
	m_bEnableRoll(DEFAULT_ENABLE_ROLL),

	m_bEnableDebug(DEFAULT_ENABLE_DEBUG)

#ifdef _COLLISION_EFFECT_DEBUG
	// String types must be intialised to corresponding values
	,
	m_obMaterialType(DEFAULT_MATERIAL_TYPE_STR),
	m_obMassType(DEFAULT_MASS_TYPE_STR)
#endif // _COLLISION_EFFECT_DEBUG
{ }


//------------------------------------------------------------------------------------------
//	CollisionEffectFilterDef (ctor)
//!	Alternate constructor. Initialises collision effect filter definition members to specific
//!	values.
//!	@note Because of the hased string members, it seems filter definitions cannot be constant
//!	static members of a class (critical section violation at runtime).
//!	@sa CollisionEffectHandler::Initialise(void)
//------------------------------------------------------------------------------------------
CollisionEffectFilterDef::CollisionEffectFilterDef(
	unsigned int uiMaterialType,
	unsigned int uiMassType,
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
	bool bEnableDebug)
:
	m_uiMaterialType(uiMaterialType),
	m_uiMassType(uiMassType),
	m_fBounceMinProjVel(fBounceMinProjVel),
	m_fBounceMinInterval(fBounceMinInterval),
	m_fCrashMinProjVel(fCrashMinProjVel),
	m_fCrashMinInterval(fCrashMinInterval),
	m_fSlideMinProjVel(fSlideMinProjVel),
	m_fSlideTimeout(fSlideTimeout),
	m_fRollMinAngVel(fRollMinAngVel),
	m_fRollTimeout(fRollTimeout),
	m_bEnableBounce(bEnableBounce),
	m_bEnableCrash(bEnableCrash),
	m_bEnableSlide(bEnableSlide),
	m_bEnableRoll(bEnableRoll),

	m_bEnableDebug(bEnableDebug)

#ifdef _COLLISION_EFFECT_DEBUG
	// String types must be intialised to corresponding values
	,
	//m_obMaterialType(CollisionEffectManager::VolumeMaterialBitfieldToString(m_uiMaterialType).c_str()),
	m_obMassType(CollisionEffectManager::VolumeMassBitfieldToString(m_uiMassType).c_str())
#endif // _COLLISION_EFFECT_DEBUG
{ }


//------------------------------------------------------------------------------------------
//	~CollisionEffectFilterDef (dtor)
//!	Default destructor.
//------------------------------------------------------------------------------------------
CollisionEffectFilterDef::~CollisionEffectFilterDef(void)
{ }


//------------------------------------------------------------------------------------------
//	PostConstruct
//------------------------------------------------------------------------------------------
void CollisionEffectFilterDef::PostConstruct(void)
{
	// HC: For time being, need to recalculate everything here - This means you will need to load the game on debug/development and save out the physics effect xml
	
#ifndef _RELEASE
	m_uiMaterialType=Physics::PhysicsMaterialTable::Get().GetEffectMaterialBitfield(m_obMaterialType.c_str());
	m_uiMassType=CollisionEffectManager::ParseBitfieldString(m_obMassType.GetDebugString(), CollisionEffectManager::VolumeMassFromString);
#endif // _RELEASE

/*
// HC: Disabled since we are recalculating everything anyway

#ifdef _COLLISION_EFFECT_DEBUG
	DebugIntegrityCheck();
#endif // _COLLISION_EFFECT_DEBUG
*/
}


//------------------------------------------------------------------------------------------
//	EditorChangeValue
//!	Debug editor callback.
//!	@param obItem	Name of item which has been changed.
//!	@param obValue	New value.
//!	@return True if notification processed successfully, false otherwise (indicates default
//!	change behavior should not proceed).
//------------------------------------------------------------------------------------------
bool CollisionEffectFilterDef::EditorChangeValue(CallBackParameter obItem, CallBackParameter obValue)
{
/*
#ifdef _COLLISION_EFFECT_DEBUG
	
	bool bReflect = false;

	// Special processing for string items
	CHashedString obItemHash(obItem);
	CHashedString obItemValue(obValue);
	if (CHashedString(HASH_STRING_MATERIALTYPE) == obItemHash)
	{
		//m_uiMaterialType = CollisionEffectManager::ParseBitfieldString(obItemValue.GetDebugString(), CollisionEffectManager::VolumeMaterialFromString);
		m_uiMaterialType=Physics::PhysicsMaterialTable::Get().GetEffectMaterialBitfield(m_obMaterialType.c_str());
		ntPrintf("MaterialType_Code = %d (%s)\n", m_uiMaterialType, obItemValue.GetDebugString());
		bReflect = true;
	}
	if (CHashedString(HASH_STRING_MASSTYPE) == obItemHash)
	{
		m_uiMassType = CollisionEffectManager::ParseBitfieldString(obItemValue.GetDebugString(), CollisionEffectManager::VolumeMassFromString);
		ntPrintf("MassType_Code = %d (%s)\n", m_uiMassType, obItemValue.GetDebugString());
		bReflect = true;
	}
	
	if (bReflect)
	{
		DataObject* pobThis = ObjectDatabase::Get().GetDataObjectFromPointer(this);
		if (pobThis)
			ObjectDatabase::Get().SignalObjectChange(pobThis);

		ntPrintf("Collision Effect Filter Definition bitfields updated. (May need to refresh Welder.)\n");
	}

#else
	UNUSED(obItem);
	UNUSED(obValue);
#endif // _COLLISION_EFFECT_DEBUG
*/

	return true;
}


//------------------------------------------------------------------------------------------
//	Deinitialise
//!	Initialises collision effect handler system.
//!	@note This is required for the surrogate collision effect filter.
//!	@sa CollisionEffectHandler::Deinitialise(void)
//!	@sa CollisionEffectFilterDef::CollisionEffectFilterDef(void)
//------------------------------------------------------------------------------------------
void CollisionEffectHandler::Initialise(void)
{
	if (!s_pobSURROGATE)
	{
		s_pobSURROGATE = NT_NEW CollisionEffectFilterDef(
			DEFAULT_MATERIAL_TYPE,
			CollisionEffectManager::WORLD,
			DEFAULT_BOUNCE_MIN_PROJ_VEL,
			DEFAULT_BOUNCE_MIN_INTERVAL,
			DEFAULT_CRASH_MIN_PROJ_VEL,
			DEFAULT_CRASH_MIN_INTERVAL,
			DEFAULT_SLIDE_MIN_PROJ_VEL,
			DEFAULT_SLIDE_TIMEOUT,
			DEFAULT_ROLL_MIN_ANG_VEL,
			DEFAULT_ROLL_TIMEOUT,
			DEFAULT_ENABLE_BOUNCE,
			DEFAULT_ENABLE_CRASH,
			DEFAULT_ENABLE_SLIDE,
			DEFAULT_ENABLE_ROLL,
			DEFAULT_ENABLE_DEBUG);
	}
}


//------------------------------------------------------------------------------------------
//	Deinitialise
//!	Deinitialises collision effect handler system.
//!	@sa CollisionEffectHandler::Initialise(void)
//!	@sa CollisionEffectFilterDef::CollisionEffectFilterDef(void)
//------------------------------------------------------------------------------------------
void CollisionEffectHandler::Deinitialise(void)
{
	if (s_pobSURROGATE)
	{
		delete s_pobSURROGATE;
		s_pobSURROGATE = 0;
	}
}


//------------------------------------------------------------------------------------------
//	CollisionEffectHandler (ctor)
//!	Default constructor. Initialises collision effect handler members and optionally sets
//!	associated definition object.
//!	@note Object will not be valid for use until an associated collision effect handler
//!	definition has been set.
//!	@sa CollisionEffectHandler::SetDef(CollisionEffectFilterDef* pobDef)
//------------------------------------------------------------------------------------------
CollisionEffectHandler::CollisionEffectHandler(void)
:
	m_pobCollisionFilterDef(0),
	m_pobCurImpactPartner(0),
	m_pobCurContinuousPartner(0),
	m_fImpactDelay(0.0f),
	m_fContinuousTimeout(0.0f),
	m_uiContinuousSoundId(0),
	m_eContinuousCollision(CollisionEffectManager::ANY_COLLISION)
#ifdef _COLLISION_EFFECT_DEBUG
	,
	m_pcDebugImpactInfo(NULL),
	m_pcDebugContinuousInfo(NULL),
	m_fDebugImpactInfoTimeout(0.0f),
	m_fDebugContinuousInfoTimeout(0.0f),
	m_uiDebugImpactInfoCol(0),
	m_uiDebugContinuousInfoCol(0)
#endif // _COLLISION_EFFECT_DEBUG
{
#ifdef _COLLISION_EFFECT_DEBUG
	DebugResetInfo();
#endif // _COLLISION_EFFECT_DEBUG
}


//------------------------------------------------------------------------------------------
//	~CollisionEffectHandler (dtor)
//!	Default destructor. Deinitialises collision effect handler, cleaning up as necessary.
//------------------------------------------------------------------------------------------
CollisionEffectHandler::~CollisionEffectHandler()
{
	// Cleanup
	StopContinuousEffect();

#ifdef _COLLISION_EFFECT_DEBUG
	delete[] m_pcDebugImpactInfo;
	delete[] m_pcDebugContinuousInfo;
#endif // _COLLISION_EFFECT_DEBUG
}


//------------------------------------------------------------------------------------------
//	SetDefName
//!	Assocates a collision effect filter definition name with this collision effect handler.
//!	@param obDefName	Definition object name in object database. Can be a null string (this
//!	will remove the associated definition).
//!	@param bKeepLooking	Indicates if the named object is not found, it should be repeatedly
//!	looked for on subsequent updates. Ignored if removing definition. If definition object is
//!	not found by name and keep looking is not requested, all associated collision effect
//!	filter definition information will be nullified.
//!	@note Any currently set definition assocation will be lost.
//!	@sa CollisionEffectHandler::SetDef(CollisionEffectFilterDef* pobDef)
//!	@sa CollisionEffectHandler::LookupDef(void)
//------------------------------------------------------------------------------------------
void CollisionEffectHandler::SetDefName(const CHashedString& obDefName, bool bKeepLooking)
{
	SetDef(0);

	m_obCollisionFilterDefName = obDefName;
	if (!LookupDef() && !bKeepLooking)
	{
		m_obCollisionFilterDefName = CHashedString::nullString;
	}
}


//------------------------------------------------------------------------------------------
//	LookupDef
//!	Looks up currently associated collision filter definition name in the object database,
//!	and sets the collision filter definition pointer if possible.
//!	@return True if an appropriate object is found and set (if name is null, no object will
//!	be set, but with a successful return).
//!	@note Any currently set definition assocation will be lost.
//!	@sa CollisionEffectHandler::SetDefName(const CHashedString& obName)
//!	@sa CollisionEffectHandler::SetDef(CollisionEffectFilterDef* pobDef)
//------------------------------------------------------------------------------------------
bool CollisionEffectHandler::LookupDef(void)
{
	if (m_obCollisionFilterDefName.IsNull())
	{
		SetDef(0);
		return !m_pobCollisionFilterDef;
	}

	SetDef(ObjectDatabase::Get().GetPointerFromName<CollisionEffectFilterDef*>(m_obCollisionFilterDefName));
	return m_pobCollisionFilterDef;
}


//------------------------------------------------------------------------------------------
//	SetDef
//!	Assocates a collision effect filter definition with this collision effect handler.
//!	@param pobDef	Pointer to associated definition object. Can be null.
//!	@note Stops all continuous sounds if set collision effect filter definition is changed.
//!	@note Has no effect if currently set collision effect filter definition matches request.
//!	@sa CollisionEffectHandler::SetDefName(const CHashedString& obName)
//!	@sa CollisionEffectHandler::LookupDef(void)
//------------------------------------------------------------------------------------------
void CollisionEffectHandler::SetDef(CollisionEffectFilterDef* pobDef)
{
	if (m_pobCollisionFilterDef == pobDef)
		return;

	m_pobCollisionFilterDef = pobDef;

	// If the definition is changed, some cleanup is req'd
	StopContinuousEffect();
}


//------------------------------------------------------------------------------------------
//	GetDef
//!	Retrieves associated collision effect filter definition.
//!	@return Pointer to associated definition object, or 0 if one is not set.
//------------------------------------------------------------------------------------------
CollisionEffectFilterDef* CollisionEffectHandler::GetDef(void)
{
	return m_pobCollisionFilterDef;
}


//------------------------------------------------------------------------------------------
//	IsValid
//!	@return True if collision effect handler is ready to process collision events, false
//!	otherwise.
//------------------------------------------------------------------------------------------
bool CollisionEffectHandler::IsValid(void)
{
	return (bool)m_pobCollisionFilterDef;
}


//------------------------------------------------------------------------------------------
//	Update
//!	Updates collision effect handler.
//!	@param fTimeDelta	Time since this update was last called (seconds).
//------------------------------------------------------------------------------------------
void CollisionEffectHandler::Update(float fTimeDelta)
{
	if (!m_pobCollisionFilterDef)
	{
		// Check if currently looking for an associated definition object
		if (!m_obCollisionFilterDefName.IsNull())
		{
			LookupDef();
			// No need to proceed below, as there should be nothing to update...
		}

		return;
	}

	// Handle impact delay
	if (m_fImpactDelay > 0.0f && (m_fImpactDelay -= fTimeDelta) < 0.0f)
	{
		m_fImpactDelay = 0.0f;
		m_pobCurImpactPartner = 0;
	}

	// Handle continuous sound timeout
	if (m_uiContinuousSoundId != 0)
	{
		ntAssert_p(m_pobCollisionFilterDef, ("Collision effect handler playing a continuous sound without an associated filter definition!"));

		if ((m_fContinuousTimeout -= fTimeDelta) < 0.0f)
		{
			StopContinuousEffect();
		}
	}

#ifdef _COLLISION_EFFECT_DEBUG
	// Debug info rendering
	if (!m_pobCollisionFilterDef->m_bEnableDebug)
		return;

	if (m_fDebugImpactInfoTimeout > 0.0f)
	{
		if ((m_fDebugImpactInfoTimeout -= fTimeDelta) < 0.0f)
			m_fDebugImpactInfoTimeout = 0.0f;

		g_VisualDebug->Printf3D(m_obDebugImpactInfoPos, 0.0f,0.0f, m_uiDebugImpactInfoCol, 0, m_pcDebugImpactInfo);
	}
	if (m_fDebugContinuousInfoTimeout > 0.0f)
	{
		m_fDebugContinuousInfoTimeout -= fTimeDelta;

		g_VisualDebug->Printf3D(m_obDebugContinuousInfoPos, 0.0f,0.0f, m_uiDebugContinuousInfoCol, 0, m_pcDebugContinuousInfo);
	}
#endif // _COLLISION_EFFECT_DEBUG
}


//------------------------------------------------------------------------------------------
//	ProcessEvent
//!	Handles audio for a collision.
//!	@param obEvent	Related collision parameters.
//!	@return True on success (a sound has been triggered or update), false otherwise.
//!	@note Ideally this function should only be used to process collision events invovling
//!	the owning entity of this collision effect handler. By design: a collision effect handler
//!	maintains only a single continuous sound identifier (for rolling/sliding sounds etc.),
//!	and prevents impact sound overlaps according to the parameters set on the associated
//!	definition object.
//------------------------------------------------------------------------------------------
bool CollisionEffectHandler::ProcessEvent(const CollisionEffectHandlerEvent& obEvent)
{
	// Verify
	if (!m_pobCollisionFilterDef || !CollisionEffectManager::Get().CollisionEffectsEnabled())
		return false;

	// Process collision type
	switch (obEvent.m_eType)
	{
	case CollisionEffectManager::ANY_COLLISION:
		// No processing
		return false;
	// Single shot sounds (impacts)
	case CollisionEffectManager::BOUNCE:
	case CollisionEffectManager::CRASH:
		{
			// Prevent double-taps
			if (obEvent.m_pobOther && obEvent.m_pobOther->GetCurrentImpactPartner() == this)
				// Partner handler already processing an impact collision involving this one
				return false;

			// Do not proceed during impact delay
			if (m_fImpactDelay > 0.0f)
				return false;

			float fImpactDelay = 0.0f;

			// Verify collision type enabled and velocity threshold met
			if (obEvent.m_eType == CollisionEffectManager::BOUNCE)
			{
				if (!m_pobCollisionFilterDef->m_bEnableBounce
					|| obEvent.m_fRelevantVelocity < m_pobCollisionFilterDef->m_fBounceMinProjVel)
						return false;

				// Set scaling velocities and impact delay
				fImpactDelay = m_pobCollisionFilterDef->m_fBounceMinInterval;
			}
			else // if (obEvent.m_eType == CollisionEffectManager::CRASH)
			{
				if (!m_pobCollisionFilterDef->m_bEnableCrash
					|| obEvent.m_fRelevantVelocity < m_pobCollisionFilterDef->m_fCrashMinProjVel)
						return false;

				// Set scaling velocities and impact delay
				fImpactDelay = m_pobCollisionFilterDef->m_fCrashMinInterval;
			}

			// Obtain most suitable collision effect definition
			CollisionEffectDef* pobDef = GetEffectDef(obEvent);
			if (!pobDef)
			{
				// Update debug info (even for invalid collision defs)
				DEBUG_REFRESH_INFO(&obEvent, pobDef);

				return false;
			}

			// Prepare sound effect
			unsigned int uiSoundId = 0;
			if (!AudioSystem::Get().Sound_Prepare(uiSoundId, pobDef->m_obSound.c_str()))
				return false;

			// Parameters
			AudioSystem::Get().Sound_SetParameterVelocity(uiSoundId, "projectedVelocity", obEvent.m_fRelevantVelocity);

			// Position and play sound
			AudioSystem::Get().Sound_SetPosition(uiSoundId, obEvent.m_obCollisionPosition);
			AudioSystem::Get().Sound_Play(uiSoundId);

			// Set minimum impact delay time and impact partner
			m_fImpactDelay = fImpactDelay;
			m_pobCurImpactPartner = obEvent.m_pobOther;


			// TODO particle effects, etc.
			// NOTE No error returns beyond this point - a sound has been played (above)


			// Update debug info
			DEBUG_REFRESH_INFO(&obEvent, pobDef);
		}
		break;
	// Looping sounds (continuous collisions)
	case CollisionEffectManager::SLIDE:
	case CollisionEffectManager::ROLL:
		{
			// Continuous effect control handled separately (this will update debug info)
			if (!MaintainContinuousEffect(obEvent))
				return false;

			// NOTE No error returns beyond this point - a sound has been played (above)
		}
		break;
	}

	// Success
	return true;
}


//------------------------------------------------------------------------------------------
//	GetEffectDef
//!	Retrieves an appropriate collision effect definition for a supplied collision event.
//!	@param obEvent	Details physical interaction.
//!	@return Pointer to collision effect definition, or 0 if not found.
//------------------------------------------------------------------------------------------
CollisionEffectDef* CollisionEffectHandler::GetEffectDef(const CollisionEffectHandlerEvent& obEvent)
{
	// Validate
	if (!m_pobCollisionFilterDef || !obEvent.m_pobOther)
		return 0;

	// If there is no definition for the other party, use the surrogate definition.
	// This is usually required for the environment etc.
	const CollisionEffectFilterDef* pobOtherDef = obEvent.m_pobOther->GetDef();
	if (!pobOtherDef)
		pobOtherDef = s_pobSURROGATE;
	ntAssert_p(pobOtherDef, ("Collision Effect Handler system has not been initialised!"));

	// Fill out manager event, noting any custom overrides as applicable
	CollisionEffectManager::CollisionEffectManagerEvent obMgrEvent;
	obMgrEvent.m_uiType = obEvent.m_eType;
	obMgrEvent.m_uiMass1 = 
		obEvent.m_uiCustomMass1 == CollisionEffectManager::INVALID_MASS
		? m_pobCollisionFilterDef->m_uiMassType
		: obEvent.m_uiCustomMass1;
	obMgrEvent.m_uiMass2 =
		obEvent.m_uiCustomMass2 == CollisionEffectManager::INVALID_MASS
		? pobOtherDef->m_uiMassType
		: obEvent.m_uiCustomMass2;
	obMgrEvent.m_uiMaterial1 = 
		obEvent.m_uiCustomMaterial1 == CollisionEffectManager::INVALID_MATERIAL
		? m_pobCollisionFilterDef->m_uiMaterialType
		: obEvent.m_uiCustomMaterial1;
	obMgrEvent.m_uiMaterial2 = obEvent.m_uiCustomMaterial2 == CollisionEffectManager::INVALID_MATERIAL
		? pobOtherDef->m_uiMaterialType
		: obEvent.m_uiCustomMaterial2;

	// Lookup matching effect definition via manger
	CollisionEffectDef* pobResult = CollisionEffectManager::Get().GetEffectDef(obMgrEvent);


#ifdef _COLLISION_EFFECT_DEBUG
	/*
	// Print failure info to TTY as req'd
	if (m_pobCollisionFilterDef->m_bEnableDebug && !pobResult)
	{
		ntPrintf("MISSING COLLISION EFFECT DEF\n");
		ntPrintf("\tType: %s (%u)\n\tMass 1: %s (%u)\n\tMass 2: %s (%u)\n\tMaterial 1: %s (%u)\n\tMaterial 2: %s (%u)\n\n",
			CollisionEffectManager::CollisionTypeBitfieldToString(obMgrEvent.m_uiType).c_str(),
			obMgrEvent.m_uiType,
			CollisionEffectManager::VolumeMassBitfieldToString(obMgrEvent.m_uiMass1).c_str(),
			obMgrEvent.m_uiMass1,
			CollisionEffectManager::VolumeMassBitfieldToString(obMgrEvent.m_uiMass2).c_str(),
			obMgrEvent.m_uiMass2,
			CollisionEffectManager::VolumeMaterialBitfieldToString(obMgrEvent.m_uiMaterial1).c_str(),
			obMgrEvent.m_uiMaterial1,
			CollisionEffectManager::VolumeMaterialBitfieldToString(obMgrEvent.m_uiMaterial2).c_str(),
			obMgrEvent.m_uiMaterial2);
	}
	*/
#endif // _COLLISION_EFFECT_DEBUG


	return pobResult;
}


//------------------------------------------------------------------------------------------
//	MaintainContinuousEffect
//!	Starts, changes, or potentially stops continuous effects (for rolls, slides, etc.) based
//!	on (new) collision information.
//!	@param obEvent	Related collision parameters.
//!	@return True if continuous effect started or continued, false otherwise.
//!	@note Continuous effect will be stopped if necessary thresholds are not met.
//------------------------------------------------------------------------------------------
bool CollisionEffectHandler::MaintainContinuousEffect(const CollisionEffectHandlerEvent& obEvent)
{
	ntAssert_p(0 != m_pobCollisionFilterDef, ("Attempt to start continuous sound with invalid collision effect filter definition!"))

	// Prevent double-taps
	if (obEvent.m_pobOther && obEvent.m_pobOther->GetCurrentContinuousPartner() == this)
		// Partner handler already processing a continuous collision involving this one
		return ContinuousEffectActive();
			// Don't stop any current continuous effect here, as it may be updated in time to
			// keep it alive (otherwise it will die of natural causes anyway)

	// Verify collision type enabled and velocity threshold met
	float fTimeout = 0.0f;
	if ((obEvent.m_eType == CollisionEffectManager::SLIDE
			&& (!m_pobCollisionFilterDef->m_bEnableSlide
				|| obEvent.m_fRelevantVelocity < m_pobCollisionFilterDef->m_fSlideMinProjVel
				|| (fTimeout = m_pobCollisionFilterDef->m_fSlideTimeout) <= 0.0f))
		|| (obEvent.m_eType == CollisionEffectManager::ROLL
			&& (!m_pobCollisionFilterDef->m_bEnableRoll
				|| obEvent.m_fRelevantVelocity < m_pobCollisionFilterDef->m_fRollMinAngVel
				|| (fTimeout = m_pobCollisionFilterDef->m_fRollTimeout) <= 0.0f)))
	{
		StopContinuousEffect();
		return false;
	}

	// Obtain most suitable collision effect definition
	CollisionEffectDef* pobDef = GetEffectDef(obEvent);
	if (!pobDef)
	{
		// Update debug info (even for invalid collision defs)
		DEBUG_REFRESH_INFO(&obEvent, pobDef);

		StopContinuousEffect();
		return false;
	}

	// Test if currently playing a continuous effect
	if (ContinuousEffectActive())
	{
		// Test if continuous effect type has changed
		if (obEvent.m_eType != m_eContinuousCollision)
			// TODO (chipb) Also test if effect definition has changed
		{
			StopContinuousEffect();
				// New effect sill be started below...
		}
		else
		{
			// Active collision effects should be updated
			// NOTE No error returns beyond this point

			// Update sound
			AudioSystem::Get().Sound_SetParameterVelocity(
				m_uiContinuousSoundId,
				obEvent.m_eType == CollisionEffectManager::ROLL ? "angularVelocity":"projectedVelocity",
				obEvent.m_fRelevantVelocity);
			AudioSystem::Get().Sound_SetPosition(m_uiContinuousSoundId, obEvent.m_obCollisionPosition);


			// TODO update particle effects, etc.


			// Current effect now updated
			m_fContinuousTimeout = fTimeout;
			m_pobCurContinuousPartner = obEvent.m_pobOther;

			// Update debug info
			DEBUG_REFRESH_INFO(&obEvent, pobDef);

			// Done
			return true;
		}
	}

	// No active collision effects, should be started at this point

	// Trigger sound
	if (!AudioSystem::Get().Sound_Prepare(m_uiContinuousSoundId, pobDef->m_obSound.c_str()))
	{
		m_uiContinuousSoundId = 0; // Possible to receive a valid identifier even on preparation failure
		return false;
	}

	// Position and play sound
	AudioSystem::Get().Sound_SetPosition(m_uiContinuousSoundId, obEvent.m_obCollisionPosition);
	AudioSystem::Get().Sound_Play(m_uiContinuousSoundId);

	// Parameters
	AudioSystem::Get().Sound_SetParameterVelocity(
		m_uiContinuousSoundId,
		obEvent.m_eType == CollisionEffectManager::ROLL ? "angularVelocity":"projectedVelocity",
		obEvent.m_fRelevantVelocity);


	// TODO particle effects, etc.
	// NOTE No error returns beyond this point - a sound has been played (above)


	// Continuous effects now active
	m_eContinuousCollision = obEvent.m_eType;
	m_fContinuousTimeout = fTimeout;
	m_pobCurContinuousPartner = obEvent.m_pobOther;

	// Update debug info
	DEBUG_REFRESH_INFO(&obEvent, pobDef);

	// Done
	return true;
}


//------------------------------------------------------------------------------------------
//	StopContinuousEffect
//!	Stops and resets any active continuous effect (for rolls, slides, etc.).
//------------------------------------------------------------------------------------------
void CollisionEffectHandler::StopContinuousEffect(void)
{
	// Verify
	if (ContinuousEffectActive())
		return;

	m_eContinuousCollision = CollisionEffectManager::ANY_COLLISION;
	m_fContinuousTimeout = 0.0f;
	m_pobCurContinuousPartner = 0;

	// Sound cleanup
	if (0 != m_uiContinuousSoundId)
	{
		AudioSystem::Get().Sound_Stop(m_uiContinuousSoundId);
		m_uiContinuousSoundId = 0;
	}


	// TODO particle effects cleanup, etc.
}


//------------------------------------------------------------------------------------------
//	ContinuousEffectActive
//!	Indicates whether or not a continuouss effect is currently active.
//!	@return True if a continous collision effect is currently active (e.g. a looping sound is
//!	playing), false otherwise.
//------------------------------------------------------------------------------------------
bool CollisionEffectHandler::ContinuousEffectActive(void)
{
	return CollisionEffectManager::ANY_COLLISION == m_eContinuousCollision;
}


#ifdef _COLLISION_EFFECT_DEBUG


//------------------------------------------------------------------------------------------
//	DebugRefreshInfo
//!	Debug only. Updates all debug related info structures typically in response to a newly
//!	started or updated collision effect.
//!	@param pobEvent	Associated handler event which has prompted effect. Passing null
//!	resets all collision info.
//!	@param pobDef	Retrieved effect definition (or null if one was not found).
//!	@note Has no effect if there is no associated collision effect filter definition or if
//!	debug is disabled on said definition.
//------------------------------------------------------------------------------------------
void CollisionEffectHandler::DebugRefreshInfo(const CollisionEffectHandlerEvent* pobEvent, const CollisionEffectDef* pobDef)
{
	if (!m_pobCollisionFilterDef || !m_pobCollisionFilterDef->m_bEnableDebug)
		return; 

	if (!pobEvent)
	{
		DebugResetInfo();
		return;
	}

	switch (pobEvent->m_eType)
	{
	case CollisionEffectManager::ANY_COLLISION:
		break;
	case CollisionEffectManager::BOUNCE:
		if (pobDef)
		{
			DataObject* pobDefDO = ObjectDatabase::Get().GetDataObjectFromPointer(pobDef);
			const char* pcDefName = pobDefDO ? pobDefDO->GetName().GetString():"UNKNOWN";

			sprintf(m_pcDebugImpactInfo, "%s:%s (b)", pobDef->m_obSound.c_str(), pcDefName);
			m_uiDebugImpactInfoCol = (!pobEvent->m_pobOther || !pobEvent->m_pobOther->GetDef()) ? DEBUG_IMPACT_INFO_COLOUR_SURROGATE:DEBUG_IMPACT_INFO_COLOUR_BOUNCE;
		}
		else
		{
			sprintf(m_pcDebugImpactInfo, "BOUNCE");
			m_uiDebugImpactInfoCol = DEBUG_IMPACT_INFO_COLOUR_MISSING;
		}
		m_fDebugImpactInfoTimeout = DEBUG_IMPACT_INFO_TIMEOUT_BOUNCE;
		m_obDebugImpactInfoPos = pobEvent->m_obCollisionPosition;
		break;
	case CollisionEffectManager::CRASH:
		if (pobDef)
		{
			DataObject* pobDefDO = ObjectDatabase::Get().GetDataObjectFromPointer(pobDef);
			const char* pcDefName = pobDefDO ? pobDefDO->GetName().GetString():"UNKNOWN";

			sprintf(m_pcDebugImpactInfo, "%s:%s (c)", pobDef->m_obSound.c_str(), pcDefName);
			m_uiDebugImpactInfoCol = (!pobEvent->m_pobOther || !pobEvent->m_pobOther->GetDef()) ? DEBUG_IMPACT_INFO_COLOUR_SURROGATE:DEBUG_IMPACT_INFO_COLOUR_CRASH;
		}
		else
		{
			sprintf(m_pcDebugImpactInfo, "CRASH");
			m_uiDebugImpactInfoCol = DEBUG_IMPACT_INFO_COLOUR_MISSING;
		}
		m_fDebugImpactInfoTimeout = DEBUG_IMPACT_INFO_TIMEOUT_CRASH;
		m_obDebugImpactInfoPos = pobEvent->m_obCollisionPosition;
		break;
	case CollisionEffectManager::SLIDE:
		if (pobDef)
		{
			DataObject* pobDefDO = ObjectDatabase::Get().GetDataObjectFromPointer(pobDef);
			const char* pcDefName = pobDefDO ? pobDefDO->GetName().GetString():"UNKNOWN";

			sprintf(m_pcDebugContinuousInfo, "%s:%s (s)", pobDef->m_obSound.c_str(), pcDefName);
			m_uiDebugContinuousInfoCol = (!pobEvent->m_pobOther || !pobEvent->m_pobOther->GetDef()) ? DEBUG_IMPACT_INFO_COLOUR_SURROGATE:DEBUG_CONTINUOUS_INFO_COLOUR_SLIDE;
		}
		else
		{
			sprintf(m_pcDebugContinuousInfo, "SLIDE");
			m_uiDebugContinuousInfoCol = DEBUG_CONTINUOUS_INFO_COLOUR_MISSING;
		}
		m_fDebugContinuousInfoTimeout = DEBUG_CONTINUOUS_INFO_TIMEOUT_SLIDE;
		m_obDebugContinuousInfoPos = pobEvent->m_obCollisionPosition;
		break;
	case CollisionEffectManager::ROLL:
		if (pobDef)
		{
			DataObject* pobDefDO = ObjectDatabase::Get().GetDataObjectFromPointer(pobDef);
			const char* pcDefName = pobDefDO ? pobDefDO->GetName().GetString():"UNKNOWN";

			sprintf(m_pcDebugContinuousInfo, "%s:%s (r)", pobDef->m_obSound.c_str(), pcDefName);
			m_uiDebugContinuousInfoCol = (!pobEvent->m_pobOther || !pobEvent->m_pobOther->GetDef()) ? DEBUG_IMPACT_INFO_COLOUR_SURROGATE:DEBUG_CONTINUOUS_INFO_COLOUR_ROLL;
		}
		else
		{
			sprintf(m_pcDebugContinuousInfo, "ROLL");
			m_uiDebugContinuousInfoCol = DEBUG_CONTINUOUS_INFO_COLOUR_MISSING;
		}
		m_fDebugContinuousInfoTimeout = DEBUG_CONTINUOUS_INFO_TIMEOUT_ROLL;
		m_obDebugContinuousInfoPos = pobEvent->m_obCollisionPosition;
		break;
	}
}


//------------------------------------------------------------------------------------------
//	DebugResetInfo
//!	Debug only. Re-initialises all debug related info structures.
//------------------------------------------------------------------------------------------
void CollisionEffectHandler::DebugResetInfo(void)
{
	if (!m_pcDebugImpactInfo)
		m_pcDebugImpactInfo = NT_NEW_ARRAY char[s_iDebugInfoSize];
	memset(m_pcDebugImpactInfo, 0, s_iDebugInfoSize);;
	if (!m_pcDebugContinuousInfo)
		m_pcDebugContinuousInfo = NT_NEW_ARRAY char[s_iDebugInfoSize];
	memset(m_pcDebugContinuousInfo, 0, s_iDebugInfoSize);;
	m_fDebugImpactInfoTimeout = 0.0f;
	m_fDebugContinuousInfoTimeout = 0.0f;
	m_obDebugImpactInfoPos = CPoint();
	m_obDebugContinuousInfoPos = CPoint();
	m_uiDebugImpactInfoCol = DC_BLACK;
	m_uiDebugContinuousInfoCol = DC_BLACK;
}


//--------------------------------------------------------------------------------------
//	DebugIntegrityCheck
//!	Debug only. Verifies debug strings match internal bitfield members, and warns if they
//!	do not.
//!	@note This function corrects member bitfields to match strings, and advises user to
//!	resave associated XML file with the Welder tool.
//--------------------------------------------------------------------------------------
void CollisionEffectFilterDef::DebugIntegrityCheck(void)
{
	static const char* pcPrefix = "Collision Effect Filter Definition Integrity Check FAIL\n\t";

	bool bOkay = true;

	DataObject* pobThis = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	const char* pcName = pobThis ? pobThis->GetName().GetString():"UNKNOWN (lookup failed!)";

	// Verify material type
	/*
	if (!m_obMaterialType.IsNull())
	{
		unsigned int uiStore = CollisionEffectManager::ParseBitfieldString(m_obMaterialType.GetDebugString(), CollisionEffectManager::VolumeMaterialFromString);
		if (m_uiMaterialType != uiStore)
		{
			bOkay = false;

			ntPrintf(
				"%s%s material type mismatch (code %u [\"%s\"]is not \"%s\" [%u]).",
				pcPrefix,
				pcName,
				m_uiMaterialType,
				CollisionEffectManager::VolumeMaterialBitfieldToString(m_uiMaterialType).c_str(),
				m_obMaterialType.GetDebugString(),
				uiStore);

			// Correct bitfield to match string
			m_uiMaterialType = uiStore;
		}
	}
	*/

	// Verify mass type
	if (!m_obMassType.IsNull())
	{
		unsigned int uiStore = CollisionEffectManager::ParseBitfieldString(m_obMassType.GetDebugString(), CollisionEffectManager::VolumeMassFromString);
		if (m_uiMassType != uiStore)
		{
			bOkay = false;
			ntPrintf(
				"%s%s mass type mismatch (code %u [\"%s\"]is not \"%s\" [%u]).",
				pcPrefix,
				pcName,
				m_uiMassType,
				CollisionEffectManager::VolumeMassBitfieldToString(m_uiMassType).c_str(),
				m_obMassType.GetDebugString(),
				uiStore);

			// Correct bitfield to match string
			m_uiMassType = uiStore;
		}
	}

	// Extra alert
	if (!bOkay)
	{
		ntstd::String obWarning = pcPrefix;
		obWarning += pcName;
		if (pobThis && pobThis->GetParent())
		{
			obWarning += "\n\nTo correct this problem, open Welder and re-save\n\t";
			obWarning += pobThis->GetParent()->m_FileName;
		}

		user_warn_msg((obWarning.c_str()));
	}
}


#endif // _COLLISION_EFFECT_DEBUG
