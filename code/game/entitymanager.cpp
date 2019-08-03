/***************************************************************************************************
*
*	DESCRIPTION		Core Entity System Implementation
*
*	NOTES
*
***************************************************************************************************/

#include "Physics/config.h"
#include "Physics/system.h"
#include "Physics/advancedcharactercontroller.h"

#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/entityarcher.h"
#include "game/attacks.h"
#include "game/staticentity.h"
#include "game/luaglobal.h"
#include "game/aicomponent.h"
#include "game/inputcomponent.h"
#include "game/messagehandler.h"
#include "game/movement.h"
#include "game/awareness.h"
#include "game/anonymouscomponent.h"
#include "game/query.h"
#include "game/luaexptypes.h"
#include "game/fsm.h"
#include "game/aimcontroller.h"


#include "ai/aiformationcomponent.h"

#include "anim/animator.h"
#include "anim/hierarchy.h"
#include "audio/gameaudiocomponents.h"
#include "camera/sceneelementcomponent.h"
#include "core/gatso.h"
#include "core/timer.h"
#include "gfx/clump.h"
#include "gfx/levelofdetail.h"
#include "input/inputhardware.h"
#include "lua/ninjalua.h"
#include "objectdatabase/dataobject.h"
#include "area/areasystem.h"
#include "exec/exec.h"

#include "hair/chaincore.h"
#include "gui/guimanager.h"

#include "blendshapes/BlendShapesComponent.h"
#include "physics/LookAtComponent.h"

#include "entityboss.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkvisualize/hkDebugDisplay.h>
#endif

#ifdef PLATFORM_PS3
#include "army/armymanager.h"
#endif

#ifdef _PROFILING
#define PER_ENTITY_GATSO 1
#endif

// Statics
const CEntityManager::EntityBucketInfo CEntityManager::m_aobEntityBucketInfos[CEntity::m_iNumEntTypes] = 
{
	{"Player"},
	{"AI"},
	{"Interactable"},
	{"Static"},
	{"Unknown"},
	{"Boss"},
	{"Collision"},
	{"Projectile"},
};

int32_t CEntityManager::m_iCurrentQueries = 0;
int32_t CEntityManager::m_iMaxQueries = 0;


/***************************************************************************************************
* Start exposing the element to Lua
***************************************************************************************************/
LUA_EXPOSED_START(CEntityManager)
	LUA_EXPOSED_METHOD(GetPlayer,		GetPlayer, "Get the player entity", "", "") 
LUA_EXPOSED_END(CEntityManager)

// reset the force field value of all the entity
void CEntityManager::ResetForceField( void )
{
#ifdef PLATFORM_PC // FIXME_WIL forcefield on ps3?
	// Lets go through all our entities
	for ( QuickPtrList<CEntity>::iterator obIt = m_entities.begin(); obIt != m_entities.end(); ++obIt)
	{
		if ( (*obIt)->GetPhysicsSystem() )
		{
			(*obIt)->GetPhysicsSystem()->GetForceResult().ResetForce();
		}
	}
#endif
}


/***************************************************************************************************
*
*	FUNCTION		CEntityManager::GetEntityForLua
*
*	DESCRIPTION		
*
***************************************************************************************************/
int CEntityManager::GetEntityForLua( NinjaLua::LuaState& State )
{
	// Check for 2 args, table, index
	lua_checkstack( State, 2 );

	// Get the index, that's what we''re interested in
	const char* pcEntityName = lua_tostring( State, 2 );

	// Is there an entity going by that name?
	CEntity* pEnt = Get().FindEntity( pcEntityName );

	// Yes, no? Ifnot just return now
	if( !pEnt )
		return (0);

	// TEMP. Need a better mechanism to get the real type of the entity...
	if(pEnt->IsPlayer())
		return (NinjaLua::LuaValue::Push(State, (Player*)pEnt));


	// Ask the entity for it's lua value, and return object if valid
	return (NinjaLua::LuaValue::Push(State, pEnt));

}

/***************************************************************************************************
*
*	FUNCTION		CEntityManager::SetEntityForLua
*
*	DESCRIPTION		
*
***************************************************************************************************/
int CEntityManager::SetEntityForLua( NinjaLua::LuaState& /*State*/ )
{
	ntError_p( false, ("You can't set an entry in the Entities table") );

	return (0);
}


