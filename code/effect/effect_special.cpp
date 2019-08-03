/***************************************************************************************************
*
*	DESCRIPTION		Collection of classes for tweaking renderer properties during a special
*
*	NOTES		
*
***************************************************************************************************/

#include "effect_special.h"

#include "game/special.h"
#include "game/entitymanager.h"
#include "game/query.h"

#include "game/renderablecomponent.h"
#include "gfx/meshinstance.h"
#include "gfx/lenseffects.h"
#include "objectdatabase/dataobject.h"

START_CHUNKED_INTERFACE(CEffectSpecialDef, Mem::MC_EFFECTS)
	IFLOAT		(CEffectSpecialDef, KillKeyValScalar)
	IFLOAT		(CEffectSpecialDef, KillBloomRange)
	IFLOAT		(CEffectSpecialDef, KillBloomPower)
	IFLOAT		(CEffectSpecialDef, KillIntroLen)
	IFLOAT		(CEffectSpecialDef, KillOutroLen)
	ILIGHT		(CEffectSpecialDef, KillSig_DiffuseMod)
	ILIGHT		(CEffectSpecialDef, KillSig_SpecularMod)
	ILIGHT		(CEffectSpecialDef, KillSig_ReflectMod)

	IFLOAT		(CEffectSpecialDef, SlowKeyValScalar)
	IFLOAT		(CEffectSpecialDef, SlowBloomRange)
	IFLOAT		(CEffectSpecialDef, SlowBloomPower)
	IFLOAT		(CEffectSpecialDef, SlowIntroLen)
	IFLOAT		(CEffectSpecialDef, SlowOutroLen)
	ILIGHT		(CEffectSpecialDef, SlowSig_DiffuseMod)
	ILIGHT		(CEffectSpecialDef, SlowSig_SpecularMod)
	ILIGHT		(CEffectSpecialDef, SlowSig_ReflectMod)
END_STD_INTERFACE

// Space for the static list of registered effects
ntstd::List< CEffectSpecial*, Mem::MC_EFFECTS >* CEffectSpecial::m_pobRegisteredSpecials = 0;

/***************************************************************************************************
*
*	FUNCTION		CEffectSpecialDef::CEffectSpecialDef
*
*	DESCRIPTION		initialiser
*
***************************************************************************************************/
CEffectSpecialDef::CEffectSpecialDef( void )
{
	m_fKillKeyValScalar = 1.0f;
	m_fKillBloomRange = 20.0f;
	m_fKillBloomPower = 1.5f;
	m_fKillIntroLen = 2.0f;
	m_fKillOutroLen = 1.0f;
	m_obKillSig_DiffuseMod	= CVector( 1.0f, 0.1f, 0.05f, 500.0f );
	m_obKillSig_SpecularMod = CVector( 1.0f, 0.1f, 0.05f, 500.0f );
	m_obKillSig_ReflectMod	= CVector( 1.0f, 0.1f, 0.05f, 1000.0f );

	m_fSlowKeyValScalar = 1.0f;
	m_fSlowBloomRange = 20.0f;
	m_fSlowBloomPower = 1.5f;
	m_fSlowIntroLen = 5.0f;
	m_fSlowOutroLen = 3.0f;
	m_obSlowSig_DiffuseMod	= CVector( 0.1f, 1.0f, 0.05f, 100.0f );
	m_obSlowSig_SpecularMod	= CVector( 0.1f, 1.0f, 0.05f, 100.0f );
	m_obSlowSig_ReflectMod	= CVector( 0.1f, 1.0f, 0.05f, 200.0f );
}

