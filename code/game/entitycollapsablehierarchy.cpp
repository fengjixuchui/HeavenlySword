//--------------------------------------------------
//!
//!	\file game/entityCollapsableHierarchy.cpp
//!	Definition of the collapsable hierarchy object
//!
//--------------------------------------------------

// Includes
#include "game/entitycollapsablehierarchy.h"
#include "objectdatabase/dataobject.h"
#include "Physics/animatedlg.h"
#include "audio/audiohelper.h"
#include "effect/fxhelper.h"
#include "game/entityprojectile.h"

// Components
#include "game/interactioncomponent.h"
#include "Physics/system.h"


typedef ntstd::List<Object_CollapsableHierarchySubPart*, Mem::MC_ENTITY>::iterator SubPartIterator;


// ForceLink function for PS3
void ForceLinkFunctionCollapsableHierarchy()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionCollapsableHierarchy() !ATTN!\n");
}


// Used to create unique sub part names
int g_iSubPartID = 0;


// CollapsableHierarchySubPartDef interface
START_CHUNKED_INTERFACE(CollapsableHierarchySubPartDef, Mem::MC_ENTITY)
	PUBLISH_VAR_AS( m_Description, Description )
	PUBLISH_VAR_AS( m_Clump, Clump )
	PUBLISH_VAR_AS( m_PositionOffset, PositionOffset )
	PUBLISH_VAR_AS( m_iPartIndex, PartIndex )
	PUBLISH_CONTAINER_AS( m_aiChildIndices, ChildPartIndices )
END_STD_INTERFACE

START_CHUNKED_INTERFACE(Object_CollapsableHierarchySubPart, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE

// Att_CollapsableHierarchy interface
START_CHUNKED_INTERFACE(Att_CollapsableHierarchy, Mem::MC_ENTITY)
	PUBLISH_CONTAINER_AS( m_PfxListPartHit,				PfxListPartHit )
	PUBLISH_CONTAINER_AS( m_PfxListPartBreakOff,		PfxListPartBreakOff )
	PUBLISH_CONTAINER_AS( m_PfxListPartDestroy,			PfxListPartDestroy )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_SfxPartHit, "",		SfxPartHit )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_SfxPartBreakOff, "", SfxPartBreakOff )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_SfxPartDestroy, "",	SfxPartDestroy )

	// Vulnerabilities here
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bVulnerable_Bolt, true, Vulnerable_Bolt )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_iNumLevelsToInheritHitVelocity, 2, NumLevelsToInheritHitVelocity )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fPercentageVelocityInheritedFromParent, 20.0f, PercentageVelocityInheritedFromParent )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fMaxBreakVelocity, 0.5f, MaxBreakVelocity )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fPercentageVelocityInheritedFromProjectile, 5.0f, PercentageVelocityInheritedFromProjectile )

	PUBLISH_PTR_CONTAINER_AS( m_SubPartDefinitions, SubPartDefinitions )
END_STD_INTERFACE


