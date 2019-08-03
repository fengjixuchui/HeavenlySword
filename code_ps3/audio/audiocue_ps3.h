/***************************************************************************************************
*
*   $Header:: /game/audiocue_pc.h 6     19/08/03 13:24 Harvey                                      $
*
*	Header file for CSoundBank and related classes.
*
*	CHANGES		
*
*	02/07/2003	Harvey	Created
*
***************************************************************************************************/

#ifndef AUDIOCUE_PS3_H
#define AUDIOCUE_PS3_H

#include "audio/audioengine.h"

#define HEADER_SIZE						sizeof(SOUNDBANKHEADER)
#define CUE_SIZE						sizeof(AUDIO_CUE_PARAMETERS)
#define VOLUME_CONTROL_SIZE				sizeof(AUDIO_VOLUME_CONTROL)
#define PITCH_CONTROL_SIZE				sizeof(AUDIO_PITCH_CONTROL)
#define PARAMEQ_CONTROL_SIZE			sizeof(AUDIO_PARAMEQ_CONTROL)

#define SOUND_POOL_SIZE					20
#define MAX_CONTROL_POINTS				10

enum PLAYBACK_MODE
{
	PLAYBACK_NORMAL=0,				// Playback in order defined
	PLAYBACK_RANDOM=1,				// Standard random
	PLAYBACK_RANDOM_NOWEIGHTS=2,	// Select from pool randomly, disregarding weights
	PLAYBACK_SHUFFLED=3,			// Create a shuffled list and loop
};

enum VOLUME_CURVE
{
	CURVE_LINEAR=0,					// y=x
	CURVE_EXPONENTIAL=1,			// y=x^2
	CURVE_LOGARITHMIC=2,			// y=1-(x^2)
};

enum AUDIO_CUE_FLAG
{
	FLAG_LOOP				= 1,
	FLAG_PAUSABLE			= 2,
	FLAG_VOLUME_VARIATION	= 4,
	FLAG_PITCH_VARIATION	= 8,
	FLAG_REVERB				= 16,
	FLAG_PARAMEQ			= 32,
};

struct SOUND_ENTRY
{
	u_int uiWaveFileID;
	u_int uiWaveBankID;
	u_short usWeight; // (1-100) Only applies if the playback mode is set to RANDOM
};

struct SOUNDBANKHEADER
{
	u_int uiID;

	unsigned short usCueCount;

	unsigned short usVolumeControlCount;
	unsigned short usPitchControlCount;
	unsigned short usParamEqControlCount;
};

struct AUDIO_CUE_PARAMETERS
{
	u_int m_uiID; // ID for this cue

	u_int m_uiCategoryID; // Category ID which this cue belongs to
	
	u_short m_usPriority; // Priority (255 is top priority)
	
	u_char m_ucFlags; // Operation flags

	u_char m_ucPlayCount; // Number of times this should play (0 means infinite loop)
	
	u_char m_ucMaxConcurrentStart; // Maximum times this cue can be fired in the same frame
	u_char m_ucMaxConcurrentPlaying; // Maximum cues of this type that can be playing at any one time

	float m_fDelayTime; // A delay before this cue begins playing

	float m_fVolume; // Base volume
	float m_fEffectsVolume; // Base effects volume
	float m_fPitch; // Base pitch

	float m_fMinVolume; // Minimum volume (if volume variation is specified)
	float m_fMaxVolume; // Maximum volume (if volume variation is specified)

	float m_fMinPitch; // Minimum pitch (if pitch variation is specified)
    float m_fMaxPitch; // Maximum pitch (if pitch variation is specified)

	float m_fMinDist; // Minimum distance before rolloff occurs (used if 3d positioned)
	float m_fMaxDist; // Maximum distance before rolloff stops (used if 3d positioned)

	float m_fFadeIn; // Volume fade in duration (only applies to looping sounds)
	float m_fFadeOut; // Volume fade out duration (only applies to looping sounds)

	VOLUME_CURVE m_eFadeInCurve;
	VOLUME_CURVE m_eFadeOutCurve;

