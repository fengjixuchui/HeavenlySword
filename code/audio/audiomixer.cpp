#include "audio/audiomixer.h"
#include "audio/audiosystem.h"
#include "audio/imusic.h"
#include "core/timer.h"
#include "objectdatabase/dataobject.h"

#include "fmod_event.h"
#include "fmod_errors.h"



START_CHUNKED_INTERFACE	(AudioMixerTarget,	Mem::MC_MISC)
	ISTRING			(AudioMixerTarget, Category)
	IFLOAT			(AudioMixerTarget, Volume)
	IFLOAT			(AudioMixerTarget, Pitch)
	IENUM			(AudioMixerTarget, Pause, MIXER_PAUSE)
	IENUM			(AudioMixerTarget, Stop, MIXER_STOP)
	DECLARE_EDITORCHANGEVALUE_CALLBACK(EditorChangeValue)
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(AudioMixerProfile,	Mem::MC_MISC)
	PUBLISH_PTR_CONTAINER_AS(m_obMixerTargets, MixerTargets)
	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
	DECLARE_EDITORRENAME_CALLBACK(EditorRename)
END_STD_INTERFACE







//----------------------------------------------------------------------------------------------------------------------------------------------

AudioMixerTarget::AudioMixerTarget () :
	m_fVolume(1.0f),
	m_fPitch(0.0f),
	m_ePause(NOPAUSE),
	m_eStop(NOSTOP),
	m_pobCategory(0)
{
}

bool AudioMixerTarget::EditorChangeValue (CallBackParameter, CallBackParameter)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	ObtainCategory();

	if (m_pobCategory)
		m_pobCategory->DoTransition(this, 0.1f);

	return true;

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioMixerTarget::ObtainCategory ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	//if (!m_pobCategory && AudioMixer::Exists())
	if (AudioMixer::Exists())
		m_pobCategory = AudioMixer::Get().GetCategory(m_obCategory);

#endif // _AUDIO_SYSTEM_ENABLE
}

//----------------------------------------------------------------------------------------------------------------------------------------------

AudioMixerProfile::AudioMixerProfile ()
{
	AudioMixer::Get().AddProfile(this);
}

AudioMixerProfile::~AudioMixerProfile ()
{
	if (AudioMixer::Exists())
		AudioMixer::Get().RemoveProfile(this);
}

void AudioMixerProfile::OnPostConstruct ()
{
	UpdateName();
}

bool AudioMixerProfile::EditorRename (CallBackParameter)
{
    UpdateName();
	return true;
}

void AudioMixerProfile::UpdateName ()
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );

	if ( pDO )
		m_obName = CHashedString(pDO->GetName());
}

void AudioMixerProfile::Init ()
{
	for(ntstd::List<AudioMixerTarget*>::iterator obIt=m_obMixerTargets.begin(); obIt!=m_obMixerTargets.end(); ++obIt)
	{
		(*obIt)->ObtainCategory();
	}
}

void AudioMixerProfile::DoTransition (float fTransitionTime)
{
	for(ntstd::List<AudioMixerTarget*>::iterator obIt=m_obMixerTargets.begin(); obIt!=m_obMixerTargets.end(); ++obIt)
	{
		if ((*obIt)->m_pobCategory)
			(*obIt)->m_pobCategory->DoTransition(*obIt,fTransitionTime);
	}
}

void AudioMixerProfile::Mute (bool bMute)
{
	for(ntstd::List<AudioMixerTarget*>::iterator obIt=m_obMixerTargets.begin(); obIt!=m_obMixerTargets.end(); ++obIt)
	{
		if ((*obIt)->m_pobCategory)
			(*obIt)->m_pobCategory->Mute(bMute);
	}
}

