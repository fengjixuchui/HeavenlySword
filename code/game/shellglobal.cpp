//------------------------------------------------------------------------------------------
//!
//!	\file shellglobal.cpp
//! Object that handles creation of minimal resources required to run a game
//!
//------------------------------------------------------------------------------------------

#include "game/shellconfig.h"
#include "game/shellglobal.h"
#include "game/shellmain.h"
#include "game/shelllevel.h"

#ifdef PLATFORM_PS3
#	include "exec/PPU/elfmanager.h"
#	include "core/wad.h"
#endif

#include "game/entityanimcontainer.h"
#include "game/randmanager.h"
#include "game/capturesystem.h"
#include "game/luamem.h"
#include "game/luaglobal.h"
#include "game/attackdebugger.h"
#include "game/command.h"
#include "game/keybinder.h"
#include "game/superstylesafety.h"
#include "game/messagehandler.h"
#include "game/playeroptions.h"

#include "core/timer.h"
#include "core/gatso.h"
#include "core/keystring_util.h"

#include "hud/hudmanager.h"

#include "exec/exec.h"
#include "anim/animloader.h"
#include "blendshapes/anim/BSAnimcontainer.h"

#include "gfx/graphicsdevice.h"
#include "gfx/surfacemanager.h"
#include "gfx/display.h"
#include "gfx/renderer.h"
#include "gfx/shader.h"
#include "gfx/rendercontext.h"
#include "gfx/texturemanager.h"
#include "gfx/sector.h"
#include "gfx/levelofdetail.h"
#include "gfx/clump.h"
#include "gfx/fxmaterial.h"

#include "input/inputhardware.h"
#include "input/mouse.h"

#include "physics/dynamicsallocator.h"
#include "physics/verletmanager.h"
#include "physics/physicsmaterial.h"
#include "physics/advancedcharactercontroller.h"

#include "objectdatabase/dataobject.h"
#include "objectdatabase/neteditinterface.h"
#include "objectdatabase/guidclient.h"

