//--------------------------------------------------
//!
//!	\file checkpointmanager.h
//!	Singleton that looks after checkpoints and related
//! data in the game.
//!
//--------------------------------------------------


#if !defined( GAME_CHECKPOINTMANAGER_H )
#define GAME_CHECKPOINTMANAGER_H

#include "editable/enumlist.h"
#include "objectdatabase/dataobject.h"
#include "game/combatstyle.h"


/***************************************************************************************************
*
*	CLASS			ComboUnlockData
*
*	DESCRIPTION		Combo Unlock Data class for storing all the unlocks
*
***************************************************************************************************/
class ComboUnlockData
{
	// Declare dataobject interface
	HAS_INTERFACE(ComboUnlockData)

public:

	ComboUnlockData();
	~ComboUnlockData();

	// Post Construct
	void OnPostConstruct();

	DeepList m_ComboUnlocks;
};


/***************************************************************************************************
*
*	CLASS			ComboUnlockItemData
*
*	DESCRIPTION		A Combo Unlock Item
*
***************************************************************************************************/
class CAttackLink;
class ComboUnlockItemData
{
	// Declare dataobject interface
	HAS_INTERFACE(ComboUnlockItemData)

public:

	// Function to simply see if a combo can be unlocked - doesn't actually unlock it
	bool CanComboBeUnlocked( const StanceStylePoints& obStylePoints );

	CHashedString		m_OriginalLink;
	CHashedString		m_UnlockLink;
	int					m_UnlockStylePoints;
	STANCE_STYLE		m_eStanceStyle;
};


/***************************************************************************************************
*
*	CLASS			GameSaveUniqueData
*
*	DESCRIPTION		Other data that is saved out along with the checkpoints.  This is one-off data,
*					not saved per-checkpoint like the rest
*
***************************************************************************************************/
class GameSaveUniqueData
{
public:
	// Constructor
	GameSaveUniqueData() { m_obSuperStyleLevel = HL_ONE; };

	// Destructor
	~GameSaveUniqueData() {};

	// Gets the Stance Style points last used to view the combos in the menu system
	const StanceStylePoints& GetStylePointsUsedForComboMenu( void ) const { return m_obStylePointsUsedForComboMenu; };
	void SetStylePointsUsedForComboMenu( const StanceStylePoints& obPoints ) { m_obStylePointsUsedForComboMenu = obPoints; };

	const HIT_LEVEL& GetSuperStyleLevel( void ) const { return m_obSuperStyleLevel; };
	void SetSuperStyleLevel( const HIT_LEVEL& obSuperLevel ) { m_obSuperStyleLevel = obSuperLevel; };

private:

	// Stance Style points last used to view the combos in the menu system
	StanceStylePoints	m_obStylePointsUsedForComboMenu;
	HIT_LEVEL			m_obSuperStyleLevel;

};

/***************************************************************************************************
*
*	CLASS			CheckpointGlobalData
*
*	DESCRIPTION		A structure for storing generic data used by a specific checkpoint
*
*	WARNING			If you change the member variables, update the version number in checkpointmanager.cpp
*
***************************************************************************************************/
class CheckpointGenericData
{
public:
	// Constructor
	CheckpointGenericData();

	// Destructor
	~CheckpointGenericData() {};

	// Style points
	const StanceStylePoints& GetStanceStylePoints( void ) const { return m_obStanceStylePoints; };
	void SetStanceStylePoints( const StanceStylePoints& obNewStylePoints )
	{
		m_obStanceStylePoints.m_iStylePointsSpeed = obNewStylePoints.m_iStylePointsSpeed;
		m_obStanceStylePoints.m_iStylePointsPower = obNewStylePoints.m_iStylePointsPower;
		m_obStanceStylePoints.m_iStylePointsRange = obNewStylePoints.m_iStylePointsRange;
		m_obStanceStylePoints.m_iStylePointsAerial = obNewStylePoints.m_iStylePointsAerial;
		m_obStanceStylePoints.m_iStylePointsMisc = obNewStylePoints.m_iStylePointsMisc;
		m_obStanceStylePoints.m_iStylePointsOverall = obNewStylePoints.m_iStylePointsOverall;
	}

	// Newly hit checkpoint flag
	bool IsNewlyHitCheckpoint( void ) const { return m_bNewlyHitCheckpoint; };
	void ClearNewlyHitCheckpointFlag( void ) { m_bNewlyHitCheckpoint = false; };

#ifndef _GOLD_MASTER
	void SetNewlyHitCheckpointFlag( void ) { m_bNewlyHitCheckpoint = true; };
#endif

private:

	// Style points
	StanceStylePoints m_obStanceStylePoints;
	
	// Newly hit checkpoint?
	bool				m_bNewlyHitCheckpoint;
	
};


/***************************************************************************************************
*
*	CLASS			CheckpointGlobalData
*
*	DESCRIPTION		A structure for storing global data used across the entire game (multiple levels)
*
*	WARNING			If you change the member variables, update the version number in checkpointmanager.cpp
*
***************************************************************************************************/
class CheckpointGlobalData
{
public:
	// Constructor
	CheckpointGlobalData();

	// Get
	double	GetLifeClockDelta( void )			{ return m_dLifeClockDeltaTime; };

	// Set
	void	SetLifeClockDelta( double dDelta )	{ m_dLifeClockDeltaTime = dDelta; };

private:
	// Delta time for the lifeclock
	double	m_dLifeClockDeltaTime;

};


