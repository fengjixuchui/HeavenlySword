/***************************************************************************************************
*
*	DESCRIPTION		Contains functionality used for searching the game database
*
*	NOTES
*
***************************************************************************************************/

#ifndef _QUERY_H
#define _QUERY_H

#include "editable/enumlist.h"
#include "game/keywords.h"

// Forward declarations
class CEntity;
class CEntityManager;

// Setup some typedefs
typedef ntstd::Vector< CEntity* > QueryResultsContainerType;
typedef ntstd::Vector< const CEntity* > QueryExcludedEntitiesContainerType;

/***************************************************************************************************
*
*	CLASS			CEntityQueryClause
*
*	DESCRIPTION		This class provides a base class for a query clause.
*
***************************************************************************************************/

class CEntityQueryClause
{
public:

	// Construction Destruction
	CEntityQueryClause( void ) {}
	virtual ~CEntityQueryClause( void ) {}

	// Find if an entity passes this test -
	// must be const because it must be repeatable...
	virtual bool Visit( const CEntity& obTestEntity ) const = 0;

protected:

};


/***************************************************************************************************
*
*	CLASS			CEntityQuery
*
*	DESCRIPTION		This is a full query object.  Add clauses to it and then pass it to the entity
*					manager which will give the objects a list of entities that fit the clauses.
*
***************************************************************************************************/

class CEntityQuery
{
public:

	// Construction Destruction
	CEntityQuery( void );
	~CEntityQuery( void );

	// Add a clause to the query - the query does NOT take ownership
	// of the clause we do not want to create copy constructors for each 
	// of our clause objects. Most queries should be stack based anyway
	void AddClause( const CEntityQueryClause& obClause );

	// This allows us to use the existing searches in a negative way
	void AddUnClause( const CEntityQueryClause& obClause );

	// We may want to exclude some entities from a search - it is very likely 
	// that we are likely to want to exclude the 'owner' of the search e.g if you 
	// want to find all the character within 10 meters of your player - you don't want
	// your player to come in the list
	void AddExcludedEntity( const CEntity& obExcludedEntity );

	// Retrieve a list of results from the class
	QueryResultsContainerType& GetResults( void ) { return m_obSelectedResults; }

	// Clear the list (Needed if you want to hold to a Query object for more than a frame)
	void ClearSelectedResults ( void ) { m_obSelectedResults.clear(); }

protected:

	// The Entity Manager will know how a query is structured
	friend class CEntityManager;
	friend class EntityQueryExecutor;

	// We'll hide these lists in here - the implementation could
	// possible change in the future, but the interface needs
	// to be as stable as possible
	typedef ntstd::Vector< const CEntityQueryClause* > QuerySelectionContainerType; 
	QuerySelectionContainerType m_obSelectionDetails;
	QuerySelectionContainerType m_obUnSelectionDetails;
	QueryResultsContainerType m_obSelectedResults;
	QueryExcludedEntitiesContainerType m_obExcludedEntities;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	EntityQueryExecutor
//! Executes a query in a given query space
//!                                                                                         
//------------------------------------------------------------------------------------------
class EntityQueryExecutor
{
public:
	EntityQueryExecutor() {;}
	EntityQueryExecutor(CEntityQuery* pQuery, const ntstd::List<CEntity*>* pQuerySpace);
	~EntityQueryExecutor() {;}

	void SetQuery(CEntityQuery* pQuery)    {m_pQuery = pQuery;}
	void SetSpace(const ntstd::List<CEntity*>* pSpace) {m_pSpace = pSpace;}

	CEntity *GetFirstMatch() {return GetFirstMatch_Private();}
	CEntity *GetNextMatch()  {return GetNextMatch_Private();}

	QueryResultsContainerType& GetAllMatches();

// Helpers
private:
	CEntity* GetFirstMatch_Private();
	CEntity* GetNextMatch_Private();

private:
	CEntityQuery*                   m_pQuery;
	const ntstd::List<CEntity*>*          m_pSpace;
	ntstd::List<CEntity*>::const_iterator m_itEnt;
};

/***************************************************************************************************
*
*	CLASS			CEntityQueryTools
*
*	DESCRIPTION		This is a static class that contains useful tools for looking for entities. It
*					is invisaged that this will be removed when a full database type search is
*					created for the entity manager.  For the moment this will do - it will 
*					essentially group together any useful bits so they all may be considered
*					properly in preproduction.
*
***************************************************************************************************/

class CEntityQueryTools
{
public:

	// Find the entity closest to the given point in the given list - will return 0 for an empty list
	static CEntity*	FindClosestEntity( const CPoint& obPosition, const ntstd::List< CEntity* >& obEntities ); 

	// Check whether an entity is within the given results set - returns true if it is so
	static bool		EntityInList( const CEntity* pobEntity, const ntstd::List< const CEntity* >& obEntities );

};


/***************************************************************************************************
****************************************************************************************************
****************************************************************************************************
*                                                                                                  *
*        BELOW ARE ALL THE POSSIBLE SEARCH QUERIES THAT CAN BE LINKED INTO THIS SYSTEM             *
*                                                                                                  *
****************************************************************************************************
****************************************************************************************************
***************************************************************************************************/



/***************************************************************************************************
*
*	CLASS			CEQCAlways
*
*	DESCRIPTION		Very simple query that accepts all entities regardless of race or religion.
*
***************************************************************************************************/

class CEQCAlways : public CEntityQueryClause
{
public:
	virtual bool Visit( CEntity const& ) const { return true; }
};



/***************************************************************************************************
*
*	CLASS			CEQCProximityColumn
*
*	DESCRIPTION		Checks if an entity of within the column about the y axis of the given matrix
*					of the given radius.
*
***************************************************************************************************/

class CEQCProximityColumn : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCProximityColumn( void ) : m_obMatrix( CONSTRUCT_IDENTITY ), m_fRadius( 0.0f ) {}
	virtual ~CEQCProximityColumn( void ) {}

	// Check what if the entity is with the defined column
	virtual bool Visit( const CEntity& obTestEntity ) const;

	// Details that need to be set before querying
	void	SetMatrix( const CMatrix& obMatrix )	{ m_obMatrix = obMatrix; }
	void	SetTranslation( const CPoint& obTranslation )	{ m_obMatrix.SetTranslation(obTranslation); }
	void	SetRadius( float fRadius )		{ m_fRadius = fRadius; }

protected:

	// Details to describe our column
	CMatrix		m_obMatrix;
	float		m_fRadius;

};


/***************************************************************************************************
*
*	CLASS			CEQCProximitySphere
*
*	DESCRIPTION		Checks if an entity of within the volume described be a sphere about the given
*					matrix.
*
***************************************************************************************************/

class CEQCProximitySphere : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCProximitySphere( void ) : m_obPosition( CONSTRUCT_CLEAR ), m_fRadiusSquared( 0.0f ) {}
	virtual ~CEQCProximitySphere( void ) {}

	// Check what if the entity is with the defined sphere
	virtual bool Visit( const CEntity& obTestEntity ) const;

	// Details that need to be set before querying
	void	Set (const CPoint& obPosition, float fRadius) { m_obPosition=obPosition; m_fRadiusSquared=fRadius*fRadius; }

protected:

	// Details to describe our sphere
	CPoint		m_obPosition;
	float		m_fRadiusSquared;

};

/***************************************************************************************************
*
*	CLASS			CEQCProximitySegment
*
*	DESCRIPTION		Checks if the given entity is with the segment of a column decribed by the y
*					axis of the member matrix and the member radius value.  The angle describes the
*					full slice - ie the point has to be within 50% of the full sweep from the 
*					z axis of the member matrix.
*
***************************************************************************************************/

class CEQCProximitySegment : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCProximitySegment( void ) : m_obMatrix( CONSTRUCT_IDENTITY ), m_fRadius( 0.0f ) {}
	virtual ~CEQCProximitySegment( void ) {}

	// Check what if the entity is with the defined segment
	virtual bool Visit( const CEntity& obTestEntity ) const;

	// Details that need to be set before querying
	void	SetMatrix( const CMatrix& obMatrix )	{ m_obMatrix = obMatrix; }
	void	SetRadius( float fRadius )		{ m_fRadius = fRadius; }
	float	GetRadius( ) const				{ return m_fRadius; }
	void	SetAngle( float fAngle )		{ m_fAngle = ( fAngle / 2.0f ); }


	// Full access is given to the matrix
	CMatrix& GetMatrix(void) { return m_obMatrix; }