/***************************************************************************************************
*
*	FUNCTION		CEntityManager::CEntityManager
*
*	DESCRIPTION		
*
***************************************************************************************************/

CEntityManager::CEntityManager()
{
	ATTACH_LUA_INTERFACE(CEntityManager);

	m_bToRagdoll = false;
	m_pPrimaryPlayer = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create the Entities meta-table for lua.
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	NinjaLua::LuaState& luaState = CLuaGlobal::Get().State();

	NinjaLua::LuaObject luaGlobs = luaState.GetGlobals();

	NinjaLua::LuaObject luaEntsTbl = NinjaLua::LuaObject::CreateTable(luaState);
	luaGlobs.Set("Entities", luaEntsTbl);

	// Create the metatable
	NinjaLua::LuaObject luaEntsMetaTbl = NinjaLua::LuaObject::CreateTable(luaState);
	luaEntsMetaTbl.Set("__index",    NinjaLua::LuaObject(luaState, &CEntityManager::GetEntityForLua));
	luaEntsMetaTbl.Set("__newindex", NinjaLua::LuaObject(luaState, &CEntityManager::SetEntityForLua));

	luaEntsTbl.SetMetaTable(luaEntsMetaTbl);


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Expose the entity manager to Lua 
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	// Add the global Formation Manager instance. 
	NinjaLua::LuaState& State = CLuaGlobal::Get().State();
	NinjaLua::LuaValue::Push( luaState, this );
	State.GetGlobals().Set( "EntityManager", NinjaLua::LuaObject( -1, State, false) );

	int iEntitySize = sizeof(CEntity);
	UNUSED( iEntitySize );
	ntPrintf("Entity Size - %d\n", iEntitySize);

}


/***************************************************************************************************
*
*	FUNCTION		CEntityManager::~CEntityManager
*
*	DESCRIPTION		Destructor for CEntityManager class.. nothing in here yet, but who knows what
*					the future holds.
*			
*					The reason we don't destroy entities in here at the moment is because they don't
*					know how to remove themselves from any listspace structures used for rendering.
*
*					Also I'm not yet sure what they should do when removing themselves from their 
*					immediate parents..
*
***************************************************************************************************/
CEntityManager::~CEntityManager()
{
	// check to see if the entities are still around, if so bad, bad squishy
	ntError_p( m_entities.empty(), ("Entity Manager destroyed before all entity are") );
//ent	ntError_p( m_obStaticEntities.empty(), ("Entity Manager destroyed before all static entity are") );
}