/***************************************************************************************************
*
*	FUNCTION		CEffectSpecialDef::CalculateDesitnationProperty
*
*	DESCRIPTION		hide our config data acces
*
***************************************************************************************************/
bool CEffectSpecialDef::CalculateDesitnationProperty(	CVector& obResult, const float* pfSrc,
														EFFECT_SPECIAL_TYPE eType, int iProperty ) const
{
	obResult = CVector( pfSrc );
	if ( eType == KILLING_SPECIAL )
	{
		switch( iProperty )
		{
		case PROPERTY_DIFFUSE_COLOUR0:
		case PROPERTY_DIFFUSE_COLOUR1:
		case PROPERTY_DIFFUSE_COLOUR2:
			obResult.X() = m_obKillSig_DiffuseMod.X() * m_obKillSig_DiffuseMod.W();
			obResult.Y() = m_obKillSig_DiffuseMod.Y() * m_obKillSig_DiffuseMod.W();
			obResult.Z() = m_obKillSig_DiffuseMod.Z() * m_obKillSig_DiffuseMod.W();
			return true;

		case PROPERTY_SPECULAR_COLOUR:
			obResult.X() = m_obKillSig_SpecularMod.X() * m_obKillSig_SpecularMod.W();
			obResult.Y() = m_obKillSig_SpecularMod.Y() * m_obKillSig_SpecularMod.W();
			obResult.Z() = m_obKillSig_SpecularMod.Z() * m_obKillSig_SpecularMod.W();
			return true;
		
		case PROPERTY_SPECULAR_COLOUR2:
			obResult.X() = m_obKillSig_SpecularMod.X() * m_obKillSig_SpecularMod.W();
			obResult.Y() = m_obKillSig_SpecularMod.Y() * m_obKillSig_SpecularMod.W();
			obResult.Z() = m_obKillSig_SpecularMod.Z() * m_obKillSig_SpecularMod.W();
			return true;

		case PROPERTY_REFLECTANCE_COLOUR:
			obResult.X() = m_obKillSig_ReflectMod.X() * m_obKillSig_ReflectMod.W();
			obResult.Y() = m_obKillSig_ReflectMod.Y() * m_obKillSig_ReflectMod.W();
			obResult.Z() = m_obKillSig_ReflectMod.Z() * m_obKillSig_ReflectMod.W();
			return true;
		}
	}
	else
	{
		ntAssert( eType == SLOWDOWN_SPECIAL );

		switch( iProperty )
		{
		case PROPERTY_DIFFUSE_COLOUR0:
		case PROPERTY_DIFFUSE_COLOUR1:
		case PROPERTY_DIFFUSE_COLOUR2:
			obResult.X() = m_obSlowSig_DiffuseMod.X() * m_obSlowSig_DiffuseMod.W();
			obResult.Y() = m_obSlowSig_DiffuseMod.Y() * m_obSlowSig_DiffuseMod.W();
			obResult.Z() = m_obSlowSig_DiffuseMod.Z() * m_obSlowSig_DiffuseMod.W();
			return true;

		case PROPERTY_SPECULAR_COLOUR:
			obResult.X() = m_obSlowSig_SpecularMod.X() * m_obSlowSig_SpecularMod.W();
			obResult.Y() = m_obSlowSig_SpecularMod.Y() * m_obSlowSig_SpecularMod.W();
			obResult.Z() = m_obSlowSig_SpecularMod.Z() * m_obSlowSig_SpecularMod.W();
			return true;

		case PROPERTY_REFLECTANCE_COLOUR:
			obResult.X() = m_obSlowSig_ReflectMod.X() * m_obSlowSig_ReflectMod.W();
			obResult.Y() = m_obSlowSig_ReflectMod.Y() * m_obSlowSig_ReflectMod.W();
			obResult.Z() = m_obSlowSig_ReflectMod.Z() * m_obSlowSig_ReflectMod.W();
			return true;
		}
	}
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CEffectSpecialDef::GetBloomParameters
*
*	DESCRIPTION		hide our config data acces
*
***************************************************************************************************/
void CEffectSpecialDef::GetBloomParameters( float& fRange, float& fPower, EFFECT_SPECIAL_TYPE eType ) const
{
	if ( eType == KILLING_SPECIAL )
	{
		fRange = m_fKillBloomRange;
		fPower = m_fKillBloomPower;
	}
	else
	{
		ntAssert( eType == SLOWDOWN_SPECIAL );
		fRange = m_fSlowBloomRange;
		fPower = m_fSlowBloomPower;
	}

	fRange = max( fRange, 1.0f );
}

/***************************************************************************************************
*
*	FUNCTION		CEffectSpecialDef::GetKeyValParameters
*
*	DESCRIPTION		hide our config data acces
*
***************************************************************************************************/
float CEffectSpecialDef::GetKeyValParameters( EFFECT_SPECIAL_TYPE eType ) const
{
	if ( eType == KILLING_SPECIAL )
	{
		return m_fKillKeyValScalar;
	}
	else
	{
		ntAssert( eType == SLOWDOWN_SPECIAL );
		return m_fSlowKeyValScalar;
	}
}
	
// add clase result
void CEffectSpecial::AddResultToProdded(CEntityQuery& eq)
{
	CEntityManager::Get().FindEntitiesByType( eq, CEntity::EntType_AllButStatic );

	// Build a list of the sub entities that we will poke during the effects
	QueryResultsContainerType::iterator obEnd = eq.GetResults().end();
	for ( QueryResultsContainerType::iterator obIt = eq.GetResults().begin(); obIt != obEnd; ++obIt )
	{
		m_obProddedEntities.push_back( *obIt );
	}	
}

/***************************************************************************************************
*
*	FUNCTION		CEffectSpecial::CEffectSpecial
*
*	DESCRIPTION		initialiser
*
***************************************************************************************************/
CEffectSpecial::CEffectSpecial( const CEntity* pobParent ) :
	m_pobParent(pobParent),
	m_bInvalid(true),
	m_bWasActive(false),
	m_fTimeInEffect(0.0f),
	m_fCurrLerpValue(0.0f),
	m_fLerpAtSwitch(0.0f)
{
#if 0
	ntAssert(m_pobParent);

	// sword must have a parent matching these criteria
	CEntityQuery	obParentCriteria;
	CEQCIsPlayer	obPlayerClause;
	CEQCIsThis		obActualParent( m_pobParent );

	obParentCriteria.AddClause( obPlayerClause );
	obParentCriteria.AddClause( obActualParent );
	
	// find the sword(s) of the players
	{
		CEntityQuery obSwordQuery;
	
		CEQCIsSubStringInName	obNameClause( "SWORD" );
		CEQCDoesParentFitQuery	obParentClause( obParentCriteria );
	
		obSwordQuery.AddClause( obNameClause );
		obSwordQuery.AddClause( obParentClause );
		
		AddResultToProdded( obSwordQuery );
	}
	
/*
	Why the hell did I do that ???????
	
	// find the hair(s) of the players
	{
		CEntityQuery obHairQuery;
	
		CEQCIsSubStringInName	obNameClause( "HAIR" );
		CEQCDoesParentFitQuery	obParentClause( obParentCriteria );
	
		obHairQuery.AddClause( obNameClause );
		obHairQuery.AddClause( obParentClause );
		
		AddResultToProdded( obHairQuery );
	}
*/
	// see if we can find the heroines sword, so we can change its properties
	BuildProddedEntities();
	
	// find our definition XML struct
	m_bOwnsDef = false;
	m_pobDef = ObjectDatabase::Get().GetPointerFromName<CEffectSpecialDef*>( "SpecialMoveDef" );

	if (!m_pobDef)
	{
		m_bOwnsDef = true;
		m_pobDef = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) CEffectSpecialDef;
	}

	// Register ourselves
	if ( !m_pobRegisteredSpecials )
		m_pobRegisteredSpecials = NT_NEW_CHUNK( Mem::MC_EFFECTS ) ntstd::List< CEffectSpecial*, Mem::MC_EFFECTS >;

	m_pobRegisteredSpecials->push_back( this );
#endif
}