protected:

	// Details to describe our segment
	CMatrix		m_obMatrix;
	float		m_fRadius;
	float		m_fAngle;
};


/***************************************************************************************************
*
*	CLASS			CEQCHeightRange
*
*	DESCRIPTION		Check the height of an object - has a top and bottom height - looks straight
*					at the Y coordinate.
*
***************************************************************************************************/

class CEQCHeightRange : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCHeightRange( void ) : m_fTop( 0.0f ), m_fBottom( 0.0f ) {}
	virtual ~CEQCHeightRange( void ) {}

	// Check what if the entity is with the defined segment
	virtual bool Visit( const CEntity& obTestEntity ) const;

	// Details that need to be set before querying
	void	SetTop( float fTop )			{ m_fTop = fTop; }
	void	SetBottom( float fBottom )		{ m_fBottom = fBottom; }
	void	SetRelativeY( float fY )		{ m_fRelativeY = fY; }

protected:

	// Details to describe our segment
	float		m_fRelativeY;
	float		m_fTop;
	float		m_fBottom;
};


/***************************************************************************************************
*
*	CLASS			CEQLookable
*
*	DESCRIPTION		checks how "lookable" an entity is. Only entities with LookAtInfo
*					will be considered. 
*	NOTE			zero is considered the top most priority
*
***************************************************************************************************/

class CEQLookable : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQLookable( void ) : m_iMaxPriority( 0xffffffff ) {}
	virtual ~CEQLookable( void ) {}

	// check that entity is lookable and below the given threshold
	virtual bool Visit( const CEntity& obTestEntity ) const;

	// set the max bound for priority so we'll only check [0,iPriority]
	void	SetMaxPriority( u_int iPriority ) { m_iMaxPriority = iPriority; }

protected:
	u_int	m_iMaxPriority;
};

/***************************************************************************************************
*
*	CLASS			CEQCIsEnemy
*
*	DESCRIPTION		Checks if an entity is an enemy
*
***************************************************************************************************/

class CEQCIsEnemy : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsEnemy( void ) {}
	virtual ~CEQCIsEnemy( void ) {}

	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:

};

/***************************************************************************************************
*
*	FUNCTION		CEQCIsTargetableByPlayer
*
*	DESCRIPTION		Checks whether this entity is either a player of an enemy, in which case is 
*					targetable
*
***************************************************************************************************/

class CEQCIsTargetableByPlayer : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsTargetableByPlayer( void ) {}
	virtual ~CEQCIsTargetableByPlayer( void ) {}

	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const;
};

/***************************************************************************************************
*
*	FUNCTION		CEQCIsTargetableForEvade
*
*	DESCRIPTION		Checks whether this entity is either a player or an enemy, and is not dead.
*
***************************************************************************************************/

class CEQCIsTargetableForEvade : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsTargetableForEvade( void ) {}
	virtual ~CEQCIsTargetableForEvade( void ) {}

	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const;
};

/***************************************************************************************************
*
*	FUNCTION		CEQCIsGroundAttackable
*
*	DESCRIPTION		Checks whether this entity is floored or rise waiting
*
***************************************************************************************************/
class CEQCIsGroundAttackable : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsGroundAttackable( void ) {}
	virtual ~CEQCIsGroundAttackable( void ) {}

	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const;
};



/***************************************************************************************************
*
*	CLASS			CEQCIsInNinjaSequence
*
*	DESCRIPTION		Checks if an entity is currently involved in a ninja sequence/cutscene
*
***************************************************************************************************/

class CEQCIsInNinjaSequence : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsInNinjaSequence( void ) {}
	virtual ~CEQCIsInNinjaSequence( void ) {}

	// Check whether the entity is in a ninja sequence/cutscene
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:

};



/***************************************************************************************************
*
*	CLASS			CEQCIsAerialComboable
*
*	DESCRIPTION		Checks if an entity is a player or an enemy and is KOed in their combat state
*
***************************************************************************************************/
class CEQCIsAerialComboable : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsAerialComboable( void ) {}
	virtual ~CEQCIsAerialComboable( void ) {}

	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:

};

