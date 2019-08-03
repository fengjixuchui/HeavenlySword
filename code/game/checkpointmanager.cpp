//--------------------------------------------------
//!
//!	\file game/checkpointmanager.cpp
//!	Checkpoint manager code
//!
//--------------------------------------------------


#include "game/checkpointmanager.h"
#include "game/entitycheckpoint.h"
//#include "game/gamedatafileutils.h"

#include "hud/hudmanager.h"
#include "gui/guimanager.h"
#include "game/savedata.h"
#include "game/combatstyle.h"
#include "game/attacks.h"
#include "editable/enumlist.h"

#include "game/shellmain.h"
#include "game/shelllevel.h"


// An estimate for the total number of checkpoints in the game.
// Required to create sufficient save/load buffer sizes.
// Currently 49 Checkpoints, including the end level code-created ones.
// ... and round it up, so we have a spare
#define MAX_NUM_CHECKPOINTS_IN_GAME 50

// Save Data version - in case more than just the checkpoints are being saved out
static const int	s_iCurrentSaveDataVersion = 4;		// 5th version, 6-12-06 - DHotop.  Unique save data extended to include HIT_LEVEL.

// Initialise static member variables
void* CheckpointManager::m_pSaveDataBuffer = 0;
size_t CheckpointManager::m_iSaveDataBufferSize = 0;
bool CheckpointManager::m_bSaveDataBusy = false;


//--------------------------------------------------
//!
//!	Interfaces for combo unlock data
//!
//--------------------------------------------------
START_CHUNKED_INTERFACE(ComboUnlockData, Mem::MC_MISC)
	PUBLISH_DEEP_LIST_AS( m_ComboUnlocks, Combos )

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE

START_CHUNKED_INTERFACE(ComboUnlockItemData, Mem::MC_MISC)
	PUBLISH_VAR_AS( m_OriginalLink, OriginalLink )
	PUBLISH_VAR_AS( m_UnlockLink, UnlockLink )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_UnlockStylePoints, 0, StylePoints )
	PUBLISH_GLOBAL_ENUM_WITH_DEFAULT_AS( m_eStanceStyle, StanceStyle, STANCE_STYLE, STANCE_STYLE_SPEED )
END_STD_INTERFACE


//--------------------------------------------------
//!
//!	Current File Format for Checkpoint Data
//!
//--------------------------------------------------

// Save Data Version - The main save data version.
// Checkpoints - Global Data Version
// Checkpoints - Generic Data Version
// Checkpoints - Level Data Version
// Checkpoints - Number of checkpoints
// Checkpoints - The Checkpoint Data


//--------------------------------------------------
//!
//!	ComboUnlockData::ComboUnlockData()
//!	Consructor
//!
//--------------------------------------------------
ComboUnlockData::ComboUnlockData()
{

}


//--------------------------------------------------
//!
//!	ComboUnlockData::~ComboUnlockData()
//!	Destructor
//!
//--------------------------------------------------
ComboUnlockData::~ComboUnlockData()
{
	// Remove pointer
	if ( CheckpointManager::Exists() )
	{
		CheckpointManager::Get().SetComboUnlockDataPtr( 0 );
	}
}


//--------------------------------------------------
//!
//!	ComboUnlockData::OnPostConstruct()
//!	OnPostConstruct
//!
//--------------------------------------------------
void ComboUnlockData::OnPostConstruct()
{
	// Add a pointer to me to the checkpoint manager
	CheckpointManager::Get().SetComboUnlockDataPtr( this );
}

//--------------------------------------------------
//!
//!	ComboUnlockItemData::CanComboBeUnlocked()
//!	Checks to see if the combo can be unlocked
//!
//--------------------------------------------------
bool ComboUnlockItemData::CanComboBeUnlocked( const StanceStylePoints& obStylePoints )
{
	switch ( m_eStanceStyle )
	{
		case STANCE_STYLE_SPEED:
		{
			if ( obStylePoints.m_iStylePointsSpeed >= m_UnlockStylePoints )
			{
				return true;
			}
		}
		break;

		case STANCE_STYLE_POWER:
		{
			if ( obStylePoints.m_iStylePointsPower >= m_UnlockStylePoints )
			{
				return true;
			}
		}
		break;

		case STANCE_STYLE_RANGE:
		{
			if ( obStylePoints.m_iStylePointsRange >= m_UnlockStylePoints )
			{
				return true;
			}
		}
		break;

		case STANCE_STYLE_AERIAL:
		{
			if ( obStylePoints.m_iStylePointsAerial >= m_UnlockStylePoints )
			{
				return true;
			}
		}
		break;

		default:
		{
			ntAssert_p( 0, ("Unknown Combo Style enum\n") );
		}
		break;
	}

	return false;
}