/***************************************************************************************************
*
*	FUNCTION		CEntityManager::UnparentAllHierarchies
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntityManager::UnparentAllHierarchies ()
{
	// HC: Make sure all hierarchies are not parented to anything!
	// I added this because there were situations where an entity with a child hierarchy was being destroyed
	// before the child entity (causing the ntAssert in the entity destructor).

	// Run through all the entities in the buckets
	for (int iIndex = 0; iIndex < CEntity::m_iNumEntTypes; iIndex++)
	{
		EntityBucket& obBucket = GetBucket((CEntity::EntIndex)iIndex);
		for (EntityBucket::iterator obIt = obBucket.begin(); obIt != obBucket.end(); ++obIt)
	{
		if ((*obIt)->GetHierarchy())
		{
			if ( (*obIt)->GetHierarchy()->GetRootTransform()->GetParent() )
			{
				(*obIt)->GetHierarchy()->GetRootTransform()->RemoveFromParent();
			}
		}
	}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CEntityManager::Update
*
*	DESCRIPTION		Updates all entities..
*
*	NOTES			If entity update causes the creation of new entities, or destruction of existing
*					ones, then we're going to have to be *really* careful with the state of the 
*					iterator in this function.
*
*					I'd rather see a mechanism of having deferred creation & deletion of entities
*					developed rather than hacks-n-bodges.
*
***************************************************************************************************/
void	CEntityManager::Update( void )
{
	// Update stats
	if (m_iCurrentQueries > m_iMaxQueries) m_iMaxQueries = m_iCurrentQueries;
	m_iCurrentQueries = 0;

	// static entities first
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	// Static entities
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	CGatso::Start("CEntityManager::Statics");
	EntityBucket& obBucket = GetBucket(CEntity::EntIndex_Static);
	for ( EntityBucket::iterator obIt = obBucket.begin(); obIt != obBucket.end(); )
	{
		// Get a direct pointer to the entity
		Static*	pobStatic = (Static*)*obIt;
		// This entity had been flagged for removal (even paused entities can be removed)
		if (pobStatic->IsToDestroy()) 
		{
			obBucket.erase(obIt);
			ObjectDatabase::Get().Destroy(pobStatic);
			continue;
		}
		ntAssert( CTimer::Get().GetGameTimeChange() >= 0.0f );
		pobStatic->SetLastTimeChange( CTimer::Get().GetGameTimeChange() * pobStatic->GetTimeMultiplier() );
		// if we are paused, we are finished with
		if( pobStatic->IsPaused() )
		{
			++obIt;
			continue;
		}

		++obIt;
	}

	CGatso::Stop("CEntityManager::Statics");

	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	// Dynamic entities
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	//----------------------------------------------------------------------

	
	typedef ntstd::List<CEntity*> ToBeDeleted;
	ToBeDeleted toBeDeleted;

	CGatso::Start("CEntityManager::ComponentUpdate");
	// Lets go through all our entities
	for (BucketIterator buckIt = BucketBegin(); buckIt != BucketEnd(); ++buckIt)
	{
		for ( EntityIterator obIt = buckIt->begin(); obIt != buckIt->end(); ++obIt)
		{
			CGatso::Start("CEntityManager::ComponentUpdate::Prolog");
			// Get a direct pointer to the entity
			CEntity*	pobEntity = *obIt;			

			// This entity had been flagged for removal (even paused entities can be removed)
			if (pobEntity->IsToDestroy()) 
			{
				toBeDeleted.push_back(pobEntity);
				// put in a temp fix to stop the game falling on its arse when you free ents.
				m_obEntitiesToDelete.push_back(pobEntity);
//				ObjectDatabase::Get().Destroy(pobEntity);

				CGatso::Stop("CEntityManager::ComponentUpdate::Prolog");
				continue;
			}

			// Set the time update for this particular entity
			// Deano NOTE, I've decided to always update LastTimeChange
			// even if the entity was paused... I'm in two minds about this...
			// will make components that use time and aren't pause aware work
			// as they always have, BUT may cause some issue
			// likely to use a component Pause message to handle these cases.
			ntAssert( CTimer::Get().GetGameTimeChange() >= 0.0f );
			pobEntity->SetLastTimeChange( CTimer::Get().GetGameTimeChange() * pobEntity->GetTimeMultiplier() );

			// if we are paused, we are finished with
			if( pobEntity->IsPaused() )
			{
				CGatso::Stop("CEntityManager::ComponentUpdate::Prolog");
				continue;
			}

			float fTimeStep = pobEntity->GetLastTimeChange();

			CGatso::Stop("CEntityManager::ComponentUpdate::Prolog");

			// Input component? If so, update it..( NB MUST be before the movement update )
			// I have put this before the message store update so we can send button press events
			// without delay - GH
			if ( pobEntity->GetInputComponent() )
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::Input" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetInputComponent()->Update( fTimeStep );

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::Input" );
				#endif // PER_ENTITY_GATSO
			}

			// Update any cached entity information for this frame
			if ( !pobEntity->IsStatic() )
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::UpdateVelocity" );
				#endif // PER_ENTITY_GATSO

				pobEntity->UpdateVelocity( fTimeStep );

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::UpdateVelocity" );
				#endif // PER_ENTITY_GATSO
			}


			#ifndef _RELEASE
			if (pobEntity->IsCharacter())
			{
				pobEntity->ToCharacter()->DebugDisplayHealthHistory( fTimeStep );
			}
			#endif // _RELEASE


			// WORLD MATRIX VALID HERE

			// Message handler component? If so, update it..
			if ( pobEntity->GetMessageHandler() )
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::MessageHandler" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetMessageHandler()->Update(fTimeStep);

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::MessageHandler" );
				#endif // PER_ENTITY_GATSO
			}

			// Lua table - if so think
			//if ( pobEntity->HasLuaTable() )
			//{
			//	#if PER_ENTITY_GATSO
			//	CGatso::Start( "CEntityManager::Lua'Think'" );
			//	#endif // PER_ENTITY_GATSO

			//	pobEntity->DoThinkBehaviour();
			//	
			//	#if PER_ENTITY_GATSO
			//	CGatso::Stop( "CEntityManager::Lua'Think'" );
			//	#endif // PER_ENTITY_GATSO
			//}

			// Update the scene element component
			if ( pobEntity->GetSceneElement() )
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::SceneElement" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetSceneElement()->Update( fTimeStep );
				
				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::SceneElement" );
				#endif // PER_ENTITY_GATSO
			}

			// Must update AI component before input, movement and combat
			if ( pobEntity->IsAI() )
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::AI" );
				#endif // PER_ENTITY_GATSO

				((AI*)pobEntity)->GetAIComponent()->Update( fTimeStep );
				
				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::AI" );
				#endif // PER_ENTITY_GATSO
			}

			// Update the awareness component
			if ( pobEntity->GetAwarenessComponent() )
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::Awareness" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetAwarenessComponent()->Update( fTimeStep );
				
				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::Awareness" );
				#endif // PER_ENTITY_GATSO
			}

			// Attack component? If so, update it..
			if ( pobEntity->GetAttackComponent() )
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::Attack" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetAttackComponent()->Update( fTimeStep );

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::Attack" );
				#endif // PER_ENTITY_GATSO
			}

			// Controller component? If so, update it..
			if ( pobEntity->GetMovement() )
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::Movement" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetMovement()->Update( fTimeStep );

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::Movement" );
				#endif // PER_ENTITY_GATSO
			}
			
			// FSM Updates
			if(pobEntity->GetFSM())
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::FSM" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetFSM()->Update();

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::FSM" );
				#endif // PER_ENTITY_GATSO
			}
			

			// If we end up with entities deleting themselves or 
			// creating new entities, then we're going to have to be really 
			// careful that the iterator is valid at this point!
		}
	}

	CGatso::Stop("CEntityManager::ComponentUpdate");

