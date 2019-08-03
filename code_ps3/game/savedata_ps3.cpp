//------------------------------------------------------------------------------------------
//!
//!	\file savedata_ps3.cpp
//!
//------------------------------------------------------------------------------------------

// Includes
#include "game/savedata_ps3.h"

#include "objectdatabase/dataobject.h"
#include "exec/ppu/exec_ps3.h"
#include "core/io_ps3.h"
#include "hud/hudmanager.h"
#include <sys/ppu_thread.h>


// Directory names
// TODO - the first 9 digits MUST be the game's product code without the hyphen e.g.
// HVSW-12345 becomes HVSW12345, then add 22 characters of our choosing ('-' and '_' allowed)
#define SAVE_DATA_DIR_GAME_SAVE					"ABCD12345-GAMEAUTOSAVE"
#define SAVE_DATA_DIR_GAME_OPTIONS				"ABCD12345-OPTIONSAUTOSAVE"

// Parameters
#define SDU_PARAM_TITLE							"Heavenly Sword"
#define SDU_PARAM_SUBTITLE						""
#define SDU_PARAM_DETAIL_CHECKPOINTS			"Heavenly Sword Saved Game. v1.0"
#define SDU_PARAM_DETAIL_GAME_OPTIONS			"Heavenly Sword Game Options. v1.0"
#define SDU_PARAM_PARENTAL_LEVEL				11

// Filename of the userdata
#define SAVE_DATA_SECURE_USER_DATA_FILENAME		"USERDATA.DAT"

// Platform specific paths to the required save data content files
#define SAVE_DATA_CONTENT_ICON0_PATH			"data/savefiles/icon0.png"
#define SAVE_DATA_CONTENT_ICON1_PATH			"data/savefiles/icon1.pam"
#define SAVE_DATA_CONTENT_PIC1_PATH				"data/savefiles/pic1.png"
#define SAVE_DATA_CONTENT_SND0_PATH				"data/savefiles/snd0.at3"

// Minimum file size read (at least 32K), otherwise SDU returns CELL_SAVEDATA_ERROR_INTERNAL
#define SDU_MINIMUM_FILE_SIZE_READ				( 32 * 1024 )

// Thread details
#define SAVE_DATA_THREAD_NAME					"SaveDataThread"
#define LOAD_DATA_THREAD_NAME					"LoadDataThread"
#define SDU_THREAD_PRIORITY						1500	// 0 highest to 3071

// Enumerator for files being written/read
typedef enum CONTENT_FILES
{
	SAVE_DATA_CONTENT_FILE_ICON0,
	SAVE_DATA_CONTENT_FILE_ICON1,
	SAVE_DATA_CONTENT_FILE_PIC1,
	SAVE_DATA_CONTENT_FILE_SND0,
	SAVE_DATA_CONTENT_FILE_SECURE_USER_DATA,
	SAVE_DATA_CONTENT_FILE_INDEX_END
};

// Secure File ID (16 bytes)
const char secureFileId[CELL_SAVEDATA_SECUREFILEID_SIZE] = {
	0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF
};


// Static Data initialisation
SDU_Process_Info	SaveData::m_ProcessData[SAVE_DATA_TYPE_NUM];
CHudUnitDef*		SaveData::m_pSavingIconRenderDef = 0;
CHudUnit*			SaveData::m_pSavingIconHudUnit = 0;
int					SaveData::m_iSavingIconRefCount = 0;


/***************************************************************************************************
*
*	FUNCTION		SDU_Process_Info::SDU_Process_Info
*
*	DESCRIPTION		Constructor
*	
***************************************************************************************************/
SDU_Process_Info::SDU_Process_Info()
{
	// Set defaults and pointers to 0
	m_eCurrentMode					= SDU_MODE_IDLE;
	m_eCurrentSaveStep				= SAVE_OPERATION_STEP_END;
	m_eCurrentLoadStep				= LOAD_OPERATION_STEP_END;
	m_pBuffer						= 0;
	m_iBufferSize					= 0;
	m_iBufferSizeUsed				= 0;
	m_pFileBuffer					= 0;
	m_pUserDataInternalBuffer		= 0;
	m_iUserDataInternalBufferSize	= 0;		
	m_pUserCallbackFunc				= 0;
	strcpy(m_acDirName, "");

	strcpy(m_obDataParams.m_acTitle, "");
	strcpy(m_obDataParams.m_acSubtitle, "");
	strcpy(m_obDataParams.m_acDetail, "");
	m_obDataParams.m_uParentalLevel	= 11;
	m_obDataParams.m_uAttribute		= CELL_SAVEDATA_ATTR_NODUPLICATE;
}


/***************************************************************************************************
*
*	FUNCTION		SDU_Process_Info::~SDU_Process_Info
*
*	DESCRIPTION		Destructor, the arrays should be destroyed, but make sure here
*	
***************************************************************************************************/
SDU_Process_Info::~SDU_Process_Info()
{
	if ( m_pUserDataInternalBuffer )
	{
		NT_DELETE_ARRAY_CHUNK(Mem::MC_MISC, (char*)m_pUserDataInternalBuffer );
		m_pUserDataInternalBuffer = 0;
	}

	if ( m_pFileBuffer )
	{
		NT_DELETE_ARRAY_CHUNK(Mem::MC_MISC, (char*)m_pFileBuffer );
		m_pFileBuffer = 0;
	}
}


