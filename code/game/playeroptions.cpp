/***************************************************************************************************
*
*	DESCRIPTION		Definition of the PlayerOptions object.
*
*	NOTES			
*
***************************************************************************************************/

#include "PlayerOptions.h"

#include "audio/audiomixer.h"
#include "game/savedata.h"
#include "gui/guimanager.h"
#include "gui/guitext.h"
#include "game/shellconfig.h"
#include "gui/guisubtitle.h"

// Initialise static member variables
void* CPlayerOptions::m_pSaveDataBuffer = 0;
size_t CPlayerOptions::m_iSaveDataBufferSize = 0;
bool CPlayerOptions::m_bSaveDataBusy = false;
bool CPlayerOptions::m_bVersionError = false;

// Current version number for the game options
//static const int	s_iCurrentGameOptionsVersion = 0;	// First version, 2-10-06, MBunker.
//static const int	s_iCurrentGameOptionsVersion = 1;	// 20-10-06, cam-dwhite, added m_fDialogueVolume.
static const int	s_iCurrentGameOptionsVersion = 2;	// 06-11-06, cam-dwhite, added m_bFirstBoot.


/***************************************************************************************************
*
*	DESCRIPTION		PlayerOptionsData::PlayerOptionsData
*
*	NOTES			Set code defaults
*
***************************************************************************************************/
PlayerOptionsData::PlayerOptionsData( ) :
	m_iVersion( s_iCurrentGameOptionsVersion ),
	m_iMotionControlCalibration( 0 ),
	m_iButtonMapping( 0 ),
	m_AudioLanguage( NT_AUDIO_EURO_ENGLISH ),		// Assuming we want english as default... [ARV]
	m_fSFXVolume( 1.0f ),		// 0 to 1
	m_fMusicVolume( 1.0f ),		// 0 to 1
	m_bShowSubTitles( false ),
	m_bUseMotionControl( false ),
	m_bInvertY( false ),
	m_SubtitleLanguage( NT_SUBTITLE_EURO_ENGLISH ),	// Assuming we want english as default... [ARV]
	m_fDialogueVolume( 1.0f ),	// 0 to 1
	m_bFirstBoot( true )
{
	// If user specified motion sensor in game.config then override the loaded one.
#ifndef _GOLD_MASTER
	if ( g_ShellOptions->m_bOverrideMotionSensorPlayerOption )
	{
		CPlayerOptions::Get().SetUseMotionControl( g_ShellOptions->m_bUseMotionSensor );
	}
#endif
}

//--------------------------------------------------------------------------------------------------
//!
//! PlayerOptionsData::Dump
//!
//! Dump contents of options to tty.
//!
//--------------------------------------------------------------------------------------------------
void PlayerOptionsData::Dump( const char* pcMessage )
{
	ntPrintf( "------------------------------------\n" );
	if	( pcMessage )
	{
		ntPrintf( "PlayerOptions Dump ( %s )\n", pcMessage );
	}
	else
	{	
		ntPrintf( "PlayerOptions Dump\n" );
	}
	ntPrintf( "------------------------------------\n" );
	ntPrintf( "\n" );
	ntPrintf( "m_iVersion = %d\n", m_iVersion );
	ntPrintf( "m_iMotionControlCalibration = %d\n", m_iMotionControlCalibration );
	ntPrintf( "m_iButtonMapping = %d\n", m_iButtonMapping );
	ntPrintf( "m_AudioLanguage = %s\n", Language::GetAudioLanguageName( m_AudioLanguage ) );
	ntPrintf( "m_fSFXVolume = %f\n", m_fSFXVolume );
	ntPrintf( "m_fMusicVolume = %f\n", m_fMusicVolume );
	ntPrintf( "m_bShowSubTitles = %d\n", m_bShowSubTitles );
	ntPrintf( "m_bUseMotionControl = %d\n", m_bUseMotionControl );
	ntPrintf( "m_bInvertY = %d\n", m_bInvertY );
	ntPrintf( "m_SubtitleLanguage = %s\n", Language::GetSubtitleLanguageName( m_SubtitleLanguage ) );
	ntPrintf( "m_fDialogueVolume = %f\n", m_fDialogueVolume );
	ntPrintf( "m_bFirstBoot = %d\n", m_bFirstBoot );
	ntPrintf( "\n" );
	ntPrintf( "------------------------------------\n" );
}

