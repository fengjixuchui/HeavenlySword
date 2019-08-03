/***************************************************************************************************
*
*   $Header:: /game/audioengine_pc.h 7     19/08/03 13:24 Harvey                                   $
*
*	Header file for CAudioEngine and related classes.
*
*	CHANGES		
*
*	20/06/2003	Harvey	Created
*
***************************************************************************************************/

#ifndef AUDIOENGINE_PS3_H
#define AUDIOENGINE_PS3_H

#ifndef _NO_DSOUND
#include <mmreg.h>
#include <dsound.h>
#endif

#ifdef _NO_DSOUND
typedef int IDirectSound8;
typedef int IDirectSoundBuffer8;
typedef int IDirectSound3DBuffer8;
typedef int IDirectSound3DListener8;
typedef int IDirectSoundFXParamEq8;
typedef int IDirectSoundFXI3DL2Reverb8;
#endif

#include "audio/audioresource.h"

class CAudioEngine;


// Audio Engine Notifications

enum AE_NOTIFICATION
{
	AE_NOTIFICATION_UNUSED,
	AE_NOTIFICATION_PLAY,
	AE_NOTIFICATION_STOP,
	AE_NOTIFICATION_LOOP,
	AE_NOTIFICATION_MARKER,
};

// Speaker Setup

enum SPEAKER_CONFIG
{
	SPEAKER_CONFIG_UNKNOWN,
	SPEAKER_CONFIG_MONO,
	SPEAKER_CONFIG_STEREO,
	SPEAKER_CONFIG_SURROUND,
	SPEAKER_CONFIG_DIGITAL,
};


// Effects

enum
{
	FX_NONE		= 0,
	FX_PARAMEQ	= 1,
	FX_REVERB	= 2,
};

struct AUDIO_EFFECT_PARAMEQ
{
	float fCenter;
	float fBandwidth;
	float fGain;
};

struct AUDIO_EFFECT_I3DL2REVERB
{
  long  lRoom;
  long  lRoomHF; 
  float fRoomRolloffFactor;
  float fDecayTime;
  float fDecayHFRatio;
  long lReflections;
  float fReflectionsDelay;
  long lReverb;
  float fReverbDelay; 
  float fDiffusion;
  float fDensity;
  float fHFReference;
};








/***************************************************************************************************
*
*	CLASS			CSound3DInterface
*
*	DESCRIPTION		3D processing interface for use with sound buffers.
*
***************************************************************************************************/

class CSound3DInterface
{
public:

	CSound3DInterface ();
	~CSound3DInterface ();

	bool Initialise (IDirectSoundBuffer8 *pobBuffer); // Set up 3D interface

	void SetPosition (const CPoint& obPosition,bool bImmediate=false);
	void SetMinDistance (float fMinDist,bool bImmediate=false);
	void SetMaxDistance (float fMaxDist,bool bImmediate=false);
	void SetRolloffFactor (float fRolloff,bool bImmediate=false);
	void SetDistanceFactor (float fDistanceFactor,bool bImmediate=false);
	void SetDopplerFactor (float fDopplerFactor,bool bImmediate=false);

	const CPoint& GetPosition () { return m_obPosition; }
	float GetMinDistance () { return m_fMinDistance; }
	float GetMaxDistance () { return m_fMaxDistance; }
	float GetRolloffFactor () { return m_fRolloffFactor; }
	float GetDistanceFactor () { return m_fDistanceFactor; }
	float GetDopplerFactor () { return m_fDopplerFactor; }

private:

	IDirectSound3DBuffer8*			m_pobInterface3D;		// 3d interface

	CPoint							m_obPosition;			// Position of this buffer in world space
	
	float 							m_fMinDistance;			// Minimum distance
	float 							m_fMaxDistance;			// Maximum distance
	float 							m_fRolloffFactor;		// Rolloff factor
	float 							m_fDistanceFactor;		// Distance factor
	float 							m_fDopplerFactor;		// Doppler factor
};









/***************************************************************************************************
*
*	CLASS			CSoundFXInterface
*
*	DESCRIPTION		Sound effects interface for use with sound buffers.
*
***************************************************************************************************/

class CSoundFXInterface
{
public:

	CSoundFXInterface ();
	~CSoundFXInterface ();

	bool Initialise (IDirectSoundBuffer8 *pobBuffer);

	bool SetEffects (u_int uiEffectFlags); // Allows you to set which effects are enabled for a buffer

	void SetParamEq (AUDIO_EFFECT_PARAMEQ* pstParamEq);
	void GetParamEq (AUDIO_EFFECT_PARAMEQ* pstParamEq);
	
	void SetReverb (AUDIO_EFFECT_I3DL2REVERB* pstReverb);
	void GetReverb (AUDIO_EFFECT_I3DL2REVERB* pstReverb);

private:

	IDirectSoundBuffer8*			m_pobSoundBuffer;

	u_int							m_uiEffectFlags;

	IDirectSoundFXParamEq8*			m_pobInterfaceParamEq;	// Parameter EQ interface
	IDirectSoundFXI3DL2Reverb8*		m_pobInterfaceReverb;	// I3DL2 Reverb interface
};












/***************************************************************************************************
*
*	CLASS			CSoundBuffer
*
*	DESCRIPTION		Interface for a sound buffer.
*
***************************************************************************************************/

class CSoundBuffer
{
public:

	enum SOUND_BUFFER_FLAGS
	{
		BUFFER_FLAG_3D		= (1<<0),	// Is this a 3D sound buffer?
		BUFFER_FLAG_MIXIN	= (1<<1),	// Is this a mixin buffer?
		BUFFER_FLAG_LFE		= (1<<2),	// Does this buffer output to LFE only?
		BUFFER_FLAG_LOOP	= (1<<3),	// Does the current sound playing in the buffer set for looping?
		BUFFER_FLAG_PAUSED	= (1<<4),	// Is this buffer paused?
	};

//	CSoundBuffer ();
//	~CSoundBuffer ();
//
//	bool Initialise (IDirectSound8* pobDirectSound,CWaveFile* pobWaveFile,bool b3d,bool bEffects);
//
//	bool Initialise (IDirectSound8* pobDirectSound,CWaveFile* pobWaveFile,u_long ulChannelMask);
//
//	bool Initialise (IDirectSound8* pobDirectSound,WAVEFORMATEX* pstWaveFormat,u_long ulBufferSize);
//
//	void DisplayErrorMessage (HRESULT hResult);

	// Basic functions

	void Play (bool) {}
	void Stop () {}

	bool Lock (void**,u_long,u_long) { return false; }
	bool Unlock (void*,u_long) { return false; }

	bool SetPlayCursor (u_long) { return false; }
	u_long GetPlayCursor () { return 0; }

	void SetVolume (float) {}
	void SetPitch (float) {}

	u_long GetStatus () { return 0; }

	float GetVolume () const					{ return m_fVolume; }
	float GetPitch () const						{ return m_fPitch; }

	//bool Is3d () const							{ return (m_uiFlags & BUFFER_FLAG_3D ? true : false); }

	// Interfaces that provide additional functionality to sound buffers

	CSound3DInterface& Get3DInterface () { return m_obInterface3D; }
	CSoundFXInterface& GetFXInterface () { return m_obInterfaceFX; }

private:

	IDirectSoundBuffer8*			m_pobSoundBuffer;		// Direct sound buffer
//	WAVEFORMATEX*					m_pstWaveFormat;		// Pointer to waveformat structure

	CSound3DInterface				m_obInterface3D;		// 3D interface
	CSoundFXInterface				m_obInterfaceFX;		// Effects interface
	
	u_int							m_uiFlags;				// Buffer flags
	u_long							m_ulStatus;				// Status indicating whether the buffer is playing or not
	
	float 							m_fVolume;				// Base volume for this buffer
	float 							m_fPitch;				// Base pitch for this buffer
};





/***************************************************************************************************
*
*	CLASS			CAudioStream
*
*	DESCRIPTION		Playback by streaming from a source.
*
***************************************************************************************************/

class CAudioStream
{
friend class CAudioEngine;

public:
//
//	CAudioStream (CRITICAL_SECTION* pobCriticalSection);
//	~CAudioStream ();
//
	void Play (u_int) {};
	void Stop () {};
	void Pause () {};
	void Resume () {};

	void SetVolume (float) {}
	void SetPitch (float) {}

	void SetPosition (const CPoint&) {}
	void SetMinDistance (float) {}
	void SetMaxDistance (float) {}

	int	GetAllocationID () const { return m_iAllocationID; }
	
	float GetVolume () const { return 0.0f; }
	float GetPitch () const { return 0.0f; }