//--------------------------------------------------
//!
//!	CheckpointData::CheckpointData()
//!	Constructor
//!
//--------------------------------------------------
CheckpointData::CheckpointData( int iLevel, int iCheckpoint )
{
	m_iLevel		= iLevel;
	m_iCheckpoint	= iCheckpoint;
}


//--------------------------------------------------
//!
//!	CheckpointData::~CheckpointData()
//!	Destructor
//!
//--------------------------------------------------
CheckpointData::~CheckpointData()
{
}


//--------------------------------------------------
//!
//!	CheckpointManager::CheckpointManager()
//!	Constructor
//!
//--------------------------------------------------
CheckpointManager::CheckpointManager( )
{
	m_pLUACPData = 0;
	m_pComboUnlockData = 0;

	// Create the save data buffer
	if ( !m_pSaveDataBuffer )
	{
		// Calculate the size of the save data buffer to create
		m_iSaveDataBufferSize = sizeof( int );	// Save Data version number
		m_iSaveDataBufferSize += sizeof( int ) * 3;	// The Checkpoints version numbers
		m_iSaveDataBufferSize += sizeof( CheckpointData ) * MAX_NUM_CHECKPOINTS_IN_GAME;	// Max num of checkpoints in game

		// WTF was this doing here?
		//m_iSaveDataBufferSize = 32 * 1024;

		m_pSaveDataBuffer = (void*)NT_NEW_ARRAY_CHUNK(Mem::MC_MISC) char[m_iSaveDataBufferSize];
	}
}


//--------------------------------------------------
//!
//!	CheckpointManager::~CheckpointManager()
//!	Destructor
//!
//--------------------------------------------------
CheckpointManager::~CheckpointManager( )
{
	// Clear out all the checkpoint data
	ClearCheckpointData();

	// Free the save data buffer
	if ( m_pSaveDataBuffer )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, (char*)m_pSaveDataBuffer );
		m_pSaveDataBuffer = 0;
	}
}


//--------------------------------------------------
//!
//!	CheckpointManager::GetLifeClockToPreviousCheckpoint()
//!	Calculates the lifeclock upto (not including) the
//! passed in current checkpoint
//!
//--------------------------------------------------
double CheckpointManager::GetLifeClockToPreviousCheckpoint( CheckpointData* pCurrentCheckpointData )
{
	ntAssert( pCurrentCheckpointData );

	double dLifeClock;

	if ( StyleManager::Exists() && StyleManager::Get().GetLifeClock() )
	{
		dLifeClock = StyleManager::Get().GetLifeClock()->GetInitialTime();
	}
	else
	{
		dLifeClock = 172800.0;
	}

	ntstd::Vector<CheckpointData*>::iterator iter;
	ntstd::Vector<CheckpointData*>::iterator iterEnd = m_CPDataArray.end();

	for ( iter = m_CPDataArray.begin(); iter != iterEnd; iter++ )
	{
		CheckpointData* pDataBlock = (*iter);
		
		// Check if we have reached the current checkpoint, if so return
		if ( pDataBlock == pCurrentCheckpointData )
		{
			return dLifeClock;
		}

		// Otherwise add it on
		dLifeClock += pDataBlock->m_GlobalData.GetLifeClockDelta();
	}

	// How ze hell did ve reach here?
	// Our current checkpoint should have been found by now
	ntAssert_p( 0, ("Current Checkpoint Data seems to be missing from the Array") );

	return dLifeClock;
}


//--------------------------------------------------
//!
//!	CheckpointManager::GetLifeClockToThisCheckpoint()
//!	Calculates the lifeclock upto the current checkpoint
//!
//--------------------------------------------------
double CheckpointManager::GetLifeClockToThisCheckpoint( CheckpointData* pCurrentCheckpointData )
{
	ntAssert( pCurrentCheckpointData );

	double dLifeClock;

	if ( StyleManager::Exists() && StyleManager::Get().GetLifeClock() )
	{
		dLifeClock = StyleManager::Get().GetLifeClock()->GetInitialTime();
	}
	else
	{
		dLifeClock = 172800.0;
	}


	ntstd::Vector<CheckpointData*>::iterator iter;
	ntstd::Vector<CheckpointData*>::iterator iterEnd = m_CPDataArray.end();

	for ( iter = m_CPDataArray.begin(); iter != iterEnd; iter++ )
	{
		CheckpointData* pDataBlock = (*iter);

		// Add on the delta time
		dLifeClock += pDataBlock->m_GlobalData.GetLifeClockDelta();

		// Check if we have reached the current checkpoint, if so return
		if ( pDataBlock == pCurrentCheckpointData )
		{
			return dLifeClock;
		}
	}

	// How ze hell did ve reach here?
	// Our current checkpoint should have been found by now
	ntAssert_p( 0, ("Current Checkpoint Data seems to be missing from the Array") );

	return dLifeClock;
}