/***************************************************************************************************
*
*	DESCRIPTION		CPlayerOptions::CPlayerOptions
*
*	NOTES			Constructor
*
***************************************************************************************************/
CPlayerOptions::CPlayerOptions() :
	m_uiDialogueSampleSoundID( 0 )
{
	// Initialise PlayerOptions InvertY from the config file.
	// All systems accessing m_bInvertYAxis should now read from
	// player options.
	SetLiveInvertY( g_ShellOptions->m_bInvertYAxis );

	// Create the save data buffer
	if ( !m_pSaveDataBuffer )
	{
		// Calculate the size of the save data buffer to create
		m_iSaveDataBufferSize = sizeof( PlayerOptionsData );
		m_pSaveDataBuffer = (void*)NT_NEW_ARRAY_CHUNK(Mem::MC_MISC) char[m_iSaveDataBufferSize];
	}
}


/***************************************************************************************************
*
*	DESCRIPTION		CPlayerOptions::~CPlayerOptions
*
*	NOTES			Destructor
*
***************************************************************************************************/
CPlayerOptions::~CPlayerOptions()
{
	// Free the save data buffer
	if ( m_pSaveDataBuffer )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, (char*)m_pSaveDataBuffer );
		m_pSaveDataBuffer = 0;
	}
}


//--------------------------------------------------
//!
//!	CPlayerOptions::Load()
//!	Loads in the game options data from disk
//!
//--------------------------------------------------
void CPlayerOptions::Load( void )
{
	// TODO: Add some checks to make sure loading only occurs when a user starts the game?

	m_bVersionError = false;

	// Check that we aren't loading or saving at the moment.
	if ( m_bSaveDataBusy )
	{
		ntPrintf("ERROR - Could not load game options, busy doing something\n");
		return;
	}

	// Clear the memory buffer
	memset( m_pSaveDataBuffer, 0, m_iSaveDataBufferSize );

	// Set the busy flag BEFORE the callback, as on PC that happens immediately
	m_bSaveDataBusy = true;

	// Start the loading process
	if ( !SaveData::LoadInData( m_pSaveDataBuffer, m_iSaveDataBufferSize, LoadOptionsCompleteCallback, SAVE_DATA_TYPE_GAMEOPTIONS ) )
	{
		ntPrintf("ERROR - Could not load game options, SaveData class possibly busy with options data already\n");
		m_bSaveDataBusy = false;
		return;
	}
}


//--------------------------------------------------
//!
//!	CPlayerOptions::Save()
//!	Saves the game options data to disk
//!
//--------------------------------------------------
void CPlayerOptions::Save( void )
{
	// TODO: Add some checks to make sure loading only occurs when a user starts the game?

	// Check that we aren't loading or saving at the moment.
	if ( m_bSaveDataBusy )
	{
		ntPrintf("ERROR - Could not save game options, busy doing something\n");
		return;
	}

	// Copy the game options into the buffer
	memcpy( m_pSaveDataBuffer, &m_obOptions, sizeof(PlayerOptionsData) );

	// Set the busy flag BEFORE the callback, as on PC that happens immediately
	m_bSaveDataBusy = true;

	// Start the saving process
	if ( !SaveData::SaveOutData( m_pSaveDataBuffer, m_iSaveDataBufferSize, SaveOptionsCompleteCallback, SAVE_DATA_TYPE_GAMEOPTIONS ) )
	{
		ntPrintf("ERROR - Could not save game options, SaveData class possibly busy with options data already\n");
		m_bSaveDataBusy = false;
		return;
	}

	// Show saving icon
	SaveData::ShowSavingIcon();
}