	CSound3DInterface& Get3DInterface () { ntAssert(m_pobSoundBuffer); return m_pobSoundBuffer->Get3DInterface(); }
	CSoundFXInterface& GetFXInterface () { ntAssert(m_pobSoundBuffer); return m_pobSoundBuffer->GetFXInterface(); }

private:
//
//	enum STREAM_STATE
//	{
//		STREAM_UNINITIALISED,
//		STREAM_INITIALISED,
//		STREAM_PLAYING,
//		STREAM_PAUSED,
//		STREAM_STOPPED,
//	};
//
	CSoundBuffer*			m_pobSoundBuffer;

//	CWaveFile*				m_pobWaveFile;
//
//	CRITICAL_SECTION*		m_pobCriticalSection; // Critical section object used for accessing shared data
//
//	STREAM_STATE			m_eState;
//
//	bool					m_bLastPacketSubmitted; // Last packet has been submitted (only used inside the thread)
//
	int						m_iAllocationID; // Unique ID which identifies this particular instance

//	u_char					m_ucSilence; // When filling the buffer with silence, this value is used
//	u_long					m_ulPacketSize; // Size of our packets in the buffers (buffersize/2)
//	int						m_iNextPacket; // Next packet to be read
//	u_long					m_ulDataRead; // Amount of data currently read (can be used for calculating progress)
//	u_long					m_ulEndOffset; // The position in the data buffer where this stream should finish
//	
//	u_long					m_ulPosition; // Play position offset into the data
//	u_long					m_ulPlayCursorPosition; // Position of the play cursor relative to the sound buffer
//
//	u_long					m_ulLastPosition; // Last play position position
//	u_long					m_ulLastPlayCursorPosition; // Last play cursor position in buffer (not wave data!)
//	
//	u_int					m_uiPlayCount; // The amount of times we are to play this (0=infinite)
//	u_int					m_uiPlayedCount; // The amount of times we have already played this
//
//	// These functions are only accessible to CAudioEngine
//
//	bool Initialise (IDirectSound8* pobDirectSound,CWaveFile* pobWaveFile,bool b3d);
//	bool Initialise (IDirectSound8* pobDirectSound,CWaveFile* pobWaveFile,u_long ulChannelMask);
//
//	bool Prepare (); // This is the final step in the initialisation of an audio stream
//
//	void ReadFromFile (void* pvDestination,u_long ulPacketSize); // Used inside the thread
//	bool Process (); // Used inside the thread
//
//	void StopStream ();
};




class CAudioStreamAllocation
{
//public:
//
//	CAudioStreamAllocation ()
//	{
//		m_pobAudioStream=NULL;
//		m_bDeleteStream=false;
//	}
//
//private:
//
//	CRITICAL_SECTION	m_obCriticalSection;
//	CAudioStream*		m_pobAudioStream;
//	bool				m_bDeleteStream;
};


/***************************************************************************************************
*
*	CLASS			CAudioEngine
*
*	DESCRIPTION		Main class for managing sound playback.
*
***************************************************************************************************/

class CAudioEngine : public Singleton<CAudioEngine>
{
public:

	static const int			iMAX_STREAMS		= 64;
	static const int			iTHREAD_SLEEP_TIME	= 1000/30;
	static const int			iTHREAD_STACK_SIZE	= 12*1024;


	CAudioEngine ();
	~CAudioEngine ();

	// Audio configuration

	bool			Initialise (); // Initialise the audio engine

//	void			SetSpeakerConfig (SPEAKER_CONFIG eConfig) { m_eSpeakerConfig=eConfig; }
	void			DetectSpeakerConfig ();
//	SPEAKER_CONFIG	GetSpeakerConfig () const { return m_eSpeakerConfig; } // Detect the audio system config
	IDirectSound8*	GetDirectSoundP () { return m_pobDirectSound; } // Get a pointer to the directsound interface

	// Listener properties

	void 			SetListenerPosition (float ,float ,float );
	void 			SetListenerVelocity (float ,float ,float );
	void 			SetListenerOrientation (float ,float ,float ,float ,float ,float );

	// Run-time

	CAudioStream*	CreateStream (u_int ,u_int ,bool );
	CAudioStream*	CreateStream (u_int ,u_int ,u_long );
	CAudioStream*	CreateStream (const char* pcWaveBank,const char* pcWaveFile,bool b3d); // Create a sound instance

	bool			IsPlaying (int);

	void			DoWork (); // Process audio streams
	void			StopAll (); // Kill all sounds and streams

	// Query

	int				GetActiveStreams () const { return m_iActiveStreams; }

	int				GetMaxStreams () const { return iMAX_STREAMS; }

	// Thread handling (DO NOT call this function in the main thread)

	bool			ProcessThread ();

	// Debugging

	void			DisplayDebugInfo ();

private:

	IDirectSound8*				m_pobDirectSound; // Direct sound interface
	IDirectSound3DListener8*	m_pobListener; // PC only

//	HANDLE						m_hThread; // Handle to our processing thread
//
//	SPEAKER_CONFIG				m_eSpeakerConfig; // Speaker configuration
//
//	u_int						m_uiUsed2dBuffers;
//	u_int						m_uiUsed3dBuffers;
//
//	CRITICAL_SECTION			m_aobCriticalSection [iMAX_STREAMS];
//
//	CAudioStream*				m_apobAudioStream [iMAX_STREAMS];
//
//	bool						m_abDeleteStream [iMAX_STREAMS];
//
//	bool						m_bTerminateThread;

	int							m_iActiveStreams;
};

#endif // AUDIOENGINE_PS3_H
