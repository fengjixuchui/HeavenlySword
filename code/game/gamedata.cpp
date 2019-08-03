//-----------------------------------------------------------------------------------------
//!
//!	\file game/gamedata.cpp
//!
//! Test implementation for use of the GameData utility.
//!
//!	PLEASE NOTE ALL FILE ACCESS USING LIBFS IS TEMPORARY AND WILL BE REPLACED WITH NT
//! FILE SYSTEM CALLS 
//! 
//! PARAMSFO_TITLEID is also temporary, the product code should be obtained from NT
//! specifics.
//!
//! Add gamedata_test_mode=true to game.config to enable gamedata test screen.
//!
//-----------------------------------------------------------------------------------------

#ifdef PLATFORM_PS3

// Includes
//---------------------------------------------------------------------------------------------------------------------------------
#include "game/gamedata.h"
#include <sys/ppu_thread.h>
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_common.h>
#include <sys/paths.h>
#include <sys/fs.h>
#include <cell/cell_fs.h>
#include "gui/guiinput.h"
#include <sys/sys_time.h>

// Defines
//---------------------------------------------------------------------------------------------------------------------------------

// Test game data file names.
#define GAMEDATA_1		"GAME-DATA1"
#define GAMEDATA_2		"GAME-DATA2"

// Text content information files.
#define ICON0_PNG		"ICON0.PNG"
#define ICON1_PAM		"ICON1.PAM"
#define PIC1_PNG		"PIC1.PNG"


// Static vars
//---------------------------------------------------------------------------------------------------------------------------------
CellGameDataStatGet CGameData::sGameDataStatGet;	// Copy of information returned in the SFO, is used to access title directory.
unsigned int CGameData::m_uiGameDataSizeBytes = 0;
CGameData::eTESTMODE CGameData::m_eTestMode = CGameData::TESTMODE_NONE;	// Test used in conjunction with the front end test screen.
char CGameData::m_acSourceDataDirectory[ 255 ];
bool CGameData::m_bGameDataTestMode = false;	// Are we running game data in test mode. 

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
#define GAMEDATATHREAD_STACKSIZE_BYTES	(16*1024)
#define GAMEDATADIRNAME					PARAMSFO_TITLEID	// Should be the same as PARAMSFO_TITLEID

//------------------------------------------------------------------------------------------
//!
//!	Start the GameData process.
//!
//------------------------------------------------------------------------------------------
bool CGameData::Start( void )
{
	return StartInternal( TESTMODE_NONE );
}

//------------------------------------------------------------------------------------------
//!
//!	Start the GameData process with a test mode.
//!
//------------------------------------------------------------------------------------------
bool CGameData::Start( eTESTMODE eTestMode )
{
	return StartInternal( eTestMode );
}

