//------------------------------------------------------------------------------------------
//!	@file collisioneffectmanager.h
//!	@author Chip Bell (SCEE), Harvey Cotton
//!	@date 31.08.06
//!
//!	@brief Declaration of the CollisionEffectManager and related classes.
//------------------------------------------------------------------------------------------


#ifndef _COLLISIONEFFECTMANAGER_H_
#define _COLLISIONEFFECTMANAGER_H_


#if defined(_DEBUG) || defined(_DEVELOPMENT)
#define _COLLISION_EFFECT_DEBUG
#endif


#include "core/keystring.h"

#ifdef _COLLISION_EFFECT_DEBUG

#include "core/nt_std.h"

#endif // _COLLISION_EFFECT_DEBUG


class CollisionEffectDef;


//------------------------------------------------------------------------------------------
//!	@class CollisionEffectManager
//!	@brief Manages effect descrptions.
//!	Singleton storing collision effect descriptions and retrieving those  matching given
//!	collision events.
//------------------------------------------------------------------------------------------
class CollisionEffectManager : public Singleton<CollisionEffectManager>
{
public:
	//!	Enumerates weight categories of physics system volumes.
	//!	@sa CollisionEffectManager::VolumeMassFromString(const char* pcString)
	//!	@sa CollisionEffectManager::VolumeMassToString(VOLUME_MASS eValue)
	//!	@sa CollisionEffectManager::VolumeMassBitfieldToString(unsigned int uiBitfield)
	enum VOLUME_MASS
	{
		// Any should always have the lowest rating
		ANY_MASS	= 0,
		LIGHT		= 1,
		MEDIUM		= 2,
		HEAVY		= 4,
		WORLD		= 8

		// NOTE Be sure to update debug string conversion functions when modifying this enum.
	};
	static const unsigned int INVALID_MASS = (unsigned int)-1;

	//!	Enumerates material types of physics system volumes.
	//!	@sa CollisionEffectManager::VolumeMaterialFromString(const char* pcString)
	//!	@sa CollisionEffectManager::VolumeMaterialToString(VOLUME_MATERIAL eValue)
	//!	@sa CollisionEffectManager::VolumeMaterialBitfieldToString(unsigned int uiBitfield)
	/*
	enum VOLUME_MATERIAL
	{
		// Any should always have the lowest rating
		ANY_MATERIAL	= 0,
		FLESH			= 1,
		STONE			= 2,
		METAL			= 4,
		WOOD			= 8
		// NOTE Be sure to update debug string conversion functions when modifying this enum.
	};
	static const unsigned int INVALID_MATERIAL = (unsigned int)-1;
	*/

	static const uint64_t ANY_MATERIAL = (uint64_t)0;
	static const uint64_t INVALID_MATERIAL = (uint64_t)-1;

	//!	Enumerates interaction possibilities for physics system volumes.
	//!	@sa CollisionEffectManager::CollisionTypeFromString(const char* pcString)
	//!	@sa CollisionEffectManager::CollisionTypeToString(COLLISION_TYPE eValue)
	//!	@sa CollisionEffectManager::CollisionTypeBitfieldToString(unsigned int uiBitfield)
	enum COLLISION_TYPE
	{
		ANY_COLLISION	= 0,
		BOUNCE			= 1,	//!< An impact with a negative projected velocity
		CRASH			= 2,	//!< An impact with a position projected velocity
		SLIDE			= 4,	//!< Movement perpendicular to the contact surface
		ROLL			= 8		//!< Angular movement whilst making surface contact

		// NOTE Be sure to update debug string conversion functions when modifying this enum.
	};
	static const unsigned int INVALID_COLLISION = (unsigned int)-1;


	//--------------------------------------------------------------------------------------
	//!	@class CollisionEffectManagerEvent
	//!	@brief Represents an abstracted world collision.
	//--------------------------------------------------------------------------------------
	class CollisionEffectManagerEvent
	{
	public:
		//!	Default ctor.
		CollisionEffectManagerEvent()
		:
			m_uiType(ANY_COLLISION),
			m_uiMaterial1(ANY_MATERIAL),
			m_uiMaterial2(ANY_MATERIAL),
			m_uiMass1(ANY_MASS),
			m_uiMass2(ANY_MASS)
		{}

