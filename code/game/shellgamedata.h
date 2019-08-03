//------------------------------------------------------------------------------------------
//!
//!	\file shellgamedata.h
//! Handle setup of gamedata
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLGAMEDATA_H
#define GAME_SHELLGAMEDATA_H

#ifndef GAME_SHELLMAINUPDATER_H
#include "game/shellupdater.h"
#endif

//----------------------------------------------------------------------------------------
//!
//!	GameDataShellUpdater
//! Handle initialisation of game data.
//!
//----------------------------------------------------------------------------------------
class GameDataShellUpdater : public ShellMainUpdater
{
private:
	SimpleShellUpdater*	m_pBackup;
	void InitGameData();

	enum eSTATUS
	{
		GD_SUCCEEDED,
		GD_FAILED,
		GD_UNKNOWN,
	};

	eSTATUS m_eStatus;

public:
	GameDataShellUpdater();
	virtual ~GameDataShellUpdater();
	virtual void Update();
	virtual void Render();
	virtual ShellMain::SHELL_STATE GetType() const { return ShellMain::SS_CHECK_GAME_DATA; }

	bool Successful()	const { return (m_eStatus == GD_SUCCEEDED); }
	bool Failed()		const { return (m_eStatus == GD_FAILED); }
};

#endif // GAME_SHELLGAMEDATA_H
