//------------------------------------------------------------------------------------------
//!
//!	\file shellmain.cpp
//! main update and render loops for the game
//!
//------------------------------------------------------------------------------------------

#include "game/shellmain.h"
#include "game/shelldebug.h"
#include "game/shellglobal.h"
#include "game/shellgame.h"
#include "game/shelllevel.h"
#include "game/shellupdater.h"

#include "game/shellgamedata.h"
#include "game/shellplayeroptions.h"
#include "game/shellplayerprogression.h"
#include "game/shellsyscache.h"
#include "game/shellinstallgame.h"

#include "gui/guimanager.h"

#include "core/timer.h"
#include "core/gatso.h"
#include "gfx/renderer.h"

#include "exec/exec.h"

#ifdef PLATFORM_PS3
#include "core/wad.h"
#include "core/fileio_ps3.h"
#endif

namespace ShellAsync
{
	static volatile int32_t	s_TaskComplete = 0;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::AsyncCreateGame
//!	will be called asynchronously by exec
//! after this, all objects required to run the game will be available
//!
//------------------------------------------------------------------------------------------
void	ShellMain::AsyncCreateGame( ShellMain* pParent )
{
	ShellGlobal::AsyncInstall();
	ShellGame::AsyncInstall();

	pParent->m_pDebugShell->RegisterDebugKeys();

	AtomicSetPlatform( &ShellAsync::s_TaskComplete, 1 );
}

void ShellMain::AsyncCreateGameTask( void* pParam, void* )
{
	ShellMain* pMain = (ShellMain*)pParam;
	ShellMain::AsyncCreateGame( pMain );
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::AsyncCreateLevel
//!	will be called asynchronously by exec
//! after this, all objects required to run the level will be available
//!
//------------------------------------------------------------------------------------------
void	ShellMain::AsyncCreateLevel( ShellMain* pParent )
{
	pParent->m_pCurrLevel->AsyncInstall();

	// Register keys that need the level to be in place
	pParent->m_pDebugShell->RegisterDebugKeysForLevel();

	// some remaining tidy up work
	CTimer::Get().ResetGameTime();

	// if the gui is active, there is an implicit assumption that user mode pause is active,
	// as the gui system unpauses the moment it moves to a 'HUD' screen
	if (g_ShellOptions->m_eFrontendMode != FRONTEND_NONE)
	{
		// Note, explict unpause is required, as there is a pause left over from exiting the game to return
		// this needs to be cleaned up on the gui side
		if (CHashedString( pParent->m_pCurrLevel->GetLevelName() ) == CHashedString("purgatory/purgatory"))
			pParent->RequestPauseToggle( PM_USER, false );
		else
			pParent->RequestPauseToggle( PM_USER, true );
	}

	AtomicSetPlatform( &ShellAsync::s_TaskComplete, 1 );
}

void ShellMain::AsyncCreateLevelTask( void* pParam, void* )
{
	ShellMain* pMain = (ShellMain*)pParam;
	ShellMain::AsyncCreateLevel( pMain );
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain ctor
//!
//------------------------------------------------------------------------------------------
ShellMain::ShellMain() :
	m_currState( SS_STARTUP ),
	m_pCurrLevel( 0 ),
	m_pUpdater( 0 ),
	m_levelLoadRequested( false ),
	m_levelUnloadRequested( false ),
	m_gameExitRequested( false )
{
	for (int i = 0; i < PM_MAX; i++)
	{
		m_pauseReq[i] = false;
		m_pauseReqValues[i] = false;
		m_pauseValue[i] = false;
	}

	// start processing before we hit the first update loop
	UpdateState();
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain dtor
//!
//------------------------------------------------------------------------------------------
ShellMain::~ShellMain()
{
	if (m_pUpdater)
	{
		NT_DELETE( m_pUpdater );
	}

#ifdef PLATFORM_PS3
	// sync here so rendering is finished before we quit
	// has to be here as our updater may need to render some more before quitting.
	GcKernel::SyncPreviousFrame();
#endif

	// do we need to unregister our debug keys?

	// do synchronous shutdown for time being

	// SS_SHUTDOWN_LEVEL
	if (m_pCurrLevel)
	{
		if (m_currState == SS_CREATE_LEVEL)
		{
			// TBD handle cleanup of interuppeted load

			// can we kill the loader thread or change its priority here?

			ntError_p(0,("Need to write ASYNC load interupt handling"));
			m_pCurrLevel->DestroyLevel( true );
		}
		else
		{
			m_currState = SS_SHUTDOWN_LEVEL;
			m_pCurrLevel->DestroyLevel( false );
		}

		NT_DELETE( m_pCurrLevel );
		m_pCurrLevel = 0;
	}

	// SS_SHUTDOWN_GAME
	m_currState = SS_SHUTDOWN_GAME;

	// interupted game startup should be fine
	ShellGame::Cleanup();
	ShellGlobal::Cleanup();

	// SS_EXIT
	m_currState = SS_EXIT;

	m_pDebugShell->ReleaseLogs();
	NT_DELETE( m_pDebugShell );
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::GetCurrLoadingLevel
//!	Retrieve the current loading level
//!
//------------------------------------------------------------------------------------------
ShellLevel*	ShellMain::GetCurrLoadingLevel()
{
	ntError_p( m_currState == SS_CREATE_LEVEL, ("Only valid to call this function during level creation") );
	ntError_p( m_pCurrLevel, ("Must have valid level by now") );
	return m_pCurrLevel;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::GetCurrRunningLevel
//!	Retrieve the current loaded level
//!
//------------------------------------------------------------------------------------------
ShellLevel*	ShellMain::GetCurrRunningLevel()
{
	ntError_p( HaveLoadedLevel(), ("Only valid to call this function when the level is running") );
	ntError_p( m_pCurrLevel, ("Must have valid level by now") );
	return m_pCurrLevel;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::GetCurrRunningLevel
//!	Retrieve the current loaded level
//!
//------------------------------------------------------------------------------------------
void	ShellMain::RequestGameExit()
{
	m_gameExitRequested = true;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::RequestLevelLoad
//!	Add a defered request to load a level at the next oppertunity
//!
//------------------------------------------------------------------------------------------
void	ShellMain::RequestLevelLoad( const char* pNextLevel, int iChapterNumber, int iCheckpointID )
{
	ntError_p( (m_currState == SS_RUNNING_EMPTY) || HaveLoadedLevel(), ("Invalid time to request a level load") );
	ntError_p( m_levelLoadRequested == false, ("Alread have a level load requested") );

	m_levelLoadRequested = true;
	m_nextLevel = pNextLevel;
	m_nextLevel_ChapterID = iChapterNumber;
	m_nextLevel_CheckpointID = iCheckpointID;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::RequestLevelUnload
//!	Add a defered request to unload a level at the next oppertunity
//!
//------------------------------------------------------------------------------------------
void	ShellMain::RequestLevelUnload()
{
	ntError_p( (m_currState == SS_RUNNING_LEVEL), ("Invalid time to request a level unload") );
	ntError_p( m_levelUnloadRequested == false, ("Alread have a level load requested") );

	m_levelUnloadRequested = true;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::RequestPauseToggle
//!	Add a defered request to pause / unpause a level at the next oppertunity
//!
//------------------------------------------------------------------------------------------
void	ShellMain::RequestPauseToggle( PAUSE_MODE type, bool bPaused )
{
	m_pauseReq[ type ] = true;
	m_pauseReqValues[ type ] = bPaused;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::Update
//!	Main update loop of the game
//!
//------------------------------------------------------------------------------------------
bool	ShellMain::Update()
{
	// Check for exit requests and quit if required
	if (m_gameExitRequested)
		return false;

	// Check for pause requests
	for (int i = 0; i < PM_MAX; i++)
	{
		if (m_pauseReq[i])
		{
			m_pauseValue[i] = m_pauseReqValues[i];
			m_pauseReq[i] = false;
		}
	}

	// Check for debug pause overides
#ifndef _GOLD_MASTER
	bool pauseOld[PM_MAX];
	for (int i = 0; i < PM_MAX; i++)
	{
		pauseOld[i] = m_pauseValue[i];
		
		if (m_pDebugShell->AllowSingleStep())
			m_pauseValue[i] = false;
	}
#endif
	
	// now update the game as required
	UpdateState();

	// restore actual pause modes from what they were
#ifndef _GOLD_MASTER
	for (int i = 0; i < PM_MAX; i++)
		m_pauseValue[i] = pauseOld[i];
#endif

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::StartState_CreateGame
//!	Goto the create game state
//!
//------------------------------------------------------------------------------------------
void ShellMain::StartState_CreateGame()
{
	m_currState = SS_CREATE_GAME;
	m_pUpdater = NT_NEW SimpleShellUpdater( "gui/tgs_menu/front_page_colour_npow2_nomip.dds" );

	Exec::WaitUntilFunctionTasksComplete();
	AtomicSetPlatform( &ShellAsync::s_TaskComplete, 0 );

#ifdef USE_ASYNC_STARTUP
	Exec::RunAsyncFunction( &AsyncCreateGameTask, this, 0 );
#else
	AsyncCreateGameTask(this,0);
#endif	
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::MoveToNextState
//!	Handle transition to next state
//!
//------------------------------------------------------------------------------------------
void ShellMain::MoveToNextState()
{
	switch (m_currState)
	{
	case SS_STARTUP:
		{
			#ifdef PLATFORM_PS3
			m_currState = SS_CHECK_GAME_DATA;
			m_pUpdater = NT_NEW GameDataShellUpdater();
			#else
			m_currState = SS_LOAD_PLAYER_OPTIONS;
			m_pUpdater = NT_NEW PlayerOptionsShellUpdater();
			#endif
		}
		break;

	case SS_CHECK_GAME_DATA:
		{
			// cleanup old state
			//-------------------------
			NT_DELETE( m_pUpdater );

			#ifdef PLATFORM_PS3
			// can now initialise the game data
			FileManager::GameDataInit();

			if ( g_ShellOptions->m_bUsingHDD == true )
			{
				// If we're running from HDD, we need to perform a pre-install of certain
				// files to the game-data partition. This is because things like shaders
				// (for example) need to be present before we install our wads.
				Wad::PostGameDataInstall();

				// can set up logs now if we're on hdd
				m_pDebugShell->SetupLogging();
				Debug::InitGlobalLog();
			}
			#endif

			m_currState = SS_LOAD_PLAYER_OPTIONS;
			m_pUpdater = NT_NEW PlayerOptionsShellUpdater();
		}
		break;

	case SS_LOAD_PLAYER_OPTIONS:
		{
			// cleanup old state
			//-------------------------
			NT_DELETE( m_pUpdater );

			m_currState = SS_LOAD_PLAYER_PROGRESSION;
			m_pUpdater = NT_NEW PlayerProgressionShellUpdater();
		}
		break;

	case SS_LOAD_PLAYER_PROGRESSION:
		{
			// cleanup old state
			//-------------------------
			NT_DELETE( m_pUpdater );

			#ifdef PLATFORM_PS3
			m_currState = SS_CHECK_SYS_CACHE;
			m_pUpdater = NT_NEW SysCacheShellUpdater();
			#else
			StartState_CreateGame();
			#endif
		}
		break;

	case SS_CHECK_SYS_CACHE:
		{
			// cleanup old state
			//-------------------------
			NT_DELETE( m_pUpdater );

			#ifdef PLATFORM_PS3
			// can now initialise the system cache
			FileManager::SysCacheInit();

			if ( Wad::HaveCompleteGUIGameData() == false )
			{
				m_currState = SS_INSTALL_FE;
				m_pUpdater = NT_NEW InstallFrontendShellUpdater();
			}
			else
			#endif
			{
				// Load up the global wads, this time from game-data - they will
				// replace the wads loaded from blu-ray. No need to install them
				// as they are apparently already present in game-data.
				Wad::LoadGlobalWADs( DEFAULT_MEDIA );

				StartState_CreateGame();
			}
		}
		break;

	case SS_INSTALL_FE:
		{
			// cleanup old state
			//-------------------------
			NT_DELETE( m_pUpdater );

			// Load up the global wads, this time from game-data - they will
			// replace the wads loaded from blu-ray. We should have already
			// installed them to game-data by now.
			ntError_p( Wad::HaveCompleteGUIGameData(), ("We should have finished installing the global WADs by now.") );
			Wad::LoadGlobalWADs( DEFAULT_MEDIA );

			StartState_CreateGame();
		}
		break;

	case SS_CREATE_GAME:
		{
			NT_DELETE( m_pUpdater );
			m_pUpdater = 0;

			#ifdef PLATFORM_PS3
			if ( Wad::HaveCompleteGlobalGameData() ==  false )
			{
				m_currState = SS_INSTALL_GLOBALS;
				m_pUpdater = NT_NEW InstallGlobalsShellUpdater();
			}
			else
			#endif
			{
				// with the gui turned off, this will load the start level in our game config
				// with a proper front end, we remain on SS_RUNNING_EMPTY till purgatory is loaded
				m_currState = SS_RUNNING_EMPTY;
				CGuiManager::LoadDefaultGameLevel();
			}
		}
		break;

	case SS_INSTALL_GLOBALS:
		{
			// cleanup old state
			//-------------------------
			NT_DELETE( m_pUpdater );

			// with the gui turned off, this will load the start level in our game config
			// with a proper front end, we remain on SS_RUNNING_EMPTY till purgatory is loaded

			m_currState = SS_RUNNING_EMPTY;
			CGuiManager::LoadDefaultGameLevel();
		}
		break;

	case SS_RUNNING_EMPTY:
		{
			ntError_p( m_levelLoadRequested, ("Level load must have been requested already") );
			ntError_p( m_pCurrLevel == NULL, ("Must not already have a level here") );

			m_currState = SS_CREATE_LEVEL;
			m_pUpdater = NT_NEW CreateLevelUpdater();
			m_levelLoadRequested = false;

			m_pCurrLevel = NT_NEW ShellLevel(	m_nextLevel.c_str(), 
												m_nextLevel_ChapterID,
												m_nextLevel_CheckpointID );

			// must do some setup on the main thread
			m_pCurrLevel->SyncPreInstall();

			Exec::WaitUntilFunctionTasksComplete();
			AtomicSetPlatform( &ShellAsync::s_TaskComplete, 0 );

			// Avoid large timestep from level destruction causing problems for CreateLevelUpdater render
			CTimer::Get().Update();

			#ifdef USE_ASYNC_STARTUP
			Exec::RunAsyncFunction( &AsyncCreateLevelTask, this, 0 );
			#else
			AsyncCreateLevelTask( this, 0 );
			#endif
		}
		break;

	case SS_CREATE_LEVEL:
		{
			m_currState = SS_RUNNING_LEVEL;
			NT_DELETE( m_pUpdater );
			m_pUpdater = 0;

			// do final main thread fixup work
			// note, is important that SS_RUNNING_LEVEL is true now
			m_pCurrLevel->SyncPostInstall();
		}
		break;

	case SS_RUNNING_LEVEL:
		{
			m_currState = SS_SHUTDOWN_LEVEL;

			// synchronous shutdown for time being, may end up async if required
			m_pCurrLevel->DestroyLevel( false );
			NT_DELETE( m_pCurrLevel );
			m_pCurrLevel = 0;

			m_currState = SS_RUNNING_EMPTY;
			m_levelUnloadRequested = false;
		}
		break;

	case SS_SHUTDOWN_LEVEL:
	case SS_SHUTDOWN_GAME:
	case SS_EXIT:
		ntError_p( 0, ("Must never enter our main update loop in these states") );
		break;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	ShellMain::MoveToNextState
//!	Handle transition to next state
//!
//------------------------------------------------------------------------------------------
void ShellMain::UpdateState()
{
	switch (m_currState)
	{
	case SS_STARTUP:
		{
			// construct global shell objects
			m_pDebugShell = NT_NEW ShellDebug;
			ShellGlobal::SyncInstall( this );
			
			MoveToNextState();
		}
		break;

	case SS_CHECK_GAME_DATA:
		{
			ntError_p( m_pUpdater, ("Must have a valid updater object here") );
			ntError_p( m_pUpdater->GetType() == SS_CHECK_GAME_DATA, ("Must have a valid updater object here") );

			m_pUpdater->Update();
			m_pUpdater->Render();

			GameDataShellUpdater* pUpdater = (GameDataShellUpdater*)m_pUpdater;
			if ( pUpdater->Successful() )
			{
				MoveToNextState();
			}
			else if ( pUpdater->Failed() )
			{
				// we cant quit, the updater should be displaying the appropriate error
				// message and asking the user to quit via the PS button, so we wait for 
				// that to happen.
			}
		}
		break;

	case SS_LOAD_PLAYER_OPTIONS:
		{
			ntError_p( m_pUpdater, ("Must have a valid updater object here") );
			ntError_p( m_pUpdater->GetType() == SS_LOAD_PLAYER_OPTIONS, ("Must have a valid updater object here") );

			m_pUpdater->Update();
			m_pUpdater->Render();

			PlayerOptionsShellUpdater* pUpdater = (PlayerOptionsShellUpdater*)m_pUpdater;
			if ( pUpdater->Successful() )
			{
				MoveToNextState();
			}
			else if ( pUpdater->Failed() )
			{
				// we cant quit, the updater should be displaying the appropriate error
				// message and asking the user to quit via the PS button, so we wait for 
				// that to happen.
			}
		}
		break;

	case SS_LOAD_PLAYER_PROGRESSION:
		{
			ntError_p( m_pUpdater, ("Must have a valid updater object here") );
			ntError_p( m_pUpdater->GetType() == SS_LOAD_PLAYER_PROGRESSION, ("Must have a valid updater object here") );

			m_pUpdater->Update();
			m_pUpdater->Render();

			PlayerProgressionShellUpdater* pUpdater = (PlayerProgressionShellUpdater*)m_pUpdater;
			if ( pUpdater->Successful() )
			{
				MoveToNextState();
			}
			else if ( pUpdater->Failed() )
			{
				// we cant quit, the updater should be displaying the appropriate error
				// message and asking the user to quit via the PS button, so we wait for 
				// that to happen.
			}
		}
		break;

	case SS_CHECK_SYS_CACHE:
		{
			ntError_p( m_pUpdater, ("Must have a valid updater object here") );
			ntError_p( m_pUpdater->GetType() == SS_CHECK_SYS_CACHE, ("Must have a valid updater object here") );

			m_pUpdater->Update();
			m_pUpdater->Render();

			SysCacheShellUpdater* pUpdater = (SysCacheShellUpdater*)m_pUpdater;
			if ( pUpdater->Successful() )
			{
				MoveToNextState();
			}
			else if ( pUpdater->Failed() )
			{
				// we cant quit, the updater should be displaying the appropriate error
				// message and asking the user to quit via the PS button, so we wait for 
				// that to happen.
			}
		}
		break;

	case SS_INSTALL_FE:
		{
			ntError_p( m_pUpdater, ("Must have a valid updater object here") );
			ntError_p( m_pUpdater->GetType() == SS_INSTALL_FE, ("Must have a valid updater object here") );

			m_pUpdater->Update();
			m_pUpdater->Render();

			InstallFrontendShellUpdater* pUpdater = (InstallFrontendShellUpdater*)m_pUpdater;
			if ( pUpdater->Successful() )
				MoveToNextState();
		}
		break;

	case SS_CREATE_GAME:
		{
			ntError_p( m_pUpdater, ("Must have a valid updater object here") );
			ntError_p( m_pUpdater->GetType() == SS_CREATE_GAME, ("Must have a valid updater object here") );

			m_pUpdater->Update();
			m_pUpdater->Render();

			// wait for async function to finish
			if (ShellAsync::s_TaskComplete)
				MoveToNextState();
		}
		break;

	case SS_INSTALL_GLOBALS:
		{
			ntError_p( m_pUpdater, ("Must have a valid updater object here") );
			ntError_p( m_pUpdater->GetType() == SS_INSTALL_GLOBALS, ("Must have a valid updater object here") );

			m_pUpdater->Update();
			m_pUpdater->Render();

			InstallGlobalsShellUpdater* pUpdater = (InstallGlobalsShellUpdater*)m_pUpdater;
			if ( pUpdater->Successful() )
				MoveToNextState();
		}
		break;

	case SS_RUNNING_EMPTY:
		{
			m_pDebugShell->UpdateGlobal();
			ShellGlobal::Update();

			CGatso::Start( "ShellMain::Update" );
			ShellGame::Update();
			CGatso::Stop( "ShellMain::Update" );

			m_pDebugShell->Render();

			CGatso::Start( "ShellMain::Render" );
			ShellGame::RenderSimple();
			CGatso::Stop( "ShellMain::Render" );

			if (m_levelLoadRequested)
				MoveToNextState();
		}
		break;

	case SS_CREATE_LEVEL:
		{
			ntError_p( m_pUpdater, ("Must have a valid updater object here") );
			ntError_p( m_pUpdater->GetType() == SS_CREATE_LEVEL, ("Must have a valid updater object here") );

			m_pUpdater->Update();
			m_pUpdater->Render();

			// wait for async function to finish
			if (ShellAsync::s_TaskComplete)
				MoveToNextState();
		}
		break;

	case SS_RUNNING_LEVEL:
		{
			m_pDebugShell->UpdateGlobal();
			m_pDebugShell->UpdateLevel();

			ShellGlobal::Update();

			CGatso::Start( "ShellMain::Update" );
			ShellGame::Update();
			m_pCurrLevel->Update();
			CGatso::Stop( "ShellMain::Update" );

			m_pDebugShell->Render();

			CGatso::Start( "ShellMain::Render" );
			ShellGame::RenderFull();
			CGatso::Stop( "ShellMain::Render" );

			if (m_levelLoadRequested || m_levelUnloadRequested)
				MoveToNextState();
		}
		break;

	case SS_SHUTDOWN_LEVEL:
	case SS_SHUTDOWN_GAME:
	case SS_EXIT:
		ntError_p( 0, ("Must never enter our main update loop in these states") );
		break;
	}	
}
