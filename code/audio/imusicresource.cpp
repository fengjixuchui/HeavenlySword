#include "audio/imusicresource.h"
#include "audio/imusic.h"
#include "audio/audiosystem.h"

#include "objectdatabase/dataobject.h"

#include "fmod.hpp"
#include "fmod_event.h"
#include "fmod_errors.h"




START_STD_INTERFACE							( SerialisedString )
	PUBLISH_VAR_AS							( m_obString,					String )
END_STD_INTERFACE

START_STD_INTERFACE							( InteractiveMusicTransition )
	PUBLISH_VAR_AS							( m_fFromIntensity,				FromIntensity )
	PUBLISH_VAR_AS							( m_fToIntensity,				ToIntensity )
	PUBLISH_VAR_AS							( m_fSourceEnd1,				SourceEnd1 )
	PUBLISH_VAR_AS							( m_fSourceEnd2,				SourceEnd2 )
	PUBLISH_VAR_AS							( m_fIntermediateStart1,		IntermediateStart1 )
	PUBLISH_VAR_AS							( m_fIntermediateStart2,		IntermediateStart2 )
	PUBLISH_VAR_AS							( m_fIntermediateEnd1,			IntermediateEnd1 )
	PUBLISH_VAR_AS							( m_fIntermediateEnd2,			IntermediateEnd2 )
	PUBLISH_VAR_AS							( m_fDestinationStart1,			DestinationStart1 )
	PUBLISH_VAR_AS							( m_fDestinationStart2,			DestinationStart2 )
	PUBLISH_VAR_AS							( m_obIntermediateSound,		IntermediateSound )
	PUBLISH_VAR_AS							( m_fIntermediateVolume,		IntermediateVolume )
	PUBLISH_PTR_CONTAINER_AS				( m_obTransitionExemptSamples,	TransitionExemptSamples )
	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE

START_STD_INTERFACE							( InteractiveMusicSample )
	PUBLISH_PTR_CONTAINER_AS				( m_obWaveFileList,				WaveFileList )
	_IENUM									( SelectionOrder,				SELECTION_ORDER )
	PUBLISH_VAR_AS							( m_iPlayCount,					PlayCount )
	PUBLISH_VAR_AS							( m_fVolume,					Volume )
	PUBLISH_VAR_AS							( m_fStartTime,					StartTime )
	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE

START_STD_INTERFACE							( InteractiveMusicMarker )
	PUBLISH_VAR_AS							( m_fTimeOffset,				TimeOffset )
	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE

START_STD_INTERFACE							( InteractiveMusicState )
	PUBLISH_VAR_AS							( m_fIntensity,					Intensity )
	PUBLISH_VAR_AS							( m_fBeatInterval,				BeatInterval )
	PUBLISH_VAR_AS							( m_fVolume,					Volume )
	PUBLISH_PTR_CONTAINER_AS				( m_obSampleList,				SampleList )
	PUBLISH_PTR_CONTAINER_AS				( m_obMarkerList,				MarkerList )
	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE







// ---------------------------------------------------- InteractiveMusicSample ----------------------------------------------------


InteractiveMusicSample::InteractiveMusicSample () :
	m_eSelectionOrder(MUSIC_RANDOM_NOREPEAT),
	m_iPlayCount(0),
	m_fVolume(1.0f),	
	m_fStartTime(0.0f)
{
}

InteractiveMusicSample::~InteractiveMusicSample ()
{
}

void InteractiveMusicSample::PostConstruct ()
{
	// Get the name of this object from the serialised interface
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );

	if (pDO)
	{
		m_obName = CHashedString(pDO->GetName());
	}

}

bool InteractiveMusicSample::EditorChangeValue (CallBackParameter /*pcItem*/,CallBackParameter /*pcValue*/)
{
#ifndef _RELEASE

	PostConstruct();

	/*
	// Validate the list to make sure it has the correct object types in it
	for(ntstd::List<SerialisedString*>::iterator obIt=m_obWaveFileList.begin(); obIt!=m_obWaveFileList.end(); ++obIt)
	{
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(*obIt);

		ntPrintf("-> %s\n",pDO->GetClassName());

		if (strcmp(pDO->GetClassName(),"SerialisedString")!=0)
		{
			user_warn_p(false, ("Warning: InteractiveMusicSample has an object in it's WaveFileList that is not a SerialisedString!"));
		}
	}
	*/

#endif // _RELEASE

	return true;
}

const char* InteractiveMusicSample::GetSoundFile (int iIndex)
{
	// Get the string of a particular audio file in the pool
	int i=0;

	for(ntstd::List<SerialisedString*>::iterator obIt=m_obWaveFileList.begin(); obIt!=m_obWaveFileList.end(); ++obIt)
	{
		if (i==iIndex)
		{
			return (*obIt)->m_obString.GetString();
		}

		++i;
	}

	return 0;
}

// ---------------------------------------------------- InteractiveMusicMarker ----------------------------------------------------

InteractiveMusicMarker::InteractiveMusicMarker () :
	m_fTimeOffset(0.0f)
{
}

