//------------------------------------------------------------------------------------------
//!
//!	\file savedata_pc.h
//!
//------------------------------------------------------------------------------------------


#ifndef _SAVE_DATA_PC_H
#define _SAVE_DATA_PC_H


// Callback functions that are passed into the processes and then called when the process has finished
typedef void (*SaveDataCallback)( bool bSuccessful, int iBufferSizeUsed );


// Save type
typedef enum SAVE_DATA_TYPE
{
	SAVE_DATA_TYPE_GAMESAVE = 0,
	SAVE_DATA_TYPE_GAMEOPTIONS,

	SAVE_DATA_TYPE_NUM
};


// All SDU functionality wrapped up in it's own namespace
class SaveData
{
public:

	// Saving Icon
	static void ShowSavingIcon( void ) {};
	static void HideSavingIcon( void ) {};

	// Sync on PC, so always false
	static bool	IsSavingOrLoading( void ) { return false; };

	// Load/save the game options
	static bool LoadInData( void* pBuffer, size_t iBufferSize, SaveDataCallback pCallbackFunc, SAVE_DATA_TYPE eSaveDataType);
	static bool SaveOutData( void* pBuffer, size_t iBufferSize, SaveDataCallback pCallbackFunc, SAVE_DATA_TYPE eSaveDataType );

private:

};


#endif // _SAVE_DATA_PC_H

