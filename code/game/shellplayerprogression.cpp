//------------------------------------------------------------------------------------------
//!
//!	\file shellupdater.cpp
//! main update and render loops for the game
//!
//------------------------------------------------------------------------------------------

#include "shellplayerprogression.h"
#include "game/checkpointmanager.h"

#ifdef PLATFORM_PS3
#include <sysutil/sysutil_common.h>
#endif

//------------------------------------------------------------------------------------------
//!
//!	PlayerProgressionShellUpdater::ctor
//!
//------------------------------------------------------------------------------------------
PlayerProgressionShellUpdater::PlayerProgressionShellUpdater() :
	m_pBackup(0),
	m_eStatus( PP_UNKNOWN )
{
	m_pBackup = NT_NEW SimpleShellUpdater( "startup/status_savedata_progress_colour.dds" );
	
	NT_NEW CheckpointManager();
	CheckpointManager::Get().LoadCheckpointData();
}

//------------------------------------------------------------------------------------------
//!
//!	PlayerProgressionShellUpdater::dtor
//!
//------------------------------------------------------------------------------------------
PlayerProgressionShellUpdater::~PlayerProgressionShellUpdater()
{
	NT_DELETE( m_pBackup );
}

//------------------------------------------------------------------------------------------
//!
//!	PlayerProgressionShellUpdater::Update
//!
//------------------------------------------------------------------------------------------
void PlayerProgressionShellUpdater::Update()
{
	m_pBackup->Update();

#ifdef PLATFORM_PS3
	// pump our system callbacks
	int ret = 0;
	ret = cellSysutilCheckCallback();
	ntError_p(ret==0,("Call to cellSysutilCheckCallback failed! (%d)\n",ret));
#endif

	if ( CheckpointManager::Get().GetSaveDataBusy() == false )
	{
		// load completed or failed
		bool bSaveDataAccessError = false;

		// we can exit to the next stage
		if (bSaveDataAccessError)
		{
			// TBD Martin, we need to put in the apprpriate error handling here,
			// check GameDataShellUpdater for a simple example of what we want if
			// the save data utility throws any errors.

			static bool HandledError = false;

			if (HandledError == false)
			{
				HandledError = true;

				/*
				// delete our backup screen, then create a new one with the relevant message
				NT_DELETE( m_pBackup );

				switch (SomeErrorCode)
				{
				case SaveDataCorrupt: 
					m_pBackup = NT_NEW SimpleShellUpdater( "startup/error_savedata_corrupt.dds" );
					break;

					blah blah blah
				}
				*/

				m_eStatus = PP_FAILED;
			}
		}
		else
		{
			m_eStatus = PP_SUCCEEDED;
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	PlayerProgressionShellUpdater::Render
//!
//------------------------------------------------------------------------------------------
void PlayerProgressionShellUpdater::Render()
{
	m_pBackup->Render();
}