//--------------------------------------------------
//!
//!	CheckpointManager::GetLastCheckpointNumberReachedByPlayer()
//!	Gets the last checkpoint number reached by the player.
//! Returns false if the player has not reached a checkpoint yet
//!
//--------------------------------------------------
bool CheckpointManager::GetLastCheckpointNumberReachedByPlayer( int& iLevel, int& iCheckpoint )
{
	if ( m_CPDataArray.size() <= 0 )
	{
		return false;
	}

	// Get the last checkpoint data
	CheckpointData* pData = m_CPDataArray.back();
	ntAssert( pData );

	// Set the ID's
	iLevel = pData->m_iLevel;
	iCheckpoint = pData->m_iCheckpoint;

	return true;
}


//--------------------------------------------------
//!
//!	CheckpointManager::GetDataForCheckpoint()
//!	Gets the data for a specific checkpoint.
//!
//--------------------------------------------------
CheckpointData*	CheckpointManager::GetDataForCheckpoint( int iLevel, int iCheckpoint )
{
	// [Martin]
	// ntAssert( iLevel >= 0 && iCheckpoint >= 0 );

	ntstd::Vector<CheckpointData*>::iterator iter;
	ntstd::Vector<CheckpointData*>::iterator iterEnd = m_CPDataArray.end();

	for ( iter = m_CPDataArray.begin(); iter != iterEnd; iter++ )
	{
		CheckpointData* pDataBlock = (*iter);

		if ( pDataBlock->m_iLevel == iLevel && pDataBlock->m_iCheckpoint == iCheckpoint )
		{
			return pDataBlock;
		}
	}

	// Couldn't find it, return 0
	return 0;
}


//--------------------------------------------------
//!
//!	CheckpointManager::LoadCheckpointData()
//!	Loads in the checkpoint data from disk
//!
//--------------------------------------------------
CheckpointData* CheckpointManager::CreateDataForCheckpoint( int iLevel, int iCheckpoint )
{
	// First check if it already exists
	CheckpointData* pExistingData = GetDataForCheckpoint( iLevel, iCheckpoint );
	if ( pExistingData )
	{
		return pExistingData;
	}

	// Couldn't find existing data for checkpoint, so create one
	CheckpointData* pNewDataBlock = NT_NEW CheckpointData( iLevel, iCheckpoint );

	// Add the new data into the correct place in the array
	InsertDataIntoArray( pNewDataBlock );

	// Return a pointer to the new data block
	return pNewDataBlock;
}


//--------------------------------------------------
//!
//!	CheckpointManager::GetLastCheckpoint()
//!	Gets the last checkpoint
//!
//--------------------------------------------------
CheckpointData* CheckpointManager::GetLastCheckpoint( void ) const
{
	unsigned int uArraySize = m_CPDataArray.size();

	if ( uArraySize > 0 )
	{
		return m_CPDataArray[uArraySize - 1];
	}

	return 0;
}


