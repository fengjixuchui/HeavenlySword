//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file formationmanager.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------

#include "ai/aiformationcomponent.h"
#include "ai/aiformation.h"
#include "ai/aiformationattack.h"
#include "ai/aiformationmanager.h"
#include "ai/aiformationxml.h"
#include "game/aicomponent.h"
#include "game/luaexptypes.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "game/randmanager.h"
#include "game/messagehandler.h"
#include "input/inputhardware.h"
#include "core/boostarray.h"
#include "core/timer.h"
#include "core/visualdebugger.h"
#include "core/gatso.h"
#include "objectdatabase/dataobject.h"

//**************************************************************************************************
// Start exposing the elements to Lua
//**************************************************************************************************

LUA_EXPOSED_START(FormationComponent)
	
	// Expose the add formation method... 
	LUA_EXPOSED_METHOD(AddFormation, AddFormation, "", "", "")

	// Expose the delete all formations method
	LUA_EXPOSED_METHOD(DeleteAllFormations, DeleteAllFormations, "", "", "")

	// Allows script to add entities to the formation
	LUA_EXPOSED_METHOD(AddEntity, LuaAddEntity, "", "", "")

	// Assigning formations to squads bind method
	LUA_EXPOSED_METHOD( SquadAssignment, SquadAssignment, "", "", "" )

	// Assigning attacks that can be used by squads. 
	LUA_EXPOSED_METHOD( AttackAssignment, AttackAssignment, "", "", "" )
	
	// Allow Lua to enable/disable attacks on squads
	LUA_EXPOSED_METHOD( SquadAttackAllowed, SquadAttackAllowed, "", "", "" )

	// The rules to control entry into the squads 
	LUA_EXPOSED_METHOD( SetEntryRules, SetEntryRules, "", "", "" )

	// The rules to control entry into the squads 
	LUA_EXPOSED_METHOD( SetMetaTags, SetMetaTags, "", "", "" )

	// Set the region rules. 
	LUA_EXPOSED_METHOD( SetNavRegion, SetNavRegionLock, "", "", "" )
	LUA_EXPOSED_METHOD( ClearNavRegion, ClearNavRegionLock, "", "", "" )
	
LUA_EXPOSED_END(FormationComponent)



LUA_EXPOSED_START(AIFormationComponent)
	
	// Allow entities to remove themselves from formations
	LUA_EXPOSED_METHOD(Remove, Remove, "", "", "")

	// 
	LUA_EXPOSED_METHOD_GET(Active, IsActive, "")
	LUA_EXPOSED_METHOD_SET(Active, Activate, "")

LUA_EXPOSED_END(AIFormationComponent)


#define SQUAD_CACHE_SIZE	(16)

#define NUM_DEBUG_COLOURS	7

unsigned int uiaDebugColours[NUM_DEBUG_COLOURS] =
{
	DC_RED,
	DC_GREEN,
	DC_BLUE,
	DC_PURPLE,
	DC_CYAN,
	DC_YELLOW,
	DC_WHITE,
};



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	Private structure for caching squads and formation attacking
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------

struct SquadCacheEntry
{
	CKeywords			m_Id;
	ntstd::List<AI*>	m_Entities;
};

struct SquadCache
{
	SquadCache(void) : m_EntryCount(0) {}
	
	SquadCacheEntry* FindEntry( const CKeywords& obKey )
	{
		for( int iIndex = 0; iIndex < m_EntryCount; ++iIndex )
		{
			if( m_SquadEntries[iIndex].m_Id == obKey )
				return m_SquadEntries + iIndex;
		}
		return 0;
	}

	ntstd::List<AI*>& AddEntry( const CKeywords& obKey )
	{
		ntAssert( m_EntryCount < SQUAD_CACHE_SIZE-1 );
		m_SquadEntries[m_EntryCount].m_Id = obKey;
		return m_SquadEntries[m_EntryCount++].m_Entities;
	}