// Object_CollapsableHierarchy interface
START_CHUNKED_INTERFACE(Object_CollapsableHierarchy, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_PTR_AS(	m_pSharedAttributes, SharedAttributes )

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Animated CollapsableHierarchy State Machine
//!
//--------------------------------------------------
STATEMACHINE(COLLAPSABLE_HIERARCHY_FSM, Object_CollapsableHierarchy)
	COLLAPSABLE_HIERARCHY_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE
END_STATEMACHINE //COLLAPSABLE_HIERARCHY_FSM


//--------------------------------------------------
//!
//! Animated CollapsableHierarchy State Machine
//!
//--------------------------------------------------
STATEMACHINE(COLLAPSABLE_HIERARCHY_SUBPART_FSM, Object_CollapsableHierarchySubPart)
	COLLAPSABLE_HIERARCHY_SUBPART_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Set State
				ME->m_eCollapseState = Object_CollapsableHierarchySubPart::COLLAPSE_STATE_DEFAULT;	
			}
			END_EVENT(true)

			EVENT(msg_projcol)
			{
				Object_Projectile* pProjectile = (Object_Projectile*)msg.GetEnt("Projectile");
				Att_CollapsableHierarchy* pSharedAttribs = ME->m_pSharedAttributes;
				ntAssert( pSharedAttribs );
				
				// Are we vulnerable to bolts?
				if ( pSharedAttribs->m_bVulnerable_Bolt )
				{
					// Calculate the velocity that the part should break off with
					CDirection obInitialVelocity = pProjectile->GetPhysicsSystem()->GetLinearVelocity();
					obInitialVelocity *= ( 0.01f * pSharedAttribs->m_fPercentageVelocityInheritedFromProjectile );

					// Do we need to cap the initial velocity?
					float fMaxBreakVelocitySq = pSharedAttribs->m_fMaxBreakVelocity * pSharedAttribs->m_fMaxBreakVelocity;
					if ( obInitialVelocity.LengthSquared() > fMaxBreakVelocitySq )
					{
						obInitialVelocity.Normalise();
						obInitialVelocity *= pSharedAttribs->m_fMaxBreakVelocity;
					}

					// Break off the part
					ME->BreakOffPart( obInitialVelocity, 0 );
				}
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(BROKENOFF)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Set State
				ME->m_eCollapseState = Object_CollapsableHierarchySubPart::COLLAPSE_STATE_BROKEN_OFF;

				// Reparent to world
				ME->Lua_ReparentToWorld();

				// Place under physics
				Physics::AnimatedLG* lg = (Physics::AnimatedLG*)ME->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
				if(lg && lg->IsActive())
				{
					lg->MakeDynamic();
					ME->GetPhysicsSystem()->Lua_AltSetLinearVelocity( ME->m_obInitialVelocity );
				}

				// Enable collisions with other parts
				ME->m_pMainParent->SubPartEnableCollisions( ME );

				// Notify the main parent that we're being broken off
				ME->m_pMainParent->SubPartBrokenOff( ME );
			}
			END_EVENT(true)

			EVENT(msg_projcol)
			{
				// Destroy state
				SET_STATE(DESTROYED);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(DESTROYED)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Set State
				ME->m_eCollapseState = Object_CollapsableHierarchySubPart::COLLAPSE_STATE_DESTROYED;

				// Destroy part

				// Particle effects

				// Sound effects
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

END_STATEMACHINE //COLLAPSABLE_HIERARCHY_SUBPART_FSM


//--------------------------------------------------
//!
//!	Object_CollapsableHierarchySubPart::Object_CollapsableHierarchySubPart()
//!	Default constructor
//!
//--------------------------------------------------
Object_CollapsableHierarchySubPart::Object_CollapsableHierarchySubPart()
{
	m_eType = EntType_Interactable;
	m_obInitialVelocity.Clear();
}

//--------------------------------------------------
//!
//!	Object_CollapsableHierarchySubPart::~Object_CollapsableHierarchySubPart()
//!	Default Destructor
//!
//--------------------------------------------------
Object_CollapsableHierarchySubPart::~Object_CollapsableHierarchySubPart()
{

}


//--------------------------------------------------
//!
//!	Object_CollapsableHierarchySubPart::OnPostConstruct
//!
//--------------------------------------------------
void Object_CollapsableHierarchySubPart::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	// Install components
	InstallMessageHandler();
	InstallDynamics();	

	Physics::AnimatedLG* lg = (Physics::AnimatedLG*)GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
	if(lg)
	{
		lg->Activate(true);
	}

	// Create and attach the statemachine
	COLLAPSABLE_HIERARCHY_SUBPART_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) COLLAPSABLE_HIERARCHY_SUBPART_FSM();
	ATTACH_FSM(pFSM);
}


