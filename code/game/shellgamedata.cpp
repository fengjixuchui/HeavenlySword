//------------------------------------------------------------------------------------------
//!
//!	\file shellupdater.cpp
//! main update and render loops for the game
//!
//------------------------------------------------------------------------------------------

#include "shellgamedata.h"
#include "core/wad.h"

#include <sys/ppu_thread.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_gamedata.h>
#include <sysutil/sysutil_sysparam.h>

const char* g_GameDataPath = NULL;

//------------------------------------------------------------------------------------------
//!
//! Data for the PARAM.SFO file.	
//! 
//! PLEASE NOTE, THIS IS ALL TEMPORARY FOR TESTING 
//! PURPOSES AND WILL BE REPLACED WITH PROPER DATA.
//!	
//------------------------------------------------------------------------------------------
#define PARAMSFO_TITLE			"HS SAMPLE GAME DATA"
#define PARAMSFO_TITLE00		"HS SAMPLE GAMEDATA TITLE IN JAPANESE"
#define PARAMSFO_TITLE01		"HS SAMPLE GAMEDATA TITLE IN ENGLISH"
#define PARAMSFO_TITLE02		"HS SAMPLE GAMEDATA TITLE IN FRENCH"
#define PARAMSFO_TITLE03		"HS SAMPLE GAMEDATA TITLE IN SPANISH"
#define PARAMSFO_TITLE04		"HS SAMPLE GAMEDATA TITLE IN GERMAN"
#define PARAMSFO_TITLE05		"HS SAMPLE GAMEDATA TITLE IN ITALIAN"
#define PARAMSFO_TITLE06		"HS SAMPLE GAMEDATA TITLE IN DUTCH"
#define PARAMSFO_TITLE07		"HS SAMPLE GAMEDATA TITLE IN PORTUGESE"
#define PARAMSFO_TITLE08		"HS SAMPLE GAMEDATA TITLE IN RUSSIAN"
#define PARAMSFO_TITLE09		"HS SAMPLE GAMEDATA TITLE IN KOREAN"
#define PARAMSFO_TITLE10		"HS SAMPLE GAMEDATA TITLE IN CHINESE (trad)"
#define PARAMSFO_TITLE11		"HS SAMPLE GAMEDATA TITLE IN CHINESE (sim)"
#define PARAMSFO_TITLEID		"ABCD12345"	// Title product code with hyphen removed.
#define PARAMSFO_VERSION		"01.00"	// Must be in format 00.00
#define PARAMSFO_PARENTALLEV	CELL_SYSUTIL_GAME_PARENTAL_LEVEL01
#define PARAMSFO_ATTRIBUTE		CELL_GAMEDATA_ATTR_NORMAL

#define GAMEDATATHREAD_NAME				"GameDataThread"
#define GAMEDATATHREAD_PRIORITY			1001				// 0 highest to 3071
#define GAMEDATATHREAD_STACKSIZE_BYTES	(32*1024)
#define GAMEDATADIRNAME					PARAMSFO_TITLEID	// Should be the same as PARAMSFO_TITLEID

namespace ShellGameData
{
	static volatile int32_t	s_GameDataOkay = 0;
	static volatile int32_t	s_GameDataFail = 0;

	// Copy of information returned in the SFO, is used to access title directory.
	static CellGameDataStatGet s_GameDataStatGet;

	// path used by FIOS when accessing game data
	static char s_GameDataPath[ 2048 ];

	#define GIGS_IN_KB( gigs )		(1024*1024*gigs)
	#define BYTES_IN_KB( bytes )	((bytes+1023)/1024)

	enum TEST_STATUS
	{
		GD_OK,
		GD_REQUIRE_SPACE,
		GD_DATA_CORRUPT,
		GD_OTHER_ERROR,
	};

	enum ERROR_STATUS
	{
		GD_FAILED_SPACE,
		GD_FAILED_CORRUPT,
		GD_FAILED_OTHER,
	};

	static TEST_STATUS eTestStatus = GD_OK;
	static ERROR_STATUS s_utilStatus;

