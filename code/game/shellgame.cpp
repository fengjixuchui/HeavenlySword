//------------------------------------------------------------------------------------------
//!
//!	\file shellgame.cpp
//! Object that handles loading and unloading of game resources
//!
//------------------------------------------------------------------------------------------

#include "game/shellmain.h"
#include "game/shellgame.h"
#include "game/shelllevel.h"
#include "game/shellconfig.h"

#ifdef PLATFORM_PS3
#include "army/armymanager.h"
#include "speedtree/speedtreemanager_ps3.h"
#endif

#include "exec/exec.h"

#include "core/gatso.h"
#include "core/osddisplay.h"
#include "core/debug_hud.h"

#include "gui/guisubtitle.h"

#include "physics/triggervolume.h"
#include "Physics/verletmanager.h"
#include "Physics/projectilelg.h"

#include "lua/ninjalua.h"
#include "game/timerbindings.h"
#include "game/nsmanager.h"
#include "game/credits.h"

#include "game/jumpmenu.h"
#include "game/testmenu.h"
#include "game/luardebug.h"
#include "game/capturesystem.h"
#include "game/checkpointmanager.h"

#include "hud/hudmanager.h"
#include "jamnet/netman.h"

#include "camera/camman.h"
#include "camera/camview.h"

#include "ai/aiattackselection.h"
#include "ai/aibehaviourpool.h"
#include "ai/ainavgraphmanager.h"
#include "ai/aipatrolmanager.h"
#include "ai/aicoverpoint.h"

#include "gfx/pictureinpicture.h"

#include "blendshapes/xpushapeblending.h"
#include "blendshapes/blendshapes_managers.h"
#include "water/watermanager.h"
#include "movies/movieinstance.h"
#include "movies/movieplayer.h"
#include "hair/forcefield.h"