	int					m_EntryCount;
	SquadCacheEntry		m_SquadEntries[SQUAD_CACHE_SIZE];
};

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationSquad::FormationComponent
//! Returns the number of types this squad has
//!                                                                                         
//------------------------------------------------------------------------------------------
int FormationSquad::TypeCount( const char* pcType )
{
	int iCount = 0;
	for( ntstd::List<AI*>::iterator obIt( m_Entities.begin() ); obIt != m_Entities.end(); ++obIt )
		iCount += (*obIt)->IsType( pcType ) ? 1 : 0;
	return iCount;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationSquad::operator<
//! Used for the std::list sort operator
//!                                                                                         
//------------------------------------------------------------------------------------------
bool FormationSquad::operator>( const FormationSquad& Other ) const
{
	if( !Other.m_pAssignedFormation )
		return true;

	if( !m_pAssignedFormation )
		return false;

	return m_pAssignedFormation->GetPriority() > Other.m_pAssignedFormation->GetPriority();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::FormationComponent
//! Constructor
//!                                                                                         
//------------------------------------------------------------------------------------------
FormationComponent::FormationComponent(CEntity* pEnt)
	:	CAnonymousEntComponent( "FormationComponent" ),
		m_pParent(pEnt)
{
	ntAssert(pEnt);

	ATTACH_LUA_INTERFACE(FormationComponent);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::FormationComponent                                                       
//! Destruction                                                                             
//!                                                                                         
//------------------------------------------------------------------------------------------
FormationComponent::~FormationComponent()
{
	Free( true );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::Free
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void FormationComponent::Free( bool bInDestructor )
{
	// This entity should remove itself to the formation manager 
	if (AIFormationManager::Exists())
{
		AIFormationManager::Get().RemoveFormationEntity( m_pParent );
	}

	// Clear out all of the lists....

	// Starting with the active attacks
	while( m_ActiveAttacks.size())
	{
		m_ActiveAttacks.pop_back();
	}

	// then with the attack list. 
	while( m_AttackList.size() )
	{
		FormationAttack* pDeleteMe = m_AttackList.back();
		m_AttackList.pop_back();

		NT_DELETE_CHUNK( Mem::MC_AI, pDeleteMe->m_pAttackData );
		NT_DELETE_CHUNK( Mem::MC_AI, pDeleteMe );
	}

	// Then the squad list
	while( m_SquadList.size() )
	{
		FormationSquad* pDeleteMe = m_SquadList.back();
		m_SquadList.pop_back();

		// Remove all the entities in the squad that are assigned to a formation
		while( pDeleteMe->m_Entities.size() )
		{
			AI* pEnt = pDeleteMe->m_Entities.front();
			pDeleteMe->m_Entities.pop_front();

			// only valid to poke about with other ents if we're not being destructed, other wise these
			// ents may already be deallocated
			if (bInDestructor == false)
				RemoveEntity( pEnt );
		}

		NT_DELETE_CHUNK( Mem::MC_AI, pDeleteMe );
	}

	// Then the formation list. 
	while( m_FormationList.size() )
	{
		AIFormation* pDeleteMe = m_FormationList.back();
		m_FormationList.pop_back();

		if( !pDeleteMe->m_bIsXMLConstructed )
		{
			NT_DELETE_CHUNK( Mem::MC_AI, pDeleteMe );
		}
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::Update
//! Update function for formation, called once per frame
//!                                                                                         
//------------------------------------------------------------------------------------------
void FormationComponent::Update(float fTimeDelta)
{
	UNUSED( fTimeDelta );
	CGatso::Start( "FormationSystem::Update" );

	// 
	float fValidAttackWeight = 0.0f;
	FormationAttackList obValidAttacks;


	CGatso::Start( "FormationSystem::Update Valid attacks" );

	// Create an instance of a squad attack cache
	SquadCache obSquadCache;

	// Run through all the attacks looking for valid attacks
	for( FormationAttackList::iterator obIt( m_AttackList.begin() ); obIt != m_AttackList.end(); ++obIt )
	{ 
		// Cache the pointer to the attack.
		FormationAttack* pAttack = *obIt;

		// Check there is valid attack data. 
		if( !pAttack->m_pAttackData )
			continue;

		ntstd::List<AI*> *pobEntList = 0;

		SquadCacheEntry* pEntry = obSquadCache.FindEntry( pAttack->m_Squads );

		if( !pEntry )
		{
			pobEntList = &obSquadCache.AddEntry( pAttack->m_Squads );
			
			// 
			for( int iIndex = pAttack->m_Squads.GetKeywordCount()-1; iIndex >= 0;  --iIndex )
			{
				// Find the squad from the string
				FormationSquad* pSquad = FindSquad( pAttack->m_Squads.GetKeyword( (u_int) iIndex ) );

				// If the squad doesn't exist, then contiune to the next item
				if( !pSquad )
					continue;

				// Does the squad have a formation? If not, on to the next item
				if( !pSquad->m_pAssignedFormation )
					continue;

				// If attacks are diabled for the formation, then continue
				if( pSquad->m_AttackDisabled )
					continue;

				// Get the entities from to formation by adding them to the ent list. 
				pSquad->m_pAssignedFormation->GetEntsReadyForAttack( *pobEntList );
			}
		}
		else
		{
			pobEntList = &pEntry->m_Entities;
		}

		// Now that the ent list should be filled with loads of entities, pass those
		// entities to the attack allowing attacks to become validate. 

//		int iSizeBefore = pobEntList->size();
		if( pAttack->m_pAttackData->Validate( *pobEntList, m_ActiveAttacks ) )
		{
			fValidAttackWeight += pAttack->m_pAttackData->GetWeight();
			obValidAttacks.push_back( pAttack );
		}
//		int iSizeAfter = pobEntList->size();
//		ntAssert( iSizeAfter == iSizeBefore );
	}

	CGatso::Stop( "FormationSystem::Update Valid attacks" );

	// Run through all the valid attacks looking if the attack should be activated
//	if( !m_ActiveAttacks.size() )
	{
		float fChoosenWeight = grandf( fValidAttackWeight );
		fValidAttackWeight = 0.0f;

		for( FormationAttackList::iterator obIt( obValidAttacks.begin() ); obIt != obValidAttacks.end(); ++obIt )
		{
			// Cache the pointer to the attack.
			FormationAttack* pAttack = *obIt;

			// Sort the attacks out by weighting. 
			fValidAttackWeight += pAttack->m_pAttackData->GetWeight();
				
			// If this isn' the attack we're after, then continue on. 
			if( fValidAttackWeight < fChoosenWeight )
				continue;

			// If the attack can't run simultaneously then stop all of the other attacks.
			/*
			if( !pAttack->m_pAttackData->AllowedToRunSimultaneously() )
			{
				// 
				while( m_ActiveAttacks.size() )
				{
					AIFormationAttack* pActiveAttack = m_ActiveAttacks.front();
					m_ActiveAttacks.pop_front();
					pActiveAttack->EndCurrentAttack();
				}
			}
			*/

			// Initiate the attack
			pAttack->m_pAttackData->Initiate();
				
			// Add the attack to the active list. 
			m_ActiveAttacks.push_back( pAttack->m_pAttackData );

			// Don't start more than one attack per frame! This allows
			// all the pre-requisits to be checked.
			break;
		}
	}


	// Update all the active attacks
	for( AIFormationAttackList::iterator obIt( m_ActiveAttacks.begin() ); obIt != m_ActiveAttacks.end(); )
	{
		// Cache the pointer to the attack.
		AIFormationAttack* pAttack = *obIt;

		// Update the attack, returns true when finished
		if( pAttack->Update( fTimeDelta ) )
		{
			obIt = m_ActiveAttacks.erase( obIt );
		}
		else
		{
			++obIt;
		}
	}

	CGatso::Start( "FormationSystem::Update Update squds" );

	// Update all the squads. 

	int				iDebugColour		= 0;
	uint64_t		s_uiUpdateTicker	= 0;

	for( FormationSquadList::iterator obIt( m_SquadList.begin() ); obIt != m_SquadList.end(); ++obIt )
	{
		FormationSquad* pSquad = *obIt;

		if( pSquad->m_pAssignedFormation && pSquad->m_pAssignedFormation->CanUpdate( s_uiUpdateTicker ) )
		{
			pSquad->m_pAssignedFormation->CalculateSlotPositions();
		}
	}

	// Forward the ticker for another run
	++s_uiUpdateTicker;

	for( FormationSquadList::iterator obIt( m_SquadList.begin() ); obIt != m_SquadList.end(); ++obIt )
	{
		FormationSquad* pSquad = *obIt;

		if( pSquad->m_pAssignedFormation && pSquad->m_pAssignedFormation->CanUpdate( s_uiUpdateTicker ) )
		{
			pSquad->m_pAssignedFormation->Update(fTimeDelta, uiaDebugColours[iDebugColour++ % NUM_DEBUG_COLOURS], this);
		}
	}

	// Forward the ticker
	++s_uiUpdateTicker;

	CGatso::Stop( "FormationSystem::Update Update squds" );

#if !defined(_RELEASE)

	// Allow killing of all the entities in the formation
	if ( AIFormationManager::Get().m_bKillAllEntities ) 
	{
		// Then the squad list
		for( FormationSquadList::iterator obIt( m_SquadList.begin() ); obIt != m_SquadList.end(); ++obIt )
		{
			FormationSquad* pSquad = *obIt;

			// Remove all the entities in the squad that are assigned to a formation
			for( ntstd::List<AI*>::iterator obEntIt( pSquad->m_Entities.begin() ); obEntIt != pSquad->m_Entities.end(); ++obEntIt )
			{
				Character* pEnt = (*obEntIt)->ToCharacter();
				if (pEnt)
				{
					pEnt->ToCharacter()->ChangeHealth( -100000.0f, "Ctrl-k" );
				
				if( pEnt->GetMessageHandler() )
					pEnt->Kill();
			}
		}
	}
	}

	if( AIFormationManager::DisplayDebugInfo() )
	{
		static float fTextX;
		static float fTextY;
		static u_int uiSavedGameTick = 0;
		static int   DebugIndex = 0;

		// Reset the Display every frame
		if( CTimer::Get().GetSystemTicks() != uiSavedGameTick )
		{
			fTextX = 10.0f;
			fTextY = 20.0f;
			uiSavedGameTick = CTimer::Get().GetSystemTicks();
			DebugIndex = 0;
		}

		if( m_SquadList.size() )
		{
			float fXOffset = 450.0f;

#ifdef PLATFORM_PC
			fXOffset = g_VisualDebug->GetDebugDisplayWidth() - 300.0f;
#endif

			// Show the Squads
			int iDebugColour = 0;

			for(FormationSquadList::const_iterator obIt = m_SquadList.begin(); obIt != m_SquadList.end(); ++obIt)
			{
				// Get the squad
				FormationSquad* pSquad = *obIt;

				g_VisualDebug->Printf2D(fTextX+fXOffset, fTextY, uiaDebugColours[iDebugColour % NUM_DEBUG_COLOURS], 0, "%s - %s", pSquad->m_Name.c_str(),pSquad->m_pAssignedFormation ? pSquad->m_pAssignedFormation->GetName().c_str() : "not assigned");
				fTextY += 12.0f;

				for(ntstd::List<AI*>::const_iterator obEntIt = pSquad->m_Entities.begin(); obEntIt != pSquad->m_Entities.end(); ++obEntIt)
				{
					uint32_t uColour = uiaDebugColours[iDebugColour % NUM_DEBUG_COLOURS];
					
					// Display the squads
					g_VisualDebug->Printf2D(fTextX+fXOffset, fTextY, uColour, 0, " %c%s", (*obEntIt)->GetAIComponent()->GetAIFormationComponent()->GetFormationAttack() ? '>' : ' ', (*obEntIt)->GetName().c_str());
					fTextY += 12.0f;
				}

				++iDebugColour;
			}

			fTextY = 20.0f;
		}

		if( m_AttackList.size() )
		{
			fTextY += 12.0f;

			// Show the attacks. 
			for( FormationAttackList::iterator obIt = m_AttackList.begin(); obIt != m_AttackList.end(); ++obIt, ++DebugIndex )
			{
				// Get the attack
				FormationAttack* pAttack = *obIt;

				const char* pcState = "Invalid";
				uint32_t uColour = DC_WHITE;

				if( pAttack->m_pAttackData->IsImpossible() ) 
				{
					pcState = "Impossible";
					uColour = DC_RED;
				}
				
				if( pAttack->m_pAttackData->IsValid() ) 
				{
					pcState = "Valid";
					uColour = DC_BLUE;
				}

				if( pAttack->m_pAttackData->IsActive() ) 
				{
					pcState = "Active";
					uColour = DC_GREEN;
				}

				/*
				g_VisualDebug->Printf2D(	fTextX+5.0f, fTextY, uColour, 0, "Attack: %s (%s,%s%s)",pAttack->m_Name.c_str(), 
																							pcState, 
																							pAttack->m_pAttackData->m_pcValidFailedReason,
																							AIFormationManager::Get().GetDebugContext() == pAttack->m_pAttackData->GetDebugContext() ? ",Debug" : "" );
				fTextY += 12.0f;
				*/
			}
		}
	}

#endif

	CGatso::Stop( "FormationSystem::Update" );
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::AddFormation
//! Add a formation to the component
//!                                                                                         
//------------------------------------------------------------------------------------------
bool FormationComponent::AddFormation( const char* pName, NinjaLua::LuaObject rFormationMakeup )
{
	UNUSED( pName );
	UNUSED( rFormationMakeup );

	// Should there be a check that a formation hasn't already been registered with the same name?

	// Create the formation. 
	AIFormation* pFormation = AIFormationManager::Get().CreateFormation(rFormationMakeup);

	// If not valid, return false
	if( !pFormation )
		return false;

	// Set the owner of the formation.
	pFormation->SetOwner( this->GetParent() );

	// Set the name of the formation
	pFormation->SetName( pName );

	// Add the formation to the local list of formations. 
	m_FormationList.push_back( pFormation );

	// return that the formation was added
	return true;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::DeleteAllFormations
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void FormationComponent::DeleteAllFormations(void)
{
	Free( false );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::AddEntity
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool FormationComponent::AddEntity( AI* pEnt, bool bTestOnly )
{
	UNUSED( pEnt );
	ntAssert( pEnt->GetMessageHandler() );

#ifndef _RELEASE
	if( !bTestOnly ) { ntPrintf("Adding Entity %s: ", pEnt->GetName().c_str() ); }
#endif 


	// Don't add dead entities, that would be silly. 
	if( pEnt->ToCharacter()->IsDead() )
	{
#ifndef _RELEASE
		if( !bTestOnly ) { ntPrintf("failed (dead)\n" ); }
#endif 
		return false;
	}

	// If there assert fires - it's likely that an old formation type has been used. 
	// Handle the problem without crashing the game. 
	ntAssert( m_EntryRules.IsTable() );
	if( !m_EntryRules.IsTable() )
	{
#ifndef _RELEASE
		if( !bTestOnly ) { ntPrintf("failed(no entry rules)\n" ); }
#endif 
		ntAssert_p( false, ("No entry rules found") );
		return false;
	}

	//  Check that the entity is fine. 
	ntError( pEnt->GetAIComponent() );
	ntError( pEnt->GetAIComponent()->GetAIFormationComponent() );

	// Cache the formation component for the entity
	AIFormationComponent* pEntFormComp = pEnt->GetAIComponent()->GetAIFormationComponent();


	for( int Index = 1; m_EntryRules[Index].IsTable(); ++Index )
	{
		NinjaLua::LuaObject rEntryRule = m_EntryRules[Index];

		// This rules applies only to entities of the following type
		const char* pcType = rEntryRule.GetOpt( "type", (const char*) 0 );

		// If the entity isn't the correct type, then continue to the next
		if( pcType && !pEnt->IsType( pcType ) )
			continue;

		// Obtain the squad this rule will place the entity into
		const char* pcSquadName = rEntryRule["addTo"].GetString();

		// Check that string is valid. 
		if( !pcSquadName || (*pcSquadName == 0) )
		{
			continue;
		}

		// Find the squad, create a new squad if required. 
		FormationSquad* pSquad = NewSquad( pcSquadName );

		// Sanity Check.
		ntError(pSquad!=NULL);

		// Obtain the test table. 
		NinjaLua::LuaObject TestTable = rEntryRule["test"];
		if( !TestTable.IsNil() )
		{
			// The test table contains two entries, a minimum and maximum value.
			// The number of entities of pcType type must lay within and include these 
			// two values, 

			// Obtain the high and low numbers. 
			ntError( TestTable[1].IsNumber() && TestTable[2].IsNumber() );

			int TestLow		= TestTable[1].GetInteger();
			int TestHigh	= TestTable[2].GetInteger();

			// Get the number entity types in the squad already
			int TestCount = pSquad->TypeCount( pcType );

			// If the number of TestCount isn't between TestLow and TestHigh, then move onto the next entity
			if( !(TestLow <= TestCount && TestHigh > TestCount) )
			{
				continue;
			}
		}

		bool bInNewFormation = false;

		// If the entity has a commander assigned, it is therefore assigned to a squad, remove that entity from the squad
		if( pEntFormComp->GetCommander() )
		{
			// If we're only testing for formation entry, then exit now as we're already in a formation
			if( bTestOnly )
				return false;

			// TODO:GAV
			if( pEntFormComp->GetCommander() != this )
			{
				// Entity is being moved to another commander
				ntError( false );
				continue;
			}
			else if( pSquad->m_Name != pEntFormComp->GetSquadName() )
			{
				FormationSquad* pAssignedSquad = FindSquad( pEntFormComp->GetSquadName() );

				if( pAssignedSquad )
				{
					pAssignedSquad->m_Entities.remove( pEnt );

					if( pEntFormComp->GetFormation() )
					{
						if( pEntFormComp->GetFormation()->RemoveEntity( pEnt ) )
						{
							bInNewFormation = true;
						}
						else
						{
							ntError( false );
						}
					}
				}
				else if( pEntFormComp->GetFormation() )
				{
					if( pEntFormComp->GetFormation()->RemoveEntity( pEnt ) )
					{
						bInNewFormation = true;
					}
					else
					{
						ntError( false );
					}
				}

				// Clear out the squad name for entity
				pEntFormComp->NullSquadName();

			}
			else
			{
				// Make sure that the formations are the same. else the entity will need reassigning
				//ntAssert( pEnt->GetAIComponent()->GetAIFormationComponent()->GetFormation() == pSquad->m_pAssignedFormation );
				if( pEntFormComp->GetFormation() != pSquad->m_pAssignedFormation )
				{
					// Save the formation for later, as it might be required
					AIFormation* pFormation = pEntFormComp->GetFormation();

					// Remove the entity from it's current formation. 
					if( pEntFormComp->GetFormation() && pEntFormComp->GetFormation()->RemoveEntity( pEnt ) )
					{
						bInNewFormation = true;
					}
					else
					{
						// If the entity wasn't removed from a formation - then check that the formation is 
						// is valid before asserting the point.
						ntError( pEntFormComp->GetFormation() == NULL );
					}

					// It looks like the entity was once part of this squad, make sure
					// it was removed when the squad was was reassigned to a new formation.
					pSquad->m_Entities.remove( pEnt );

					// Assign the entity to the new formation
					if (pSquad->m_pAssignedFormation->AssignSlot(pEnt, false))
					{
						// Add the entity to the squad 
						pSquad->m_Entities.push_back( pEnt );
					}
					else
					{
						pEnt->GetAIComponent()->GetAIFormationComponent()->SetFormation( pFormation );

						// Failed to add the entity - therefore completly remove the entity
						// from the formation and hope that the lua controller handles the state
						// change. 
						if( !bTestOnly && !RemoveEntity( pEnt ) )
						{
							// Holly poop batman, what happened here?
							ntAssert( false );

							// 
							Message msg( msg_formation_exited );

							// Do our best to resolve the problem, 
							pEnt->GetMessageHandler()->Receive( msg );
						}
#ifndef _RELEASE
						if( !bTestOnly ) { ntPrintf("failed(no slots)\n" ); }
#endif 
						//ntError( false );
						return false;
					}
				}

#ifndef _RELEASE
				if( !bTestOnly ) { ntPrintf("success 1\n" ); }
#endif 

				return true;
			}
		}

		// Does the squad already have an assigned formation, if so, then add the entity to the formation
		if( pSquad->m_pAssignedFormation )
		{
			if (!pSquad->m_pAssignedFormation->AssignSlot(pEnt, bTestOnly))
			{
				// TODO:GAV this should never really happen.
				//ntError( false );
				continue;
			}
		}

		// If we're only testing for formation entry, then exit now as we're already in a formation
		if( bTestOnly )
			return true;

		// Keep a local copy
		m_Entities.push_back( pEnt );

		// Add the entity to the squad 
		pSquad->m_Entities.push_back( pEnt );

		// Set the commander
		pEntFormComp->SetCommander( this );

		// Set the region lock for the entity
		pEntFormComp->SetNavRegionLock( ntStr::GetString(m_obNavRegionLock) );

		// Set the squad
		pEntFormComp->SetSquadName( pSquad->m_Name );

		// Send the entity a message saying that entering was successful
		pEnt->GetMessageHandler()->Receive( CMessageHandler::Make( m_pParent, bInNewFormation ? "msg_formation_change_successful" : "msg_formation_enter_successful" ) );

#ifndef _RELEASE
		if( !bTestOnly ) { ntPrintf("success 2\n" ); }
#endif 

		return true;
	}

	// Send the entity a message saying that entering a formation failed. 
	pEnt->GetMessageHandler()->Receive( CMessageHandler::Make( m_pParent, "msg_formation_enter_failed" ) );

#ifndef _RELEASE
	if( !bTestOnly ) { ntPrintf("failed (final)\n" ); }
#endif 

	// return false, (failed!)
	return false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::RemoveEntity
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool FormationComponent::RemoveEntity( AI* pEnt )
{
#ifndef _RELEASE
	ntPrintf("Removing entity %s: ", pEnt ? pEnt->GetName().c_str() : "NULL" ); 
#endif 

	if( !pEnt
		|| !pEnt->GetAIComponent()
		|| !pEnt->GetAIComponent()->GetAIFormationComponent()
		|| !pEnt->GetAIComponent()->GetAIFormationComponent()->GetFormation() )
	{
#ifndef _RELEASE
		ntPrintf("success (already removed)\n" );
#endif 
		return true;
	}

	const ntstd::String& obSquadName = pEnt->GetAIComponent()->GetAIFormationComponent()->GetSquadName();
	FormationSquad* pSquad = FindSquad( obSquadName );

	// Remove the entity from the local list. 
	m_Entities.remove( pEnt );

	AIFormationAttack* pAttack = pEnt->GetAIComponent()->GetAIFormationComponent()->GetFormationAttack();
	if( pAttack )
	{
		pAttack->RemoveEntity( pEnt );
	}

	// Get the formation the entity is in
	AIFormation* pFormation = pEnt->GetAIComponent()->GetAIFormationComponent()->GetFormation();

	// Remove the entity
	if( pFormation->RemoveEntity( pEnt ) )
	{

		// Send the entity a message saying that it was removed form formation
		if( pEnt->GetMessageHandler() )
		{
			pEnt->GetMessageHandler()->Receive( CMessageHandler::Make( m_pParent, "msg_formation_exited" ) );
		}

		// Send the commander a message that a entity has been removed
		if( m_pParent->GetMessageHandler() )
		{
			m_pParent->GetMessageHandler()->Receive( CMessageHandler::Make( m_pParent, "msg_formation_entity_removed", pEnt ) );
		}

		// Remove the entity from the assigned squad 
		// [Nick Ind: I have moved these lines to the top of the function as the  squad name has been nulled by the RemoveEntity() call above.
//		const ntstd::String& obSquadName = pEnt->GetAIComponent()->GetAIFormationComponent()->GetSquadName();
//		FormationSquad* pSquad = FindSquad( obSquadName );
		if( pSquad )
		{
			pSquad->m_Entities.remove( pEnt );

			if( m_pParent && m_pParent->GetMessageHandler() )
			{
			     if( pSquad->m_onDeath.size() && pEnt->ToCharacter()->IsDead() )
				  m_pParent->GetMessageHandler()->Receive( CMessageHandler::Make( m_pParent, pSquad->m_onDeath.c_str(), pEnt ) );

			     if( !pSquad->m_Entities.size() )
				  m_pParent->GetMessageHandler()->Receive( CMessageHandler::Make( m_pParent, "msg_formation_squad_empty" ) );
			}
		}
		else
		{
			ntPrintf("Could not find squad %s from which to remove %s\n", ntStr::GetString(obSquadName), ntStr::GetString(pEnt->GetName()));
		}
	
		// Null Out the squad name. 
		pEnt->GetAIComponent()->GetAIFormationComponent()->NullSquadName();

		// obtain the number of entities still in formation
		int ActiveEntCount = 0;
		for( AIFormationList::iterator obIt( m_FormationList.begin() ); obIt != m_FormationList.end(); ++obIt )
		{
			AIFormation* pFormation = *obIt;
			ActiveEntCount += pFormation->EntityCount();
		}

		if( !ActiveEntCount )
		{
			if( m_pParent->GetMessageHandler() )
			{
				m_pParent->GetMessageHandler()->Receive( CMessageHandler::Make( m_pParent, "msg_formation_no_more_entities" ) );
/*
#ifndef _RELEASE
				if( AIFormationManager::Get().m_MemoryTracker )
				{
					Mem::DumpMemoryCheckpointDifference( AIFormationManager::Get().m_MemoryTracker );
				}

				AIFormationManager::Get().m_MemoryTracker = Mem::TakeMemoryCheckpoint();
#endif
*/
			}
		}

		// 
		pEnt->GetAIComponent()->GetAIFormationComponent()->SetCommander( 0 );
		pEnt->GetAIComponent()->GetAIFormationComponent()->SetFormation( 0 );


#ifndef _RELEASE
		ntPrintf("success (removed)\n" );
#endif 
		return true;
	}

#ifndef _RELEASE
	ntPrintf("failed (removed)\n" );
#endif 

	ntError( false );

	// return false, failed!
	return false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::SquadAssignment
//! 
//! Formations assigned to squads
//!                                                                                         
//------------------------------------------------------------------------------------------
void FormationComponent::SquadAssignment( NinjaLua::LuaObject SquadList )
{
	// All the entities assigned should be removed and reassigned
	ntstd::List<AI*> ListCopy;

	// 
	for( FormationSquadList::iterator obIt( m_SquadList.begin() ); obIt != m_SquadList.end(); ++obIt )
	{
		FormationSquad* pSquad = *obIt;

		// If there is an assigned formation...
		if( pSquad->m_pAssignedFormation )
		{
			// Mark the formation as non-active
			pSquad->m_pAssignedFormation->SetActive(false, this);
		}

		ListCopy.merge( pSquad->m_Entities );
		pSquad->m_Entities.clear();

	}

	// 
	for( int Index = 1; SquadList[Index].IsTable(); ++Index )
	{
		const NinjaLua::LuaObject obEntry = SquadList[Index];

		// Obtain the squad name from the lua table entry labled 'squad'
		const char* pcSquad = obEntry.GetOpt( "squad", (const char*) 0 );

		if( !pcSquad || *pcSquad == 0 )
		{
			user_warn_p( false, ("No squad name given in squad assignement" ) );
			continue;
		}

		// Find the squad by that name, if it doesn't exist then create it.
		FormationSquad* pSquad = NewSquad( pcSquad );

		// Get the name of the formation.
		const char* pcFormation = obEntry.GetOpt( "addTo", (const char*) 0 );

		// Doesn't exist? contiune...
		if( !pcFormation || *pcFormation == 0 )
		{
			user_warn_p( false, ("No formation name given in squad assignement: %s\n", pcSquad ) );
			continue;
		}

		// Get the formation
		AIFormation* pFormation = GetFormation( pcFormation );

		// Bad formation?
		if( !pFormation )
		{
			user_warn_p( false, ("No Formation registered with the name: %s\n", pcFormation ) );
			continue;
		}

		// Assign the formation
		pSquad->m_pAssignedFormation = pFormation;

		// Get and set the onDeath message handler name
		pSquad->m_onDeath = obEntry.GetOpt( "onDeath", ntstd::String() );

		// Get and set the onKO message handler name
		pSquad->m_onKO = obEntry.GetOpt( "onKO", ntstd::String() );

		// Mark the formation as active
		pFormation->SetActive(true, this);
	}

	// Only keep squads that there are entry rules for.. 	
	FormationSquadList NewSquadList;
	for( int Index = 1; m_EntryRules[Index].IsTable(); ++Index )
	{
		const char* pcSquadName = m_EntryRules[Index]["addTo"].GetString();
		FormationSquad* pSquad = FindSquad( ntstd::String(pcSquadName) );

		if( pSquad )
		{
			NewSquadList.push_back( pSquad );
			m_SquadList.remove( pSquad );
		}
	}

	while( m_SquadList.size())
	{
		FormationSquad* pSquad = m_SquadList.front();
		m_SquadList.pop_front();
		NT_DELETE_CHUNK( Mem::MC_AI, pSquad );
	}

	m_SquadList = NewSquadList;

	// Sort the squads
	SortSquads();

	// Assign all the entities to the new squads. 
	while( ListCopy.size() )
	{
		// 
		if( !AddEntity( ListCopy.front() ) )
		{
			ntAssert_p( ListCopy.front()->ToCharacter()->IsDead(), ("Failed to add entity %s to any squad\n", ntStr::GetString(ListCopy.front()->GetName() ))  );
		}

		ListCopy.pop_front();
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::AttackAssignment
//! 
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void FormationComponent::AttackAssignment( const char* pcAtttackName )
{
	DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromName( pcAtttackName );
	one_time_assert_p( 0xBAD101, pDataObject != NULL, ("No formation attack pattern with name: %s\n", pcAtttackName) );
	if( !pDataObject ) 
		return;

	// Check the type before casting the pointer. 
	ntAssert( strcmp( pDataObject->GetClassName(), "CGromboAttackList") == 0 );
	CGromboAttackList* pAttackList = (CGromboAttackList*) pDataObject->GetBasePtr();

	// Any active attacks, remove them
	while( m_ActiveAttacks.size())
	{
		m_ActiveAttacks.pop_back();
	}

	// Remove all the current attacks assigned to the formation.
	while( m_AttackList.size() )
	{
		FormationAttack* pDeleteMe = m_AttackList.back();
		m_AttackList.pop_back();

		NT_DELETE_CHUNK( Mem::MC_AI, pDeleteMe->m_pAttackData );
		NT_DELETE_CHUNK( Mem::MC_AI, pDeleteMe );
	}

	// 
	for( CGromboAttackList::Iterator obIt( pAttackList->m_obAttacks.begin() ); obIt != pAttackList->m_obAttacks.end(); ++obIt )
	{
		//const NinjaLua::LuaObject obEntry = AttackTable[Index];
		CGromboAttackPattern* pAttackPattern = *obIt;

		// Get the name of the attack pattern
		const ntstd::String& obName = ntStr::GetString(ObjectDatabase::Get().GetDataObjectFromPointer(pAttackPattern)->GetName());

		// Create a new attack
		FormationAttack* pFormationAttack = NewAttack( obName );

		// Sanity Check
		ntAssert( pFormationAttack->m_pAttackData == NULL );

		// Add the attack pattern data
		pFormationAttack->m_pAttackData = AddAttackPattern( pAttackPattern, obName );

		// Get the start and end delays
		pFormationAttack->m_pAttackData->SetEndDelay( pAttackPattern->m_fEndDelay );
		pFormationAttack->m_pAttackData->SetStartDelay( pAttackPattern->m_fStartDelay );
		pFormationAttack->m_pAttackData->SetOnComplete( pAttackPattern->m_obOnComplete );

		// Assign the squads
		pFormationAttack->m_Squads = CKeywords(pAttackPattern->m_obSquads.c_str());
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::SquadAttackAllowed
//! 
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void FormationComponent::SquadAttackAllowed( const char* pcName, bool bValue )
{
	// Find the squad...
	FormationSquad* pSquad = FindSquad( ntstd::String(pcName) );
	
	// If not valid, then return now. 
	if( !pSquad ) 
		return;

	// 
	pSquad->m_AttackDisabled = !bValue;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::CombatStateChanged
//! 
//! called when an entities combat state changes. 
//!                                                                                         
//------------------------------------------------------------------------------------------
void FormationComponent::CombatStateChanged( AI* pEnt, COMBAT_STATE eCombatState )
{
	// If the entity has entered a KO, then there might be something interesting to perform
	if( eCombatState == CS_KO )
	{
		// Get the name of the squad the entity is in. 
		const ntstd::String& obSquadName = pEnt->GetAIComponent()->GetAIFormationComponent()->GetSquadName();

		// If the squad is valid. 
		if( obSquadName.length() > 0 )
		{
			// Find the squad
			FormationSquad* pSquad = FindSquad( obSquadName );

			// If the squad is valid, and there is a useful message for the code to send
			if( pSquad && pSquad->m_onKO.length() )
			{
				m_pParent->GetMessageHandler()->Receive( CMessageHandler::Make( m_pParent, pSquad->m_onKO.c_str(), pEnt ) );
			}
	}

		// There could be an interesting message to send from the attack as well, assuming the entity is in an attack
		AIFormationAttack* pAttack = pEnt->GetAIComponent()->GetAIFormationComponent()->GetFormationAttack();

		if( pAttack )
		{
			FormationAttack* pFoundAttack = FindAttack( pAttack->GetName() );

			if( pFoundAttack && pFoundAttack->m_onKO.length() )
			{
				m_pParent->GetMessageHandler()->Receive( CMessageHandler::Make( m_pParent, pFoundAttack->m_onKO.c_str(), pEnt ) );
			}
		}
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::ActivateFormations
//! 
//! For a given formation define table, activate the required formations. 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool FormationComponent::ActivateFormations( NinjaLua::LuaObject Def )
{
	// Make sure lua object is valid
	if( Def.IsNil() )
		return false;

	m_Def = Def;

	// Mark all the formations as unactive. once all the formations that are active are
	// activated, then formations that are still unactive but still have entities will have 
	// their entities moved to other formations.
	for( AIFormationList::iterator obIt( m_FormationList.begin() ); obIt != m_FormationList.end(); ++obIt )
	{
		AIFormation* pFormation = *obIt;
		pFormation->SetActive(false, this);
	}

	// Run through each activation define. 
	for( int Index = 1; !Def[Index].IsNil(); ++Index )
	{
		NinjaLua::LuaObject DataEntry = Def[Index];

		// The first entry is always the name of the formation,
		const char* pcName = DataEntry["Name"].GetString();

		// Find the formation
		AIFormation* pFormation = GetFormation( pcName );

		// If the formation couldn't be found, then continue on to the next one
		if( !pFormation )
			continue;

		// Mark the formation as active
		pFormation->SetActive(true, this);
	}

	// 
	return false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	TestGrombo
//! Test code to find problems in group combat. 
//!                                                                                         
//------------------------------------------------------------------------------------------

#if !defined(_RELEASE)

static bool TestGrombo( const char* pcName, const NinjaLua::LuaObject& obTop, const NinjaLua::LuaObject& obTest )
{
	for( NinjaLua::LuaIterator obIt( obTest ); obIt; ++obIt )
	{
		if( obIt.GetKey().IsString() 
			&& (strcmp( obIt.GetKey().GetString(), "next" ) == 0
				|| strcmp( obIt.GetKey().GetString(), "StartState" ) == 0
				|| strcmp( obIt.GetKey().GetString(), "next_if_koed" ) == 0
				|| strcmp( obIt.GetKey().GetString(), "next_if_recoiling" ) == 0
				|| strcmp( obIt.GetKey().GetString(), "next_if_blocked" ) == 0) )
		{
			if( !obIt.GetValue().IsString() || obTop[ obIt.GetValue().GetString() ].IsNil() )
			{
				user_error_p( false, ("Failed to find next state '%s' in grombo %s \n", obIt.GetValue().GetString(), pcName ) );
				return false;
			}
		}

		if( obIt.GetValue().IsTable() )
		{
			if( !TestGrombo( pcName, obTop, obIt.GetValue() ) )
				return false;
		}
	}

	return true;
}

#endif



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::AddAttackPattern                                                           
//! Add an attack pattern for this formation to make use of.                                
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormationAttack* FormationComponent::AddAttackPattern( const CGromboAttackPattern* pAttackPattern, const ntstd::String& rName)
{
	const CGromboInstance* pGromboInstance = pAttackPattern->m_pGromboInstance;

	// Create a new formation attack.
	AIFormationAttack* pFormationAttack = NT_NEW_CHUNK( Mem::MC_AI ) AIFormationAttack( this, pGromboInstance, rName );

	// Assign the prev combat table a nil value
//	NinjaLua::LuaObject obAICombatPrevTable;//( *def.GetState() );
//	obAICombatPrevTable["anim"];
//	obAICombatPrevTable.SetNil();

	ntstd::List<CGromboEntity*, Mem::MC_AI>::const_iterator obIt( pGromboInstance->m_obEntities.begin() );
	ntstd::List<CGromboEntity*, Mem::MC_AI>::const_iterator obEndIt( pGromboInstance->m_obEntities.end() );

	// Add the participant queries to the attack.
	for( ; obIt != obEndIt; ++obIt )
	{
		CGromboEntity* pAttackEntity = *obIt;

		CAIFormationAttackString* pobQuery = NT_NEW_CHUNK( Mem::MC_AI ) CAIFormationAttackString;

		pobQuery->m_obType					= CKeywords(pAttackEntity->m_obType.c_str());
		pobQuery->m_AngleBits				= (u_int) pAttackEntity->m_iAngle;
		pobQuery->m_CameraBits				= (u_int) pAttackEntity->m_iCamera;
		pobQuery->m_RelativeBits			= (u_int) pAttackEntity->m_iRelative;
		pobQuery->m_TimeNotInGrombo			= pAttackEntity->m_fTimeNotInGrombo;
		pobQuery->m_FormationPositionOffset	= pAttackEntity->m_fFormationPositionOffset;
		pobQuery->m_Distance				= pAttackEntity->m_fDistanceToTarget; 

		// Keep a pointer to the Attack Patter
		pobQuery->m_pPattern                = pAttackPattern;
		pobQuery->m_pEntity                 = pAttackEntity;

		// Add the query
		pFormationAttack->AddQuery(pobQuery);
	}

	// 
	pFormationAttack->SetDebugContext( m_AttackList.size() );

	// return the create formation
	return pFormationAttack;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::GetFormation
//! 
//! For a given name, find the formation with the same name
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormation* FormationComponent::GetFormation( const char* pcName )
{
	// Run through all the formations in the component, try adding the entity to each of them
	for( AIFormationList::iterator obIt( m_FormationList.begin() ); obIt != m_FormationList.end(); ++obIt )
	{
		// Get the formation
		AIFormation* pFormation = *obIt;

		if( pFormation->GetName() == pcName )
			return pFormation;
	}

	DataObject* pObject = ObjectDatabase::Get().GetDataObjectFromName( pcName );

	if( !pObject )
		return NULL;

	if( !strstr( pObject->GetClassName(), "AIFormation") )
		return NULL;

	// 
	AIFormation* pFormation = (AIFormation*) pObject->GetBasePtr();

	// Make sure the xml data is constructed. 
	pFormation->XmlConstruct();

	// Add the formation to the active formation list.
	m_FormationList.push_back( pFormation );

	// Found it. 
	return pFormation;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::ClearAttackPatterns                                                        
//! Empty out the attack patterns list.                                                     
//!                                                                                         
//------------------------------------------------------------------------------------------
void FormationComponent::ClearAttackPatterns()
{
	// If there is a current active attack, finish it. 
	//EndAttack();

	// If there are any active attacks, end them
	while(!m_ActiveAttacks.empty())
	{
		AIFormationAttack* pFormationAttack = m_ActiveAttacks.back();
		m_ActiveAttacks.pop_back();

		// End the current attack
		pFormationAttack->EndCurrentAttack();
	}

	// Remove the attacks from the formation

	while(!m_AttackList.empty())
	{
		FormationAttack* pFormationAttack = m_AttackList.back();
		m_AttackList.pop_back();

		NT_DELETE_CHUNK( Mem::MC_AI, pFormationAttack->m_pAttackData );
		NT_DELETE_CHUNK( Mem::MC_AI, pFormationAttack );
	}

	/*
	// Remove all the entities listed with attack ready status
	m_entsReadyForAttack.clear();

	// Clear the current attack
	ClearAttack();
*/
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::FindSquad
//! Find a squad
//!                                                                                         
//------------------------------------------------------------------------------------------

FormationSquad* FormationComponent::FindSquad( const ntstd::String& obName )
{
	for( FormationSquadList::iterator obIt( m_SquadList.begin() ); obIt != m_SquadList.end(); ++obIt )
	{
		if( (*obIt)->m_Name == obName )
			return (*obIt);
	}

	return 0;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::FindSquad
//! Find a squad
//!                                                                                         
//------------------------------------------------------------------------------------------

FormationSquad* FormationComponent::FindSquad( u_int uiHash )
{
	for( FormationSquadList::iterator obIt( m_SquadList.begin() ); obIt != m_SquadList.end(); ++obIt )
	{
		FormationSquad* pSquad = *obIt;
		CHashedString obHash((*obIt)->m_Name.c_str());

		if( uiHash == obHash.GetValue() )
			return pSquad;
	}

	return 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::SortSquads
//! Sorts the squads
//!                                                                                         
//------------------------------------------------------------------------------------------
template<typename T> struct SortHelper { bool operator()(T const* p1, T const* p2) { return *p1 > *p2; } };

template<> struct SortHelper<FormationSquad*> 
{
	bool operator()(FormationSquad const* p1, FormationSquad const* p2)
	{
		if(!p1) return true;
		if(!p2) return false;
		return *p1 > *p2;
	}
};


//------------------------------------------------------------------------------------------
//!  private  SortSquads
//!
//!
//!  @author GavB @date 07/12/2006
//------------------------------------------------------------------------------------------
void FormationComponent::SortSquads( void )
{
	m_SquadList.sort( SortHelper<FormationSquad*>() );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::NewSquad
//! Create a new squad
//!                                                                                         
//------------------------------------------------------------------------------------------

FormationSquad* FormationComponent::NewSquad( const char* pcName )
{
	FormationSquad* pSquad = FindSquad( ntstd::String(pcName) );

	if( pSquad )
		return pSquad;

	// Alloc the memory for the new squad. 
	pSquad = NT_NEW_CHUNK( Mem::MC_AI ) FormationSquad;

	// Assign the squad name
	pSquad->m_Name = pcName;

	// No formation assigned yet
	pSquad->m_pAssignedFormation = 0;

	// Attacks aren't currently diabled
	pSquad->m_AttackDisabled = false;

	// Add a squad to the list. 
	m_SquadList.push_back( pSquad );

	return pSquad;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::FindAttack
//! Find an attack
//!                                                                                         
//------------------------------------------------------------------------------------------

FormationAttack* FormationComponent::FindAttack( const ntstd::String& obName )
{
	for( FormationAttackList::iterator obIt( m_AttackList.begin() ); obIt != m_AttackList.end(); ++obIt )
	{
		if( (*obIt)->m_Name == obName )
			return (*obIt);
	}

	return 0;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::IsClearOfOtherSlots
//! Is the space of the given point in the context of the formation valid?
//!                                                                                         
//------------------------------------------------------------------------------------------
void FormationComponent::UpdateClearOfOtherSlots(AIFormation* pFormation, float Range)
{
	// Pre-check to see if all the formations are fixed in relation to each other.
	for( FormationSquadList::iterator obIt(m_SquadList.begin() ); obIt != m_SquadList.end(); ++obIt )
	{
		FormationSquad* pSquad = *obIt;

		if (!pSquad->m_pAssignedFormation || 
			!pFormation->m_bOverlapChecked ||
			pSquad->m_pAssignedFormation->m_bCameraRelative != pFormation->m_bCameraRelative ||
			!(pSquad->m_pAssignedFormation->GetTarget().X() == pFormation->GetTarget().X() && pSquad->m_pAssignedFormation->GetTarget().Y() == pFormation->GetTarget().Y() && pSquad->m_pAssignedFormation->GetTarget().Z() == pFormation->GetTarget().Z()))
		{
			pFormation->m_bOverlapChecked = false;
			break;
		}
		}

	if (!pFormation->m_bOverlapChecked)
	{
		for (int iSlot = 0; iSlot < pFormation->m_iSlotsInFormation; iSlot++)
		{
			AIFormationSlot* pSlot = pFormation->m_pSlots[iSlot];
			CPoint SlotPoint = pSlot->GetWorldPoint();

			bool bClearOfOtherSlots = true;

			// Process all the squads 
			for (FormationSquadList::iterator obIt(m_SquadList.begin()); obIt != m_SquadList.end(); ++obIt )
			{
				FormationSquad* pSquad = *obIt;

				// Check for a valid formation as well as the squad formation having a higher priority than the one to be tested. 
				if (pFormation->GetPriority() < pSquad->m_pAssignedFormation->GetPriority())
				{
					for (int iOtherSlot = 0; iOtherSlot < pSquad->m_pAssignedFormation->m_iSlotsInFormation; iOtherSlot++)
					{
						AIFormationSlot* pOtherSlot = pSquad->m_pAssignedFormation->m_pSlots[iOtherSlot];

						if ((pOtherSlot->GetWorldPoint() - SlotPoint).LengthSquared() < (Range * Range))
						{
							bClearOfOtherSlots = false;
							break;
						}
					}
					}

				if (!bClearOfOtherSlots)
					break;
				}

			pSlot->SetClearOfOtherSlots(bClearOfOtherSlots);
			}
		}
	}
	
//------------------------------------------------------------------------------------------
//!                                                                                         
//!	FormationComponent::NewAttack
//! Create a new attack, name must be unique
//!                                                                                         
//------------------------------------------------------------------------------------------

FormationAttack* FormationComponent::NewAttack( const ntstd::String& obName )
{
	FormationAttack* pAttack = FindAttack( obName );

	if( pAttack )
	{
		if( pAttack->m_pAttackData )
		{
			NT_DELETE_CHUNK( Mem::MC_AI, pAttack->m_pAttackData );
			pAttack->m_pAttackData = 0;
		}

		return pAttack;
	}

	// Alloc the memory for the new squad. 
	pAttack = NT_NEW_CHUNK( Mem::MC_AI ) FormationAttack;

	// Assign the name
	pAttack->m_Name = obName;

	// Clear the attack data
	pAttack->m_pAttackData = 0;

	// Add to the list. 
	m_AttackList.push_back( pAttack );

	// return the new attack
	return pAttack;
}

//------------------------------------------------------------------------------------------
//!  public  ResetOverlapCheckedStatus
//!
//!
//!  @author GavB @date 07/12/2006
//------------------------------------------------------------------------------------------
void FormationComponent::ResetOverlapCheckedStatus()
{
	// Reset all the formations so that checks for overlapping slots are done again. This is because
	// a formation added or removed will make the existing slot's overlap status stale.
	for( FormationSquadList::iterator obIt( m_SquadList.begin() ); obIt != m_SquadList.end(); ++obIt )
	{
		FormationSquad* pSquad = *obIt;

		if( pSquad->m_pAssignedFormation)
		{
			pSquad->m_pAssignedFormation->m_bOverlapChecked = false;
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationComponent::AIFormationComponent
//! Constructor
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormationComponent::AIFormationComponent(AI* pEnt)
	:	m_pParent(pEnt),
		m_uiBehaviourRefCount(0),
		m_pCommander(0),
		m_pFormation(0),
		m_pAttack(0),
		m_fFormationMovementPaused(0.0f),
		m_FreeAttackCount(0),
		m_bNoFormationUpdate(false),
		m_TimeNotInGrombo(0.0f),
		m_Active(false)
{
	ntAssert(pEnt);

	ATTACH_LUA_INTERFACE(AIFormationComponent);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationComponent::AIFormationComponent                                                       
//! Destruction                                                                             
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormationComponent::~AIFormationComponent()
{
	Remove();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationComponent::Update
//! Update function for ai formation, called once per frame
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormationComponent::Update(float fTimeDelta)
{
	UNUSED( fTimeDelta );
	CGatso::Start( "FormationEntity::Update" );

	if( m_fFormationMovementPaused )
		m_fFormationMovementPaused -= fTimeDelta;
	
	// If the entity is dead, then remove it from the formation.. 
	if( m_pParent->ToCharacter()->IsDead() )
	{
		Remove();
	}

	// If not in an attack, then increase the timer recorder
	if( !m_pAttack )
		m_TimeNotInGrombo += fTimeDelta;
	else
		m_TimeNotInGrombo = 0.0f;

	CGatso::Stop( "FormationEntity::Update" );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationComponent::SetNoFormationUpdate
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormationComponent::SetNoFormationUpdate(bool bState) 
{ 
	// Sanity Check
	//ntError_p( m_pFormation, "Please grab gav if you get this" );

	// Get the no formation update state
	m_bNoFormationUpdate = bState; 

	// If the state is disabled and the formation is valid, find the slot the entity is assigned to
	// and mark it as out of position. 
	if( !bState && m_pFormation )
	{
		const AIFormationSlot* pSlot = ((const AIFormation*)m_pFormation)->FindSlot(*m_pParent); 
	
		if( pSlot )
		{
			pSlot->SetInPosition( false );
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationComponent::Remove
//! Function that allows the entity to remove itself from the assigned formation
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormationComponent::Remove(void)
{
	if( m_pCommander )
	{
		m_pCommander->RemoveEntity( GetParent() );
	}
	else
	{
		ntError( m_pFormation == NULL );
		ntError( m_pAttack == NULL );
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationComponent::CanAttack
//! Can the entity perform one on one style attacks?
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AIFormationComponent::CanAttack(void)
{
	if( m_FreeAttackCount )
		return m_FreeAttackCount-- != 0;

	if( m_pAttack )
		return m_pAttack->CanEntityAttack( GetParent() );

	if( m_pFormation )
		return false;

	return true;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationComponent::CanBlock
//! Can the entity perform one on one style attacks?
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AIFormationComponent::CanBlock(void)
{
	if( m_pAttack )
		return m_pAttack->CanEntityBlock( GetParent() );

	return true;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationComponent::CombatStateChanged
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------

void AIFormationComponent::CombatStateChanged( AI* pEnt, COMBAT_STATE eCombatState )
{
	if( GetCommander() )
		GetCommander()->CombatStateChanged( pEnt, eCombatState );
}