void AudioMixerProfile::Pause (bool bPause)
{
	for(ntstd::List<AudioMixerTarget*>::iterator obIt=m_obMixerTargets.begin(); obIt!=m_obMixerTargets.end(); ++obIt)
	{
		if ((*obIt)->m_pobCategory)
			(*obIt)->m_pobCategory->Pause(bPause);
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------

AudioCategory::AudioCategory (FMOD::EventCategory* pobCategory,int iDepth) :
	m_pobCategory(pobCategory),
	m_iDepth(iDepth),
	m_ePause(NOPAUSE),
	m_eStop(NOSTOP)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	ntAssert(pobCategory); // Theres no reason why an invalid category should be passed in

	char* pcName=0;

	if (m_pobCategory->getInfo(0,&pcName)==FMOD_OK)
	{
		m_obName=CHashedString(pcName);
	}

	m_pobCategory->getVolume(&m_fTargetVolume);
	m_pobCategory->getPitch(&m_fTargetPitch);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioCategory::DoTransition (AudioMixerTarget* pobMixerTarget,float fTransitionTime)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (pobMixerTarget)
	{
		//printf("DoTransition %s %.3fsec\n",GetName(),fTransitionTime);

		m_fTargetVolume=pobMixerTarget->m_fVolume;
		m_fTargetPitch=pobMixerTarget->m_fPitch;
		m_fVolumeDelta=m_fTargetVolume-GetVolume();
		m_fPitchDelta=m_fTargetPitch-GetPitch();

		if (fTransitionTime > EPSILON)
		{
			m_fVolumeDelta/=fTransitionTime;
			m_fPitchDelta/=fTransitionTime;
		}
		else
		{
			m_fVolumeDelta=0.0f;
			m_fPitchDelta=0.0f;
		}

		// Pause category
		m_ePause=pobMixerTarget->m_ePause;

		if (m_ePause==IMMEDIATE_PAUSE)
		{
			Pause(true);
		}
		else if (m_ePause==IMMEDIATE_UNPAUSE)
		{
			Pause(false);
		}

		// Stop category
		m_eStop=pobMixerTarget->m_eStop;

		if (m_eStop==IMMEDIATE) // We want to stop all events immediately
		{
			Stop();
		}
	}

#else

	UNUSED(pobMixerTarget);
	UNUSED(fTransitionTime);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioCategory::SetVolume (float fVolume)
{
	m_fTargetVolume=fVolume;
	m_fVolumeDelta=0.0f;
	m_pobCategory->setVolume(fVolume);
}

void AudioCategory::Pause (bool bPause)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	FMOD_RESULT eResult;
	
	eResult=m_pobCategory->setPaused(bPause);

#ifndef _RELEASE
	if (eResult!=FMOD_OK)
	{
		ntPrintf("AudioCategory: SetPaused failed - %s\n",FMOD_ErrorString(eResult));
	}
#endif // _RELEASE

#else

	UNUSED(bPause);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioCategory::Mute (bool bMute)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	m_pobCategory->setMute(bMute);

#else

	UNUSED(bMute);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioCategory::Stop ()
{
#ifdef _AUDIO_SYSTEM_ENABLE
	m_pobCategory->stopAllEvents();
#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioCategory::Update (float fTimeDelta)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	bool bTransitionComplete=false;

	float fVolume,fPitch;
	m_pobCategory->getVolume(&fVolume);
	m_pobCategory->getPitch(&fPitch);
	
	if (fVolume!=m_fTargetVolume) // Volume needs to be updated
	{
		if (m_fVolumeDelta!=0.0f) // Gradual change
		{
			float fDelta=m_fVolumeDelta * fTimeDelta;

			fVolume+=fDelta;

			if ((fDelta<0.0f && fVolume<m_fTargetVolume) || (fDelta>0.0f && fVolume>m_fTargetVolume))
			{
				fVolume=m_fTargetVolume;
				bTransitionComplete=true;
			}

			m_pobCategory->setVolume(fVolume);
		}
		else // Immediate change
		{
			m_pobCategory->setVolume(m_fTargetVolume);
			bTransitionComplete=true;
		}
	}

	if (fPitch!=m_fTargetPitch) // Pitch needs to be updated
	{
		if (m_fPitchDelta!=0.0f) // Gradual change
		{
			float fDelta=m_fPitchDelta * fTimeDelta;

			fPitch+=fDelta;

			if ((fDelta<0.0f && fPitch<=m_fTargetPitch) || (fDelta>=0.0f && fPitch>=m_fTargetPitch))
			{
				fPitch=m_fTargetPitch;
				bTransitionComplete=true;
			}

			m_pobCategory->setPitch(fPitch);
		}
		else // Immediate change
		{
			m_pobCategory->setPitch(m_fTargetPitch);
			bTransitionComplete=true;
		}
	}

	// A volume/pitch transition has just finished
	if (bTransitionComplete)
	{
		//printf("Transition complete %s\n",GetName());

		// Check to see if we want to trigger a pause/unpause after the transition is complete
		if (m_ePause==TRANSITION_PAUSE)
		{
			Pause(true);
		}
		else if (m_ePause==TRANSITION_UNPAUSE)
		{
			Pause(false);
		}

		m_ePause=NOPAUSE;

		// Check to see if we want to trigger a stop after the transition is complete
		if (m_eStop==TRANSITION)
		{
			Stop();
		}
		
		m_eStop=NOSTOP;
	}

#else

	UNUSED(fTimeDelta);

#endif // _AUDIO_SYSTEM_ENABLE
}

float AudioCategory::GetVolume ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	float fVolume;
	m_pobCategory->getVolume(&fVolume);
	return fVolume;

#else

	return 0.0f;

#endif // _AUDIO_SYSTEM_ENABLE
}

float AudioCategory::GetPitch ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	float fPitch;
	m_pobCategory->getPitch(&fPitch);
	return fPitch;

#else

	return 0.0f;

#endif // _AUDIO_SYSTEM_ENABLE
}

