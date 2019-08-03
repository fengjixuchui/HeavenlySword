//------------------------------------------------------------------------------------------
//!
//!	\file shellglobal.cpp
//! Object that handles creation of minimal resources required to run a game
//!
//------------------------------------------------------------------------------------------

#include "game/shellconfig.h"
#include "game/shelldebug.h"
#include "game/shellglobal.h"
#include "game/shellmain.h"
#include "game/command.h"
#include "game/keybinder.h"
#include "game/capturesystem.h"

#include "exec/exec.h"
#include "exec/PPU/elfmanager.h"

#include "gfx/renderersettings.h"
#include "gfx/graphicsdevice.h"
#include "gfx/sector.h"

#include "core/wad.h"
#include "core/fileio_ps3.h"

#include "audio/audiosystem.h"

#include <fp/fpresource/fpresource.h>

#include <cell/fs/cell_fs_file_api.h>
#include <cell/fs/cell_fs_errno.h>

#include <sys/process.h>
#include <sys/paths.h>
#include <sys/timer.h>

#include <sysutil/sysutil_common.h>

#include <netex/libnetctl.h>
#include <netex/ifctl.h>
#include <netex/net.h>

#include <arpa/inet.h>

#ifdef COLLECT_SHADER_STATS
extern void ResetShaderStats();
#endif

// we need thi
namespace Util
{
	const char* GetAppHomePath();
	const char* GetBluRayDevicePath();
}

//------------------------------------------------------------------------------------------
//!
//!	InitFileSystemAndConfig
//! Reads game.config, then inits FIOS, so we can overide how it works
//!
//------------------------------------------------------------------------------------------
const char* g_BluRayPath = NULL;

void InitFileSystemAndConfig()
{
	// detect if a bluray disk is present
	bool bUsingBluRay = true;

	CellFsStat info;
	if ( cellFsStat( SYS_DEV_BDVD, &info ) != CELL_FS_SUCCEEDED ) 
		bUsingBluRay = false;

	g_BluRayPath = bUsingBluRay ? Util::GetBluRayDevicePath() : Util::GetAppHomePath();

	// Setup file system, 
	FileManager::BasicInit();

	// now we can read our game.config using the defualt media path of fios
	static char errorMSG[4096];
	bool bOkay = false;

	// read args from game.config
	static char configFile[ MAX_PATH ];
	Util::GetFiosFilePath_Platform( "game.config", configFile );
	bOkay = ShellOptions::CreateShellOptions( configFile, errorMSG );

	// Disc resources are always WADs.
	if ( bUsingBluRay )
	{
		g_ShellOptions->m_bUsingBluRay = true;
		g_ShellOptions->m_bUsingWADs = true;
	}
	else
	{
		g_ShellOptions->m_bUsingBluRay = false;
	}

	// if we ain't got one by now we are screwed...
	ntError_p( bOkay, ("%s",errorMSG) );
}