/***************************************************************************************************
*
*	FUNCTION		SDU_Process_Info::SDU_Data_Params::SetParams
*
*	DESCRIPTION		Sets the params
*	
***************************************************************************************************/
void SDU_Process_Info::SDU_Data_Params::SetParams( const char* pcTitle, const char* pcSubtitle, const char* pcDetail, unsigned int uParentalLevel, unsigned int uAttribute )
{
	ntAssert( pcTitle );
	ntAssert( pcSubtitle );
	ntAssert( pcDetail );

	int iTitleLen = strlen( pcTitle );
	int iSubtitleLen = strlen( pcSubtitle );
	int iDetailLen = strlen( pcDetail );

	// Make sure we don't overrun the buffers
	int iTitleLengthToUse = ( iTitleLen > (CELL_SAVEDATA_SYSP_TITLE_SIZE - 2) ) ? (CELL_SAVEDATA_SYSP_TITLE_SIZE - 2) : iTitleLen;
	int iSubtitleLengthToUse = ( iSubtitleLen > (CELL_SAVEDATA_SYSP_SUBTITLE_SIZE - 2) ) ? (CELL_SAVEDATA_SYSP_SUBTITLE_SIZE - 2) : iSubtitleLen;
	int iDetailLengthToUse = ( iDetailLen > (CELL_SAVEDATA_SYSP_DETAIL_SIZE - 2) ) ? (CELL_SAVEDATA_SYSP_DETAIL_SIZE - 2) : iDetailLen;

	// Store params
	strncpy( m_acTitle, pcTitle, iTitleLengthToUse );
	strncpy( m_acSubtitle, pcSubtitle, iSubtitleLengthToUse );
	strncpy( m_acDetail, pcDetail, iDetailLengthToUse );

	// Store attributes
	m_uParentalLevel = uParentalLevel;
	m_uAttribute = uAttribute;
}