bool AudioCategory::IsPaused ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	bool bPaused;
	
	if (m_pobCategory->getPaused(&bPaused)==FMOD_OK)
        return bPaused;

#endif // _AUDIO_SYSTEM_ENABLE

	return false;
}

bool AudioCategory::IsMuted ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	bool bMuted;

	if (m_pobCategory->getMute(&bMuted)==FMOD_OK)
		return bMuted;

#endif // _AUDIO_SYSTEM_ENABLE

	return false;


}

const char* AudioCategory::GetName ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	char* pcName;
	m_pobCategory->getInfo(0,&pcName);
	return pcName;

#else

	return 0;

#endif // _AUDIO_SYSTEM_ENABLE
}


//---------------------------------------------------------------------------------------------------------------------------------------------


AudioMixer::AudioMixer () :
	m_pobEventSystem(0)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	m_pobEventSystem=AudioSystem::Get().GetFMODEventSystem();

#endif // _AUDIO_SYSTEM_ENABLE
}

AudioMixer::~AudioMixer ()
{
	while(!m_obCategoryList.empty())
	{
		NT_DELETE( m_obCategoryList.back() );
		m_obCategoryList.pop_back();
	}
}

void AudioMixer::InitCategories ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_pobEventSystem)
		return;

	// Ensure the category list is completely empty first
	while(!m_obCategoryList.empty())
	{
		NT_DELETE( m_obCategoryList.back() );
		m_obCategoryList.pop_back();
	}

	/*
	int iTotalCategories;

	m_pobEventSystem->getNumCategories(&iTotalCategories);

	for(int i=0; i<iTotalCategories; ++i)
	{
		FMOD::EventCategory* pobCategory=0;
	
		if (m_pobEventSystem->getCategoryByIndex(i,&pobCategory)==FMOD_OK)
		{
			m_obCategoryList.push_back( NT_NEW AudioCategory(pobCategory) );
		}
	}
	*/
	AddCategories(m_pobEventSystem);

	for(ntstd::List<AudioMixerProfile*>::iterator obIt=m_obProfileList.begin(); obIt!=m_obProfileList.end(); ++obIt)
	{
		(*obIt)->Init();
	}

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioMixer::AddCategories (FMOD::EventSystem* pobSystem)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	int iTotalCategories=0;

	pobSystem->getNumCategories(&iTotalCategories);

	for(int i=0; i<iTotalCategories; ++i)
	{
		FMOD::EventCategory* pobEventCategory=0;
	
		if (pobSystem->getCategoryByIndex(i,&pobEventCategory)==FMOD_OK)
		{
			AudioCategory* pobAudioCategory=NT_NEW AudioCategory(pobEventCategory,0);

			if (pobAudioCategory)
			{
				m_obCategoryList.push_back( pobAudioCategory );

				AddCategories(pobEventCategory,0);
			}
		}
	}

