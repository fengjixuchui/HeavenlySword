#ifndef	_INTERACTIVEMUSIC_H
#define	_INTERACTIVEMUSIC_H

#if defined ( PLATFORM_PC )
#include "audio/interactivemusic_pc.h"
#elif defined ( PLATFORM_PS3 )
#include "audio/interactivemusic_ps3.h"
#endif

#if 0 // Work in progress


#define BYTE_ALIGNMENT	4 // This defines how the music should begin in a buffer



enum SELECTION_ORDER // To be moved to enumlist.h
{
	SEQUENTIAL,
	RANDOM_NO_REPEAT,
	SHUFFLED, // Tricky...
};

enum MUSIC_TRANSITION_TYPE // To be moved to enumlist.h
{
	TRANSITION_IMMEDIATE,			// We want to transition immediately
	TRANSITION_ON_MARKER,			// We want to transition on the next marker
	TRANSITION_INTERMEDIATE,		// We want to transition on the next marker with an intermediate sample to go between
};



//------------------------------------------------------------------------------------------
//!
//! InteractiveMusicSample
//!	Describes a sound file that represents part of a music state.
//!
//------------------------------------------------------------------------------------------

class InteractiveMusicSample
{
public:

	InteractiveMusicSample ();

	void PostConstruct ();
	void EditorChangeValue ();

	float GetVolume () const { return (m_bDebugMute ? 0.0f : m_fVolume); }
	float GetStartTime () const { return m_fStartTime; }
	int GetPlayCount () const { return m_iPlayCount; }
	//bool ChangeSoundOnLoop () const { return m_bChangeSoundOnLoop; }
	const CKeyString& GetSoundFile ();

	// ----- Serialised members -----

	ntstd::List<CKeyString*>	m_obSoundFileList;		// List of music samples
	SELECTION_ORDER				m_eSelectionOrder;		// The order in which sounds are drawn from the pool
	bool						m_bChangeSoundOnLoop;	// This determines if a new sound is drawn from the sound pool each time it loops
	int							m_iPlayCount;			// Number of times the sample is played (0 means loop indefinitely)
	float						m_fVolume;				// Base volume for this music sample
	float						m_fStartTime;			// The time offset from when this kicks in (Note: this will probably need to be quite precise...)
	bool						m_bDebugMute;			// Debug flag for muting this sample

protected:

	int							m_iLastPlayed;			// This variable is used to track the last played sound file
};

//------------------------------------------------------------------------------------------
//!
//! InteractiveMusicMarker
//!	This describes a time index point into the music state from the moment it starts playing.
//!
//------------------------------------------------------------------------------------------
class InteractiveMusicMarker
{
public:

	InteractiveMusicMarker ();

	void PostConstruct ();
	void EditorChangeValue ();

	float GetTime () const { return m_fTimeOffset; }

	// ----- Serialised members -----

	float						m_fTimeOffset;
};

//------------------------------------------------------------------------------------------
//!
//! InteractiveMusicState
//!	Defines a composition.
//!
//------------------------------------------------------------------------------------------

class InteractiveMusicState
{
public:

	InteractiveMusicState ();
	~InteractiveMusicState ();

	void PostConstruct ();
	void EditorChangeValue ();

	const CKeyString& GetName () { return m_obName; }

	// ----- Serialised members -----

	float									m_fIntensity;		// The min global intensity needed before this can be heard
	float									m_fBeatInterval;	// This acts as a secondary marker - if set to 0, then this is ignored
	float									m_fVolume;			// Base volume for all samples played in this state
	ntstd::List<InteractiveMusicSample*>	m_obSampleList;		// List of wave files that are used by this state
	ntstd::list<InteractiveMusicMarker*>	m_obMarkerList;		// Note: This should be in chronological order. The editor change value function should ensure the list is sorted.

	// Might need to add something that sorts the marker list?

private:

	CKeyString								m_obName; // This name is taken from the object database for this instance
};

//------------------------------------------------------------------------------------------
//!
//! InteractiveMusicTransition
//!	Defines a transition between music states.
//!
//------------------------------------------------------------------------------------------

class InteractiveMusicTransition
{
public:

	InteractiveMusicTransition ();
	~InteractiveMusicTransition ();

	void PostConstruct ();
	void EditorChangeValue ();

	const CKeyString& GetName () { return m_obName; }

	// ----- Serialised members -----

	CKeyString 					m_obFromState;			// The state we are transitioning from
	CKeyString 					m_obToState;			// The state we are transitioning to

	MUSIC_TRANSITION_TYPE		m_eTransitionType;		// The type of transition

	float						m_fCrossFadeTime;		// If this is zero, there is no cross fade (default is 0.05seconds)
	
	CKeyString					m_obIntermediateSound;	// The name of the intermediate sample (only applicable on intermediate transitions)
	float						m_fIntermediateVolume;	// The volume of the intermediate sample
	float						m_fIntermediateMarker;	// The time offset from when this intermediate sound started playing from when the next transition begins and this fades out.

private:

