#ifndef _AUDIOMIXER_H
#define _AUDIOMIXER_H

#include "audio/audiosystem.h"
#include "editable/enumlist.h"

class AudioCategory;




//------------------------------------------------------------------------------------------
//!
//! AudioMixerTarget
//!	
//!
//------------------------------------------------------------------------------------------
class AudioMixerTarget
{
public:

	AudioMixerTarget ();
	bool EditorChangeValue (CallBackParameter, CallBackParameter);
	void ObtainCategory ();

	// ----- Serialised members -----

	CHashedString	m_obCategory;
	float			m_fVolume;
	float			m_fPitch;
	MIXER_PAUSE		m_ePause;
	MIXER_STOP		m_eStop;

	// ----- Non-serialised members -----

	AudioCategory* m_pobCategory;
};



//------------------------------------------------------------------------------------------
//!
//! AudioMixerProfile
//!	A profile defines a collection of desired category settings.
//!
//------------------------------------------------------------------------------------------
class AudioMixerProfile
{
public:

	AudioMixerProfile ();
	~AudioMixerProfile ();

	void OnPostConstruct ();
	bool EditorRename (CallBackParameter);

	void Init (); // Needs to be called after categories have been re-initialised

	void DoTransition (float fTransitionTime);
	void Mute (bool bMute);
	void Pause (bool bPause);

	const CHashedString& GetID () { return m_obName; }
	const char* GetName () { return m_obName.GetDebugString(); }

	// ----- Serialised members -----

    ntstd::List<AudioMixerTarget*> m_obMixerTargets;

protected:
	void UpdateName ();

	CHashedString m_obName;
};



//------------------------------------------------------------------------------------------
//!
//! AudioCategory
//!	A mixer handler for an FMOD category.
//!
//------------------------------------------------------------------------------------------
class AudioCategory
{
public:

	AudioCategory (FMOD::EventCategory* pobCategory,int iDepth);

	void Update (float fTimeDelta);

	void DoTransition (AudioMixerTarget* pobMixerTarget,float fTransitionTime);

	void SetVolume (float fVolume);
	void Pause (bool bPause);
	void Mute (bool bMute);
	void Stop ();

	float GetVolume ();
	float GetPitch ();
	bool IsPaused ();
	bool IsMuted ();

	int GetDepth () { return m_iDepth; }

	const CHashedString& GetID () { return m_obName; }
	const char* GetName ();

protected:

	CHashedString			m_obName;

	FMOD::EventCategory*	m_pobCategory;

	int						m_iDepth; // This indicates how many sub categories this category has

	float 					m_fTargetVolume;
	float 					m_fTargetPitch;
	float 					m_fVolumeDelta;
	float 					m_fPitchDelta;

	MIXER_PAUSE				m_ePause;
	MIXER_STOP				m_eStop;
};



//------------------------------------------------------------------------------------------
//!
//! AudioMixer
//!	Our class that manages FMOD categories.
//!
//------------------------------------------------------------------------------------------
class AudioMixer : public Singleton<AudioMixer>
{
public:

	friend class AudioConsole;
	friend class AudioMixerProfile;
	friend class AudioMixerTarget;

	AudioMixer ();
	~AudioMixer ();

	void InitCategories (); // Iterate through category indexes and add them to the category list

	void DoWork ();

	void SetProfile (const CHashedString& obProfileName,float fTransitionTime);
	void SetPreviousProfile (float fTransitionTime);
	void SetProfileMute (const CHashedString& obProfileName,bool bMute);
	void SetProfilePause (const CHashedString& obProfileName,bool bPause);
	void SetCategoryMute (const CHashedString& obCategoryName,bool bMute);
	void SetCategoryPause (const CHashedString& obCategoryName,bool bPause);
	void SetCategoryVolume (const CHashedString& obCategoryName,float fVolume);

	float GetCategoryVolume (const CHashedString& obCategoryName);

	void PauseAll (bool bPause);
	
	bool IsMuted (const CHashedString& obCategoryName);
	bool IsPaused (const CHashedString& obCategoryName);
	const CHashedString& GetActiveProfile () { return m_obCurrentProfile; }

protected:

	void AddProfile (AudioMixerProfile* pobProfile);
	void RemoveProfile (AudioMixerProfile* pobProfile);

	void AddCategories (FMOD::EventSystem* pobSystem);
	void AddCategories (FMOD::EventCategory* pobSystem,int iDepth);

	AudioCategory* GetCategory (const CHashedString& obCategoryName);

	ntstd::List<AudioCategory*> m_obCategoryList;

	ntstd::List<AudioMixerProfile*> m_obProfileList;

	FMOD::EventSystem* m_pobEventSystem;

	CHashedString m_obPreviousProfile;
	CHashedString m_obCurrentProfile;
};



#endif // _AUDIOMIXER_H