#else

	UNUSED(pobSystem);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioMixer::AddCategories (FMOD::EventCategory* pobCategory,int iDepth)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	int iTotalCategories=0;

	pobCategory->getNumCategories(&iTotalCategories);

	for(int i=0; i<iTotalCategories; ++i)
	{
		FMOD::EventCategory* pobEventCategory=0;

		if (pobCategory->getCategoryByIndex(i,&pobEventCategory)==FMOD_OK)
		{
			AudioCategory* pobAudioCategory=NT_NEW AudioCategory(pobEventCategory,iDepth+1);

			if (pobAudioCategory)
			{
				m_obCategoryList.push_back( pobAudioCategory );

				AddCategories(pobEventCategory,iDepth+1);
			}
		}
	}

#else

	UNUSED(pobCategory);
	UNUSED(iDepth);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioMixer::DoWork ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_pobEventSystem)
		return;

	// Update our categories

	const float fTimeDelta=CTimer::Get().GetSystemTimeChange();

	for(ntstd::List<AudioCategory*>::iterator obIt=m_obCategoryList.begin(); obIt!=m_obCategoryList.end(); ++obIt)
	{
		(*obIt)->Update(fTimeDelta);
	}

	// Update music volume
	float fMusicVolume=GetCategoryVolume(CHashedString("user_music")) * GetCategoryVolume(CHashedString("music"));

	InteractiveMusicManager::Get().SetGlobalVolume(fMusicVolume);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioMixer::SetProfile (const CHashedString& obProfileName,float fTransitionTime)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (obProfileName==m_obCurrentProfile) // We are already using the requested profile
		return;

	for(ntstd::List<AudioMixerProfile*>::iterator obIt=m_obProfileList.begin(); obIt!=m_obProfileList.end(); ++obIt)
	{
		if (obProfileName==(*obIt)->GetID())
		{
			m_obPreviousProfile=m_obCurrentProfile;
			m_obCurrentProfile=obProfileName;

			// Iterate through mixer targets for this profile and apply them to our categories
			(*obIt)->DoTransition(fTransitionTime);

			return;
		}
	}

#else

	UNUSED(obProfileName);
	UNUSED(fTransitionTime);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioMixer::SetPreviousProfile (float fTransitionTime)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	for(ntstd::List<AudioMixerProfile*>::iterator obIt=m_obProfileList.begin(); obIt!=m_obProfileList.end(); ++obIt)
	{
		if (m_obPreviousProfile==(*obIt)->GetID())
		{
			CHashedString obCurrentProfile(m_obCurrentProfile);
			m_obCurrentProfile=m_obPreviousProfile;
			m_obPreviousProfile=obCurrentProfile;

			// Iterate through mixer targets for this profile and apply them to our categories
			(*obIt)->DoTransition(fTransitionTime);

			return;
		}
	}

#else

	UNUSED(fTransitionTime);
	UNUSED(bStopSoundOnTransition);

#endif // _AUDIO_SYSTEM_ENABLE
}


void AudioMixer::SetProfileMute (const CHashedString& obProfileName,bool bMute)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	for(ntstd::List<AudioMixerProfile*>::iterator obIt=m_obProfileList.begin(); obIt!=m_obProfileList.end(); ++obIt)
	{
		if (obProfileName==(*obIt)->GetID())
		{
			(*obIt)->Mute(bMute);
			return;
		}
	}

#else

	UNUSED(obProfileName);
	UNUSED(bMute);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioMixer::SetProfilePause (const CHashedString& obProfileName,bool bPause)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	for(ntstd::List<AudioMixerProfile*>::iterator obIt=m_obProfileList.begin(); obIt!=m_obProfileList.end(); ++obIt)
	{
		if (obProfileName==(*obIt)->GetID())
		{
			(*obIt)->Pause(bPause);
			return;
		}
	}

