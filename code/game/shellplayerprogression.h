//------------------------------------------------------------------------------------------
//!
//!	\file shellplayeroptions.h
//! Handle load of the player profile
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLPLAYERPROGRESSION_H
#define GAME_SHELLPLAYERPROGRESSION_H

#ifndef GAME_SHELLMAINUPDATER_H
#include "game/shellupdater.h"
#endif

//----------------------------------------------------------------------------------------
//!
//!	PlayerProgressionShellUpdater
//! Handle initialisation of game data.
//!
//----------------------------------------------------------------------------------------
class PlayerProgressionShellUpdater : public ShellMainUpdater
{
private:
	SimpleShellUpdater*	m_pBackup;
	void InitGameData();

	enum eSTATUS
	{
		PP_SUCCEEDED,
		PP_FAILED,
		PP_UNKNOWN,
	};

	eSTATUS m_eStatus;

public:
	PlayerProgressionShellUpdater();
	virtual ~PlayerProgressionShellUpdater();
	virtual void Update();
	virtual void Render();
	virtual ShellMain::SHELL_STATE GetType() const { return ShellMain::SS_LOAD_PLAYER_PROGRESSION; }

	bool Successful()	const { return (m_eStatus == PP_SUCCEEDED); }
	bool Failed()		const { return (m_eStatus == PP_FAILED); }
};

#endif // GAME_SHELLPLAYERPROGRESSION_H
