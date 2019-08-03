//------------------------------------------------------------------------------------------
//!
//!	\file shellupdater.cpp
//! main update and render loops for the game
//!
//------------------------------------------------------------------------------------------

#include "shellplayeroptions.h"
#include "game/playeroptions.h"

//------------------------------------------------------------------------------------------
//!
//!	PlayerOptionsShellUpdater::ctor
//!
//------------------------------------------------------------------------------------------
PlayerOptionsShellUpdater::PlayerOptionsShellUpdater() :
	m_pBackup(0),
	m_eStatus( PO_UNKNOWN )
{
	m_pBackup = NT_NEW SimpleShellUpdater( "startup/status_savedata_config_colour.dds" );
	
	NT_NEW CPlayerOptions();
	CPlayerOptions::Get().Load();
}

//------------------------------------------------------------------------------------------
//!
//!	PlayerOptionsShellUpdater::dtor
//!
//------------------------------------------------------------------------------------------
PlayerOptionsShellUpdater::~PlayerOptionsShellUpdater()
{
	NT_DELETE( m_pBackup );
}

//------------------------------------------------------------------------------------------
//!
//!	PlayerOptionsShellUpdater::Update
//!
//------------------------------------------------------------------------------------------
void PlayerOptionsShellUpdater::Update()
{
	m_pBackup->Update();

#ifdef PLATFORM_PS3
	// pump our system callbacks
	int ret = 0;
	ret = cellSysutilCheckCallback();
	ntError_p(ret==0,("Call to cellSysutilCheckCallback failed! (%d)\n",ret));
#endif

	if ( CPlayerOptions::Get().GetSaveDataBusy() == false )
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

				m_eStatus = PO_FAILED;
			}
		}
		else
		{
			m_eStatus = PO_SUCCEEDED;
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	PlayerOptionsShellUpdater::Render
//!
//------------------------------------------------------------------------------------------
void PlayerOptionsShellUpdater::Render()
{
	m_pBackup->Render();
}
