//------------------------------------------------------------------------------------------
//!
//!	\file shellglobal.cpp
//! Object that handles creation of minimal resources required to run a game
//!
//------------------------------------------------------------------------------------------

#include "game/shelldebug.h"
#include "game/shellglobal.h"
#include "game/shelllevel.h"
#include "game/shellmain.h"
#include "game/buildinfo.h"
#include "gfx/display.h"
#include "gfx/graphicsdevice.h"

/***************************************************************************************************
*	Called when an unhandled exception fires
*	Executes a minidump write for postmortem debugging
***************************************************************************************************/
#include "game/minidump_pc.h"
#include <time.h>
#include <direct.h>

LONG WINAPI MyUnhandledExceptionFilter ( EXCEPTION_POINTERS *ExceptionInfo )
{
	MiniDump::BSUMDRET eCCPMD ;

	// make sure we are in the content_pc directory
	Util::SetToPlatformResources();

	// get the user login name
	DWORD iBufferSize = 128;
	char loginname[128];
	GetUserName( loginname, &iBufferSize );

	// get which version of the game we are running
	char basename[128];
	char basefilename[128];
	iBufferSize = 128;
	GetModuleFileName( 0, basefilename, iBufferSize );
	_splitpath( basefilename, 0, 0, basename, 0 );

	// get the time and date
	struct tm *newtime;
	time_t aclock;
	char timename[128];
	time( &aclock );   // Get time in seconds
	newtime = localtime( &aclock );   // Convert time to struct tm form 
	sprintf( timename, "%02d-%02d-%02d_%02d-%02d", newtime->tm_mday, newtime->tm_mon+1, newtime->tm_year-100, newtime->tm_hour, newtime->tm_min );

	// check to see if donkey crashdump storage directory exists
	bool bOnNetwork = true;
	char basedirname[512] = "\\\\mummra\\\\transfer\\\\crash\\";
//	char basedirname[512] = "\\\\dualdean\\transfer\\crash\\";
	char testname[512];
	strcpy( testname, basedirname );
	strcat( testname, "do_not_delete.txt" );
	if( !File::Exists( testname ) )
	{
		bOnNetwork = false;
		strcpy( basedirname, ".\\" );
	}

	// create a directory for this dump
	char dirname[2048];
	sprintf( dirname, "%s%s_%s_%s", basedirname, loginname, timename, basename );
	_mkdir( dirname );
	strcat( dirname, "\\" );

	// create the crash dump
	// create the crash dump

	char large_filename[2048];
	char small_filename[2048];

	strcpy( small_filename, dirname );
	strcat( small_filename, "CrashDump_small.DMP" );
	strcpy( large_filename, dirname );
	strcat( large_filename, "CrashDump.DMP" );

	ntPrintf( Debug::DCU_CORE, "----------------------------------\n" );
	ntPrintf( Debug::DCU_CORE, "--------------CRASH---------------\n" );
	ntPrintf( Debug::DCU_CORE, "Crash dump located at %s\n", small_filename );
	ntPrintf( Debug::DCU_CORE, "--------------CRASH---------------\n" );
	ntPrintf( Debug::DCU_CORE, "----------------------------------\n" );

	eCCPMD = MiniDump::CreateCurrentProcessMiniDump (	MiniDumpNormal,
														small_filename,
														GetCurrentThreadId ( ),
														ExceptionInfo ) ;
	ntAssert ( MiniDump::eDUMP_SUCCEEDED == eCCPMD ) ;



	// copy the exe, pdb and log.txt
	char locfilename[256];
	char filename[2048];
	sprintf( locfilename, ".\\%s.exe", basename );
	sprintf( filename, "%s%s.exe", dirname, basename );
	CopyFile( locfilename, filename, 0 );
	sprintf( locfilename, ".\\%s.pdb", basename );
	sprintf( filename, "%s%s.pdb", dirname, basename );
	CopyFile( locfilename, filename, 0 );

	Debug::FlushLog();

	// release doesn't have a log file, so we would be copying an old one...
#if !defined(_RELEASE)
	sprintf( filename, "%sdebug.log", dirname );
	CopyFile( ".\\debug.log", filename, 0 );
#endif
	strcpy( locfilename, ".\\capture.txt");
	sprintf( filename, "%scapture.txt", dirname );
	CopyFile( locfilename, filename, 0 );

	if( g_message_box_override == false )
	{
		// display a message telling them to report the bug given the folder name
		if( bOnNetwork )
		{
			char msgtext[2048];
			sprintf( msgtext, "The game has crashed\nPlease report the crash quoting the text \"%s_%s_%s\" so we can look into it.\nIt may take some while to complete after use click O.K.\nThanks", loginname, timename, basename );
			MessageBox( 0,  msgtext, "Oh no the game has crashed :-(", 0 );
		} else
		{
			char msgtext[2048];
			sprintf( msgtext, "The game has crashed\nPlease send the \"%s_%s_%s\" directory back to us so we can look into it.\nIt may take some while to complete after use click O.K.\nThanks", loginname, timename, basename );
			MessageBox( 0,  msgtext, "Oh no the game has crashed :-(", 0 );
		}
	}

	// full crash dump take so long we do it last
	eCCPMD = MiniDump::CreateCurrentProcessMiniDump (	MiniDumpWithFullMemory,
														large_filename,
														GetCurrentThreadId ( ),
														ExceptionInfo ) ;
	ntAssert ( MiniDump::eDUMP_SUCCEEDED == eCCPMD ) ;

    return ( EXCEPTION_EXECUTE_HANDLER ) ;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::PlatformPreInit
//! Platform specific pre-initialisation
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::PlatformPreInit( ShellMain* pParent )
{
	SetUnhandledExceptionFilter ( MyUnhandledExceptionFilter ) ;

	pParent->m_pDebugShell->SetupLogging();
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::PlatformGraphicsInit
//! Platform specific graphics device initialisation
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::PlatformGraphicsInit()
{
	DisplayManager::Get().ConfigureDisplay();

	#ifdef _DEBUG
	static const char* pTitle = "Heavenly Sword Prototype" BUILDINFO " [DEBUG]";
	#elif !defined( _RELEASE )
	static const char* pTitle = "Heavenly Sword Prototype" BUILDINFO " [DEVELOPMENT]";
	#else
	static const char* pTitle = "Heavenly Sword Prototype" BUILDINFO;
	#endif

	if (DisplayManager::Get().IsFullScreen())
	{
		GraphicsDevice::Get().m_Platform.InitFullScreen( (int)DisplayManager::Get().GetDeviceWidth(), (int)DisplayManager::Get().GetDeviceHeight(), pTitle );
	}
	else
	{
		GraphicsDevice::Get().m_Platform.InitWindowed( (int)DisplayManager::Get().GetDeviceWidth(), (int)DisplayManager::Get().GetDeviceHeight(), pTitle );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::PlatformPostInit
//! Platform specific post-initialisation
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::PlatformPostInit()
{
	TryInstallXMLFile( "entities/Army/ArmyInterfaces.xml" );
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::PlatformPostDestroy
//! Platform specific post-destructor
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::PlatformPostDestroy()
{
}

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal::PlatformUpdate
//! Platform specific Update for globals
//!
//------------------------------------------------------------------------------------------
void ShellGlobal::PlatformUpdate()
{
	MSG stMsg;
	while(PeekMessage(&stMsg, 0, 0, 0, PM_REMOVE))
	{
		if(stMsg.message == WM_QUIT)
			ShellMain::Get().RequestGameExit();

		TranslateMessage(&stMsg);
		DispatchMessage(&stMsg);
	}
}
