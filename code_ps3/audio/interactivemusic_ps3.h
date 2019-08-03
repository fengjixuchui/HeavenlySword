#ifndef	_INTERACTIVEMUSIC_PS3_H
#define	_INTERACTIVEMUSIC_PS3_H

#include "editable/enumlist.h"
#include "audio/audioengine.h"

//------------------------------------------------------------------------------------------------------------------

enum SEGMENT_STAGE
{
	STAGE_INACTIVE,	// This usually indiciates the segment has finished
	STAGE_INTRO,	// Segment is currently in intro part
	STAGE_MAIN,		// Segment is currently in main part
	STAGE_OUTRO,	// Segment is currently in outro part
};

/*
enum TRACK_STOP
{
	STOP_IMMEDIATE,	// Immediately stops current sample
	STOP_ONLOOP,	// Stops current sample on loop point (end of wavefile)
	STOP_ONMARKER,	// Stops current sample on next marker (or loop point if no markers available)
	STOP_FINISH,	// Current segment plays outro sample on the next loop
	STOP_FADE,		// Fade out this track
	STOP_TIMED,		// Timed stop
};
*/

/* Declared in enumlist.h

enum SELECTION_METHOD
{
	MANUAL,				// Sample is selected through interface
	SEQUENTIAL,			// Samples are chosen in numerical order
	RANDOM,				// Samples are randomly selected
	RANDOM_NOWEIGHTS,	// Samples are randomly selected (assuming each sample has equally weighted)
	SHUFFLED,			// Samples are chosen in numerical order from a shuffled list
};

*/

//------------------------------------------------------------------------------------------------------------------



class CMusicSample
{
public:

	CMusicSample ();

	virtual ~CMusicSample ();

	virtual bool EditorChangeValue (CallBackParameter, CallBackParameter);

	void Initialise ();

	CHashedString		m_obWaveFile;		// Wave file ID
	CHashedString		m_obWaveBank;		// Wave bank ID
	int				m_iPlayCount;		// Number of times to play this sample (0 means do not play)
	int				m_iWeight;			// Probability that this sample is played (if RANDOM selection)

	CWaveFile*		m_pobWaveFile;		// Pointer to wave file (NULL by default, this is filled in by the music system)
};

//------------------------------------------------------------------------------------------------------------------

// Note: CMusicSegment not only contains information regarding the samples it uses, but
// it also deals with the logic. Its responsibility is purely to provide the music track
// with wavefiles.

class CMusicSegment
{
public:

	CMusicSegment ();

	~CMusicSegment ();

	void Initialise ();

	void SetStage (SEGMENT_STAGE eStage);

	void IncreaseLevel (); // Switch to next segment (only applicable if selection method is manual)
	
	void DecreaseLevel (); // Switch to previous segment (only applicable if selection method is manual)

	CWaveFile* GetNextWaveFileP (); // When this returns a NULL, the segment is finished

	const char* GetNameP () const; // Get actual text label (for debugging purposes)

	u_int GetID () const { return CHashedString( GetNameP() ).Get(); } // Get ID for this music segment

	CMusicSample* GetCurrentSampleP () { return m_pobCurrentSample; }

	// XML fields

	// These parameters are for the main part of the segment
	SELECTION_METHOD		m_eSelectionMethod; // Method in which we draw samples from the main pool
	int						m_iFirstSample; // Starting smaple in the main pool (manual selection only)
	float					m_fDuration; // Enables the main part to play for a limited time (Default is 0 which means it loops continuously)
	
	ntstd::List<CMusicSample*>	m_obIntroSampleList;
	ntstd::List<CMusicSample*>	m_obMainSampleList;
	ntstd::List<CMusicSample*>	m_obOutroSampleList;

	// Constants
	
	static const int iMAIN_SAMPLE_POOL_SIZE = 10;

private:

	SEGMENT_STAGE			m_eSegmentStage;
	
	int						m_iCurrentSample; // Current sample

	int						m_iCurrentSamplePlayCount; // Number of times we've played current sample

	CMusicSample*			m_apobShuffledSamples [iMAIN_SAMPLE_POOL_SIZE];	// Shuffled sample list

	float					m_fPlayTime; // Sum of file durations that have been played

	CMusicSample*			m_pobCurrentSample; // Pointer to the current music sample pointed to by this segment