	//------------------------------------------------------------------------------------------
	//!
	//!	CheckCreateStatusCallback
	//!
	//! GameData status callback, is called as a result of calling cellGameDataCheckCreate.
	//! It will be passed information about existing GameData if found.
	//!
	//! I should check contents of game data when it exists or check space on internal
	//! hard disk when creating new game data.
	//!
	//! When returning from this function the game data utility will terminate.
	//!
	//------------------------------------------------------------------------------------------
	void CheckCreateStatusCallback( CellGameDataCBResult *result, CellGameDataStatGet *get, CellGameDataStatSet *set )
	{
		// get - Holds information obtained by the utility.
		// set - Is used write to the PARAM.SFO.

		ntPrintf( "START - CheckCreateStatusCallback()\n" );

		// Make a copy of the GameData stats for future reference whilst writing files to the
		// gamedata directory.
		memcpy( &s_GameDataStatGet, get, sizeof( CellGameDataStatGet ) );

		// isNewData() will return true if no existing GameData was found.
		if	( get->isNewData )
		{
			// Populate the PARAM.SFO.
			strcpy( get->getParam.title,			PARAMSFO_TITLE );
			strcpy( get->getParam.titleLang[0],		PARAMSFO_TITLE00 );
			strcpy( get->getParam.titleLang[1],		PARAMSFO_TITLE01 );
			strcpy( get->getParam.titleLang[2],		PARAMSFO_TITLE02 );
			strcpy( get->getParam.titleLang[3],		PARAMSFO_TITLE03 );
			strcpy( get->getParam.titleLang[4],		PARAMSFO_TITLE04 );
			strcpy( get->getParam.titleLang[5],		PARAMSFO_TITLE05 );
			strcpy( get->getParam.titleLang[6],		PARAMSFO_TITLE06 );
			strcpy( get->getParam.titleLang[7],		PARAMSFO_TITLE07 );
			strcpy( get->getParam.titleLang[8],		PARAMSFO_TITLE08 );
			strcpy( get->getParam.titleLang[9],		PARAMSFO_TITLE09 );
			strcpy( get->getParam.titleLang[10],	PARAMSFO_TITLE10 );
			strcpy( get->getParam.titleLang[11],	PARAMSFO_TITLE11 );

			strcpy( get->getParam.titleId,			PARAMSFO_TITLEID );
			strcpy( get->getParam.dataVersion,		PARAMSFO_VERSION );

			cellSysutilGetSystemParamInt( CELL_SYSUTIL_SYSTEMPARAM_ID_GAME_PARENTAL_LEVEL, (int*)(&get->getParam.parentalLevel) );
			get->getParam.attribute     = PARAMSFO_ATTRIBUTE;

			// Calc how much space we require and compare with available space.
			// Set value in result which will feed back through cellGameDataCheckCreate.

			// Calc size of GameData, this includes system files and user files.
			int NEW_SIZEKB = get->sysSizeKB + BYTES_IN_KB( Wad::GetRequiredGameDataSize() );

			ntError_p( NEW_SIZEKB <= GIGS_IN_KB(5), ("Requested GameData exceeds TRC limit") );

			// Calc space required
			int NEED_SIZEKB = get->hddFreeSizeKB - NEW_SIZEKB;

			// Deal with not enough space.
			if	( NEED_SIZEKB < 0 )
			{
				result->errNeedSizeKB = NEED_SIZEKB;
				result->result = CELL_GAMEDATA_CBRESULT_ERR_NOSPACE;
				s_utilStatus = GD_FAILED_SPACE;
				return;
			}

			// Fake Test
			if	( eTestStatus == GD_REQUIRE_SPACE )
			{
				result->errNeedSizeKB = BYTES_IN_KB( Wad::GetRequiredGameDataSize() );
				result->result = CELL_GAMEDATA_CBRESULT_ERR_NOSPACE;
				s_utilStatus = GD_FAILED_SPACE;
				return;
			}
		}
		else
		{
			// GameData already exists.

			// We need to validate if our existing WADs have been corrupted somehow
			// and return the appropriate error code
			if ( Wad::AreAnyGameDataWadsCorrupted( get->gameDataPath ) )
			{
				eTestStatus = GD_DATA_CORRUPT;
			}

			// attempt recovery (re- copy over from disk) if that fails, return error
			// ARV TBC: We don't do any recovery atm - returning with GD_DATA_CORRUPT
			// should ask the user to reformat/delete the heavenly-sword game-data
			// section of the hard-drive - they should then be able to play the game
			// again fine. Need to check TRC on this, but I believe this will pass. [ARV].

			if	( eTestStatus == GD_DATA_CORRUPT )
			{
				result->result = CELL_GAMEDATA_CBRESULT_ERR_BROKEN;
				s_utilStatus = GD_FAILED_CORRUPT;
				return;
			}

			// Possible something else may be wrong with our game data here, 
			if	( eTestStatus == GD_OTHER_ERROR )
			{
				result->invalidMsg = "I wonder how this gets translated";
				result->result = CELL_GAMEDATA_CBRESULT_ERR_INVALID;
				s_utilStatus = GD_FAILED_OTHER;
				return;
			}

			// data is present and correct, though we may not have everything we require
			int NEW_SIZEKB =	BYTES_IN_KB( Wad::GetRequiredGameDataSize() ) -
								BYTES_IN_KB( Wad::GetUsedGameDataSize() );
	
			ntError_p( NEW_SIZEKB <= GIGS_IN_KB(5), ("Requested GameData exceeds TRC limit") );
			
			// Calc space required
			int NEED_SIZEKB = get->hddFreeSizeKB - NEW_SIZEKB;

			if	( NEED_SIZEKB < 0 )
			{
				result->errNeedSizeKB = NEED_SIZEKB;
				result->result = CELL_GAMEDATA_CBRESULT_ERR_NOSPACE;				
				s_utilStatus = GD_FAILED_SPACE;
				return;
			}

			// Fake Test
			if	( eTestStatus == GD_REQUIRE_SPACE )
			{
				result->errNeedSizeKB = BYTES_IN_KB( Wad::GetRequiredGameDataSize() );
				result->result = CELL_GAMEDATA_CBRESULT_ERR_NOSPACE;
				s_utilStatus = GD_FAILED_SPACE;
				return;
			}
		}

		// Update PARAM.SFO
		set->setParam = &get->getParam;
		set->reserved = NULL;

		// Set result.
		result->result = CELL_GAMEDATA_CBRESULT_OK;
		result->reserved = NULL;

		ntPrintf( "END - CheckCreateStatusCallback()\n" );
	}

