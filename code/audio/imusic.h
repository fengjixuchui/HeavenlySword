
#ifndef _IMUSIC_H
#define _IMUSIC_H


#include "editable/enumlist.h"


#define MUSIC_BUFFER_SIZE			64*1024
#define MUSIC_DECODE_BUFFER_SIZE	64*1024
#define MUSIC_STREAM_BUFFER_SIZE	32*1024
#define MUSIC_NUM_TRACKS			6
#define MUSIC_BYTES_PER_SECOND		176400
#define MUSIC_BLOCK_ALIGNMENT		4
#define MUSIC_MAX_STREAMS			8
#define MUSIC_MAX_MARKERS			64
#define MUSIC_POOL_SIZE				16


namespace FMOD
{
	class System;
	class Sound;
	class Channel;
	class EventCategory;
};

class InteractiveMusicTransition;
class InteractiveMusicState;
class InteractiveMusicSample;







//------------------------------------------------------------------------------------------
//!
//!	AudioDecoder
//!
//! FMOD wrapper for providing an interface for decoding an audio file - the file can be
//! any format supported by FMOD.
//!	
//------------------------------------------------------------------------------------------
class AudioDecoder
{
public:

	AudioDecoder ();
	~AudioDecoder ();

	void Release (); // Release the current FMOD Sound object

	bool LoadFile (const char* pcFileName,bool bAccurateTime); // Set a new audio file for stream (it will release any existing Sound being used)
	void SetLoopPoints (unsigned int uiStart,unsigned int uiEnd);
	unsigned int GetData (uint8_t* pDestination,unsigned int uiReadSize); // Decode a chunk of data to the destination address, returns the actual amount successfully read
	bool Seek (unsigned int uiPosition); // Seek to a particular position in the file
	
	const char* GetFileName () { return m_acFileName; }
	unsigned int GetLength () { return m_uiLength; } // Return the PCM length of the file
	unsigned int GetSeekPosition () { return m_uiPosition; } // Return the current PCM position in the file
	bool IsValid () const { return m_pobSound; } // Check to see if this decoder has a valid FMOD Sound object

private:

	char			m_acFileName [256];		// Name of the file thats currently configured for this decoder

	FMOD::Sound*	m_pobSound;				// Pointer to our FMOD Sound object
	unsigned int	m_uiPosition;			// The current seek offset into the file (in bytes)
	unsigned int	m_uiLength;				// Actual PCM length of the file (in bytes)
	unsigned int	m_uiStartPos;			// Start position for the stream (can't be less than 0)
	unsigned int	m_uiEndPos;				// End position for the stream (can't be greater than the length)
};






//------------------------------------------------------------------------------------------
//!
//!	InteractiveMusicStream
//!
//! Manages a stream of audio data.
//!	
//------------------------------------------------------------------------------------------

class InteractiveMusicStream
{
public:

	InteractiveMusicStream ();
	~InteractiveMusicStream ();

	void SetWorkBuffer (uint8_t* pBuffer) { m_pBuffer=pBuffer; } // Set the memory work space for this stream (assumes there is already sufficient space)

	void SetStream (const char* pcFileName,SELECTION_ORDER eSelectionOrder,int iPlayCount,float fVolume); // We are configuring the stream to play a single audio file
	void SetStream (InteractiveMusicSample* pobSample,SELECTION_ORDER eSelectionOrder,int iPlayCount,float fVolume); // We are configuring the stream to play a pool of audio files

	void SetStart (unsigned long ulStartPosition1,unsigned long ulStartPosition2); // Set the start points
	void SetEnd (unsigned long ulEndPosition1,unsigned long ulEndPosition2); // Set the end points

	void Process (unsigned long ulPosition,void* pDestination,unsigned long ulBufferSize); // Our function where we fetch data from the wave file and then mix it into the destination buffer

	bool IsFree () { return m_bFree; } // Used to test this stream to see if its available for re-use
	
	unsigned int GetSampleID (); // This is used to get information regarding the InteractiveMusicSample that is currently playing on this stream (if there is one)

protected:

	void ReadWaveData (uint8_t* pBuffer,unsigned long ulSize); // Read a certain amount of data to the target buffer
	
	bool SetNextWaveFile (); // Configures the audio decoder for the next audio file

	bool						m_bFree;					// This flag represents whether or not this stream is free

	uint8_t*					m_pBuffer;					// Work buffer for this stream
	
	unsigned long				m_ulStartPosition1; 		// Offset where the stream begins
	unsigned long				m_ulStartPosition2; 		// Offset where the stream finishes fading in
	unsigned long				m_ulEndPosition1;			// Offset where the stream begins fading out
	unsigned long				m_ulEndPosition2;			// Offset where the stream has finished

