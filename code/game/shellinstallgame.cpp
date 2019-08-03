//------------------------------------------------------------------------------------------
//!
//!	\file shellinstallgame.cpp
//! main update and render loops for the game
//!
//------------------------------------------------------------------------------------------

#include "shellinstallgame.h"
#include "core/wad.h"

#include <sys/ppu_thread.h>
#include <sysutil/sysutil_common.h>

namespace ShellInstaller
{
	static volatile int32_t	s_InstallCompleted = 0;

	//------------------------------------------------------------------------------------------
	//!
	//!	FrontEndResources
	//!
	//! Does all of content neutral and gfx resources required by GUI
	//!
	//------------------------------------------------------------------------------------------
	void FrontEndResources( uint64_t arg )
	{
		Wad::InstallGlobalWADs( DEFAULT_MEDIA );

		AtomicSetPlatform( &s_InstallCompleted, 1 );

		// Terminates the calling thread
		sys_ppu_thread_exit(0);
	}

	//------------------------------------------------------------------------------------------
	//!
	//!	GlobalResources
	//!
	//! Does all global resources required by a given level
	//!
	//------------------------------------------------------------------------------------------
	void GlobalResources( uint64_t arg )
	{
		Wad::InstallGlobalWADForLevels();

		AtomicSetPlatform( &s_InstallCompleted, 1 );

		// Terminates the calling thread
		sys_ppu_thread_exit(0);
	}
};

//------------------------------------------------------------------------------------------
//!
//!	InstallFrontendShellUpdater::ctor
//!
//------------------------------------------------------------------------------------------
InstallFrontendShellUpdater::InstallFrontendShellUpdater() :
	m_pBackup(0),
	m_bInstalled( false )
{
	m_pBackup = NT_NEW SimpleShellUpdater( "startup/status_installing_gui_colour.dds" );
	ntError_p( Wad::HaveCompleteGUIGameData() == false, ("Must not have complete GUI game data already") );

	AtomicSetPlatform( &ShellInstaller::s_InstallCompleted, 0 );

	int retVal;
	sys_ppu_thread_t threadID;
	retVal = sys_ppu_thread_create( &threadID, ShellInstaller::FrontEndResources, NULL,
									1001, (32*1024), 0, "InstallFEGameData" );

	ntError_p( CELL_OK == retVal, ( "GameData thread creation failed with error < %d >\n", retVal ) )	
}

//------------------------------------------------------------------------------------------
//!
//!	InstallFrontendShellUpdater::dtor
//!
//------------------------------------------------------------------------------------------
InstallFrontendShellUpdater::~InstallFrontendShellUpdater()
{
	NT_DELETE( m_pBackup );
}

//------------------------------------------------------------------------------------------
//!
//!	InstallFrontendShellUpdater::Update
//!
//------------------------------------------------------------------------------------------
void InstallFrontendShellUpdater::Update()
{
	m_pBackup->Update();

	// pump our system callbacks
	int ret = 0;
	ret = cellSysutilCheckCallback();
	ntError_p(ret==0,("Call to cellSysutilCheckCallback failed! (%d)\n",ret));

	if ( ShellInstaller::s_InstallCompleted )
		m_bInstalled = true;
}

//------------------------------------------------------------------------------------------
//!
//!	InstallFrontendShellUpdater::Render
//!
//------------------------------------------------------------------------------------------
void InstallFrontendShellUpdater::Render()
{
	m_pBackup->Render();
}




//------------------------------------------------------------------------------------------
//!
//!	InstallGlobalsShellUpdater::ctor
//!
//------------------------------------------------------------------------------------------
InstallGlobalsShellUpdater::InstallGlobalsShellUpdater() :
	m_pBackup(0),
	m_bInstalled( false )
{
	m_pBackup = NT_NEW SimpleShellUpdater( "startup/status_installing_game_colour.dds" );
	ntError_p( Wad::HaveCompleteGlobalGameData() == false, ("Must not have complete game data already") );

	AtomicSetPlatform( &ShellInstaller::s_InstallCompleted, 0 );

	int retVal;
	sys_ppu_thread_t threadID;
	retVal = sys_ppu_thread_create( &threadID, ShellInstaller::GlobalResources, NULL,
									1001, (32*1024), 0, "InstallGlobalGameData" );

	ntError_p( CELL_OK == retVal, ( "GameData thread creation failed with error < %d >\n", retVal ) )	
}

//------------------------------------------------------------------------------------------
//!
//!	InstallGlobalsShellUpdater::dtor
//!
//------------------------------------------------------------------------------------------
InstallGlobalsShellUpdater::~InstallGlobalsShellUpdater()
{
	NT_DELETE( m_pBackup );
}

//------------------------------------------------------------------------------------------
//!
//!	InstallGlobalsShellUpdater::Update
//!
//------------------------------------------------------------------------------------------
void InstallGlobalsShellUpdater::Update()
{
	m_pBackup->Update();

	// pump our system callbacks
	int ret = 0;
	ret = cellSysutilCheckCallback();
	ntError_p(ret==0,("Call to cellSysutilCheckCallback failed! (%d)\n",ret));

	if ( ShellInstaller::s_InstallCompleted )
		m_bInstalled = true;
}

//------------------------------------------------------------------------------------------
//!
//!	InstallGlobalsShellUpdater::Render
//!
//------------------------------------------------------------------------------------------
void InstallGlobalsShellUpdater::Render()
{
	m_pBackup->Render();
}

