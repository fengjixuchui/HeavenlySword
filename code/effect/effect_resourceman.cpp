//--------------------------------------------------
//!
//!	\file effect_resourceman.cpp
//!	Manages exotic resources used by effects
//!
//--------------------------------------------------

#include "effect/effect_resourceman.h"
#include "effect_error.h"
#include "game/luaglobal.h"
#include "gfx/hardwarecaps.h"
#include "core/file.h"
#include "core/fileattribute.h"

#ifdef PLATFORM_PC // FIXME_WIL
#include <sys/stat.h>
#else
#include <time.h>
#endif


static char g_aScriptName[512] = "INVALID";

#ifndef _RELEASE
static char errorMSG[512];
#endif

//--------------------------------------------------
//!
//!	ScriptResource
//! Usefull lua file resource type
//!
//--------------------------------------------------
ScriptResource::ScriptResource()
{
	m_pScriptName = 0;
	EffectResourceMan::Get().RegisterResource( *this );
}

void ScriptResource::SetFile( const char* pName )
{
	ntError(pName);
	int ilen = strlen(pName);

	if (m_pScriptName)
		NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, m_pScriptName );

	// [scee_st] was only NT_NEW before
	m_pScriptName = NT_NEW_ARRAY_CHUNK ( Mem::MC_EFFECTS ) char [ilen+1];
	strcpy(m_pScriptName,pName);

	m_scriptModDate = -1;
	m_fCheckTimer = 2.0f;

	Util::GetFiosFilePath( m_pScriptName, g_aScriptName );
	m_bFileExists = File::Exists( g_aScriptName );

	if (!m_bFileExists)
	{
		#ifndef _RELEASE
		// file is not found
		sprintf( errorMSG, "FILE %s does not exist\n", m_pScriptName );
		EffectErrorMSG::AddDebugError( errorMSG );
		#endif
		m_fCheckTimer = 5.0f;
	}
}

ScriptResource::~ScriptResource()
{
	if (m_pScriptName)
		NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, m_pScriptName );
	EffectResourceMan::Get().ReleaseResource( *this );
}

void ScriptResource::GenerateResources()
{
	if (m_bFileExists)
	{
		Util::GetFiosFilePath( m_pScriptName, g_aScriptName );

		CFileAttribute oTmpStat(g_aScriptName);
		m_scriptModDate = oTmpStat.GetModifyTime();

		lua_State* L = &(*CLuaGlobal::Get().State());
		
		int stackTop = lua_gettop(L);					// store stack top for restore
		int status = luaL_loadfile(L, g_aScriptName);		// load and install our script file

		if (status == 0)
		{
			status = lua_pcall(L, 0, LUA_MULTRET, 0);	// call main
			if (status != 0)
			{
				#ifndef _RELEASE
				// install failed for some reason
				sprintf( errorMSG, "ERROR PARSING FILE %s: %s\n", g_aScriptName, lua_tostring(L, -1) );
				EffectErrorMSG::AddDebugError( errorMSG );
				#endif
			}
		}
		else
		{
			#ifndef _RELEASE
			// read failed for some reason
			sprintf( errorMSG, "ERROR PARSING FILE %s: %s\n", g_aScriptName, lua_tostring(L, -1) );
			EffectErrorMSG::AddDebugError( errorMSG );
			#endif
		}

		// restore stack
		lua_settop(L, stackTop);
	}
}

bool ScriptResource::ResourcesOutOfDate() const
{
	if (!m_pScriptName)
		return false;

	m_fCheckTimer -= CTimer::Get().GetSystemTimeChange();

	if (m_bFileExists)
	{
		if (m_fCheckTimer < 0.0f)
		{
			m_fCheckTimer = 2.0f;
			Util::GetFiosFilePath( m_pScriptName, g_aScriptName );

			CFileAttribute oTmpStat(g_aScriptName);
			if (oTmpStat.GetModifyTime() > m_scriptModDate ) return true;

			return false;
		}
	}
	else
	{
		if (m_fCheckTimer < 0.0f)
		{
			Util::GetFiosFilePath( m_pScriptName, g_aScriptName );
			m_bFileExists = File::Exists( g_aScriptName );
			m_fCheckTimer = 5.0f;
			return m_bFileExists;
		}
	}

	return false;
}

