//--------------------------------------------------
//!
//!	\file WaterManager.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "water/WaterManager.h"
#include "water/waterinstance.h"
#include "water/waterinstancedef.h"
#include "water/waterdmadata.h"
#include "anim/transform.h"
#include "gfx/renderer.h"
#include "gfx/sector.h"
#include "objectdatabase/dataobject.h"

#ifdef PLATFORM_PS3
#include "exec/ppu/dmabuffer_ps3.h"
#include "exec/ppu/exec_ps3.h"
#include "exec/ppu/spuprogram_ps3.h"
#include "exec/ppu/sputask_ps3.h"
#include "exec/ppu/ElfManager.h"
#endif

#include "core/visualdebugger.h"
#include "input/inputhardware.h"



const static char* WATER_ELF_NAME = "water_spu_ps3.mod";

#ifdef PLATFORM_PS3
#	define WATER_INITIAL_STATE  true
#else
#	define WATER_INITIAL_STATE  false
#endif


WaterManager::WaterManager()
: m_obInstanceMap()
, m_bEnabled( WATER_INITIAL_STATE )
{
#ifdef PLATFORM_PS3
	ElfManager::Get().Load( WATER_ELF_NAME );
	m_pobSpuProgram = ElfManager::Get().GetProgram( WATER_ELF_NAME );
#endif
	m_bDebugRender					= false;
	m_bRenderPoints					= false;
	m_bRenderSpuLines				= false;
	m_bRenderBuoys					= false;
	m_bRenderSpuLines				= false;
}

WaterManager::~WaterManager()
{
	for ( InstanceMap_t::iterator it = m_obInstanceMap.begin(); it != m_obInstanceMap.end(); ++it )
	{
		if ( it->second )
		{
			NT_DELETE_CHUNK( Mem::MC_GFX, it->second );
		}
	}
}

WaterInstance*	WaterManager::GetNearestWaterInstanceTo( const CPoint& obPoint )
{
	float fNearest = 999999999.0f;
	WaterInstance* pobNearest = 0;
	for (InstanceMap_t::iterator obIt = m_obInstanceMap.begin(); obIt != m_obInstanceMap.end(); obIt++)
	{
		float fDistance = CDirection((*obIt).second->GetTransform()->GetWorldMatrix().GetTranslation() - obPoint).LengthSquared();
		if ( fDistance < fNearest )
		{
			pobNearest = (*obIt).second;
		}
	}

	return pobNearest;
}

WaterInstance*	WaterManager::GetWaterInstance( WaterInstanceDef& obDef )
{
	WaterInstance*& pobInstance = m_obInstanceMap[ &obDef ];
	if ( !pobInstance )
	{
		pobInstance = NT_NEW_CHUNK( Mem::MC_GFX ) WaterInstance( &obDef );
		CSector::Get().GetRenderables().AddRenderable( pobInstance );
	}

	return pobInstance;
}


WaterInstance*	WaterManager::GetWaterInstance( CHashedString obName )
{
	WaterInstance* pInstance = 0;
	WaterInstanceDef* pobDef = ObjectDatabase::Get().GetPointerFromName<WaterInstanceDef*>( obName );
	user_warn_p( pobDef, ("WATER - Couldn't get %s definition from database\n", ntStr::GetString(obName) ) );
	if ( pobDef )
	{
		pInstance = GetWaterInstance( *pobDef );
	}
	
	return pInstance;
}


void WaterManager::DestroyWaterInstance( WaterInstanceDef& obDef )
{
	InstanceMap_t::iterator it =  m_obInstanceMap.find( &obDef );
	if ( it != m_obInstanceMap.end() )
	{
		CSector::Get().GetRenderables().RemoveRenderable( it->second );
		NT_DELETE_CHUNK( Mem::MC_GFX, it->second );
		m_obInstanceMap.erase( it );
	}
}


void WaterManager::DestroyWaterInstance( WaterInstance& obInstance )
{
	DestroyWaterInstance( obInstance.GetDefinition() );
}