//------------------------------------------------------------------------------------------
//!
//!	The real start.
//!
//------------------------------------------------------------------------------------------
bool CGameData::StartInternal( eTESTMODE eTestMode )
{
	// cellGameDataCheckCreate must be run on a separate thread so not to interupt main thread.
	ntPrintf("START - GameData::Start()\n");

	Util::SetToPlatformResources();
	Util::GetFullGameDataFilePath( "gamedata", &m_acSourceDataDirectory[0] );
	Util::SetToNeutralResources();

	// Important to disable gui input just in case a system dialog box is displayed.
	// If not disabled input will be processed by both OS and GUI system resulting in
	// cyclic menu option selection.
	CGuiInput::Get().BlockInput( true );

	m_eTestMode = eTestMode;

	// cellGameDataCheckCreate must to be executed within a seperate thread to avoid be interupted.
	int retVal;
	sys_ppu_thread_t threadID;
	retVal = sys_ppu_thread_create( &threadID, ThreadGameData, NULL,
									GAMEDATATHREAD_PRIORITY, GAMEDATATHREAD_STACKSIZE_BYTES, 0, GAMEDATATHREAD_NAME );

	if	( CELL_OK != retVal )
	{
		ntPrintf( "GameData thread creation failed with error < %d >\n", retVal );
		return false;
	}
	
	ntPrintf("END - GameData::Start()\n");

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	ThreadGameData
//!
//! cellGameDataCheckCreate must be run from a seperate thread to avoid main thread
//! disruption/
//!
//------------------------------------------------------------------------------------------
void CGameData::ThreadGameData( uint64_t arg  )
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
		// Save the data here
		ntPrintf( "ThreadGameData() - SaveSampleData()\n");

		SaveSampleData();
	}
	else if ( CELL_GAMEDATA_ERROR_CBRESULT == retVal )
	{
		// sysutil will display a error box.
	}

	// Now safe to enable GUI input.  Remember to update the pad status to make sure
	// previous readings are flushed.
	CGuiInput::Get().BlockInput( false );
	CGuiInput::Get().UpdatePadStatus();

	ntPrintf( "END - ThreadGameData()\n");

	// Terminates the calling thread, ie the one created in CGameData::Save() 
	sys_ppu_thread_exit(0);
}


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
void CGameData::CheckCreateStatusCallback( CellGameDataCBResult *result, CellGameDataStatGet *get, CellGameDataStatSet *set )
{
	// get - Holds information obtained by the utility.
	// set - Is used write to the PARAM.SFO.

	//ntPrintf( "START - CheckCreateStatusCallback()\n" );

	// Make a copy of the GameData stats for future reference whilst writing files to the
	// gamedata directory.
	memcpy( &sGameDataStatGet, get, sizeof( CellGameDataStatGet ) );

	// isNewData() will return true if no existing GameData was found.
	bool bIsNewData = get->isNewData;

	if	( m_bGameDataTestMode )
	{
		if	( ( TESTMODE_NOGAMEDATA_HAVESPACE == m_eTestMode ) || ( TESTMODE_NOGAMEDATA_NOSPACE == m_eTestMode ) )
		{
			bIsNewData = true;
		}
	}

	if	( bIsNewData )
	{
		// Calc how much space we require and compare with available space.
		// Set value in result which will feed back through cellGameDataCheckCreate.
		//ntPrintf( "Creating new GameData < %s >.\n", get->contentInfoPath );

		int NEW_SIZEKB = 0;
		int NEED_SIZEKB = 0;

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

		// Calc size of GameData, this includes system files and user files.
		NEW_SIZEKB += get->sysSizeKB;

		// Get hdd free size but cap to TRC suggestions.
		int hddFreeSizeKB = CapHDDFreeSpaceToTRCLimit( get->hddFreeSizeKB );

		if	( m_bGameDataTestMode )
		{
			if	( TESTMODE_NOGAMEDATA_HAVESPACE == m_eTestMode )
			{
				NEW_SIZEKB += ONE_GB_AS_KB;
			}
			else if ( TESTMODE_NOGAMEDATA_NOSPACE == m_eTestMode )
			{
				NEW_SIZEKB += SIX_GB_AS_KB;
			}

			// Calc space required but not available.
			NEED_SIZEKB = hddFreeSizeKB - NEW_SIZEKB;

			//ntPrintf("hddFreeSizeKB=[%d] NEW_SIZEKB=[%d] NEED_SIZEKB=[%d]\n", hddFreeSizeKB, NEW_SIZEKB, NEED_SIZEKB);
		}
		else
		{
			// Calc space required but not available.
			NEED_SIZEKB = hddFreeSizeKB - NEW_SIZEKB;

			//ntPrintf("hddFreeSizeKB=[%d] NEW_SIZEKB=[%d] NEED_SIZEKB=[%d]\n", get->hddFreeSizeKB, NEW_SIZEKB, NEED_SIZEKB);
		}

		// Deal with not enough space.
		if	( NEED_SIZEKB < 0 )
		{
			result->errNeedSizeKB = NEED_SIZEKB;
			result->result = CELL_GAMEDATA_CBRESULT_ERR_NOSPACE;
			
			//ntPrintf("HDD size check error. needs %d KB disc space more.\n", result->errNeedSizeKB * -1 );
			return;
		}
	}
	else
	{
		// GameData already exists.

		//ntPrintf("GameData %s already exists.\n", get->contentInfoPath );

		int CURRENT_SIZEKB = 0;
		int MINIMUM_SIZEKB = 0;

		// Get how much space is free on hard disc.
		CURRENT_SIZEKB = get->hddFreeSizeKB;

		// Cap free space on hdd as per TRC.
		CURRENT_SIZEKB = CapHDDFreeSpaceToTRCLimit( CURRENT_SIZEKB );

		MINIMUM_SIZEKB = CURRENT_SIZEKB;

		if	( m_bGameDataTestMode )
		{
			if	( TESTMODE_GAMEDATA_HAVESPACE == m_eTestMode )
			{
				MINIMUM_SIZEKB -= ONE_GB_AS_KB;
			}
			else if ( TESTMODE_GAMEDATA_NOSPACE == m_eTestMode )
			{
				MINIMUM_SIZEKB -= SIX_GB_AS_KB;
			}
		}

		if	( MINIMUM_SIZEKB < 0 )
		{
			result->errNeedSizeKB = MINIMUM_SIZEKB;
			result->result = CELL_GAMEDATA_CBRESULT_ERR_NOSPACE;
			
			//ntPrintf("HDD size check error. needs %d KB disc space more.\n", result->errNeedSizeKB * -1 );
			return;
		}
	}

	// Update PARAM.SFO
	set->setParam = &get->getParam;
	set->reserved = NULL;

	// Set result.
	result->result = CELL_GAMEDATA_CBRESULT_OK;
	result->reserved = NULL;

	//ntPrintf( "END - CheckCreateStatusCallback()\n" );
}