//--------------------------------------------------
//!
//!	CheckpointManager::UnlockCombos()
//!	Unlock the combos based on style points
//!
//--------------------------------------------------
void CheckpointManager::UnlockCombos( const StanceStylePoints& obStylePoints, bool bNotifyHUD )
{
	// Can only do this if we have unlock data
	if ( m_pComboUnlockData )
	{
		DeepListIterator comboIter = m_pComboUnlockData->m_ComboUnlocks.DeepIterator();

		// For each unlock
		while ( comboIter )
		{
			ComboUnlockItemData* pComboItem = (ComboUnlockItemData*)comboIter.GetValue();
			ntAssert( pComboItem );

			// Assume we can't unlock the combo until we've checked the unlock score below
			bool bUnlockCombo = false;

			switch ( pComboItem->m_eStanceStyle )
			{
				case STANCE_STYLE_SPEED:
				{
					if ( obStylePoints.m_iStylePointsSpeed >= pComboItem->m_UnlockStylePoints )
					{
						bUnlockCombo = true;
					}
				}
				break;

				case STANCE_STYLE_POWER:
				{
					if ( obStylePoints.m_iStylePointsPower >= pComboItem->m_UnlockStylePoints )
					{
						bUnlockCombo = true;
					}
				}
				break;

				case STANCE_STYLE_RANGE:
				{
					if ( obStylePoints.m_iStylePointsRange >= pComboItem->m_UnlockStylePoints )
					{
						bUnlockCombo = true;
					}
				}
				break;

				case STANCE_STYLE_AERIAL:
				{
					if ( obStylePoints.m_iStylePointsAerial >= pComboItem->m_UnlockStylePoints )
					{
						bUnlockCombo = true;
					}
				}
				break;

				default:
				{
					ntAssert_p( 0, ("Unknown Combo Style enum\n") );
				}
				break;
			}

			// Do we unlock the combo or not?
			if ( bUnlockCombo )
			{
				CAttackLink* pOriginalLink = (CAttackLink*)ObjectDatabase::Get().GetPointerFromName<CAttackLink*>( pComboItem->m_OriginalLink );
				CAttackLink* pUnlockLink = (CAttackLink*)ObjectDatabase::Get().GetPointerFromName<CAttackLink*>( pComboItem->m_UnlockLink );
				ntAssert( pOriginalLink && pUnlockLink );

				// Are we notifying the HUD of these combo unlocks?
				if ( bNotifyHUD )
				{
					// It's a new unlock if the current force swap pointer is 0
					if ( !pOriginalLink->GetForceSwapWithLink() )
					{
						// Notify HUD
						switch ( pComboItem->m_eStanceStyle )
						{
							case STANCE_STYLE_SPEED: { CHud::Get().CreateMessage( "UNLOCK_SPEEDCOMBO", 0.5f, 0.8f ); } break;
							case STANCE_STYLE_POWER: { CHud::Get().CreateMessage( "UNLOCK_POWERCOMBO", 0.5f, 0.8f ); } break;
							case STANCE_STYLE_RANGE: { CHud::Get().CreateMessage( "UNLOCK_RANGECOMBO", 0.5f, 0.8f ); } break;
							case STANCE_STYLE_AERIAL: { CHud::Get().CreateMessage( "UNLOCK_AERIALCOMBO", 0.5f, 0.8f ); } break;
							default: { ntAssert_p( 0, ("Unknown Combo Style enum\n") );	} break;
						}

						CHud::Get().RemoveMessage( 2.0f );
					}
				}

				// Unlock that combo!
				pOriginalLink->SetForceSwapWithLink( pUnlockLink );
			}

			// Onto the next combo
			++comboIter;
		}
	}
}


//--------------------------------------------------
//!
//!	CheckpointManager::CacheStylePointTotals()
//!	Goes through all the checkpoints, adding up the
//! style points and then
//!
//--------------------------------------------------
void CheckpointManager::CacheStylePointTotals( void )
{
	ntstd::Vector<CheckpointData*>::iterator iter;
	ntstd::Vector<CheckpointData*>::iterator iterEnd = m_CPDataArray.end();

	StanceStylePoints obTotalPoints;

	// For each checkpoint
	for ( iter = m_CPDataArray.begin(); iter != iterEnd; iter++ )
	{
		CheckpointData* pDataBlock = (*iter);

		// Add on the style points
		obTotalPoints += pDataBlock->m_GenericData.GetStanceStylePoints();
	}

	// Store this value in the style manager
	StyleManager::Get().GetStats().m_obCachedStylePointTotals = obTotalPoints;
}


//--------------------------------------------------
//!
//!	CheckpointManager::LoadCheckpointData()
//!	Loads in the checkpoint data from disk
//!
//--------------------------------------------------
void CheckpointManager::LoadCheckpointData( void )
{
	// TODO: Add some checks to make sure loading only occurs when a user starts the game?

	// Check that we aren't loading or saving at the moment.
	if ( m_bSaveDataBusy )
	{
		ntPrintf("ERROR - Could not load checkpoint data, busy doing something\n");
		return;
	}

	// Clear out the current checkpoint data before loading in new data
	ClearCheckpointData();

	// Clear the memory buffer
	memset( m_pSaveDataBuffer, 0, m_iSaveDataBufferSize );

	// Set the busy flag BEFORE the callback, as on PC that happens immediately
	m_bSaveDataBusy = true;

	// Start the loading process
	if ( !SaveData::LoadInData( m_pSaveDataBuffer, m_iSaveDataBufferSize, LoadCompleteCallback, SAVE_DATA_TYPE_GAMESAVE ) )
	{
		ntPrintf("ERROR - Could not load checkpoint data, SaveData class possibly busy with checkpoint data already\n");
		m_bSaveDataBusy = false;
		return;
	}

}