/***************************************************************************************************
*
*	CLASS			CheckpointLevelData
*
*	DESCRIPTION		A structure for storing level data used by a specific checkpoint for LUA data
*
*	WARNING			If you change the member variables, update the version number in checkpointmanager.cpp
*
***************************************************************************************************/
class CheckpointLevelData
{
public:
	// Constructor
	CheckpointLevelData();

	// Reset the iterator on the array (used for LUA writing/reading)
	void ResetLUAIterator( void )	{ m_iLUAIterator = 0; }

	// Save functions
	void SaveBool( bool bValue );
	void SaveFloat( float fValue );

	// Load functions
	bool LoadBool( void );
	float LoadFloat( void );

private:

	static const int	s_iNumDataEntries = 20;

	enum DATA_UNION_STATE
	{
		eDataUnused,
		eDataNumber,
		eDataBool
	};

	struct DataEntry
	{
		union Data
		{
			float	m_fNumber;
			bool	m_bBool;
		};
		
		Data				m_Data;
		DATA_UNION_STATE	m_eUnionState;
	};

	DataEntry	m_DataArray[s_iNumDataEntries];

	int			m_iLUAIterator;
};


/***************************************************************************************************
*
*	CLASS			CheckpointDataBlock
*
*	DESCRIPTION		Keeps the global, generic and level data blocks together along with the level and
*					checkpoint ID for easy retrieval.
*
***************************************************************************************************/
class CheckpointData
{
public:

	CheckpointData( int iLevel, int iCheckpoint );
	~CheckpointData();

	int						m_iLevel;		// Level Num
	int						m_iCheckpoint;	// Checkpoint ID

	CheckpointGlobalData	m_GlobalData;	// Global Data
	CheckpointGenericData	m_GenericData;	// Generic Data
	CheckpointLevelData		m_LevelData;	// Level Data
};


/***************************************************************************************************
*
*	CLASS			CheckpointManager
*
*	DESCRIPTION		The checkpoint manager.
*
***************************************************************************************************/
class CheckpointManager : public Singleton<CheckpointManager>
{
public:

	CheckpointManager();
	~CheckpointManager();

	// Register LUA bindings
	static void Register();

	static void RestartFromLastCheckpoint();

	// LifeClock
	double	GetLifeClockToPreviousCheckpoint( CheckpointData* pCurrentCheckpointData );
	double	GetLifeClockToThisCheckpoint( CheckpointData* pCurrentCheckpointData );

	// Get last checkpoint reached by player
	bool	GetLastCheckpointNumberReachedByPlayer( int& iLevel, int& iCheckpoint );

	// Unique Save Data
	//------------------------------------------------------------
	GameSaveUniqueData& GetUniqueSaveData( void ) { return m_obUniqueSaveData; };

	// Checkpoint Data Access
	//------------------------------------------------------------
	// Function for getting the data for a checkpoint, returns 0 if it doesn't exist
	CheckpointData*	GetDataForCheckpoint( int iLevel, int iCheckpoint );

	// Creates a data block for a checkpoint and returns a pointer to it
	CheckpointData* CreateDataForCheckpoint( int iLevel, int iCheckpoint );

	// Gets the last checkpoint the user reached
	CheckpointData* GetLastCheckpoint( void ) const;

	// Combo unlocking & Style Points
	//------------------------------------------------------------
	void UnlockCombos( const StanceStylePoints& obStylePoints, bool bNotifyHUD );
	void SetComboUnlockDataPtr( ComboUnlockData* pComboUnlockData ) { m_pComboUnlockData = pComboUnlockData; };
	void CacheStylePointTotals( void );

	// Saving and Loading
	//------------------------------------------------------------
	void	LoadCheckpointData();
	void	SaveCheckpointData();

	// Reading and Writing functions for LUA bind functions
	//------------------------------------------------------------
	
	// Set the CP Data we want LUA to save/load level data to/from
	void SetLUAAccessCPData( CheckpointData* pCurrentCheckpointData );

	// Save functions
	void LUASaveBoolToCPData( bool bValue );
	void LUASaveFloatToCPData( float fValue );

	// Load functions
	bool LUALoadBoolFromCPData( void );
	float LUALoadFloatFromCPData( void );

	// Finish function when a loading/saving section has completed - must be called
	void FinishedLUAAccessOfCPData( void );

	// Runs over all the existing checkpoints and clears their hit status
	void ClearAllCheckpointHitFlags( void );

#ifndef _GOLD_MASTER
	void Debug_ClearCheckpointData()	{ ClearCheckpointData(); }
#endif

	// check to see if we're accessing the HDD
	static bool GetSaveDataBusy() { return m_bSaveDataBusy; }

private:

	void	InsertDataIntoArray( CheckpointData* pCPData );

	void	ClearCheckpointData( void );

	// Pointer to checkpoint data that is currently being read from/written to
	// This pointer should be 0 when no loading or saving is occuring
	CheckpointData*		m_pLUACPData;

	// The main array of checkpoint data
	ntstd::Vector<CheckpointData*>	m_CPDataArray;

	// The other save data
	GameSaveUniqueData	m_obUniqueSaveData;

	// Combo Unlock Data
	ComboUnlockData* m_pComboUnlockData;

	// Static callbacks required for async loading and saving
	static void LoadCompleteCallback( bool bSuccessful, int iBufferSizeUsed );
	static void	SaveCompleteCallback( bool bSuccessful, int iBufferSizeUsed );

	// Static buffer used to save from or load into.  Must exist while SaveData class is using it.
	static void*	m_pSaveDataBuffer;
	static size_t	m_iSaveDataBufferSize;

	// Are we busy saving/loading data - used to stop another save or load taking place
	static bool m_bSaveDataBusy;
};


#endif // GAME_CHECKPOINTMANAGER_H
