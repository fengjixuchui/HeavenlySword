//------------------------------------------------------
//!
//!	\file army/armyrenderpool.cpp
//!
//------------------------------------------------------

#include "army/army_ppu_spu.h"
#include "army/armydef.h"
#include "army/armymanager.h"
#include "army/army_section.h"
#include "army/battlefield.h"
#include "army/battlefield_chunk.h"
#include "army/battalion.h"
#include "army/general.h"
#include "army/grunt.h"
#include "army/unit.h"
#include "army/armyrenderable.h"

#include "camera/camutils.h"

#include "effect/army_impostors.h"

#include "game/renderablecomponent.h"
#include "game/weapons.h"
#include "game/entityspawnpoint.h"
#include "game/randmanager.h"
#include "gfx/sector.h"

#include "objectdatabase/dataobject.h"

#include "army/armyrenderpool.h"


ArmyRenderPool::ArmyRenderPool( const Battlefield* pBattlefield, const ArmyUnitParameters* pParams, uint32_t iPoolSize, uint32_t iMaxImposters ) :
	m_pBattlefield( pBattlefield ),
	m_pParams( pParams ),
	m_iPoolSize( iPoolSize ),
	m_pArmyRenderables( 0 ),
	m_pBombRenderables( 0 ),
	m_SpriteSeeds( 0 )
{
	m_pArmyRenderables = NT_NEW_ARRAY_CHUNK( Mem::MC_ARMY ) ArmyRenderable[ m_iPoolSize ];

	ImpostorDef idleDef, walkDef, runDef, deadDef;
	idleDef.m_fWidth = 2.0f;
	idleDef.m_fHeight = 2.0f;
	idleDef.m_type = ImpostorDef::I_ANIMATED;
	idleDef.m_fCycleTime = 0.5f;
	idleDef.m_fAlphaTest = 0.2f;

	walkDef.m_fWidth = 2.0f;
	walkDef.m_fHeight = 2.0f;
	walkDef.m_type = ImpostorDef::I_ANIMATED;
	walkDef.m_fCycleTime = 0.5f;
	walkDef.m_fAlphaTest = 0.2f;
	
	runDef.m_fWidth = 2.0f;
	runDef.m_fHeight = 2.0f;
	runDef.m_type = ImpostorDef::I_ANIMATED;
	runDef.m_fCycleTime = 0.5f;
	runDef.m_fAlphaTest = 0.2f;
	
	deadDef.m_fWidth = 2.0f;
	deadDef.m_fHeight = 2.0f;
	deadDef.m_fAlphaTest = 0.2f;

	CKeyString clumpName;
	CHashedString animContainer;
	BasicWeaponSetDef* pWeapons;
	if( pParams )
	{
		clumpName = CKeyString( pParams->m_SpawnPool->GetTemplate()->GetClumpString() );
		animContainer = pParams->m_SpawnPool->GetTemplate()->GetAnimatorContainerName();
		// we assume all army guys used basic weapons
		pWeapons = (BasicWeaponSetDef*) pParams->m_SpawnPool->GetTemplate()->GetWeaponsDef();
		m_halfHeight = pParams->m_fHalfHeight;
		idleDef.m_surfaceTex = ntstd::String( pParams->m_IdleColourSprite.GetString() );
		idleDef.m_normalTex = ntstd::String( pParams->m_IdleNormalSprite.GetString() );
		idleDef.m_fCycleTime = pParams->m_IdleCycleTime;
		walkDef.m_surfaceTex = ntstd::String( pParams->m_WalkColourSprite.GetString() );
		walkDef.m_normalTex = ntstd::String( pParams->m_WalkNormalSprite.GetString() );
		walkDef.m_fCycleTime = pParams->m_WalkCycleTime;
		runDef.m_surfaceTex = ntstd::String( pParams->m_RunColourSprite.GetString() );
		runDef.m_normalTex = ntstd::String( pParams->m_RunNormalSprite.GetString() );
		runDef.m_fCycleTime = pParams->m_RunCycleTime;
		deadDef.m_surfaceTex = ntstd::String( pParams->m_DeadColourSprite.GetString() );
		deadDef.m_normalTex = ntstd::String( pParams->m_DeadNormalSprite.GetString() );
	} else
	{
		clumpName = CKeyString( "entities/characters/shieldman/shieldman.clump" );
		animContainer = CHashedString( "ShieldmanAnimContainer" );

		DataObject* pWeaponDO = ObjectDatabase::Get().GetDataObjectFromName( CHashedString("Sword_Create") );
		ntAssert( pWeaponDO && strcmp( pWeaponDO->GetClassName(), "BasicWeaponSetDef")== 0 );
		pWeapons = (BasicWeaponSetDef*) pWeaponDO->GetBasePtr();
		m_halfHeight = 0.9f;
		idleDef.m_surfaceTex = "imp_shieldman_idle_colour_alpha.tai";
		idleDef.m_normalTex = "imp_shieldman_idle_normal_mono.tai";
		walkDef.m_surfaceTex = "imp_shieldman_walk_colour_alpha.tai";
		walkDef.m_normalTex = "imp_shieldman_walk_normal_mono.tai";
		runDef.m_surfaceTex = "imp_shieldman_run_colour_alpha.tai";
		runDef.m_normalTex = "imp_shieldman_run_normal_mono.tai";
		deadDef.m_surfaceTex = "imp_shieldman_dead_colour_alpha.tai";
		deadDef.m_normalTex = "imp_shieldman_dead_normal_mono.tai";

	}

	for( uint32_t i=0;i < m_iPoolSize;i++)
	{
		ArmyRenderable* pRenderable = &m_pArmyRenderables[i];

		ArmyRenderableDef def( clumpName, animContainer );
		pRenderable->Init( def );

		// start with an idle animation on everybody (and force it so the basic non-animated pose isn't stupid)
		// and then disable and hide weapons they will then only be turned on as required
		pRenderable->PlayAnimation( GAM_IDLE );
		pRenderable->Update( 0.33f, NULL );		// No point in batching here - this is on construction.
		pRenderable->UpdatePartTwo();
		pRenderable->DisableRendering();

		// add to the free list
		m_FreeList.push_back( pRenderable );
	}



	
	// heres a list of all the current army texture atlases
	/*
	def.m_surfaceTex = "imp_shieldman_idle_colour_alpha.tai";
	def.m_normalTex = "imp_shieldman_idle_normal_mono.tai";
	def.m_surfaceTex = "imp_shieldman_walk_colour_alpha.tai";
	def.m_normalTex = "imp_shieldman_walk_normal_mono.tai";
	def.m_surfaceTex = "imp_shieldman_run_colour_alpha.tai";
	def.m_normalTex = "imp_shieldman_run_normal_mono.tai";
	def.m_surfaceTex = "imp_shieldman_dead_colour_alpha.tai";
	def.m_normalTex = "imp_shieldman_dead_normal_mono.tai";

	def.m_surfaceTex = "imp_fodder_idle_colour_alpha.tai";
	def.m_normalTex = "imp_fodder_idle_normal_mono.tai";
	def.m_surfaceTex = "imp_fodder_walk_colour_alpha.tai";
	def.m_normalTex = "imp_fodder_walk_normal_mono.tai";
	def.m_surfaceTex = "imp_fodder_run_colour_alpha.tai";
	def.m_normalTex = "imp_fodder_run_normal_mono.tai";
	def.m_surfaceTex = "imp_fodder_dead_colour_alpha.tai";
	def.m_normalTex = "imp_fodder_dead_normal_mono.tai";

	def.m_surfaceTex = "imp_axeman_idle_colour_alpha.tai";
	def.m_normalTex = "imp_axeman_idle_normal_mono.tai";
	def.m_surfaceTex = "imp_axeman_walk_colour_alpha.tai";
	def.m_normalTex = "imp_axeman_walk_normal_mono.tai";
	def.m_surfaceTex = "imp_axeman_run_colour_alpha.tai";
	def.m_normalTex = "imp_axeman_run_normal_mono.tai";
	def.m_surfaceTex = "imp_axeman_dead_colour_alpha.tai";
	def.m_normalTex = "imp_axeman_dead_normal_mono.tai";
	*/
	memset( m_pImposters, 0, sizeof( ArmyImpostors*) * MAX_GRUNT_ANIM );
	m_pImposters[ GAM_IDLE ] = NT_NEW_CHUNK( Mem::MC_ARMY ) ArmyImpostors( iMaxImposters, idleDef );
//	m_pImposters[ GAM_WALK ] = NT_NEW_CHUNK( Mem::MC_ARMY ) ArmyImpostors( iMaxImposters, walkDef );
	m_pImposters[ GAM_RUN ] = NT_NEW_CHUNK( Mem::MC_ARMY ) ArmyImpostors( iMaxImposters, runDef );
	m_pImposters[ GAM_DEAD ] = NT_NEW_CHUNK( Mem::MC_ARMY ) ArmyImpostors( iMaxImposters, deadDef );

	m_SpriteSeeds = NT_NEW_ARRAY_CHUNK( Mem::MC_ARMY ) float[ iMaxImposters ];
	for( uint32_t i=0;i < iMaxImposters;i++)
	{
		// unique seed per particle
		m_SpriteSeeds[i] = erandf( 1.0f );
	}

	for( uint32_t i=0; i < MAX_GRUNT_ANIM;++i )
	{
		if( m_pImposters[i] )
		{
			CSector::Get().GetRenderables().AddRenderable( m_pImposters[i] );
		}
	}

}
ArmyRenderPool::~ArmyRenderPool()
{
	NT_DELETE_ARRAY_CHUNK( Mem::MC_ARMY, m_SpriteSeeds );

	for( uint32_t i=0; i < MAX_GRUNT_ANIM;++i )
	{
		if( m_pImposters[i] )
		{
			CSector::Get().GetRenderables().RemoveRenderable( m_pImposters[i] );
			NT_DELETE_CHUNK(  Mem::MC_ARMY, m_pImposters[i] );
		}
	}
	if( m_pBombRenderables )
	{
		for( uint32_t i=0;i < m_iPoolSize;i++)
		{
			m_pBombRenderables[ i ].GetRootTransform()->RemoveFromParent();
		}
	}

	NT_DELETE_ARRAY_CHUNK(  Mem::MC_ARMY, m_pBombRenderables );
	NT_DELETE_ARRAY_CHUNK(  Mem::MC_ARMY, m_pArmyRenderables );
}