	//------------------------------------------------------------------------------------------
	//!
	//!	ThreadGameData
	//!
	//! cellGameDataCheckCreate must be run from a seperate thread to avoid main thread
	//! disruption
	//!
	//------------------------------------------------------------------------------------------
	void ThreadGameData( uint64_t arg  )
	{
		ntPrintf( "START - ThreadGameData()\n");

		// get our memory container for the thread
		uint32_t memContainer = Mem::MemoryContainer_Create( 3 * Mem::Mb );

		unsigned int retVal = 0;

		ntPrintf( "START - cellGameDataCheckCreate2()" );

		retVal = cellGameDataCheckCreate2(	CELL_GAMEDATA_VERSION_CURRENT,
											GAMEDATADIRNAME,
											CELL_GAMEDATA_ERRDIALOG_ALWAYS,
											CheckCreateStatusCallback,
											memContainer );

		ntPrintf( "END - cellGameDataCheckCreate2() : 0x%x\n", retVal );

		// now we can free memory container
		Mem::MemoryContainer_Destroy( memContainer );

		if	( CELL_GAMEDATA_RET_OK == retVal )
		{
			// Record the game data path for FIOS to use
			ntPrintf( "ThreadGameData() - Success()\n");
			ntPrintf("Place Gamedata files under : %s\n\n", s_GameDataStatGet.gameDataPath);

			strcpy( s_GameDataPath, s_GameDataStatGet.gameDataPath );

			uint32_t game_data_path_len = (uint32_t)strlen( s_GameDataPath );
			ntError( game_data_path_len < 2048 );

			if ( s_GameDataPath[ game_data_path_len - 1 ] != '/' )
				s_GameDataPath[ game_data_path_len ] = '/';

			g_GameDataPath = &s_GameDataPath[ 0 ];

			AtomicSetPlatform( &s_GameDataOkay, 1 );
		}
		else if ( CELL_GAMEDATA_ERROR_CBRESULT == retVal )
		{
			// sysutil will display a error box.
			ntPrintf( "ThreadGameData() - Failure()\n");
			AtomicSetPlatform( &s_GameDataFail, 1 );
		}
		else if ( CELL_GAMEDATA_ERROR_BROKEN == retVal )
		{
			// we've got a corrup param.sfo
			ntError_p( 0, ("Unhandled error condition %x", retVal) )
			s_utilStatus = GD_FAILED_OTHER;
			AtomicSetPlatform( &s_GameDataFail, 1 );
		}
		else
		{
			ntError_p( 0, ("Unhandled error condition %x", retVal) )
			s_utilStatus = GD_FAILED_OTHER;
			AtomicSetPlatform( &s_GameDataFail, 1 );
		}

		ntPrintf( "END - ThreadGameData()\n");

		// Terminates the calling thread
		sys_ppu_thread_exit(0);
	}
};