/***************************************************************************************************
*
*	FUNCTION		SDU_Process_Info::CommonSetup
*
*	DESCRIPTION		Sets the basic information and buffers that are common to both loading and saving
*	
***************************************************************************************************/
void SDU_Process_Info::CommonSetup( void* pBuffer, int iBufferSize, SaveDataCallback pCallbackFunc, SAVE_DATA_TYPE eSaveDataType )
{
	ntAssert( pBuffer );
	ntAssert( iBufferSize > 0 );
	ntAssert( pCallbackFunc );
	ntAssert( eSaveDataType >= 0 && eSaveDataType < SAVE_DATA_TYPE_NUM );

	// Copy in parameters
	m_pBuffer = pBuffer;
	m_iBufferSize = iBufferSize;
	m_pUserCallbackFunc = pCallbackFunc;

	// Setup generic info
	m_iBufferSizeUsed = 0;
	m_pFileBuffer = 0;

	// Create the internal buffer which must be 32K minimum
	ntAssert( m_pUserDataInternalBuffer == 0 );
	m_iUserDataInternalBufferSize = (iBufferSize < SDU_MINIMUM_FILE_SIZE_READ) ? SDU_MINIMUM_FILE_SIZE_READ : iBufferSize;
	m_pUserDataInternalBuffer = (void*)NT_NEW_ARRAY_CHUNK(Mem::MC_MISC) char[m_iUserDataInternalBufferSize];

	// Clear internal buffer
	memset(m_pUserDataInternalBuffer, 0, m_iUserDataInternalBufferSize);

	// Set data specific info
	switch( eSaveDataType )
	{
		case SAVE_DATA_TYPE_GAMESAVE:
		{
			// Specific Params
			m_obDataParams.SetParams( SDU_PARAM_TITLE, SDU_PARAM_SUBTITLE, SDU_PARAM_DETAIL_CHECKPOINTS, SDU_PARAM_PARENTAL_LEVEL, CELL_SAVEDATA_ATTR_NODUPLICATE );

			// Set the directory we are dealing with to the game options one
			strcpy( m_acDirName, SAVE_DATA_DIR_GAME_SAVE );
		}
		break;

		case SAVE_DATA_TYPE_GAMEOPTIONS:
		{
			// Specific Params
			m_obDataParams.SetParams( SDU_PARAM_TITLE, SDU_PARAM_SUBTITLE, SDU_PARAM_DETAIL_GAME_OPTIONS, SDU_PARAM_PARENTAL_LEVEL, CELL_SAVEDATA_ATTR_NODUPLICATE );
			
			// Set the directory we are dealing with to the game options one
			strcpy( m_acDirName, SAVE_DATA_DIR_GAME_OPTIONS );
		}
		break;

		default:
		{
			ntAssert_p( 0, ("Unknown save data type\n") );
		}
		break;
	}
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::LoadInData
*
*	DESCRIPTION		Sets up the load, and kicks off the loading sub thread
*	
***************************************************************************************************/
bool SaveData::LoadInData( void* pBuffer, int iBufferSize, SaveDataCallback pCallbackFunc, SAVE_DATA_TYPE eSaveDataType )
{
#ifdef USE_SAVE_DATA

	// Check if the system is currently loading/saving this save data type
	if ( m_ProcessData[eSaveDataType].m_eCurrentMode != SDU_MODE_IDLE )
	{
		return false;
	}

	// Setup the basic data
	m_ProcessData[eSaveDataType].CommonSetup( pBuffer, iBufferSize, pCallbackFunc, eSaveDataType );

	// Set data specific info
	switch( eSaveDataType )
	{
		case SAVE_DATA_TYPE_GAMESAVE:
		{
			// Cell SDU Callback functions
			m_ProcessData[eSaveDataType].m_pDataStatusCallback = CallbackDataStatusLoadCheckpoints;
			m_ProcessData[eSaveDataType].m_pFileCallback = CallbackDataFileLoadCheckpoints;
		}
		break;

		case SAVE_DATA_TYPE_GAMEOPTIONS:
		{
			// Cell SDU Callback functions
			m_ProcessData[eSaveDataType].m_pDataStatusCallback = CallbackDataStatusLoadGameOptions;
			m_ProcessData[eSaveDataType].m_pFileCallback = CallbackDataFileLoadGameOptions;
		}
		break;

		default:
		{
			ntAssert_p( 0, ("Unknown save data type\n") );
		}
		break;
	}

	ntPrintf("------------------------------\n" \
			 "SaveData: Starting Loading Thread\n" \
			 "------------------------------\n");

	// Set the mode to loading
	m_ProcessData[eSaveDataType].m_eCurrentMode = SDU_MODE_LOADING;

	// Do the loading on a separate thread
	Exec::CreatePPUThread( &ThreadAutoLoad, (uint64_t)eSaveDataType, SDU_THREAD_PRIORITY, LOAD_DATA_THREAD_NAME );

	return true;

#else
	return false;
#endif
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::SaveOutData
*
*	DESCRIPTION		Sets up the save, and kicks off the saving sub thread
*	
***************************************************************************************************/
bool SaveData::SaveOutData( void* pBuffer, int iBufferSize, SaveDataCallback pCallbackFunc, SAVE_DATA_TYPE eSaveDataType )
{
#ifdef USE_SAVE_DATA

	// Check if the system is currently loading/saving this save data type
	if ( m_ProcessData[eSaveDataType].m_eCurrentMode != SDU_MODE_IDLE )
	{
		return false;
	}

	// Setup the basic data
	m_ProcessData[eSaveDataType].CommonSetup( pBuffer, iBufferSize, pCallbackFunc, eSaveDataType );

	// Copy the data to save into the internal buffer
	if ( iBufferSize < m_ProcessData[eSaveDataType].m_iUserDataInternalBufferSize )
	{
		memcpy( m_ProcessData[eSaveDataType].m_pUserDataInternalBuffer , pBuffer, iBufferSize );
	}
	else
	{
		memcpy( m_ProcessData[eSaveDataType].m_pUserDataInternalBuffer, pBuffer, m_ProcessData[eSaveDataType].m_iUserDataInternalBufferSize );
	}

	// Set data specific info
	switch( eSaveDataType )
	{
		case SAVE_DATA_TYPE_GAMESAVE:
		{
			// Cell SDU Callback functions
			m_ProcessData[eSaveDataType].m_pDataStatusCallback = CallbackDataStatusSaveCheckpoints;
			m_ProcessData[eSaveDataType].m_pFileCallback = CallbackDataFileSaveCheckpoints;
		}
		break;

		case SAVE_DATA_TYPE_GAMEOPTIONS:
		{
			// Cell SDU Callback functions
			m_ProcessData[eSaveDataType].m_pDataStatusCallback = CallbackDataStatusSaveGameOptions;
			m_ProcessData[eSaveDataType].m_pFileCallback = CallbackDataFileSaveGameOptions;
		}
		break;

		default:
		{
			ntAssert_p( 0, ("Unknown save data type\n") );
		}
		break;
	}

	ntPrintf("------------------------------\n" \
			 "SaveData: Starting Saving Thread\n" \
			 "------------------------------\n");

	// Set the mode to saving
	m_ProcessData[eSaveDataType].m_eCurrentMode = SDU_MODE_SAVING;

	// Do the saving on a separate thread
	Exec::CreatePPUThread( &ThreadAutoSave, (uint64_t)eSaveDataType, SDU_THREAD_PRIORITY, SAVE_DATA_THREAD_NAME );

	return true;

#else
	return false;
#endif
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::ThreadAutoSave
*
*	DESCRIPTION		Auto Save function that is called in a separate thread
*	
***************************************************************************************************/
void SaveData::ThreadAutoSave( uint64_t arg )
{
#ifdef USE_SAVE_DATA

	// allocate a memory container of the appropriate size
	uint32_t memContainer = Mem::MemoryContainer_Create( 4*Mem::Mb );

	// Cast the thread argument to the data type
	SAVE_DATA_TYPE eDataType = (SAVE_DATA_TYPE)arg;

	// Setup the cell buffer
	CellSaveDataSetBuf bufferSettings;

	bufferSettings.dirListMax = 0;
	bufferSettings.fileListMax = SAVE_DATA_CONTENT_FILE_INDEX_END;
	memset( bufferSettings.reserved, 0, sizeof( bufferSettings.reserved ) );
	bufferSettings.bufSize = SAVE_DATA_CONTENT_FILE_INDEX_END * sizeof(CellSaveDataFileStat);
	bufferSettings.buf = (void*)NT_NEW_ARRAY_CHUNK(Mem::MC_MISC) char[bufferSettings.bufSize];

	// Call the Cell Auto Save function, this will not return until it is complete!
	int ret = cellSaveDataAutoSave(
		CELL_SAVEDATA_VERSION_CURRENT,
		m_ProcessData[eDataType].m_acDirName,
		CELL_SAVEDATA_ERRDIALOG_NONE,
		&bufferSettings,
		m_ProcessData[eDataType].m_pDataStatusCallback,
		m_ProcessData[eDataType].m_pFileCallback,
		memContainer
		);

	// Free the container
	Mem::MemoryContainer_Destroy( memContainer );
	
	bool bSuccessful = true;
	if ( ret != CELL_SAVEDATA_RET_OK )
	{
		ntPrintf("ThreadAutoSave failed with error code < %x >\n", ret);
		bSuccessful = false;
	}

	ntPrintf("------------------------------\n" \
			 "SaveData: Saving Finished\n" \
			 "------------------------------\n");
	
	// Call the user's callback function indicating the outcome
	if ( m_ProcessData[eDataType].m_pUserCallbackFunc )
	{
		m_ProcessData[eDataType].m_pUserCallbackFunc( bSuccessful, m_ProcessData[eDataType].m_iBufferSizeUsed );
	}

	// We've finished so set the state to idle
	m_ProcessData[eDataType].m_eCurrentMode = SDU_MODE_IDLE;

	// NOTE: This is the last point before the thread ends.
	// So make sure all memory allocated by the class is freed up.

	// Free up the buffer settings buffers...
	if ( bufferSettings.buf )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, (char*)bufferSettings.buf );
	}

	// Free up buffers used in the saving process
	if (m_ProcessData[eDataType]. m_pFileBuffer )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, (char*)m_ProcessData[eDataType].m_pFileBuffer );
		m_ProcessData[eDataType].m_pFileBuffer = 0;
	}

	if ( m_ProcessData[eDataType].m_pUserDataInternalBuffer )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, (char*)m_ProcessData[eDataType].m_pUserDataInternalBuffer );
		m_ProcessData[eDataType].m_pUserDataInternalBuffer = 0;
		m_ProcessData[eDataType].m_iUserDataInternalBufferSize = 0;
	}

	sys_ppu_thread_exit( 0 );

