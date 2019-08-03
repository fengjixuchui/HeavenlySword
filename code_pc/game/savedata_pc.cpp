//------------------------------------------------------------------------------------------
//!
//!	\file savedata_pc.cpp
//!
//------------------------------------------------------------------------------------------

// Includes
#include "game/savedata_pc.h"


// Filename of the userdata
#define SAVE_DATA_SECURE_USER_DATA_FILENAME			"userdata.dat"


//-------------------------------------------------------
//
// TO BE INPLEMENTED!!!
//
// ... only if there is time to do it.  Not important
//
//-------------------------------------------------------


bool SaveData::LoadInData( void* pBuffer, size_t iBufferSize, SaveDataCallback pCallbackFunc, SAVE_DATA_TYPE eSaveDataType )
{
	UNUSED( pBuffer );
	UNUSED( iBufferSize );
	UNUSED( pCallbackFunc );
	UNUSED( eSaveDataType );

	ntAssert( pBuffer );
	ntAssert( iBufferSize > 0 );
	ntAssert( pCallbackFunc );

	// Call the callback straight away, so it can do cleanup on PC
	pCallbackFunc(true, 0);

	return true;
}


bool SaveData::SaveOutData( void* pBuffer, size_t iBufferSize, SaveDataCallback pCallbackFunc, SAVE_DATA_TYPE eSaveDataType )
{
	UNUSED( pBuffer );
	UNUSED( iBufferSize );
	UNUSED( pCallbackFunc );
	UNUSED( eSaveDataType );

	ntAssert( pBuffer );
	ntAssert( iBufferSize > 0 );
	ntAssert( pCallbackFunc );

	// Call the callback straight away, so it can do cleanup on PC
	pCallbackFunc(true, 0);

	return true;
}

