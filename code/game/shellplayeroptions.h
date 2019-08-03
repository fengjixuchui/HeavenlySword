//------------------------------------------------------------------------------------------
//!
//!	\file shellplayeroptions.h
//! Handle load of the player profile
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLPLAYEROPTIONS_H
#define GAME_SHELLPLAYEROPTIONS_H

#ifndef GAME_SHELLMAINUPDATER_H
#include "game/shellupdater.h"
#endif

//----------------------------------------------------------------------------------------
//!
//!	PlayerOptionsShellUpdater
//! Handle initialisation of game data.
//!
//----------------------------------------------------------------------------------------
class PlayerOptionsShellUpdater : public ShellMainUpdater
{
private:
	SimpleShellUpdater*	m_pBackup;
	void InitGameData();

	enum eSTATUS
	{
		PO_SUCCEEDED,
		PO_FAILED,
		PO_UNKNOWN,
	};

	eSTATUS m_eStatus;

public:
	PlayerOptionsShellUpdater();
	virtual ~PlayerOptionsShellUpdater();
	virtual void Update();
	virtual void Render();
	virtual ShellMain::SHELL_STATE GetType() const { return ShellMain::SS_LOAD_PLAYER_OPTIONS; }

	bool Successful()	const { return (m_eStatus == PO_SUCCEEDED); }
	bool Failed()		const { return (m_eStatus == PO_FAILED); }
};

#endif // GAME_SHELLPLAYEROPTIONS_H