/***************************************************************************************************
*
*	DESCRIPTION		CPlayerOptions::LoadOptionsCompleteCallback(
*
*	NOTES			Callback for when the loading process completes
*
***************************************************************************************************/
void CPlayerOptions::LoadOptionsCompleteCallback( bool bSuccessful, int iBufferSizeUsed )
{
	// Did loading fail?
	if ( !bSuccessful )
	{
		ntPrintf("CPlayerOptions: Load failed\n");

		// The process has finished
		m_bSaveDataBusy = false;

		return;
	}

	// Was there any data to load?
	if ( iBufferSizeUsed == 0 )
	{
		ntPrintf("CPlayerOptions: No data to load\n");

		// The process has finished
		m_bSaveDataBusy = false;

		return;
	}

	// Check the version
	int iLoadedVersion;
	memcpy(&iLoadedVersion, m_pSaveDataBuffer, sizeof(int));

	if ( iLoadedVersion != s_iCurrentGameOptionsVersion )
	{
		// Version mismatch
		ntPrintf("ERROR - CPlayerOptions: version mismatch\n");

		// The process has finished
		m_bSaveDataBusy = false;

		// Signal a version error occured.
		m_bVersionError = true;

		return;
	}

	// Version matches so copy the data across
	memcpy( &CPlayerOptions::Get().m_obOptions, m_pSaveDataBuffer, sizeof(PlayerOptionsData) );

	// The process has finished
	m_bSaveDataBusy = false;

	// If user specified motion sensor in game.config then override the loaded one.
#ifndef _GOLD_MASTER
	if ( g_ShellOptions->m_bOverrideMotionSensorPlayerOption )
	{
		CPlayerOptions::Get().SetUseMotionControl( g_ShellOptions->m_bUseMotionSensor );
	}
#endif

	// Display options settings.
	CPlayerOptions::Get().m_obOptions.Dump( "LOAD" );

	// Make sure this works...
	//CSubtitleMan::Get().Enable( CPlayerOptions::Get().m_obOptions.m_bShowSubTitles );
}


/***************************************************************************************************
*
*	DESCRIPTION		CPlayerOptions::SaveOptionsCompleteCallback(
*
*	NOTES			Callback for when the saving process completes
*
***************************************************************************************************/
void CPlayerOptions::SaveOptionsCompleteCallback( bool bSuccessful, int iBufferSizeUsed  )
{
	// Hide saving icon
	SaveData::HideSavingIcon();

	// Did saving fail?
	if ( !bSuccessful )
	{
		ntPrintf("CPlayerOptions: Save failed\n");

		// The process has finished
		m_bSaveDataBusy = false;

		return;
	}

	// Was there any data to load?
	if ( iBufferSizeUsed == 0 )
	{
		ntPrintf("ERROR - CPlayerOptions: No data was saved???\n");

		// The process has finished
		m_bSaveDataBusy = false;

		return;
	}

	// The process has finished
	m_bSaveDataBusy = false;

	// Display options settings.
	CPlayerOptions::Get().m_obOptions.Dump( "SAVE" );
}


/***************************************************************************************************
*
*	DESCRIPTION		Set methods.
*
*	NOTES			
*
***************************************************************************************************/

void CPlayerOptions::SetUseMotionControl( const bool bUseMotionControl )
{
	m_obOptionsTemp.m_bUseMotionControl = bUseMotionControl;
}

void CPlayerOptions::SetMotionControlCalibration( const int iMotionControlCalibration )
{
	m_obOptionsTemp.m_iMotionControlCalibration = iMotionControlCalibration;
}

void CPlayerOptions::SetButtonMapping( const int iButtonMapping )
{
	m_obOptionsTemp.m_iButtonMapping = iButtonMapping;
}

void CPlayerOptions::SetSFXVol( const float fVolume )
{
	m_obOptionsTemp.m_fSFXVolume = fVolume;

	// Game accepts volume level 0..1
	AudioMixer::Get().SetCategoryVolume ( "user_sfx", fVolume );
}

void CPlayerOptions::SetMusicVol( const float fVolume )
{
	m_obOptionsTemp.m_fMusicVolume = fVolume;

	// Game accepts volume level 0..1
	AudioMixer::Get().SetCategoryVolume ( "user_music", fVolume );
}

void CPlayerOptions::SetDialogueVol( const float fVolume )
{
	m_obOptionsTemp.m_fDialogueVolume = fVolume;

	// Game accepts volume level 0..1
	AudioMixer::Get().SetCategoryVolume ( "user_vo", fVolume );

	// Do a sound test here.
	if	( false == AudioSystem::Get().Sound_IsPlaying( m_uiDialogueSampleSoundID ) )
	{
		if ( AudioSystem::Get().Sound_Prepare( m_uiDialogueSampleSoundID, "ui/ui_sb", "ui_looping_vo_test" ) )
		{
			AudioSystem::Get().Sound_SetVolume( m_uiDialogueSampleSoundID, fVolume );
			AudioSystem::Get().Sound_Play( m_uiDialogueSampleSoundID );
		}
	}
}

void CPlayerOptions::SetShowSubtitles( const bool bShow )
{
	m_obOptionsTemp.m_bShowSubTitles = bShow;
	CSubtitleMan::Get().Enable(bShow);
}