		unsigned int m_uiType;			//!< The type of collision  represented by this event
		uint64_t m_uiMaterial1;		//!< Material identifier of first body
		uint64_t m_uiMaterial2;		//!< Material identifier of second body
		unsigned int m_uiMass1;			//!< Mass of first body
		unsigned int m_uiMass2;			//!< Mass of second body
	};

	CollisionEffectManager(void);
	~CollisionEffectManager(void);

	void AddEffectDef(CollisionEffectDef* pobEffectDef);
	void RemoveEffectDef(CollisionEffectDef* pobEffectDef);
	CollisionEffectDef* GetEffectDef(const CollisionEffectManagerEvent& obEvent);

	void EnableCollisionEffects(bool bEnable);
	bool CollisionEffectsEnabled(void);


private:
	ntstd::List<CollisionEffectDef*> m_obEffectDefList;	//!<	List of collision effect definitions
	bool m_bEnableCollisionEffects;						//!<	Indicates whether or not collision effects are enabled


#ifdef _COLLISION_EFFECT_DEBUG
public:
	static unsigned int ParseBitfieldString(const char* pcString, unsigned int (*pfEnumFromString)(const char*));
	static unsigned int VolumeMassFromString(const char* pcString);
	static const char* VolumeMassToString(VOLUME_MASS eValue);
	static ntstd::String VolumeMassBitfieldToString(unsigned int uiBitfield);
	//static unsigned int VolumeMaterialFromString(const char* pcString);
	//static const char* VolumeMaterialToString(VOLUME_MATERIAL eValue);
	//static ntstd::String VolumeMaterialBitfieldToString(unsigned int uiBitfield);
	static unsigned int CollisionTypeFromString(const char* pcString);
	static const char* CollisionTypeToString(COLLISION_TYPE eValue);
	static ntstd::String CollisionTypeBitfieldToString(unsigned int uiBitfield);

protected:
	static const char* NormaliseString(char* pcString);
#endif // _COLLISION_EFFECT_DEBUG
};




//------------------------------------------------------------------------------------------
//!	@class CollisionEffectDef
//!	@brief Describes a collision effect tag.
//!	@brief This is a serialised object which links a particular type of collision with a
//!	sound and particle effect.
//------------------------------------------------------------------------------------------
class CollisionEffectDef
{
public:
	CollisionEffectDef(void);
	~CollisionEffectDef(void);

	void PostConstruct(void);
	bool EditorChangeValue(CallBackParameter obItem, CallBackParameter obValue);

	float GetScore(const CollisionEffectManager::CollisionEffectManagerEvent& obEvent);

	// Serialised data...
	unsigned int	m_uiCollisionType;	//!< Collision type
	uint64_t		m_uiMaterial1;		//!< Primary body material
	uint64_t		m_uiMaterial2;		//!< Secondary body material
	unsigned		int m_uiMass1;		//!< Primary body mass
	unsigned		int m_uiMass2;		//!< Secondary body mass
	ntstd::String	m_obSound;			//!< FMOD event for associated collision sound
	CHashedString	m_obParticleDef;	//!< Identifies particle effect for this collision
	CHashedString 	m_obCollisionType;	//!< String representation for m_uiType
	ntstd::String 	m_obMaterial1;		//!< String representation for m_uiMaterial1
	ntstd::String 	m_obMaterial2;		//!< String representation for m_uiMaterial2
	CHashedString 	m_obMass1;			//!< String representation for m_uiMass1
	CHashedString 	m_obMass2;			//!< String representation for m_uiMass2


private:
	void CalcScore(void);
	int BitCount(uint64_t n);

	float m_fScore;	//!< A relevance rating used when selecting most approriate definition for a collision event


#ifdef _COLLISION_EFFECT_DEBUG
	void DebugIntegrityCheck(void);
#endif // _COLLISION_EFFECT_DEBUG
};

#endif // _COLLISIONEFFECTMANAGER_H_
