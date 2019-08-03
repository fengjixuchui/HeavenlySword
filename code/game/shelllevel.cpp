//------------------------------------------------------------------------------------------
//!
//!	\file shelllevel.cpp
//! Object that handles loading and unloading of level resources
//!
//------------------------------------------------------------------------------------------

#include "game/shelllevel.h"
#include "game/shellmain.h"

#include "core/io.h"
#include "core/gatso.h"
#include "core/profiling.h"

#include "objectdatabase/dataobject.h"
#include "objectdatabase/neteditinterface.h"

#include "gui/guitext.h"

#include "physics/world.h"
#include "physics/physicsloader.h"
#include "physics/triggervolume.h"
#include "Physics/dynamicsallocator.h"
#include "Physics/verletmanager.h"
#include <hkbase/memory/hkMemory.h>

#include "game/entitymanager.h"
#include "game/entitybrowser.h"
#include "game/syncdmovement.h"
#include "game/syncdcombat.h"
#include "game/chatterboxman.h"						// Dario
#include "game/attackdebugger.h"
#include "game/inputcomponent.h"
#include "game/timeofday.h"
#include "game/nsmanager.h"
#include "game/timerbindings.h"
#include "game/keybinder.h"
#include "game/jumpmenu.h"
#include "game/entitycheckpoint.h"
#include "game/combatstyle.h"
#include "game/projectilemanager.h"

#ifdef PLATFORM_PS3
#include "army/armymanager.h"
#include "speedtree/speedtreemanager_ps3.h"
#include "speedtree/speedgrass_ps3.h"
#include "water/watermanager.h"
#include "physics/verletmanager.h"
#include "core/wad.h"
#endif

#include "hud/hudmanager.h"

#include "camera/camman.h"
#include "camera/camview.h"

#include "audio/audiomixer.h"

#include "ai/aiformationmanager.h"
#include "ai/aiinitialreactionman.h"				// Dario
#include "ai/aiuseobjectqueueman.h"					// Dario
#include "ai/aihearingman.h"						// Dario
#include "ai/ainavigationsystem/ainavigsystemman.h" // Dario
#include "ai/aibehaviourpool.h"
#include "ai/ainavgraphmanager.h"
#include "ai/aipatrolmanager.h"
#include "ai/aicoverpoint.h"

#include "effect/effect_resourceman.h"

#include "gfx/materialdebug.h"
#include "gfx/texturemanager.h"
#include "gfx/shadowsystem.h"
#include "blendshapes/xpushapeblending.h"

#include "hair/effectchain.h"
#include "hair/forcefield.h"

#include "area/areasystem.h"
#include "area/areasystem.inl"
#include "area/arearesourcedb.h"


#ifndef _GOLD_MASTER	
#include "game/capturesystem.h"			// currently only for game exit screen capture on autotest
#endif