//--------------------------------------------------
//!
//!	CheckpointManager::SaveCheckpointData()
//!	Saves the checkpoint data to disk
//!
//--------------------------------------------------
void CheckpointManager::SaveCheckpointData( void )
{
	// Check that we aren't loading or saving at the moment.
	if ( m_bSaveDataBusy )
	{
		ntPrintf("ERROR - Could not save checkpoint data, busy doing something\n");
		return;
	}

	// Clear the memory buffer
	memset( m_pSaveDataBuffer, 0, m_iSaveDataBufferSize );

	// Write to the start of the buffer
	char* pWritePtr = (char*)m_pSaveDataBuffer;

	// Save Data version number
	memcpy( (void*)pWritePtr, &s_iCurrentSaveDataVersion, sizeof(int) );
	pWritePtr += sizeof(int);

	// Write the unique data
	memcpy( (void*)pWritePtr, &m_obUniqueSaveData, sizeof(GameSaveUniqueData) );
	pWritePtr += sizeof(GameSaveUniqueData);

	// Number of checkpoints
	int iNumCheckpoints = CheckpointManager::Get().m_CPDataArray.size();
	memcpy( (void*)pWritePtr, &iNumCheckpoints, sizeof(int) );
	pWritePtr += sizeof(int);

	// The checkpoints
	ntstd::Vector<CheckpointData*>::iterator pIter;
	ntstd::Vector<CheckpointData*>::iterator pEndIter = CheckpointManager::Get().m_CPDataArray.end();
	for ( pIter = CheckpointManager::Get().m_CPDataArray.begin(); pIter != pEndIter; pIter++ )
	{
		CheckpointData* pData = (*pIter);
		ntAssert( pData );

		memcpy( (void*)pWritePtr, (void*)pData, sizeof(CheckpointData) ); 
	}

	// Set the busy flag BEFORE the callback, as on PC that happens immediately
	m_bSaveDataBusy = true;

	// Start the saving process
	if ( !SaveData::SaveOutData( m_pSaveDataBuffer, m_iSaveDataBufferSize, SaveCompleteCallback, SAVE_DATA_TYPE_GAMESAVE ) )
	{
		ntPrintf("ERROR - Could not load checkpoint data, SaveData class possibly busy with checkpoint data already\n");
		m_bSaveDataBusy = false;
		return;
	}
	
	// Show the saving icon, we do this here so that it is thread safe
	//SaveData::ShowSavingIcon();
}


//--------------------------------------------------
//!
//!	CheckpointManager::LoadCompleteCallback()
//!	Static callback function, called when the loading is complete
//!
//--------------------------------------------------
void CheckpointManager::LoadCompleteCallback( bool bSuccessful, int iBufferSizeUsed )
{
	// The process has finished
	m_bSaveDataBusy = false;

	// Did loading fail?
	if ( !bSuccessful )
	{
		ntPrintf("CheckpointManager: Load failed\n");
		return;
	}

	// Was there any data to load?
	if ( iBufferSizeUsed == 0 )
	{
		ntPrintf("CheckpointManager: No data to load\n");
		return;
	}

	// Read from the start of the buffer
	char* pReadPtr = (char*)m_pSaveDataBuffer;

	// Check save data version
	int iLoadedSaveDataVersion = 0;
	memcpy( &iLoadedSaveDataVersion, pReadPtr, sizeof(int) );
	pReadPtr += sizeof(int);

	// Compare version
	if ( iLoadedSaveDataVersion != s_iCurrentSaveDataVersion )
	{
		// Version mismatch
		ntPrintf("ERROR - CheckpointManager: Save Data version mismatch\n");
		return;
	}

	// Read in the unique data
	memcpy( &CheckpointManager::Get().m_obUniqueSaveData, pReadPtr, sizeof(GameSaveUniqueData) );
	pReadPtr += sizeof(GameSaveUniqueData);

	// Read in number of checkpoints
	int iNumCheckpointsLoaded = 0;
	memcpy( &iNumCheckpointsLoaded, pReadPtr, sizeof(int) );
	pReadPtr += sizeof(int);

	for ( int i = 0; i < iNumCheckpointsLoaded; i++ )
	{
		// Create a new checkpoint data class for the entry
		CheckpointData* pNewCheckpoint = NT_NEW_CHUNK(Mem::MC_MISC) CheckpointData( 0, 0 );

		// Copy it from the buffer
		memcpy(pNewCheckpoint, pReadPtr, sizeof(CheckpointData) );
		pReadPtr += sizeof(CheckpointData);

		// Add it to the array
		CheckpointManager::Get().m_CPDataArray.push_back( pNewCheckpoint ); 
	}
}