	u_long m_ulChannelMask; // Mixbin configuration

	PLAYBACK_MODE m_ePlaybackMode; // Playback method

	u_char m_ucTotalSounds; // Total sounds used by this cue
	
	SOUND_ENTRY m_astSoundEntry [SOUND_POOL_SIZE]; // Each sound in the pool with corresponding weights
};

struct AUDIO_VOLUME_CONTROL
{
	u_int m_uiID;

	u_char m_ucTotalPoints;

	float m_afInput [MAX_CONTROL_POINTS]; // Input
	float m_afVolume [MAX_CONTROL_POINTS]; // Output
};

struct AUDIO_PITCH_CONTROL
{
	u_int m_uiID;

	u_char m_ucTotalPoints;

	float m_afInput [MAX_CONTROL_POINTS]; // Input
	float m_afPitch [MAX_CONTROL_POINTS]; // Output
};

struct AUDIO_PARAMEQ_CONTROL
{
	u_int m_uiID;

	u_char m_ucTotalPoints;

	float m_afInput [MAX_CONTROL_POINTS]; // Input
	float m_afCenter [MAX_CONTROL_POINTS]; // Output
	float m_afBandwidth [MAX_CONTROL_POINTS]; // Output
	float m_afGain [MAX_CONTROL_POINTS]; // Output
};

/***************************************************************************************************
*
*	CLASS			CAudioCue
*
*	DESCRIPTION		This is a description of an instance where we want to play something.
*
***************************************************************************************************/

class CAudioCue
{
public:

	CAudioCue (AUDIO_CUE_PARAMETERS* pstParam);

	~CAudioCue ();

	void Shuffle (); // Shuffle the sound pool (if applicable)

	void GetSound (u_int& uiBankID,u_int& uiSoundID);

	AUDIO_CUE_PARAMETERS* GetParametersP () { return m_pstCueParameters; }

	u_int GetID () const { return m_pstCueParameters->m_uiID; }

private:

	AUDIO_CUE_PARAMETERS* m_pstCueParameters; // Pointer to a cue parameter structure

	SOUND_ENTRY* m_pstShuffled [SOUND_POOL_SIZE]; // Shuffled sound list (used if playback mode is SHUFFLED)

	int m_iCurrentSound;
};

/***************************************************************************************************
*
*	CLASS			CSoundBank
*
*	DESCRIPTION		Stores a collection of audio cues.
*
***************************************************************************************************/

class CSoundBank
{
public:

	CSoundBank ();

	~CSoundBank ();

	bool Open (const char*);

	bool Reload ();

	CAudioCue* FindAudioCueP (u_int);

	bool GetVolumeControl (const char* ,float ,float &);

	bool GetPitchControl (const char* ,float ,float &);

	bool GetParamEqControl (const char* ,float ,AUDIO_EFFECT_PARAMEQ*);

	const char* GetPath () { return m_acPath; }

	u_int GetID () const { return m_stHeader.uiID; }

	u_short GetTotalCues () const { return m_stHeader.usCueCount; }

protected:

	// TODO Deano changed from HANDLE to void*
	bool					ReadData (void* hFileHandle);
	void					ClearData ();

	char					m_acPath [MAX_PATH];
	
//	FILETIME				m_obModifiedTime; // Modified time of sound bank file
//
	SOUNDBANKHEADER			m_stHeader; // Wavebank header
//
//	AUDIO_CUE_PARAMETERS*	m_pstAudioCue; // Array of waveinfo structures
//	AUDIO_VOLUME_CONTROL*	m_pstVolumeControl; // Array of volume control structures
//	AUDIO_PITCH_CONTROL*	m_pstPitchControl; // Array of pitch control structures
//	AUDIO_PARAMEQ_CONTROL*	m_pstParamEqControl; // Array of parameter eq control structures
//
//	ntstd::List<CAudioCue*>		m_obAudioCueList; // Individual wave files
};

#endif //  AUDIOCUE_PS3_H
