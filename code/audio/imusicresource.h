#ifndef _IMUSICRESOURCE_H
#define _IMUSICRESOURCE_H


#include "editable/enumlist.h"


// Forward declarations
namespace FMOD
{
	class Sound;
};







//------------------------------------------------------------------------------------------
//!
//!	SerialisedString
//!
//! A serialised hash string.
//!	
//------------------------------------------------------------------------------------------
class SerialisedString
{
public:

	SerialisedString () {}

	CKeyString				m_obString;
};





//------------------------------------------------------------------------------------------
//!
//!	InteractiveMusicSample
//!
//! A transition describes how a change in intensity should be handled.
//!	
//------------------------------------------------------------------------------------------

class InteractiveMusicSample
{
public:

	InteractiveMusicSample ();
	~InteractiveMusicSample ();

	void PostConstruct ();
	bool EditorChangeValue (CallBackParameter /*pcItem*/,CallBackParameter /*pcValue*/);

	const CHashedString& GetName () { return m_obName; } // Name of this instance
	
	const char* GetSoundFile (int iIndex);
	int GetNumWaves () { return m_obWaveFileList.size(); }

	// ----- Serialised members -----

	ntstd::List<SerialisedString*>		m_obWaveFileList;		// List of music samples
	SELECTION_ORDER						m_eSelectionOrder;		// The order in which sounds are drawn from the pool
	int									m_iPlayCount;			// Number of times the sample is played (0 means loop indefinitely)
	float								m_fVolume;				// Base volume for this music sample
	float								m_fStartTime;			// The time offset from when this kicks in (Note: this will probably need to be quite precise...)

protected:

	CHashedString						m_obName; // Name of this instance
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
	bool EditorChangeValue (CallBackParameter /*pcItem*/,CallBackParameter /*pcValue*/);

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
	bool EditorChangeValue (CallBackParameter /*pcItem*/,CallBackParameter /*pcValue*/);

	const CHashedString& GetName () { return m_obName; } // Name of this instance

	// ----- Serialised members -----

	float									m_fIntensity;		// The min global intensity needed before this can be heard
	float									m_fBeatInterval;	// This acts as a secondary marker - if set to 0, then this is ignored
	float									m_fVolume;			// Base volume for all samples played in this state
	ntstd::List<InteractiveMusicSample*>	m_obSampleList;		// List of wave files that are used by this state
	ntstd::List<InteractiveMusicMarker*>	m_obMarkerList;		// Note: This should be in chronological order. The editor change value function should ensure the list is sorted.

	// Might need to add something that sorts the marker list?

private:

	CHashedString							m_obName; // // Name of this instance
};





//------------------------------------------------------------------------------------------
//!
//!	InteractiveMusicTransition
//!
//! A transition describes how a change in intensity should be handled.
//!	
//------------------------------------------------------------------------------------------

class InteractiveMusicTransition
{
public:

	InteractiveMusicTransition ();
	~InteractiveMusicTransition ();

	void PostConstruct ();
	bool EditorChangeValue (CallBackParameter /*pcItem*/,CallBackParameter /*pcValue*/);

	const CHashedString& GetName (); // Name of this instance

	const char* GetIntermediateSoundFile ();

	bool IsExempt (unsigned int uiSampleID);

	// ----- Serialised members -----

	float				m_fFromIntensity;									// Current intensity
	float				m_fToIntensity;										// Target intensity

	float				m_fSourceEnd1;										// Time at which the existing state starts fading out
	float				m_fSourceEnd2;										// Time at which the existing state has faded out and stops

	float				m_fIntermediateStart1;								// Time at which the intermediate begins fading in
	float				m_fIntermediateStart2;								// Time at which the intermediate finishes fading in
	float				m_fIntermediateEnd1;								// Time at which the intermediate starts fading out
	float				m_fIntermediateEnd2;								// Time at which the intermediate finishes fading out and stops

	float				m_fDestinationStart1;								// Time at which the next state starts fading in
	float				m_fDestinationStart2;								// Time at which the next state finishes fading in

	CKeyString			m_obIntermediateSound;								// Specifies if an intemediate sound should be used for the transition
	float				m_fIntermediateVolume;								// The volume of the intermediate sound

	ntstd::List<InteractiveMusicSample*>	m_obTransitionExemptSamples;	// Samples that are currently playing that ignore the transition

protected:

	CHashedString		m_obName; // Name of this instance
};





#endif // _IMUSICRESOURCE_H