//------------------------------------------------------------------------------------------
//!
//!	FileAllocLoad
//!
//! Temporary function for loading files.
//!
//------------------------------------------------------------------------------------------
int CGameData::FileAllocLoad(const char *filePath, void **buf, unsigned int *size)
{
	int ret;
	int fd;
	CellFsStat status;
	uint64_t readlen;

	ret = cellFsOpen(filePath, CELL_FS_O_RDONLY, &fd, NULL, 0);
	if(ret != CELL_FS_SUCCEEDED){
		ntPrintf("file %s open error : 0x%x\n", filePath, ret);
		return -1;
	}

	ret = cellFsFstat(fd, &status);
	if(ret != CELL_FS_SUCCEEDED){
		ntPrintf("file %s get stat error : 0x%x\n", filePath, ret);
		cellFsClose(fd);
		return -1;
	}

	*buf = malloc( status.st_size );
	if( *buf == NULL ) {
		ntPrintf("alloc failed\n");
		cellFsClose(fd);
		return -1;
	}

	ret = cellFsRead(fd, *buf, status.st_size, &readlen);
	if(ret != CELL_FS_SUCCEEDED || status.st_size != readlen ) {
		ntPrintf("file %s read error : 0x%x\n", filePath, ret);
		cellFsClose(fd);
		free(*buf);
		*buf = NULL;
		return -1;
	}

	ret = cellFsClose(fd);
	if(ret != CELL_FS_SUCCEEDED){
		ntPrintf("file %s close error : 0x%x\n", filePath, ret);
		free(*buf);
		*buf = NULL;
		return -1;
	}

	*size = status.st_size;

	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	FileSimpleSave
//!
//! Temporary function for saving files.
//!
//------------------------------------------------------------------------------------------
int CGameData::FileSimpleSave(const char *filePath, void *buf, unsigned int fileSize)
{
	int ret;
	int fd;
	uint64_t writelen;

	if( buf == NULL ) {
		ntPrintf("buffer is null\n");
	}

	ret = cellFsOpen(filePath, CELL_FS_O_WRONLY|CELL_FS_O_CREAT|CELL_FS_O_TRUNC, &fd, NULL, 0);
	if(ret != CELL_FS_SUCCEEDED){
		ntPrintf("file %s open error : 0x%x\n", filePath, ret);
		return -1;
	}

	ret = cellFsWrite(fd, buf, fileSize, &writelen);
	if(ret != CELL_FS_SUCCEEDED || fileSize != writelen ) {
		ntPrintf("file %s read error : 0x%x\n", filePath, ret);
		cellFsClose(fd);
		return -1;
	}

	ret = cellFsClose(fd);
	if(ret != CELL_FS_SUCCEEDED){
		ntPrintf("file %s close error : 0x%x\n", filePath, ret);
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	SaveSampleData
//!
//! Temporary function for saving the GameData files.
//!
//------------------------------------------------------------------------------------------
void CGameData::SaveSampleData( void )
{
	int ret ;
	void *_file_buffer = NULL;
	unsigned int fsize;
	char filePath_I[CELL_FS_MAX_FS_PATH_LENGTH];
	char filePath_O[CELL_FS_MAX_FS_PATH_LENGTH];

	const char *fileList[] = {
		GAMEDATA_1,
		GAMEDATA_2
	};
	int cnt = sizeof(fileList)/sizeof(char *);
	int i = 0 ;
	for(i=0 ; i < cnt ; i++){
		snprintf(filePath_I, CELL_FS_MAX_FS_PATH_LENGTH, "%s/%s", &m_acSourceDataDirectory[0], fileList[i]);
		ret = FileAllocLoad(filePath_I, &_file_buffer, &fsize);

		if(ret == 0){
			snprintf(filePath_O, CELL_FS_MAX_FS_PATH_LENGTH, "%s/%s", sGameDataStatGet.gameDataPath, fileList[i]);
			FileSimpleSave(filePath_O, _file_buffer, fsize);
		}
		if(_file_buffer){
			free(_file_buffer) ;
			_file_buffer = NULL ;
		}
	}

	const char *fileList2[] = {
		ICON0_PNG,
		ICON1_PAM,
		PIC1_PNG
	};
	cnt = sizeof(fileList2)/sizeof(char *);
	for(i=0 ; i < cnt ; i++){
		snprintf(filePath_I, CELL_FS_MAX_FS_PATH_LENGTH, "%s/%s", &m_acSourceDataDirectory[0], fileList2[i]);
		ntPrintf( "filePath_I = %s.\n", filePath_I );

		ret = FileAllocLoad(filePath_I, &_file_buffer, &fsize);

		if(ret == 0){
			snprintf(filePath_O, CELL_FS_MAX_FS_PATH_LENGTH, "%s/%s", sGameDataStatGet.contentInfoPath, fileList2[i]);
			FileSimpleSave(filePath_O, _file_buffer, fsize);
		}
		if(_file_buffer){
			free(_file_buffer) ;
			_file_buffer = NULL ;
		}
	}

}

//------------------------------------------------------------------------------------------
//!
//!	CleanUp
//!
//! Temporary function for removing my GameData files from the test USRDIR.
//!
//------------------------------------------------------------------------------------------
void CGameData::CleanUp( void )
{
/*
	ntPrintf( "START - CGameData::CleanUp\n" );

	int fileDescriptor = 0;
	CellFsErrno fsError;
	int gameDataPathLength;

	char gameDataPath[ CELL_FS_MAX_FS_PATH_LENGTH ] = {SYS_DEV_HDD0"/game/NTHS00003/USRDIR"};
	gameDataPathLength = strlen( gameDataPath );
	ntPrintf( "gameDataPath is < %s >\n", gameDataPath );

	fsError = cellFsOpendir( gameDataPath, &fileDescriptor );
	if	( CELL_FS_SUCCEEDED == fsError )
	{
		CellFsDirent dirEntry;
		uint64_t bytesRead;
		char Filename[ CELL_FS_MAX_FS_FILE_NAME_LENGTH ];

		while	( true )
		{
			fsError = cellFsReaddir( fileDescriptor, &dirEntry, &bytesRead );

			// cellFsReaddir returns success when no more files to read, so just check
			// for filename length instead.
			//if	( CELL_FS_SUCCEEDED == fsError )
			if	( dirEntry.d_namlen > 0 )
			{
				if	( CELL_FS_TYPE_REGULAR == dirEntry.d_type )
				{
					// NOTE : dirEntry.d_name does not include, terminator.
					sprintf( Filename, "%s%s", gameDataPath, dirEntry.d_name );
					Filename[ gameDataPathLength + dirEntry.d_namlen ] = '\0';
					
					ntPrintf( "Attempting to unlink < %s >\n", Filename );
					fsError = cellFsUnlink( Filename );
					if	( CELL_FS_SUCCEEDED != fsError )
					{
						ntPrintf( "cellFsUnlink failed for < %s >\n", Filename );
					}
				}
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		ntPrintf( "Directory not found < %s >\n", gameDataPath );
	}

	ntPrintf( "END - CGameData::CleanUp\n" );
*/
}

//------------------------------------------------------------------------------------------
//!
//!	CalcGameDataSizeKB
//!
//! Calc KB as suggested in the docs.
//!
//------------------------------------------------------------------------------------------
int CGameData::CalcGameDataSizeKB( void )
{
int gameDataSizeKB;

	gameDataSizeKB = ( m_uiGameDataSizeBytes + 1023 ) / 1024;

	return gameDataSizeKB;
}

//------------------------------------------------------------------------------------------
//!
//!	CapHDDFreeSpaceToTRCLimit
//!
//! TRC suggests 5GB limit for GameData so cap the freeSpace param and return.
//!
//------------------------------------------------------------------------------------------
int CGameData::CapHDDFreeSpaceToTRCLimit( int freeSpace )
{
int cappedFreeSpace = freeSpace;

	if	( cappedFreeSpace > FIVE_GB_AS_KB )
	{
		cappedFreeSpace = FIVE_GB_AS_KB;
	}

	return cappedFreeSpace;
}

#endif // PLATFORM_PS3