//--------------------------------------------------
//!
//!	Object_CollapsableHierarchySubPart::BreakOffPart
//!	Breaks off the part, sets the state machine to BrokenOff
//! and places it under physics
//!
//--------------------------------------------------
void Object_CollapsableHierarchySubPart::BreakOffPart( const CDirection& obInitialVelocity, int iRecursionLevels )
{
	ntAssert( m_pSharedAttributes );

	//ntPrintf("Breaking off part %d\n", m_pDef->m_iPartIndex );

	// Only break it off it's in its default state
	if ( m_eCollapseState != COLLAPSE_STATE_DEFAULT )
	{
		return;
	}

	// Store the velocity this part should get broken off with
	m_obInitialVelocity = obInitialVelocity;

	// Send messages to child parts to break off
	if ( !m_apChildSubParts.empty() )
	{
		CDirection obInheritedBreakVelocity( CONSTRUCT_CLEAR );

		// Do we inherit some velocity?
		if ( iRecursionLevels < m_pSharedAttributes->m_iNumLevelsToInheritHitVelocity )
		{
			// Inherit velocity
			obInheritedBreakVelocity = obInitialVelocity;

			// Scale velocity to percentage inherited
			obInheritedBreakVelocity *= ( 0.01f * m_pSharedAttributes->m_fPercentageVelocityInheritedFromParent );
		}

		// Go through the children popping them off the list as we use them
		Object_CollapsableHierarchySubPart* pChild = 0;

		if ( !m_apChildSubParts.empty() )
		{
			pChild = m_apChildSubParts.front();
			m_apChildSubParts.pop_front();
		}

		while ( pChild )
		{
			pChild->BreakOffPart( obInheritedBreakVelocity, iRecursionLevels + 1 );

			if ( !m_apChildSubParts.empty() )
			{
				pChild = m_apChildSubParts.front();
				m_apChildSubParts.pop_front();
			}
			else
			{
				pChild = 0;
			}
		}

		// Break double-links with parent parts
		SubPartIterator endParentIter = m_apParentSubParts.end();
		for ( SubPartIterator parentIter = m_apParentSubParts.begin(); parentIter != endParentIter; parentIter++ )
		{
			Object_CollapsableHierarchySubPart* pParent = (*parentIter);
			pParent->RemoveChildSubPartPtr( this );
		}
		m_apParentSubParts.clear();
	}

	// Set the state machine to BrokenOff
	EXTERNALLY_SET_STATE( COLLAPSABLE_HIERARCHY_SUBPART_FSM, BROKENOFF );
}


//--------------------------------------------------
//!
//!	Object_CollapsableHierarchySubPart::RemoveChildSubPartPtr()
//!	Removes a child part from the child list
//!
//--------------------------------------------------
void Object_CollapsableHierarchySubPart::RemoveChildSubPartPtr( Object_CollapsableHierarchySubPart* pChildPart )
{
	ntAssert( pChildPart );

	m_apChildSubParts.remove( pChildPart );
}


//--------------------------------------------------
//!
//!	Object_CollapsableHierarchy::Object_CollapsableHierarchy()
//!	Default constructor
//!
//--------------------------------------------------
Object_CollapsableHierarchy::Object_CollapsableHierarchy()
{
	m_eType = EntType_Interactable;

	m_pSharedAttributes = 0;
}

//--------------------------------------------------
//!
//!	Object_CollapsableHierarchy::~Object_CollapsableHierarchy()
//!	Default Destructor
//!
//--------------------------------------------------
Object_CollapsableHierarchy::~Object_CollapsableHierarchy()
{

}


//--------------------------------------------------
//!
//!	Object_CollapsableHierarchy::OnPostConstruct
//!
//--------------------------------------------------
void Object_CollapsableHierarchy::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	// Install components
	InstallMessageHandler();
	InstallDynamics();

	// Create the sub parts
	ConstructSubParts();

	// Disable all collisions between the sub parts and the main part
	DisableAllInternalCollisions();

	// Create and attach the statemachine
	COLLAPSABLE_HIERARCHY_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) COLLAPSABLE_HIERARCHY_FSM();
	ATTACH_FSM(pFSM);
}