/***************************************************************************************************
*
*	FUNCTION		CEffectSpecial::BuildProddedEntities
*
*	DESCRIPTION		Construct the list of bodies to prod
*
***************************************************************************************************/

void CEffectSpecial::BuildProddedEntities( void )
{
	ntAssert( m_bInvalid );

	for (	ntstd::List<CEntity*>::iterator obIt = m_obProddedEntities.begin();
			obIt != m_obProddedEntities.end(); ++obIt )
	{
		SetPerEntityEvilness( *obIt );
	}
	ntAssert_p( !m_obProddedEntities.empty(), ("Swords not found for special move effect") );
	m_bInvalid = false;
}


/***************************************************************************************************
*
*	FUNCTION		CEffectSpecial::Cleanup
*
*	DESCRIPTION		cleanup: not in destructor, as cant do this when entity is being destroyed. Sigh.
*
***************************************************************************************************/
void	CEffectSpecial::Cleanup( void )
{
	ntAssert(!m_bInvalid);
	for (	ntstd::List<CEntity*>::iterator obIt = m_obProddedEntities.begin();
			obIt != m_obProddedEntities.end(); ++obIt )
	{
		ClearPerEntityEvilness( *obIt );
	}
	m_bInvalid = true;
}

/***************************************************************************************************
*
*	FUNCTION		CEffectSpecial::CEffectSpecial
*
*	DESCRIPTION		cleanup: not in destructor, as cant do this when entity is being destroyed. Sigh.
*
***************************************************************************************************/
CEffectSpecial::~CEffectSpecial( void )
{
	if (m_bOwnsDef)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pobDef );
	}

	if (m_pobRegisteredSpecials)
	{
		// Clear ourselves from the register
		m_pobRegisteredSpecials->remove( this );

		// Destroy the list if we have run out
		if ( m_pobRegisteredSpecials->size() == 0 )
		{
			NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pobRegisteredSpecials );
			m_pobRegisteredSpecials = 0;
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CEffectSpecial::Clear
*
*	DESCRIPTION		A static function to call reset effect on registered effects
*
***************************************************************************************************/

void CEffectSpecial::Clear( void )
{
	if ( m_pobRegisteredSpecials )
	{
		// Call reset effect on all the registered effects
		ntstd::List< CEffectSpecial*, Mem::MC_EFFECTS >::iterator obEnd = m_pobRegisteredSpecials->end();
		for ( ntstd::List< CEffectSpecial*, Mem::MC_EFFECTS >::iterator obIt = m_pobRegisteredSpecials->begin(); obIt != obEnd; ++obIt )
		{
			( *obIt )->Cleanup();
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CEffectSpecial::Build
*
*	DESCRIPTION		A static function to rebuild effect on registered effects
*
***************************************************************************************************/

void CEffectSpecial::Build( void )
{
	if ( m_pobRegisteredSpecials )
	{
		// Call reset effect on all the registered effects
		ntstd::List< CEffectSpecial*, Mem::MC_EFFECTS >::iterator obEnd = m_pobRegisteredSpecials->end();
		for ( ntstd::List< CEffectSpecial*, Mem::MC_EFFECTS >::iterator obIt = m_pobRegisteredSpecials->begin(); obIt != obEnd; ++obIt )
		{
			( *obIt )->BuildProddedEntities();
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CEffectSpecial::SetPerEntityEvilness
*
*	DESCRIPTION		overide mesh stuff
*
***************************************************************************************************/
void	CEffectSpecial::SetPerEntityEvilness( CEntity* pobVictim )
{
	CRenderableComponent* pobRenderableComp = pobVictim->GetRenderableComponent();

	for(	CRenderableComponent::MeshInstanceList::const_iterator obIt = pobRenderableComp->GetMeshInstances().begin();
			obIt != pobRenderableComp->GetMeshInstances().end(); ++obIt )		
	{
		// for each mesh instance, make sure it buffers its material property table
		(*obIt)->GetMaterialInstance()->SetPropertyTableOveride( true );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CEffectSpecial::SetPerEntityEvilness
*
*	DESCRIPTION		overide mesh stuff
*
***************************************************************************************************/
void	CEffectSpecial::ClearPerEntityEvilness( CEntity* pobVictim )
{
	CRenderableComponent* pobRenderableComp = pobVictim->GetRenderableComponent();

	for(	CRenderableComponent::MeshInstanceList::const_iterator obIt = pobRenderableComp->GetMeshInstances().begin();
			obIt != pobRenderableComp->GetMeshInstances().end(); ++obIt )		
	{
		// this should free up the mesh property overide table, 
		(*obIt)->GetMaterialInstance()->SetPropertyTableOveride(false);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CEffectSpecial::UpdatePerEntityEvilness
*
*	DESCRIPTION		update func, twiddles variables parametricaly
*
***************************************************************************************************/
void	CEffectSpecial::UpdatePerEntityEvilness( CEntity* pobVictim, float fIntensity, EFFECT_SPECIAL_TYPE eType )
{
	fIntensity = clamp( fIntensity, 0.0f, 1.0f );

	CRenderableComponent* pobRenderableComp = pobVictim->GetRenderableComponent();

	for ( CRenderableComponent::MeshInstanceList::const_iterator obIt = pobRenderableComp->GetMeshInstances().begin();
			obIt != pobRenderableComp->GetMeshInstances().end(); ++obIt )		
	{
		// we look through the buffered table and poke what we want, using the real one as a reference
		CMaterialProperty*	pobTable = (*obIt)->GetMaterialInstance()->GetPropertyOverideTable( );
		ntAssert( pobTable );

		const CMaterialProperty* pobTemplate = (*obIt)->GetMaterialInstance()->GetPropertyDefaultTable();

		for( int i = 0; i < (*obIt)->GetMaterialInstance()->GetPropertyTableSize(); i++ )
		{
			const float* pfFloatRef = pobTemplate[i].GetFloatData().afFloats;
			float* pfFloatData = pobTable[i].GetFloatData().afFloats;

			// get the modified destination property from our definition
			CVector obLerpDest;
			if (m_pobDef->CalculateDesitnationProperty( obLerpDest, pfFloatRef, eType, pobTable[i].m_iPropertyTag ))
			{
				// override the cached copy
				Lerp4Floats( pfFloatData, pfFloatRef, obLerpDest, fIntensity );
			}
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CEffectSpecial::CreateNewSpecial
*
*	DESCRIPTION		Create requested effect
*
***************************************************************************************************/
CEffectSpecial* CEffectSpecial::CreateNewSpecial( const CEntity* pobParent, EFFECT_SPECIAL_TYPE eType )
{
	// There is now a single special that may have different functionality on a per stance basis. For now
	// i have just added a default parameter to this function so that eType will be KILLING_SPECIAL.  We may
	// need to remove one of the specials or add another (one for each stance) when the design is locked down - GH

	switch( eType )
	{
	case SLOWDOWN_SPECIAL:	return NT_NEW_CHUNK( Mem::MC_EFFECTS ) CEffectSlowdownSpecial(pobParent);
	case KILLING_SPECIAL:	return NT_NEW_CHUNK( Mem::MC_EFFECTS ) CEffectKillingSpecial(pobParent);
	}
	ntAssert(0);
	return NULL;
}

/***************************************************************************************************
*
*	FUNCTION		CEffectSpecial::Lerp4Floats
*
*	DESCRIPTION		'orrble func to cope with funcky data format od material properties
*
*	NOTES			copies values to local vectors in case result is same as src.
*
***************************************************************************************************/
void CEffectSpecial::Lerp4Floats( float* pobResult, const float* pobSrc, const CVector& obDest, float fLerp )
{
	CVector obResult( CVector::Lerp( CVector(pobSrc), obDest, fLerp ) );		
	pobResult[0] = obResult.X(); pobResult[1] = obResult.Y(); pobResult[2] = obResult.Z(); pobResult[3] = obResult.W();
}











/***************************************************************************************************
*
*	FUNCTION		CEffectSlowdownSpecial::Update
*
*	DESCRIPTION		update func
*
***************************************************************************************************/
bool	CEffectSlowdownSpecial::Update( float fTimeStep, bool bActive )
{
	ntAssert(!m_bInvalid);

	float fIntroTime = m_pobDef->m_fSlowIntroLen;
	float fOutroTime = m_pobDef->m_fSlowOutroLen;

	if ((bActive) && (!m_bWasActive))
	{
		// just been triggerd again, start over
		m_bWasActive = true;
		m_fLerpAtSwitch = m_fCurrLerpValue;
		m_fTimeInEffect = m_fLerpAtSwitch * fIntroTime;
	}

	if ((!bActive) && (m_bWasActive))
	{
		// just released, start exiting
		m_bWasActive = false;
		m_fLerpAtSwitch = m_fCurrLerpValue;
		m_fTimeInEffect = fOutroTime - (m_fLerpAtSwitch * fOutroTime);
	}

	if( bActive )
		m_fCurrLerpValue =  max( m_fLerpAtSwitch, min(m_fTimeInEffect / fIntroTime, 1.0f) );
	else
		m_fCurrLerpValue =  min( m_fLerpAtSwitch, (1.0f - min(m_fTimeInEffect / fOutroTime, 1.0f)));
	
	// we're gonna overide material properties, according to some definition template...
	for (	ntstd::List<CEntity*>::iterator obIt = m_obProddedEntities.begin();
			obIt != m_obProddedEntities.end(); ++obIt )
	{
		UpdatePerEntityEvilness( *obIt, m_fCurrLerpValue, SLOWDOWN_SPECIAL );
	}

	// we're also gonna poke the hell out of the lens effects, just to annoy simon
	float fRemappedLerp = fcosf(((m_fCurrLerpValue * 90.0f)-90.0f) * DEG_TO_RAD_VALUE);
	float fBloomRange, fBloomPower;
	m_pobDef->GetBloomParameters( fBloomRange, fBloomPower, SLOWDOWN_SPECIAL );
	CStaticLensConfig::SetOverideParameters((1.0f*(1.0f-fRemappedLerp))+(0.0f*(fRemappedLerp)),
											(1.0f*(1.0f-fRemappedLerp))+((1.0f/fBloomRange)*(fRemappedLerp)),
											(1.0f*(1.0f-fRemappedLerp))+(fBloomPower*(fRemappedLerp)),
											0 );

	// we adjust the fKeyValueMapping to oversaturate the color values
	// (WD deprecated 10.03.05 when moved to localised lighting model)
//	float fNewKeyValueScalar =	(1.0f*(1.0f-m_fCurrLerpValue))+
//								(m_pobDef->GetKeyValParameters( SLOWDOWN_SPECIAL ) *(m_fCurrLerpValue));
//
//	CRendererSettings::fKeyValueMapping *= fNewKeyValueScalar;

	m_fTimeInEffect += fTimeStep;

	if ((!bActive) && (m_fTimeInEffect > fOutroTime))
		return true;

	return false;
}






/***************************************************************************************************
*
*	FUNCTION		CEffectKillingSpecial::Update
*
*	DESCRIPTION		update func
*
***************************************************************************************************/
bool	CEffectKillingSpecial::Update( float fTimeStep, bool bActive )
{
	ntAssert(!m_bInvalid);

	float fIntroTime = m_pobDef->m_fKillIntroLen;
	float fOutroTime = m_pobDef->m_fKillOutroLen;

	if ((bActive) && (!m_bWasActive))
	{
		// just been triggerd again, start over
		m_bWasActive = true;
		m_fLerpAtSwitch = m_fCurrLerpValue;
		m_fTimeInEffect = m_fLerpAtSwitch * fIntroTime;
	}

	if ((!bActive) && (m_bWasActive))
	{
		// just released, start exiting
		m_bWasActive = false;
		m_fLerpAtSwitch = m_fCurrLerpValue;
		m_fTimeInEffect = fOutroTime - (m_fLerpAtSwitch * fOutroTime);
	}

	if( bActive )
		m_fCurrLerpValue =  max( m_fLerpAtSwitch, min(m_fTimeInEffect / fIntroTime, 1.0f) );
	else
		m_fCurrLerpValue =  min( m_fLerpAtSwitch, (1.0f - min(m_fTimeInEffect / fOutroTime, 1.0f)));

	// we're gonna overide material properties, according to some definition template...
	for (	ntstd::List<CEntity*>::iterator obIt = m_obProddedEntities.begin();
			obIt != m_obProddedEntities.end(); ++obIt )
	{
		UpdatePerEntityEvilness( *obIt, m_fCurrLerpValue, KILLING_SPECIAL );
	}

	// we're also gonna poke the hell out of the lens effects, just to annoy simon
	float fRemappedLerp = fcosf(((m_fCurrLerpValue * 90.0f)-90.0f) * DEG_TO_RAD_VALUE);
	float fBloomRange, fBloomPower;
	m_pobDef->GetBloomParameters( fBloomRange, fBloomPower, KILLING_SPECIAL );
	CStaticLensConfig::SetOverideParameters((1.0f*(1.0f-fRemappedLerp))+(0.0f*(fRemappedLerp)),
											(1.0f*(1.0f-fRemappedLerp))+((1.0f/fBloomRange)*(fRemappedLerp)),
											(1.0f*(1.0f-fRemappedLerp))+(fBloomPower*(fRemappedLerp)),
											0 );

	// we adjust the fKeyValueMapping to oversaturate the color values
	// (WD deprecated 10.03.05 when moved to localised lighting model)
//	float fNewKeyValueScalar =	(1.0f*(1.0f-m_fCurrLerpValue))+
//								(m_pobDef->GetKeyValParameters( KILLING_SPECIAL ) *(m_fCurrLerpValue));
//
//	CRendererSettings::fKeyValueMapping *= fNewKeyValueScalar;

	m_fTimeInEffect += fTimeStep;	

	if ((!bActive) && (m_fTimeInEffect > fOutroTime))
		return true;

	return false;
}









