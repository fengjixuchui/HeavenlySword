//------------------------------------------------------------------------------------------
//!
//!	\file savedata_ps3.h
//!
//------------------------------------------------------------------------------------------


#ifndef _SAVE_DATA_PS3_H
#define _SAVE_DATA_PS3_H

#define USE_SAVE_DATA

// Includes
#include <sysutil/sysutil_savedata.h>	// PS3 Save Data Utility (SDU)

// Forward Declarations
class CHudUnitDef;
class CHudUnit;

// Callback functions that are passed into the processes and then called when the process has finished
typedef void (*SaveDataCallback)( bool bSuccessful, int iBufferSizeUsed );

// Save type
typedef enum SAVE_DATA_TYPE
{
	SAVE_DATA_TYPE_GAMESAVE = 0,
	SAVE_DATA_TYPE_GAMEOPTIONS,

	SAVE_DATA_TYPE_NUM
};


// Enumerators for saving status etc.
typedef enum SDU_MODE
{
	SDU_MODE_IDLE,
	SDU_MODE_SAVING,
	SDU_MODE_LOADING
};

// Enumerator for the saving steps
typedef enum SAVE_OPERATION_STEPS
{
	SAVE_OPERATION_STEP_ICON0,
	SAVE_OPERATION_STEP_ICON1,
	SAVE_OPERATION_STEP_PIC1,
	SAVE_OPERATION_STEP_SND0,
	SAVE_OPERATION_STEP_SECURE_USER_DATA,
	SAVE_OPERATION_STEP_END
};

// Enumerator for the loading steps
typedef enum LOAD_OPERATION_STEPS
{
	LOAD_OPERATION_STEP_SECURE_USER_DATA,
	LOAD_OPERATION_STEP_END
};


/***************************************************************************************************
*
*	CLASS			SDU_Process_Info
*
*	DESCRIPTION		All the buffers, data and info related to a particular save/load type i.e. checkpoints
*
***************************************************************************************************/
class SDU_Process_Info
{
public:

	// Data params related to the save/load
	typedef struct
	{
		char					m_acTitle[CELL_SAVEDATA_SYSP_TITLE_SIZE];
		char					m_acSubtitle[CELL_SAVEDATA_SYSP_SUBTITLE_SIZE];
		char					m_acDetail[CELL_SAVEDATA_SYSP_DETAIL_SIZE];
		unsigned int			m_uParentalLevel;
		unsigned int			m_uAttribute;

		void SetParams( const char* pcTitle, const char* pcSubtitle, const char* pcDetail, unsigned int uParentalLevel, unsigned int uAttribute );
	} SDU_Data_Params;
	
	// Constructor & Destructor
	SDU_Process_Info();
	~SDU_Process_Info();

	// Function for setting the basic info and buffers which are the same for both load and save
	void CommonSetup( void* pBuffer, int iBufferSize, SaveDataCallback pCallbackFunc, SAVE_DATA_TYPE eSaveDataType );

	// The mode we're currently in
	SDU_MODE				m_eCurrentMode;

	// The current step for loading and saving processes
	SAVE_OPERATION_STEPS	m_eCurrentSaveStep;
	LOAD_OPERATION_STEPS	m_eCurrentLoadStep;

	// Main Cell SDU callbacks to use
	CellSaveDataStatCallback	m_pDataStatusCallback;
	CellSaveDataFileCallback	m_pFileCallback;

	// Buffers we're using to write from/load to
	void*					m_pBuffer;
	int						m_iBufferSize;

	// How much of the buffer did we end up writing into/loading from?
	int						m_iBufferSizeUsed;

	// Buffer for reading in save data content files
	void*					m_pFileBuffer;

	// Internal buffer for the file reading/writing, must be the minimum SDU allowed size or larger
	void*					m_pUserDataInternalBuffer;
	int						m_iUserDataInternalBufferSize;

	// Parameters for the save/load	(used by SDU created PARAM.SFO files) 
	SDU_Data_Params			m_obDataParams;

	// The directory we are saving/loading from
	char					m_acDirName[MAX_PATH];

	// Callback function to be called when the process has finished
	SaveDataCallback		m_pUserCallbackFunc;
};


/***************************************************************************************************
*
*	CLASS			SaveData
*
*	DESCRIPTION		Static based class to take care of our game's save data functionality
*
***************************************************************************************************/
class SaveData
{
public:

	static bool	LoadInData( void* pBuffer, int iBufferSize, SaveDataCallback pCallbackFunc, SAVE_DATA_TYPE eSaveDataType );
	static bool SaveOutData( void* pBuffer, int iBufferSize, SaveDataCallback pCallbackFunc, SAVE_DATA_TYPE eSaveDataType );

	static void ShowSavingIcon( void );
	static void HideSavingIcon( void );

private:

	// Save and Load functions to be threaded.
	static void ThreadAutoSave( uint64_t arg );
	static void ThreadAutoLoad( uint64_t arg );

	static bool	IsSavingOrLoading( void );

	// Save specific callbacks, basically call the generic ones below, but pass in additional data
	static void CallbackDataStatusSaveCheckpoints( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set );
	static void CallbackDataFileSaveCheckpoints( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set );
	static void CallbackDataStatusLoadCheckpoints( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set );
	static void CallbackDataFileLoadCheckpoints( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set );

	static void CallbackDataStatusSaveGameOptions( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set );
	static void CallbackDataFileSaveGameOptions( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set );
	static void CallbackDataStatusLoadGameOptions( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set );
	static void CallbackDataFileLoadGameOptions( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set );

	// Generic Callbacks
	static void CallbackDataStatusSave( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set, SDU_Process_Info& obInfo );
	static void CallbackDataFileSave( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set, SDU_Process_Info& obInfo );

	static void CallbackDataStatusLoad( CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set, SDU_Process_Info& obInfo );
	static void CallbackDataFileLoad( CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set, SDU_Process_Info& obInfo );
	
	// Helper function to add up the file size of all the content files
	static int GetTotalContentFileSize( void );

	// The process data for the particular save/load types
	static SDU_Process_Info	m_ProcessData[SAVE_DATA_TYPE_NUM];

	// Cached pointer to the saving icon render def
	static CHudUnitDef*	m_pSavingIconRenderDef;

	// Hud Unit pointer (during save)
	static CHudUnit*	m_pSavingIconHudUnit;

	// Saving Icon ref count
	static int			m_iSavingIconRefCount;
};


#endif // _SAVE_DATA_PS3_H
