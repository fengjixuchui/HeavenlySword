//------------------------------------------------------------------------------------------
//!
//!	\file shellupdater.h
//! main update and render loops for the game
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLMAINUPDATER_H
#define GAME_SHELLMAINUPDATER_H

#ifndef GAME_SHELLMAIN_H
#include "game/shellmain.h"
#endif

class ShellImage;
class ShellAnimIcon;

//----------------------------------------------------------------------------------------
//!
//!	ShellMainUpdater
//!	Class defines interface for various shell update modes, if they require local storage.
//!
//----------------------------------------------------------------------------------------
class ShellMainUpdater
{
public:
	virtual ~ShellMainUpdater() {};
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual ShellMain::SHELL_STATE GetType() const = 0;
};

//----------------------------------------------------------------------------------------
//!
//!	SimpleShellUpdater
//! Simplest type, used for early game initialisation
//!
//! note, cannot pump the hud or anything else, as we do not know if our XML files have been
//! installed yet
//!
//----------------------------------------------------------------------------------------
class SimpleShellUpdater : public ShellMainUpdater
{
private:
	ShellImage*		m_pBackground;
	ShellAnimIcon*	m_pAnimatedIcon;
	float			m_Lifetime;

	void NumberAt( float frac, float fX, float fY, float fSize, CVector& col );

public:
	SimpleShellUpdater( const char* pBackground );
	virtual ~SimpleShellUpdater();
	virtual void Update();
	virtual void Render();
	virtual ShellMain::SHELL_STATE GetType() const { return ShellMain::SS_CREATE_GAME; }
};

//----------------------------------------------------------------------------------------
//!
//!	CreateLevelUpdater
//! We have everything we need, use gui if present 
//!
//----------------------------------------------------------------------------------------
class CreateLevelUpdater : public ShellMainUpdater
{
private:
	SimpleShellUpdater*	m_pBackup;
	float				m_loadTime;

public:
	CreateLevelUpdater();
	virtual ~CreateLevelUpdater();
	virtual void Update();
	virtual void Render();
	virtual ShellMain::SHELL_STATE GetType() const { return ShellMain::SS_CREATE_LEVEL; }
};

#endif // GAME_SHELLMAINUPDATER_H