//------------------------------------------------------------------------------------------
//!
//!	GameDataShellUpdater::ctor
//!
//------------------------------------------------------------------------------------------
GameDataShellUpdater::GameDataShellUpdater() :
	m_pBackup(0),
	m_eStatus( GD_UNKNOWN )
{
	m_pBackup = NT_NEW SimpleShellUpdater( "startup/status_gamedata_check_colour.dds" );
	InitGameData();
}

//------------------------------------------------------------------------------------------
//!
//!	GameDataShellUpdater::dtor
//!
//------------------------------------------------------------------------------------------
GameDataShellUpdater::~GameDataShellUpdater()
{
	NT_DELETE( m_pBackup );
}

//------------------------------------------------------------------------------------------
//!
//!	GameDataShellUpdater::InitGameData
//!
//------------------------------------------------------------------------------------------
void GameDataShellUpdater::InitGameData()
{
	// cellGameDataCheckCreate must be run on a separate thread so not to interupt main thread.
	ntPrintf("START - GameData::InitGameData()\n");

	// cellGameDataCheckCreate must to be executed within a seperate thread to avoid be interupted.
	int retVal;
	sys_ppu_thread_t threadID;
	retVal = sys_ppu_thread_create( &threadID, ShellGameData::ThreadGameData, NULL,
									GAMEDATATHREAD_PRIORITY, GAMEDATATHREAD_STACKSIZE_BYTES, 0, GAMEDATATHREAD_NAME );

	ntError_p( CELL_OK == retVal, ( "GameData thread creation failed with error < %d >\n", retVal ) )	
	ntPrintf("END - GameData::Start()\n");
}

//------------------------------------------------------------------------------------------
//!
//!	GameDataShellUpdater::Update
//!
//------------------------------------------------------------------------------------------
void GameDataShellUpdater::Update()
{
	m_pBackup->Update();

	// pump our system callbacks
	int ret = 0;
	ret = cellSysutilCheckCallback();
	ntError_p(ret==0,("Call to cellSysutilCheckCallback failed! (%d)\n",ret));

	if ( ShellGameData::s_GameDataOkay )
	{
		// we can exit to the next stage
		m_eStatus = GD_SUCCEEDED;

		// have a quick peek to see if we need to do a full or partial install later on.
		Wad::RecordGameDataUsage();
	}
	else if ( ShellGameData::s_GameDataFail )
	{
		// error message will have been displayed by the utility, now 
		// we give them a better explanation and ask them to quit.
		m_eStatus = GD_FAILED;

		// delete our backup screen, then create a new one with the relevant message
		NT_DELETE( m_pBackup );
		
		switch ( ShellGameData::s_utilStatus )
		{
		case ShellGameData::GD_FAILED_SPACE:
			m_pBackup = NT_NEW SimpleShellUpdater( "startup/error_gamedata_space.dds" );
			break;

		case ShellGameData::GD_FAILED_CORRUPT:
			m_pBackup = NT_NEW SimpleShellUpdater( "startup/error_gamedata_corrupt.dds" );
			break;

		case ShellGameData::GD_FAILED_OTHER:
			m_pBackup = NT_NEW SimpleShellUpdater( "startup/error_gamedata_other.dds" );
			break;
		}

		// clear var so we dont do this again, wait for user to quit
		ShellGameData::s_GameDataFail = 0;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	GameDataShellUpdater::Render
//!
//------------------------------------------------------------------------------------------
void GameDataShellUpdater::Render()
{
	m_pBackup->Render();
}