void WaterManager::DestroyWaterInstance( CHashedString obName )
{
	WaterInstanceDef* pobDef = ObjectDatabase::Get().GetPointerFromName<WaterInstanceDef*>( obName );
	user_warn_p( pobDef, ("WATER - Couldn't get %s definition from database\n", ntStr::GetString(obName) ) );
	if ( pobDef )
	{
		DestroyWaterInstance( *pobDef );
	}
}


void WaterManager::CreateInstanceAreaResources( WaterInstance& obInstance )
{
	obInstance.CreateDmaResources();
}


void WaterManager::CreateInstanceAreaResources( CHashedString obName )
{
	WaterInstance* pInstance = GetWaterInstance( obName );
	user_warn_p( pInstance, ("WATER - Couldn't create water instance %s's area resources \n", ntStr::GetString(obName) ) );
	if ( pInstance )
	{
		CreateInstanceAreaResources( *pInstance );
	}
}


void WaterManager::DestroyInstanceAreaResources( WaterInstance& obInstance )
{
	obInstance.DestroyDmaResources();
}


void WaterManager::DestroyInstanceAreaResources( CHashedString obName )
{
	WaterInstance* pInstance = GetWaterInstance( obName );
	user_warn_p( pInstance, ("WATER - Couldn't destroy water instance %s's area resources \n", ntStr::GetString(obName) ) );
	if ( pInstance )
	{
		DestroyInstanceAreaResources( *pInstance );
	}
}


void WaterManager::Update( float fTimeStep )
{
	if ( m_bEnabled )
	{
		for ( InstanceMap_t::iterator it = m_obInstanceMap.begin(); it != m_obInstanceMap.end(); ++it )
		{
			if ( it->second->IsRendering() && it->second->HasDmaResources() )
			{
				it->second->Update( fTimeStep );
				SendToSpu( it->second );
			}
		}
#ifndef _RELEASE
		DebugUpdate();
#endif
	}
}


void WaterManager::SendToSpu( WaterInstance* pobWater )
{
	UNUSED( pobWater );

#ifdef PLATFORM_PS3
	SPUTask task( m_pobSpuProgram );
	pobWater->SetupDma( task );
	Exec::RunTask( &task );
#endif
}


void WaterManager::DebugUpdate( void )
{
	if ( CInputHardware::Get().GetKeyboard().IsKeyHeld( KEYC_W ) )
	{
		if (CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_D ))
		{
			m_bDebugRender = !m_bDebugRender;
			if ( m_bDebugRender )
				ntPrintf( "WATER - Debug render ON\n" );
			else
				ntPrintf( "WATER - Debug render OFF\n" );
		}

		if (m_bDebugRender && CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_N ))
		{
			m_bRenderSpuLines = !m_bRenderSpuLines;
			if ( m_bRenderSpuLines )
				ntPrintf( "WATER - SPU lines render ON\n" );
			else
				ntPrintf( "WATER - SPU lines render OFF\n" );
		}

		if (CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_B ))
		{
			m_bRenderBuoys = !m_bRenderBuoys;
			if ( m_bRenderBuoys )
				ntPrintf( "WATER - buoy debug render ON\n" );
			else
				ntPrintf( "WATER - buoy debug render OFF\n" );
		}
#ifdef PLATFORM_PS3
		if (CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_O ))
		{
			for ( InstanceMap_t::iterator it = m_obInstanceMap.begin(); it != m_obInstanceMap.end(); ++it )
			{
				ntPrintf( "WATER - reloading debug shaders...\n" );
				it->second->m_pobVertexShader_Colour->GenerateResources();
				it->second->m_pobPixelShader_Colour->GenerateResources();
				it->second->m_pobPixelShader_Depth->GenerateResources();
				it->second->m_pobVertexShader_Depth->GenerateResources();
			}
		}
#endif
	}
}

void WaterManager::DebugRender( void )
{
#ifndef _RELEASE
	if ( m_bEnabled && m_bDebugRender )
	{
		for ( InstanceMap_t::iterator it = m_obInstanceMap.begin(); it != m_obInstanceMap.end(); ++it )
		{
			it->second->DebugRender( m_bRenderBuoys, m_bRenderSpuLines );
		}
	}
#endif
}

