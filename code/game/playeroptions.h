/***************************************************************************************************
*
*	DESCRIPTION		Class to store player options.
*
*	NOTES			
*
***************************************************************************************************/

#ifndef	_PLAYER_OPTIONS_H
#define	_PLAYER_OPTIONS_H

#include "game/languages.h"

/***************************************************************************************************
*
*	CLASS			CPlayerOptionsData
*
*	DESCRIPTION		Data block for the player options, so they can be written out in one go
*
***************************************************************************************************/
struct PlayerOptionsData
{
	// Constructor
	PlayerOptionsData();
	void Dump( const char* pcMessage = NULL );

	// NOTE: Update version number if modifying the below member variables in any way.
	int m_iVersion;
	int	m_iMotionControlCalibration;
	int m_iButtonMapping;
	AudioLanguage m_AudioLanguage;
	float m_fSFXVolume;				// 0..1 in game
	float m_fMusicVolume;			// 0..1 in game
	bool m_bShowSubTitles;
	bool m_bUseMotionControl;
	bool m_bInvertY;
	SubtitleLanguage m_SubtitleLanguage;
	float m_fDialogueVolume;		// 0..1 in game
	bool m_bFirstBoot;
};

/***************************************************************************************************
*
*	CLASS			CPlayerOptions
*
*	DESCRIPTION		Stores player specific game settings.
*
***************************************************************************************************/

class CPlayerOptions : public Singleton< CPlayerOptions >
{
public:

	// Constructor & Destructor
	CPlayerOptions();
	~CPlayerOptions();

	void SetUseMotionControl( const bool bUseMotionControl );
	bool GetUseMotionControl() const;

	void SetMotionControlCalibration( const int iMotionControlCalibration );
	int GetMotionControlCalibration() const;

	void SetButtonMapping( const int iButtonMapping );
	int GetButtonMapping() const;

	void SetSFXVol( const float fVolume );
	float GetSFXVol() const;

	void SetMusicVol( const float fVolume );
	float GetMusicVol() const;

	void SetDialogueVol( const float fVolume );
	float GetDialogueVol() const;

	void SetShowSubtitles( const bool bShow );
	bool GetShowSubtitles() const;
	bool GetLiveShowSubtitles() const;

	// Audio language.
	void				SetAudioLanguage	( AudioLanguage lang );
	void				SetLiveAudioLanguage	( AudioLanguage lang );	// This sets value in m_obOptions not m_obOptionsTemp.
	AudioLanguage		GetAudioLanguage	() const;
	AudioLanguage		GetLiveAudioLanguage( void ) const;

	// Subtitle language.
	void				SetSubtitleLanguage	( SubtitleLanguage lang );
	void				SetLiveSubtitleLanguage	( SubtitleLanguage lang );	// This sets value in m_obOptions not m_obOptionsTemp.
	SubtitleLanguage	GetSubtitleLanguage	() const;
	SubtitleLanguage	GetLiveSubtitleLanguage( void ) const;

	// InvertY Axis
	void SetInvertY( const bool bInvert );
	void SetLiveInvertY( const bool bInvert );	// This sets value in m_obOptions not m_obOptionsTemp.
	bool GetInvertY() const;

	// Get/Set First boot.
	void SetLiveFirstBoot( const bool bFirstBoot );
	const bool GetLiveFirstBoot( void );

	void Save();
	void Load();

	// Get the value of a option from the option string.
	float GetFloat( const char* pcOption ) const;
	int GetInt( const char* pcOption ) const;
	bool GetBool( const char* pcOption ) const;

	// Copy the current option settings to the temp version.
	// The temp version will be commited if options are changed.
	void CopyToTemp( void );

	// Copy temp options to proper options.
	void ComitChanges( void );

	// Get the changed state of the options.
	bool GetChanged( void ) const;

	const bool GetSaveDataBusy( void );

	const bool GetVersionError( void );
	void ClearVersionError( void );

	void StopPlayingDialogueSample( void );

private:

	// Save this
	PlayerOptionsData m_obOptions;

	// Static callbacks required for async loading and saving
	static void LoadOptionsCompleteCallback( bool bSuccessful, int iBufferSizeUsed );
	static void	SaveOptionsCompleteCallback( bool bSuccessful, int iBufferSizeUsed );

	// Static buffer used to save from or load into.  Must exist while SaveData class is using it.
	static void*	m_pSaveDataBuffer;
	static size_t	m_iSaveDataBufferSize;

	// Are we busy saving/loading data - used to stop another save or load taking place
	static bool m_bSaveDataBusy;

	// Temp options. These are modified in the options screen then written back to
	// m_obOptions on confirmation.
	PlayerOptionsData m_obOptionsTemp;

	static bool m_bVersionError;

	unsigned int m_uiDialogueSampleSoundID;
};

#endif
