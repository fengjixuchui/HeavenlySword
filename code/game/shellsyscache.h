//------------------------------------------------------------------------------------------
//!
//!	\file shellsyscache.h
//! Handle setup of system cache
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLSYSCACHE_H
#define GAME_SHELLSYSCACHE_H

#ifndef GAME_SHELLMAINUPDATER_H
#include "game/shellupdater.h"
#endif

//----------------------------------------------------------------------------------------
//!
//!	SysCacheShellUpdater
//! Handle initialisation of system cache area of harddrive
//!
//----------------------------------------------------------------------------------------
class SysCacheShellUpdater : public ShellMainUpdater
{
private:
	SimpleShellUpdater*	m_pBackup;
	void InitSysCache();

	enum eSTATUS
	{
		SC_SUCCEEDED,
		SC_FAILED,
		SC_UNKNOWN,
	};

	eSTATUS m_eStatus;

public:
	SysCacheShellUpdater();
	virtual ~SysCacheShellUpdater();
	virtual void Update();
	virtual void Render();
	virtual ShellMain::SHELL_STATE GetType() const { return ShellMain::SS_CHECK_SYS_CACHE; }

	bool Successful()	const { return (m_eStatus == SC_SUCCEEDED); }
	bool Failed()		const { return (m_eStatus == SC_FAILED); }
};

#endif // GAME_SHELLSYSCACHE_H
