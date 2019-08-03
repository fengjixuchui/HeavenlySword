/***************************************************************************************************
*
*	DESCRIPTION		Contains functionality used for searching the game database
*
*	NOTES
*
***************************************************************************************************/

#include "Physics/config.h"
#include "Physics/system.h"
#include "Physics/singlerigidlg.h"
#include "Physics/advancedcharactercontroller.h"

// Necessary Includes
#include "game/query.h"
#include "game/entityinfo.h"
#include "game/interactioncomponent.h"
#include "game/aicomponent.h"
#include "game/luaattrtable.h"
#include "game/entitymanager.h"
#include "core/boundingvolumes.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/attacks.h"
#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"
#include "audio/gameaudiocomponents.h"
#include "Physics/collisionbitfield.h"
#include "Physics/world.h"
#include "physics/lookatinfo.h"
#include "game/entityrangedweapon.h"
#include "game/entityboss.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/entity/hkRigidBody.h>
#endif

/***************************************************************************************************
*
*	FUNCTION		CEntityQuery::CEntityQuery
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CEntityQuery::CEntityQuery( void )
{
	m_obSelectedResults.reserve(32);
	m_obSelectionDetails.reserve(8);
}

/***************************************************************************************************
*
*	FUNCTION		CEntityQuery::~CEntityQuery
*
*	DESCRIPTION		Destruction
*
*					This query object does not own the clauses that are added or the entities that
*					it is used to retrieve so we only need to clear out the pointers held in lists.
*			
***************************************************************************************************/

CEntityQuery::~CEntityQuery( void )
{
	m_obSelectionDetails.clear();
	m_obUnSelectionDetails.clear();
	m_obSelectedResults.clear();
	m_obExcludedEntities.clear();
}


/***************************************************************************************************
*
*	FUNCTION		CEntityQuery::AddClause
*
*	DESCRIPTION		We do not take ownership of a clause when it is added.  The usage of clauses
*					should mainly be stack based and there would be a high maintainance overhead
*					creating copy constructors for all future derived clauses.
*
***************************************************************************************************/

void CEntityQuery::AddClause( const CEntityQueryClause& obClause )
{
	m_obSelectionDetails.push_back( &obClause );
}


/***************************************************************************************************
*
*	FUNCTION		CEntityQuery::AddUnClause
*
*	DESCRIPTION		Negative clauses - used the same as the positive selections - but not
*
***************************************************************************************************/

void CEntityQuery::AddUnClause( const CEntityQueryClause& obClause )
{
	m_obUnSelectionDetails.push_back( &obClause );
}


/***************************************************************************************************
*
*	FUNCTION		CEntityQuery::AddExcludedEntity
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CEntityQuery::AddExcludedEntity( const CEntity& obExcludedEntity )
{
	m_obExcludedEntities.push_back( &obExcludedEntity );
}


/***************************************************************************************************
*
*	FUNCTION		CEQCProximityColumn::Visit
*
*	DESCRIPTION		Checks whether the given entity is within the coumn volumn decribed by the y
*					axis of the member matrix and the member radius value.
*
***************************************************************************************************/

bool CEQCProximityColumn::Visit( const CEntity& obTestEntity ) const
{
	// Check that our search criteria are set to sensible values
	ntAssert( m_fRadius > 0.0f ); 
	
	// Find the world space position of our given entity
	CPoint obPosition = obTestEntity.GetPosition();

	// Find the position relative to our member matrix
	obPosition -= m_obMatrix.GetTranslation();

	// Convert the point to a direction for the following calcumalation
	CDirection obDirection( obPosition );

	// Find the offsets in the x and z axis described by our member matrix
	float fXOffset = m_obMatrix.GetXAxis().Dot( obDirection );
	float fZOffset = m_obMatrix.GetZAxis().Dot( obDirection );

	// See if the offset on the XZ plane is within our member radius
	if ( ( ( fXOffset * fXOffset ) + ( fZOffset * fZOffset ) )  < ( m_fRadius * m_fRadius ) )
		return true;

	// The given entity failed the test
	return false;

}