//#	define CHECK_PRE_ANIM_SPU_SPEED
#	if defined( CHECK_PRE_ANIM_SPU_SPEED )
#		ifdef PLATFORM_PS3
		CGatso::Start( "CEntityManager::Pre EnitityManager Pause" );
		Exec::FrameEnd();
		Exec::FrameReset();
		CGatso::Stop( "CEntityManager::Pre EnitityManager Pause" );
#		endif
#	endif

#	if PER_ENTITY_GATSO
		CGatso::Start( "CEntityManager::Animator::Update" );
#	endif // PER_ENTITY_GATSO

	{
		// We first update all the objects that AREN'T parented to another hierarchy.
		// We keep track of the entities we should be updating that ARE parented as
		// they need to be updated AFTER the entity that they're parented to.
		ntstd::List< CEntity * > parented_entities;
		{
			// Start a batched animator update.
			AnimatorBatchUpdate anim_batcher;
			anim_batcher.StartBatch();

			// Do all the animator updates together.
			for ( BucketIterator buckIt = BucketBegin(); buckIt != BucketEnd(); ++buckIt )
			{
				for ( EntityIterator obIt = buckIt->begin(); obIt != buckIt->end(); ++obIt )
				{
					CEntity *pobEntity = *obIt;

					if ( pobEntity->IsToDestroy() ) 
					{
						continue;
					}

					if ( pobEntity->IsPaused() )
					{
						continue;
					}

					if ( pobEntity->GetAnimator() == NULL || pobEntity->GetHierarchy() == NULL )
					{
						continue;
					}

					ntError( pobEntity->GetHierarchy() != NULL );
					if ( pobEntity->GetHierarchy()->GetParent() != CHierarchy::GetWorld() )
					{
						parented_entities.push_back( pobEntity );
						continue;
					}
					else
					{
						Transform *root_parent = pobEntity->GetHierarchy()->GetRootTransform()->GetParent();
						if ( root_parent != NULL )
						{
							if ( root_parent->GetParentHierarchy() != CHierarchy::GetWorld() )
							{
								parented_entities.push_back( pobEntity );
								continue;
							}
						}
					}

					ntError( pobEntity->GetAnimator() != NULL );
					anim_batcher.AddAnimator( pobEntity->GetAnimator(), pobEntity->GetLastTimeChange() );
				}
			}

			// We've finished updating the batch.
			anim_batcher.FinishBatch();
		}

		{
			AnimatorBatchUpdate anim_batcher;
			anim_batcher.StartBatch();

			for (	ntstd::List< CEntity * >::iterator it = parented_entities.begin();
					it != parented_entities.end();
					++it )
			{
				CEntity *entity = *it;

				// Set the GpSkeleton internal root to be the world-matrix of the
				// parent of this hierarchy's root to simulate hierarchy parenting
				// to the ATG animation system.
				CHierarchy *hierarchy = entity->GetHierarchy_nonconst();
				ntError( hierarchy != NULL );
				hierarchy->SetGpSkeletonRootFromParent();

				ntError( entity->GetAnimator() != NULL );
				anim_batcher.AddAnimator( entity->GetAnimator(), entity->GetLastTimeChange() );

				// We don't need to reset the GpSkeleton-root as this is done when we
				// unlink a parented Transform object in Transform::RemoveFromParent().
			}

			anim_batcher.FinishBatch();
		}
	}

