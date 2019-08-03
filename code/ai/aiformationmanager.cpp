//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiformationmanager.cpp                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------
                                                                                            
                                                                                            
//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------

#include "ai/aiformationmanager.h"
#include "ai/aiformation.h"
#include "ai/aiformationattack.h"
#include "ai/aiformation_circle.h"
#include "ai/aiformation_line.h"
#include "ai/aiformation_mapped.h"
#include "ai/aiformationcomponent.h"
#include "ai/aisafezone.h"
#include "ai/aiformationxml.h"

#include "core/visualdebugger.h"

#include "game/entitymanager.h"
#include "game/messagehandler.h"
#include "game/luaexptypes.h"
#include "game/luahelp.h"

#include "input/inputhardware.h"

/***************************************************************************************************
* Start exposing the elements to Lua
***************************************************************************************************/

LUA_EXPOSED_START(AIFormationManager)
	LUA_EXPOSED_METHOD(GetCommander, GetCommander, "", "", "")
	LUA_EXPOSED_METHOD(AddFormationEntity, AddFormationEntity, "", "", "")
	LUA_EXPOSED_METHOD(PlayerPlayingDervish, PlayerPlayingDervish, "", "", "")
	LUA_EXPOSED_METHOD(IsAttackingPlayer, IsAttackingPlayer, "", "", "")
	LUA_EXPOSED_METHOD_RAW(DisableAllFormationEntities, DisableAllFormationEntities, "", "", "")
LUA_EXPOSED_END(AIFormationManager)


//------------------------------------------------------------------------------------------
// Static Members                                                                           
//------------------------------------------------------------------------------------------