/***************************************************************************************************
*
*	FUNCTION		CEntityQueryTools::FindClosestEntity
*
*	DESCRIPTION		Finds the entity closest to the given point, returns 0 if the list is empty.
*
***************************************************************************************************/

CEntity* CEntityQueryTools::FindClosestEntity( const CPoint& obPosition, const ntstd::List< CEntity* >& obEntities )
{
	// Set up a return value
	CEntity* pobClosestEntity = 0;
	
	// ...and storage for the closest item so far
	float fSmallestDistance = FLT_MAX;

	// Iterate through the given list
	for ( ntstd::List< CEntity* >::const_iterator obIt = obEntities.begin(); obIt != obEntities.end(); ++obIt )
	{
		// Find the world space position of our given entity
		CPoint obEntityPosition = ( *obIt )->GetPosition();

		// Find the position relative to our given position
		obEntityPosition -= obPosition;

		const float fDistanceSquared=obEntityPosition.LengthSquared();

		// If this is the closest entity so far update our info
		if ( fDistanceSquared < fSmallestDistance )
		{
			pobClosestEntity = ( *obIt );
			fSmallestDistance = fDistanceSquared;
		}
	}

	return pobClosestEntity;
} 


/***************************************************************************************************
*
*	FUNCTION		CEntityQueryTools::EntityInList
*
*	DESCRIPTION		Checks whether the given entity in in the result set.  Returns true if it is so.
*
***************************************************************************************************/