/***************************************************************************************************
*
*	CLASS			CEQCIsCombatActive
*
*	DESCRIPTION		Checks if an entity has an enabled combat component
*
***************************************************************************************************/
class CEQCIsCombatComponentActive : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsCombatComponentActive( void ) {}
	virtual ~CEQCIsCombatComponentActive( void ) {}

	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:

};

/***************************************************************************************************
*
*	CLASS			CEQCHealthLTE
*
*	DESCRIPTION		Checks if an entity has an a curtain amount of health
*
***************************************************************************************************/
class CEQCHealthLTE : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCHealthLTE( float fCheck )  : m_fHealthCheck(fCheck) {}
	virtual ~CEQCHealthLTE( void ) {}

	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:
	float m_fHealthCheck;

};


/***************************************************************************************************
*
*	CLASS			CEQCIsCombatTargetingDisabled
*
*	DESCRIPTION		Checks if an entity has an enabled combat targeting component
*
***************************************************************************************************/
class CEQCIsCombatTargetingDisabled : public CEntityQueryClause
{
	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const;
};



/***************************************************************************************************
*
*	CLASS			CEQCIsLockonable
*
*	DESCRIPTION		Checks if an entity is an enemy
*
***************************************************************************************************/

class CEQCIsLockonable : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsLockonable( void ) {}
	virtual ~CEQCIsLockonable( void ) {}

	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:

};

/***************************************************************************************************
*
*	CLASS			CEQCIsInteractionTarget
*
*	DESCRIPTION		Check the interaction type.
*
***************************************************************************************************/
class CEQCIsInteractionTarget : public CEntityQueryClause
{
public:

	CEQCIsInteractionTarget() {}

	virtual ~CEQCIsInteractionTarget( void ) {}

	virtual bool Visit( const CEntity& obTestEntity ) const;
};


/***************************************************************************************************
*
*	CLASS			CEQCIsCanTakeStandardHit
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CEQCIsCanTakeStandardHit : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsCanTakeStandardHit( void ) {}
	virtual ~CEQCIsCanTakeStandardHit( void ) {}

	// Check whether the entity can be hit
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:

};


/***************************************************************************************************
*
*	CLASS			CEQCIsCanTakeSpeedExtraHit
*
*	DESCRIPTION		
*
***************************************************************************************************/
class CEQCIsCanTakeSpeedExtraHit : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsCanTakeSpeedExtraHit( void ) {}
	virtual ~CEQCIsCanTakeSpeedExtraHit( void ) {}

	// Check whether the entity can be hit
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:

};


/***************************************************************************************************
*
*	CLASS			CEQCIsThrownTarget
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CEQCIsThrownTarget : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsThrownTarget( void ) {}
	virtual ~CEQCIsThrownTarget( void ) {}

	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:

};

/***************************************************************************************************
*
*	CLASS			CEQCIsLargeInteractable
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CEQCIsLargeInteractable : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsLargeInteractable( void ) {}
	virtual ~CEQCIsLargeInteractable( void ) {}

	// Check whether the entity is a large interactable
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:

};


/***************************************************************************************************
*
*	CLASS			CEQCIsLargeInteractable
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CEQCShouldAvoid : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCShouldAvoid( void ) {}
	virtual ~CEQCShouldAvoid( void ) {}

	// Check whether the entity is a large interactable
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:

};



/***************************************************************************************************
*
*	CLASS			CEQCIsSubStringInName
*
*	DESCRIPTION		Checks if an entity has this sub string (case insensitve) in its name
*
***************************************************************************************************/

class CEQCIsSubStringInName : public CEntityQueryClause
{
public:

	CEQCIsSubStringInName( const char* pcSubStr )
	{
		ntAssert(pcSubStr);
		m_pcSubString.Reset( NT_NEW char[ strlen(pcSubStr)+1 ] );
		
		strcpy( m_pcSubString.Get(), pcSubStr );
		ntstd::transform( m_pcSubString.Get(), m_pcSubString.Get() + strlen( m_pcSubString.Get() ), m_pcSubString.Get(), &ntstd::Tolower );
	}

	virtual ~CEQCIsSubStringInName( void ) {}

	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:
	CScopedArray<char> m_pcSubString;
};



