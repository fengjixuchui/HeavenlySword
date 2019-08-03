//------------------------------------------------------------------------------------------
//!
//!	\file shelllevel.h
//! Object that handles loading and unloading of level resources
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLLEVEL_H
#define GAME_SHELLLEVEL_H

class ShellMain;
class Object_Checkpoint;

//------------------------------------------------------------------------------------------
//!
//!	TryInstallXMLFile
//! Publicly available helper function, seems to be used alot.
//!
//------------------------------------------------------------------------------------------
bool TryInstallXMLFile( const char* pcFile );

//------------------------------------------------------------------------------------------
//!
//!	ShellLevel
//! Object that handles loading and unloading of level resources
//!
//------------------------------------------------------------------------------------------
class ShellLevel
{
public:
	friend class ShellMain;

	// Level details
	const char* GetLevelName()		const	{ return m_LevelName.c_str(); }
	int			GetLevelNumber()	const	{ return m_levelNumber; }

	// Current run checkpoint ID - for restarting player at last checkpoint
	void	SetLastCheckpointID( int checkpointID ) { m_currentRunLastCheckpointID = checkpointID; }
	int		GetLastCheckpointID()	const	{ return m_currentRunLastCheckpointID; }
	int		GetStartCheckpointID()	const	{ return m_startCheckpointID; }

	// Checkpoint configuration
	void	RegisterCheckpoint( Object_Checkpoint* pobCheckpoint );
	Object_Checkpoint* GetCheckpointByID( int iCheckpointID );
	void	StartAtCheckpoint( int iCheckpointID );
	void	ActivateChapterCompleteCheckpoint();

private:
	ShellLevel( const char* pLevelName,
				int levelNumber,
				int startCheckpoint );

	~ShellLevel();

	void	SyncPreInstall();
	void	AsyncInstall();
	void	SyncPostInstall();

	void	DestroyLevel( bool bAbortedLoad );

	void	InstallLevelFiles();

	void 	Event_OnLevelStart();
	void 	Event_OnLevelEnd();

	ntstd::String	m_LevelName;
	int				m_levelNumber;
	int				m_startCheckpointID;
	int				m_currentRunLastCheckpointID;

	bool			m_bFirstFrame;

	// Array of checkpoints for the level
	ntstd::Vector<Object_Checkpoint*>	m_CheckpointArray;

	// update level specific objects
	void	Update();
};

#endif // GAME_SHELLLEVEL_H