	// Helper functions

	CMusicSample*			GetSampleInList (ntstd::List<CMusicSample*> *pobList,int iIndex);

	void					SetRandomSample (ntstd::List<CMusicSample*> *pobList);

};

//------------------------------------------------------------------------------------------------------------------



// Work in progress!

enum TRACK_NOTIFICATION
{
	NOTIFICATION_UNUSED,
	NOTIFICATION_LOOP,		// Track has hit a loop point (start or end of a file)
	NOTIFICATION_MARKER,	// Track has hit a marker
};




class CMusicStream // This represents a sample process on a music track
{
public:

	void Initialise (CWaveFile* pobWaveFile);

	CWaveFile*	m_pobWaveFile; // Pointer to the wavefile

	u_long		m_ulDataRead; // Amount of data currently read (can be used for calculating progress)
	u_long		m_ulEndOffset; // The position in the data buffer where this stream should finish

	u_int		m_uiTotalMarkers; // Total markers in this stream
};


// Note: the processing thread doesn't need to know anything about the segment.
// It only needs to know the current wavefile and the next wavefile.

class CMusicTrack // A special audio stream that spins constantly on the music process thread
{
public:

	CMusicTrack (const char* pcName);
	~CMusicTrack ();

//	bool Initialise (IDirectSound8* pobDirectSound,WAVEFORMATEX* pstFormat,u_long ulBufferSize);

	//void Synchronize (CMusicTrack* pobMusicTrack); // Sets the play cursor to match the play cursor position of another stream

	void SetSegment (CMusicSegment* pobSegment,SEGMENT_STAGE eStage,TRACK_STOP eStopOption);
	void Stop (TRACK_STOP eStopOption,float fDuration=0.0f); // Terminates all segments on this track

	void Begin (); // Begin playing this buffer

	
	void MainUpdate (); // Main thread update
	void ThreadUpdate (); // Thread update

	CWaveFile* GetNextWaveP ();

	
	void ReadFromFile (void* pvDestination,u_long ulOffset); // Inside thread


	

	void IncreaseLevel (); // Increase level on the current segment
	void DecreaseLevel (); // Decrease level on the current segment

	void RestartSegment ();
	
	void Pause ();
	void Resume ();

	void SetVolume (float fVolume);
	void SetPitch (float fPitch);

	float GetVolume () const { return m_fBaseVolume; }
	float GetPitch () const { return m_fBasePitch; }

	const char* GetNameP () const { return ntStr::GetString(m_obName); }
	u_int GetID () const { return m_obName.GetHash(); }

	float GetPlayPosition () const; // Get the play position (in seconds)
	float GetDuration () const; // Get the duration (in seconds)
	float GetNextMarkerPosition () const;
	float GetTimeRemainingUntilTransition () { return m_fTimeRemaining; }
	//float GetProgress () const; // Get the percentage progress of this stream (0.0 - 1.0)

	CMusicSegment* GetCurrentMusicSegmentP () { return m_pobCurrentSegment; }

	u_int GetCurrentSegmentID () const;

	void SetTrackNotification (TRACK_NOTIFICATION eNotification);

	TRACK_NOTIFICATION GetTrackNotification ();

	

private:

	float GetTimeRemainingToTransition () const;

	CSoundBuffer*				m_pobSoundBuffer;


	CHashedString					m_obName; // Name of this music track

	float						m_fBaseVolume;

	float						m_fBasePitch;


	// Position monitoring

	TRACK_NOTIFICATION			m_eNotification;
	u_long						m_ulNotificationOffset; // An offset into the sound buffer where something interesting has happened

	u_long						m_ulPosition; // Offset into a wave file thats playing

	u_int						m_uiCurrentMarker;
	u_long						m_ulNextMarkerOffset;

	bool						m_bResetPosition; // This is flagged when a switch to a new stream is made

	// Volume fading

	bool						m_bRestoreFadeVolume; // If this is flagged, it resets the fader to 1.0
	float						m_fFadeVolume;
	float						m_fFadeDecrement;


	float						m_fTimeRemaining; // Estimated time remaining until transition to next stage/segment

	float						m_fStopTime;
	bool						m_bStopTrack;
	TRACK_STOP					m_eTrackStop;