#endif
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::ThreadAutoLoad
*
*	DESCRIPTION		Auto Load function that is called in a separate thread
*	
***************************************************************************************************/
void SaveData::ThreadAutoLoad( uint64_t arg )
{
#ifdef USE_SAVE_DATA

	// allocate a memory container of the appropriate size
	uint32_t memContainer = Mem::MemoryContainer_Create( 4*Mem::Mb );

	//ntPrintf("SAVEDATA: MEMORY CONTAINER %u CREATED\n", memContainer);

	// Cast the thread argument to the data type
	SAVE_DATA_TYPE eDataType = (SAVE_DATA_TYPE)arg;

	// Setup the cell buffer
	CellSaveDataSetBuf bufferSettings;

	bufferSettings.dirListMax = 0;
	bufferSettings.fileListMax = SAVE_DATA_CONTENT_FILE_INDEX_END;
	memset( bufferSettings.reserved, 0, sizeof( bufferSettings.reserved ) );
	bufferSettings.bufSize = SAVE_DATA_CONTENT_FILE_INDEX_END * sizeof(CellSaveDataFileStat);
	bufferSettings.buf = (void*)NT_NEW_ARRAY_CHUNK(Mem::MC_MISC) char[bufferSettings.bufSize];

	// Call the Cell Auto Load function, this will not return until it is complete!
	int ret = cellSaveDataAutoLoad(
		CELL_SAVEDATA_VERSION_CURRENT,
		m_ProcessData[eDataType].m_acDirName,
		CELL_SAVEDATA_ERRDIALOG_NONE,
		&bufferSettings,
		m_ProcessData[eDataType].m_pDataStatusCallback,
		m_ProcessData[eDataType].m_pFileCallback,
		memContainer );

	// Free the container
	Mem::MemoryContainer_Destroy( memContainer );

	bool bSuccessful = true;
	if ( ret != CELL_SAVEDATA_RET_OK )
	{
		ntPrintf("ThreadAutoLoad failed with error code < %x >\n", ret);
		bSuccessful = false;
	}

	ntPrintf("------------------------------\n" \
			 "SaveData: Loading Finished\n" \
			 "------------------------------\n");
	
	// Call the user's callback function indicating the outcome
	if ( m_ProcessData[eDataType].m_pUserCallbackFunc )
	{
		m_ProcessData[eDataType].m_pUserCallbackFunc( bSuccessful, m_ProcessData[eDataType].m_iBufferSizeUsed );
	}

	// We've finished so set the state to idle
	m_ProcessData[eDataType].m_eCurrentMode = SDU_MODE_IDLE;

	// NOTE: This is the last point before the thread ends.
	// So make sure all memory allocated by the class is freed up.

	// Free up the buffer settings buffers...
	if ( bufferSettings.buf )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, (char*)bufferSettings.buf );
	}

	// Free up buffers used in the saving process
	if (m_ProcessData[eDataType]. m_pFileBuffer )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, (char*)m_ProcessData[eDataType].m_pFileBuffer );
		m_ProcessData[eDataType].m_pFileBuffer = 0;
	}

	if ( m_ProcessData[eDataType].m_pUserDataInternalBuffer )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, (char*)m_ProcessData[eDataType].m_pUserDataInternalBuffer );
		m_ProcessData[eDataType].m_pUserDataInternalBuffer = 0;
		m_ProcessData[eDataType].m_iUserDataInternalBufferSize = 0;
	}

	sys_ppu_thread_exit( 0 );

