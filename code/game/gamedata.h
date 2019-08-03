//------------------------------------------------------------------------------------------
//!
//!	\file code\game\gamedata.h
//! 
//! Skeleton definition for the GameData utility.
//!
//------------------------------------------------------------------------------------------

#ifdef PLATFORM_PS3

#ifndef _GAMEDATA_H
#define _GAMEDATA_H

#include <sysutil/sysutil_gamedata.h>

class CGameData
{
private:
	enum eGB
	{
		ONE_GB_AS_KB = 1024*1024,
		TWO_GB_AS_KB = ONE_GB_AS_KB*2,
		FOUR_GB_AS_KB = ONE_GB_AS_KB*4,
		FIVE_GB_AS_KB = ONE_GB_AS_KB*5,
		SIX_GB_AS_KB = ONE_GB_AS_KB*6
	};

public:
	// Test modes used in conjunction with the frontend GameData test screen
	enum eTESTMODE
	{
		TESTMODE_NONE,
		TESTMODE_NOGAMEDATA_HAVESPACE,
		TESTMODE_NOGAMEDATA_NOSPACE,
		TESTMODE_GAMEDATA_HAVESPACE,
		TESTMODE_GAMEDATA_NOSPACE
	};

	// Cleanup my test GameData files
	static void CleanUp( void );
	
	// Start the GameData process
	static bool Start( void );

	// Start the GameData process with a test mode
	static bool Start( eTESTMODE eTestMode );

	static bool m_bGameDataTestMode;

private:
	// Real start.
	static bool StartInternal( eTESTMODE eTestMode );
	
	// Callback called by the utility with SFO information
	static void CheckCreateStatusCallback( CellGameDataCBResult *result, CellGameDataStatGet *get, CellGameDataStatSet *set );
	
	// Thread responsible for calling cellGameDataCheckCreate
	static void ThreadGameData( uint64_t arg  );

	// TEMPORARY : File load/save functions, use libfs directly.  These functions will be replaced
	// by calls to NT file system
    static int FileAllocLoad(const char *filePath, void **buf, unsigned int *size);
	static int FileSimpleSave(const char *filePath, void *buf, unsigned int fileSize);
	static void SaveSampleData( void );
	
	// Convert GameData size from bytes to KB.
	static int CalcGameDataSizeKB( void );
	
	// Cap HDD free space.
	static int CapHDDFreeSpaceToTRCLimit( int freeSpace );

	// Copy of info passed back by cellGameDataCheckCreate
	static CellGameDataStatGet sGameDataStatGet;
	
	static unsigned int m_uiGameDataSizeBytes;
	static eTESTMODE m_eTestMode;

	static char m_acSourceDataDirectory[ 255 ];
};

#endif // _GAMEDATA_H

#endif // PLATFORM_PS3