	bool						m_bForceStreamChange; // If this is set to true, it will force the thread to switch to the next stream

	u_long						m_ulChangeEndOffset; // If this is not zero, we want to change the end offset for the current stream

	// Stream management

	CMusicStream				m_aobStream [2];
	int							m_iCurrentStream;

	CMusicSegment*				m_pobCurrentSegment;
	CMusicSegment*				m_pobNextSegment;

	u_char						m_ucSilence; // When filling the buffer with silence, this value is used

	u_long						m_ulPacketSize; // Size of our packets in the buffers (buffersize/2)
	int							m_iNextPacket; // Next packet to be read

	u_long						m_ulPlayCursorPosition; // Position of the play cursor relative to the sound buffer
	u_long						m_ulLastPlayCursorPosition; // Last play cursor position in buffer (not wave data!)

	bool						m_bRestart;


	void						ChangeAudioStream ();

	void						RestoreFadeVolume ();

	void						StopOnNextMarker ();
};





class CInteractiveMusicManager : public Singleton<CInteractiveMusicManager>
{
public:
	
	CInteractiveMusicManager ();

	~CInteractiveMusicManager ();

	void Initialise (long lChannels,u_long ulSamplesPerSec,long lBitsPerSample,int iMillisec); // This is only called once

	bool AddMusicTrack (const char* pcName); // Called after initialise

	bool Begin (); // Start the thread

	bool PlaySegment (const char* pcSegment,const char* pcTrack,TRACK_STOP eStopOption);

	bool StopTrack (const char* pcTrackName,TRACK_STOP eStopOption,float fDuration=0.0f);

	bool SetCrossoverTrigger (const char* pcSourceTrack, float fTimeRemaining, const char* pcTargetTrack, const char* pcCrossoverSegment);

	void StopAll (bool bOnMarker);

	void Pause ();

	void Resume ();

	void Update ();

	void DebugRender ();

	void EnableDebugRender (bool bEnable) { m_bDebugRender=bEnable; }
	
	bool IsSegmentPlaying (const char* pcSegment,const char* pcTrack); // Determine if a segment is playing

	TRACK_NOTIFICATION GetTrackNotification (const char* pcTrack);

	void DisplayDebugInfo ();

	// Thread function - used internally!

	bool ThreadUpdate ();

	// Volume / Pitch settings

	void SetTrackVolume (const char* pcTrackName,float fVolume);
	void SetTrackPitch (const char* pcTrackName,float fPitch);

	float GetTrackVolume (const char* pcTrackName);
	float GetTrackPitch (const char* pcTrackName);

	void SetVolume (float fVolume) { m_fGlobalVolume=fVolume; }
	void SetPitch (float fPitch) { m_fGlobalPitch=fPitch; }

	float GetVolume () const { return m_fGlobalVolume; }
	float GetPitch () const { return m_fGlobalPitch; }

	// Constants

	static const int iTHREAD_STACK_SIZE = 12*1024;
	static const int iTHREAD_SLEEP_TIME = 1000/30;

private:

	IDirectSound8*			m_pobDirectSoundInterface;

//	WAVEFORMATEX			m_stWaveFormat; // The format for the music system

	u_long					m_ulBufferSize; // Buffersize to be used by all music tracks

//	HANDLE					m_hProcessThread; // Handle to the music thread

	ntstd::List<CMusicTrack*>		m_obMusicTrackList; // List of music tracks used by the thread

	bool					m_bDebugRender; // Flag for debug rendering

	bool					m_bRequestPause; // Request a pause on all sound buffers
	bool					m_bRequestResume; // Request a resume on all sound buffers

	bool					m_bTerminateThread; // Flag to signal the thread to stop

	float					m_fGlobalVolume; // Global volume modifier for all tracks
	float					m_fGlobalPitch; // Global pitch modifier for all tracks


	CMusicTrack*			m_pobCrossover_SourceTrack;
	CMusicTrack*			m_pobCrossover_TargetTrack;
	CMusicSegment*			m_pobCrossover_Segment;
	float					m_fCrossover_TimeRemaining;

	CMusicTrack*			GetMusicTrackP (const char* pcTrack);
	
	CMusicSegment*			GetMusicSegmentP (const char* pcSegment);
};

#endif // _INTERACTIVEMUSIC_PS3_H