//--------------------------------------------------
//!
//!	EffectResourceMan::ctor
//! Just loads and caches FX files at the moment
//!
//--------------------------------------------------
EffectResourceMan::EffectResourceMan()
{
	// this is needed to know when it's time to refresh resources
	m_fCurrentTime = 0.0f;
	m_iFirstRefreshed[0] = m_iFirstRefreshed[1] = -1;
	m_iRefreshCounter = 0;

#ifdef PLATFORM_PC // FIXME_WIL
	HRESULT hr;
	hr = D3DXCreateEffectPool( m_effectPool.AddressOf() );
	ntAssert( hr == S_OK );

	// particle systems

	AddFXFileToPool( "fxshaders\\psystem_simple_cpu.fx", "psystem_simple_cpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_simple_gpu.fx", "psystem_simple_gpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_complex_cpu.fx", "psystem_complex_cpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_complex_gpu.fx", "psystem_complex_gpu", m_effectPool );

	AddFXFileToPool( "fxshaders\\psystem_simple_rot_cpu.fx", "psystem_simple_rot_cpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_simple_rot_gpu.fx", "psystem_simple_rot_gpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_complex_rot_cpu.fx", "psystem_complex_rot_cpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_complex_rot_gpu.fx", "psystem_complex_rot_gpu", m_effectPool );

	AddFXFileToPool( "fxshaders\\psystem_simple_orientquad_cpu.fx", "psystem_simple_orientquad_cpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_simple_orientquad_gpu.fx", "psystem_simple_orientquad_gpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_complex_orientquad_cpu.fx", "psystem_complex_orientquad_cpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_complex_orientquad_gpu.fx", "psystem_complex_orientquad_gpu", m_effectPool );

	AddFXFileToPool( "fxshaders\\psystem_simple_axisray_cpu.fx", "psystem_simple_axisray_cpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_simple_axisray_gpu.fx", "psystem_simple_axisray_gpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_complex_axisray_cpu.fx", "psystem_complex_axisray_cpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_complex_axisray_gpu.fx", "psystem_complex_axisray_gpu", m_effectPool );

	AddFXFileToPool( "fxshaders\\psystem_simple_velscaleray_cpu.fx", "psystem_velscaleray_cpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_simple_velscaleray_gpu.fx", "psystem_velscaleray_gpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_complex_velscaleray_cpu.fx", "psystem_complex_velscaleray_cpu", m_effectPool );
	AddFXFileToPool( "fxshaders\\psystem_complex_velscaleray_gpu.fx", "psystem_complex_velscaleray_gpu", m_effectPool );

	// trail systems

	AddFXFileToPool( "fxshaders\\effecttrail_simple.fx", "effecttrail_simple", m_effectPool );
	AddFXFileToPool( "fxshaders\\effecttrail_line.fx", "effecttrail_line", m_effectPool );

	// fake materials -- these use varaibles normaly present in our material system

	hr = D3DXCreateEffectPool( m_fakeMaterialPool.AddressOf() );
	ntAssert( hr == S_OK );

	if( HardwareCapabilities::Get().SupportsShaderModel3() )
	{
		AddFXFileToPool( "fxshaders\\rangestancechain_nv40.fx", "rangestancechain", m_fakeMaterialPool );
		AddFXFileToPool( "fxshaders\\cloth_nv40.fx", "cloth", m_fakeMaterialPool );
		AddFXFileToPool( "fxshaders\\flibble.fx", "flibble", m_fakeMaterialPool );
	} else
	{
		AddFXFileToPool( "fxshaders\\rangestancechain_ati.fx", "rangestancechain", m_fakeMaterialPool );
		AddFXFileToPool( "fxshaders\\cloth_ati.fx", "cloth", m_fakeMaterialPool );
	}
#endif
}

//--------------------------------------------------
//!
//!	EffectResourceMan::dtor
//!
//--------------------------------------------------
EffectResourceMan::~EffectResourceMan()
{
	while (!m_FXHandles.empty())
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_FXHandles.back() );
		m_FXHandles.pop_back();
	}
}

//--------------------------------------------------
//!
//!	EffectResourceMan::AddFXFileToPool
//!
//--------------------------------------------------
void EffectResourceMan::AddFXFileToPool( const char* pFXFile, const char* pName, FXPoolHandle pool )
{
	for (	ntstd::List<FXHandle*, Mem::MC_EFFECTS>::iterator it = m_FXHandles.begin();
			it !=  m_FXHandles.end(); ++it )
	{
		if ( strcmp( (*it)->GetName(), pName ) == 0 )
		{
			ntError_p( 0 , ("Adding an FX file that already exists\n") );
		}
	}

	m_FXHandles.push_back( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) FXHandle( pFXFile, pName, pool.Get() ) );
}

//--------------------------------------------------
//!
//!	EffectResourceMan::RetrieveFXHandle
//!
//--------------------------------------------------
FXHandle* EffectResourceMan::RetrieveFXHandle( const char* pName )
{
	for (	ntstd::List<FXHandle*, Mem::MC_EFFECTS>::iterator it = m_FXHandles.begin();
			it !=  m_FXHandles.end(); ++it )
	{
		if ( strcmp( (*it)->GetName(), pName ) == 0 )
		{
			return (*it);
		}
	}

#ifndef PLATFORM_PS3
	ntError_p( 0 , ("FX file %s does not exist\n", pName) );
#endif
	
	static FXHandle invalidHandle;
	return &invalidHandle;
}

//--------------------------------------------------
//!
//!	EffectResourceMan::RefreshResources
//!
//--------------------------------------------------
#define fUpdateTime (1.0f)	// resources refresh time (in seconds..)
							// Please note that this time has to be greater than a single frame time!