#	if PER_ENTITY_GATSO
		CGatso::Stop( "CEntityManager::Animator::Update" );
#	endif // PER_ENTITY_GATSO

	// Now do all the animator pre-render updates together.
	for (BucketIterator buckIt = BucketBegin(); buckIt != BucketEnd(); ++buckIt)
	{
		for ( EntityIterator obIt = buckIt->begin(); obIt != buckIt->end(); ++obIt)
		{
			CEntity *pobEntity = *obIt;

			if ( pobEntity->IsToDestroy() ) 
			{
				continue;
			}

			if( pobEntity->IsPaused() )
			{
				continue;
			}

			// Animator component? If so, update it..
			if ( pobEntity->GetAnimator() )
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::Animator::PreRenderUpdate" );
				#endif // PER_ENTITY_GATSO
				
				pobEntity->GetAnimator()->UpdateResults();

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::Animator::PreRenderUpdate" );
				#endif // PER_ENTITY_GATSO

			}
		}
	}

#ifdef PLATFORM_PS3
	ArmyManager::Get().UpdateKickSPUs( CTimer::Get().GetGameTimeChange() );
#endif	

	CGatso::Start("CEntityManager::3rdUpdate");

	for (BucketIterator buckIt = BucketBegin(); buckIt != BucketEnd(); ++buckIt)
	{
		for ( EntityIterator obIt = buckIt->begin(); obIt != buckIt->end(); ++obIt)
		{
			CEntity *pobEntity = *obIt;

			if ( pobEntity->IsToDestroy() ) 
			{
				continue;
			}

			if( pobEntity->IsPaused() )
			{
				continue;
			}

			float fTimeStep = pobEntity->GetLastTimeChange();

			// blendshapes component update if present
			if ( pobEntity->GetBlendShapesComponent()  )
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::BlendShapesComponent" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetBlendShapesComponent()->Update( fTimeStep );

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::BlendShapesComponent" );
				#endif // PER_ENTITY_GATSO

				// compare CAnimator/BSAnimator debug print info
				//pobEntity->GetBlendShapesComponent()->DebugPrint();
			}

			// Movement component? If so, update it..
			if ( pobEntity->GetMovement() )
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::MovementPost" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetMovement()->UpdatePostAnimator( fTimeStep );

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::MovementPost" );
				#endif // PER_ENTITY_GATSO

				#ifdef _DEBUG_DRAW_HIERARCHY_IN_HAVOK_VISUALISER
				//if(pobEntity->IsPlayer())
					Physics::HavokDebugDraw::DrawHierarchyInVisualiser(*(pobEntity->GetMovement()->GetAnimatorP()));
				#endif
			}

			if ( pobEntity->GetPhysicsSystem() )
			{
				#if PER_ENTITY_GATSO
				GATSO_PHYSICS_START( "Physics::All" );
				GATSO_PHYSICS_START( "PhysicsSystem::Update" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetPhysicsSystem()->Update( CTimer::Get().GetGameTimeChange() * pobEntity->GetTimeMultiplier() );

				#if PER_ENTITY_GATSO
				GATSO_PHYSICS_STOP( "PhysicsSystem::Update" );
				GATSO_PHYSICS_STOP( "Physics::All" );
				#endif // PER_ENTITY_GATSO
			}



#if 1
			if ( pobEntity->GetOneChain())
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::GetOneChainComponent::Update" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetOneChain()->Update( fTimeStep );

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::GetOneChainComponent::Update" );
				#endif // PER_ENTITY_GATSO
			}

			// Update any entities specific to that entity
			pobEntity->UpdateDerivedComponents(fTimeStep);