//------------------------------------------------------------------------------------------
//!
//!	TryInstallXMLFile
//!
//------------------------------------------------------------------------------------------
bool TryInstallXMLFile( const char* pcFile )
{
	static char acFileName[MAX_PATH];
	Util::GetFiosFilePath( pcFile, acFileName );

	if ( File::Exists( acFileName ) )
	{
		// Tell people what we're up to
		ntPrintf("XML loading \'%s\'\n", pcFile);

		// Open the XML file in memory
		FileBuffer obFile( acFileName, true );

		if ( !ObjectDatabase::Get().LoadDataObject( &obFile, pcFile ) )
		{
			ntError_p( false, ( "Failed to parse XML file '%s'", pcFile ) );
			return false;
		}

		return true;
	}

	ntPrintf("WARNING! Could not find XML in '%s'...\n", pcFile );
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellLevel ctor
//!
//------------------------------------------------------------------------------------------
ShellLevel::ShellLevel( const char* pLevelName,
						int levelNumber,
						int startCheckpoint ) :

	m_LevelName( pLevelName ),
	m_levelNumber( levelNumber ),
	m_startCheckpointID( startCheckpoint ),
	m_currentRunLastCheckpointID( 0 ),
	m_bFirstFrame(true)
{
	ntError_p( pLevelName, ("MUST have a valid level name here") );
}

//------------------------------------------------------------------------------------------
//!
//!	ShellLevel ctor
//!
//------------------------------------------------------------------------------------------
ShellLevel::~ShellLevel()
{

}

//------------------------------------------------------------------------------------------
//!
//!	ShellLevel::SyncInstall
//! Level setup that must be run on the main thread
//!
//------------------------------------------------------------------------------------------
void	ShellLevel::SyncPreInstall()
{
	// must be one on main thread as will allocate a thread memory object required for running
	// of havok
	ntAssert( !Physics::CPhysicsWorld::Exists() ) ;
	NT_NEW Physics::CPhysicsWorld();
	NT_NEW Physics::CPhysicsLoader();

	// Update the strings file and font
	// must be done on main thread as accesses and changes the GuiResources singleton, as
	// does the main gui manager update function
	CStringManager::Get().LevelChange( GetLevelName() );
}

//------------------------------------------------------------------------------------------
//!
//!	ShellLevel::SyncPostInstall
//! Level setup that must be run on the main thread
//!
//------------------------------------------------------------------------------------------
void	ShellLevel::SyncPostInstall()
{
	// Call LUA OnLevelStart Function
	Event_OnLevelStart();

#if !defined(_RELEASE)
	// Load in the user.lua now all the systems have started
	CLuaGlobal::Get().InstallOptionalFile("scripts/user.lua");
#endif

	// build the resources for any effects that have been created during level load
	EffectResourceMan::Get().LoadResources();

	// Need to redo the GUI camera transform now, or it will be invalid
	if (CGuiManager::Exists())
		CGuiManager::Get().UpdateTransform();

	// Attempt to start at the correct checkpoint
	StartAtCheckpoint( m_startCheckpointID );
}

//------------------------------------------------------------------------------------------
//!
//!	ShellLevel::SetupLevel
//! Create all the singletons required by a level, load in our level data
//!
//------------------------------------------------------------------------------------------
void	ShellLevel::AsyncInstall()
{
	LOAD_PROFILE( ShellLevel_AsyncInstall )

#	ifdef PLATFORM_PS3
	{
		Wad::InstallAndLoadLevelWADs( m_LevelName.c_str() );
	}
#	endif

#ifdef USE_ASYNC_STARTUP
	// we have to create some local thread memory for this thread, so havok 
	// memory allocation works
	hkThreadMemory* g_LocalHKMem;
	char* g_LocalHKStack;
	Physics::CPhysicsWorld::Get().CreateLocalThreadMem( &g_LocalHKMem, &g_LocalHKStack );
#endif

	// create a container for this level
	ObjectDatabase::Get().SetCurrentContainer( ObjectDatabase::Get().AddContainer( GetLevelName() ) );

	// Create entity dependant singletons
	Physics::CPhysicsWorld::Get().SetupCollisionFilter();

	ntAssert( !CEntityManager::Exists() ) ;
	NT_NEW	CEntityManager();

	ntAssert( !ProjectileManager::Exists() ) ;
	NT_NEW	ProjectileManager();

	ntAssert( !CEntityBrowser::Exists() ) ;
	NT_NEW CEntityBrowser();

	ntAssert(!CamMan::Exists());
	ntAssert(!CamSceneElementList::Exists());
	NT_NEW CamMan();
	NT_NEW CamSceneElementList();

	ntAssert(!AIFormationManager::Exists());
	NT_NEW AIFormationManager();

	ntAssert(!SyncdCombat::Exists());
	NT_NEW SyncdCombat();
	
	ntAssert(!SyncdMovement::Exists());
	NT_NEW SyncdMovement();

	// Creating the CChatterBox Singleton and register keybinding (Dario)
	ntAssert(!CChatterBoxMan::Exists());
	NT_NEW CChatterBoxMan();			

	// Creating the new Navigation SYSTEM Manager (Dario)
	ntAssert(!CAINavigationSystemMan::Exists());
	NT_NEW CAINavigationSystemMan();

	// Creating the AI Hearign System manager
	ntAssert(!CAIHearingMan::Exists());
	NT_NEW CAIHearingMan();

	// Creating the CAIInitialReactionMan Singleton and register keybinding (Dario)
	ntAssert(!CAIInitialReactionMan::Exists());
	NT_NEW CAIInitialReactionMan();			
	
	// Creating the CAIQueueManager Singleton and register keybinding (Dario)
	ntAssert(!CAIQueueManager::Exists());
	NT_NEW CAIQueueManager();
	
	// Reset the attack debugger
	AttackDebugger::Get().Reset();

	// reset our input hokey input swtiching
	CInputComponent::Reset();

	// signal were about to load the level
	AreaManager::Get().SignalLoadingLevel();

	// load our level specific files
	//--------------------------------------------
	InstallLevelFiles();

	// Initialise the Camera Manager - Require a player for proper construction of the debug camera.
	CamMan::Get().Init();

	// activate the area system
	AreaManager::Get().ActivateLevelFromCheckpoint( m_startCheckpointID );

	// load the global lighting data from lighting.xml
	// note from now on global lighting is loaded when the level is loaded

	// Sort out the time of day too
	if (TimeOfDay::Exists())
		TimeOfDay::Get().PostSerialisation();

	// dump out recorded material usage
	MaterialDebug::DumpMaterialUsage();

	// Configure HUD
	CHud::Get().Initialise( );

#ifdef USE_ASYNC_STARTUP
	// free up local thread
	Physics::CPhysicsWorld::Get().FreeLocalThreadMem( &g_LocalHKMem, &g_LocalHKStack );
#endif

	LOAD_PROFILE_END( ShellLevel_AsyncInstall )
	LOAD_PROFILE_OUTPUT_RESULTS

	ntPrintf( "[[LEVEL_LOAD_SUCCESSFUL]]\n" ); // picked up by automated tests [scee_st]
}

//------------------------------------------------------------------------------------------
//!
//!	ShellLevel::DestroyLevel
//! Cleanup all data and objects created by level
//!
//------------------------------------------------------------------------------------------
void	ShellLevel::DestroyLevel( bool bAbortedLoad )
{
	if (bAbortedLoad == true)
	{
		ntError_p( 0, ("Need to write async load abort code") );
	};

	Event_OnLevelEnd();

#ifdef PLATFORM_PS3
	// Reset the army manager
	if ( ArmyManager::Exists() )
	{
		ArmyManager::Get().Deactivate();
	}
#endif

	// Reset the area system
	AreaManager::Get().Reset();

	XPUShapeBlending::Get().Reset();

	// Reset the ninja sequence manager
	NSManager::Get().Reset();

	// Reset triggers		
	CTriggerVolumeManager::Get().Reset();

	// reset if we're editing
	CNetEditInterface::Get().Reset();

	// clean out our effects if we have any
	EffectManager::Get().Reset();

	// Need to remove GUI camera transform now, or it will be invalid
	if (CGuiManager::Exists())
		CGuiManager::Get().RemoveTransform();

	// clean up any singletons that rely on entities
	CamMan::Kill();
	CamSceneElementList::Kill();
	LuaTimerManager::Get().ClearTimers();

	if (CHud::Exists())
		CHud::Get().Reset();

	// Ensure no transforms are parented to anything
	CEntityManager::Get().UnparentAllHierarchies();
	
	// Clear the target entity pointer thats used in lua global, very shortly it will no longer be valid!
	CLuaGlobal::Get().SetTarg(0);
	
	ntAssert(AIBehaviourPool::Exists());
	AIBehaviourPool::Get().LevelUnload();

	// free the level container
	ObjectDatabase::Get().DeleteContainer( ObjectDatabase::Get().GetPointerFromName<ObjectContainer*>( GetLevelName() ) );

	// Hack to remove global scope forward references, really we should just remove
	// the references to XML objects within the above container.
	ObjectDatabase::Get().CleanForwardReferences();

	// now we should clean level resources as entities have released their pointers
	AreaResourceDB::Get().UnloadDatabase();

#if defined( PLATFORM_PS3 ) && defined( _SPEEDTREE )
	if ( g_ShellOptions->m_bUseSpeedTree )
	{
		SpeedTreeManager::Get().OnLevelUnload();
	}
#endif

	// destroy formation manager after ents released
	AIFormationManager::Kill();

	ntAssert( CAINavGraphManager::Exists() );
	CAINavGraphManager::Get().LevelUnload();

	ntAssert(AIAlertManager::Exists());
	AIAlertManager::Get().LevelUnload();

	ntAssert(AICoverManager::Exists());
	AICoverManager::Get().LevelUnload();

	// get rid of the projectile manager
	ProjectileManager::Kill();

	// Destroy our entity / entity-template managers
	CEntityBrowser::Kill();
	CEntityManager::Kill();

	// Kill the sync'd movement manager
	SyncdMovement::Kill();

	// Kill the sysnced combat manager
	SyncdCombat::Kill();

	// kill the havok world
	Physics::CPhysicsLoader::Kill();
	Physics::CPhysicsWorld::Kill();
	Physics::CDynamicsAllocator::Get().FreeAllocations();

	// Kill the ChatterBox Manager & the Initial Reaction Manager (Dario)
	CChatterBoxMan::Kill();			
	CAINavigationSystemMan::Kill(); // (Dario)
	CAIHearingMan::Kill();			// (Dario)
	CAIInitialReactionMan::Kill();
	CAIQueueManager::Kill();

	if (Physics::VerletManager::Exists())
		Physics::VerletManager::Get().Reset();

	TextureManager::Get().Clear();

	// reset the shadow controller state
	CShadowSystemController::Get().LevelReset();

	// Remove any keybindings that were registered with the previous level
	KeyBindManager::Get().DestroyNonGlobals();

	// Remove any old commands that were registered with the previous level
	CommandManager::Get().DestroyNonGlobals();

	if (ChainRessource::Exists())
		ChainRessource::Get().LevelReset();

	// Unload all FMOD project files
	AudioSystem::Get().UnloadAllProjects();
}

//------------------------------------------------------------------------------------------
//!
//!	ShellLevel::InstallLevelFiles
//! Search for level specific files and install them
//!
//------------------------------------------------------------------------------------------
void	ShellLevel::InstallLevelFiles()
{
	LOAD_PROFILE( ShellLevel_InstallLevelFiles )

	Physics::CPhysicsWorld::Get().Start_AddingPhysicsObjects();

	#ifdef PLATFORM_PS3
	//-----------------------------------------
	// process custom level shader dictionary
	{
		LOAD_PROFILE( LoadShaderDictionary )

		char pNeutralDict[ MAX_PATH ];
		snprintf( pNeutralDict, MAX_PATH, "levels/%s.dict", GetLevelName() );

		char pPlatformDict[ MAX_PATH ];
		Util::GetFiosFilePath( pNeutralDict, pPlatformDict );

		if( File::Exists( pPlatformDict ) )
			ShaderManager::Get().LoadDictionary( pPlatformDict );
		else
			user_warn_p(0, ("A custom shader dictionary for this level is not present!!!!") );
	}
	#endif

	//--------------------------------
	// Install area manifests and prime the clump cache.
	ntError_p( AreaResourceDB::Exists(), ("Must have AreaResourceDB here.") );
	AreaResourceDB::Get().PreLevelLoadInit( GetLevelName() );

	//--------------------------------
	// process lua manifest file
	{
		LOAD_PROFILE( ProcessLuaManifestFile )

		char acManifestName[ MAX_PATH ];
		snprintf( acManifestName, MAX_PATH, "levels/%s.man", GetLevelName() );

		CLuaGlobal::Get().InstallOptionalFile( acManifestName );
	}

	//--------------------------------
	// Now load entity instances for the level
	{
		LOAD_PROFILE( LoadEntityInstances )

		char acLevelFile[ MAX_PATH ];
		snprintf( acLevelFile, MAX_PATH, "levels/%s.xml", GetLevelName() );

		TryInstallXMLFile( acLevelFile );
	}

	// finish off any area resources that rely on serialised data
	AreaResourceDB::Get().PostLevelLoadInit();

	// make sure level start and end function are removed incase the user hasn't set one
	NinjaLua::LuaObject funcStart = CLuaGlobal::Get().State().GetGlobals()["OnLevelStart"];
	NinjaLua::LuaObject funcEnd = CLuaGlobal::Get().State().GetGlobals()["OnLevelEnd"];

	funcStart.AssignNil(CLuaGlobal::Get().State());
	funcEnd.AssignNil(CLuaGlobal::Get().State());

	//--------------------------------
	// Now load and process a lua file, to see if we have any level specific things to do
	// this should have a table called Level with a C
	{
		LOAD_PROFILE( ProcessLuaLevelFile )

		char acLevelLuaFile[ MAX_PATH ];
		snprintf( acLevelLuaFile, MAX_PATH, "levels/%s.lua", GetLevelName() );

		CLuaGlobal::Get().InstallOptionalFile( acLevelLuaFile );
	}

	//--------------------------------
	// Now load lighting, if it exists
	{
		LOAD_PROFILE( ProcessLightingFile )

		char acLevelFile[ MAX_PATH ];
		snprintf( acLevelFile, MAX_PATH, "levels/%s_lighting.xml", GetLevelName() );

		bool levelLightFileOk = TryInstallXMLFile( acLevelFile );
		if( levelLightFileOk )
		{
			LevelLightingCtrl::SwitchLightingFile( acLevelFile );
		} else
		{
			LevelLightingCtrl::LoadLevelLightingDef( "data\\default_lighting.xml" );
		}
	}

	JumpMenu::Get().Load( GetLevelName() );
	Physics::CPhysicsWorld::Get().End_AddingPhysicsObjects();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	ShellLevel::Event_OnLevelStart
//! Call a lua function to indicate the level has started.
//! JML - Addition 13-12-05 - Level Event Functions
//!                                                                                         
//------------------------------------------------------------------------------------------
void	ShellLevel::Event_OnLevelStart()
{
	NinjaLua::LuaObject func = CLuaGlobal::Get().State().GetGlobals()["OnLevelStart"]; // Put this and OnLevelEnd in level.h or something
	if(func.IsFunction())
	{
		NinjaLua::LuaFunction fn(func);
		fn();
	}

	NinjaLua::LuaObject func2 = CLuaGlobal::Get().State().GetGlobals()["OnReallyLevelStart"]; // Put this and OnLevelEnd in level.h or something
	if(func2.IsFunction())
	{
		NinjaLua::LuaFunction fn(func2);
		fn();
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	ShellLevel::Event_OnLevelEnd
//! Call a lua function to indicate the level has finished.
//! JML - Addition 13-12-05 - Level Event Functions
//!                                                                                         
//------------------------------------------------------------------------------------------
void	ShellLevel::Event_OnLevelEnd()
{
	NinjaLua::LuaObject func = CLuaGlobal::Get().State().GetGlobals()["OnLevelEnd"]; // Put this and OnLevelEnd in level.h or something
	if(func.IsFunction())
	{
		NinjaLua::LuaFunction fn(func);
		fn();
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	ShellLevel::RegisterCheckpoint
//! Registers a checkpoint with the level
//!                                                                                         
//------------------------------------------------------------------------------------------
void ShellLevel::RegisterCheckpoint(Object_Checkpoint* pobCheckpoint)
{
	ntAssert(pobCheckpoint);

	ntstd::Vector<Object_Checkpoint*>::iterator iter;
	ntstd::Vector<Object_Checkpoint*>::iterator iterEnd = m_CheckpointArray.end();

	for ( iter = m_CheckpointArray.begin(); iter != iterEnd; iter++ )
	{
		Object_Checkpoint* pobIterCP = (*iter);

		// Sanity check
		ntAssert( pobCheckpoint->GetID() != pobIterCP->GetID() );

		if ( pobCheckpoint->GetID() <= pobIterCP->GetID() )
		{
			m_CheckpointArray.insert( iter, pobCheckpoint );
			return;
		}
	}

	// If we get to this point, then add it to the end
	m_CheckpointArray.push_back( pobCheckpoint );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	ShellLevel::GetCheckpointByID
//! Gets the checkpoint with the passed in ID
//!                                                                                         
//------------------------------------------------------------------------------------------
Object_Checkpoint* ShellLevel::GetCheckpointByID( int nCheckpointID )
{
	ntAssert( nCheckpointID >= 0 );

	ntstd::Vector<Object_Checkpoint*>::const_iterator obIt;
	ntstd::Vector<Object_Checkpoint*>::const_iterator obItEnd = m_CheckpointArray.end();

	for (obIt = m_CheckpointArray.begin(); obIt != obItEnd; obIt++)
	{
		if ( (*obIt)->GetID() == nCheckpointID )
		{
			return (*obIt);
		}
	}

	// Couldn't find the checkpoint
	return 0;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	ShellLevel::StartAtCheckpoint
//! Starts at the checkpoint with the passed in ID
//!                                                                                         
//------------------------------------------------------------------------------------------
void ShellLevel::StartAtCheckpoint( int nCheckpointID )
{
	// Move player to checkpoint
	Object_Checkpoint* pCheckpoint = GetCheckpointByID( nCheckpointID );
	
	if ( pCheckpoint )
	{
		pCheckpoint->StartAtCheckpoint();
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	ShellLevel::ActivateChapterCompleteCheckpoint
//! Activates the last checkpoint in the chapter so that it is saved.  Only called when the
//! player SUCCESSFULLY completes a chapter.
//!                                                                                         
//------------------------------------------------------------------------------------------
void ShellLevel::ActivateChapterCompleteCheckpoint( void )
{
	Object_Checkpoint* pCheckpoint = m_CheckpointArray.back();
	ntAssert( pCheckpoint );

	// Get the player we are using at this checkpoint
	CEntity* pPlayer = CEntityManager::Get().GetPlayer();

	// Sanity Checks
	ntAssert( pPlayer);
	ntAssert( pPlayer->IsPlayer() );

	// Save out checkpoint data
	pCheckpoint->SaveCheckpoint( pPlayer );
}

//------------------------------------------------------------------------------------------
//!
//!	ShellLevel::Update
//! update anything that relys on a level being present
//!
//------------------------------------------------------------------------------------------
void	ShellLevel::Update()
{
	// we never update the level code if any of our pause modes are active...
	// BUT for the moment, we always ignore pauses on first frame, till people
	// move initialisation code out of the first update
	if( ShellMain::Get().IsPausedAny() && (m_bFirstFrame == false) )
		return;

	float fGameTimeStep = CTimer::Get().GetGameTimeChange();

	CGatso::Start( "ShellLevel::TimeOfDay" );
	if (TimeOfDay::Exists())
		TimeOfDay::Get().Update();
	CGatso::Stop( "ShellLevel::TimeOfDay" );

#ifdef PLATFORM_PS3
	ArmyManager::Get().SetCamera( CamMan::GetPrimaryView()->GetEyePos(), CamMan::GetPrimaryView()->GetViewTransform()->GetWorldMatrix().GetZAxis() );
	ArmyManager::Get().Update( fGameTimeStep );

	CGatso::Start( "ShellLevel::WaterManager::Update" );
	WaterManager::Get().Update( fGameTimeStep );
	CGatso::Stop( "ShellLevel::WaterManager::Update" );
#endif

	if(Physics::VerletManager::Exists() && !Physics::VerletManager::Get().GetFlags(Physics::VerletManager::IS_PAUSED) )
	{
		CGatso::Start( "ShellLevel::VerletManager" );
		Physics::VerletManager::Get().Update( fGameTimeStep );
		CGatso::Stop( "ShellLevel::VerletManager" );
	}

	if(ChainRessource::Exists())
	{
		CGatso::Start( "ShellLevel::ChainRessource" );
		ChainRessource::Get().Update();
		CGatso::Stop( "ShellLevel::ChainRessource" );
	}

	GATSO_PHYSICS_START( "Physics::All" );
	GATSO_PHYSICS_START( "ShellLevel::PhysicsWorld" );
	Physics::CPhysicsWorld::Get().Update( fGameTimeStep );
	GATSO_PHYSICS_STOP( "ShellLevel::PhysicsWorld" );
	GATSO_PHYSICS_STOP( "Physics::All" );

	//! start blendshape update 
	//! needs to be flushed before rendering
	CGatso::Start( "ShellLevel::XPUShapeBlending::BeginUpdate" );
	XPUShapeBlending::Get().BeginUpdate();
	CGatso::Stop( "ShellLevel::XPUShapeBlending::BeginUpdate" );

	CGatso::Start( "ShellLevel::AINavGraphManager" );
	CAINavGraphManager::Get().Update( fGameTimeStep );
	CGatso::Stop( "ShellLevel::AINavGraphManager" );

	CGatso::Start( "ShellLevel::CChatterBoxMan" );
	CChatterBoxMan::Get().Update( fGameTimeStep );
	CGatso::Stop( "ShellLevel::CChatterBoxMan" );

	// Dario - New Naviagtion System. ChatterBox, Hearing, CAIQueueManager
	CGatso::Start( "ShellLevel::CAINavigationSystemMan" );
	CAINavigationSystemMan::Get().Update( fGameTimeStep );
	CGatso::Stop( "ShellLevel::CAINavigationSystemMan" );
	
	CGatso::Start( "ShellLevel::CAIHearingMan" );
	CAIHearingMan::Get().Update( fGameTimeStep );
	CGatso::Stop( "ShellLevel::CAIHearingMan" );

	CGatso::Start( "ShellLevel::CAIInitialReactionMan" );
	CAIInitialReactionMan::Get().Update( fGameTimeStep );
	CGatso::Stop( "ShellLevel::CAIInitialReactionMan" );
	
	CGatso::Start( "ShellLevel::CAIQueueManager" );
	CAIQueueManager::Get().Update( fGameTimeStep );
	CGatso::Stop( "ShellLevel::CAIQueueManager" );

	CGatso::Start( "ShellLevel::AIAlertManager" );
	AIAlertManager::Get().Update( fGameTimeStep );
	CGatso::Stop( "ShellLevel::AIAlertManager" );

	CGatso::Start( "ShellLevel::AIBehaviourPool" );
	AIBehaviourPool::Get().Update( fGameTimeStep );
	CGatso::Stop( "ShellLevel::AIBehaviourPool" );

	CGatso::Start("ShellLevel::AIFormationManager");
	AIFormationManager::Get().Update(fGameTimeStep);
	CGatso::Stop("ShellLevel::AIFormationManager");

	CGatso::Start( "ShellLevel::NSManager" );
	NSManager::Get().Update( fGameTimeStep );
	CGatso::Stop( "ShellLevel::NSManager" );

	CGatso::Start( "ShellLevel::SyncdMovement" );
	SyncdMovement::Get().Update();
	CGatso::Stop( "ShellLevel::SyncdMovement" );

	CGatso::Start( "ShellLevel::SyncdCombat" );
	SyncdCombat::Get().Update();
	CGatso::Stop( "ShellLevel::SyncdCombat" );

	/////////////////////////////////////////////////
	// ENTITY UPDATE
	/////////////////////////////////////////////////
	CGatso::Start( "ShellLevel::AreaManager" );
	AreaManager::Get().Update();
	CGatso::Stop( "ShellLevel::AreaManager" );

	if(ForceFieldManager::Exists())
	{
		CGatso::Start( "ShellLevel::ForceFieldManager" );
		ForceFieldManager::Get().Update(CTimer::Get().GetGameTimeInfo());
		CGatso::Stop( "ShellLevel::ForceFieldManager" );
	}

	CGatso::Start( "ShellLevel::EntityManager" );
	CEntityManager::Get().Update();
	CGatso::Stop( "ShellLevel::EntityManager" );

	CGatso::Start( "ShellLevel::Triggers" );
	CTriggerVolumeManager::Get().Update( fGameTimeStep );
	CGatso::Stop( "ShellLevel::Triggers" );

	// NOTE! is unclear wether lua should be considered scoped by the
	// level or not.
	LuaTimerManager::Get().UpdateTimers();

	CGatso::Start( "ShellLevel::LuaGlobal" );
	CLuaGlobal::Get().Update();
	CGatso::Stop( "ShellLevel::LuaGlobal" );

	// HC: I've moved this because the cool cams rely on the characters root matrix for its positioning, unless the
	// camera update comes after it will cause jittering in the camera!
	CGatso::Start( "ShellLevel::CameraMan" );
	CamMan::Get().Update();
	CGatso::Stop( "ShellLevel::CameraMan" );

	if(StyleManager::Exists() )
		StyleManager::Get().Update( fGameTimeStep );

	CGatso::Start( "ShellLevel::Hierarchy" );
	CHierarchy::UpdateWorld();
	CGatso::Stop( "ShellLevel::Hierarchy" );

#ifdef _SPEEDTREE
	if (SpeedTreeManager::Exists())
	{
		CGatso::Start( "SpeedTreeManager::Animate" );
		SpeedTreeManager::Get().PerFrameUpdate( fGameTimeStep );
		CGatso::Stop( "SpeedTreeManager::Animate" );

		SpeedGrass::Update( fGameTimeStep );
	}
#endif

	//! flush shapeblending batches
	CGatso::Start( "ShellLevel::XPUShapeBlending::Flush" );
	XPUShapeBlending::Get().Flush();
	CGatso::Stop( "ShellLevel::XPUShapeBlending::Flush" );

	if (m_bFirstFrame)
		m_bFirstFrame = false;


	// Auto-exit code. Moved from shellglobal.cpp
	// Slightly unpleasant that I have to just ram it in here, wonder if there's a cleaner solution
#ifndef _GOLD_MASTER	
	if ( g_ShellOptions->m_iFrameCountdownBeforeAutoExit != 0 )
	{
		// There is a countdown set...
		if ( --g_ShellOptions->m_iFrameCountdownBeforeAutoExit == 0 )
		{
			// bye!
			ntPrintf( "[[AUTO_EXIT_TRIGGERED]]\n" );
			ShellMain::Get().RequestGameExit();
		}
		else if ( g_ShellOptions->m_iFrameCountdownBeforeAutoExit == 2 )
		{
			ntPrintf( "Triggering screen capture before exit...\n" );
			if ( CaptureSystem::Exists() )
			{
				CaptureSystem::Get().DoScreenShot();
			}
		}
	}
#endif

}
