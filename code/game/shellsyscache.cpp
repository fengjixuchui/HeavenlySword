//------------------------------------------------------------------------------------------
//!
//!	\file shellupdater.cpp
//! main update and render loops for the game
//!
//------------------------------------------------------------------------------------------

#include "shellsyscache.h"
#include "core/wad.h"

#include <sys/ppu_thread.h>
#include <sysutil/sysutil_syscache.h>

//------------------------------------------------------------------------------------------
//!
//! Data for the PARAM.SFO file.	
//! 
//! PLEASE NOTE, THIS IS ALL TEMPORARY FOR TESTING 
//! PURPOSES AND WILL BE REPLACED WITH PROPER DATA.
//!	
//------------------------------------------------------------------------------------------
#define PARAMSFO_TITLEID		"ABCD12345"	// Title product code with hyphen removed.

const char *g_SysCachePath;

namespace ShellSysCache
{
	static volatile int32_t	s_SysCacheOkay = 0;
	static volatile int32_t	s_SysCacheFail = 0;

	// path used by FIOS when accessing game data
	static char s_SysCachePath[ CELL_SYSCACHE_PATH_MAX+1 ];

	//------------------------------------------------------------------------------------------
	//!
	//!	ThreadSysCache
	//!
	//! cellSysCacheMount must be run from a seperate thread to avoid main thread
	//! disruption
	//!
	//------------------------------------------------------------------------------------------
	void ThreadSysCache( uint64_t arg  )
	{
		CellSysCacheParam params;
		params.reserved = NULL;
		strcpy( params.cacheId, PARAMSFO_TITLEID );

		// Uncomment this line to clear the system-cache every time we load the game.
	//	params.cacheId[ 0 ] = '\0';

		int32_t ret = cellSysCacheMount( &params );
		if ( ret != CELL_SYSCACHE_RET_OK_RELAYED && ret != CELL_SYSCACHE_RET_OK_CLEARED )
		{
			ntPrintf( "cellSysCacheMount failed\n" );
			g_SysCachePath = NULL;

			AtomicSetPlatform( &s_SysCacheFail, 1 );
			g_SysCachePath = &s_SysCachePath[ 0 ];

		}
		else
		{
			strcpy( s_SysCachePath, params.getCachePath );

			uint32_t sys_cache_path_len = (uint32_t)strlen( s_SysCachePath );
			ntError( sys_cache_path_len < CELL_SYSCACHE_PATH_MAX );

			if ( s_SysCachePath[ sys_cache_path_len - 1 ] != '/' )
				s_SysCachePath[ sys_cache_path_len ] = '/';

			g_SysCachePath = &s_SysCachePath[ 0 ];
			AtomicSetPlatform( &s_SysCacheOkay, 1 );
		}

		sys_ppu_thread_exit(0);
	}
};

//------------------------------------------------------------------------------------------
//!
//!	SysCacheShellUpdater::ctor
//!
//------------------------------------------------------------------------------------------
SysCacheShellUpdater::SysCacheShellUpdater() :
	m_pBackup(0),
	m_eStatus( SC_UNKNOWN )
{
	m_pBackup = NT_NEW SimpleShellUpdater( "startup/status_systemcache_colour.dds" );
	InitSysCache();
}

//------------------------------------------------------------------------------------------
//!
//!	SysCacheShellUpdater::dtor
//!
//------------------------------------------------------------------------------------------
SysCacheShellUpdater::~SysCacheShellUpdater()
{
	NT_DELETE( m_pBackup );
}

//------------------------------------------------------------------------------------------
//!
//!	SysCacheShellUpdater::InitGameData
//!
//------------------------------------------------------------------------------------------
void SysCacheShellUpdater::InitSysCache()
{
	// cellGameDataCheckCreate must be run on a separate thread so not to interupt main thread.
	ntPrintf("START - SysCacheShellUpdater::InitSysCache()\n");

	// cellSysCacheMount must to be executed within a seperate thread to avoid be interupted.
	int retVal;
	sys_ppu_thread_t threadID;
	retVal = sys_ppu_thread_create( &threadID, ShellSysCache::ThreadSysCache, NULL,
									1001, (32*1024), 0, "SystemCacheThread" );

	ntError_p( CELL_OK == retVal, ( "System cache thread creation failed with error < %d >\n", retVal ) )	
	ntPrintf("END - SysCacheShellUpdater::InitSysCache()\n");
}

//------------------------------------------------------------------------------------------
//!
//!	SysCacheShellUpdater::Update
//!
//------------------------------------------------------------------------------------------
void SysCacheShellUpdater::Update()
{
	m_pBackup->Update();

	// pump our system callbacks
	int ret = 0;
	ret = cellSysutilCheckCallback();
	ntError_p(ret==0,("Call to cellSysutilCheckCallback failed! (%d)\n",ret));

	if ( ShellSysCache::s_SysCacheOkay )
	{
		// we can exit to the next stage
		m_eStatus = SC_SUCCEEDED;

		// have a quick peek to see if we need to do a full or partial install later on.
		Wad::RecordSysCacheUsage();
	}
	else if ( ShellSysCache::s_SysCacheFail )
	{
		// error message will have been displayed by the utility, now 
		// we give them a better explanation and ask them to quit.
		m_eStatus = SC_FAILED;

		// delete our backup screen, then create a new one with the relevant message
		NT_DELETE( m_pBackup );
		m_pBackup = NT_NEW SimpleShellUpdater( "startup/error_syscache.dds" );

		// clear var so we dont do this again, wait for user to quit
		ShellSysCache::s_SysCacheFail = 0;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	SysCacheShellUpdater::Render
//!
//------------------------------------------------------------------------------------------
void SysCacheShellUpdater::Render()
{
	m_pBackup->Render();
}