ArmyRenderable* ArmyRenderPool::Allocate()
{
	if( m_FreeList.empty() )
		return 0;

	ArmyRenderable* pRenderable = m_FreeList.front();
	m_FreeList.pop_front();
	//ntPrintf("Allocated 1 ArmyRenderable - %d Available\n", m_FreeList.size());

	return pRenderable;
}

void ArmyRenderPool::Deallocate( ArmyRenderable* pRenderable )
{
	m_FreeList.push_back( pRenderable );
	//ntPrintf("Deallocated 1 ArmyRenderable - %d Available\n", m_FreeList.size());
}

bool ArmyRenderPool::AnySpareRenderables()
{
	return m_FreeList.empty() ? false : true;
}

void ArmyRenderPool::ResetSprites()
{
	for( uint32_t i=0; i < MAX_GRUNT_ANIM;++i )
	{
		if( m_pImposters[i] )
		{
			m_pImposters[i]->GetBound() = CAABB( CONSTRUCT_CLEAR );
			m_pImposters[i]->GetImpostors().SetNumImpostors( 0 );
		}
	}
}

void ArmyRenderPool::SetSprite( const CPoint& pos, const uint32_t gruntID, const uint8_t iState )
{
	if( m_pImposters[ iState ] == 0 )
		return;

	CPoint offpos( pos + CDirection(0, m_halfHeight, 0) );
	m_pImposters[ iState ]->GetImpostors().SetPosition( offpos, m_pImposters[ iState ]->GetImpostors().GetNumImposters() );
	m_pImposters[ iState ]->GetImpostors().SetSeed( m_SpriteSeeds[ gruntID ], m_pImposters[ iState ]->GetImpostors().GetNumImposters() );
	m_pImposters[ iState ]->GetBound().Union( offpos ); 
	m_pImposters[ iState ]->GetImpostors().SetNumImpostors( m_pImposters[ iState ]->GetImpostors().GetNumImposters() + 1 );
}

void ArmyRenderPool::AddBombs( const CKeyString& bombName )
{
	m_pBombRenderables = NT_NEW_ARRAY_CHUNK( Mem::MC_ARMY ) ArmyRenderable[ m_iPoolSize ];

	static uint32_t r_weapon_hash = (uint32_t) CHashedString("r_weapon").GetHash();

	for( uint32_t i=0;i < m_iPoolSize;i++) 
	{
		ArmyRenderableDef def( bombName, CHashedString(), false, false, m_pArmyRenderables[i].GetHierarchy()->GetTransformFromHash( r_weapon_hash ) );
		m_pBombRenderables[ i ].Init( def );
	}
}