//------------------------------------------------------------------------------------------
//!
//!	InitialiseNetwork
//! intialise the PS3 network stack
//!
//------------------------------------------------------------------------------------------
void InitialiseNetwork( int attempts = 60, int millisecTimeout = 500, bool bSilent = false )
{
	// initialise the network system, required to obtain the MAC address for UUID
	// and obviously for NET fs and our debug network support
	uint32_t initResult = 0;
	initResult = sys_net_initialize_network();
	ntError_p( initResult >= 0, ("sys_net_initialize_network() failed(%d)\n", initResult) );
		
	initResult = cellNetCtlInit();
	if ( initResult == CELL_NET_CTL_ERROR_NOT_TERMINATED )
	{
		cellNetCtlTerm();
		initResult = cellNetCtlInit();
	}

	ntError_p( initResult >= 0, ("cellNetCtlInit() failed(%x)\n", initResult) );

	if (bSilent == false)
	{
		ntPrintf("Obtaining IP address\n");
	}

	int state = 0;
	while (attempts--)
	{
		initResult = cellNetCtlGetState(&state);
		ntError_p( initResult >= 0, ("cellNetCtlGetState() failed(%x)\n", initResult) );
			
		switch (state)
		{
		case CELL_NET_CTL_STATE_Disconnected:
			if (bSilent == false)
			{
				ntPrintf("%d: CELL_NET_CTL_STATE_Disconnected\n",attempts);
			}
			break;

		case CELL_NET_CTL_STATE_Connecting:
			if (bSilent == false)
			{
				ntPrintf("%d: CELL_NET_CTL_STATE_Connecting\n",attempts);
			}
			break;

		case CELL_NET_CTL_STATE_IPObtaining:
			if (bSilent == false)
			{
				ntPrintf("%d: CELL_NET_CTL_STATE_IPObtaining\n",attempts);
			}
			break;

		case CELL_NET_CTL_STATE_IPObtained:
			if (bSilent == false)
			{
				ntPrintf("%d: CELL_NET_CTL_STATE_IPObtained\n",attempts);
			}
			break;
		}

		if (state == CELL_NET_CTL_STATE_IPObtained)
			break;

		// pause for a second then try again
		sys_timer_usleep(millisecTimeout * 1000);
	}

	if (state == CELL_NET_CTL_STATE_IPObtained)
	{
		sys_net_show_ifconfig();
		g_ShellOptions->m_bNetworkAvailable = true;
		return;
	}

	g_ShellOptions->m_bNetworkAvailable = false;
}

//------------------------------------------------------------------------------------------
//!
//!	UpdateNetworkStatus
//! This is just a "cleaned" version of what used to be in UpdateGlobals()
//! This monitors the network status and updates the g_ShellOptions->m_bNetworkAvailable flag
//! It no longer calls ShutdownNetwork() which could hose Welder
//!
//------------------------------------------------------------------------------------------
void UpdateNetworkStatus( bool bSilent = false )
{
	if (g_ShellOptions->m_bNetworkAvailable == false)
	{
		// Hmmm, we have no network. Check every now and again to see if IP has been obtained
		
		static const int TEST_INTERVAL = 30 * 60;		// i.e every minute	assuming full frame rate
		static int framedelay = TEST_INTERVAL;

		if (framedelay-- == 0)
		{
			framedelay = TEST_INTERVAL;
			
			// OK, let's do a check. Previously, this would just flatten sys_network, but 
			// if you're doing Live Update, not good!
			
			// Instead, we just query network status again.
			int initResult = 0;
			int state;
			
			initResult = cellNetCtlGetState(&state);
			ntError_p( initResult >= 0, ("cellNetCtlGetState() failed(%x)\n", initResult) );
				
			switch (state)
			{
			case CELL_NET_CTL_STATE_Disconnected:
			
				if ( !bSilent )
				{
					ntPrintf("CELL_NET_CTL_STATE_Disconnected\n");
				}
				break;
	
			case CELL_NET_CTL_STATE_Connecting:
			case CELL_NET_CTL_STATE_IPObtaining:
				if ( !bSilent )
				{
					ntPrintf( "***Still waiting for a net connection***\n");
				}
				
				break;
	
			case CELL_NET_CTL_STATE_IPObtained:
			
				// yay!
				if ( !bSilent )
				{
					ntPrintf("CELL_NET_CTL_STATE_IPObtained\n");
				}
				g_ShellOptions->m_bNetworkAvailable = true;
				break;
			default:
				break;
			}
		}
	}
	else
	{
		// Currently it's not clear what to do if the network goes down -- looks like most
		// systems won't cope?
	}	
}

//------------------------------------------------------------------------------------------
//!
//!	ShutdownNetwork
//!
//------------------------------------------------------------------------------------------
void ShutdownNetwork()
{
	cellNetCtlTerm();
	sys_net_finalize_network();
	g_ShellOptions->m_bNetworkAvailable = false;
}

//**************************************************************************************
//	reload all elf cmd
//**************************************************************************************
static struct ElfManagerKeyBind
{
	COMMAND_RESULT ElfManager_CommandReload()
	{
		ElfManager::Get().ReloadAll();
		return CR_SUCCESS;
	}
} sElfManager_CommandReload;

