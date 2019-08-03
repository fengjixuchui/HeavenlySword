//------------------------------------------------------------------------------------------
//!
//!	\file shellconfig.cpp
//! Defines startup shell configuration options only
//!
//------------------------------------------------------------------------------------------

#include "game/shellconfig.h"
#include "game/gamedata.h"
#include "lua/ninjalua.h"
#include "gfx/renderersettings.h"
#include "gfx/fxmaterial.h"
#include "effect/effect_manager.h"
#include "physics/verletmanager.h"
#include "physics/ikfootplacement.h"
#include "core/OSDDisplay.h"
#include "core/gatso.h"

// global pointer to shell options
ShellOptions*			g_ShellOptions = NULL;
NinjaLua::LuaState*		g_pLuaState = NULL;

bool g_bAllowOldData = true;
bool g_bAllowMissingData = true;
bool g_bAllowMissingContainers = true;

extern bool g_bSortGatso;
extern bool g_bEnableLuaLogs;

//------------------------------------------------------------------------------------------
//!
//!	ShellOptions::CreateShellOptions
//! Setup the global shell options
//!
//------------------------------------------------------------------------------------------
bool	ShellOptions::CreateShellOptions( const char* pFileName, char* pErrorMSG )
{
	ntError_p( g_ShellOptions == NULL, ("Already have global shell options") );
	g_ShellOptions = NT_NEW ShellOptions;
	if ( g_ShellOptions->SetFromFile(pFileName,pErrorMSG) )
		return true;

	NT_DELETE( g_ShellOptions );
	g_ShellOptions = NULL;
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellOptions::DestroyShellOptions
//!	Cleanup
//!
//------------------------------------------------------------------------------------------
void	ShellOptions::DestroyShellOptions()
{
	ntError_p( g_ShellOptions, ("Dont have global shell options") );

	NT_DELETE( g_ShellOptions );
	g_ShellOptions = NULL;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellOptions ctor
//!
//------------------------------------------------------------------------------------------
ShellOptions::ShellOptions() :

#ifdef PLATFORM_PC

	m_contentNeutral( "Z:/HS/content_neutral" ),
	m_contentPlatform( "Z:/HS/content_pc" ),

	m_iBBWidth( 1280 ),
	m_iBBHeight( 720 ),
	m_iFSWidth( 1280 ),
	m_iFSHeight( 960 ),

	m_bFullscreen( false ),

#endif

#ifdef _GOLD_MASTER
	m_bUsingHDD( true ),
#else
	m_bUsingHDD( false ),
#endif
	m_bUsingWADs( false ),
	m_bUsingBluRay( true ),

	m_renderQuality( RENDER_QUALITY_BEST ),

	m_dbgStartLevel( "empty" ),
	m_dbgStartCheckpoint( 0 ),

	m_bInvertDebugCamX( false ),			
	m_bInvertDebugCamY( true ),

	m_bLuaDebugging( false ),
	m_bLuaDebuggingAutoLaunch( false ),
	m_bLuaProfiling( false ),
	m_bLuaDebugOnWarnings( false ),

	m_bOneHitKills( false ),
	m_bDisableAIBlocking( false ),
	m_bFullSuperStyleCost( true ),
	m_bInteractableRagdolls( true ),
	m_bPickupRagdolls( false ),				// SCEE_SWright - off by default to protect those crazy Americans from interfering with the dead.
	m_bUseHeroHealth( true ),
	m_bLCCharge( false ),
	m_bStyleDuringSS( true ),

	
	m_bAudioEngine( true ),
	m_bEnableMusic( false ),
	m_bLiveAudioEditing( false ),
	m_bMusicDebug( false ),
	
	m_bUsingMotionControllerOnNSs( false ),
	m_bInvertYAxis( false ),
	m_bEvades( true ),
	m_bOverrideMotionSensorPlayerOption( false ),
    m_bUseMotionSensor( false ),
	m_bUseMotionSensor_ForCams(false),

	m_bUseHUD( true ),
	m_bEnableGui( false ),
	m_bStraightToGame( false ),
	m_bSkipLevelSelect( true ),
	m_bGuiSoundFirst( true ),
	m_bGuiAttractMode( false ),
	m_bGuiXOSwap( false ),
	m_iGUIStartupLevelSkipCount( 0 ),
	m_eFrontendMode( FRONTEND_NONE ),
	m_bEnableScreenCalibration( false ),
	m_bFrontEndAutoTest(false),
	m_bUseCounter(false),

	m_bUseHair( true ),
	m_bHairOnSPU( true ),
	m_bUseSpeedTree( true ),
	m_bEnableBlendShapes( false ),
	m_iBSVertsPerBatch( 1024 ),
	m_bEnableWater( false ),

	m_obCaptureName("HeavenlySword"),
	m_obCaptureDir("Capture"),

#ifdef PLATFORM_PS3
	m_bUseHavokDebugger(false),
#else
	m_bUseHavokDebugger(true),
#endif
	m_bDoOneSideCollision( false ), 
	m_SkipMovies( false ),
	m_obUserName( "" ),
	m_iFrameCountdownBeforeAutoExit( 0 ),
	m_iNetworkAttemptCycles(10),
	m_bNetworkAvailable( false ),
	m_bUseArmies( true ),

	m_bBehaviourDebug(false),
	m_bChatterBoxPrintDebug(false),
	m_bPrintPathDebug(false),
	m_bTestAIVols(false),
	m_bDebugArcher(false),
	m_bQueueOnObjects(true),
	m_bDivingDebug(false),
	m_bRangedTargetDebug(false)
{}

//------------------------------------------------------------------------------------------
//!
//!	ShellOptions::SetFromFile
//! Init from a given file
//!
//------------------------------------------------------------------------------------------
bool ShellOptions::SetFromFile( const char* pFileName, char* pErrorMSG )
{
	if ( File::Exists(pFileName) )
	{
		g_pLuaState = NinjaLua::LuaState::Create( false, false );

		#ifdef PLATFORM_PC
		
		static const int BUFFFER_SIZE = 1024;
		char aTempBuffer[BUFFFER_SIZE];

		// build and set into the pre-init lua context the login name
		DWORD iBufferSize = BUFFFER_SIZE;
		GetUserName( aTempBuffer, &iBufferSize );

		g_pLuaState->GetGlobals().Set("LoginName", aTempBuffer);

		#endif

		// load our config file as a lua file
		lua_State* L = &(**g_pLuaState);
		int stackTop = lua_gettop(L);

		if (luaL_loadfile(L, pFileName) == 0)
		{
			if (lua_pcall(L, 0, LUA_MULTRET, 0) == 0)
			{
				// move variables from lua state to C++ state
				//-------------------------------------------------
				#ifdef PLATFORM_PC

				LoadSTDString( m_contentNeutral,	"content_neutral" );
				LoadSTDString( m_contentPlatform,	"content_platform" );

				LoadInteger( m_iBBWidth,			"backbuffer_width" );
				LoadInteger( m_iBBHeight,			"backbuffer_height" );
				LoadInteger( m_iFSWidth,			"fullscreen_width" );
				LoadInteger( m_iFSHeight,			"fullscreen_height" );

				LoadBoolean( m_bFullscreen,			"fullscreen" );

				#endif

				// rendering options
				//-------------------------------------------------
				m_renderQuality = RENDER_QUALITY_BEST;
				LoadInteger( *(int*)&m_renderQuality, "render_quality" );

				#ifdef PLATFORM_PS3

				LoadBoolean( CRendererSettings::bUseHeresy, "use_heresy" );

				CHashedString displayMode;
				LoadHashString( displayMode, "display_mode" );

				if (displayMode == CHashedString("1080p"))
					CRendererSettings::eDisplayMode = DisplayManager::SIXTEEN_NINE_1080;

				if (displayMode == CHashedString("720p"))
					CRendererSettings::eDisplayMode = DisplayManager::SIXTEEN_NINE_720;

				if (displayMode == CHashedString("480p"))
					CRendererSettings::eDisplayMode = DisplayManager::FOUR_THREE_480;

				if (displayMode == CHashedString("480pw"))
					CRendererSettings::eDisplayMode = DisplayManager::SIXTEEN_NINE_480;

				bool bDisableAA = false;
				LoadBoolean( bDisableAA, "disable_aa" );
				if (bDisableAA)
					CRendererSettings::eAAMode = AAM_NONE;

				LoadBoolean( CRendererSettings::bProfileLowFillrate,	"profile_low_fillrate" );
				LoadBoolean( CRendererSettings::bUseLowRezMode,			"use_low_rez_mode" );
				LoadBoolean( CRendererSettings::bUseZPrepass,			"use_z_prepass" );
				LoadBoolean( CRendererSettings::bUseFastShadowRecieve,	"use_fast_shadow_recieve" );
				LoadBoolean( CRendererSettings::bEnableBatchRenderer,	"enable_batch_renderer" );
				LoadBoolean( CRendererSettings::bEnableSPURenderer,		"enable_spu_renderer" );
				LoadBoolean( CRendererSettings::bEnableBeyondDebugging, "enable_beyond_debugging");

				#endif	// PLATFORM_PS3

				LoadBoolean( CRendererSettings::bEnableSky,				"enable_sky" );
				LoadBoolean( CRendererSettings::bEnableLensEffects,		"enable_lens" );
				LoadBoolean( CRendererSettings::bForceShaderModel2,		"ForceShaderModel2" );
				LoadBoolean( CRendererSettings::bEnableShadows,			"enable_shadows"  );
				LoadBoolean( CRendererSettings::bLogTextureInfo,		"log_texture_info" );
				LoadBoolean( CRendererSettings::bUseTextures,			"use_textures" );
				LoadBoolean( CRendererSettings::bUseGTFTextures,		"use_gtf" );
				LoadBoolean( CRendererSettings::bEnableOptimiseMeshesOnLoad, "OptimiseMeshesOnLoad" );
				LoadBoolean( CRendererSettings::bEnableDebugPrimitives, "enable_debug_prims" );

				bool bDisableFXSafety = true;
				LoadBoolean( bDisableFXSafety, "disable_fx_safety" );
				FXMaterialManager::s_bUseFXSafety = !bDisableFXSafety;

				// start level options
				LoadSTDString( m_dbgStartLevel, "startlevel" );
				LoadInteger( m_dbgStartCheckpoint, "startcheckpoint" );

				// camera options
				LoadBoolean( m_bInvertDebugCamX, "invertx" );
				LoadBoolean( m_bInvertDebugCamY, "inverty" );

				// Lua Debugging
				LoadBoolean( g_bEnableLuaLogs, "enable_lua_logs" );
				LoadBoolean( m_bLuaDebugging, "lua_debug" );
				if(m_bLuaDebugging)
				{
					LoadBoolean(m_bLuaDebuggingAutoLaunch, "lua_asserts");
					LoadBoolean(m_bLuaProfiling,           "lua_profile");
					LoadBoolean(m_bLuaDebugOnWarnings,     "lua_warningsaserrors");
				}

				// combat options
				LoadBoolean( m_bOneHitKills, "one_hit_kills" );
				LoadBoolean( m_bDisableAIBlocking, "disable_ai_blocking" );
				LoadBoolean( m_bFullSuperStyleCost, "full_superstyle_cost" );
				LoadBoolean( m_bPickupRagdolls, "pickup_ragdolls" );
				LoadBoolean( m_bInteractableRagdolls, "interact_ragdolls" );
				LoadBoolean( m_bUseHeroHealth, "use_hero_health" );
				LoadBoolean( m_bLCCharge, "lifeclock_charge" );
				LoadBoolean( m_bStyleDuringSS, "style_during_superstyle" );

				// audio options
				LoadBoolean( m_bAudioEngine, "audio_engine" );
				//LoadBoolean( m_bAudioEngine, "enable_audio" );
				LoadBoolean( m_bEnableMusic, "enable_music" );
				LoadBoolean( m_bLiveAudioEditing, "live_audio_editing" );
				LoadBoolean( m_bMusicDebug, "music_debug" );

				// input options
				//-------------------------------------------------

				// scee.sbashow - a setting as to whether to use motion controller on NSs
				LoadBoolean(m_bUsingMotionControllerOnNSs, "using_motioncontroller_on_nss");

				// All code using m_bInvertYAxis should now use
				// CPlayerOptions::Get().GetInvertY().  PlayerOptions inverty is
				// initialised from game.config during construction.
				LoadBoolean( m_bInvertYAxis, "invert_yaxis" );

				LoadBoolean( m_bEvades, "evades" );

				// If 'use_motion_sensor' is in the game.config then use it to override player options when they are loaded.
				if ( g_pLuaState->GetGlobals()[ "use_motion_sensor" ].IsBoolean() )
					m_bOverrideMotionSensorPlayerOption = true;
				else
					m_bOverrideMotionSensorPlayerOption = false;

				LoadBoolean( m_bUseMotionSensor, "use_motion_sensor" );
				m_bUseMotionSensor_ForCams = m_bUseMotionSensor;
				LoadBoolean(m_bUseMotionSensor_ForCams, "use_motion_sensor_tweakers");

				// GUI / frontend
				//-------------------------------------------------

				LoadBoolean( m_bUseHUD,			"use_hud" );
				LoadBoolean( m_bEnableGui,		"enable_gui" );
				LoadBoolean( m_bStraightToGame,	"straight_to_game" );
				LoadBoolean( m_bSkipLevelSelect,"skip_level_select" );
				LoadBoolean( m_bGuiSoundFirst,	"gui_sound_first" );
				LoadBoolean( m_bGuiAttractMode,	"gui_attract_mode" );
				LoadBoolean( m_bGuiXOSwap,		"gui_XOswap" );
				
				m_iGUIStartupLevelSkipCount = m_bSkipLevelSelect ? 3 : 1;

				// Work out our frontend mode
				int iFrontendMode = -1;
				ntstd::String obFEMode;
				LoadSTDString(obFEMode, "frontend_mode");

				if (obFEMode == "none")
					iFrontendMode = FRONTEND_NONE;
				else if (obFEMode == "e3")
					iFrontendMode = FRONTEND_E3;
				else if (obFEMode == "levelselect") 
					iFrontendMode = FRONTEND_LEVEL_SELECT;
				else if (obFEMode == "autotest" || obFEMode == "final" || obFEMode == "purgatory" )
				{
					if (obFEMode == "autotest")
						m_bFrontEndAutoTest = true;
					iFrontendMode = FRONTEND_FINAL;
				}
				else if (obFEMode == "tgs") 
					iFrontendMode = FRONTEND_TGS;
				else
				LoadInteger(iFrontendMode,		"frontend_mode");

				if (iFrontendMode == -1)	//not specified
				{
					// in this case, read the old switches to work out what needs to be done
					ntPrintf("frontend_mode needs to be specified... using old params to determine mode\n");

					if (!m_bEnableGui)
						m_eFrontendMode = FRONTEND_NONE;
					else if (m_bStraightToGame)
						m_eFrontendMode = FRONTEND_E3;
					else //if (use lvl select ^^)
						m_eFrontendMode = FRONTEND_LEVEL_SELECT;
				}
				else
				{
					ntAssert(iFrontendMode >= 0 && iFrontendMode < FRONTEND_MAX);
					m_eFrontendMode = (FRONTEND_MODE)iFrontendMode;
				}
				ntPrintf("frontend_mode = %d\n", (int)m_eFrontendMode);

				LoadBoolean(m_bUseCounter,		"use_counter");
				
#if !defined(_GOLD_MASTER)

#define DEBUG_GUI_BOOL(name, x) {extern bool x; LoadBoolean(x, name); }
#define DEBUG_GUI_INT(name, x) {extern int x; LoadInteger(x, name); }

				DEBUG_GUI_BOOL("checkpoint_data_unlock_all", g_bUnlockAllGameData);
				DEBUG_GUI_BOOL("checkpoint_data_clear_existing", g_bClearAllGameData);
				DEBUG_GUI_INT("checkpoint_data_chapter", g_iChaptersComplete);
				DEBUG_GUI_INT("checkpoint_data_checkpoint", g_iUpToCheckpoint);
				DEBUG_GUI_BOOL("checkpoint_data_clear_hit_flags", g_bClearHitFlags);
				DEBUG_GUI_BOOL("checkpoint_data_set_final_hit_flag", g_bSetLastHitFlags);
#endif

				LoadBoolean( m_bEnableScreenCalibration, "enable_screen_calibration" );

				// effects
				LoadBoolean( m_bUseHair,			"hair" );
				LoadBoolean( m_bHairOnSPU,			"hairSPU" );
				LoadBoolean( m_bUseSpeedTree,		"speedtree" );
				LoadBoolean( m_bEnableBlendShapes,	"enable_blendshapes" );
				LoadInteger( m_iBSVertsPerBatch,	"blendshapes_verts_per_batch" );	
				LoadBoolean( m_bEnableWater,		"enable_water" );

				bool bEffectsDisabled = false;
				LoadBoolean( bEffectsDisabled,		"disable_effects" );
				if (bEffectsDisabled)
					EffectManager::ToggleRender();

				bool bEnableSoft = false;
				LoadBoolean( bEnableSoft,			"enable_soft" );
				Physics::VerletManager::SetEnabled(bEnableSoft);

				#ifdef PLATFORM_PC					
				if ( CRendererSettings::bEnableOptimiseMeshesOnLoad )
					m_bEnableBlendShapes = false;
				#endif

				// capture system
				LoadSTDString(m_obCaptureName,		"capturename");
				LoadSTDString(m_obCaptureDir,		"capturedir");

				// misc
				//-------------------------------------------------

				LoadBoolean( m_bUseHavokDebugger,	"use_havok_debugger" );
				LoadBoolean( m_bDoOneSideCollision,	"one_side_collisions" );
				LoadBoolean( m_SkipMovies,			"skip_movies" );
				LoadHashString(m_obUserName,		"user_name");
				LoadInteger( m_iFrameCountdownBeforeAutoExit, "auto_exit_time" );
				LoadInteger( m_iNetworkAttemptCycles, "network_attempts" );
								
#				ifdef PLATFORM_PS3
				{
					LoadBoolean( m_bUsingHDD,			"using_hdd" );
					LoadBoolean( m_bUsingWADs,			"using_wads" );

					// If we're using WADs then we also need to be using GTFs.
					if ( m_bUsingWADs )
					{
						 CRendererSettings::bUseGTFTextures = true;
					}
				}

				// This variable is defined in entitymessagehub.cpp.
				{
					extern bool g_UseArmy;
					LoadBoolean( g_UseArmy,				"use_army" );
				}
#				endif

				#ifdef PLATFORM_PS3
				LoadBoolean( CGameData::m_bGameDataTestMode, "gamedata_test_mode" );
				#endif

				bool bOSDDebugChannel = true;
				LoadBoolean( bOSDDebugChannel,		"debug_channel" );
				OSD::SetChannelEnable( OSD::DEBUG_CHAN, bOSDDebugChannel );

				LoadBoolean( Physics::IKFootPlacement::m_gIKEnable, "enable_ik" );

				#ifdef	_GATSO
				
				bool bEnableGatso = false;
				LoadBoolean( bEnableGatso, "enable_gatso" );
				CGatso::m_gbDisableGatso = !bEnableGatso;
				
				LoadBoolean( g_bSortGatso, "sort_gatso" );
				
				#endif

				// temp hack to help the move to the batch exporter
				LoadBoolean( g_bAllowOldData, "allow_old_data" );

				// temp hacks to allow missing data to be loaded.
				LoadBoolean( g_bAllowMissingData, "allow_missing_arm_data" );				
				LoadBoolean( g_bAllowMissingContainers, "allow_missing_anim_containers" );

				if( g_pLuaState->GetGlobals()["FixedTimeChange"].IsNumber() )
				{
					CTimer::m_bUsedFixedTimeChange = true;
					CTimer::m_fFixedTimeChange = (float)g_pLuaState->GetGlobals()["FixedTimeChange"].GetNumber();
				}

				// DARIO debug options
				LoadBoolean( m_bBehaviourDebug,			"behaviour_debug" );
				LoadBoolean( m_bChatterBoxPrintDebug,	"chatterbox_print_debug" );
				LoadBoolean( m_bPrintPathDebug,			"path_debug" );
				LoadBoolean( m_bTestAIVols,				"test_ai_volumes" );
				LoadBoolean( m_bDebugArcher,			"debug_archer" );
				LoadBoolean( m_bQueueOnObjects,			"queue_on_objects" );
				LoadBoolean( m_bDivingDebug,			"diving_debug" );
				LoadBoolean( m_bRangedTargetDebug,		"rangedtarget_debug" );
			}
			else
			{
				sprintf( pErrorMSG, "%s", lua_tostring(L, -1) );
				lua_settop(L, stackTop);
				NinjaLua::LuaState::Destroy( g_pLuaState );
				g_pLuaState = NULL;
				return false;
			}
		}
		else
		{
			sprintf( pErrorMSG, "%s", lua_tostring(L, -1) );
			lua_settop(L, stackTop);
			NinjaLua::LuaState::Destroy( g_pLuaState );
			g_pLuaState = NULL;
			return false;
		}

		// thats it, cleanup
		// it appears we now have to manually call the lua garbage collector
		lua_gc(*g_pLuaState, LUA_GCCOLLECT, 0);
		NinjaLua::LuaState::Destroy( g_pLuaState );
		g_pLuaState = NULL;
	}
	else
	{
		sprintf( pErrorMSG, "Could not find game.config, defaults no longer supported. Have you set your working directory?" );
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellOptions::LoadType
//! Copy the relevant var from our local lua state if it exists
//!
//------------------------------------------------------------------------------------------
void	ShellOptions::LoadSTDString( ntstd::String& dest, const char* varName )
{
	ntError_p( g_pLuaState, ("Must have a valid lua state here") );
	if ( g_pLuaState->GetGlobals()[ varName ].IsString() )
		dest = g_pLuaState->GetGlobals()[ varName ].GetString();
}

void	ShellOptions::LoadHashString( CHashedString& dest, const char* varName )
{
	ntError_p( g_pLuaState, ("Must have a valid lua state here") );
	if ( g_pLuaState->GetGlobals()[ varName ].IsString() )
		dest = g_pLuaState->GetGlobals()[ varName ].GetString();
}

void	ShellOptions::LoadBoolean( bool& dest, const char* varName )
{
	ntError_p( g_pLuaState, ("Must have a valid lua state here") );
	if ( g_pLuaState->GetGlobals()[ varName ].IsBoolean() )
		dest = g_pLuaState->GetGlobals()[ varName ].GetBoolean();
}

void	ShellOptions::LoadInteger( int& dest, const char* varName )
{
	ntError_p( g_pLuaState, ("Must have a valid lua state here") );
	if ( g_pLuaState->GetGlobals()[ varName ].IsInteger() )
		dest = g_pLuaState->GetGlobals()[ varName ].GetInteger();
	else if ( g_pLuaState->GetGlobals()[ varName ].IsString() )
		dest = atoi( g_pLuaState->GetGlobals()[ varName ].GetString() );
}

void	ShellOptions::LoadFloat( float& dest, const char* varName )
{
	ntError_p( g_pLuaState, ("Must have a valid lua state here") );
	if ( g_pLuaState->GetGlobals()[ varName ].IsNumber() )
		dest = _R( g_pLuaState->GetGlobals()[ varName ].GetNumber() );
}