/***************************************************************************************************
*
*	CLASS			CEQCIsRangedWeaponWithType
*
*	DESCRIPTION		Checks if an entity has is a ranged weapon of a specific type.
*
***************************************************************************************************/
class CEQCIsRangedWeaponWithType : public CEntityQueryClause
{
public:
	CEQCIsRangedWeaponWithType(RANGED_WEAPON_TYPE eWeaponType)	//TODO: We have an enum for weapon types.
	{
		ntAssert_p((eWeaponType > RWT_NONE) && (eWeaponType < RWT_NUMTYPES), ("Trying to query for invalid ranged weapon type"));
		//TODO: Check that weapon-type is in-range.
		m_eWeaponType = eWeaponType;
	}

	virtual ~CEQCIsRangedWeaponWithType() {}
	virtual bool Visit( const CEntity& obTestEntity ) const;
protected:
	RANGED_WEAPON_TYPE m_eWeaponType;	//TODO: ENUM!
};



/***************************************************************************************************
*
*	CLASS			CEQCDoesParentFitQuery
*
*	DESCRIPTION		Checks to see if the parent of an entity fits this query
*
***************************************************************************************************/

class CEQCDoesParentFitQuery : public CEntityQueryClause
{
public:

	CEQCDoesParentFitQuery( CEntityQuery& obParentCriteria );

	virtual ~CEQCDoesParentFitQuery( void ) {}

	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:
	ntstd::List<const CEntity*>	 m_obCandidates;
};

/***************************************************************************************************
*
*	CLASS			CEQCIsThis
*
*	DESCRIPTION		Checks if an entity is this entity
*
***************************************************************************************************/

class CEQCIsThis : public CEntityQueryClause
{
public:

	// Construction Destruction
	CEQCIsThis( const CEntity* pobEntity ) { m_pobEntity = pobEntity; }
	virtual ~CEQCIsThis( void ) {}

	// Check whether the entity is an enemy
	virtual bool Visit( const CEntity& obTestEntity ) const { return m_pobEntity == &obTestEntity; }

protected:
	const CEntity* m_pobEntity;
};


/***************************************************************************************************
*
*	CLASS			CEQCIsInFront
*
*	DESCRIPTION		Checks if an entity is in front of a given direction relative to a point.
*
***************************************************************************************************/
class CEQCIsInFront : public CEntityQueryClause
{
public:

	CEQCIsInFront( const CPoint& obPosition, const CDirection& obDirection );
	virtual ~CEQCIsInFront( void ) {}

	virtual bool Visit( const CEntity& obTestEntity ) const;

protected:

	CPoint m_obPosition;
	CDirection m_obDirection;
	float m_fK;
};


//------------------------------------------------------------------------------------------
//!
//!	EQCMetaMatch
//!	Match an entity on some meta (or not) data associated with it.
//!
//------------------------------------------------------------------------------------------
class EQCMetaMatch : public CEntityQueryClause
{
public:
	EQCMetaMatch(const CHashedString& key, int  value)				 {m_key = key; m_iType = TYPE_INT;    m_iVal = value;}
	EQCMetaMatch(const CHashedString& key, bool value)				 {m_key = key; m_iType = TYPE_BOOL;   m_bVal = value;}
	EQCMetaMatch(const CHashedString& key, const CKeyString& value) {m_key = key; m_iType = TYPE_STRING; m_sVal = NT_NEW CKeyString(value);}
	virtual ~EQCMetaMatch() {if(m_iType == TYPE_STRING && m_sVal) NT_DELETE( m_sVal );}

	virtual bool Visit(const CEntity& ent) const;

protected:
	static const int TYPE_INT  = 0;
	static const int TYPE_BOOL = 1;
	static const int TYPE_STRING = 2;

protected:
	CHashedString m_key;
	char  m_iType;
	union
	{
		int         m_iVal;
		bool        m_bVal;
		CKeyString* m_sVal;
	};
};

//------------------------------------------------------------------------------------------
//!
//!	EQCIsChildEntity
//!	Match an entity on some meta (or not) data associated with it.
//!
//------------------------------------------------------------------------------------------
class EQCIsChildEntity : public CEntityQueryClause
{
public:

	EQCIsChildEntity (CEntity* pobParentEntity);

	virtual bool Visit ( const CEntity& obTestEntity ) const;

protected:

	CEntity* m_pobParentEntity;
};