void InteractiveMusicMarker::PostConstruct ()
{
	if (m_fTimeOffset<0.0f) // Ensure the time offset is valid
		m_fTimeOffset=0.0f;
}

bool InteractiveMusicMarker::EditorChangeValue (CallBackParameter /*pcItem*/,CallBackParameter /*pcValue*/)
{
#ifndef _RELEASE

	PostConstruct();

#endif // _RELEASE

	return true;
}

// ---------------------------------------------------- InteractiveMusicState ----------------------------------------------------

InteractiveMusicState::InteractiveMusicState () :
	m_fIntensity(0.0f),	
	m_fBeatInterval(0.0f),	
	m_fVolume(1.0f)
{
}

InteractiveMusicState::~InteractiveMusicState ()
{
	if (InteractiveMusicManager::Exists())
		InteractiveMusicManager::Get().UnregisterMusicState(this);
}

void InteractiveMusicState::PostConstruct ()
{
	// Get the name of this object from the serialised interface
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );

	if (pDO)
	{
		m_obName = CHashedString(pDO->GetName());
	}

	InteractiveMusicManager::Get().RegisterMusicState(this);
}

bool InteractiveMusicState::EditorChangeValue (CallBackParameter /*pcItem*/,CallBackParameter /*pcValue*/)
{
	return true;
}




// ---------------------------------------------------- InteractiveMusicTransition ----------------------------------------------------

InteractiveMusicTransition::InteractiveMusicTransition () :
	m_fFromIntensity(0.0f),
	m_fToIntensity(0.0f),
	m_fSourceEnd1(0.0f),
	m_fSourceEnd2(0.0f),
	m_fIntermediateStart1(0.0f),
	m_fIntermediateStart2(0.0f),
	m_fIntermediateEnd1(0.0f),
	m_fIntermediateEnd2(0.0f),
	m_fDestinationStart1(0.0f),
	m_fDestinationStart2(0.0f),
	m_fIntermediateVolume(1.0f)
{
}

InteractiveMusicTransition::~InteractiveMusicTransition ()
{
	if (InteractiveMusicManager::Exists())
		InteractiveMusicManager::Get().UnregisterMusicTransition(this);
}

void InteractiveMusicTransition::PostConstruct ()
{
	// Get the name of this object from the serialised interface
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );

	if (pDO)
	{
		m_obName = CHashedString(pDO->GetName());

		//ntPrintf("%s\n",m_obName.GetDebugString());
	}

	InteractiveMusicManager::Get().RegisterMusicTransition(this);
}

bool InteractiveMusicTransition::EditorChangeValue (CallBackParameter /*pcItem*/,CallBackParameter /*pcValue*/)
{
#ifndef _RELEASE

	// Ensure all values are valid

	if (m_fFromIntensity<0.0f) m_fFromIntensity=0.0f;
	if (m_fFromIntensity>1.0f) m_fFromIntensity=1.0f;

	if (m_fToIntensity<0.0f) m_fToIntensity=0.0f;
	if (m_fToIntensity>1.0f) m_fToIntensity=1.0f;

	if (m_fSourceEnd1<0.0f) m_fSourceEnd1=0.0f;
	if (m_fSourceEnd2<0.0f) m_fSourceEnd2=0.0f;
	if (m_fSourceEnd1>m_fSourceEnd2) m_fSourceEnd1=m_fSourceEnd2;

	if (m_fIntermediateStart1<0.0f) m_fIntermediateStart1=0.0f;
	if (m_fIntermediateStart2<0.0f) m_fIntermediateStart2=0.0f;
	if (m_fIntermediateStart1>m_fIntermediateStart2) m_fIntermediateStart1=m_fIntermediateStart2;
	
	if (m_fIntermediateEnd1<0.0f) m_fIntermediateEnd1=0.0f;
	if (m_fIntermediateEnd2<0.0f) m_fIntermediateEnd2=0.0f;
	if (m_fIntermediateEnd1>m_fIntermediateEnd2) m_fIntermediateEnd1=m_fIntermediateEnd2;

	if (m_fDestinationStart1<0.0f) m_fDestinationStart1=0.0f;
	if (m_fDestinationStart2<0.0f) m_fDestinationStart2=0.0f;
	if (m_fDestinationStart1>m_fDestinationStart2) m_fDestinationStart1=m_fDestinationStart2;

#endif // _RELEASE

	return true;
}

const CHashedString& InteractiveMusicTransition::GetName ()
{
	return m_obName;
}

const char* InteractiveMusicTransition::GetIntermediateSoundFile ()
{
	if (m_obIntermediateSound.IsNull())
		return 0;

	return m_obIntermediateSound.GetString();
}

bool InteractiveMusicTransition::IsExempt (unsigned int uiSampleID)
{
	// This is used to check if the given InteractiveMusicSample should be ignored by this transition, but cross checking it with samples listed in its exemption list
	for(ntstd::List<InteractiveMusicSample*>::iterator obIt=m_obTransitionExemptSamples.begin(); obIt!=m_obTransitionExemptSamples.end(); ++obIt)
	{
		if (uiSampleID==(*obIt)->GetName().GetHash())
			return true;
	}

	return false;
}