//**************************************************************************************
//	register shortcut
//**************************************************************************************
void RegisterElfManagerKeys()
{
	CommandBaseNoInput* pobCommandSPA = CommandManager::Get().CreateCommandNoInput("ElfManagerRegisterKeys", &sElfManager_CommandReload, &ElfManagerKeyBind::ElfManager_CommandReload, "SPU: reload all elf program");
	KeyBindManager::Get().RegisterKeyNoInput("script", pobCommandSPA, "SPU: reload all elf program", KEYS_PRESSED, KEYC_R, KEYM_CTRL | KEYM_ALT);
}

//**************************************************************************************
//	anisotropic filtering bind
//**************************************************************************************
static struct AnisotropicFilteringQualityBind
{
	COMMAND_RESULT ToggleTextureAnisotropicFilterQuality( void )
	{
		int tmp = (CRendererSettings::eAnisotropicFilterQuality + 1) % 3;
		CRendererSettings::eAnisotropicFilterQuality = ANISOTROPIC_FILTERING_QUALITY( tmp );

		switch( CRendererSettings::eAnisotropicFilterQuality )
		{
		case AFQ_Disabled:
			ntPrintf( ": %s (%i)", "disabled",tmp );
			break;
		case AFQ_Low:
			ntPrintf( ": %s (%i)", "low",tmp );
			break;
		case AFQ_High:
			ntPrintf( ": %s (%i)", "high",tmp );
			break;
		}

		return CR_SUCCESS;
	}

} sAnisotropicFilteringQualityBind;

//**************************************************************************************
//	register shortcut
//**************************************************************************************
void RegisterTextureAnisotropicFilteringToggleKey( void )
{
	CommandBaseNoInput* pobCommand = CommandManager::Get().CreateCommandNoInput("Anisotropic Filtering Quality", &sAnisotropicFilteringQualityBind, &AnisotropicFilteringQualityBind::ToggleTextureAnisotropicFilterQuality, "Toggle texture anisotropic filtering");
	KeyBindManager::Get().RegisterKeyNoInput("rendering", pobCommand, "Anisotropic Filtering Quality", KEYS_PRESSED, KEYC_F, KEYM_CTRL | KEYM_SHIFT  );
}