void CPlayerOptions::SetInvertY( const bool bInvert )
{
	m_obOptionsTemp.m_bInvertY = bInvert;

	//ntPrintf( "CPlayerOptions::SetInvertY( %d )\n", bInvert );
}

void CPlayerOptions::SetLiveInvertY( const bool bInvert )
{
	m_obOptions.m_bInvertY = bInvert;

	//ntPrintf( "CPlayerOptions::SetLiveInvertY( %d )\n", bInvert );
}

void CPlayerOptions::SetAudioLanguage( AudioLanguage lang )
{
	m_obOptionsTemp.m_AudioLanguage = lang;
}

//--------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::SetLiveAudioLanguage
//!
//! This version of the set audio language function sets the live version instead
//! of the temp version.  It is called during the startup sequence where the language
//! is initially set.  This is necessary because when entering the options screen the live
//! set of options is copied into a temp set and would overwrite the language set during
//! startup if SetAudioLanguage was used.
//!
//--------------------------------------------------------------------------------------------
void CPlayerOptions::SetLiveAudioLanguage( AudioLanguage lang )
{
	m_obOptions.m_AudioLanguage = lang;
}

void CPlayerOptions::SetSubtitleLanguage( SubtitleLanguage lang )
{
	m_obOptionsTemp.m_SubtitleLanguage = lang;

	CStringManager::Get().SetLanguage( lang );
}

//--------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::SetLiveSubtitleLanguage
//!
//! This version of the set subtitle language function sets the live version instead
//! of the temp version.  It is called during the startup sequence where the language
//! is initially set.  This is necessary because when entering the options screen the live
//! set of options is copied into a temp set and would overwrite the language set during
//! startup if SetSubtitleLanguage was used.
//!
//--------------------------------------------------------------------------------------------
void CPlayerOptions::SetLiveSubtitleLanguage( SubtitleLanguage lang )
{
	m_obOptions.m_SubtitleLanguage = lang;

	CStringManager::Get().SetLanguage( lang );
}

void CPlayerOptions::SetLiveFirstBoot( const bool bFirstBoot )
{
	m_obOptions.m_bFirstBoot = bFirstBoot;
}

/***************************************************************************************************
*
*	DESCRIPTION		Get methods.
*
*	NOTES			
*
***************************************************************************************************/

bool CPlayerOptions::GetUseMotionControl( void ) const
{
	return m_obOptionsTemp.m_bUseMotionControl;
}

int	CPlayerOptions::GetMotionControlCalibration( void ) const
{
	return m_obOptionsTemp.m_iMotionControlCalibration;
}

int CPlayerOptions::GetButtonMapping( void ) const
{
	return m_obOptionsTemp.m_iButtonMapping;
}

float CPlayerOptions::GetSFXVol( void ) const
{
	return m_obOptionsTemp.m_fSFXVolume;
}

float CPlayerOptions::GetMusicVol( void ) const
{
	return m_obOptionsTemp.m_fMusicVolume;
}

float CPlayerOptions::GetDialogueVol( void ) const
{
	return m_obOptionsTemp.m_fDialogueVolume;
}

bool CPlayerOptions::GetShowSubtitles( void ) const
{
	return m_obOptionsTemp.m_bShowSubTitles;
}

bool CPlayerOptions::GetLiveShowSubtitles( void ) const
{
	return m_obOptions.m_bShowSubTitles;
}

bool CPlayerOptions::GetInvertY( void ) const
{
	//ntPrintf( "CPlayerOptions::GetInvertY()\n" );

	return m_obOptionsTemp.m_bInvertY;
}

AudioLanguage CPlayerOptions::GetAudioLanguage( void ) const
{
	return m_obOptionsTemp.m_AudioLanguage;
}

SubtitleLanguage CPlayerOptions::GetSubtitleLanguage( void ) const
{
	return m_obOptionsTemp.m_SubtitleLanguage;
}

AudioLanguage CPlayerOptions::GetLiveAudioLanguage( void ) const
{
	return m_obOptions.m_AudioLanguage;
}

SubtitleLanguage CPlayerOptions::GetLiveSubtitleLanguage( void ) const
{
	return m_obOptions.m_SubtitleLanguage;
}

const bool CPlayerOptions::GetLiveFirstBoot( void )
{
	return m_obOptions.m_bFirstBoot;
}