//------------------------------------------------------------------------------------------
//!
//!	ShellGame ctor
//!
//------------------------------------------------------------------------------------------
void ShellGame::AsyncInstall()
{
	//load gameinfo. Chapter/checkpoint data
	TryInstallXMLFile( "data/gameinfo.xml" );

#ifndef _GOLD_MASTER
	// The very first thing we'll do is to start the lua debugger, then it
	// can be connected to debug any lua errors in the following items.
	if(g_ShellOptions->m_bLuaDebugging)
	{
		LuaDebugServer::StartListening(g_ShellOptions->m_bLuaDebuggingAutoLaunch, g_ShellOptions->m_bLuaProfiling);
	}
#endif //_GOLD_MASTER

	// Start On Screen Debugging host
	OSD::Init();

#ifdef PLATFORM_PS3
	NT_NEW ArmyManager();
#endif

	// world hierachy
	CHierarchy::CreateWorld();

	NT_NEW ForceFieldManager();

#ifdef _SPEEDTREE
	if(g_ShellOptions->m_bUseSpeedTree)
		NT_NEW SpeedTreeManager();
#endif

	// blendshapes stuff
	NT_NEW BSClumpManager();
	NT_NEW BSAnimManager();
	NT_NEW XPUShapeBlending( g_ShellOptions->m_iBSVertsPerBatch );
	if ( g_ShellOptions->m_bEnableBlendShapes )
		XPUShapeBlending::Get().Enable();

	// water
	NT_NEW WaterManager();
	if ( g_ShellOptions->m_bEnableWater )
		WaterManager::Get().Enable();

	// AI - Move all this into a single function?
	////////////////////////////////////////////
	NT_NEW CAINavGraphManager();                  //
	NT_NEW AIPatrolManager();					   //
	NT_NEW AIAlertManager();					   //
	NT_NEW AICoverManager();					   //
	//NT_NEW AIAdHocFormationManager();             //
	NT_NEW AIBehaviourPool();			           //
											//
	AICombatComponent::Init();                 //

	// create the Ninja Sequence and Cutscene manager
	NT_NEW NSManager();

	// Set the capture manager mode - MUST COME BEFORE CGameAudioManagerInitialisation
	CaptureSystem::Get().SetCaptureName(g_ShellOptions->m_obCaptureName.c_str());
	CaptureSystem::Get().SetCaptureDir(g_ShellOptions->m_obCaptureDir.c_str());

	// Initialise the sound system
	NT_NEW GameAudioManager ();

	// Initialise the game audio
	GameAudioManager::Get().LoadResources();

	NT_NEW MoviePlayer();
	MoviePlayer::Get().Initialise();

	// Create and initialise the GUI
	NT_NEW CGuiManager;

//	NT_NEW CheckpointManager();
//	CheckpointManager::Get().LoadCheckpointData();

	// not sure this should be installed here?
	TryInstallXMLFile( "entities/characters/hero/hero_combos.xml" );

	NT_NEW CTriggerVolumeManager ();
	NT_NEW Physics::ProjectileManager();

	NT_NEW DebugHUD();

	NT_NEW JumpMenu();
	NT_NEW TestMenu();

	// Create subtitle system
	NT_NEW CSubtitleMan("Body");
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGame::cleanup
//! Removes all singletons.
//! ATTN! make sure you test for singleton existance, installation may have been aborted.
//!
//------------------------------------------------------------------------------------------
void	ShellGame::Cleanup()
{
	if (CheckpointManager::Exists())
	CheckpointManager::Kill();

	if (CSubtitleMan::Exists())
	CSubtitleMan::Kill();

	if (JumpMenu::Exists())
	JumpMenu::Kill();
	
	if (TestMenu::Exists())	
	TestMenu::Kill();

	if (DebugHUD::Exists())	
	DebugHUD::Kill();

#ifdef PLATFORM_PS3
	if (ArmyManager::Exists())	
	ArmyManager::Kill();
#endif

	// Shutdown networking
	NetMan::CleanUp();

#ifndef _GOLD_MASTER
	// And the lua Debugging
	LuaDebugServer::CleanUp();
#endif //_GOLD_MASTER
	
	if (LuaTimerManager::Exists())	
	LuaTimerManager::Kill();

	// Kill the Gui Manager
	if (CGuiManager::Exists())
		CGuiManager::Kill();
	
#ifdef _SPEEDTREE
	if(SpeedTreeManager::Exists())
		SpeedTreeManager::Kill();
#endif

	// destroy force feild manager
	if(ForceFieldManager::Exists())
		ForceFieldManager::Kill();

	if ( MoviePlayer::Exists() )
	{
		MoviePlayer::Get().DeInitialise();
		MoviePlayer::Kill();
	}

	// kill kill mame kill
	if (GameAudioManager::Exists())
		GameAudioManager::Kill();

	// CAIPathFinder::Kill();

	// kill the ai manager object
	if (CAINavGraphManager::Exists())
	CAINavGraphManager::Kill();
	
	if (AIPatrolManager::Exists())
	AIPatrolManager::Kill();

	if (AIAlertManager::Exists())
	AIAlertManager::Kill();

	if (AICoverManager::Exists())
	AICoverManager::Kill();

	if (AIAdHocFormationManager::Exists())
	AIAdHocFormationManager::Kill();

	if (AIBehaviourPool::Exists())
	AIBehaviourPool::Kill();

	AICombatComponent::CleanUp();

	if (NSManager::Exists())
	NSManager::Kill();

	if (XPUShapeBlending::Exists())
	XPUShapeBlending::Kill();

	if (BSClumpManager::Exists())
	BSClumpManager::Kill();

	if (BSAnimManager::Exists())
	BSAnimManager::Kill();

	if (WaterManager::Exists())
	WaterManager::Kill();

	// Kill the world
	if (CHierarchy::GetWorld())
	CHierarchy::DestroyWorld();

	// Must...destroy...CTriggerVolumeManager...
	if (CTriggerVolumeManager::Exists())
	CTriggerVolumeManager::Kill();

	if (Physics::ProjectileManager::Exists())
	Physics::ProjectileManager::Kill();
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGame::Update
//! update anything that does not rely on a level being present
//!
//------------------------------------------------------------------------------------------
void	ShellGame::Update( bool bUpdateGui )
{
	Mem::FrameReset();

	// What is the required audio behavior in the differing pause types?
	if (GameAudioManager::Exists())
		GameAudioManager::Get().Update();

	if(NetMan::IsRunning())
		NetMan::Update();

	// may disable this during async loads
	if (bUpdateGui)
		CGuiManager::Get().Update();

	if (CCredits::Exists() && CCredits::Get().Running())
		CCredits::Get().Update();

	if	(
		(ShellMain::Get().IsPausedByUser() == false) &&
		(ShellMain::Get().IsPausedBySystem() == false)
		)
	{
		MoviePlayer::Get().AdvanceToNextFrame();

		// if the game is paused by code, we still update subtitles but
		// with system time instead of game time
		float fTimeStep = 
			ShellMain::Get().IsPausedByCode() ?
			CTimer::Get().GetSystemTimeChange() :
			CTimer::Get().GetGameTimeChange();

		CSubtitleMan::Get().Update( fTimeStep );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGame::RenderSimple
//! stripped down basic rendering
//!
//------------------------------------------------------------------------------------------
void	ShellGame::RenderSimple()
{
	// must be here so is updated post entity / hierarchy update
	CGatso::Start( "ShellGame::EffectManager.Update()" );
	EffectManager::Get().UpdateManager();
	CGatso::Stop( "ShellGame::EffectManager.Update()" );

	MoviePlayer::Get().Update();

	// Sets up simple colour and z buffer if we're not rendering the full level
	Renderer::Get().m_pPIPManager->RenderBasic();

	EffectManager::Get().RenderLDR();

	CGuiManager::Get().Render();

	if (CCredits::Exists() && CCredits::Get().Running())
		CCredits::Get().Render();

	CSubtitleMan::Get().Render();

	// flip
	CGatso::Start( "ShellGame::Present" );
	Renderer::Get().m_pPIPManager->PresentBasic();
	CGatso::Stop( "ShellGame::Present" );
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::RenderFull
//! Full rendering, including the level
//!
//------------------------------------------------------------------------------------------
void	ShellGame::RenderFull()
{
	// must be here so is updated post entity / hierarchy update
	CGatso::Start( "ShellGame::EffectManager.Update()" );
	EffectManager::Get().UpdateManager();
	CGatso::Stop( "ShellGame::EffectManager.Update()" );

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
	// syncin with SPU task, because flags and hair needs to be completed
	// should only need to sync with hair and flags really, but at the moment
	// 29/03/2006 16:42:21
	// our job manager is very simple and do not allow "fine grained" scheduling
	// BTW, Osiris is very likely to rely on the same sync point here	
	// HAIR SPU CODE RELIES ON THIS SYNC SINCE I REMOVED A PER JOB SYNC IN THE HAIR CODE (Marco)
	CGatso::Start( "ShellGame::SYNC_WITH_SPU_BEFORE_RENDERING" );
	Exec::FrameEnd();
	Exec::FrameReset();
	CGatso::Stop( "ShellGame::SYNC_WITH_SPU_BEFORE_RENDERING"  );

	MoviePlayer::Get().Update();

	if(Physics::VerletManager::Exists())
		Physics::VerletManager::Get().RendererUpdate();

#if !defined(_GOLD_MASTER) && defined(PLATFORM_PS3)
	bool bDumpStats = false;
	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F1 ) )
	{
		bDumpStats = true;
		GC_RESET_METRICS();
	}
#endif

	Renderer::Get().m_pPIPManager->RenderLevel();

	CGuiManager::Get().Render();

	if (CCredits::Exists() && CCredits::Get().Running())
		CCredits::Get().Render();

	CHud::Get().Render( CTimer::Get().GetGameTimeChange() );

	CSubtitleMan::Get().Render();

	CGatso::Start( "ShellGame::Present" );
	Renderer::Get().m_pPIPManager->PresentLevel();
	CGatso::Stop( "ShellGame::Present" );

#if !defined(_GOLD_MASTER) && defined(PLATFORM_PS3)
	if (bDumpStats)
		Renderer::Get().DumpStats();
#endif
}