//--------------------------------------------------
//!
//!	Object_CollapsableHierarchy::SubPartBrokenOff
//! Sub Part is now going under physics control so
//! enable collisions with it
//!
//--------------------------------------------------
void Object_CollapsableHierarchy::SubPartBrokenOff( Object_CollapsableHierarchySubPart* pBrokenOffSubPart )
{
	ntAssert( pBrokenOffSubPart );

	// Remove it from the list
	m_obSubParts.remove( pBrokenOffSubPart );
}


//--------------------------------------------------
//!
//!	Object_CollapsableHierarchy::SubPartEnableCollisions
//! Sub Part is now going under physics control so
//! enable collisions with it
//!
//--------------------------------------------------
void Object_CollapsableHierarchy::SubPartEnableCollisions( Object_CollapsableHierarchySubPart* pSubPart )
{
	ntAssert( pSubPart );

	// Main (this) vs. Sub Part
	m_pobInteractionComponent->AllowCollisionWith( pSubPart );

	// Sub Part vs. Other Sub Parts
	ntstd::List<Object_CollapsableHierarchySubPart*, Mem::MC_ENTITY>::iterator iter;
	ntstd::List<Object_CollapsableHierarchySubPart*, Mem::MC_ENTITY>::iterator iterEnd = m_obSubParts.end();
	
	Object_CollapsableHierarchySubPart* pOtherSubPart = 0;
	for ( iter = m_obSubParts.begin(); iter != iterEnd; iter++ )
	{
		pOtherSubPart = (*iter);
		if ( pOtherSubPart != pSubPart )
		{
			pSubPart->GetInteractionComponent()->AllowCollisionWith( pOtherSubPart );
		}
	}
}