//--------------------------------------------------
//!
//!	CheckpointManager::SaveCompleteCallback()
//!	Static callback function, called when the saving is complete
//!
//--------------------------------------------------
void CheckpointManager::SaveCompleteCallback( bool bSuccessful, int iBufferSizeUsed  )
{
	// The process has finished
	m_bSaveDataBusy = false;

	// Hide saving icon
	//SaveData::HideSavingIcon();

	// Did saving fail?
	if ( !bSuccessful )
	{
		ntPrintf("CheckpointManager: Save failed\n");
		return;
	}

	// Was there any data to load?
	if ( iBufferSizeUsed == 0 )
	{
		ntPrintf("ERROR - CheckpointManager: No data was saved???\n");
		return;
	}

	// The process has finished
	m_bSaveDataBusy = false;
}


//--------------------------------------------------
//!
//!	CheckpointManager::ClearCheckpointData()
//! Clears and deletes the current checkpoint data
//!
//--------------------------------------------------
void CheckpointManager::ClearCheckpointData( void )
{
	ntstd::Vector<CheckpointData*>::iterator iter;
	ntstd::Vector<CheckpointData*>::iterator iterEnd = m_CPDataArray.end();

	// Delete each checkpoint data entry
	for ( iter = m_CPDataArray.begin(); iter != iterEnd; iter++ )
	{
		NT_DELETE( (*iter) );
	}

	// Clear the data array
	m_CPDataArray.clear();
}


//--------------------------------------------------
//!
//!	Static functions for LUA Bindings
//!
//--------------------------------------------------
static void Checkpoints_SaveBool( bool bValue )
{
	CheckpointManager::Get().LUASaveBoolToCPData( bValue );
}

static void Checkpoints_SaveNumber( float fNumber )
{
	CheckpointManager::Get().LUASaveFloatToCPData( fNumber );
}

static bool Checkpoints_LoadBool( )
{
	return CheckpointManager::Get().LUALoadBoolFromCPData();
}

static float Checkpoints_LoadNumber( )
{
	return CheckpointManager::Get().LUALoadFloatFromCPData();
}


//--------------------------------------------------
//!
//!	CheckpointManager::RestartFromLastCheckpoint()
//!	Restart from the last checkpoint
//!
//--------------------------------------------------
void CheckpointManager::RestartFromLastCheckpoint()
{
	// Remember the last checkpoint
	const ShellLevel* pPlayingLevel = ShellMain::Get().GetCurrRunningLevel();
	ntAssert( pPlayingLevel );

	//Reset the current level, start at the checkpoint we're interested in
	CGuiManager::Get().ResetLevel(	pPlayingLevel->GetLevelName(),
									pPlayingLevel->GetLastCheckpointID());
}


//--------------------------------------------------
//!
//!	CheckpointManager::GetDataForCheckpoint()
//!	Gets the data for a specific checkpoint.
//!
//--------------------------------------------------
void CheckpointManager::Register( void )
{
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	obGlobals.Set("Checkpoints", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));

	obGlobals["Checkpoints"].Register("SaveBool", Checkpoints_SaveBool);
	obGlobals["Checkpoints"].Register("SaveNumber", Checkpoints_SaveNumber);
	obGlobals["Checkpoints"].Register("LoadBool", Checkpoints_LoadBool);
	obGlobals["Checkpoints"].Register("LoadNumber", Checkpoints_LoadNumber);
	obGlobals["Checkpoints"].Register("RestartFromLastCheckpoint",	CheckpointManager::RestartFromLastCheckpoint);
}


