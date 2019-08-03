//------------------------------------------------------------------------------------------
//!
//!	\file shellconfig.h
//! Defines startup shell configuration options only
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLCONFIG_H
#define GAME_SHELLCONFIG_H

//------------------------------------------------------------------------------------------
//!
//!	RENDER_QUALITY_MODE
//! Startup render mode
//!
//------------------------------------------------------------------------------------------
enum RENDER_QUALITY_MODE
{
	RENDER_QUALITY_BEST = 0, 
	RENDER_QUALITY_LOW_SHADOWS, 
	RENDER_QUALITY_NO_SHADOWS, 
	RENDER_QUALITY_FASTEST, 
	RENDER_QUALITY_VERTEX_LIT, 

	RENDER_MAX,
};

//------------------------------------------------------------------------------------------
//!
//!	FRONTEND_MODE
//! What mode the front end screen starts in
//!
//------------------------------------------------------------------------------------------
enum FRONTEND_MODE
{
	FRONTEND_NONE,
	FRONTEND_E3,
	FRONTEND_LEVEL_SELECT,
	FRONTEND_FINAL,
	FRONTEND_TGS,

	FRONTEND_MAX
};

//------------------------------------------------------------------------------------------
//!
//!	ShellOptions
//! Options used by the game
//!
//------------------------------------------------------------------------------------------
class ShellOptions
{
private:
	ShellOptions();
	bool SetFromFile( const char* pFileName, char* pErrorMSG );

	// methods for setting our config object up
	static void LoadSTDString( ntstd::String& dest, const char* varName );
	static void LoadHashString( CHashedString& dest, const char* varName );
	static void LoadBoolean( bool& dest, const char* varName );
	static void LoadInteger( int& dest, const char* varName );
	static void LoadFloat( float& dest, const char* varName );

public:
	static bool CreateShellOptions( const char* pFileName, char* pErrorMSG );
	static void DestroyShellOptions();

	#ifdef PLATFORM_PC

	// resource paths
	ntstd::String	m_contentNeutral;
	ntstd::String	m_contentPlatform;

	// render options
	int				m_iBBWidth, m_iBBHeight;
	int				m_iFSWidth, m_iFSHeight;
	bool			m_bFullscreen;

	#endif

	// file system options
	bool			m_bUsingHDD;
	bool			m_bUsingWADs;
	bool			m_bUsingBluRay;

	RENDER_QUALITY_MODE	m_renderQuality;
	void ChangeRenderQuality()
	{
		m_renderQuality = (RENDER_QUALITY_MODE)(m_renderQuality + 1);
		if (m_renderQuality == RENDER_MAX)
			m_renderQuality = RENDER_QUALITY_BEST;
	}
	
	// Accessor function, to make sure we check both flags for picking up ragdolls.
	bool CanPickupRagdolls( void )	{ return ( m_bInteractableRagdolls && m_bPickupRagdolls ); }

	// start level options
	ntstd::String	m_dbgStartLevel;
	int				m_dbgStartCheckpoint;

	// camera options
	bool			m_bInvertDebugCamX;
	bool			m_bInvertDebugCamY;

	// Lua Debugger
	bool			m_bLuaDebugging;			// Will we accept remote debug connections?
	bool			m_bLuaDebuggingAutoLaunch;  // Automatically launch the debugger on lua asserts?
	bool			m_bLuaProfiling;			// Collect profiling information on scritps?
	bool			m_bLuaDebugOnWarnings;		// Launch the debugging for lua warnings?

	// combat options
	bool			m_bOneHitKills;
	bool			m_bDisableAIBlocking;		// Disable the blocking of AI's
	bool			m_bFullSuperStyleCost;		// Does a superstyle move use the whole bar?
	bool			m_bInteractableRagdolls;	// Can ragdolls be interacted with at all?
private:
	bool			m_bPickupRagdolls;			// Can player pickup ragdolls? Forced to use accessor.

public:
	bool			m_bUseHeroHealth;			// Use the health system on Hero with HS
	bool			m_bLCCharge;				// Hold the grab button to charge LC from the SS bar
	bool			m_bStyleDuringSS;			// Gain Style points while performing a SS move

