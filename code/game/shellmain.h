//------------------------------------------------------------------------------------------
//!
//!	\file shellmain.h
//! main update and render loops for the game
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLMAIN_H
#define GAME_SHELLMAIN_H

class ShellDebug;
class ShellGlobal;
class ShellGame;
class ShellLevel;
class ShellMainUpdater;

#define USE_ASYNC_STARTUP 

//------------------------------------------------------------------------------------------
//!
//!	ShellMain
//!	Main update and render loops of the game 
//!
//------------------------------------------------------------------------------------------
class ShellMain : public Singleton<ShellMain>
{
public:
	enum SHELL_STATE
	{
		// afterthis, we will be able to perform
		// basic renderering, media access, etc
		SS_STARTUP = 0,

		// Check status of game data, create if missing, display HDD access warning
		SS_CHECK_GAME_DATA,

		// Retrieve player configuration options
		SS_LOAD_PLAYER_OPTIONS,

		// Retrieve player progression
		SS_LOAD_PLAYER_PROGRESSION,

		// Check status of system cache, display HDD access warning
		SS_CHECK_SYS_CACHE,

		// This stage will only be run on first boot (or maybe if
		// a prior install was aborted by the user)
		SS_INSTALL_FE,

		// afterthis, we will have access to the full
		// renderer and game level singletons and player / game data
		SS_CREATE_GAME,

		// This stage will only be run on first boot (or maybe if
		// a prior install was aborted by the user) can potentially
		// use gui if we require anything fancy
		SS_INSTALL_GLOBALS,

		// simple state where no level is loaded, can still render
		SS_RUNNING_EMPTY,

		// afterthis, we can run a game level
		SS_CREATE_LEVEL,

		// flag a level is running
		SS_RUNNING_LEVEL,

		// flag a level is exiting
		SS_SHUTDOWN_LEVEL,

		// flag a game is exiting
		SS_SHUTDOWN_GAME,

		// flag the app is exiting
		SS_EXIT,
	};

	enum PAUSE_MODE
	{
		// for use with stopping level updates during FMV's
		PM_INTERNAL = 0,

		// for use with the user pressing the start button (quit current level, sound etc)
		PM_USER,

		// for use when the game suspends everything (PS button quit application, misc system dialogs)
		PM_SYSTEM,

		PM_MAX,
	};

	ShellMain();
	~ShellMain();

	bool	Update();

	// signal we want a state change of various types
	void	RequestLevelLoad(	const char* pNextLevel,
								int iChapterNumber,
								int iCheckpointID );
	void	RequestLevelUnload();
	void	RequestGameExit();
	void	RequestPauseToggle( PAUSE_MODE type, bool bPaused );

	// Query level objects
	ShellLevel*		GetCurrLoadingLevel();
	ShellLevel*		GetCurrRunningLevel();

	// test to see if we're loading on a seperate thread
	bool	IsLoadingAsync() const { return ((m_currState == SS_CREATE_GAME) || (m_currState == SS_CREATE_LEVEL)); }

	// test to see if we have a loaded level
	bool	HaveLoadedLevel() const { return (m_currState == SS_RUNNING_LEVEL); }

	// test to see what pauses are active (they're not mutually exclusive)
	bool	IsPausedByCode()	const { return m_pauseValue[PM_INTERNAL]; }
	bool	IsPausedByUser()	const { return m_pauseValue[PM_USER]; }
	bool	IsPausedBySystem()	const { return m_pauseValue[PM_SYSTEM]; }
	bool	IsPausedAny()		const { return (m_pauseValue[PM_INTERNAL] || m_pauseValue[PM_USER] || m_pauseValue[PM_SYSTEM]); }

	SHELL_STATE		GetShellState() const { return m_currState; }
	
	// debug functionality
	ShellDebug*		m_pDebugShell;

private:
	static void AsyncCreateGame( ShellMain* pParent );
	static void AsyncCreateGameTask( void* pParam, void* );

	static void AsyncCreateLevel( ShellMain* pParent );
	static void AsyncCreateLevelTask( void* pParam, void* );

	void	MoveToNextState();
	void	UpdateState();

	void	StartState_CreateGame();

	SHELL_STATE			m_currState;
	ShellLevel*			m_pCurrLevel;
	ShellMainUpdater*	m_pUpdater;

	// for level load requests
	bool			m_levelLoadRequested;
	std::string		m_nextLevel;
	int				m_nextLevel_ChapterID;
	int				m_nextLevel_CheckpointID;
	bool			m_levelUnloadRequested;

	// for game exit requests
	bool			m_gameExitRequested;

	// for pause requests
	bool			m_pauseReq[PM_MAX];
	bool			m_pauseReqValues[PM_MAX];
	bool			m_pauseValue[PM_MAX];
};

#endif