#endif
			// this must be updated after we're done messing with the skeleton
			if ( pobEntity->GetLookAtComponent() )
			{
			#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::LookAtComponent::Update" );
			#endif // PER_ENTITY_GATSO

				pobEntity->GetLookAtComponent()->Update( fTimeStep );

			#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::LookAtComponent::Update" );
			#endif // PER_ENTITY_GATSO
			}


			if ( pobEntity->GetFormationComponent())
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::GetFormationComponent::Update" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetFormationComponent()->Update( fTimeStep );

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::GetFormationComponent::Update" );
				#endif // PER_ENTITY_GATSO
			}
			
			if ( pobEntity->GetAimingComponent())
			{
				#if PER_ENTITY_GATSO
				CGatso::Start( "CEntityManager::GetAimingComponent::Update" );
				#endif // PER_ENTITY_GATSO

				pobEntity->GetAimingComponent()->Update( fTimeStep );

				#if PER_ENTITY_GATSO
				CGatso::Stop( "CEntityManager::GetAimingComponent::Update" );
				#endif // PER_ENTITY_GATSO
			}

			

			// If we end up with entities deleting themselves or 
			// creating new entities, then we're going to have to be really 
			// careful that the iterator is valid at this point!
		}
	}			
	CGatso::Stop("CEntityManager::3rdUpdate");


	CGatso::Start("CEntityManager::RemoveEntities");
	// remov from the manager
	for(ToBeDeleted::iterator it = toBeDeleted.begin();
		it != toBeDeleted.end();
		++it)
	{
		(*it)->RemoveFromWorld( false );
		Remove( *it );
	}
	CGatso::Stop("CEntityManager::RemoveEntities");

}

/***************************************************************************************************
*
*	FUNCTION		CEntityManager::Add
*
*	DESCRIPTION		Adds an entity to the manager class. 
*
*	INPUTS			pobEntity			-	Pointer to the entity to add
*
*	NOTES			This is only called as part of the CEntity construction process.
*
***************************************************************************************************/
void	CEntityManager::Add( CEntity* pobEntity )
{
	ntAssert( pobEntity );
	m_entities.add( *pobEntity );
	Bucket( pobEntity );
}

/***************************************************************************************************
*
*	FUNCTION		CEntityManager::Add
*
*	DESCRIPTION		Adds an entity to the manager class. 
*
*	INPUTS			pobEntity			-	Pointer to the entity to add
*
*	NOTES			This is only called as part of the CEntity construction process.
*
***************************************************************************************************/
void	CEntityManager::Add( Static* pobEntity )
{
	ntAssert( pobEntity );
	m_statics.add( *pobEntity );
	Bucket( pobEntity );
}

/***************************************************************************************************
*
*	FUNCTION		CEntityManager::Bucket
*
*	DESCRIPTION		Adds an entity to type buckets. 
*
*	INPUTS			pobEntity			-	Pointer to the entity to add
*
*	NOTES			This is only called as part of the CEntity construction process.
*
***************************************************************************************************/
void CEntityManager::Bucket( CEntity* pobEntity )
{
	ntAssert(pobEntity);

	// Push into the bucket
	GetBucket(pobEntity->GetEntType()).push_back(pobEntity);
}


/***************************************************************************************************
*
*	FUNCTION		CEntityManager::Remove
*
*	DESCRIPTION		Removes an entity from the manager class. 
*
*	INPUTS			pobEntity			-	Pointer to the entity to remove
*
*	NOTES			This is only called as part of the CEntity destruction process.
*
***************************************************************************************************/
void	CEntityManager::Remove( CEntity* pobEntity )
{
	ntAssert( pobEntity );

	// find and remove
	if ( m_entities.find(*pobEntity) )
	{
		m_entities.remove(*pobEntity);
	}

	// also remove from bucket

	EntityBucket& obBucket = GetBucket(pobEntity->GetEntType());

	// not normal, may be a static
	for ( EntityBucket::iterator obIt = obBucket.begin(); obIt != obBucket.end(); ++obIt )
	{
		// When we find our entity, remove it from the list and then return to the caller
		if ( ( *obIt ) == pobEntity )
		{
			obBucket.erase( obIt );
			return;
		}
	}

	// If we got here, then we didn't find the entity in our list.
	//ntAssert( false );
}