//--------------------------------------------------
//!
//!	CheckpointManager::SetLUAAccessCPData()
//!	Sets the pointer to the CP Data we want LUA to
//! read/write from/to.
//!
//--------------------------------------------------
void CheckpointManager::SetLUAAccessCPData( CheckpointData* pCurrentCheckpointData )
{
	ntAssert( pCurrentCheckpointData );

	// Debug check to make sure we weren't using one already
	ntAssert( m_pLUACPData == 0);

	// Set pointer
	m_pLUACPData = pCurrentCheckpointData;

	// Tell the level data in the cp data to reset it's iterator
	m_pLUACPData->m_LevelData.ResetLUAIterator();
}


//--------------------------------------------------
//!
//!	CheckpointManager::LUASaveBoolToCPData()
//!	Saves a bool to the level data of the cp data
//!
//--------------------------------------------------
void CheckpointManager::LUASaveBoolToCPData( bool bValue )
{
	// Debug check to make sure we're using one first
	ntAssert( m_pLUACPData );

	m_pLUACPData->m_LevelData.SaveBool( bValue );
}


//--------------------------------------------------
//!
//!	CheckpointManager::LUASaveFloatToCPData()
//!	Saves a float to the level data of the cp data
//!
//--------------------------------------------------
void CheckpointManager::LUASaveFloatToCPData( float fValue )
{
	// Debug check to make sure we're using one first
	ntAssert( m_pLUACPData );

	m_pLUACPData->m_LevelData.SaveFloat( fValue );
}


//--------------------------------------------------
//!
//!	CheckpointManager::LUALoadBoolFromCPData()
//!	Loads a bool from the level data of the cp data
//!
//--------------------------------------------------
bool CheckpointManager::LUALoadBoolFromCPData( void )
{
	// Debug check to make sure we're using one first
	ntAssert( m_pLUACPData );

	return m_pLUACPData->m_LevelData.LoadBool();
}


//--------------------------------------------------
//!
//!	CheckpointManager::LUALoadFloatFromCPData()
//!	Loads a float from the level data of the cp data
//!
//--------------------------------------------------
float CheckpointManager::LUALoadFloatFromCPData( void )
{
	// Debug check to make sure we're using one first
	ntAssert( m_pLUACPData );

	return m_pLUACPData->m_LevelData.LoadFloat();
}


//--------------------------------------------------
//!
//!	CheckpointManager::FinishedLUAAccessOfCPData()
//!	Indicate that we have finished reading/writing
//!
//--------------------------------------------------
void CheckpointManager::FinishedLUAAccessOfCPData( void )
{
	// Debug check to make sure we're using one first
	ntAssert( m_pLUACPData );

	// Set pointer to 0
	m_pLUACPData = 0;
}


//--------------------------------------------------
//!
//!	CheckpointManager::ClearAllCheckpointHitFlags()
//!	Indicate that we have finished reading/writing
//!
//--------------------------------------------------
void CheckpointManager::ClearAllCheckpointHitFlags( void )
{
	ntstd::Vector<CheckpointData*>::iterator iter;
	ntstd::Vector<CheckpointData*>::iterator iterEnd = m_CPDataArray.end();

	// For each checkpoint
	for ( iter = m_CPDataArray.begin(); iter != iterEnd; iter++ )
	{
		CheckpointData* pDataBlock = (*iter);

		// Clear the flag
		pDataBlock->m_GenericData.ClearNewlyHitCheckpointFlag();
	}
}


//--------------------------------------------------
//!
//!	CheckpointManager::InsertDataIntoArray()
//!	Inserts the data into the correct place
//! in the array
//!
//--------------------------------------------------
void CheckpointManager::InsertDataIntoArray( CheckpointData* pDataBlock )
{
	ntAssert( pDataBlock );

	ntstd::Vector<CheckpointData*>::iterator iter;
	ntstd::Vector<CheckpointData*>::iterator iterEnd = m_CPDataArray.end();

	for ( iter = m_CPDataArray.begin(); iter != iterEnd; iter++ )
	{
		CheckpointData* pIterBlock = (*iter);

		// If the level number is less, then insert it straight away
		if ( pDataBlock->m_iLevel < pIterBlock->m_iLevel )
		{
			m_CPDataArray.insert( iter, pDataBlock );
			return;
		}
		// Else, check if the level number is the same and if the checkpoint ID is less
		else if ( pDataBlock->m_iLevel == pIterBlock->m_iLevel && pDataBlock->m_iCheckpoint < pIterBlock->m_iCheckpoint )
		{
			m_CPDataArray.insert( iter, pDataBlock );
			return;
		}
	}

	// If we reached this point, then the checkpoint doesn't come before the others,
	// so add it to the end
	m_CPDataArray.push_back( pDataBlock );
}