#else

	UNUSED(obProfileName);
	UNUSED(bPause);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioMixer::SetCategoryMute (const CHashedString& obCategoryName,bool bMute)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	for(ntstd::List<AudioCategory*>::iterator obIt=m_obCategoryList.begin(); obIt!=m_obCategoryList.end(); ++obIt)
	{
		if (obCategoryName==(*obIt)->GetID())
		{
			(*obIt)->Mute(bMute);
			return;
		}
	}

#else

	UNUSED(obCategoryName);
	UNUSED(bMute);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioMixer::SetCategoryPause (const CHashedString& obCategoryName,bool bPause)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	for(ntstd::List<AudioCategory*>::iterator obIt=m_obCategoryList.begin(); obIt!=m_obCategoryList.end(); ++obIt)
	{
		if (obCategoryName==(*obIt)->GetID())
		{
			(*obIt)->Pause(bPause);
			return;
		}
	}

#else

	UNUSED(obCategoryName);
	UNUSED(bPause);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioMixer::SetCategoryVolume (const CHashedString& obCategoryName,float fVolume)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	for(ntstd::List<AudioCategory*>::iterator obIt=m_obCategoryList.begin(); obIt!=m_obCategoryList.end(); ++obIt)
	{
		if (obCategoryName==(*obIt)->GetID())
		{
			(*obIt)->SetVolume(fVolume);
			return;
		}
	}

#else

	UNUSED(obCategoryName);
	UNUSED(fVolume);

#endif // _AUDIO_SYSTEM_ENABLE
}

float AudioMixer::GetCategoryVolume (const CHashedString& obCategoryName)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	for(ntstd::List<AudioCategory*>::iterator obIt=m_obCategoryList.begin(); obIt!=m_obCategoryList.end(); ++obIt)
	{
		if (obCategoryName==(*obIt)->GetID())
		{
			return (*obIt)->GetVolume();
		}
	}

#else

	UNUSED(obCategoryName);

#endif // _AUDIO_SYSTEM_ENABLE

	return 0.0f;
}

void AudioMixer::PauseAll (bool bPause)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	for(ntstd::List<AudioCategory*>::iterator obIt=m_obCategoryList.begin(); obIt!=m_obCategoryList.end(); ++obIt)
	{
		(*obIt)->Pause(bPause);
	}

#else

	UNUSED(bPause);

#endif // _AUDIO_SYSTEM_ENABLE
}

bool AudioMixer::IsMuted (const CHashedString& obCategoryName)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	for(ntstd::List<AudioCategory*>::iterator obIt=m_obCategoryList.begin(); obIt!=m_obCategoryList.end(); ++obIt)
	{
		if (obCategoryName==(*obIt)->GetID())
		{
			return (*obIt)->IsMuted();
		}
	}

	return false;

#else

	UNUSED(obCategoryName);

	return false;

#endif // _AUDIO_SYSTEM_ENABLE
}

bool AudioMixer::IsPaused (const CHashedString& obCategoryName)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	for(ntstd::List<AudioCategory*>::iterator obIt=m_obCategoryList.begin(); obIt!=m_obCategoryList.end(); ++obIt)
	{
		if (obCategoryName==(*obIt)->GetID())
		{
			return (*obIt)->IsPaused();
		}
	}

	return false;

#else

	UNUSED(obCategoryName);

	return false;

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioMixer::AddProfile (AudioMixerProfile* pobProfile)
{
	for(ntstd::List<AudioMixerProfile*>::iterator obIt=m_obProfileList.begin(); obIt!=m_obProfileList.end(); ++obIt)
	{
		if (pobProfile==(*obIt)) // This profile is already in the profile list for some reason...
			return;
	}

	m_obProfileList.push_back(pobProfile);
}

void AudioMixer::RemoveProfile (AudioMixerProfile* pobProfile)
{
	m_obProfileList.remove(pobProfile);
}

AudioCategory* AudioMixer::GetCategory (const CHashedString& obCategoryName)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	for(ntstd::List<AudioCategory*>::iterator obIt=m_obCategoryList.begin(); obIt!=m_obCategoryList.end(); ++obIt)
	{
		if (obCategoryName==(*obIt)->GetID())
		{
			return (*obIt);
		}
	}

#else

	UNUSED(obCategoryName);

#endif // _AUDIO_SYSTEM_ENABLE
	
	return 0;
}