	// audio options
	bool			m_bAudioEngine;				// Enable/disable sound
	bool			m_bEnableMusic;				// Enable/disable music system
	bool			m_bLiveAudioEditing;		// Enable/disable live audio editing
	bool			m_bMusicDebug;				// Enable/disable music control debug display
	
	// input options
	bool			m_bUsingMotionControllerOnNSs; // scee.sbashow - for testing NSs with motion controller or not.
	bool			m_bInvertYAxis;				// Invert Y axis for first person control schemes
	bool			m_bEvades;					// Evades on or off (off to make the debug cam usable)
	bool			m_bOverrideMotionSensorPlayerOption;	// Was 'use_motion_sensor' specified in the game.config
	bool			m_bUseMotionSensor;			// Enable the motion sensor for aftertouch
	bool			m_bUseMotionSensor_ForCams;	// Enable the motion sensor for camera tweaking

	// GUI / frontend
	bool			m_bUseHUD;					// Is the Hud On?
	bool			m_bEnableGui;				// Are we using the GUI?
	bool			m_bStraightToGame;			// If the GUI is enabled still go straight into the game. 
	bool			m_bSkipLevelSelect;			// Jump to the simple level select. 
	bool			m_bGuiSoundFirst;			// GUI sound before GUI action?
	bool			m_bGuiAttractMode;			// GUI Attract mode
	bool			m_bGuiXOSwap;				// GUI O confirm X cancel mode
	int				m_iGUIStartupLevelSkipCount;// used by the debug startup sequence to determine when to actually start loading a level
	FRONTEND_MODE	m_eFrontendMode;			// Determines frontend behaviour
	bool			m_bEnableScreenCalibration;	// Enable the Screen Calibration to run on startup.
	bool			m_bFrontEndAutoTest;		/// Frontend in autotest mode. this will make some menu's select default options.
	bool			m_bUseCounter;				// Enable the startup onscreen timer

	// effects
	bool			m_bUseHair;					// hair on
	bool			m_bHairOnSPU;				// hair on SPU (PC: emulated)
	bool			m_bUseSpeedTree;			// are we using speed tree
	bool			m_bEnableBlendShapes;		// enable blendshapes (PC version only)
	int				m_iBSVertsPerBatch;
	bool			m_bEnableWater;

	// capture system
	ntstd::String	m_obCaptureName;			// The output name of the capture mode
	ntstd::String	m_obCaptureDir;				// The output dir of the capture mode
	
	// misc
	bool			m_bUseHavokDebugger;		// Disables the havoc debugger
	bool			m_bDoOneSideCollision;      // One / Double side collision on triangles by Havok
	bool			m_SkipMovies;
	CHashedString	m_obUserName;				// Get the username
	int				m_iFrameCountdownBeforeAutoExit;	// number of frames left before game auto-exits (PS3 only). Used for automated testing.
	int				m_iNetworkAttemptCycles;	// Number of times to attempt getting an IP address.
	bool			m_bNetworkAvailable;		// do we have a valid game network stack
	bool			m_bUseArmies;				// do we want to use the army sections of the game

	// DARIO debug options
	bool			m_bBehaviourDebug;			// Print behaviours debug info. Dario 
	bool			m_bChatterBoxPrintDebug;	// Print Chatterbox debug info. Dario 
	bool			m_bPrintPathDebug;			// Print A* Path. Dario
	bool			m_bTestAIVols;				// The player shows a collision and LOS with AI volumes. Dario
	bool			m_bDebugArcher;				// Print debug archer info. Dario 
	bool			m_bQueueOnObjects;			// Allows queues on objects (e.g. ladders)
	bool			m_bDivingDebug;				// Debug Render and console prints related to AIs diving away from bolts
	bool			m_bRangedTargetDebug;		// Debug Render the ranged target-points for AIs (GavC).
};		

//------------------------------------------------------------------------------------------
//!
//!	Global accessor for debug shell options
//!
//------------------------------------------------------------------------------------------
extern ShellOptions* g_ShellOptions;

#endif