//--------------------------------------------------
//!
//!	CheckpointGenericData Code
//!
//--------------------------------------------------


//--------------------------------------------------
//!
//!	CheckpointGenericData::CheckpointGenericData()
//!	Constructor
//!
//--------------------------------------------------
CheckpointGenericData::CheckpointGenericData()
{
	// As checkpoint data is only created when the player hits the checkpoint, we set this to true.
	m_bNewlyHitCheckpoint = true;
	m_obStanceStylePoints.Reset();
}


//--------------------------------------------------
//!
//!	CheckpointGlobalData Code
//!
//--------------------------------------------------


//--------------------------------------------------
//!
//!	CheckpointGlobalData::CheckpointGlobalData()
//!	Constructor
//!
//--------------------------------------------------
CheckpointGlobalData::CheckpointGlobalData()
{
	// Set defaults
	// Very negative so that on the first run through a checkpoint a loss of time
	// can still be considered a better time and worth saving
	m_dLifeClockDeltaTime = -9999999.0;
}


//--------------------------------------------------
//!
//!	CheckpointLevelData Code
//!
//--------------------------------------------------


//--------------------------------------------------
//!
//!	CheckpointLevelData::CheckpointLevelData()
//!	Constructor
//!
//--------------------------------------------------
CheckpointLevelData::CheckpointLevelData()
{
	// Set defaults
	for ( int i = 0; i < CheckpointLevelData::s_iNumDataEntries; i++ )
	{
		m_DataArray[i].m_eUnionState = eDataUnused;
		m_DataArray[i].m_Data.m_bBool = false;
	}
}


//--------------------------------------------------
//!
//!	CheckpointLevelData::SaveBool()
//!	Save a bool to the array
//!
//--------------------------------------------------
void CheckpointLevelData::SaveBool( bool bValue )
{
	// Check the iterator hasn't passed the end of the array
	ntError_p( m_iLUAIterator < CheckpointLevelData::s_iNumDataEntries, ("Run out of save slots for checkpoint LUA data") );

	// Save the data
	m_DataArray[m_iLUAIterator].m_Data.m_bBool = bValue;
	m_DataArray[m_iLUAIterator].m_eUnionState = eDataBool;

	// Move the iterator along
	m_iLUAIterator++;
}


//--------------------------------------------------
//!
//!	CheckpointLevelData::SaveFloat()
//!	Save a float to the array
//!
//--------------------------------------------------
void CheckpointLevelData::SaveFloat( float fValue )
{
	// Check the iterator hasn't passed the end of the array
	ntError_p( m_iLUAIterator < CheckpointLevelData::s_iNumDataEntries, ("Run out of save slots for checkpoint LUA data") );

	// Save the data
	m_DataArray[m_iLUAIterator].m_Data.m_fNumber = fValue;
	m_DataArray[m_iLUAIterator].m_eUnionState = eDataNumber;

	// Move the iterator along
	m_iLUAIterator++;
}


//--------------------------------------------------
//!
//!	CheckpointLevelData::LoadBool()
//!	Loads a bool from the array
//!
//--------------------------------------------------
bool CheckpointLevelData::LoadBool( void )
{
	// Check the iterator hasn't passed the end of the array
	ntError_p( m_iLUAIterator < CheckpointLevelData::s_iNumDataEntries, ("Run out of save slots for checkpoint LUA data") );

	// Check the array entry is a bool
	ntError_p( m_DataArray[m_iLUAIterator].m_eUnionState == eDataBool, ("Checkpoint data stored is not a bool") );
	
	// Return the value and increment iterator
	return m_DataArray[m_iLUAIterator++].m_Data.m_bBool;
}


//--------------------------------------------------
//!
//!	CheckpointLevelData::LoadFloat()
//!	Loads a float from the array
//!
//--------------------------------------------------
float CheckpointLevelData::LoadFloat( void )
{
	// Check the iterator hasn't passed the end of the array
	ntError_p( m_iLUAIterator < CheckpointLevelData::s_iNumDataEntries, ("Run out of save slots for checkpoint LUA data") );

	// Check the array entry is a float
	ntError_p( m_DataArray[m_iLUAIterator].m_eUnionState == eDataNumber, ("Checkpoint data stored is not a number") );
	
	// Return the value and increment iterator
	return m_DataArray[m_iLUAIterator++].m_Data.m_fNumber;
}
