//------------------------------------------------------------------------------------------
//!
//!	\file shellglobal.h
//! Object that handles creation of minimal resources required to run a game
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLGLOBAL_H
#define GAME_SHELLGLOBAL_H

class ShellMain;

//------------------------------------------------------------------------------------------
//!
//!	ShellGlobal
//! Object that handles creation of minimal resources required to run a game
//!
//------------------------------------------------------------------------------------------
class ShellGlobal
{
public:
	friend class ShellMain;
	friend class CreateLevelUpdater;

private:
	static void SyncInstall( ShellMain* pParent );
	static void AsyncInstall();
	static void Cleanup();

	static void PlatformPreInit( ShellMain* pParent );
	static void PlatformGraphicsInit();
	static void PlatformPostInit();
	static void PlatformPostDestroy();
	static void PlatformUpdate();

	static void Update( bool bUpdateExec = true );
};

#endif // GAME_SHELLGLOBAL_H
