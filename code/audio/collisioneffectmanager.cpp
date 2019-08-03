//--------------------------------------------------------------------------------------
//!	@file collisioneffectmanager.cpp
//!	@author Chip Bell (SCEE), Harvey Cotton
//!	@date 31.08.06
//!
//!	@brief Implementation of the CollisionEffectManager and related classes.
//--------------------------------------------------------------------------------------


#include "audio/collisioneffectmanager.h"

#include "audio/collisioneffecthandler.h"
#include "core/hashcodes.h"
#include "objectdatabase/dataobject.h"

#include "physics/physicsmaterial.h"


#ifdef _COLLISION_EFFECT_DEBUG
#include <ctype.h>
#include <string.h>

#include "core/debug.h"


#define IS_WHITESPACE(character)										\
	(' ' == (character) || '\t' == (character) || '\n' == (character))

#define SEPARATOR_CHAR													\
	','

#define RETURN_ENUM_VALUE(string, enumValue)							\
	if (strcmp(string, #enumValue) == 0)								\
		return enumValue;

#define RETURN_ENUM_STRING(value, enumValue)							\
	if (enumValue == value)												\
		return #enumValue;

#define APPEND_BITFIELD_STRING(bitfield, enumValue, strObj, sepChar)	\
	if (((bitfield)&(enumValue)))										\
	{																	\
		strObj += #enumValue;											\
		strObj += sepChar;												\
	}
#endif // _COLLISION_EFFECT_DEBUG


START_STD_INTERFACE							( CollisionEffectDef )
	PUBLISH_VAR_AS							( m_obCollisionType,	CollisionType )
	PUBLISH_VAR_AS							( m_obMaterial1,		MaterialType1 )
	PUBLISH_VAR_AS							( m_obMass1,			MassType1 )
	PUBLISH_VAR_AS							( m_obMaterial2,		MaterialType2 )
	PUBLISH_VAR_AS							( m_obMass2,			MassType2 )

	PUBLISH_VAR_AS							( m_obSound,			Sound )
	PUBLISH_VAR_AS							( m_obParticleDef,		ParticleDef )

	PUBLISH_VAR_AS							( m_uiCollisionType,	CollisionType_Code )
	PUBLISH_VAR_AS							( m_uiMaterial1,		MaterialType1_Code )
	PUBLISH_VAR_AS							( m_uiMass1,			MassType1_Code )
	PUBLISH_VAR_AS							( m_uiMaterial2,		MaterialType2_Code )
	PUBLISH_VAR_AS							( m_uiMass2,			MassType2_Code )

	DECLARE_POSTPOSTCONSTRUCT_CALLBACK		( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE


//--------------------------------------------------------------------------------------
//	CollisionEffectDef
//!	Default ctor.
//--------------------------------------------------------------------------------------
CollisionEffectDef::CollisionEffectDef(void)
:
	// Intialise to defaults
	m_uiCollisionType(CollisionEffectManager::ANY_COLLISION),
	m_uiMaterial1(CollisionEffectManager::ANY_MATERIAL),
	m_uiMaterial2(CollisionEffectManager::ANY_MATERIAL),
	m_uiMass1(CollisionEffectManager::ANY_MASS),
	m_uiMass2(CollisionEffectManager::ANY_MASS),
#ifdef _COLLISION_EFFECT_DEBUG
	// String types must be intialised to corresponding values
	m_obCollisionType("ANY_COLLISION"),
	m_obMaterial1("ANY_MATERIAL"),
	m_obMaterial2("ANY_MATERIAL"),
	m_obMass1("ANY_MASS"),
	m_obMass2("ANY_MASS"),
#endif // _COLLISION_EFFECT_DEBUG
	m_fScore(0.0f)
{
	CollisionEffectManager::Get().AddEffectDef(this);
}


//--------------------------------------------------------------------------------------
//	~CollisionEffectDef
//!	Default dtor.
//--------------------------------------------------------------------------------------
CollisionEffectDef::~CollisionEffectDef(void)
{
	if (CollisionEffectManager::Exists())
		CollisionEffectManager::Get().RemoveEffectDef(this);
}


//--------------------------------------------------------------------------------------
//	PostContruct
//!	Finalises initialisation.
//--------------------------------------------------------------------------------------
void CollisionEffectDef::PostConstruct(void)
{
	// HC: For time being, need to recalculate everything here - This means you will need to load the game on debug/development and save out the physics effect xml

#ifndef _RELEASE
	m_uiCollisionType = CollisionEffectManager::ParseBitfieldString(m_obCollisionType.GetDebugString(), CollisionEffectManager::CollisionTypeFromString);
	m_uiMaterial1=Physics::PhysicsMaterialTable::Get().GetEffectMaterialBitfield(m_obMaterial1.c_str());
	m_uiMaterial2=Physics::PhysicsMaterialTable::Get().GetEffectMaterialBitfield(m_obMaterial2.c_str());
	m_uiMass1 = CollisionEffectManager::ParseBitfieldString(m_obMass1.GetDebugString(), CollisionEffectManager::VolumeMassFromString);
	m_uiMass2 = CollisionEffectManager::ParseBitfieldString(m_obMass2.GetDebugString(), CollisionEffectManager::VolumeMassFromString);
#endif // _RELEASE

	CalcScore();


/*
// HC: Temporarily disabled

#ifdef _COLLISION_EFFECT_DEBUG
	DebugIntegrityCheck();
#endif // _COLLISION_EFFECT_DEBUG
*/
}


//------------------------------------------------------------------------------------------
//	EditorChangeValue
//!	Debug editor callback (reinitialises).
//!	@param obItem	Name of item which has been changed.
//!	@param obValue	New value.
//!	@return True if notification processed successfully, false otherwise (indicates default
//!	change behavior should not proceed).
//------------------------------------------------------------------------------------------
bool CollisionEffectDef::EditorChangeValue(CallBackParameter obItem, CallBackParameter obValue)
{
#ifdef _COLLISION_EFFECT_DEBUG

	CalcScore();

	bool bReflect = false;

	// Special processing for string items
	CHashedString obItemHash(obItem);
	CHashedString obItemValue(obValue);
	if (CHashedString(HASH_STRING_COLLISIONTYPE) == obItemHash)
	{
		m_uiCollisionType = CollisionEffectManager::ParseBitfieldString(obItemValue.GetDebugString(), CollisionEffectManager::CollisionTypeFromString);
		ntPrintf("CollisionType_Code = %d (%s)\n", m_uiCollisionType, obItemValue.GetDebugString());
		bReflect = true;
	}
	else if (CHashedString(HASH_STRING_MATERIALTYPE1) == obItemHash)
	{
		//m_uiMaterial1 = CollisionEffectManager::ParseBitfieldString(obItemValue.GetDebugString(), CollisionEffectManager::VolumeMaterialFromString);
		ntPrintf("MaterialType1_Code = %d (%s)\n", m_uiMaterial1, obItemValue.GetDebugString());
		bReflect = true;
	}
	else if (CHashedString(HASH_STRING_MATERIALTYPE2) == obItemHash)
	{
		//m_uiMaterial2 = CollisionEffectManager::ParseBitfieldString(obItemValue.GetDebugString(), CollisionEffectManager::VolumeMaterialFromString);
		ntPrintf("MaterialType2_Code = %d (%s)\n", m_uiMaterial2, obItemValue.GetDebugString());
		bReflect = true;
	}
	else if (CHashedString(HASH_STRING_MASSTYPE1) == obItemHash)
	{
		m_uiMass1 = CollisionEffectManager::ParseBitfieldString(obItemValue.GetDebugString(), CollisionEffectManager::VolumeMassFromString);
		ntPrintf("MassType1_Code = %d (%s)\n", m_uiMass1, obItemValue.GetDebugString());
		bReflect = true;
	}
	else if (CHashedString(HASH_STRING_MASSTYPE2) == obItemHash)
	{
		m_uiMass2 = CollisionEffectManager::ParseBitfieldString(obItemValue.GetDebugString(), CollisionEffectManager::VolumeMassFromString);
		ntPrintf("MassType2_Code = %d (%s)\n", m_uiMass2, obItemValue.GetDebugString());
		bReflect = true;
	}

	if (bReflect)
	{
		DataObject* pobThis = ObjectDatabase::Get().GetDataObjectFromPointer(this);
		if (pobThis)
			ObjectDatabase::Get().SignalObjectChange(pobThis);

		ntPrintf("Collision Effect Definition bitfields updated. (May need to refresh Welder.)\n");
	}

#else
	UNUSED(obItem);
	UNUSED(obValue);
#endif // _COLLISION_EFFECT_DEBUG

	// Recalculate score if values are altered in Welder
	//CalcScore();

	return true;
}


//--------------------------------------------------------------------------------------
//	GetScore
//!	Retrieves definition score (for collision effect manager queries).
//!	@param obEvent		Collision effect manager event.
//!	@return Score value, which is negative for no match.
//!	@note The higher the score, the more suitable the collision effect definition.
//--------------------------------------------------------------------------------------
float CollisionEffectDef::GetScore(const CollisionEffectManager::CollisionEffectManagerEvent& obEvent)
{
	if (m_uiCollisionType==0 || (m_uiCollisionType&obEvent.m_uiType))
	{
		if (((m_uiMaterial1==0 || (m_uiMaterial1 & obEvent.m_uiMaterial1)) && (m_uiMaterial2==0 || (m_uiMaterial2 & obEvent.m_uiMaterial2))) // Check to see if materials match
			|| ((m_uiMaterial1==0 || (m_uiMaterial1 & obEvent.m_uiMaterial2)) && (m_uiMaterial2==0 || (m_uiMaterial2 & obEvent.m_uiMaterial1))) ) // Switch materials and see if they match
		{
			if (((m_uiMass1==0 || (m_uiMass1 & obEvent.m_uiMass1)) && (m_uiMass2==0 || (m_uiMass2 & obEvent.m_uiMass2))) // Check to see if the masses match
				|| ((m_uiMass1==0 || (m_uiMass1 & obEvent.m_uiMass2)) && (m_uiMass2==0 || (m_uiMass2 & obEvent.m_uiMass1))) ) // Switch masses and see if they match
			{
				return m_fScore;
			}
		}
	}

	// No match
	return -1.0f;
}


//--------------------------------------------------------------------------------------
//	CalcScore
//!	Computes and stores collision effect definition score. Score should be recalculated
//!	whenever any of the internal material, mass, etc. parameters are modified.
//--------------------------------------------------------------------------------------
void CollisionEffectDef::CalcScore(void)
{
	// Get the effect material bitfields
	m_uiMaterial1=Physics::PhysicsMaterialTable::Get().GetEffectMaterialBitfield(m_obMaterial1.c_str());
	m_uiMaterial2=Physics::PhysicsMaterialTable::Get().GetEffectMaterialBitfield(m_obMaterial2.c_str());

	m_fScore = 0.0f;

	// Compute score as the sum of the respective reciprocals of the collision type, material type and mass bit counts.
	// i.e. score = 1/(collision type total) + 1/(material 1 total) + 1/(material 2 total) + 1/(mass 1 total) + 1/(mass 2 total)

	float fColType = (float)BitCount(m_uiCollisionType);
	if (fColType > 0.0f)
		m_fScore += 1.0f/fColType;

	float fMaterial1 = (float)BitCount(m_uiMaterial1);
	if (fMaterial1 > 0.0f)
		m_fScore += 1.0f/fMaterial1;

	float fMaterial2 = (float)BitCount(m_uiMaterial2);
	if (fMaterial2 > 0.0f)
		m_fScore += 1.0f/fMaterial2;

	float fMass1 = (float)BitCount(m_uiMass1);
	if (fMass1 > 0.0f)
		m_fScore += 1.0f/fMass1;

	float fMass2 = (float)BitCount(m_uiMass2);
	if (fMass2 > 0.0f)
		m_fScore += 1.0f/fMass2;
}


//--------------------------------------------------------------------------------------
//	BitCount
//!	Retrieves total number of set bits in an unsigned integer.
//!	@param n	Integer in question.
//!	@return Number of set bits for supplied value.
//!	@note Assumes 32-bit architecture.
//--------------------------------------------------------------------------------------
int CollisionEffectDef::BitCount (uint64_t n)
{
	/*
	// Parallel bit count algorithm
#define TWO(c) (0x1u << (c))
#define MASK(c) (((unsigned int)(-1)) / (TWO(TWO(c)) + 1u))
#define COUNT(x,c) ((x) & MASK(c)) + (((x) >> (TWO(c))) & MASK(c))

	n = COUNT(n, 0);
	n = COUNT(n, 1);
	n = COUNT(n, 2);
	n = COUNT(n, 3);
	n = COUNT(n, 4);
	n = COUNT(n, 5); // 64-bit integers

	return (float)n ;
	*/

	// Sparse ones bit count algorithm
    int count=0;

    while(n)
    {
        ++count;
        n &= (n - 1);     
    }
    return count;
}


#ifdef _COLLISION_EFFECT_DEBUG


//--------------------------------------------------------------------------------------
//	DebugIntegrityCheck
//!	Debug only. Verifies debug strings match internal bitfield members, and warns if they
//!	do not.
//!	@note This function corrects member bitfields to match strings, and advises user to
//!	resave associated XML file with the Welder tool.
//--------------------------------------------------------------------------------------
void CollisionEffectDef::DebugIntegrityCheck(void)
{
	static const char* pcPrefix = "Collision Effect Definition Integrity Check FAIL\n\t";

	bool bOkay = true;

	DataObject* pobThis = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	const char* pcName = pobThis ? pobThis->GetName().GetString():"UNKNOWN (lookup failed!)";

	// Verify collision type
	if (!m_obCollisionType.IsNull())
	{
		unsigned int uiStore = CollisionEffectManager::ParseBitfieldString(m_obCollisionType.GetDebugString(), CollisionEffectManager::CollisionTypeFromString);
		if (m_uiCollisionType != uiStore)
		{
			bOkay = false;
			ntPrintf(
				"%s%s collision type mismatch (code %u [\"%s\"]is not \"%s\" [%u]).",
				pcPrefix,
				pcName,
				m_uiCollisionType,
				CollisionEffectManager::CollisionTypeBitfieldToString(m_uiCollisionType).c_str(),
				m_obCollisionType.GetDebugString(),
				uiStore);

			// Correct bitfield to match string
			m_uiCollisionType = uiStore;
		}
	}

	// Verify material 1
	/*
	if (!m_obMaterial1.IsNull())
	{
		unsigned int uiStore = CollisionEffectManager::ParseBitfieldString(m_obMaterial1.GetDebugString(), CollisionEffectManager::VolumeMaterialFromString);
		if (m_uiMaterial1 != uiStore)
		{
			bOkay = false;

			ntPrintf(
				"%s%s material type 1 mismatch (code %u [\"%s\"]is not \"%s\" [%u]).",
				pcPrefix,
				pcName,
				m_uiMaterial1,
				CollisionEffectManager::VolumeMaterialBitfieldToString(m_uiMaterial1).c_str(),
				m_obMaterial1.GetDebugString(),
				uiStore);


			// Correct bitfield to match string
			m_uiMaterial1 = uiStore;
		}
	}
	*/

	// Verify material 2
	/*
	if (!m_obMaterial2.IsNull())
	{
		unsigned int uiStore = CollisionEffectManager::ParseBitfieldString(m_obMaterial2.GetDebugString(), CollisionEffectManager::VolumeMaterialFromString);
		if (m_uiMaterial2 != uiStore)
		{
			bOkay = false;

			ntPrintf(
				"%s%s material type 2 mismatch (code %u [\"%s\"]is not \"%s\" [%u]).",
				pcPrefix,
				pcName,
				m_uiMaterial2,
				CollisionEffectManager::VolumeMaterialBitfieldToString(m_uiMaterial2).c_str(),
				m_obMaterial2.GetDebugString(),
				uiStore);

				// Correct bitfield to match string
			m_uiMaterial2 = uiStore;
		}
	}
	*/

	// Verify mass 1
	if (!m_obMass1.IsNull())
	{
		unsigned int uiStore = CollisionEffectManager::ParseBitfieldString(m_obMass1.GetDebugString(), CollisionEffectManager::VolumeMassFromString);
		if (m_uiMass1 != uiStore)
		{
			bOkay = false;
			ntPrintf(
				"%s%s mass type 1 mismatch (code %u [\"%s\"]is not \"%s\" [%u]).",
				pcPrefix,
				pcName,
				m_uiMass1,
				CollisionEffectManager::VolumeMassBitfieldToString(m_uiMass1).c_str(),
				m_obMass1.GetDebugString(),
				uiStore);

			// Correct bitfield to match string
			m_uiMass1 = uiStore;
		}
	}

	// Verify mass 2
	if (!m_obMass2.IsNull())
	{
		unsigned int uiStore = CollisionEffectManager::ParseBitfieldString(m_obMass2.GetDebugString(), CollisionEffectManager::VolumeMassFromString);
		if (m_uiMass2 != uiStore)
		{
			bOkay = false;
			ntPrintf(
				"%s%s mass type 2 mismatch (code %u [\"%s\"]is not \"%s\" [%u]).",
				pcPrefix,
				pcName,
				m_uiMass2,
				CollisionEffectManager::VolumeMassBitfieldToString(m_uiMass2).c_str(),
				m_obMass2.GetDebugString(),
				uiStore);

			// Correct bitfield to match string
			m_uiMass2 = uiStore;
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

		// Values may have been changed above
		CalcScore();
	}
}


#endif // _COLLISION_EFFECT_DEBUG




//--------------------------------------------------------------------------------------
//	CollisionEffectManager
//!	Default ctor.
//--------------------------------------------------------------------------------------
CollisionEffectManager::CollisionEffectManager(void)
:
	m_bEnableCollisionEffects(true)
{
	CollisionEffectHandler::Initialise();
}


//--------------------------------------------------------------------------------------
//	~CollisionEffectManager
//!	Default dtor.
//--------------------------------------------------------------------------------------
CollisionEffectManager::~CollisionEffectManager(void)
{
	CollisionEffectHandler::Deinitialise();
}


//--------------------------------------------------------------------------------------
//	AddEffectDef
//!	Adds a collision effect definition to the manager.
//!	@param pobEffectDef	Effect definition to add.
//--------------------------------------------------------------------------------------
void CollisionEffectManager::AddEffectDef(CollisionEffectDef* pobEffectDef)
{
	m_obEffectDefList.push_back(pobEffectDef);
}


//--------------------------------------------------------------------------------------
//	RemoveEffectDef
//!	Removes a collision effect definition from the manager.
//!	@param pobEffectDef	Effect definition to remove.
//--------------------------------------------------------------------------------------
void CollisionEffectManager::RemoveEffectDef(CollisionEffectDef* pobEffectDef)
{
	m_obEffectDefList.remove(pobEffectDef);
}


//--------------------------------------------------------------------------------------
//	GetEffectDef
//!	Retrieves an appropriate collision effect definition for a supplied collision event.
//!	@param obEvent	Details physical interaction.
//!	@return Pointer to collision effect definition, or 0 if not found.
//--------------------------------------------------------------------------------------
CollisionEffectDef* CollisionEffectManager::GetEffectDef(const CollisionEffectManagerEvent& obEvent)
{
	if (!m_bEnableCollisionEffects)
		return 0;

	float fScore = -1.0f;
	CollisionEffectDef* pobEffectDef = 0;
	for (ntstd::List<CollisionEffectDef*>::iterator obIt = m_obEffectDefList.begin(); obIt != m_obEffectDefList.end(); ++obIt)
	{
		float fThisScore = (*obIt)->GetScore(obEvent);
		if (fThisScore > fScore)
		{
			fScore = fThisScore;
			pobEffectDef = *obIt;
		}
	}

	return pobEffectDef;
}


//--------------------------------------------------------------------------------------
//	EnableCollisionEffects
//!	Toggles collision effects.
//!	@param	bEnable	Indicates collision effects should be enabled (true) or disabled (false).
//!	@note If collision effects are disabled effect definitions cannot be retrieved.
//!	@sa CollisionEffectManager::CollisionEffectsEnabled(void)
//--------------------------------------------------------------------------------------
void CollisionEffectManager::EnableCollisionEffects(bool bEnable)
{
	m_bEnableCollisionEffects = bEnable;
}


//--------------------------------------------------------------------------------------
//	CollisionEffectsEnabled
//!	Indicates whether or not collision effects are enabled.
//!	@return True if collision effects should be enabled, false otherwise.
//!	@note If collision effects are disabled effect definitions cannot be retrieved.
//!	@sa CollisionEffectManager::EnableCollisionEffects(bool bEnable)
//--------------------------------------------------------------------------------------
bool CollisionEffectManager::CollisionEffectsEnabled(void)
{
	return m_bEnableCollisionEffects;
}


#ifdef _COLLISION_EFFECT_DEBUG


//--------------------------------------------------------------------------------------
//	ParseBitfieldString
//!	Parses a string bitfield. Values should be separated with the separator character
//!	defined as SEPARATOR_CHAR (typically comma).
//!	@param pcString			Bitfield enums.
//!	@param pfEnumFromString	Function to convert string tokens (which may be empty) into
//!	bitfield enumeration values.
//!	@return Bitfield identifier matching string.
//!	@note Unrecognised enums interpreted as 0.
//--------------------------------------------------------------------------------------
unsigned int CollisionEffectManager::ParseBitfieldString(const char* pcString, unsigned int (*pfEnumFromString)(const char*))
{
	unsigned int uiResult = 0;

	char* pcTemp = NT_NEW_ARRAY char[strlen(pcString) + 1];

	strcpy(pcTemp, pcString);
	char* pcSep = 0;
	char* pcPos = pcTemp;
	while ('\0' != pcPos[0])
	{
		// Tokenise
		pcSep = strchr(pcPos, SEPARATOR_CHAR);
		if (pcSep)
		{
			pcSep[0] = '\0';
			uiResult |= (*pfEnumFromString)(NormaliseString(pcPos));
			pcPos = pcSep + 1;
		}
		else
		{
			uiResult |= (*pfEnumFromString)(NormaliseString(pcPos));
			break;
		}
	}
	
	NT_DELETE_ARRAY( pcTemp );

	return uiResult;
}


//--------------------------------------------------------------------------------------
//	NormaliseString
//!	Strips whitespace and converts a string to uppercase.
//!	@param pcString	String to tidy.
//!	@return Pointer to first non-whitespace character in string, which will have been
//!	shortened so as to have no trailing whitespace (may be an empty string).
//!	@note Destructive. String is converted and re-terminated in place.
//--------------------------------------------------------------------------------------
const char* CollisionEffectManager::NormaliseString(char* pcString)
{
	// Strip trailing whitespace
	int iLength = strlen(pcString);
	char* pcPos = &pcString[iLength - 1];
	while (IS_WHITESPACE(pcPos[0]))
	{
		--pcPos;
		if (pcPos < pcString)
		{
			// String is all whitespace
			pcString[0] = '\0';
			return pcString;
		}
	}
	pcPos[1] = '\0';

	// Strip leading whitespace
	pcPos = pcString;
	while (IS_WHITESPACE(pcPos[0]))
	{
		++pcPos;
		if ('\0' == pcPos[0])
		{
			// String is all whitespace (this should never actually occur at this point)
			pcString[0] = '\0';
			return pcString;
		}
	}

	// Convert to uppercase
	const char* pcResult = pcPos;
	while ('\0' != pcPos[0])
	{
		pcPos[0] = (char)toupper((int)pcPos[0]);
		++pcPos;
	}

	// Done
	return pcResult;
}


//--------------------------------------------------------------------------------------
//	VolumeMassFromString
//!	Retrieves enum value from string.
//!	@param pcString	String giving enum name.
//!	@return Enum value from string name, or ANY_MASS on error.
//!	@sa CollisionEffectManager::VolumeMassToString(VOLUME_MASS eValue)
//!	@sa CollisionEffectManager::VolumeMassBitfieldToString(unsigned int uiBitfield)
//--------------------------------------------------------------------------------------
unsigned int CollisionEffectManager::VolumeMassFromString(const char* pcString)
{
	if (strcmp(pcString, "ANY") == 0)
		return ANY_MASS;
	RETURN_ENUM_VALUE(pcString, ANY_MASS);
	RETURN_ENUM_VALUE(pcString, LIGHT);
	RETURN_ENUM_VALUE(pcString, MEDIUM);
	RETURN_ENUM_VALUE(pcString, HEAVY);
	RETURN_ENUM_VALUE(pcString, WORLD);
	return ANY_MASS;
}


//--------------------------------------------------------------------------------------
//	VolumeMassToString
//!	Retrieves string from enum value.
//!	@param eValue	Enum value;
//!	@return String name from enum value, or "ANY_MATERIAL" on error.
//!	@sa CollisionEffectManager::VolumeMassFromString(const char* pcString)
//!	@sa CollisionEffectManager::VolumeMassBitfieldToString(unsigned int uiBitfield)
//--------------------------------------------------------------------------------------
const char* CollisionEffectManager::VolumeMassToString(VOLUME_MASS eValue)
{
	RETURN_ENUM_STRING(eValue, ANY_MASS);
	RETURN_ENUM_STRING(eValue, LIGHT);
	RETURN_ENUM_STRING(eValue, MEDIUM);
	RETURN_ENUM_STRING(eValue, HEAVY);
	RETURN_ENUM_STRING(eValue, WORLD);
	return "ANY_MASS";
}


//--------------------------------------------------------------------------------------
//	VolumeMassBitfieldToString
//!	Retrieves string from bitfield value.
//!	@param uiBitfield	Bitfield value;
//!	@return Separator delimited string from bitfield value, or "ANY_MASS" on error.
//!	@sa CollisionEffectManager::VolumeMassToString(VOLUME_MASS eValue)
//!	@sa CollisionEffectManager::VolumeMassFromString(const char* pcString)
//--------------------------------------------------------------------------------------
ntstd::String CollisionEffectManager::VolumeMassBitfieldToString(unsigned int uiBitfield)
{
	ntstd::String obResult;
	APPEND_BITFIELD_STRING(uiBitfield, LIGHT, obResult, SEPARATOR_CHAR);
	APPEND_BITFIELD_STRING(uiBitfield, MEDIUM, obResult, SEPARATOR_CHAR);
	APPEND_BITFIELD_STRING(uiBitfield, HEAVY, obResult, SEPARATOR_CHAR);
	APPEND_BITFIELD_STRING(uiBitfield, WORLD, obResult, SEPARATOR_CHAR);
	return (obResult.length() > 0)
		? obResult.substr(0, obResult.length() - 1)
		: "ANY_MASS";
}

/*
//--------------------------------------------------------------------------------------
//	VolumeMaterialFromString
//!	Retrieves enum value from string.
//!	@param pcString	String giving enum name.
//!	@return Enum value from string name, or ANY_MATERIAL on error.
//!	@sa CollisionEffectManager::VolumeMaterialToString(VOLUME_MATERIAL eValue)
//--------------------------------------------------------------------------------------
unsigned int CollisionEffectManager::VolumeMaterialFromString(const char* pcString)
{
	if (strcmp(pcString, "ANY") == 0)
		return ANY_MATERIAL;
	RETURN_ENUM_VALUE(pcString, FLESH);
	RETURN_ENUM_VALUE(pcString, STONE);
	RETURN_ENUM_VALUE(pcString, METAL);
	RETURN_ENUM_VALUE(pcString, WOOD);
	return ANY_MATERIAL;
}


//--------------------------------------------------------------------------------------
//	VolumeMaterialToString
//!	Retrieves string from enum value.
//!	@param eValue	Enum value;
//!	@return String name from enum value, or "ANY_MATERIAL" on error.
//!	@sa CollisionEffectManager::VolumeMaterialFromString(const char* pcString)
//--------------------------------------------------------------------------------------
const char* CollisionEffectManager::VolumeMaterialToString(VOLUME_MATERIAL eValue)
{
	RETURN_ENUM_STRING(eValue, ANY_MATERIAL);
	RETURN_ENUM_STRING(eValue, FLESH);
	RETURN_ENUM_STRING(eValue, STONE);
	RETURN_ENUM_STRING(eValue, METAL);
	RETURN_ENUM_STRING(eValue, WOOD);
	return "ANY_MATERIAL";
}

//--------------------------------------------------------------------------------------
//	VolumeMaterialBitfieldToString
//!	Retrieves string from bitfield value.
//!	@param uiBitfield	Bitfield value;
//!	@return Separator delimited string from bitfield value, or "ANY_MATERIAL" on error.
//!	@sa CollisionEffectManager::VolumeMaterialToString(VOLUME_MATERIAL eValue)
//!	@sa CollisionEffectManager::VolumeMaterialFromString(const char* pcString)
//--------------------------------------------------------------------------------------
ntstd::String CollisionEffectManager::VolumeMaterialBitfieldToString(unsigned int uiBitfield)
{
	ntstd::String obResult;
	APPEND_BITFIELD_STRING(uiBitfield, FLESH, obResult, SEPARATOR_CHAR);
	APPEND_BITFIELD_STRING(uiBitfield, STONE, obResult, SEPARATOR_CHAR);
	APPEND_BITFIELD_STRING(uiBitfield, METAL, obResult, SEPARATOR_CHAR);
	APPEND_BITFIELD_STRING(uiBitfield, WOOD, obResult, SEPARATOR_CHAR);
	return (obResult.length() > 0)
		? obResult.substr(0, obResult.length() - 1)
		: "ANY_MATERIAL";
}
*/

//--------------------------------------------------------------------------------------
//	CollisionTypeFromString
//!	Retrieves enum value from string.
//!	@param pcString	String giving enum name.
//!	@return Enum value from string name, or ANY_COLLISION on error.
//!	@sa CollisionEffectManager::CollisionTypeToString(COLLISION_TYPE eValue)
//--------------------------------------------------------------------------------------
unsigned int CollisionEffectManager::CollisionTypeFromString(const char* pcString)
{
	if (strcmp(pcString, "ANY") == 0)
		return ANY_COLLISION;
	RETURN_ENUM_VALUE(pcString, ANY_COLLISION);
	RETURN_ENUM_VALUE(pcString, BOUNCE);
	RETURN_ENUM_VALUE(pcString, CRASH);
	RETURN_ENUM_VALUE(pcString, SLIDE);
	RETURN_ENUM_VALUE(pcString, ROLL);
	return ANY_COLLISION;
}


//--------------------------------------------------------------------------------------
//	CollisionTypeToString
//!	Retrieves string from enum value.
//!	@param eValue	Enum value;
//!	@return String name from enum value, or "ANY_COLLISION" on error.
//!	@sa CollisionEffectManager::VolumeMaterialFromString(const char* pcString)
//--------------------------------------------------------------------------------------
const char* CollisionEffectManager::CollisionTypeToString(COLLISION_TYPE eValue)
{
	RETURN_ENUM_STRING(eValue, ANY_COLLISION);
	RETURN_ENUM_STRING(eValue, BOUNCE);
	RETURN_ENUM_STRING(eValue, CRASH);
	RETURN_ENUM_STRING(eValue, SLIDE);
	RETURN_ENUM_STRING(eValue, ROLL);
	return "ANY_COLLISION";
}


//--------------------------------------------------------------------------------------
//	CollisionTypeBitfieldToString
//!	Retrieves string from bitfield value.
//!	@param uiBitfield	Bitfield value;
//!	@return Separator delimited string from bitfield value, or "ANY_COLLISION" on error.
//!	@sa CollisionEffectManager::CollisionTypeToString(COLLISION_TYPE eValue)
//!	@sa CollisionEffectManager::CollisionTypeFromString(const char* pcString)
//--------------------------------------------------------------------------------------
ntstd::String CollisionEffectManager::CollisionTypeBitfieldToString(unsigned int uiBitfield)
{
	ntstd::String obResult;
	APPEND_BITFIELD_STRING(uiBitfield, BOUNCE, obResult, SEPARATOR_CHAR);
	APPEND_BITFIELD_STRING(uiBitfield, CRASH, obResult, SEPARATOR_CHAR);
	APPEND_BITFIELD_STRING(uiBitfield, SLIDE, obResult, SEPARATOR_CHAR);
	APPEND_BITFIELD_STRING(uiBitfield, ROLL, obResult, SEPARATOR_CHAR);
	return (obResult.length() > 0)
		? obResult.substr(0, obResult.length() - 1)
		: "ANY_COLLISION";
}


#endif // _COLLISION_EFFECT_DEBUG