#include "effect/effect_manager.h"
#include "hair/effectchain.h"

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::SyncInstall
//! must be done on main thread before we can enter a basic render loop
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::SyncInstall( ShellMain* pParent )
{
	PlatformPreInit( pParent );

	// ------------------------------------------------------------------------------
	// Exec's ready go
	// ------------------------------------------------------------------------------
	Exec::Init();

#	ifdef PLATFORM_PS3
		NT_NEW ElfManager();
#	endif

#	ifdef PLATFORM_PS3
		// We need to have certain things installed to continue, so we load
		// up the global WADs from blu-ray without copying them anywhere. Later
		// on they'll be copied to game-data as part of the install and then
		// we'll use those instead.
		Wad::LoadGlobalWADs( BLU_RAY_MEDIA );
#	endif

	NT_NEW CAnimLoader;
	NT_NEW AnimContainerManager;
	NT_NEW BSAnimContainerManager;

	NT_NEW GraphicsDevice;
	NT_NEW SurfaceManager;

	NT_NEW DisplayManager;

	PlatformGraphicsInit();

	NT_NEW CInputHardware;

	NT_NEW DebugShaderCache;
	NT_NEW TextureManager;
	NT_NEW Renderer();

	RenderingContext::Initialise();

	NT_NEW CTimer;

#	ifdef _PROFILING
	NT_NEW CProfiler;

	// allows us to have an open ended profile capture in our update func
	PROFILER_START;
#	endif
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::AsyncInstall
//! Creation of these can take their time on a second thread
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::AsyncInstall()
{
	NT_NEW ShaderManager();
	NT_NEW CSector;

	if (MouseInput::Exists())
		MouseInput::Get().Initialise();

	NT_NEW RandManager();
	NT_NEW CaptureSystem();

	NT_NEW CLuaMemoryMan;
	NT_NEW CLuaGlobal;
	NT_NEW Physics::CDynamicsAllocator();
	NT_NEW CLODManager();
	NT_NEW CClumpLoader();
	NT_NEW FXMaterialManager();
	NT_NEW EffectManager();
	NT_NEW AttackDebugger(); // Can be removed for any build type if required
	NT_NEW GuidClient();
	NT_NEW ObjectDatabase();
	NT_NEW CNetEditInterface();
	NT_NEW AreaManager();
	NT_NEW CHud();

	NT_NEW MessageGlobals();

	// key binder
	NT_NEW CommandManager();
	NT_NEW KeyBindManager();

	if(g_ShellOptions->m_bUseHair)
	{
		NT_NEW ChainRessource();
	}

	if(Physics::VerletManager::IsEnabled())
	{
		NT_NEW Physics::VerletManager();
	}

	// Initialise our gatso
	CGatso::Initialise();

	// Create a superstyle safety manager
	NT_NEW SuperStyleSafetyManager();

	// finish initialising the sector manager
	CSector::Get().CreateEffects();

	// now load in global soft interfaces
	TryInstallXMLFile( "entities/entlib.xml" ); 
	TryInstallXMLFile( "entities/objects/object_interfaces.xml" );
	TryInstallXMLFile( "data/sound/chatter_interfaces.xml" );

	// this really is global data required by the game
	TryInstallXMLFile( "data/hud/hud.xml" );
	TryInstallXMLFile( "data/levelofdetail.xml" );

	// this is global data that should be level resource specific but requires code refactoring.
	TryInstallXMLFile( "entities/ninjasequence/ninjasequence_buttons.xml" );
	TryInstallXMLFile( "data/effects/ns_effects.xml" );

	// load physics material library
	NT_NEW Physics::PhysicsMaterialTable( "physics/physicsmaterials.xml" );
	NT_NEW Physics::RagdollPerformanceManager();

	//front end, audio hooks
	CLuaGlobal::Get().InstallOptionalFile( "data/sound/frontendmusiccallbacks.lua" );

	PlatformPostInit();
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::Cleanup
//! Removes all singletons.
//! ATTN! make sure you test for singleton existance, installation may have been aborted.
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::Cleanup()
{
	// stuff loaded async
	if (AreaManager::Exists())
	AreaManager::Kill();

	if (RandManager::Exists())
	RandManager::Kill();

	if (CaptureSystem::Exists())
	CaptureSystem::Kill();

	if (CNetEditInterface::Exists())
	CNetEditInterface::Kill();

	if (ObjectDatabase::Exists())
	ObjectDatabase::Kill();

	if (GuidClient::Exists())
	GuidClient::Kill();

	if (AttackDebugger::Exists())
	AttackDebugger::Kill();

	if (EffectManager::Exists())
	EffectManager::Kill();

	if (FXMaterialManager::Exists())
	FXMaterialManager::Kill();

	if (CClumpLoader::Exists())
	CClumpLoader::Kill();

	if (CLODManager::Exists())
	CLODManager::Kill();

	if (Physics::CDynamicsAllocator::Exists())
	Physics::CDynamicsAllocator::Kill();

	if(Physics::VerletManager::Exists())
	Physics::VerletManager::Kill();

	if(ChainRessource::Exists())
	ChainRessource::Kill();

	if (SuperStyleSafetyManager::Exists())
	SuperStyleSafetyManager::Kill();

	if (CLuaGlobal::Exists())
	CLuaGlobal::Kill();

	if (CLuaMemoryMan::Exists())
	CLuaMemoryMan::Kill();

	if (MouseInput::Exists())
	MouseInput::Kill();

	if (Physics::PhysicsMaterialTable::Exists())
	Physics::PhysicsMaterialTable::Kill();

	if (Physics::RagdollPerformanceManager::Exists())
	Physics::RagdollPerformanceManager::Kill();

	if (KeyBindManager::Exists())
	KeyBindManager::Kill();

	if (CommandManager::Exists())
	CommandManager::Kill();

	// stuff loaded sync
	CProfiler::Kill();

	CTimer::Kill();

	CInputHardware::Kill();

	RenderingContext::Destroy();
	CSector::Kill();
	
	MessageGlobals::Kill();	

	if ( CHud::Exists() )
	CHud::Kill();

	if ( CPlayerOptions::Exists() )
	CPlayerOptions::Kill();

	Renderer::Kill();
	DebugShaderCache::Kill();
	ShaderManager::Kill();
	DisplayManager::Kill();
	TextureManager::Kill();

	SurfaceManager::Kill();
	GraphicsDevice::Kill();

	CAnimLoader::Kill();
	AnimContainerManager::Kill();
	BSAnimContainerManager::Kill();

	ClassFactory::DestroyHelpers();

#ifdef PLATFORM_PS3
	ElfManager::Kill();
#endif

	Exec::Shutdown();
	PlatformPostDestroy();
	DestroyGlobalStringTable();
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::Update
//! Update for globals
//!
//------------------------------------------------------------------------------------------
void	ShellGlobal::Update( bool bUpdateExec )
{
	if (bUpdateExec)
	{
		// sync previous frame
		Exec::FrameEnd();
		Exec::FrameReset();
	}

	// capture costs of previous frame
	CGatso::Update();
	PROFILER_STOP;
	PROFILER_DISPLAY;
	PROFILER_START;
	CGatso::Clear();

#ifdef PLATFORM_PS3
	// pump our system callbacks
	int ret = 0;
	ret = cellSysutilCheckCallback();
	ntError_p(ret==0,("Call to cellSysutilCheckCallback failed! (%d)\n",ret));
#endif

	CTimer::Get().Update( ShellMain::Get().IsPausedAny() );

	PlatformUpdate();

	CInputHardware::Get().Update(CTimer::Get().GetSystemTimeChange());

	if (MouseInput::Exists())
		MouseInput::Get().Update();
}