//------------------------------------------------------------------------------------------
//!
//!	EQCIsSuitableForIncidental
//!	Can the entity perform an incidental animation?
//!
//------------------------------------------------------------------------------------------
class EQCIsSuitableForIncidental : public CEntityQueryClause
{
public:

	EQCIsSuitableForIncidental (void) {}

	virtual bool Visit ( const CEntity& obTestEntity ) const;
};


//!------------------------------------------------------------------------------
//!  CEQCOr
//!  Adds _OR_ type functionality to the query system
//!
//!  Base class CEntityQueryClause 
//!
//!  @author GavB @date 29/06/2006
//!------------------------------------------------------------------------------
class CEQCOr : public CEntityQueryClause
{
public:
	CEQCOr(){}
	virtual ~CEQCOr(){}

	/// 
	virtual bool Visit( CEntity const& ) const;

	/// 
	void AddClause( const CEntityQueryClause& rClause ) { m_obClauses.push_back( &rClause ); }

private:
	ntstd::List< const CEntityQueryClause* > m_obClauses;
};

//!------------------------------------------------------------------------------
//!  CEQCAnd
//!  Adds _AND_ type functionality to the query system
//!
//!  Base class CEntityQueryClause 
//!
//!  @author GavB @date 29/06/2006
//!------------------------------------------------------------------------------
class CEQCAnd : public CEntityQueryClause
{
public:
	CEQCAnd(){}
	virtual ~CEQCAnd(){}

	/// 
	virtual bool Visit( CEntity const& ) const;

	/// 
	void AddClause( const CEntityQueryClause& rClause ) { m_obClauses.push_back( &rClause ); }

private:
	ntstd::List< const CEntityQueryClause* > m_obClauses;
};

//!------------------------------------------------------------------------------
//!  CEQCIsEntity
//!  Checks whether the entity is one in the list built up within
//!
//!  Base class CEntityQueryClause 
//!
//!  @author GavB @date 29/06/2006
//!------------------------------------------------------------------------------
class CEQCIsEntity : public CEntityQueryClause
{
public:
	CEQCIsEntity(){}
	virtual ~CEQCIsEntity(){}

	/// 
	virtual bool Visit( CEntity const& ) const;

	/// 
	void AddEntity( const CEntity& rEnt ) { m_obEntities.push_back( &rEnt ); }

private:
	ntstd::List< const CEntity* > m_obEntities;
};

//!------------------------------------------------------------------------------
//!  CEQCIsType
//!  Checks whether the entity is of the following type
//!
//!  Base class CEntityQueryClause 
//!
//!  @author GavB @date 29/06/2006
//!------------------------------------------------------------------------------
class CEQCIsType : public CEntityQueryClause
{
public:
	CEQCIsType( ) {}
	virtual ~CEQCIsType(){}

	/// 
	virtual bool Visit( CEntity const& ) const;

	// Set the keywords. 
	void SetKeywords( const CKeywords& obKeys )
	{
		m_Items = obKeys;
	}

private:
	
	// Description made as keywords
	CKeywords	m_Items;
};

//!------------------------------------------------------------------------------
//!  CEQCIsDescription
//!  Checks whether the entity is of the following description
//!
//!  Base class CEntityQueryClause 
//!
//!  @author GavB @date 29/06/2006
//!------------------------------------------------------------------------------
class CEQCIsDescription : public CEntityQueryClause
{
public:
	CEQCIsDescription( ) {}
	virtual ~CEQCIsDescription(){}

	/// 
	virtual bool Visit( CEntity const& ) const;

	// Set the keywords. 
	void SetKeywords( const CKeywords& obKeys )
	{
		m_Items = obKeys;
	}

private:

	// Description made as keywords
	CKeywords	m_Items;
};

//!------------------------------------------------------------------------------
//!  CEQCIsEntityInArea
//!  Checks whether the entity is in a Aera
//!
//!  Base class CEntityQueryClause 
//!
//!  @author GavB @date 16/08/2006
//!------------------------------------------------------------------------------
class CEQCIsEntityInArea : public CEntityQueryClause
{
public:
	CEQCIsEntityInArea(uint32_t uArea) : m_Area(uArea) {}
	virtual ~CEQCIsEntityInArea(){}

	/// 
	virtual bool Visit( CEntity const& ) const;
private:
	uint32_t	m_Area;
};


#endif // _QUERY_H