//--------------------------------------------------
//!
//!	Object_CollapsableHierarchy::ConstructSubParts()
//!	Creates all the sub parts of the object and adds
//! them to the list
//!
//--------------------------------------------------
void Object_CollapsableHierarchy::ConstructSubParts()
{
	ntAssert( m_pSharedAttributes );

	Object_CollapsableHierarchySubPart* pNewPart = 0;
	CollapsableHierarchySubPartDef* pNewDef = 0;
	char name[64] = {0};

	// For each new sub part to create
	ntstd::List<CollapsableHierarchySubPartDef*, Mem::MC_ENTITY>::iterator iter;
	ntstd::List<CollapsableHierarchySubPartDef*, Mem::MC_ENTITY>::iterator iterEnd = m_pSharedAttributes->m_SubPartDefinitions.end();

	for ( iter = m_pSharedAttributes->m_SubPartDefinitions.begin(); iter != iterEnd; iter++ )
	{
		pNewDef = (*iter);

		// Generate a name for it
		sprintf(name, "CollapsableHierarchySubPart%d", g_iSubPartID);
		g_iSubPartID++;

		// Create it in Database
		DataObject* pDO = ObjectDatabase::Get().ConstructObject("Object_CollapsableHierarchySubPart", name, GameGUID(), 0, true, false);

		pNewPart = (Object_CollapsableHierarchySubPart*)pDO->GetBasePtr();
		ntAssert( pNewPart );

		// Give it an attribute table and get a pointer to it
		pNewPart->SetAttributeTable(LuaAttributeTable::Create());
		pNewPart->GetAttributeTable()->SetDataObject(pDO);
		LuaAttributeTable* pAttributes = pNewPart->GetAttributeTable();
		
		// Name
		pAttributes->SetAttribute("Name", name);

		// Clump
		pAttributes->SetAttribute("Clump", pNewDef->m_Clump.c_str());

		// Description
		pAttributes->SetAttribute("Description", "subpart");

		// DefaultDynamics
		pAttributes->SetAttribute("DefaultDynamics", "Animated");

		// Copy in data from the def
		pNewPart->m_Description = "subpart";

		ObjectDatabase::Get().DoPostLoadDefaults(pDO);

		// Rotate the piece itself
		pNewPart->SetRotation( GetRotation() );

		// Rotate position around parent
		CMatrix obRotationMatrix( GetRotation() );

		CPoint obNewPos = pNewDef->m_PositionOffset * obRotationMatrix;

		// Add on the position of the base part
		obNewPos += GetPosition();

		// Move to correct position
		pNewPart->SetPosition( obNewPos );
	
		// Parent sub part to this main base
		pNewPart->SetParentEntity( this );

		// Link to parent and vice versa
		pNewPart->m_pMainParent = this;

		// Set the shared attributes pointer
		pNewPart->m_pSharedAttributes = m_pSharedAttributes;

		// Point part at it's definition
		pNewPart->m_pDef = pNewDef;

		m_obSubParts.push_back( pNewPart );
	}

	// Link up the parts
	SubPartIterator partIterEnd = m_obSubParts.end();

	// Go through each part as if it was the parent, and then link to its child parts
	for ( SubPartIterator parentIter = m_obSubParts.begin(); parentIter != partIterEnd; parentIter++ )
	{
		Object_CollapsableHierarchySubPart* pParentPart = (*parentIter);
		ntAssert( pParentPart->m_pDef );

		// Go through the child indices
		ntstd::List<int>::iterator childIndexIterEnd = pParentPart->m_pDef->m_aiChildIndices.end();
		for ( ntstd::List<int>::iterator childIndexIter = pParentPart->m_pDef->m_aiChildIndices.begin(); childIndexIter != childIndexIterEnd; childIndexIter++ )
		{
			int iChildIndex = (*childIndexIter);
			
			// Sanity check - can't link to itself!
			ntAssert( iChildIndex != pParentPart->m_pDef->m_iPartIndex );

			// Go through all the sub parts looking for the child index
			for ( SubPartIterator childIter = m_obSubParts.begin(); childIter != partIterEnd; childIter++ )
			{
				Object_CollapsableHierarchySubPart* pChildPart = (*childIter);

				if ( pChildPart == pParentPart )
					continue;

				// Check for same indices
				if ( pChildPart->m_pDef->m_iPartIndex == iChildIndex )
				{
					// MATCH! LINK THEM!
					//ntPrintf("Collapsable hierarchy, Parent: %d, Child %d\n", pParentPart->m_pDef->m_iPartIndex, pChildPart->m_pDef->m_iPartIndex );
					pChildPart->m_apParentSubParts.push_back( pParentPart );
					pParentPart->m_apChildSubParts.push_back( pChildPart );
				}
			}
		}		
	}
}


//--------------------------------------------------
//!
//!	Object_CollapsableHierarchy::DisableAllInternalCollisions()
//!	Turns off the collisions between all sub parts
//! and this main part
//!
//--------------------------------------------------
void Object_CollapsableHierarchy::DisableAllInternalCollisions()
{
	Object_CollapsableHierarchySubPart* pSubPart = 0;

	ntstd::List<Object_CollapsableHierarchySubPart*, Mem::MC_ENTITY>::iterator iter;
	ntstd::List<Object_CollapsableHierarchySubPart*, Mem::MC_ENTITY>::iterator iterEnd = m_obSubParts.end();

	// Main (this) vs. Sub Parts
	for ( iter = m_obSubParts.begin(); iter != iterEnd; iter++ )
	{
		pSubPart = (*iter);
		ntAssert( pSubPart );

		m_pobInteractionComponent->ExcludeCollisionWith( pSubPart );
	}

	// Sub Parts vs. Sub Parts
	ntstd::List<Object_CollapsableHierarchySubPart*, Mem::MC_ENTITY>::iterator iterSecond;
	for ( iter = m_obSubParts.begin(); iter != iterEnd; iter++ )
	{
		pSubPart = (*iter);

		// Start the second iterator at the next element
		iterSecond = iter;
		iterSecond++;

		for ( ; iterSecond != iterEnd; iterSecond++ )
		{
			ntAssert( pSubPart->GetInteractionComponent() );

			pSubPart->GetInteractionComponent()->ExcludeCollisionWith( (*iterSecond) );
		}
	}
}