//--------------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::GetFloat
//!
//! Get the float value of the specified player option.
//!
//--------------------------------------------------------------------------------------------------
float CPlayerOptions::GetFloat( const char* pcOption ) const
{
	if ( ! strcmp( "F_SFX_VOLUME", pcOption ) )
	{
		return ( CPlayerOptions::Get().GetSFXVol() );
	}
	else if ( ! strcmp( "F_MUSIC_VOLUME", pcOption ) )
	{
		return ( CPlayerOptions::Get().GetMusicVol() );
	}
	else if ( ! strcmp( "F_DIALOGUE_VOLUME", pcOption ) )
	{
		return ( CPlayerOptions::Get().GetDialogueVol() );
	}
	else
	{
		ntAssert_p( false, ("CPlayerOptions::GetFloat - Option <%s> not supported.", pcOption) );
	}

	return 0.0f;
}

//--------------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::Getint
//!
//! Get the int value of the specified player option.
//!
//--------------------------------------------------------------------------------------------------
int CPlayerOptions::GetInt( const char* pcOption ) const
{
	ntAssert_p( false , ("CPlayerOptions::GetInt - Option <%s> not supported.", pcOption) );

	UNUSED ( pcOption );

	return 0;
}

//--------------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::GetBool
//!
//! Get the bool value of the specified player option.
//!
//--------------------------------------------------------------------------------------------------
bool CPlayerOptions::GetBool( const char* pcOption ) const
{
	if	( ! strcmp( "B_MOTION_CONTROL", pcOption  ) )
	{
		return ( CPlayerOptions::Get().GetUseMotionControl() );
	}
	else if ( ! strcmp( "B_INVERT_Y", pcOption ) )
	{
		return ( CPlayerOptions::Get().GetInvertY() );
	}
	else if ( ! strcmp( "B_SHOW_SUBTITLES", pcOption ) )
	{
		return ( CPlayerOptions::Get().GetShowSubtitles() );
	}
	else
	{
		ntAssert_p( false, ("CPlayerOptions::GetBool - Option <%s> not supported.", pcOption) );
	}

	return false;
}

//--------------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::Changed
//!
//! Returns TRUE if the options changed.
//!
//--------------------------------------------------------------------------------------------------
bool CPlayerOptions::GetChanged( void ) const
{
	if	( memcmp( &m_obOptions, &m_obOptionsTemp, sizeof( m_obOptions ) ) )
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::ComitChanges
//!
//! Commit changes to live options.
//!
//--------------------------------------------------------------------------------------------------
void CPlayerOptions::ComitChanges( void )
{
	NT_MEMCPY( &m_obOptions, &m_obOptionsTemp, sizeof(m_obOptionsTemp) );

	// Options are being comited to the live version so apply any changes.
	CSubtitleMan::Get().Enable( GetLiveShowSubtitles() );
}

//--------------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::CopyToTemp
//!
//! Copy current option settings to the temp options.
//!
//--------------------------------------------------------------------------------------------------
void CPlayerOptions::CopyToTemp( void )
{
	NT_MEMCPY( &m_obOptionsTemp, &m_obOptions, sizeof(m_obOptions) );
}

//--------------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::GetSaveDataBusy
//!
//! Returns TRUE if save/load in progress.
//!
//--------------------------------------------------------------------------------------------------
const bool CPlayerOptions::GetSaveDataBusy( void )
{
	return m_bSaveDataBusy;
}

//--------------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::GetVersionError
//!
//! Returns TRUE if a version error occured upon load.
//!
//--------------------------------------------------------------------------------------------------
const bool CPlayerOptions::GetVersionError( void )
{
	return m_bVersionError;
}

//--------------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::ClearVersionError
//!
//! Clear the version error flag.
//!
//--------------------------------------------------------------------------------------------------
void CPlayerOptions::ClearVersionError( void )
{
	m_bVersionError = false;
}

//--------------------------------------------------------------------------------------------------
//!
//! CPlayerOptions::StopPlayingDialogueSample
//!
//! Stop playing the dialogue sample.
//!
//--------------------------------------------------------------------------------------------------
void CPlayerOptions::StopPlayingDialogueSample( void )
{
	AudioSystem::Get().Sound_SetVolume( m_uiDialogueSampleSoundID, 0.0f );
	AudioSystem::Get().Sound_Stop( m_uiDialogueSampleSoundID );
	m_uiDialogueSampleSoundID = 0;
}
