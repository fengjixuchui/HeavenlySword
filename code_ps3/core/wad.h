//*******************************************************************************
//	
//	Wad.h.
//	
//******************************************************************************/

#ifndef WAD_H_
#define WAD_H_

#include "core/mediatypes.h"

namespace Wad
{
	//
	//	Loads the global WADs' contents into the ArchiveManager.
	//
	void LoadGlobalWADs( MediaTypes read_media );

	//
	//	Load a specific WADs contents into the ArchiveManager.
	//
	void LoadWad( const char *wad_name, MediaTypes read_media, int32_t area_number = -1 );

	//
	//	Installs the WADs that are always needed for the game.
	//	This should be called from within a game-installation
	//	section.
	//
	//	Global WADs are installed to the system-cache.
	//
	void InstallGlobalWADs( MediaTypes destination_media );

	//
	//	Installs the level_name_global.wad for this level.
	//
	void InstallGlobalWADForLevels();

	//
	//	Installs the WADs that are required to start loading
	//	a level of the game. NOTE: This will not install ALL
	//	the WADs for the given level - the remainder are
	//	installed by a thread (setup by this function) that
	//	trickle-installs the remaining WADs in the background.
	//
	void InstallAndLoadLevelWADs( const char *level_name );

	//
	//	Installs any files that are required before the global
	//	or level installs have finished.
	//
	void PostGameDataInstall();

	//
	//	These functions help with our initial GameData setup
	//

	//	Static size required by final game
	inline uint32_t	GetRequiredGameDataSize() { return 1024*1024*1024*1; } // 1 Gb

	//	Contents of file written to in GameData everytime we install a WAD
	//	If our install is interupted we need to know how much more space we require
	inline uint32_t	GetUsedGameDataSize() { return GetRequiredGameDataSize(); }

	//	Used to determine whether we require any more game data 
	//	installation as an explicit stage in game shell.
	bool	HaveCompleteGUIGameData();
	bool	HaveCompleteGlobalGameData();

	//	Check GameData directories for status of any previous installs
	//	NOTE:	FIOS won't be setup to use game-data when this is called so we
	//			just use CellFS calls directly from within this function.
	void	RecordGameDataUsage();

	//	Check SysCache directories for status of any previous installs
	//	NOTE:	FIOS won't be setup to use sys-cache when this is called so we
	//			just use CellFS calls directly from within this function.
	void	RecordSysCacheUsage();

	//	Check that any existing WADs aren't corrupt.
	//	NOTE:	FIOS won't be setup to use game-data when this is called so we
	//			just use CellFS calls directly from within this function.
	bool	AreAnyGameDataWadsCorrupted( const char *game_data_path );
}

#endif // !WAD_H_
