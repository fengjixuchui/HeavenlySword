/***************************************************************************************************
*
*   $Header:: /game/main.cpp 78    14/08/03 10:48 Dean                                             $
*
*	The program entry point.
*
*	CHANGES		
*
*	05/02/2003	Simon	Created
*	07/08/2003	Wil		Moved contents to shell.cpp
*
***************************************************************************************************/

#include "core/memman.h"
#include "game/shellconfig.h"
#include "game/shellmain.h"


// this only exits on PC, hence the extern rather than header declaration
extern bool SetResourceDirectory( const char* pDir );

/***************************************************************************************************
*
*	FUNCTION		InitialiseCRT
*
*	DESCRIPTION		Initialises the CRT. This only needs to be done once, as the first code called 
*					by the game.
*
***************************************************************************************************/

void InitialiseCRT()
{


	// Once this is done we're allowed to call 'NT_NEW' anywhere..
#ifdef	ATG_MEMORY_DEBUG_ENABLED
	FwMemDebug::Initialise();
#endif

//#ifdef	_DEBUG
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);
//#endif	//_DEBUG

	Util::EnableFPUExceptions();

	// Finally, set our FPU hardware to have traditional/IEEE internal floating point precision.
	_controlfp( _PC_24, _MCW_PC );
}

/***************************************************************************************************
*
*	FUNCTION		main
*
*	DESCRIPTION		The fun starts here :)
*
***************************************************************************************************/

