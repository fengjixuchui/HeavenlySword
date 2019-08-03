//--------------------------------------------------
//!
//!	\file game/entitycheckpoint.h
//!	Definition of the checkpoint object
//!
//--------------------------------------------------

#ifndef	_ENTITY_CHECKPOINT_H
#define	_ENTITY_CHECKPOINT_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"

class CheckpointData;

//--------------------------------------------------
//!
//! Class CheckpointSummaryDef
//! Properties for the checkpoint summary screen
//! Likly to be shared by all check points so not directly on Object_Checkpoint
//!
//--------------------------------------------------
class CheckpointSummaryDef
{
	// Declare dataobject interface
	HAS_INTERFACE(CheckpointSummaryDef)

public:

	CheckpointSummaryDef()
	:	m_fPosX ( 0.5f )
	,	m_fPosY ( 0.5f )
	,	m_fTimeOnScreen ( 5.0f )
	,	m_fNewNameDelay ( 2.0f )
	,	m_bDoSummary	( false )
	,	m_bDoNewCheckpointName	( false )
	{};

	float m_fPosX;
	float m_fPosY;
	float m_fTimeOnScreen;
	float m_fNewNameDelay;

	bool m_bDoSummary;
	bool m_bDoNewCheckpointName;
};
//--------------------------------------------------
//!
//! Class Object_Checkpoint.
//! A checkpoint object type
//!
//--------------------------------------------------
class Object_Checkpoint : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_Checkpoint)

public:
	// Constructor
	Object_Checkpoint();

	// Destructor
	~Object_Checkpoint();

	// Post Construct
	void OnPostConstruct();

	// Jumps and performs loading of checkpoint data
	void			StartAtCheckpoint( void );

	// Called when the player hits a checkpoint
	void			PlayerHitCheckpoint( void );

	// Save and load functions
	void			SaveCheckpoint( CEntity* pPlayer );
	void			LoadCheckpoint( CEntity* pPlayer );

	// Accessors
	const CPoint&	GetPosition() const		{ return m_Position; }
	const CQuat&	GetOrientation() const	{ return m_Orientation; }
	int				GetID() const			{ return m_iID; }

	// Hack for MS14
	void			DebugRenderCheckpoint();

protected:

	// Style point setup for combo unlocking etc.
	void			SetupStylePointsForCheckpoint( void );

	// Loading and saving functions
	bool			SaveGlobalData( CEntity* pPlayer );
	bool			SaveGenericData( CEntity* pPlayer );
	bool			SaveLevelSpecificData( CEntity* pPlayer );

	bool			LoadGlobalData( CEntity* pPlayer );
	bool			LoadGenericData( CEntity* pPlayer );
	bool			LoadLevelSpecificData( CEntity* pPlayer );

	void			SummaryScreen( CEntity* pPlayer );

	// Helper functions
	double			LifeClockDeltaThisCheckpoint( CEntity* pPlayer );
	ntstd::String   CheckpointName( int iLevelID, int iCheckpointID );

	// Disable all the previous checkpoints (used by StartAtCheckpoint function)
	void			DisableAllPreviousCheckpoints( void );

	// Disable's this checkpoint
	void			DisableCheckpoint( void );

	// Debug Output of checkpoint data
	void			DebugPlayerValues( CEntity* pPlayer );

	// Object description.
	CKeyString		m_Description;

	// ID for the checkpoint.
	int				m_iID;

	// Level ID for the checkpoint
	int				m_iLevelID;

	// Position and orientation of the checkpoint.
	// Used to position the player when starting from a checkpoint.
	CPoint			m_Position;
	CQuat			m_Orientation;

	// LUA function to attempt to call when game saves at this checkpoint.  This allows for
	// designers to save out any level specific data they want.
	CHashedString	m_SaveFunction;
	
	// LUA function to attempt to call when the player starts at this checkpoint.
	// Should match the save function and load in the data that was written out.
	CHashedString	m_LoadFunction;

	// LUA function to call when the player starts at this checkpoint.  This allows the
	// designers to perform actions they want to happen when starting at the checkpoint.
	// e.g. change the player to the archer, get some enemies to attack or kick off a NS.
	CHashedString	m_StartFunction;

	// Data structure to write the checkpoint data to, stored in the Checkpoint Manager
	CheckpointData*	m_pobCheckpointData;

	// Parameters for the checkpoint summary screen
	CheckpointSummaryDef* m_pobSummaryDef;
};


#endif // _ENTITY_CHECKPOINT_H