/***************************************************************************************************
*
*	FUNCTION		CEntityManager::FindEntity
*
*	DESCRIPTION		We need to locate CEntity objects by name, so here's a function that'll
*					do just that. 
*
***************************************************************************************************/
CEntity*	CEntityManager::FindEntity( const char* pcEntityName )
{
	// If our input is sensible
	if ( pcEntityName )
		return m_entities.find(pcEntityName);

	return 0;
}

CEntity*	CEntityManager::FindEntity( const CHashedString& name)
{
	return m_entities.find(name.GetHash());
}

volatile uint32_t CEntityManager::s_bPassedTest = 0;

void CEntityManager::FindEntityFunctionTaskAdaptor( void* a, void* b )
{
	if( s_bPassedTest )
	{
		CEntityQueryClause* pQuery = (CEntityQueryClause*) a;
		CEntity* pEntity = (CEntity*) b;
		if( ! (pQuery->Visit( *pEntity ) ) )
		{
			AtomicSet( &s_bPassedTest, 0 );
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CEntityManager::FindEntitiesByType
*
*	DESCRIPTION		This is the beginnings of a decent querying system for entities.  This will need
*					to be extended massively in the future.  For now the implementation is VERY 
*					simple and simply does the job.
*
*					We need to go through the full list of entities and add all those that pass
*					all the given tests back to the list in the query object.
*
***************************************************************************************************/
void CEntityManager::FindEntitiesByType( CEntityQuery& obQuery, uint32_t uiMask  )
{
	CGatso::Start( "CEntityManager::FindEntitiesByType" );
	// Update stats
	m_iCurrentQueries++;

	// Do the search
	int iIndex = 0;
	while (uiMask)
	{
		// If the type bit is set
		if (uiMask & 0x01)
		{
			FindEntitiesHelper(obQuery, (CEntity::EntIndex)iIndex);
		}

		// Move on to the next entity
		uiMask >>= 1;
		iIndex++;
	}
	CGatso::Stop( "CEntityManager::FindEntitiesByType" );
}

/***************************************************************************************************
*
*	FUNCTION		CEntityManager::FindEntitiesHelper
*
*	DESCRIPTION		This is the beginnings of a decent querying system for entities.  This will need
*					to be extended massively in the future.  For now the implementation is VERY 
*					simple and simply does the job.
*
*					We need to go through the full list of entities and add all those that pass
*					all the given tests back to the list in the query object.
*
***************************************************************************************************/
void CEntityManager::FindEntitiesHelper( CEntityQuery& obQuery, CEntity::EntIndex eIndex )
{
	// Get bucket
	EntityBucket& obBucket = GetBucket(eIndex);

	// Iterate through the entity list
	for ( EntityBucket::iterator obEntityIt = obBucket.begin(); obEntityIt != obBucket.end(); ++obEntityIt )
	{
		CEntity* pEntity = *obEntityIt;

		// Early out on paused entities
		if(pEntity->IsPaused()) continue;

		// Set a parameter that tells us whether the item has passed all the tests
		bool bPassedTest = true;

		// Iterate through the list of queries and try them all out on the current entity
		for( CEntityQuery::QuerySelectionContainerType::iterator obQueryIt = obQuery.m_obSelectionDetails.begin(); obQueryIt != obQuery.m_obSelectionDetails.end(); ++obQueryIt )
		{
			// Check if the test is passed
			if ( !( *obQueryIt )->Visit( *( pEntity ) ) )
			{
				// If we failed the test then set the flag and break out
				bPassedTest = false;
				break;
			}
		}

		// Now check against the negative clauses
		if ( !bPassedTest ) continue;

		// Iterate through the list of inverted queries and try them all out on the current entity
		for( CEntityQuery::QuerySelectionContainerType::iterator obQueryIt = obQuery.m_obUnSelectionDetails.begin(); obQueryIt != obQuery.m_obUnSelectionDetails.end(); ++obQueryIt )
		{
			// Check if the test is failed - if this passes the test we remove it
			if ( ( *obQueryIt )->Visit( *( pEntity ) ) )
			{
				// If we failed the test then set the flag and break out
				bPassedTest = false;
				break;
			}
		}

		// If the item passes all the clauses so far...
		if ( !bPassedTest ) continue;

		// Iterate through the list of excluded entities to make sure it isn't on of them
		for ( QueryExcludedEntitiesContainerType::iterator obExcludeIt = obQuery.m_obExcludedEntities.begin(); obExcludeIt != obQuery.m_obExcludedEntities.end(); ++obExcludeIt )
		{
			if ( ( *obExcludeIt ) == ( pEntity ) )
			{
				// This entity is excluded from the results
				bPassedTest = false;
				break;
			}
		}

		// If we passed the test add this entity to our list of found items
		if ( bPassedTest )
		{
			obQuery.m_obSelectedResults.push_back( ( pEntity ) );
		}
	}

}


//------------------------------------------------------------------------------------------
//!
//!	CEntityManager::SetPrimaryPlayer
//!	Set the primary player entity
//!
//------------------------------------------------------------------------------------------
void CEntityManager::SetPrimaryPlayer(Player* pPlayer)
{
	m_pPrimaryPlayer = pPlayer;

	// This really shouldn't be.
	// What if we have two views with primary entities in different areas?
		AreaManager::Get().SetPrinclipleEntity( pPlayer );
}



/***************************************************************************************************
*
*	FUNCTION		CEntityManager::Remove
*
*	DESCRIPTION		Removes an entity from the manager class. 
*
*	INPUTS			pobEntity			-	Pointer to the entity to remove
*
*	NOTES			This is only called as part of the CEntity destruction process.
*
***************************************************************************************************/
void	CEntityManager::Remove( Static* pobEntity )
{
	// find and remove
	if ( m_statics.find(*pobEntity) )
	{
		m_statics.remove(*pobEntity);
	}

	EntityBucket& obBucket = GetBucket(CEntity::EntIndex_Static);

	// Lets go through all our animations
	for ( EntityBucket::iterator obIt = obBucket.begin(); obIt != obBucket.end(); ++obIt )
	{
		// When we find our entity, remove it from the list and then return to the caller
		if ( ( *obIt ) == pobEntity )
		{
			obBucket.erase( obIt );
			return;
		}
	}

	// If we got here, then we didn't find the entity in our list. Something is screwed.
	ntAssert( false );
}

/***************************************************************************************************
*
*	FUNCTION		CEntityManager::FindStatic
*
*	DESCRIPTION		We need to locate CEntity objects by name, so here's a function that'll
*					do just that. 
*
***************************************************************************************************/
Static*	CEntityManager::FindStatic( const char* pcEntityName )
{
	// If our input is sensible
	if ( pcEntityName )
		return m_statics.find(pcEntityName);

	return 0;
}

Static*	CEntityManager::FindStatic( const CHashedString& name)
{
	return m_statics.find(name.GetHash());
}

//-----------------------------------------------------------------------------------------
//! 
//! CEntityManager::GetBucket
//! Get the bucket according to entity type
//!
//----------------------------------------------------------------------------------------
CEntityManager::EntityBucket& CEntityManager::GetBucket(CEntity::EntType eType)
{
	ntAssert(eType != 0);
	// Convert bit to index value 0-31
	int iMask = eType;
	int iIndex = 0;
	while (iMask >>= 1)
	{
		iIndex++;
	}

	// Just in case...
	ntError_p( iIndex < CEntity::m_iNumEntTypes, ("Index out of bounds.") );
	return m_aobEntityBuckets[iIndex];
}

//-----------------------------------------------------------------------------------------
//! 
//! CEntityManager::GetBucket
//! Get the bucket according to entity type
//!
//----------------------------------------------------------------------------------------
COMMAND_RESULT CEntityManager::GenerateReport()
{
	ntPrintf("\n\n// EntityReport -------------------------------------------------------\n");
	int iIndex = 0;
	int iCount = 0;
	for (BucketIterator buckIt = BucketBegin(); buckIt != BucketEnd(); ++buckIt)
	{
		const char* pcName = m_aobEntityBucketInfos[iIndex].m_pcName;
		UNUSED( pcName );
		int iSize = buckIt->size();
		ntPrintf("Bucket \"%s\" x %d\n", pcName, iSize);
		iCount+=iSize;
		iIndex++;
	}
	ntPrintf("Total = %d\n", iCount);
	ntPrintf("\nMax queries = %d\n", m_iMaxQueries);
	ntPrintf("// ~EntityReport ------------------------------------------------------\n\n");
	return CR_SUCCESS;
}