#endif
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::IsSavingOrLoading
*
*	DESCRIPTION		Returns true if some form of data is being loaded or saved
*	
***************************************************************************************************/
bool SaveData::IsSavingOrLoading( void )
{
	for ( int i = 0; i < SAVE_DATA_TYPE_NUM; i++ )
	{
		if ( m_ProcessData[i].m_eCurrentMode != SDU_MODE_IDLE )
		{
			return true;
		}
	}

	// Nothing is being saved
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::Callback... (Checkpoints)
*
*	DESCRIPTION		Dummy callback functions, just so we can call the main callback function but with
*					a parameter.  AAaaaargh!
*	
***************************************************************************************************/
void SaveData::CallbackDataStatusSaveCheckpoints( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set )
{
	CallbackDataStatusSave( cbResult, get, set, m_ProcessData[SAVE_DATA_TYPE_GAMESAVE] );
}

void SaveData::CallbackDataFileSaveCheckpoints( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set )
{
	CallbackDataFileSave( cbResult, get, set, m_ProcessData[SAVE_DATA_TYPE_GAMESAVE] );
}

void SaveData::CallbackDataStatusLoadCheckpoints( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set )
{
	CallbackDataStatusLoad( cbResult, get, set, m_ProcessData[SAVE_DATA_TYPE_GAMESAVE] );
}

void SaveData::CallbackDataFileLoadCheckpoints( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set )
{
	CallbackDataFileLoad( cbResult, get, set, m_ProcessData[SAVE_DATA_TYPE_GAMESAVE] );
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::Callback... (Game Options)
*
*	DESCRIPTION		Dummy callback functions, just so we can call the main callback function but with
*					a parameter.  AAaaaargh!
*	
***************************************************************************************************/
void SaveData::CallbackDataStatusSaveGameOptions( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set )
{
	CallbackDataStatusSave( cbResult, get, set, m_ProcessData[SAVE_DATA_TYPE_GAMEOPTIONS] );
}

void SaveData::CallbackDataFileSaveGameOptions( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set )
{
	CallbackDataFileSave( cbResult, get, set, m_ProcessData[SAVE_DATA_TYPE_GAMEOPTIONS] );
}

void SaveData::CallbackDataStatusLoadGameOptions( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set )
{
	CallbackDataStatusLoad( cbResult, get, set, m_ProcessData[SAVE_DATA_TYPE_GAMEOPTIONS] );
}

void SaveData::CallbackDataFileLoadGameOptions( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set )
{
	CallbackDataFileLoad( cbResult, get, set, m_ProcessData[SAVE_DATA_TYPE_GAMEOPTIONS] );
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::CallbackDataStatusSave
*
*	DESCRIPTION		Data status callback function
*	
***************************************************************************************************/
void SaveData::CallbackDataStatusSave( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set, SDU_Process_Info& obInfo )
{
	ntPrintf("START - CallbackDataStatusSave\n");

	// We want to overwrite the existing data
	set->reCreateMode = CELL_SAVEDATA_RECREATE_NO;
	set->setParam = &get->getParam;
	set->reserved = NULL;

	// Fill in parameters
	strcpy( set->setParam->title, obInfo.m_obDataParams.m_acTitle );
	strcpy( set->setParam->subTitle, obInfo.m_obDataParams.m_acSubtitle );
	strcpy( set->setParam->detail, obInfo.m_obDataParams.m_acDetail );
	set->setParam->attribute = obInfo.m_obDataParams.m_uAttribute;
	set->setParam->parentalLevel = obInfo.m_obDataParams.m_uParentalLevel;
	memset( set->setParam->reserved, 0, 256 * sizeof(char) );

	// Are we saving new data?
	if ( get->isNewData == CELL_SAVEDATA_ISNEWDATA_YES )
	{
		//ntPrintf("INFO - CallbackDataStatusSave - New Data\n");

		int iRequiredSpaceBytes = 0;

		// First calculate the size of the user data (our buffer to write basically)
		iRequiredSpaceBytes += obInfo.m_iUserDataInternalBufferSize;

		// Add on the content files
		iRequiredSpaceBytes += GetTotalContentFileSize();

		// Convert bytes to KB
		int FILE_SIZE_KB = ( ( iRequiredSpaceBytes + 1023 ) / 1024 );

		// Add on system size
		int NEW_SIZE_KB = FILE_SIZE_KB + get->sysSizeKB;

		// Calculate total size needed on HDD
		int NEED_SIZE_KB = get->hddFreeSizeKB - NEW_SIZE_KB;

		// Do we need more space?
		if ( NEED_SIZE_KB < 0 )
		{
			cbResult->errNeedSizeKB = NEED_SIZE_KB;
			cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_NOSPACE;
			ntPrintf("ERROR - Need %dKB more of free space on HDD\n", NEED_SIZE_KB * -1 );
			return;
		}

		// As we are new data, we need to start the saving process at the beginning,
		// so all files are written out.
		obInfo.m_eCurrentSaveStep = SAVE_OPERATION_STEP_ICON0;
	}
	else
	{
		ntPrintf("INFO - Data exists already\n");

		int iRequiredSpaceBytes = 0;

		// First calculate the size of the user data (our buffer to write basically)
		iRequiredSpaceBytes += obInfo.m_iUserDataInternalBufferSize;

		// Add on the content files
		iRequiredSpaceBytes += GetTotalContentFileSize();

		// Convert bytes to KB
		int FILE_SIZE_KB = ( ( iRequiredSpaceBytes + 1023 ) / 1024 );

		// Add on system size
		int NEW_SIZE_KB = FILE_SIZE_KB + get->sysSizeKB;

		// Check if the new save is larger than the current save, if so we may need more space
		if ( NEW_SIZE_KB > get->sizeKB )
		{
			// Calculate the difference between the new save and current save
			int DIFF_SIZE_KB = NEW_SIZE_KB - get->sizeKB;

			// Calculate total size needed on HDD
			int NEED_SIZE_KB = get->hddFreeSizeKB - DIFF_SIZE_KB;

			// Do we need more space?
			if ( NEED_SIZE_KB < 0 )
			{
				cbResult->errNeedSizeKB = NEED_SIZE_KB;
				cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_NOSPACE;
				ntPrintf("ERROR - Need %dKB more of free space on HDD\n", NEED_SIZE_KB * -1 );
				return;
			}
		}

		// As there is existing data, we only need to write out the new user data,
		// not all the icons and sound effect files
		obInfo.m_eCurrentSaveStep = SAVE_OPERATION_STEP_SECURE_USER_DATA;
	}

	// Continue processing normally
	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
	cbResult->reserved = NULL;

	ntPrintf("END - CallbackDataStatusSave\n");
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::CallbackDataFileSave
*
*	DESCRIPTION		File operation callback function
*	
***************************************************************************************************/
void SaveData::CallbackDataFileSave( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set, SDU_Process_Info& obInfo )
{
	ntPrintf("BEGIN - CallbackDataFileSave\n");

	char acContentFilePath[MAX_PATH];

	// Clear any previous file buffer allocation
	if ( obInfo.m_pFileBuffer )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, (char*)obInfo.m_pFileBuffer );
		obInfo.m_pFileBuffer = 0;
	}

	// Assume we want to move onto saving the next file unless specified otherwise in subsequent code.
	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

	// Write the one file
	switch ( obInfo.m_eCurrentSaveStep )
	{
		case SAVE_OPERATION_STEP_ICON0:
		{
			ntPrintf("INFO - Saving ICON0.PNG\n");
			
			// Load data file from disk, so it can be written out for the save			
			Util::GetFiosFilePath_Platform( SAVE_DATA_CONTENT_ICON0_PATH, acContentFilePath );

			if ( !File::Exists( acContentFilePath ) )
			{
				ntPrintf("ERROR - Failed to load '%s'\n", SAVE_DATA_CONTENT_ICON0_PATH);
				cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_FAILURE;
				return;
			}

			FileBuffer obContentFile( acContentFilePath );
			
			// Copy the contents from this file buffer into our static one
			obInfo.m_pFileBuffer = (void*)NT_NEW_ARRAY_CHUNK(Mem::MC_MISC) char[obContentFile.GetSize()];
			memcpy( obInfo.m_pFileBuffer, *obContentFile, obContentFile.GetSize() );

			// Setup the file write
			set->fileOperation	= CELL_SAVEDATA_FILEOP_WRITE;
			set->fileType		= CELL_SAVEDATA_FILETYPE_CONTENT_ICON0;
			set->fileBuf		= obInfo.m_pFileBuffer;
			set->fileBufSize	= obContentFile.GetSize();
			set->fileSize		= obContentFile.GetSize();

			// Move onto the next step, the next time this callback function is called
			obInfo.m_eCurrentSaveStep = SAVE_OPERATION_STEP_ICON1;

		}
		break;

		case SAVE_OPERATION_STEP_ICON1:
		{
			ntPrintf("INFO - Saving ICON1.PAM\n");

			// Load data file from disk, so it can be written out for the save			
			Util::GetFiosFilePath_Platform( SAVE_DATA_CONTENT_ICON1_PATH, acContentFilePath );

			if ( !File::Exists( acContentFilePath ) )
			{
				ntPrintf("ERROR - Failed to load '%s'\n", SAVE_DATA_CONTENT_ICON1_PATH);
				cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_FAILURE;
				return;
			}

			FileBuffer obContentFile( acContentFilePath );
			
			// Copy the contents from this file buffer into our static one
			obInfo.m_pFileBuffer = (void*)NT_NEW_ARRAY_CHUNK(Mem::MC_MISC) char[obContentFile.GetSize()];
			memcpy( obInfo.m_pFileBuffer, *obContentFile, obContentFile.GetSize() );

			// Setup the file write
			set->fileOperation	= CELL_SAVEDATA_FILEOP_WRITE;
			set->fileType		= CELL_SAVEDATA_FILETYPE_CONTENT_ICON1;
			set->fileBuf		= obInfo.m_pFileBuffer;
			set->fileBufSize	= obContentFile.GetSize();
			set->fileSize		= obContentFile.GetSize();

			// Move onto the next step, the next time this callback function is called
			obInfo.m_eCurrentSaveStep = SAVE_OPERATION_STEP_PIC1;
		}
		break;

		case SAVE_OPERATION_STEP_PIC1:
		{
			ntPrintf("INFO - Saving PIC1.PNG\n");

			// Load data file from disk, so it can be written out for the save			
			Util::GetFiosFilePath_Platform( SAVE_DATA_CONTENT_PIC1_PATH, acContentFilePath );

			if ( !File::Exists( acContentFilePath ) )
			{
				ntPrintf("ERROR - Failed to load '%s'\n", SAVE_DATA_CONTENT_PIC1_PATH);
				cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_FAILURE;
				return;
			}

			FileBuffer obContentFile( acContentFilePath );
			
			// Copy the contents from this file buffer into our static one
			obInfo.m_pFileBuffer = (void*)NT_NEW_ARRAY_CHUNK(Mem::MC_MISC) char[obContentFile.GetSize()];
			memcpy( obInfo.m_pFileBuffer, *obContentFile, obContentFile.GetSize() );

			// Setup the file write
			set->fileOperation	= CELL_SAVEDATA_FILEOP_WRITE;
			set->fileType		= CELL_SAVEDATA_FILETYPE_CONTENT_PIC1;
			set->fileBuf		= obInfo.m_pFileBuffer;
			set->fileBufSize	= obContentFile.GetSize();
			set->fileSize		= obContentFile.GetSize();

			// Move onto the next step, the next time this callback function is called
			obInfo.m_eCurrentSaveStep = SAVE_OPERATION_STEP_SND0;
		}
		break;

		case SAVE_OPERATION_STEP_SND0:
		{
			ntPrintf("INFO - Saving SND0.AT3\n");

			// Load data file from disk, so it can be written out for the save			
			Util::GetFiosFilePath_Platform( SAVE_DATA_CONTENT_SND0_PATH, acContentFilePath );

			if ( !File::Exists( acContentFilePath ) )
			{
				ntPrintf("ERROR - Failed to load '%s'\n", SAVE_DATA_CONTENT_SND0_PATH);
				cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_FAILURE;
				return;
			}

			FileBuffer obContentFile( acContentFilePath );
			
			// Copy the contents from this file buffer into our static one
			obInfo.m_pFileBuffer = (void*)NT_NEW_ARRAY_CHUNK(Mem::MC_MISC) char[obContentFile.GetSize()];
			memcpy( obInfo.m_pFileBuffer, *obContentFile, obContentFile.GetSize() );

			// Setup the file write
			set->fileOperation	= CELL_SAVEDATA_FILEOP_WRITE;
			set->fileType		= CELL_SAVEDATA_FILETYPE_CONTENT_SND0;
			set->fileBuf		= obInfo.m_pFileBuffer;
			set->fileBufSize	= obContentFile.GetSize();
			set->fileSize		= obContentFile.GetSize();

			// Move onto the next step, the next time this callback function is called
			obInfo.m_eCurrentSaveStep = SAVE_OPERATION_STEP_SECURE_USER_DATA;
		}
		break;

		case SAVE_OPERATION_STEP_SECURE_USER_DATA:
		{
			ntPrintf("INFO - Saving USER_DATA\n");

			// Setup the file write - always use protected files for now.
			set->fileOperation	= CELL_SAVEDATA_FILEOP_WRITE;
			set->fileType		= CELL_SAVEDATA_FILETYPE_SECUREFILE;
			set->fileBuf		= obInfo.m_pUserDataInternalBuffer;
			set->fileBufSize	= obInfo.m_iUserDataInternalBufferSize;
			set->fileSize		= obInfo.m_iUserDataInternalBufferSize;
			set->fileName		= SAVE_DATA_SECURE_USER_DATA_FILENAME;

			// Copy across the secure file ID
			memcpy( set->secureFileId, secureFileId, CELL_SAVEDATA_SECUREFILEID_SIZE );

			// Move onto the next step, the next time this callback function is called
			obInfo.m_eCurrentSaveStep = SAVE_OPERATION_STEP_END;
		}
		break;

		case SAVE_OPERATION_STEP_END:
		{
			// Saving successful, set the buffer used size
			obInfo.m_iBufferSizeUsed = obInfo.m_iBufferSize < obInfo.m_iUserDataInternalBufferSize ? obInfo.m_iBufferSize : obInfo.m_iUserDataInternalBufferSize;

			// We're all done!
			cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
		}
		break;

		default:
		{
			ntAssert_p(0, ("Invalid save operation step\n") );
		}
		break;
	}

	ntPrintf("END - CallbackDataFileSave\n");
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::CallbackDataStatusLoad
*
*	DESCRIPTION		Data status callback function
*	
***************************************************************************************************/
void SaveData::CallbackDataStatusLoad( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set, SDU_Process_Info& obInfo )
{
	ntPrintf("START - CallbackDataStatusLoad\n");

	// If this is new data, then it doesn't exist!
	if( get->isNewData )
	{
		ntPrintf("INFO - New Data, does not exist '%s'\n", get->dir.dirName);
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
		obInfo.m_iBufferSizeUsed = 0;
		return;
	}
	else
	{
		ntPrintf("INFO - Data exists '%s'\n", get->dir.dirName);

		// Check for broken data
		if ( get->fileListNum < get->fileNum )
		{
			ntPrintf("INFO - Data broken", get->dir.dirName);
			cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
			return;
		}

		// Look for the user data file
		bool bUserDataFound = false;

		for( unsigned int i = 0; i < get->fileListNum; i++ )
		{
			ntPrintf("Looking for userdata: %s\n", get->fileList[i].fileName);

			// If we haven't found the data already and the filename matches...
			if ( !bUserDataFound && strcmp(get->fileList[i].fileName, SAVE_DATA_SECURE_USER_DATA_FILENAME) == 0)
			{
				ntPrintf("INFO - Found user data '%s'\n", SAVE_DATA_SECURE_USER_DATA_FILENAME);
				bUserDataFound = true;

				// Check that the file can fit into our buffer
				if ( get->fileList[i].st_size > (unsigned int)obInfo.m_iUserDataInternalBufferSize )
				{
					ntPrintf("INFO - File too big for buffer '%s'\n", SAVE_DATA_SECURE_USER_DATA_FILENAME);
					cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
					return;
				}

				// Don't through any more files
				break;
			}
		}

		if ( !bUserDataFound )
		{
			ntPrintf("INFO - Data broken, user data file not found", get->dir.dirName);
			cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_NODATA;
			return;
		}
	}

	set->reCreateMode = CELL_SAVEDATA_RECREATE_NO;
	set->setParam = 0;
	set->reserved = 0;

	// Move onto the next stage
	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
	cbResult->reserved = 0;

	// Set the load step to the first one
	obInfo.m_eCurrentLoadStep = LOAD_OPERATION_STEP_SECURE_USER_DATA;

	ntPrintf("END - CallbackDataStatusLoad\n");
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::CallbackDataFileLoad
*
*	DESCRIPTION		File operation callback function
*	
***************************************************************************************************/
void SaveData::CallbackDataFileLoad( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set, SDU_Process_Info& obInfo )
{
	ntPrintf("START - CallbackDataFileLoad\n");

	// Assume we want to move onto loading the next file unless specified otherwise in subsequent code.
	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

	// Load the one file
	switch ( obInfo.m_eCurrentLoadStep )
	{
		case LOAD_OPERATION_STEP_SECURE_USER_DATA:
		{
			set->fileOperation = CELL_SAVEDATA_FILEOP_READ;
			set->fileBuf = obInfo.m_pUserDataInternalBuffer;
			set->fileBufSize = obInfo.m_iUserDataInternalBufferSize;
			set->fileName = SAVE_DATA_SECURE_USER_DATA_FILENAME;
			set->fileSize = obInfo.m_iUserDataInternalBufferSize; // File size should be the same as buffer size?
			set->fileType = CELL_SAVEDATA_FILETYPE_SECUREFILE;
			memcpy( set->secureFileId, secureFileId, CELL_SAVEDATA_SECUREFILEID_SIZE );

			// Move onto the next step, the next time this callback function is called
			obInfo.m_eCurrentLoadStep = LOAD_OPERATION_STEP_END; 
		}
		break;

		case LOAD_OPERATION_STEP_END:
		{
			// User Data loading successful, copy loaded data into output buffer
			obInfo.m_iBufferSizeUsed = obInfo.m_iBufferSize < obInfo.m_iUserDataInternalBufferSize ? obInfo.m_iBufferSize : obInfo.m_iUserDataInternalBufferSize;
			memcpy( obInfo.m_pBuffer, obInfo.m_pUserDataInternalBuffer, obInfo.m_iBufferSizeUsed );

			// We're all done!
			cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
		}
		break;

		default:
		{
			ntAssert_p(0, ("Invalid load operation step\n") );
		}
		break;
	}

	ntPrintf("END - CallbackDataFileLoad\n");
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::GetTotalContentFileSize
*
*	DESCRIPTION		Helper function to calculate the total size of the four content files.
*					i.e. pics, icons and sound
*	
***************************************************************************************************/
int SaveData::GetTotalContentFileSize( void )
{
	char acContentFilePath[MAX_PATH];
	int iTotalSize = 0;

	// Icon 0
	Util::GetFiosFilePath_Platform( SAVE_DATA_CONTENT_ICON0_PATH, acContentFilePath );
	FileBuffer obIcon0File( acContentFilePath );
	iTotalSize += obIcon0File.GetSize();

	// Icon 1
	Util::GetFiosFilePath_Platform( SAVE_DATA_CONTENT_ICON1_PATH, acContentFilePath );
	FileBuffer obIcon1File( acContentFilePath );
	iTotalSize += obIcon1File.GetSize();

	// Pic 1
	Util::GetFiosFilePath_Platform( SAVE_DATA_CONTENT_PIC1_PATH, acContentFilePath );
	FileBuffer obPic1File( acContentFilePath );
	iTotalSize += obPic1File.GetSize();

	// Snd 0
	Util::GetFiosFilePath_Platform( SAVE_DATA_CONTENT_SND0_PATH, acContentFilePath );
	FileBuffer obSnd0File( acContentFilePath );
	iTotalSize += obSnd0File.GetSize();

	return iTotalSize;
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::ShowSavingIcon
*
*	DESCRIPTION		Shows the saving icon if it isn't being shown already.
*	
***************************************************************************************************/
void SaveData::ShowSavingIcon( void )
{
	m_iSavingIconRefCount++;

	// Make sure we have the RenderDef for the saving icon
	if ( !m_pSavingIconRenderDef )
	{
		m_pSavingIconRenderDef = ObjectDatabase::Get().GetPointerFromName<CHudUnitDef*>("SavingIconRD");
	}

	ntAssert(m_pSavingIconRenderDef);

	// If the hud unit doesn't exist, then create one
	if ( !m_pSavingIconHudUnit )
	{
		m_pSavingIconHudUnit = CHud::Get().CreateHudElement( m_pSavingIconRenderDef );
		m_pSavingIconHudUnit->BeginEnter();
	}
}


/***************************************************************************************************
*
*	FUNCTION		SaveData::HideSavingIcon
*
*	DESCRIPTION		Attempts to hide the saving icon, but if another save is still saving it remains
*					shown.
*	
***************************************************************************************************/
void SaveData::HideSavingIcon( void )
{
	m_iSavingIconRefCount--;

	if ( m_iSavingIconRefCount <= 0 )
	{
		m_iSavingIconRefCount = 0;

		ntAssert( m_pSavingIconHudUnit );

		m_pSavingIconHudUnit->BeginExit();
		CHud::Get().RemoveHudElement( m_pSavingIconHudUnit );

		m_pSavingIconHudUnit = 0;
	}
}