int main(int, char* )
{
	Mem::Init();

	if( !FwMem::IsInitialised() )
	{
		static FwMemConfig g_projectMemConfig = { Mem::ATG_AllocCallback, Mem::ATG_FreeCallback };
		FwMem::Initialise( &g_projectMemConfig );
	}

	// lets get the debugging stuff working as early as possible
	Debug::Init();

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

#define ALLOW_HEAP_OVERFLOW		true

	// start up the small heap allocator for FwStd to work
	FwSmallHeap::SmallHeapDefinition	definitionTable[] = 
	{
		// this is pretty weak, if we dont specify the
		// byte size, it will silenty allocate from the heap.
		{ 4,	(int)(_8KBYTE / 4),		ALLOW_HEAP_OVERFLOW	},
		{ 8,	(int)(_0KBYTE / 8),		ALLOW_HEAP_OVERFLOW	},
		{ 12,	(int)(_4096KBYTE / 12),	ALLOW_HEAP_OVERFLOW	},
		{ 16,	(int)(_16KBYTE / 16),	ALLOW_HEAP_OVERFLOW	},
		{ 20,	(int)(_64KBYTE / 20),	ALLOW_HEAP_OVERFLOW	},
		{ 24,	(int)(_2048KBYTE / 24),	ALLOW_HEAP_OVERFLOW	},
		{ 28,	(int)(_4KBYTE / 28),	ALLOW_HEAP_OVERFLOW	},
		{ 32,	(int)(_4096KBYTE / 32),	ALLOW_HEAP_OVERFLOW	},
		{ 36,	(int)(_512KBYTE / 36),	ALLOW_HEAP_OVERFLOW	},
		{ 40,	(int)(_1KBYTE / 40),	ALLOW_HEAP_OVERFLOW	},
		{ 44,	(int)(_8KBYTE / 44),	ALLOW_HEAP_OVERFLOW	},
		{ 48,	(int)(_1024KBYTE / 48),	ALLOW_HEAP_OVERFLOW	},
		{ 52,	(int)(_1KBYTE / 52),	ALLOW_HEAP_OVERFLOW	},
		{ 56,	(int)(_0KBYTE / 56),	ALLOW_HEAP_OVERFLOW	},
		{ 60,	(int)(_0KBYTE / 60),	ALLOW_HEAP_OVERFLOW	},
		{ 64,	(int)(_0KBYTE / 64),	ALLOW_HEAP_OVERFLOW	},
		{ 68,	(int)(_2KBYTE / 68),	ALLOW_HEAP_OVERFLOW	},
		{ 72,	(int)(_128KBYTE / 72),	ALLOW_HEAP_OVERFLOW	},
		{ 76,	(int)(_0KBYTE / 76),	ALLOW_HEAP_OVERFLOW	},
		{ 80,	(int)(_0KBYTE / 80),	ALLOW_HEAP_OVERFLOW	},
		{ 84,	(int)(_0KBYTE / 84),	ALLOW_HEAP_OVERFLOW	},
		{ 88,	(int)(_0KBYTE / 88),	ALLOW_HEAP_OVERFLOW	},
		{ 92,	(int)(_0KBYTE / 92),	ALLOW_HEAP_OVERFLOW	},
		{ 96,	(int)(_1KBYTE / 96),	ALLOW_HEAP_OVERFLOW	},
		{100,	(int)(_0KBYTE / 100),	ALLOW_HEAP_OVERFLOW	},
		{104,	(int)(_0KBYTE / 104),	ALLOW_HEAP_OVERFLOW	},
		{108,	(int)(_0KBYTE / 108),	ALLOW_HEAP_OVERFLOW	},
		{112,	(int)(_0KBYTE / 112),	ALLOW_HEAP_OVERFLOW	},
		{116,	(int)(_0KBYTE / 116),	ALLOW_HEAP_OVERFLOW	},
		{120,	(int)(_0KBYTE / 120),	ALLOW_HEAP_OVERFLOW	},
		{124,	(int)(_0KBYTE / 124),	ALLOW_HEAP_OVERFLOW	},
		{128,	(int)(_0KBYTE / 128),	ALLOW_HEAP_OVERFLOW	},
		{ 0,	0,						true	},
	};

	int iCumulativeHeap = 0;
	int iSlot = 0;

	while(definitionTable[iSlot].m_size != 0)
	{
		iCumulativeHeap += definitionTable[iSlot].m_capacity * definitionTable[iSlot].m_size;
		iSlot++;
	}
	ntPrintf("****************************************************\n");
	ntPrintf("* MemMan Base Memory Stats:						  \n");
	ntPrintf("* XDDR Free %d Mbytes\n", Mem::GetBaseMemStats().iInitialXddrFree / Mem::Mb );
	ntPrintf("* GDDR Free %d Mbytes\n", Mem::GetBaseMemStats().iInitialGddrFree / Mem::Mb );
	ntPrintf("* Debug Free %d Mbytes\n", Mem::GetBaseMemStats().iInitialDebugFree / Mem::Mb );
	ntPrintf("****************************************************\n");

	ntPrintf("****************************************************\n");
	ntPrintf("* TOTAL SMALL HEAP ALLOCS: %d Kbytes\n", iCumulativeHeap / 1024 );
	ntPrintf("****************************************************\n");

	FwSmallHeap::Initialise( definitionTable, ALLOW_HEAP_OVERFLOW );

	// check for previous instance of this app
	if( FindWindow( "Heavenly Sword", 0 ) )
	{
		MessageBox( 0, 
			"All your base are belong to SaiTong. Please don't run the game twice.", 
			"Hold It!", MB_ICONEXCLAMATION | MB_SYSTEMMODAL 
		);
		return 0;
	}

	// Initialise the CRT
	InitialiseCRT();

	// Initialise the keystrings
	// ALEXEY_TODO : 
	//CHashedString::Initialise();

	// make sure we always run on the same physical processor if
	// we are multicore, as the timer code requires this.
	HANDLE	process = GetCurrentProcess();
	DWORD	processAffinity	= 0;
	DWORD	systemAffinity	= 0;

	if( !GetProcessAffinityMask( process, &processAffinity, &systemAffinity ) )
	{
		ntAssert(0);
	}

	ntAssert( processAffinity & 1 );

	// main thread always on first processor
	processAffinity = 1;

#ifndef BAD_QPF
	if( !SetProcessAffinityMask( process, processAffinity ) )
	{
		ntAssert(0);
	}
#else
	SetThreadAffinityMask( GetCurrentThread(), processAffinity );
#endif
		
	{
		static char errorMSG[4096];

		// read args from game.config (if it exists)
		static char configFile[ MAX_PATH ];
		Util::GetFiosFilePath( "game.config", configFile );

		// pull out our config file or comand line args
		if ( ShellOptions::CreateShellOptions( configFile, errorMSG ) )
		{
			Util::SetPlatformDir( g_ShellOptions->m_contentPlatform.c_str() );
			Util::SetNeutralDir( g_ShellOptions->m_contentNeutral.c_str() );

			if (!SetResourceDirectory( g_ShellOptions->m_contentNeutral.c_str() ))
				return 0;

			// Create the game
			CScopedPtr<ShellMain> pobGame( NT_NEW ShellMain() );

			// Update the game
			while( pobGame->Update() ) {}
		}
		else
		{
			// warning when game.config not found or incorrectly processed.
			// useful when visual studio change the working directory whitout saying anyhting
			MessageBox( 0, errorMSG, "Warning", MB_ICONERROR | MB_SYSTEMMODAL );
		}
	}

	if (g_ShellOptions)
		ShellOptions::DestroyShellOptions();
	FwSmallHeap::Shutdown();

	ntPrintf("The End.\n");
	
	Debug::Kill();

#ifdef ATG_MEMORY_DEBUG_ENABLED

	// Do a memory dump
//	MemReport::ShowCurrent();

#endif

	// Currently cannot do this on dev or rel because of some static 'locale' deleting. sigh.
#if defined(_DEBUG) || defined(_DEBUG_FAST)
	FwMem::Shutdown();
#endif

	Mem::Kill();


	return 0;
}



