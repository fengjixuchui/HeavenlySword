//------------------------------------------------------------------------------------------
//!
//!	\file shellgame.h
//! Object that handles loading and unloading of game resources
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLGAME_H
#define GAME_SHELLGAME_H

//------------------------------------------------------------------------------------------
//!
//!	ShellGame
//! Object that handles loading and unloading of game resources
//!
//------------------------------------------------------------------------------------------
class ShellGame
{
public:
	friend class ShellMain;
	friend class CreateLevelUpdater;

private:
	static void AsyncInstall();
	static void Cleanup();

	static void Update( bool bUpdateGui = true );
	static void RenderSimple();
	static void RenderFull();
};

#endif // GAME_SHELLGAME_H