void EffectResourceMan::RefreshResources()
{
	// only do this in debug or developement builds
#ifndef _RELEASE

	int iFXCount = m_FXHandles.size();
	int iResourceCount = m_resources.size();

	// we don't want to refresh resources on a per gameloop basis..
	// so we try to spread resources refresh over a given time budget (fUpdateTime)
	float fTime = static_cast<float>(CTimer::Get().GetSystemTime());
	if (((fTime - m_fCurrentTime) < fUpdateTime) && (m_fCurrentTime != 0.0f))
	{
		// count how many refreshes are we going to do per updatetime
		m_iRefreshCounter++;

	// This stuff SHOULD be splitted over 2 source files (PC and PS3 version)
	// but I'm not going to do it now cause we still don't have a replacement
	// for the FX stuff on PS3 and we still don't know at this time what we are gonna do..
	// So I'm gonna wait to have a clear picture, for now let's just have a #ifdef (Marco)
#if defined( PLATFORM_PC )
		// if there are fx to refresh go on..
		if (iFXCount != 0)
		{
			m_fRefreshDDA[0] += m_fRefreshRate[0];
			int iTemp = static_cast<int>(m_fRefreshDDA[0]);

			// if next FX has not been refreshed yet and if we haven't refreshed all the FXs..
			if ((iTemp > m_iFirstRefreshed[0]) && (iTemp <= iFXCount))
			{
				ntstd::List<FXHandle*, Mem::MC_EFFECTS>::iterator FXIterator = m_FXHandles.begin();
				// walk till we reach the first unrefreshed FX 
				for (int i=0; i < m_iFirstRefreshed[0]; i++) FXIterator++;
				
				//check if the FX resource need to be refreshed
				while ((m_iFirstRefreshed[0] < iTemp) && (FXIterator != m_FXHandles.end()))
				{
					if ((*FXIterator)->IsOutOfDate())
						(*FXIterator)->ReloadMe();
					
					// next FX please..
					FXIterator++;
					m_iFirstRefreshed[0]++;
    			} 
			}
		}
#endif
		
		// see the coomments above..
		if (iResourceCount != 0)
		{
			m_fRefreshDDA[1] += m_fRefreshRate[1];
			int iTemp = static_cast<int>(m_fRefreshDDA[1]);

			if ((iTemp > m_iFirstRefreshed[1]) && (iTemp <= iResourceCount))
			{
				ntstd::List<EffectResource*, Mem::MC_EFFECTS>::iterator ResourceIterator = m_resources.begin();
				for (int i=0; i < m_iFirstRefreshed[1]; i++) ResourceIterator++;
		
				while ((m_iFirstRefreshed[1] < iTemp) && (ResourceIterator != m_resources.end()))
				{
					if ((*ResourceIterator)->ResourcesOutOfDate())
						(*ResourceIterator)->GenerateResources();

					ResourceIterator++;
					m_iFirstRefreshed[1]++;
    			}
			}	
		}
	}
	// once in a while (default 1 sec) we are going to 'recalibrate'..
	else
	{	
		//debug code
		//ntPrintf("%i\n", m_iRefreshCounter);
		//ntPrintf("%i %i\n", m_iFirstRefreshed[0], m_iFirstRefreshed[1]);
		//ntPrintf("%f %f\n", m_fRefreshRate[0], m_fRefreshRate[1]);
		//ntPrintf("%f %f\n\n", m_fRefreshDDA[0], m_fRefreshDDA[1]);

		// DDAs start value is set to 0.5 to spread ntError in both list walking directions
		m_fRefreshDDA[0] = m_fRefreshDDA[1] = 0.5f;

		// reset 'next FX to be refreshed' counters
		m_iFirstRefreshed[0] = m_iFirstRefreshed[1] = -1;

		float fDummy = 1.0f / static_cast<float>(ntstd::Max(m_iRefreshCounter - 1, 1));
		
		//update refreshing rate
		m_fRefreshRate[0] = static_cast<float>(iFXCount) * fDummy;
		m_fRefreshRate[1] = static_cast<float>(iResourceCount) * fDummy;
		
		//save current time, that's needed in order to know when we have to recalibrate everything
		m_fCurrentTime = fTime;
		m_iRefreshCounter = 0;

	}

#endif // _RELEASE

	// this is to get around some point sprite shaders not working on 1st compile.
	// i suspect there are problems with the spec for using point sprites with pixel
	// shaders in FX files, (namely where the tex coord is supposed to be).
	// will investigate using cgFX and FX composer. WD, 04-01-2005

#if defined( PLATFORM_PC )
	static bool bInitialiseHack = true;

	if (bInitialiseHack)
	{
		for (	ntstd::List<FXHandle*, Mem::MC_EFFECTS>::iterator it = m_FXHandles.begin();
				it !=  m_FXHandles.end(); ++it )
		{
			(*it)->ReloadMe();
		}

		bInitialiseHack = false;
	}
#endif
}

//--------------------------------------------------
//!
//!	EffectResourceMan::ForceRecompile
//!
//--------------------------------------------------
void EffectResourceMan::ForceRecompile()
{
	for (	ntstd::List<FXHandle*, Mem::MC_EFFECTS>::iterator it = m_FXHandles.begin();
			it !=  m_FXHandles.end(); ++it )
	{
		(*it)->ReloadMe( true );
	}
}