	AudioDecoder				m_obAudioDecoder;			// Our audio file decoder

	InteractiveMusicSample*		m_pobMusicSample;			// Pointer to music sample (which contains a pool of audio files)

	char						m_acTargetFileName [256];	// Name of our target audio file for this stream

	int							m_iNumWaves;				// Number of wave files
	int							m_iWaveIndex;				// Index of current wavefile
	SELECTION_ORDER				m_eSelectionOrder;			// The ordering type in which sounds are drawn from the pool
	int							m_iPlayCount;				// Number of times sound files that should be played (0 means loop indefinitely)
	float						m_fVolume;					// The base volume of this stream (0.0 to 1.0)
};




//------------------------------------------------------------------------------------------
//!
//!	InteractiveMusicManager
//!
//! Manager class for the music system.
//!	
//------------------------------------------------------------------------------------------

class InteractiveMusicManager : public Singleton<InteractiveMusicManager>
{
public:

	friend class InteractiveMusicSample;
	friend class InteractiveMusicState;
	friend class InteractiveMusicTransition;

	InteractiveMusicManager ();
	~InteractiveMusicManager ();

	// ----- Main thread functions -----
	void Init ();
	void SetMediaPath (const char* pcPath);
	void SetGlobalVolume (float fVolume);
	void SetPause (bool bPause);
	void SetIntensity (float fIntensity);
	void StopAll (float fFadeTime);

	// ----- Worker thread functions -----
	void ThreadUpdate (void* pBuffer,unsigned int uiSize);

	// ----- Debugging -----
	float GetGlobalVolume ();
	float GetPosition ();
	float GetIntensity () { return m_fGlobalIntensity; }
	void ExtendFilePath (const char* pcFileName,char* pcOutputPath);

protected:

	// Marker functions
	void SetMarkerInformation (InteractiveMusicState* pobMusicState,unsigned long ulPosition);
	void UpdateMarkers (unsigned long ulPosition,unsigned long ulBufferSize);
	unsigned long GetNextMarker (unsigned long ulPosition);

	// Registration functions
	void RegisterMusicState (InteractiveMusicState* pobMusicState);
	void RegisterMusicTransition (InteractiveMusicTransition* pobMusicTransition);
	void UnregisterMusicState (InteractiveMusicState* pobMusicState);
	void UnregisterMusicTransition (InteractiveMusicTransition* pobMusicTransition);

	// Utility functions
	InteractiveMusicState* FindMusicState (const CHashedString& obName);
	InteractiveMusicTransition* FindMusicTransition (const CHashedString& obName);
	InteractiveMusicStream* FindFreeStream ();
	bool IsPlaying (unsigned int uiSampleID);

	// ----- System -----

	CriticalSection			m_obCriticalSection; // For multi-threading

	uint8_t*				m_pStreamBuffer; // A temporary buffer used for streams before mixing their output

	FMOD::System*			m_pobSystem; // Pointer to our FMOD system
	FMOD::Sound*			m_pobSound; // Pointer to our FMOD sound
	FMOD::Channel*			m_pobChannel; // Pointer to our FMOD channel
	FMOD::EventCategory*	m_pobCategory; // This is taken from the event system and is used to set the base volume & pause status for the music system

	float					m_fGlobalIntensity; // This value represents the current intensity of the music

	float					m_fStopFade;

	unsigned long			m_ulPosition; // This position is used to schedule streams on this track

	char					m_acMediaPath [256]; // Media path prefix used by all audio files

	// ----- Markers -----

	unsigned long			m_ulNextBeatMarker; // Position of next beat interval
	unsigned long			m_ulNextUserMarker; // Position of next user marker

	unsigned long			m_ulBeatInterval; // Beat interval length

	int						m_iNumMarkers; // Total marker deltas
	int						m_iCurrentMarker; // Index of current marker delta

	unsigned long			m_ulMarkerDelta [MUSIC_MAX_MARKERS]; //  Array of marker deltas

	// ----- Transition control -----

	InteractiveMusicTransition*					m_pobTransition; // Pointer to the active transition (set to 0 if there is no transition)
	InteractiveMusicState*						m_pobTargetMusicState; // Pointer to the target music state (if we are in a transition)

	// ----- Streams -----

	InteractiveMusicStream						m_obStream [MUSIC_MAX_STREAMS]; // Fixed size array of streams

	// ----- Resources -----

	ntstd::List<InteractiveMusicState*>			m_obMusicStateList; // List of music states
	ntstd::List<InteractiveMusicTransition*>	m_obMusicTransitionList; // List of music transitions
};




#endif // _IMUSIC_H