bool CEntityQueryTools::EntityInList( const CEntity* pobEntity, const ntstd::List< const CEntity* >& obEntities )
{
	for ( ntstd::List< const CEntity* >::const_iterator obIt = obEntities.begin(); obIt != obEntities.end(); ++obIt )
	{
		if ( *obIt == pobEntity )
			return true;
	}

	// If we are here then we haven't found it
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CEQCProximitySphere::Visit
*
*	DESCRIPTION		Checks if an entity of within the volume described be a sphere about the given
*					matrix.
*
***************************************************************************************************/

bool CEQCProximitySphere::Visit( const CEntity& obTestEntity ) const
{
	// Check that our search criteria are set to sensible values
	ntAssert( m_fRadiusSquared > 0.0f ); 

	// Find the world space position of our given entity
	CPoint obPosition(obTestEntity.GetPosition());

	// Find the position relative to our member matrix
	obPosition -= m_obPosition;

	// See if the relative position is within the selected distance
	// ... (Dario) and desregard paused entities
	if ( !obTestEntity.IsPaused() && obPosition.LengthSquared() < m_fRadiusSquared )
		return true;

	// The given entity failed the test
	return false;

}

/***************************************************************************************************
*
*	FUNCTION		CEQCProximitySegment::Visit
*
*	DESCRIPTION		Checks if the given entity is with the segment of a column decribed by the y
*					axis of the member matrix and the member radius value.  The wedge is centered 
*					about the z axis of the member matrix.
*
*	NOTES			Takes full angle of wedge as input
*
***************************************************************************************************/

bool CEQCProximitySegment::Visit( const CEntity& obTestEntity ) const
{
	// Check that our search criteria are set to sensible values
	//ntAssert( m_fRadius > 0.0f ); 
	if (m_fRadius <= 0.0f)
		return false;

	ntAssert( ( m_fAngle <= TWO_PI ) );
	
	CDirection obToTarget = obTestEntity.GetPosition() ^ m_obMatrix.GetTranslation();
	obToTarget.Y() = 0.0f;

	if (obToTarget.LengthSquared() < ( m_fRadius * m_fRadius ))
	{
		obToTarget.Normalise();

		// We need to check the bounds on a dot product so pre calculate here
		float fDProduct = obToTarget.Dot( m_obMatrix.GetZAxis());
		if ( fDProduct > 1.0f ) fDProduct = 1.0f;
		if ( fDProduct < -1.0f ) fDProduct = -1.0f;

		float fAngle = facosf( fDProduct );
		if ( fabsf(fAngle) < m_fAngle )
			return true;
	}

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CEQCHeightRange::Visit
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CEQCHeightRange::Visit( const CEntity& obTestEntity ) const
{
	// WHAT THE HELL IS THIS!!!!!!! -GH
	// This is for long entities like ladders where you can't simply check the root position - HC
	/*
	const float fTestEntityHeight=obTestEntity.GetInteractionComponent()->GetEntityHeight();
	
	if (fTestEntityHeight>0.0f)
	{
		const float fTOLERANCE=0.5f;

		float fDY=m_fRelativeY - obTestEntity.GetPosition().Y();
		
		if (fDY>=-fTOLERANCE && fDY<=(fTestEntityHeight+fTOLERANCE))
		{
			return true;
		}
	}
	else
	{
	*/
		// Check that our search criteria are set to sensible values
		// ntAssert( m_fTop > m_fBottom ); 
		
		// Get the current height of our test entity
		float fEntityHeight = obTestEntity.GetPosition().Y() - m_fRelativeY;

		// Check the height range
		if ( ( fEntityHeight >= m_fBottom ) && ( fEntityHeight <= m_fTop ) )
		{
			return true;
		}
	//}

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CEQLookable::Visit
*
*	DESCRIPTION		Checks if this entity is interesting enough to look at
*
***************************************************************************************************/

bool CEQLookable::Visit( const CEntity& obTestEntity ) const
{
	return ( obTestEntity.GetLookAtInfo() && obTestEntity.GetLookAtInfo()->m_iPriority <= m_iMaxPriority );
}


/***************************************************************************************************
*
*	FUNCTION		CEQCIsEnemy::Visit
*
*	DESCRIPTION		Checks whether this entity is a bad man
*
***************************************************************************************************/

bool CEQCIsEnemy::Visit( const CEntity& obTestEntity ) const
{
	// Dodgy test for the moment - bosses are separate entity types but still enemies
	return obTestEntity.IsEnemy() || obTestEntity.IsBoss();
}
	
/***************************************************************************************************
*
*	FUNCTION		CEQCIsTargetableByPlayer::Visit
*
*	DESCRIPTION		Checks whether this entity is either a player of an enemy, in which case is 
*					targetable
*
***************************************************************************************************/
bool CEQCIsTargetableByPlayer::Visit( const CEntity& obTestEntity ) const
{
	// Too expensive at the moment, and BuildStrikeFromData in attacks.cpp should stop attacks through walls most of the time anyway
	/*if (CEntityManager::Get().GetPlayer()) 
	{
		// Test for wall geoms between me and my new target
		CPoint obMyPos(CEntityManager::Get().GetPlayer()->GetPosition());
		CDirection obMeToTarget(obMyPos - obTestEntity.GetPosition());
		float fLength = obMeToTarget.Length();
		obMeToTarget.Normalise();
		CPoint obLook( obMyPos + ( obMeToTarget * fLength ) );

		// Find closest intersection
		Physics::TRACE_LINE_QUERY stQuery;

		Physics::RaycastCollisionFlag obFlag; 
		obFlag.base = 0;
		obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obFlag.flags.i_collide_with = Physics::STATIC_ENVIRONMENT_BIT;

		if (Physics::CPhysicsWorld::Get().TraceLine( obMyPos, obLook, &obTestEntity, stQuery, obFlag ))
		{
			// We've hit a wall, so bail 
			return false;
		}
	}*/

	//Quick and dirty fix to stop the player from targeting the demon directly (shouldn't attempt to grab the demon etc)
	if(obTestEntity.IsBoss())
	{
		Boss* pobBoss = (Boss*)&obTestEntity;
		if(pobBoss->GetBossType() == Boss::BT_DEMON)
		{
			return false;
		}
	}

	//Otherwise go through the regular criteria.
	return ((obTestEntity.IsPlayer() || obTestEntity.IsAI() || obTestEntity.IsBoss() ) && 
		obTestEntity.GetAttackComponent() && 
		obTestEntity.GetAttackComponent()->AI_Access_GetState() != CS_DEAD);
}

/***************************************************************************************************
*
*	FUNCTION		CEQCIsGroundAttackable::Visit
*
*	DESCRIPTION		Checks whether this entity is floored or rise waiting
*
***************************************************************************************************/
bool CEQCIsGroundAttackable::Visit( const CEntity& obTestEntity ) const
{
	return (obTestEntity.GetAttackComponent() && 
			(obTestEntity.GetAttackComponent()->AI_Access_GetState() == CS_FLOORED ||
			obTestEntity.GetAttackComponent()->AI_Access_GetState() == CS_RISE_WAIT) );
}

/***************************************************************************************************
*
*	FUNCTION		CEQCIsTargetableForEvade::Visit
*
*	DESCRIPTION		Checks whether this entity is floored or rise waiting
*
***************************************************************************************************/
bool CEQCIsTargetableForEvade::Visit( const CEntity& obTestEntity ) const
{
	return (obTestEntity.GetAttackComponent() && 
			obTestEntity.GetAttackComponent()->AI_Access_GetState() != CS_DEAD );
}

/***************************************************************************************************
*
*	CLASS			CEQCIsAerialComboable
*
*	DESCRIPTION		Checks if an entity is a player or an enemy and is KOed in their combat state 
*					and struck by a range fast attack
*
***************************************************************************************************/
bool CEQCIsAerialComboable::Visit( const CEntity& obTestEntity ) const
{
	return ((obTestEntity.IsPlayer() || obTestEntity.IsEnemy()) && 
		obTestEntity.GetAttackComponent() && 
		obTestEntity.GetAttackComponent()->AI_Access_GetState() == CS_KO &&
		obTestEntity.GetAttackComponent()->GetStruckStrike()->GetAttackDataP()->m_eAttackClass == AC_RANGE_FAST );
}

/***************************************************************************************************
*
*	CLASS			CEQCIsCombatComponentActive
*
*	DESCRIPTION		Checks if an entity has an enabled combat component
*
***************************************************************************************************/
bool CEQCIsCombatComponentActive::Visit( const CEntity& obTestEntity ) const
{
	return obTestEntity.GetAttackComponent() ? !obTestEntity.GetAttackComponent()->GetDisabled() : false;
}

/***************************************************************************************************
*
*	CLASS			CEQCHealthLTE
*
*	DESCRIPTION		Checks if an entity has an enabled combat component
*
***************************************************************************************************/
bool CEQCHealthLTE::Visit( const CEntity& obTestEntity ) const
{
	return obTestEntity.IsCharacter() ? obTestEntity.ToCharacter()->GetCurrHealth() <= m_fHealthCheck : false;
}

/***************************************************************************************************
*
*	CLASS			CEQCIsCombatTargetingDisabled
*
*	DESCRIPTION		Checks if an entity has an enabled combat targeting component
*
***************************************************************************************************/
bool CEQCIsCombatTargetingDisabled::Visit( const CEntity& obTestEntity ) const
{
	return obTestEntity.GetAttackComponent() ? obTestEntity.GetAttackComponent()->GetTargetingDisabled() : false;
}

/***************************************************************************************************
*
*	FUNCTION		CEQCIsInNinjaSequence::Visit
*
*	DESCRIPTION		Checks whether this entity is currently involved in a ninja sequence/cutscene
*
***************************************************************************************************/

bool CEQCIsInNinjaSequence::Visit( const CEntity& obTestEntity ) const
{
	return obTestEntity.IsInNinjaSequence(); // Dodgy test for the moment
}


/***************************************************************************************************
*
*	FUNCTION		CEQCIsLockonable::Visit
*
*	DESCRIPTION		Checks whether this entity is a bad man
*
***************************************************************************************************/

bool CEQCIsLockonable::Visit( const CEntity& obTestEntity ) const
{
	// Dodgy test for the moment

	return obTestEntity.IsLockonable();
}


/***************************************************************************************************
*
*	FUNCTION		CEQCIsInteractionTarget::Visit
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CEQCIsInteractionTarget::Visit( const CEntity& obTestEntity ) const
{
	// Skip inactive or phantomised ragdolls.
	if ( obTestEntity.IsCharacter() && obTestEntity.GetPhysicsSystem() )
	{
		Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*)obTestEntity.GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
		if ( pobCharacterState &&
			 ( !pobCharacterState->IsRagdollActive() || pobCharacterState->GetAdvancedRagdoll()->IsPhantom() ) )
		{
			return false;
		}
	}
	
	// Can this object be picked up?
	return ( obTestEntity.GetInteractionComponent() && obTestEntity.GetInteractionComponent()->GetInteractionPriority()!=NONE );
}


/***************************************************************************************************
*
*	FUNCTION		CEQCIsCanTakeStandardHit::Visit
*
*	DESCRIPTION		Checks whether a character will take normal attacks
*
***************************************************************************************************/

bool CEQCIsCanTakeStandardHit::Visit( const CEntity& obTestEntity ) const
{
	if ( obTestEntity.GetAttackComponent() && obTestEntity.GetAttackComponent()->CanTakeStandardHit() )
	{
		return true;
	}

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CEQCIsCanTakeSpeedExtraHit::Visit
*
*	DESCRIPTION		Checks whether a character will take speed extra attacks
*
***************************************************************************************************/
bool CEQCIsCanTakeSpeedExtraHit::Visit( const CEntity& obTestEntity ) const
{
	if ( obTestEntity.GetAttackComponent() && obTestEntity.GetAttackComponent()->CanTakeSpeedExtraHit() )
	{
		return true;
	}

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CEQCIsThrownTarget::Visit
*
*	DESCRIPTION		Checks whether this entity is a bad man
*
***************************************************************************************************/

bool CEQCIsThrownTarget::Visit( const CEntity& obTestEntity ) const
{
	if ( obTestEntity.GetAttackComponent() && obTestEntity.ToCharacter()->GetCurrHealth()>0.0f && obTestEntity.GetAttackComponent()->AI_Access_GetState()==CS_STANDARD)
		return true;
	
	return false;
}
	

/***************************************************************************************************
*
*	FUNCTION		CEQCIsLargeInteractable::Visit
*
*	DESCRIPTION		Checks whether this entity is a large interactable object
*
***************************************************************************************************/

bool CEQCIsLargeInteractable::Visit( const CEntity& obTestEntity ) const
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	if (obTestEntity.GetPhysicsSystem() )
	{
		Physics::SingleRigidLG* pobRigid = static_cast<Physics::SingleRigidLG*>(const_cast<CEntity&>(obTestEntity).GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::SINGLE_RIGID_BODY_LG ));

		if (pobRigid)
		{
	        Physics::EntityCollisionFlag info;
			info.base = pobRigid->GetRigidBody()->getCollidable()->getCollisionFilterInfo();
	 
			if(info.flags.i_am & Physics::LARGE_INTERACTABLE_BIT)
			{
				return true;
			};
		}
	}
#else
	UNUSED(  obTestEntity );
#endif
	return false;
}



/***************************************************************************************************
*
*	FUNCTION		CEQCShouldAvoid::Visit
*
*	DESCRIPTION		Checks whether this entity should be avoided by AI characters
*
***************************************************************************************************/

bool CEQCShouldAvoid::Visit( const CEntity& obTestEntity ) const
{
	DataObject*			pDO			= ObjectDatabase::Get().GetDataObjectFromPointer( &obTestEntity );
	StdDataInterface*	pInterface	= ObjectDatabase::Get().GetInterface( pDO );

	if( pInterface->GetFieldByName( "SharedAttributes" ) != 0 )
	{
		void*	pSub = pInterface->Get<void*>( pDO, "SharedAttributes" );
		if (pSub)
		{
			DataObject*			pSubDO			= ObjectDatabase::Get().GetDataObjectFromPointer( pSub );
			StdDataInterface*	pSubInterface	= ObjectDatabase::Get().GetInterface( pSubDO );

			if( pSubInterface->GetFieldByName( "AIAvoid" ) != 0 )
			{
				return pSubInterface->Get<bool>( pSubDO, "AIAvoid" );
			}
		}
	}
	/*
	if (obTestEntity.GetAIComponent())
	{
		return true;
	}
	*/

	return false;
}



/***************************************************************************************************
*
*	FUNCTION		CEQCIsSubStringInName::Visit
*
*	DESCRIPTION		Checks if an entity has this sub string (case insensitve) in its name
*
***************************************************************************************************/

bool CEQCIsSubStringInName::Visit( const CEntity& obTestEntity ) const
{
	CScopedArray<char> pcNameTemp;

	pcNameTemp.Reset( NT_NEW char[strlen(obTestEntity.GetName().c_str())+1 ] );

	strcpy( pcNameTemp.Get(), obTestEntity.GetName().c_str() );
	ntstd::transform( pcNameTemp.Get(), pcNameTemp.Get() + strlen( pcNameTemp.Get() ), pcNameTemp.Get(), &tolower );

	return !!strstr( pcNameTemp.Get(), m_pcSubString.Get() );
}



/***************************************************************************************************
*
*	FUNCTION		CEQCIsRangedWeaponWithType::Visit
*
*	DESCRIPTION		Checks if an entity is a ranged weapon of a specific type.
*
***************************************************************************************************/
bool CEQCIsRangedWeaponWithType::Visit( const CEntity& obTestEntity ) const
{
	if(obTestEntity.IsRangedWeapon())
	{
		Object_Ranged_Weapon* pRangedWeapon = (Object_Ranged_Weapon*)&obTestEntity;
		if(pRangedWeapon->GetRangedWeaponType() == m_eWeaponType)
		{
			return true;
		}
	}

	return false;
}



/***************************************************************************************************
*
*	FUNCTION		CEQCDoesParentFitQuery::Visit
*
*	DESCRIPTION		See if the parent of this entity matches any of our candidates
*
***************************************************************************************************/
CEQCDoesParentFitQuery::CEQCDoesParentFitQuery( CEntityQuery& obParentCriteria )
{
	// Fire off the Query, cache the results
	CEntityManager::Get().FindEntitiesByType( obParentCriteria, CEntity::EntType_AllButStatic );

	for (	QueryResultsContainerType::iterator obIt = obParentCriteria.GetResults().begin();
			obIt != obParentCriteria.GetResults().end(); ++obIt )
	{
		m_obCandidates.push_back( *obIt );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CEQCDoesParentFitQuery::Visit
*
*	DESCRIPTION		See if the parent of this entity matches any of our candidates
*
***************************************************************************************************/

bool CEQCDoesParentFitQuery::Visit( const CEntity& obTestEntity ) const
{
	for (	ntstd::List<const CEntity*>::const_iterator obIt = m_obCandidates.begin();
			obIt != m_obCandidates.end(); ++obIt )
	{
		if ((*obIt) == obTestEntity.GetParentEntity())
			return true;
	}
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CEQCIsInFront::CEQCIsInFront
*
*	DESCRIPTION		
*
***************************************************************************************************/
CEQCIsInFront::CEQCIsInFront( const CPoint& obPosition, const CDirection& obDirection )
{
	m_obPosition=obPosition;

	m_obDirection=obDirection;
	m_obDirection.Normalise();
	
	m_fK=m_obPosition.X()*m_obDirection.X() + m_obPosition.Y()*m_obDirection.Y() + m_obPosition.Z()*m_obDirection.Z();
}

/***************************************************************************************************
*
*	FUNCTION		CEQCIsInFront::Visit
*
*	DESCRIPTION		See if the parent of this entity matches any of our candidates
*
***************************************************************************************************/
bool CEQCIsInFront::Visit( const CEntity& obTestEntity ) const
{
	if (obTestEntity.GetAttackComponent()) // Make sure we have a character
	{
		const CPoint& obPosition=obTestEntity.GetPosition();
		
		float fK2=obPosition.X()*m_obDirection.X() + obPosition.Y()*m_obDirection.Y() + obPosition.Z()*m_obDirection.Z();

		//ntPrintf("Entity %s is not infront (K=%f  K2=%f  Direction=%f,%f,%f)\n",obTestEntity.GetName().c_str(),m_fK,fK2,m_obDirection.X(),m_obDirection.Y(),m_obDirection.Z());

		if (fK2>m_fK) // Make sure this entity is in front
		{
			return true;
		}
	}

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	EQCMetaMatch::Visit
//!	
//!
//------------------------------------------------------------------------------------------
bool EQCMetaMatch::Visit(const CEntity& ent) const
{
	if(m_iType == TYPE_STRING)
	{
		const ntstd::String& obVal = ent.GetAttributeTable()->GetString( m_key );

		if( strcmp( obVal.c_str(), ntStr::GetString(*m_sVal)) == 0 )
			return true;
	}

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	EntityQueryExecutor::EntityQueryExecutor
//!	Construction
//!
//------------------------------------------------------------------------------------------
EntityQueryExecutor::EntityQueryExecutor(CEntityQuery* pQuery, const ntstd::List<CEntity*>* pQuerySpace)
	: m_pQuery(pQuery), m_pSpace(pQuerySpace)
{

}


//------------------------------------------------------------------------------------------
//!
//!	EntityQueryExecutor::GetAllMatches
//!	Returns a list of all the entities that match this query
//!
//------------------------------------------------------------------------------------------
QueryResultsContainerType& EntityQueryExecutor::GetAllMatches()
{
	// Add all the entities to the result set.
	for(CEntity* pEnt = GetFirstMatch_Private(); pEnt; pEnt = GetNextMatch_Private())
		m_pQuery->m_obSelectedResults.push_back((*m_itEnt));


	// And return the results.
	return m_pQuery->GetResults();
}


//------------------------------------------------------------------------------------------
//!
//!	EntityQueryExecutor::GetFirstMatch_Private
//!	Internal match first function.
//!
//------------------------------------------------------------------------------------------
CEntity* EntityQueryExecutor::GetFirstMatch_Private()
{
	m_itEnt = m_pSpace->begin();
	return GetNextMatch_Private();
}


//------------------------------------------------------------------------------------------
//!
//!	EntityQueryExecutor::GetFirstMatch_Private
//!	Internal match next function.
//!
//------------------------------------------------------------------------------------------
CEntity* EntityQueryExecutor::GetNextMatch_Private()
{
	for(; m_itEnt != m_pSpace->end(); m_itEnt++)
	{
		if(!*m_itEnt)
			continue;

		if( (*m_itEnt)->IsPaused() )
			continue;

		bool bPassedTest = true;
		// Check against positive clauses
		for(CEntityQuery::QuerySelectionContainerType::const_iterator itQuery = m_pQuery->m_obSelectionDetails.begin(); 
			itQuery != m_pQuery->m_obSelectionDetails.end(); itQuery++)
		{
			// Check if the test is passed
			if (!(*itQuery)->Visit(*(*m_itEnt)))
			{
				// If we failed the test then set the flag and break out
				bPassedTest = false;
				break;
			}
		}

		if(!bPassedTest)
			continue;

		// Now check against the negative clauses
		for(CEntityQuery::QuerySelectionContainerType::const_iterator itQuery = m_pQuery->m_obUnSelectionDetails.begin();
			itQuery != m_pQuery->m_obUnSelectionDetails.end(); itQuery++)
		{
			// Check if the test is failed - if this passes the test we remove it
			if((*itQuery)->Visit(*(*m_itEnt)))
			{
				// If we failed the test then set the flag and break out
				bPassedTest = false;
				break;
			}
		}

		if(!bPassedTest)
			continue;
	
		// The item has passed all the clauses so far...
		// Iterate through the list of excluded entities to make sure it isn't on of them
		for(QueryExcludedEntitiesContainerType::const_iterator itExclude = m_pQuery->m_obExcludedEntities.begin();
			itExclude != m_pQuery->m_obExcludedEntities.end(); itExclude++ )
		{
			if((*itExclude) == (*m_itEnt))
			{
				// This entity is excluded from the results
				bPassedTest = false;
				break;
			}
		}

		// If we passed the test add this entity to our list of found items
		if(bPassedTest)
			return *m_itEnt;
	}

	// And return the results.
	return 0;
}


EQCIsChildEntity::EQCIsChildEntity (CEntity* pobParentEntity) :
	m_pobParentEntity(pobParentEntity)
{
}

bool EQCIsChildEntity::Visit ( const CEntity& obTestEntity ) const
{
	if (obTestEntity.GetParentEntity()==m_pobParentEntity)
		return true;

	return false;
}


bool EQCIsSuitableForIncidental::Visit ( const CEntity& obTestEntity ) const
{
	if( !obTestEntity.IsAI() || 
		/*!obTestEntity.GetAIComponent()->GetFormation() || */
		!obTestEntity.IsAI() || 
		 ((const AI&)obTestEntity).GetAIComponent()->IsRecovering() )
	{
		return false;
	}

	/*
	if( obTestEntity.GetAIComponent()->GetFormationAttack() )
	{
		return false;
	}
	*/

	if(obTestEntity.ToCharacter()->IsDead() )
	{
		return false;
	}

	if( obTestEntity.GetAttackComponent()->AI_Access_GetState() != CS_STANDARD )
	{
		return false;
	}

	if( obTestEntity.GetEntityAudioChannel() && 
		obTestEntity.GetEntityAudioChannel()->IsPlaying(CHANNEL_VOICE) &&
		obTestEntity.GetEntityAudioChannel()->IsPlaying(CHANNEL_VOICE_HIGHPRI))
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------------------
//!  public virtual  Visit
//!
//!  @param [in]  rEnt	const CEntity &    The entity to check
//!
//!  @return bool whether the entity passes the visit for the check
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 29/06/2006
//------------------------------------------------------------------------------------------
bool CEQCOr::Visit( const CEntity& rEnt ) const
{
	// Loop through all the clauses
	for( ntstd::List< const CEntityQueryClause* >::const_iterator obIt = m_obClauses.begin();
			obIt != m_obClauses.end();
				++obIt )
	{
		if( (*obIt)->Visit( rEnt ) )
			return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!  public virtual  Visit
//!
//!  @param [in]  rEnt	const CEntity &    The entity to check
//!
//!  @return bool whether the entity passes the visit for the check
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 29/06/2006
//------------------------------------------------------------------------------------------
bool CEQCAnd::Visit( const CEntity& rEnt ) const
{
	u_int uiPassed = 0;

	// Loop through all the clauses
	for( ntstd::List< const CEntityQueryClause* >::const_iterator obIt = m_obClauses.begin();
			obIt != m_obClauses.end();
				++obIt )
	{
		if( (*obIt)->Visit( rEnt ) )
			++uiPassed;
	}

	return uiPassed == (u_int)m_obClauses.size();
}

//------------------------------------------------------------------------------------------
//!  public virtual  Visit
//!
//!  @param [in]  rEnt	const CEntity &    The entity to check
//!
//!  @return bool whether the entity passes the visit for the check
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 29/06/2006
//------------------------------------------------------------------------------------------
bool CEQCIsEntity::Visit( const CEntity& rEnt ) const
{
	// Loop through all the clauses
	for( ntstd::List< const CEntity* >::const_iterator obIt = m_obEntities.begin();
			obIt != m_obEntities.end();
				++obIt )
	{
		if( *obIt == &rEnt )
			return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!  public virtual constant  Visit
//!
//!  @param [in]        const CEntity &    
//!
//!  @return bool 
//!
//!  @remarks Checks whether the entity is of the following type
//!
//!  @author GavB @date 03/08/2006
//------------------------------------------------------------------------------------------
bool CEQCIsType::Visit( const CEntity& rEnt ) const
{
	return m_Items.Contains( rEnt.GetType().c_str() );
}


//------------------------------------------------------------------------------------------
//!  public virtual constant  Visit
//!
//!  @param [in]        const CEntity &    
//!
//!  @return bool 
//!
//!  @remarks Checks whether the entity is of the following type
//!
//!  @author GavB @date 03/08/2006
//------------------------------------------------------------------------------------------
bool CEQCIsDescription::Visit( const CEntity& rEnt ) const
{
	const CKeywords& obKeys = rEnt.GetAttributeTable()->GetString("Description").c_str();

	return m_Items.ContainsAny(obKeys);
}


//------------------------------------------------------------------------------------------
//!  public virtual constant  Visit
//!
//!  @param [in]        const CEntity &    
//!
//!  @return bool <TODO: insert return value description here>
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 16/08/2006
//------------------------------------------------------------------------------------------
bool CEQCIsEntityInArea::Visit( const CEntity& rEnt ) const
{
	return rEnt.InArea(m_Area);
	
}