	CKeyString					m_obName; // This name is taken from the object database for this instance
};


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
//!
//! InteractiveMusicController
//!	Manages a music state
//!
//------------------------------------------------------------------------------------------









enum TRANSITION_TYPE
{
	TRANSITION_NONE,
	TRANSITION_IMMEDIATE,
	TRANSITION_ON_MARKER,
};




enum TRACK_STATE
{
	IDLE,
	PENDING_PLAY,
	PLAYING,
	PENDING_STOP,
};

class InteractiveMusicTrack
{
public:

	InteractiveMusicTrack ();
	~InteractiveMusicTrack ();

	void 
	
	void Process ();		// Called in the main thread
	void Thread_Process();	// Called in the interactive music thread

	void PlaySample (double dStartTime,InteractiveMusicSample* pobSample);
	void StopSample (double dTime);
	
	void SetFadeVolume (float fVolume) { m_fFadeVolume=fVolume; }
	void SetUserVolume (float fVolume) { m_fUserVolume=fVolume; }

protected:

	static const int			iMAX_SOUND_FILES = 20;

	TRACK_STATE					m_eState;

	CriticalSection				m_obCriticalSection;

	uint8_t*					m_pBuffer; // Pointer to our data buffer
	
	TRANSITION_TYPE				m_eTransitionType;

	float						m_fFadeVolume;
	float						m_fUserVolume;
	float						m_fFinalVolume;

	double						m_dTime; // This is actually actually updated in the Thread_Process, and it is used to determine when transitions and stops occur.
										// It is incremented by the buffer size each time we hit a new buffer



	// This section of information is used by the thread and deals with scheduling of sound files to be streamed...
	// Note: need to keep track of the current AND next wave file details when scheduling data to be streamed, for when the track data changes?




	double						m_dTransitionTime;					// The time at which we begin our new sample set

	InteractiveMusicSample*		m_pobMusicSample;					// The current music sample that is being used

	int							m_iLoopsRemaining;					// This represents the number of times a sound file has been played

	AudioWaveBank*				m_pobWaveBank;
	unsigned long				m_ulStartOffset;
	unsigned long				m_ulEndOffset;

	unsigned long				m_ulSeekPosition;					// The position we are currently seeking to in the current sound file (this value is relative to the start position)

	double						m_dStopTime;						// If non-zero, it schedules a time where any sound being fed to the stream is cut off
};





enum MUSIC_STATE
{
	STATE_IDLE,
	STATE_TRANSITIONING,
	STATE_PLAYING,
};


class InteractiveMusicController
{
public:

	InteractiveMusicController () :
		m_pobThisMusicState(0),
		m_pobNextMusicState(0),
		m_pobMusicTransition(0),
		m_eState(STATE_IDLE)
	{
	}

	void StartTransition (InteractiveMusicTransition* pobTransition)
	{
		// Check to see if we have a valid transition passed in
		if (!pobTransition)
			return;

		// Check to see if this is a valid transition - FromState should match whatever the current state is
		if ( (!m_pobThisMusicState && (pobTransition->m_obFromState!="default" || pobTransition->m_obFromState!="" || pobTransition->m_obFromState!="NULL")) ||
			m_pobThisMusicState->GetName()!=pobTransition->m_obFromState)
		{
			return;
		}

		m_pobMusicTransition=pobTransition;
		m_fCrossFadeTime=m_pobMusicTransition->m_fCrossFadeTime;

		switch(m_pobMusicTransition->m_eTransitionType)
		{
			case TRANSITION_STANDARD:			m_eState=STATE_TRANSITION_STANDARD_WAITING_FOR_MARKER;		break;
			case TRANSITION_INTERMEDIATE:		m_eState=STATE_TRANSITION_INTERMEDIATE_WAITING_FOR_MARKER;	break;
			case TRANSITION_XFADE_IMMEDIATE:	m_eState=STATE_TRANSITION_XFADE_CROSSFADING;				break;
			case TRANSITION_XFADE_ON_MARKER:	m_eState=STATE_TRANSITION_XFADE_WAITING_FOR_MARKER;			break;
		}
	}

	void Process (float fTimeDelta); // PPU update






/*
enum MUSIC_STATE
{
	STATE_IDLE,

	STATE_TRANSITION_STANDARD_PERFORM, // Immediately stop the existing music state and start the next one
	STATE_TRANSITION_STANDARD_WAITING_FOR_MARKER, // We are waiting for the next marker

	STATE_TRANSITION_XFADE_CROSSFADING, // We are currently crossfading
	STATE_TRANSITION_XFADE_WAITING_FOR_MARKER, // We are waiting for the next marker or loop point before we start crossfading


	STATE_TRANSITION_INTERMEDIATE_WAITING_FOR_MARKER, // We are waiting for the next marker
	STATE_TRANSITION_INTERMEDIATE_PLAYING_INTERM, // We are currently playing the intermediate sample

	STATE_PLAYING,
};
*/


