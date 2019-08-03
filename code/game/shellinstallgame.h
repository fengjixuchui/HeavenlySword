//------------------------------------------------------------------------------------------
//!
//!	\file shellinstallgame.h
//! Handle setup of gamedata
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLINSTALL_H
#define GAME_SHELLINSTALL_H

#ifndef GAME_SHELLMAINUPDATER_H
#include "game/shellupdater.h"
#endif

//----------------------------------------------------------------------------------------
//!
//!	InstallFrontendShellUpdater
//! content_neutral and select
//!
//----------------------------------------------------------------------------------------
class InstallFrontendShellUpdater : public ShellMainUpdater
{
private:
	SimpleShellUpdater*	m_pBackup;

	bool m_bInstalled;

public:
	InstallFrontendShellUpdater();
	virtual ~InstallFrontendShellUpdater();
	virtual void Update();
	virtual void Render();
	virtual ShellMain::SHELL_STATE GetType() const { return ShellMain::SS_INSTALL_FE; }

	bool Successful()	const { return m_bInstalled; }
};

//----------------------------------------------------------------------------------------
//!
//!	InstallGlobalsShellUpdater
//! Handle initialisation of game data.
//!
//----------------------------------------------------------------------------------------
class InstallGlobalsShellUpdater : public ShellMainUpdater
{
private:
	SimpleShellUpdater*	m_pBackup;

	bool m_bInstalled;

public:
	InstallGlobalsShellUpdater();
	virtual ~InstallGlobalsShellUpdater();
	virtual void Update();
	virtual void Render();
	virtual ShellMain::SHELL_STATE GetType() const { return ShellMain::SS_INSTALL_GLOBALS; }

	bool Successful()	const { return m_bInstalled; }
};


#endif // GAME_SHELLINSTALL_H