#ifndef _RELEASE
bool AIFormationManager::m_bDebug = false;
#endif



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationManager::AIFormationManager                                                  
//! Construction                                                                            
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormationManager::AIFormationManager()
{
	ATTACH_LUA_INTERFACE(AIFormationManager);

	m_DebugAttackContext = -1;

#ifndef _RELEASE
	m_bKillAllEntities = false;
	m_MemoryTracker = 0;
#endif

	// Add the global Formation Manager instance. 
	NinjaLua::LuaState& State = CLuaGlobal::Get().State();
	NinjaLua::LuaValue::Push( State, this );
	State.GetGlobals().Set( "FormationMan", NinjaLua::LuaObject( -1, State, false) );

	// Making sure variables are initialised
	m_bPlayerPlayingDervish = false;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationManager::~AIFormationManager                                                 
//! Destruction                                                                             
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormationManager::~AIFormationManager()
{
#ifndef _RELEASE
	if( m_MemoryTracker )
	{
		Mem::FreeMemoryCheckpoint( m_MemoryTracker );
	}
#endif

	ATTACH_LUA_INTERFACE(AIFormationManager);

	// Set a nil value to the Global formation type
	NinjaLua::LuaState& State = CLuaGlobal::Get().State();
	State.GetGlobals().Set( "FormationMan", NinjaLua::LuaObject(State) );
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationManager::CreateFormation                                                     
//! Create a new formation for use by the AIs.                                              
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormation* AIFormationManager::CreateFormation(const NinjaLua::LuaObject& def)
{
	// Check that all is well
	///////////////////////////
	ntAssert(def[DEF_TYPE].IsNumber());
	
	// Make the new formation
	///////////////////////////
	AIFormation* pFormation;
	
	switch(def[DEF_TYPE].GetInteger())
	{
	case AFT_CIRCLE:
		pFormation = NT_NEW_CHUNK( Mem::MC_AI ) AIFormation_Circle(def);
		break;

	case AFT_LINE:
		pFormation = NT_NEW_CHUNK( Mem::MC_AI ) AIFormation_Line(def);
		break;
		
	case AFT_MAPPED:
		pFormation = NT_NEW_CHUNK( Mem::MC_AI ) AIFormation_Mapped(def);
		break;

	case AFT_ARBITRARY:
		pFormation = 0;
		break;

	default:
		pFormation = 0;
		ntAssert(false);
	}
		
	// Is there a target entity specified?
	if(def[DEF_TARGET_ENTITY].IsUserData())
		pFormation->m_pTargetEntity = def[DEF_TARGET_ENTITY].GetUserData<CEntity*>();
	else if(def[DEF_TARGET_ENTITY].IsFunction())
	{
		NinjaLua::LuaFunctionRet<CEntity*> obFunc( def[DEF_TARGET_ENTITY] );
		pFormation->m_pTargetEntity = obFunc();
		ntError(pFormation->m_pTargetEntity);
	}

	// There might be a separate lockon entity
	if (def[DEF_LOCKON_ENTITY].IsUserData())
		pFormation->m_pLockonEntity = def[DEF_LOCKON_ENTITY].GetUserData<CEntity*>();
	else
		pFormation->m_pLockonEntity = pFormation->m_pTargetEntity;

	// How about an offset?
	if (def[DEF_TARGET_OFFSET].IsTable())
		pFormation->m_ptOffset = CLuaHelper::PointFromTable(def[DEF_TARGET_OFFSET]);

	if (def[DEF_CONSTRAINT_ANGLE].IsNumber())
		pFormation->m_fConstraintAngle = def[DEF_CONSTRAINT_ANGLE].GetFloat();

	// Idle animation difficulty
	if (def[DEF_IDLE_ANIM].IsString())
		pFormation->m_IdleAnim = def[DEF_IDLE_ANIM].GetString();

	// Does the formation rotate relative to the camera view?
	if (def[DEF_CAMERARELATIVE].IsBoolean())
		pFormation->m_bCameraRelative = def[DEF_CAMERARELATIVE].GetBoolean();

	// Does the formation rotate relative to the camera view?
	if (def[DEF_CLOSEST_SLOT].IsBoolean())
		pFormation->m_bClosestSlot = def[DEF_CLOSEST_SLOT].GetBoolean();

	// Get the priority
	if (def[DEF_PRIORITY].IsNumber())
		pFormation->m_fPriority = def[DEF_PRIORITY].GetFloat();
	
	if (def[DEF_ANIM_SPEED_TEST].IsNumber())
		pFormation->m_fIdleAnimSpeed = def[DEF_ANIM_SPEED_TEST].GetFloat();

	if (def[DEF_AGGRESSIVE].IsBoolean())
		pFormation->m_bAggressive = def[DEF_AGGRESSIVE].GetBoolean();

	// Allocate the slots in the formation
	pFormation->AllocateSlots();

	// All Done.
	return pFormation;
}



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationManager::Update                                                              
//! Update all the formations in the game.                                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormationManager::Update(float fTimeDelta)
{
	UNUSED( fTimeDelta );
#ifndef _RELEASE
    if( m_bDebug )
	{
		g_VisualDebug->Printf2D(10.0f,8.0f, DC_WHITE, 0, "Debug Context %d", m_DebugAttackContext );
	}

	static int iCounter = 8;
	if( m_bKillAllEntities && !iCounter-- )
	{
		m_bKillAllEntities = false;
		iCounter = 8;
	}
#endif
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationManager::AdvanceDebugAttackContext                                             
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
COMMAND_RESULT AIFormationManager::AdvanceDebugAttackContext()
{
	m_DebugAttackContext++;

	int iMaxCount = 0;

	for( ntstd::List<CEntity*>::const_iterator obIt( m_FormationEntities.begin() ); obIt != m_FormationEntities.end(); ++obIt )
	{
		CEntity* pFormationEntity = *obIt;
		int iCount = pFormationEntity->GetFormationComponent()->AttackCount();
		if( iCount > iMaxCount )
			iMaxCount = iCount;
	}

	if( m_DebugAttackContext >= iMaxCount )
		m_DebugAttackContext = -1;

	return CR_SUCCESS; 
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationManager::GetCommander
//! For a given entity, find the best suited commander.
//!                                                                                         
//------------------------------------------------------------------------------------------
const CEntity* AIFormationManager::GetCommander(const CEntity* pAIEnt ) const
{
	// If the pAIEnt isn't given, then use the player. 
	if( !pAIEnt )
	{
		pAIEnt = CEntityManager::Get().GetPlayer();
	}

	CEntity*	pBestMatch = 0;
	float		fBestDestSqr = FLT_MAX;

	// Run through the list entities and find the nearest one. 
	for( ntstd::List<CEntity*>::const_iterator obIt( m_FormationEntities.begin() ); obIt != m_FormationEntities.end(); ++obIt )
	{
		CEntity* pFormationEntity = *obIt;

		ntAssert( pFormationEntity && "For some reason the entity formation pointer isn't valid" );
		ntAssert( pFormationEntity->GetFormationComponent() && "Check that the formation entity is using the construction script 'GroupCombatEntityOnConstruct'" );

		// Is there an active formation on the entity?
		if( !pFormationEntity->GetFormationComponent() || !pFormationEntity->GetFormationComponent()->ActiveFormationAvailable() )
			continue;

		// Work out the distance to the formation entity
		float fLenSqr = (pFormationEntity->GetPosition() - pAIEnt->GetPosition()).LengthSquared();

		// Is the formation the closest to the entity
		if( fLenSqr < fBestDestSqr )
		{
			fBestDestSqr = fLenSqr;
			pBestMatch = pFormationEntity;
		}
	}


	// Return the best match
	return pBestMatch;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationManager::AddFormationEntity
//! Add a formation entity to this list
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormationManager::AddFormationEntity( CEntity* pEnt )
{
	// Place the entity at the back of the list. 
	m_FormationEntities.push_back( pEnt );

}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationManager::IsAttackingPlayer
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AIFormationManager::IsAttackingPlayer( void )
{
	// for each formation, remove the required ones from the list. 
	for( ntstd::List<CEntity*>::iterator obIt = m_FormationEntities.begin(); obIt != m_FormationEntities.end(); ++obIt )
	{
		CEntity* pEnt = *obIt;
		
		FormationComponent* pFormationC = pEnt->GetFormationComponent();

		if( pFormationC )
		{
			if( pFormationC->GetEntityList().size() )
				return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationManager::RemoveFormationEntity
//! Removes an entity from the formation list 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormationManager::RemoveFormationEntity( CEntity* pEnt )
{
	m_FormationEntities.remove( pEnt );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationManager::DisableAllFormationEntities
//! Removes an entity from the formation list 
//!                                                                                         
//------------------------------------------------------------------------------------------
int AIFormationManager::DisableAllFormationEntities( NinjaLua::LuaState& rLuaState )
{
	NinjaLua::LuaArgs obArgs( rLuaState );

	ntstd::Vector< CEntity* > obDontTouch;

	// Get all the don't touch elements from the stack
	for( int Index = 1; NinjaLua::LuaValue::Is<CEntity*>( rLuaState, Index ); ++Index )
	{
		CEntity* pEntity = NinjaLua::LuaValue::Get<CEntity*>( rLuaState, Index );
		obDontTouch.push_back( pEntity );
	}

	// List of entities to remove
	ntstd::Vector< CEntity* > obRemoveList;

	// for each formation, remove the required ones from the list. 
	for( ntstd::List<CEntity*>::iterator obIt = m_FormationEntities.begin(); obIt != m_FormationEntities.end(); ++obIt )
	{
		CEntity* pEnt = *obIt;
		bool bFound = false;

		// Make sure that the entity can be removed. 
		for( ntstd::Vector< CEntity* >::const_iterator obFindIt = obDontTouch.begin(); obFindIt != obDontTouch.end(); ++obFindIt )
		{
			if( *obFindIt == pEnt )
			{
				bFound = true;
				break;
			}
		}

		// If not in the list, mark the entity for removal
		if( !bFound )
		{
			obRemoveList.push_back( pEnt );
		}
	}

	// Remove all the required entities
	while( obRemoveList.size() )
	{
		obRemoveList.back()->GetFormationComponent()->DeleteAllFormations();
		obRemoveList.pop_back();
	}

	return (0);
}

#ifndef _RELEASE
//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationManager::LuaFileReloaded
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormationManager::LuaFileReloaded()
{
	// Run through the list entities and find the nearest one. 
	for( ntstd::List<CEntity*>::const_iterator obIt( m_FormationEntities.begin() ); obIt != m_FormationEntities.end(); ++obIt )
	{
		CEntity* pFormationEntity = *obIt;

		if( pFormationEntity->GetFormationComponent() )
			pFormationEntity->GetFormationComponent()->ReStartCurrentFormation();
	}
}
#endif