	void Update (float fTimeDelta)
	{
		switch(m_eState)
		{
			case STATE_TRANSITION_STANDARD_WAITING_FOR_MARKER:
			{
				if (m_pobThisMusicState==0) // Nothing is currently playing
				{
					m_eState=STATE_TRANSITION_STANDARD_PERFORM;
				}

				// When we've hit the next marker...

				m_eState=

				break;
			}

			case MUSIC_PLAYING:
			{
				break;
			}

			default:
				break;
		}
	}



	void Thread_Process ();
	void Thread_RefillBuffer (uint8_t* pTargetBuffer,unsigned int iTargetBufferSize); // Request a buffer refill (thread safe)
	bool Thread_IsBufferRefilling (); // Test to see if the buffer is in the process of refilling (thread safe)
	bool Thread_IsStreamExpired ();


protected:

	const int iMAX_TRACKS = 6;

	CriticalSection					m_obCriticalSection;

	MUSIC_STATE						m_eState;

	float							m_fTimeIndex; // What music sample actually determines the time index for the markers?
    
	float							m_fFinalVolume;
	float							m_fFadeVolume;

	float							m_fCrossFadeTime;

	InteractiveMusicState*			m_pobThisMusicState;
	InteractiveMusicState*			m_pobNextMusicState;
	InteractiveMusicTransition*		m_pobMusicTransition;

	// Markers
	double							m_dNextInterval;
	double							m_dNextMarker;


	int								m_iCurrentTrackSet;
	
	InteractiveMusicTrack			m_obTrack [2][iMAX_TRACKS];
	
	InteractiveMusicTrack			m_obIntermediateTrack;


	// Stream buffer stuff



	STREAM_BUFFER					m_obThisStream;
	STREAM_BUFFER					m_obNextStream;
};



	// Marker information
	double m_dNextIntervalMarker;
	double m_dNextUserMarker;

	float m_fBeatInterval;

	int m_iTotalMarkerDeltas;
	int m_iCurrentMarker;

	float m_afUserMarkerDelta [20];

	// Marker control
	void SetMarkerInformation (InteractiveMusicState* pobMusicState);
	void UpdateMarkers ();
	double GetNextMarker ();










	

	double				m_dTime;

	void IncrementTime () // This is called every time a new buffer is read
	{
		m_dTime+=m_fBufferDuration;	
	}

	float				m_fBufferDuration; // Buffer size / (44100 * 2 * 2)


	unsigned long							m_ulBufferSize;

	unsigned long							m_ulBuffersRead;


	unsigned long							m_ulNextMarkerOffset;
	
	
	float									m_fBeatInterval;

	unsigned long							m_ulBeatInterval;
	ntstd::list<InteractiveMusicMarker*>	m_obMarkerList;

	unsigned int							m_uiNextMarker;
	


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------
//!
//! InteractiveMusicManager
//!	The manager class for all music and user requests.
//!
//------------------------------------------------------------------------------------------
class InteractiveMusicManager : public CSingleton<InteractiveMusicManager>
{
public:

	enum IM_DEBUG_FLAGS
	{
		DEBUG_TTY_INIT			=	1,
		DEBUG_TTY_RESOURCE		=	2,
		DEBUG_TTY_PLAY			=	4,
		DEBUG_TTY_TRANSITION	=	8,	
	};

	friend class InteractiveMusicState;
	friend class InteractiveMusicTransition;

	InteractiveMusicManager ();
	~InteractiveMusicManager ();

	void Init (const CKeyString& obAudioLayer);

	void DoWork ();

	void StartTransition (const CKeyString& obTransitionName);
	void SetGlobalVolume (float fVolume) { m_fGlobalVolume=fVolume; }
	void SetIntensity (float fIntensity);

	void SetGlobalVolumeTransition (float fTargetVolume,float fDuration);

	float GetLayerVolume () { return (m_pobAudioLayer ? m_pobAudioLayer->GetVolume() : 1.0f); }
	float GetGlobalVolume () { return m_fGlobalVolume; }
	float GetIntensity () { return m_fGlobalIntensity; }

protected:

	void RegisterMusicState (InteractiveMusicState* pobMusicState);
	void RegisterMusicTransition (InteractiveMusicTransition* pobMusicTransition);
	void UnregisterMusicState (InteractiveMusicState* pobMusicState);
	void UnregisterMusicTransition (InteractiveMusicTransition* pobMusicTransition);

	InteractiveMusicState* FindMusicState (const CKeyString& obName);
	InteractiveMusicTransition* FindMusicTransition (const CKeyString& obName);

	CAudioLayer*								m_pobAudioLayer; // Audio layer used by the music system
	float										m_fGlobalVolume; // An global music volume modifier
	float										m_fGlobalIntensity;
	bool										m_bIntensityChanged; // This flag indicates if the intensity has been changed
	unsigned int								m_uiDebugFlags;

	ntstd::List<InteractiveMusicState*>			m_obMusicStateList;
	ntstd::list<InteractiveMusicTransition*>	m_obMusicTransitionList;
};



#endif // Work in progress



#endif // _INTERACTIVEMUSIC_H