//**************************************************************************************
//	register callback
//**************************************************************************************
static void cb_exitgame( uint64_t status, uint64_t param, void * userdata )
{
	(void)param;
	(void)userdata;

	switch ( status )
	{
	case CELL_SYSUTIL_REQUEST_EXITGAME:
		{
			// need to detect if the other thread is active, and potentially
			// abort the level load to we can speed our progress to the exit

			ntPrintf( "CELL_SYSUTIL_REQUEST_EXITGAME\n" );
			ShellMain::Get().RequestGameExit();
		}
		break;
	
	default:
		{
		ntPrintf( "unknown status 0x%Lx\n", status );
		break;
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::PlatformPreInit
//! Platform specific pre-initialisation
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::PlatformPreInit( ShellMain* pParent )
{
	// Disable Java Assist Exceptions
	vector unsigned short disableJavaExceptions = { 0, 0, 0, 0, 0, 0, 1, 0 };
	vec_mtvscr( vec_or( vec_mfvscr(), disableJavaExceptions ) );

	// now read in our game.config for our setup params
	InitFileSystemAndConfig();

	// ug. cannot setup loggin till after our file system has been setup
	// eek, even worse cant do this when running off HDD till much later
	if (g_ShellOptions->m_bUsingHDD == false)
		pParent->m_pDebugShell->SetupLogging();

	// one for ICE and Scream
	int numSPUThreads = 0;

	// Hopefully this should go down to 0 at some point. :(
	numSPUThreads += AudioSystem::GetSPUThreadCount(); // Reserve a number of threads for audio

	Exec::ReserveSPUThreads( numSPUThreads );

	// ------------------------------------------------------------------------------
	// lets get the debugging stuff working as early as possible
	Debug::Init();

	// ------------------------------------------------------------------------------
#define _0KBYTE		(0)
#define _1KBYTE		(1024<<0)
#define _2KBYTE		(1024<<1)
#define _4KBYTE		(1024<<2)
#define _8KBYTE		(1024<<3)
#define _16KBYTE	(1024<<4)
#define _32KBYTE	(1024<<5)
#define _64KBYTE	(1024<<6)
#define _128KBYTE	(1024<<7)
#define _256KBYTE	(1024<<8)
#define _512KBYTE	(1024<<9)
#define _1024KBYTE	(1024<<10)
#define _2048KBYTE	(1024<<11)
#define _4096KBYTE	(1024<<12)

	// start up the small heap allocator for FwStd to work
	FwSmallHeap::SmallHeapDefinition	definitionTable[] = 
	{
		// this is pretty weak, if we dont specify the
		// byte size, it will silenty allocate from the heap.
		{ 4,	(int)(_1KBYTE / 4),		true	},
		{ 8,	(int)(_0KBYTE / 8),		true	},
		{ 12,	(int)(_0KBYTE / 12),	true	},
		{ 16,	(int)(_0KBYTE / 16),	true	},
		{ 20,	(int)(_0KBYTE / 20),	true	},
		{ 24,	(int)(_8KBYTE / 24),	true	},
		{ 28,	(int)(_0KBYTE / 28),	true	},
		{ 32,	(int)(_8KBYTE / 32),	true	},
		{ 36,	(int)(_0KBYTE / 36),	true	},
		{ 40,	(int)(_8KBYTE / 40),	true	},
		{ 44,	(int)(_0KBYTE / 44),	true	},
		{ 48,	(int)(_8KBYTE / 48),	true	},
		{ 52,	(int)(_0KBYTE / 52),	true	},
		{ 56,	(int)(_8KBYTE / 56),	true	},
		{ 60,	(int)(_0KBYTE / 60),	true	},
		{ 64,	(int)(_8KBYTE / 64),	true	},
		{ 68,	(int)(_0KBYTE / 68),	true	},
		{ 72,	(int)(_4KBYTE / 72),	true	},
		{ 76,	(int)(_0KBYTE / 76),	true	},
		{ 80,	(int)(_8KBYTE / 80),	true	},
		{ 84,	(int)(_0KBYTE / 84),	true	},
		{ 88,	(int)(_0KBYTE / 88),	true	},
		{ 92,	(int)(_0KBYTE / 92),	true	},
		{ 96,	(int)(_0KBYTE / 96),	true	},
		{100,	(int)(_0KBYTE / 100),	true	},
		{104,	(int)(_0KBYTE / 104),	true	},
		{108,	(int)(_0KBYTE / 108),	true	},
		{112,	(int)(_8KBYTE / 112),	true	},
		{116,	(int)(_0KBYTE / 116),	true	},
		{120,	(int)(_4KBYTE / 120),	true	},
		{124,	(int)(_0KBYTE / 124),	true	},
		{128,	(int)(_1KBYTE / 128),	true	},
		{ 0,	0,						true	},
	};

	int iCumulativeHeap = 0;
	int iSlot = 0;

	while(definitionTable[iSlot].m_size != 0)
	{
		iCumulativeHeap += definitionTable[iSlot].m_capacity * definitionTable[iSlot].m_size;
		iSlot++;
	}


	FwSmallHeap::Initialise( definitionTable, false );
	// ------------------------------------------------------------------------------

	ntPrintf("****************************************************\n");
	ntPrintf("* MemMan Base Memory Stats:						  \n");
	ntPrintf("* XDDR Free %d Mbytes\n", Mem::GetBaseMemStats().iInitialXddrFree / Mem::Mb );
	ntPrintf("* GDDR Free %d Mbytes\n", Mem::GetBaseMemStats().iInitialGddrFree / Mem::Mb );
	ntPrintf("* Debug Free %d Mbytes\n", Mem::GetBaseMemStats().iInitialDebugFree / Mem::Mb );
	ntPrintf("****************************************************\n");

	ntPrintf("****************************************************\n");
	ntPrintf("* TOTAL SMALL HEAP ALLOCS: %d Kbytes\n", iCumulativeHeap / 1024 );
	ntPrintf("****************************************************\n");

#ifndef _GOLD_MASTER
	InitialiseNetwork( g_ShellOptions->m_iNetworkAttemptCycles, 500, false);
	if (g_ShellOptions->m_bNetworkAvailable == false)
	{                 
		// see if restarting the network stack helps
		ShutdownNetwork();
		InitialiseNetwork(2,500,false);

		if (g_ShellOptions->m_bNetworkAvailable == false)
		{
			// no? sigh. no welder for this beastie then.
			ntPrintf( "cellNetCtlGetState() failed. We have no IP\n" );
			g_ShellOptions->m_bUseHavokDebugger = false;
		}
	}
#endif

	// register a quit method
	int ret = 0;
	ret = cellSysutilRegisterCallback( 0, cb_exitgame, NULL );
	ntError_p( ret == 0, ("cellSysutilRegisterCallback() = 0x%x\n",ret) );
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::PlatformGraphicsInit
//! Platform specific graphics device initialisation
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::PlatformGraphicsInit()
{
	GraphicsDevice::Get().m_Platform.Init();
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::PlatformPostInit
//! Platform specific post-initialisation
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::PlatformPostInit()
{
#ifndef _GOLD_MASTER
	CSector::Get().RegisterKey();
	RegisterElfManagerKeys();
	RegisterTextureAnisotropicFilteringToggleKey();
#endif
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::PlatformPostDestroy
//! Platform specific post-destructor
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::PlatformPostDestroy()
{
	FwSmallHeap::Shutdown();

#ifndef _GOLD_MASTER
	ShutdownNetwork();
#endif

	ShellOptions::DestroyShellOptions();

	ntPrintf("The End.\n");
	ntPrintf("[[SHELL_EXIT_SUCCESSFUL]]\n");    // picked up by automated tests

	Debug::Kill();

	// Unregister the cb_exitgame callback we register above, in the ctor.
	int ret = cellSysutilUnregisterCallback( 0 );	
	ntError_p( ret == CELL_OK, ("Failed to unregister our quit callback") );
	UNUSED( ret );
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::PlatformUpdate
//! Platform specific Update for globals
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::PlatformUpdate()
{
#ifdef COLLECT_SHADER_STATS
    ResetShaderStats();
#endif

	//char buffer[512];
	//sprintf(buffer, "NumDynamicStrings : %i NumConstStrings : %i Percentage : %f",
	//	CHashedString::s_CountDynamics, CHashedString::s_CountStatics, CHashedString::s_CountStatics / ((float)(CHashedString::s_CountDynamics + CHashedString::s_CountStatics) * 0.01f)); 

	//g_VisualDebug -> Printf2D(10, 10, DC_WHITE, 0, buffer);

	//sprintf(buffer, "NumDynamicSTDStrings : %i NumConstSTDStrings : %i Percentage : %f",
	//	ntstd::String::s_CountDynamics, ntstd::String::s_CountStatics, ntstd::String::s_CountStatics / ((float)(ntstd::String::s_CountDynamics + ntstd::String::s_CountStatics) * 0.01f)); 

	//g_VisualDebug -> Printf2D(10, 25, DC_WHITE, 0, buffer);
	//sprintf(buffer, "VRAM allocs : %d     Host allocs : %d     Free VRAM : %d      Free host : %d",
	//	GcKernel::ms_instance.m_vramRefCount, GcKernel::ms_instance.m_hostMemoryRefCount,
	//	GcKernel::ms_instance.m_freeVram, GcKernel::ms_instance.m_freeHostMemory);
	//g_VisualDebug -> Printf2D(10, 25, DC_WHITE, 0, buffer);

#ifndef _RELEASE
	// Monitor the network to gain an IP address
	UpdateNetworkStatus( true );
#endif

}
