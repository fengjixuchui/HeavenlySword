

#include "audio/interactivemusic.h"
#include "audio/soundutil.h"
#include "editable/enumlist.h"
#include "core/visualdebugger.h"
#include "game/randmanager.h"
#include "objectdatabase/dataobject.h"
#include "input/inputhardware.h" // Debugging
#include "core/timer.h"

START_STD_INTERFACE (CMusicSegment)
	IENUM		(CMusicSegment,	SelectionMethod, SELECTION_METHOD)
	IINT		(CMusicSegment,	FirstSample)
	IFLOAT		(CMusicSegment,	Duration)
	PUBLISH_PTR_CONTAINER_AS		(m_obIntroSampleList, IntroSampleList)
	PUBLISH_PTR_CONTAINER_AS		(m_obMainSampleList,	MainSampleList)
	PUBLISH_PTR_CONTAINER_AS		(m_obOutroSampleList,	OutroSampleList)
END_STD_INTERFACE

START_STD_INTERFACE	(CMusicSample)
	ISTRING		(CMusicSample, WaveFile)
	ISTRING		(CMusicSample, WaveBank)
	IINT		(CMusicSample, PlayCount)
	IINT		(CMusicSample, Weight)
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

//CRITICAL_SECTION gobCriticalSection;

#ifndef _RELEASE

#define _DISPLAY_ERRORS
#define INTERACTIVEMUSIC_PRINTF

#endif // _RELEASE

//---------------------------------------------------------------------------------------------------------------------------------------------------

/***************************************************************************************************
*
*	FUNCTION		CMusicSample::CMusicSample
*
*	DESCRIPTION		
*
***************************************************************************************************/

CMusicSample::CMusicSample () :
	m_iPlayCount(1),
	m_iWeight(100),
	m_pobWaveFile(0)
{
}

/***************************************************************************************************
*
*	FUNCTION		CMusicSample::~CMusicSample
*
*	DESCRIPTION		
*
***************************************************************************************************/

CMusicSample::~CMusicSample ()
{
}

/***************************************************************************************************
*
*	FUNCTION		CMusicSample::~CMusicSample
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CMusicSample::EditorChangeValue (CallBackParameter, CallBackParameter)
{
#ifndef _RELEASE

	m_pobWaveFile=CAudioResourceManager::Get().FindWaveFileP(m_obWaveBank,m_obWaveFile);

	if (m_pobWaveFile==0)
	{
		one_time_assert_p( 1, 0, ("CMusicSample: Wave file %s (%s) is not found", ntStr::GetString(m_obWaveFile), ntStr::GetString(m_obWaveBank)) )
	}

#endif // _RELEASE

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CMusicSample::Initialise
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CMusicSample::Initialise ()
{
	// Set the pointer to the wave file if its not already set

	if (!m_pobWaveFile)
	{
		m_pobWaveFile=CAudioResourceManager::Get().FindWaveFileP(m_obWaveBank,m_obWaveFile);

		if (!m_pobWaveFile)
			ntPrintf("CMusicSample %s does not have a valid wavefile (%s:%s)\n",ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )), ntStr::GetString(m_obWaveBank), ntStr::GetString(m_obWaveFile));
	}
}



//---------------------------------------------------------------------------------------------------------------------------------------------------




/***************************************************************************************************
*
*	FUNCTION		CMusicSegment::CMusicSegment
*
*	DESCRIPTION		
*
***************************************************************************************************/

CMusicSegment::CMusicSegment () :
	m_eSelectionMethod(SEQUENTIAL),
	m_iFirstSample(0),
	m_fDuration(0.0f),
	m_pobCurrentSample(0)
{
}

/***************************************************************************************************
*
*	FUNCTION		CMusicSegment::~CMusicSegment
*
*	DESCRIPTION		
*
***************************************************************************************************/

CMusicSegment::~CMusicSegment ()
{
}

/***************************************************************************************************
*
*	FUNCTION		CMusicSegment::Initialise
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CMusicSegment::Initialise ()
{
//	ntPrintf("CMusicSegment: Initialising %s (selection=%d  intro=%d  main=%d  outro=%d)\n",
//		*m_obName,
//		m_eSelectionMethod,
//		m_obIntroSampleList.size(),
//		m_obMainSampleList.size(),
//		m_obOutroSampleList.size());		

	m_eSegmentStage=STAGE_INACTIVE;

	m_pobCurrentSample=NULL;

	// Make sure samples have pointers to their wavefiles

	ntstd::List<CMusicSample*>::iterator obSampleIt;

	for(obSampleIt=m_obIntroSampleList.begin(); obSampleIt!=m_obIntroSampleList.end(); ++obSampleIt)
	{
		(*obSampleIt)->Initialise();
	}

	for(obSampleIt=m_obMainSampleList.begin(); obSampleIt!=m_obMainSampleList.end(); ++obSampleIt)
	{
		(*obSampleIt)->Initialise();
	}

	for(obSampleIt=m_obOutroSampleList.begin(); obSampleIt!=m_obOutroSampleList.end(); ++obSampleIt)
	{
		(*obSampleIt)->Initialise();
	}

	// Shuffle our music sample list

	const int iSampleCount=m_obMainSampleList.size();

	if (m_eSelectionMethod==SHUFFLED && iSampleCount>0)
	{
		CMusicSample* apobSampleList [iMAIN_SAMPLE_POOL_SIZE];
		
		int iCount=0;

		for(ntstd::List<CMusicSample*>::iterator obSampleIt=m_obMainSampleList.begin();
			obSampleIt!=m_obMainSampleList.end();
			++obSampleIt)
		{
			apobSampleList[iCount]=*obSampleIt;
			iCount++;
		}

		int iCurrent=0;

		while(iCurrent<iSampleCount)
		{
			int iIndex=drand() % iSampleCount;

			if (apobSampleList[iIndex])
			{
				m_apobShuffledSamples[iCurrent]=apobSampleList[iIndex];
				apobSampleList[iIndex]=NULL;
				++iCurrent;
			}
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CMusicSegment::SetStage
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CMusicSegment::SetStage (SEGMENT_STAGE eStage)
{
	m_eSegmentStage=eStage;

	// When entering a new stage, we either move to the next if there are no samples OR
	// we set the initial sample for that stage (it's index and it's playcount).

	// ----- INTRO STAGE -----

	if (m_eSegmentStage==STAGE_INTRO)
	{
		if (m_obIntroSampleList.size()==0) // There is no intro, jump to main
		{
			m_eSegmentStage=STAGE_MAIN;
		}
		else // Set the initial sample and sample count
		{
			SetRandomSample(&m_obIntroSampleList);
			m_iCurrentSamplePlayCount=1;
		}
	}

	// ----- MAIN STAGE -----

	if (m_eSegmentStage==STAGE_MAIN)
	{
		if (m_obMainSampleList.size()==0) // No samples in the main part
		{
			m_eSegmentStage=STAGE_OUTRO; // Change to outro
		}
		else // Set the initial sample and sample count
		{
			m_fPlayTime=0.0f;

			switch(m_eSelectionMethod)
			{
				case MANUAL:
				case SEQUENTIAL:
				{
					m_iCurrentSample=m_iFirstSample;
					CMusicSample* pobSample=GetSampleInList(&m_obMainSampleList,m_iCurrentSample);
					m_iCurrentSamplePlayCount=pobSample->m_iPlayCount;
					break;
				}

				case RANDOM:
				{
					SetRandomSample(&m_obMainSampleList);

					break;
				}

				case RANDOM_NOWEIGHTS:
				{
					m_iCurrentSample=(drand() % (m_obMainSampleList.size() * 10))/10;
					CMusicSample* pobSample=GetSampleInList(&m_obMainSampleList,m_iCurrentSample);
					m_iCurrentSamplePlayCount=pobSample->m_iPlayCount;
					break;
				}

				case SHUFFLED:
				{
					m_iCurrentSample=0;
					m_iCurrentSamplePlayCount=m_apobShuffledSamples[0]->m_iPlayCount;
					break;
				}
			}
		}
	}

	// ----- OUTRO STAGE -----

	if (m_eSegmentStage==STAGE_OUTRO)
	{
		if (m_obOutroSampleList.size()==0)
		{
			m_eSegmentStage=STAGE_INACTIVE;
		}
		else
		{
			SetRandomSample(&m_obOutroSampleList);
			m_iCurrentSamplePlayCount=1;
		}
	}

	/*
	switch(m_eSegmentStage)
	{
		case STAGE_INACTIVE:
		{
			ntPrintf("Setting stage to inactive for segment %s\n",GetNameC());
			break;
		}
		case STAGE_INTRO:
		{
			ntPrintf("Setting stage to intro for segment %s\n",GetNameC());
			break;
		}
		case STAGE_MAIN:
		{
			ntPrintf("Setting stage to main for segment %s\n",GetNameC());
			break;
		}
		case STAGE_OUTRO:
		{
			ntPrintf("Setting stage to outro for segment %s\n",GetNameC());
			break;
		}
	}
	*/
}

/***************************************************************************************************
*
*	FUNCTION		CMusicSegment::GetNameP
*
*	DESCRIPTION		Get actual text label (for debugging purposes)
*
***************************************************************************************************/
const char* CMusicSegment::GetNameP() const
{ 
	return ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )); 
}


/***************************************************************************************************
*
*	FUNCTION		CMusicSegment::GetNextWaveFileP
*
*	DESCRIPTION		
*
***************************************************************************************************/

CWaveFile* CMusicSegment::GetNextWaveFileP ()
{
	// Check to see if the segment is finished
	
	if (m_eSegmentStage==STAGE_INACTIVE) 
		return NULL;

	// Check to see if we are in the main stage and playing for a limited duration

	if (m_eSegmentStage==STAGE_MAIN && m_fDuration>0.0f && m_fPlayTime>m_fDuration)
	{
		SetStage(STAGE_OUTRO); // Move to the outro stage
	}

	// Check to see if we have finished playing the current sample
	
	if (m_iCurrentSamplePlayCount==0)
	{
		switch(m_eSegmentStage)
		{
			case STAGE_INTRO:
			{
				SetStage(STAGE_MAIN); // Intro has finished playing, move straight to main
				break;
			}

			case STAGE_MAIN:
			{
				// Set the current sample

				switch(m_eSelectionMethod)
				{
					//case MANUAL:
					case SEQUENTIAL:
					case SHUFFLED:
					{
						m_iCurrentSample++;
					
						if (m_iCurrentSample>=(int)m_obMainSampleList.size())
							m_iCurrentSample=0;

						// Set the play count

						if (m_eSelectionMethod==SEQUENTIAL)
						{
							CMusicSample* pobSample=GetSampleInList(&m_obMainSampleList,m_iCurrentSample);
							m_iCurrentSamplePlayCount=pobSample->m_iPlayCount;
						}
						else
						{
							m_iCurrentSamplePlayCount=m_apobShuffledSamples[m_iCurrentSample]->m_iPlayCount;
						}

						break;
					}

					case RANDOM:
					{
						SetRandomSample(&m_obMainSampleList);

						break;
					}

					default:
						break;
				}

				break;
			}

			case STAGE_OUTRO:
			{
					SetStage(STAGE_INACTIVE); // We have finished playing the outro
				break;
			}

			default:
				break;
		}
	}

	// Check to see if the segment is finished
	
	if (m_eSegmentStage==STAGE_INACTIVE) 
		return NULL;

	// Determine the current wavefile

	CWaveFile* pobWaveFile=NULL;

	switch(m_eSegmentStage)
	{
		case STAGE_INTRO:
		{
			m_pobCurrentSample=GetSampleInList(&m_obIntroSampleList,m_iCurrentSample);
			pobWaveFile=m_pobCurrentSample->m_pobWaveFile;

			m_iCurrentSamplePlayCount--;

			break;
		}

		case STAGE_MAIN:
		{
			if (m_eSelectionMethod==SHUFFLED) // We are drawing from the shuffled list
			{
				m_pobCurrentSample=m_apobShuffledSamples[m_iCurrentSample];
				pobWaveFile=m_pobCurrentSample->m_pobWaveFile;
			}
			else // We are drawing from the pool
			{
				m_pobCurrentSample=GetSampleInList(&m_obMainSampleList,m_iCurrentSample);
				pobWaveFile=m_pobCurrentSample->m_pobWaveFile;
			}

			m_fPlayTime+=(float)pobWaveFile->GetDuration()/1000.0f;

			m_iCurrentSamplePlayCount--;

			break;
		}

		case STAGE_OUTRO:
		{
			m_pobCurrentSample=GetSampleInList(&m_obOutroSampleList,m_iCurrentSample);
			pobWaveFile=m_pobCurrentSample->m_pobWaveFile;
			m_iCurrentSamplePlayCount--;
			break;
		}

		default:
			break;
	}

	return pobWaveFile;
}

/***************************************************************************************************
*
*	FUNCTION		CMusicSegment::IncreaseLevel
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CMusicSegment::IncreaseLevel ()
{
	if (m_eSelectionMethod==MANUAL && m_iCurrentSample<((int)m_obMainSampleList.size()-1))
	{
		m_iCurrentSample++;

		CMusicSample* pobSample=GetSampleInList(&m_obMainSampleList,m_iCurrentSample);
		m_iCurrentSamplePlayCount=pobSample->m_iPlayCount;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CMusicSegment::DecreaseLevel
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CMusicSegment::DecreaseLevel ()
{
	if (m_eSelectionMethod==MANUAL && m_iCurrentSample>0)
	{
		m_iCurrentSample--;

		CMusicSample* pobSample=GetSampleInList(&m_obMainSampleList,m_iCurrentSample);
		m_iCurrentSamplePlayCount=pobSample->m_iPlayCount;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CMusicSegment::GetSampleInList
*
*	DESCRIPTION		
*
***************************************************************************************************/

CMusicSample* CMusicSegment::GetSampleInList (ntstd::List<CMusicSample*> *pobList,int iIndex)
{
	if (pobList->size()==1)
		return pobList->front();
	
	int iCount=0;

	for(ntstd::List<CMusicSample*>::iterator obSampleIt=pobList->begin();
		obSampleIt!=pobList->end();
		++obSampleIt)
	{
		if (iIndex==iCount)
			return *obSampleIt;

		iCount++;
	}

	ntAssert(0);

	return NULL; // Not found
}

/***************************************************************************************************
*
*	FUNCTION		CMusicSegment::SetRandomSample
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CMusicSegment::SetRandomSample (ntstd::List<CMusicSample*> *pobList)
{
	ntAssert(pobList->size()>0); // The list must have more than one item
	
	if (pobList->size()==1)
	{
		m_iCurrentSample=0;
		m_iCurrentSamplePlayCount=pobList->front()->m_iPlayCount;
		return;
	}

	const int iRand=(int)((drand() % 100)+1);

	//ntPrintf("rand=%d\n",iRand);

	int iIndex=0;
	int iCurrent=0;

	for(ntstd::List<CMusicSample*>::iterator obSampleIt=pobList->begin();
		obSampleIt!=pobList->end();
		++obSampleIt)
	{
		const int iWeight=(*obSampleIt)->m_iWeight;

		//ntPrintf("\tcomparing %d against %d to %d\n",iWeight,iCurrent,iCurrent+iWeight);

		if (iWeight>0 && iRand>=iCurrent && iRand<=iCurrent+iWeight)
		{
			m_iCurrentSample=iIndex; // Set the sample index
			m_iCurrentSamplePlayCount=(*obSampleIt)->m_iPlayCount; // Set the sample play count
			return;
		}

		iCurrent+=iWeight;
		iIndex++;
	}

	ntAssert(0); // We should never reach this!
}






//---------------------------------------------------------------------------------------------------------------------------------------------------







/***************************************************************************************************
*
*	FUNCTION		CMusicStream::Initialise
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CMusicStream::Initialise (CWaveFile* pobWaveFile)
{
	m_ulDataRead=0;

	if (pobWaveFile)
	{
		m_ulEndOffset=pobWaveFile->GetDataSize();
		m_uiTotalMarkers=pobWaveFile->GetMarkerCount();
	}
	else
	{
		m_ulEndOffset=0;
		m_uiTotalMarkers=0;
	}

	m_pobWaveFile=pobWaveFile; // Make sure we set this last
}







//---------------------------------------------------------------------------------------------------------------------------------------------------






CMusicTrack::CMusicTrack (const char* pcName) :
	m_pobSoundBuffer(0),
	m_obName(pcName)
{
}

CMusicTrack::~CMusicTrack ()
{
	NT_DELETE( m_pobSoundBuffer );
}

//bool CMusicTrack::Initialise (IDirectSound8* pobDirectSound,WAVEFORMATEX* pstFormat,u_long ulBufferSize)
//{
//	ntAssert(pobDirectSound); // Check for valid DirectSound interface
//	ntAssert(pstFormat); // Check for valid wave format
//	ntAssert(!m_pobSoundBuffer); // Make sure initialise hasn't already been called
//
//	// Create the buffer
//
//	m_pobSoundBuffer=NT_NEW CSoundBuffer ();
//
//	ntAssert(m_pobSoundBuffer);
//
//	if (!m_pobSoundBuffer->Initialise(pobDirectSound,pstFormat,ulBufferSize))
//	{
//		ntPrintf("CMusicTrack: Failed to initialise sound stream\n");
//		return false;
//	}
//
//	// Initialise members
//
//	m_fBaseVolume=1.0f;
//	m_fBasePitch=1.0f;
//
//	m_iCurrentStream=0;
//	m_aobStream[0].Initialise(NULL);
//	m_aobStream[1].Initialise(NULL);
//	m_pobCurrentSegment=NULL;
//	m_pobNextSegment=NULL;
//
//	m_ulPacketSize=ulBufferSize >> 1;
//	m_ucSilence=(pstFormat->wBitsPerSample==8 ? 128 : 0);
//	m_iNextPacket=0;
//	m_ulPlayCursorPosition=0;
//	m_ulLastPlayCursorPosition=0;
//	m_ulPosition=0;
//	m_ulNextMarkerOffset=0;
//	m_ulChangeEndOffset=0;
//
//	m_fFadeVolume=1.0f;
//	m_fFadeDecrement=0.0f;
//
//	m_fTimeRemaining=0.0f;
//
//	m_fStopTime=0.0f;
//	m_bStopTrack=false;
//
//	m_bForceStreamChange=false;
//
//	m_bRestoreFadeVolume=false;
//
//	m_eNotification=NOTIFICATION_UNUSED;
//	m_ulNotificationOffset=0;
//
//	m_bResetPosition=false;
//	
//	m_bRestart=false;
//
//	ntPrintf("CMusicTrack: Initialised %s (%d bytes)\n",*m_obName,ulBufferSize);
//
//	return true;
//}

void CMusicTrack::SetSegment (CMusicSegment* pobSegment,SEGMENT_STAGE eStage,TRACK_STOP eStopOption)
{
	ntAssert(pobSegment);

	if (!m_pobCurrentSegment) // There is current no segment assigned to this track
	{
		m_pobCurrentSegment=pobSegment;
	}
	else // There is already a segment assigned to this track, so queue this one and stop the current
	{
		m_pobNextSegment=pobSegment;

		m_bStopTrack=true;
		m_eTrackStop=eStopOption;
	}

	pobSegment->SetStage(eStage); // Set the initial stage on the segment
}

void CMusicTrack::Stop (TRACK_STOP eStopOption,float fDuration)
{
	//ntPrintf("CMusicTrack: Requesting stop\n");

	if (m_pobCurrentSegment || m_pobNextSegment) // Ignore stop request if there is no segment
	{
		m_bStopTrack=true;

		m_eTrackStop=eStopOption;

		if (m_eTrackStop==STOP_FADE)
		{
			ntAssert(fDuration!=0.0f); // Duration cannot be 0!

			m_fFadeDecrement=1.0f/(fDuration*60.0f);
		}
		else if (m_eTrackStop==STOP_TIMED)
		{
			m_fStopTime=fDuration;
		}
	}
	else
	{
		if (eStopOption==STOP_IMMEDIATE)
		{
//			EnterCriticalSection(&gobCriticalSection);

			CMusicStream* pobCurrentStream=&m_aobStream[m_iCurrentStream];
			CMusicStream* pobNextStream=&m_aobStream[!m_iCurrentStream];

			if (pobCurrentStream->m_pobWaveFile || pobNextStream->m_pobWaveFile)
			{
				m_bForceStreamChange=true;

				if (pobCurrentStream->m_pobWaveFile==NULL) // Set the current stream
				{
					pobCurrentStream->Initialise(GetNextWaveP());
				}

				if (pobNextStream->m_pobWaveFile==NULL) // Set the next wave file
				{
					pobNextStream->Initialise(GetNextWaveP());
				}
			}

//			LeaveCriticalSection(&gobCriticalSection);
		}
	}
}

CWaveFile* CMusicTrack::GetNextWaveP ()
{
	// Note: transition between segments is handled here

	CWaveFile* pobWaveFile=NULL;

	// Determine the next valid segment

	if (m_pobCurrentSegment) // Make sure there is current segment
	{
		pobWaveFile=m_pobCurrentSegment->GetNextWaveFileP();

		if (!pobWaveFile) // This segment is finished
		{
			if (m_bRestart)
			{
				m_pobCurrentSegment->Initialise();
				m_pobCurrentSegment->SetStage(STAGE_INTRO);
				pobWaveFile=m_pobCurrentSegment->GetNextWaveFileP();
				m_bRestart=false;
			}
			else
			{
				ntPrintf("CMusicTrack: Segment '%s' has finished on track '%s'\n",
					m_pobCurrentSegment->GetNameP(),GetNameP());

				m_pobCurrentSegment=m_pobNextSegment;

				m_pobNextSegment=NULL;
				
				if (m_pobCurrentSegment) // Find out if next segement has been specified
				{
					pobWaveFile=m_pobCurrentSegment->GetNextWaveFileP();

					ntAssert(pobWaveFile); // If we are entering a new segment for the first time, its safe to assume it should be able to provide a wavefile!
				}
			}
		}
	}

//	if (pobWaveFile)
//		ntPrintf("CMusicTrack: Getting wavefile\n");
//	else
//		ntPrintf("CMusicTrack: No wavefile\n");

	return pobWaveFile;
}

void CMusicTrack::MainUpdate ()
{
	// ----- Update volume/pitch -----

//	EnterCriticalSection(&gobCriticalSection);

	if (m_bRestoreFadeVolume)
	{
		m_fFadeVolume=1.0f;
		m_bRestoreFadeVolume=false;
	}

//	LeaveCriticalSection(&gobCriticalSection);

	const float fVolume=m_fBaseVolume*m_fFadeVolume*CInteractiveMusicManager::Get().GetVolume();
	
	const float fPitch=m_fBasePitch*CInteractiveMusicManager::Get().GetPitch();

	if (m_pobSoundBuffer->GetVolume()!=fVolume)
        m_pobSoundBuffer->SetVolume(fVolume);

	if (m_pobSoundBuffer->GetPitch()!=fPitch)
        m_pobSoundBuffer->SetPitch(fPitch);

	// Update segments - move to next segment if necessary

	if (!m_pobCurrentSegment) // There is no current segment, therefore there is nothing to update
		return;

	// ----- Handle Stop Request -----

	bool bChangeNextWaveFile=false;

	if (m_bStopTrack)
	{
		switch(m_eTrackStop)
		{
			case STOP_IMMEDIATE: // Force an immediate switch to the next segment
			{
				m_pobCurrentSegment->SetStage(STAGE_INACTIVE);
				ChangeAudioStream();
				bChangeNextWaveFile=true;
				m_bStopTrack=false;
				break;
			}

			case STOP_ONLOOP: // Switch to the next segment after the current stream has finished playing
			{
				m_pobCurrentSegment->SetStage(STAGE_INACTIVE);
				bChangeNextWaveFile=true;
				m_bStopTrack=false;
				break;
			}

			case STOP_ONMARKER: // Switch to the next segment on the next marker (or loop point) on the current stream
			{
				StopOnNextMarker();
				m_pobCurrentSegment->SetStage(STAGE_INACTIVE);
				bChangeNextWaveFile=true;
				m_bStopTrack=false;
				break;
			}

			case STOP_FINISH: // Force current segment to switch to outro stage
			{
				StopOnNextMarker();
				m_pobCurrentSegment->SetStage(STAGE_OUTRO);
				bChangeNextWaveFile=true;
				m_bStopTrack=false;
				break;
			}

			case STOP_FADE: // Fade out the current stream and switch to the next segment
			{
				float fNewVolume=m_fFadeVolume-m_fFadeDecrement;

				if (fNewVolume<=0.0f) // Check to see if the fade is complete
				{
					fNewVolume=0.0f;
					m_fFadeDecrement=0.0f;

					m_pobCurrentSegment->SetStage(STAGE_INACTIVE); // Current segment is now finished
					bChangeNextWaveFile=true; // Lets change to the next file
					m_bStopTrack=false; // Our stop request is complete

					ChangeAudioStream();
				}

				m_fFadeVolume=fNewVolume;
				
				break;
			}

			case STOP_TIMED:
			{
				m_fStopTime-=CTimer::Get().GetSystemTimeChange();

				if (m_fStopTime<=0.0f)
				{
					StopOnNextMarker();
					m_pobCurrentSegment->SetStage(STAGE_OUTRO);
					bChangeNextWaveFile=true;
					m_bStopTrack=false;
					m_fStopTime=0.0f;
				}
			}
		}
	}

	// Calculate our time remaining until next transition (returns 0 if there is no scheduled transition)
	
	m_fTimeRemaining=GetTimeRemainingToTransition();

	// Note, we only reinitialise a stream once it has been made invalid by the thread update

//	EnterCriticalSection(&gobCriticalSection);

	CMusicStream* pobCurrentStream=&m_aobStream[m_iCurrentStream];
	CMusicStream* pobNextStream=&m_aobStream[!m_iCurrentStream];

	if (pobCurrentStream->m_pobWaveFile==NULL) // Set the current stream
	{
		pobCurrentStream->Initialise(GetNextWaveP());
	}

	if (bChangeNextWaveFile || pobNextStream->m_pobWaveFile==NULL) // Set the next wave file
	{
		pobNextStream->Initialise(GetNextWaveP());
	}

//	LeaveCriticalSection(&gobCriticalSection);
}




void CMusicTrack::Begin ()
{
	ntAssert(m_pobSoundBuffer); // Make sure we have a sound buffer

	m_pobSoundBuffer->Play(true); // NOTE: The sound buffer ALWAYS loops for streams, but that doesn't necessarily mean the stream loops.

	ntPrintf("CMusicTrack: %s is now playing\n", ntStr::GetString(m_obName));
}


float CMusicTrack::GetPlayPosition () const // Get the play position (in seconds)
{
	u_long ulPosition;
	CWaveFile* pobWaveFile;

//	EnterCriticalSection(&gobCriticalSection);
	ulPosition=m_ulPosition;
	pobWaveFile=m_aobStream[m_iCurrentStream].m_pobWaveFile;
//	LeaveCriticalSection(&gobCriticalSection);

	if (pobWaveFile)
	{
		return ((float)ulPosition / (float)pobWaveFile->GetBytesPerSec());
	}

	return 0.f;
}

float CMusicTrack::GetDuration () const
{
	CWaveFile* pobWaveFile;

//	EnterCriticalSection(&gobCriticalSection);
	pobWaveFile=m_aobStream[m_iCurrentStream].m_pobWaveFile;
//	LeaveCriticalSection(&gobCriticalSection);

	if (pobWaveFile)
	{
		return ((float)pobWaveFile->GetDuration() / 1000.0f);
	}
	
	return 0.f;
}

float CMusicTrack::GetTimeRemainingToTransition () const
{
	//if (m_bStopTrack)
	{
		u_long ulPosition,ulEndOffset1,ulEndOffset2;
		CWaveFile* pobWaveFile1;
		CWaveFile* pobWaveFile2;

//		EnterCriticalSection(&gobCriticalSection);
		ulPosition=m_ulPosition;
		ulEndOffset1=m_aobStream[m_iCurrentStream].m_ulEndOffset;
		ulEndOffset2=m_aobStream[!m_iCurrentStream].m_ulEndOffset;
		pobWaveFile1=m_aobStream[m_iCurrentStream].m_pobWaveFile;
		pobWaveFile2=m_aobStream[!m_iCurrentStream].m_pobWaveFile;
//		LeaveCriticalSection(&gobCriticalSection);

		switch(m_eTrackStop)
		{
			case STOP_ONLOOP: // Switch to the next segment after the current stream has finished playing
			case STOP_ONMARKER: // Switch to the next segment on the next marker (or loop point) on the current stream
			case STOP_FINISH: // Force current segment to switch to outro stage
			{
				if (pobWaveFile1 && ulEndOffset1>0)
				{
					return ((float)(ulEndOffset1-ulPosition) / (float)pobWaveFile1->GetBytesPerSec());
				}

				break;
			}

			case STOP_FADE:
			{
				// Hmmmm....
				break;
			}
	
			case STOP_TIMED:
			{
				// Predict how much time remaining until this segment is completed

				const float fBytesPerSec=(float)pobWaveFile1->GetBytesPerSec();
				const float fTimeElapsed=(float)ulPosition / fBytesPerSec;
				const float fTimeRemaining=(float)(ulEndOffset1-ulPosition) / fBytesPerSec;
				const float fDuration=(float)(pobWaveFile1->GetDuration() / 1000);
				const float fEstimatedStopTime=fTimeElapsed+m_fStopTime;

				if (m_fStopTime==0.0f) // Stop on next marker has been called
				{
					return fTimeRemaining;
				}
				else if (fEstimatedStopTime<fDuration || pobWaveFile2==0) // Predict where the stop point will be in the CURRENT target wavefile
				{
					unsigned long ulEstimatedStopOffset=ulPosition + (unsigned long)(m_fStopTime*fBytesPerSec);

					u_long ulOffset=0;
					u_int uiIndex;		
				
					pobWaveFile1->GetNextMarker(ulEstimatedStopOffset,ulOffset,uiIndex);

					return (((float)ulOffset / fBytesPerSec)-fTimeElapsed);
				}
				else // Predict where the stop point will be in the NEXT target wavefile
				{
					float fOverTime=fEstimatedStopTime-fDuration;
					
					float fNewOffset=(float)pobWaveFile2->GetBytesPerSec() * fOverTime;

					u_long ulOffset=0;
					u_int uiIndex;
					
					pobWaveFile2->GetNextMarker ((u_long)fNewOffset,ulOffset,uiIndex);

					return (((float)ulOffset / (float)pobWaveFile2->GetBytesPerSec())+fTimeRemaining); // Time remaining in current wavefile + next marker in next wavefile
				}

				break;
			}

			default:
				break;
		}
	}

	return 0.f;
}

float CMusicTrack::GetNextMarkerPosition () const
{
	u_long ulPosition;
	CWaveFile* pobWaveFile;

//	EnterCriticalSection(&gobCriticalSection);
	ulPosition=m_ulNextMarkerOffset;
	pobWaveFile=m_aobStream[m_iCurrentStream].m_pobWaveFile;
//	LeaveCriticalSection(&gobCriticalSection);

	if (pobWaveFile)
	{
		return ((float)ulPosition / (float)pobWaveFile->GetBytesPerSec());
	}

	return 0.f;
}

// !!! Threaded function !!!
void CMusicTrack::ReadFromFile (void* pvDestination,u_long ulOffset)
{
	CMusicStream* pobCurrentStream=&m_aobStream[m_iCurrentStream];
	CMusicStream* pobNextStream=&m_aobStream[!m_iCurrentStream];

	// Check to see if both wave files are still valid, if not fill the buffer with silence
	
	if (pobCurrentStream->m_pobWaveFile==NULL && pobNextStream->m_pobWaveFile==NULL)
	{
		memset( pvDestination, m_ucSilence, m_ulPacketSize );

		return;
	}

	// Change endoffset if we are stopping stream on a marker
	
	u_long ulChangeOffset;

//	EnterCriticalSection(&gobCriticalSection);

	ulChangeOffset=m_ulChangeEndOffset;

	if (m_ulChangeEndOffset>0)
		m_ulChangeEndOffset=0;

//	LeaveCriticalSection(&gobCriticalSection);

	if (ulChangeOffset>0)
	{
//		const u_long ulDataRead=pobCurrentStream->m_ulDataRead;
		
//		u_long ulNextMarkerOffset;
//		u_int uiNextMarkerIndex;
			
//		pobCurrentStream->m_pobWaveFile->GetNextMarker(ulDataRead,ulNextMarkerOffset,uiNextMarkerIndex);

		pobCurrentStream->m_ulEndOffset=ulChangeOffset;
	}

	// Read wave data into the sound buffer

	bool bChangeStream=false;

	const u_long ulBytesRemaining=pobCurrentStream->m_ulEndOffset-pobCurrentStream->m_ulDataRead;

	if (ulBytesRemaining<m_ulPacketSize) // Amount remaining is less than the packetsize, therefore we have reached the end of the wave data
	{
		// Read the last remaining fragment into the buffer
		
		if (ulBytesRemaining>0)
		{
			pobCurrentStream->m_pobWaveFile->GetWaveData(pvDestination,pobCurrentStream->m_ulDataRead,ulBytesRemaining);
		}

		// In this bit, we need to decide whether the remainder of the buffer needs to be filled with silence (if we are finishing)
		// or fill it with a portion of the beginning of the wave data (if we are looping)

		if (pobNextStream->m_pobWaveFile) // A next wave file has been specified
		{
			pobNextStream->m_pobWaveFile->GetWaveData((uint8_t*)pvDestination+ulBytesRemaining,0,m_ulPacketSize-ulBytesRemaining);

			pobNextStream->m_ulDataRead=m_ulPacketSize-ulBytesRemaining;
		}
		else  // We are finishing
		{
			memset( ((uint8_t*)pvDestination)+ulBytesRemaining,
					m_ucSilence, 
					m_ulPacketSize-ulBytesRemaining );
		}

		bChangeStream=true;

		m_bResetPosition=true;
		
		m_ulNotificationOffset=ulOffset+ulBytesRemaining;
	}
	else // Read in a full packet
	{
		if (pobCurrentStream->m_ulDataRead==0) // We have started a new stream
		{
			m_bResetPosition=true;
			m_ulNotificationOffset=ulOffset;
		}

		pobCurrentStream->m_pobWaveFile->GetWaveData(pvDestination,pobCurrentStream->m_ulDataRead,m_ulPacketSize);

		pobCurrentStream->m_ulDataRead+=m_ulPacketSize;
	}

	// See if we need to prematurely change to the next stream
	
//	EnterCriticalSection(&gobCriticalSection);

	if (m_bForceStreamChange)
	{
		bChangeStream=true;
		m_bForceStreamChange=false;
	}

//	LeaveCriticalSection(&gobCriticalSection);


	// Check to see if we need to change to the next stream

	if (bChangeStream)
	{
//		EnterCriticalSection(&gobCriticalSection); // CRITICAL SECTION BEGIN
			
		m_iCurrentStream=!m_iCurrentStream; // Change to the next stream

		pobCurrentStream->m_pobWaveFile=NULL; // Previous stream is no longer valid

//		LeaveCriticalSection(&gobCriticalSection); // CRITICAL SECTION END
	}
}



// !!! Threaded function !!!
void CMusicTrack::ThreadUpdate ()
{
	if (!m_pobSoundBuffer) // No sound buffer, no update!
		return;

	// ----- Write sound data to the buffer -----

	void* pvBuffer=NULL;

	if (m_ulPlayCursorPosition>=m_ulPacketSize) // We are in the second segment
	{
		if (m_iNextPacket==0 && m_pobSoundBuffer->Lock(&pvBuffer,0,m_ulPacketSize)) // We are ready to read data into the first segment
		{
			ReadFromFile(pvBuffer,0);

			m_pobSoundBuffer->Unlock(pvBuffer,m_ulPacketSize);

			m_iNextPacket=1;
		}
	}
	else if (m_ulPlayCursorPosition<m_ulPacketSize) // We are in the first segment
	{
		if (m_iNextPacket==1 && m_pobSoundBuffer->Lock(&pvBuffer,m_ulPacketSize,m_ulPacketSize)) // We are ready to read into the second segment
		{
			ReadFromFile(pvBuffer,m_ulPacketSize);

			m_pobSoundBuffer->Unlock(pvBuffer,m_ulPacketSize);

			m_iNextPacket=0;
		}
	}

	// ----- Get play cursor position -----

	m_ulLastPlayCursorPosition=m_ulPlayCursorPosition;
	
	m_ulPlayCursorPosition=m_pobSoundBuffer->GetPlayCursor();

	// ----- Calculate position -----

	const u_long ulBufferSize=m_ulPacketSize<<1;
	u_long ulBytesPlayed;

	if (m_ulLastPlayCursorPosition>m_ulPlayCursorPosition)
		ulBytesPlayed=(ulBufferSize-m_ulLastPlayCursorPosition)+m_ulPlayCursorPosition;
	else
		ulBytesPlayed=m_ulPlayCursorPosition-m_ulLastPlayCursorPosition;
	
	m_ulPosition+=ulBytesPlayed;

	if (m_ulPosition>m_aobStream[m_iCurrentStream].m_ulEndOffset) // Make sure position cannot exceed end offset of current stream
		m_ulPosition=m_aobStream[m_iCurrentStream].m_ulEndOffset;

	if (m_bResetPosition)
	{
		// Check to see if we have hit the notification offset

		if ((m_ulPlayCursorPosition>=m_ulNotificationOffset && m_ulLastPlayCursorPosition<=m_ulNotificationOffset) ||
			(m_ulLastPlayCursorPosition>m_ulPlayCursorPosition && (m_ulNotificationOffset>m_ulLastPlayCursorPosition || m_ulNotificationOffset<m_ulPlayCursorPosition)))
		{
			m_ulPosition=m_ulPlayCursorPosition-m_ulNotificationOffset; // Adjust the position

			m_bResetPosition=false;

			RestoreFadeVolume(); // Restore fade volume

			SetTrackNotification(NOTIFICATION_LOOP); // We have hit a loop point

			m_uiCurrentMarker=0; // Reset marker index
		}
	}
	else if (m_aobStream[m_iCurrentStream].m_uiTotalMarkers>0)
	{
		// Fire marker notifications

		m_ulNextMarkerOffset=m_aobStream[m_iCurrentStream].m_pobWaveFile->GetMarkerOffset(m_uiCurrentMarker);

		if (m_uiCurrentMarker<m_aobStream[m_iCurrentStream].m_uiTotalMarkers && m_ulPosition>m_ulNextMarkerOffset)
		{
			//ntPrintf("position=%d  index=%d  marker=%d\n",m_ulPosition,m_uiCurrentMarker,ulMarkerOffset);

			SetTrackNotification(NOTIFICATION_MARKER);

			++m_uiCurrentMarker;
		}
	}
}


void CMusicTrack::SetVolume (float fVolume)
{
	m_fBaseVolume=fVolume;
}

void CMusicTrack::SetPitch (float fPitch)
{
	m_fBasePitch=fPitch;
}

u_int CMusicTrack::GetCurrentSegmentID () const
{
	if (m_pobCurrentSegment)
		return m_pobCurrentSegment->GetID();

	return 0; // There is no current segment for this track
}


void CMusicTrack::ChangeAudioStream ()
{
//	EnterCriticalSection(&gobCriticalSection);
	
	m_bForceStreamChange=true;

//	LeaveCriticalSection(&gobCriticalSection);
}


void CMusicTrack::RestoreFadeVolume ()
{
//	EnterCriticalSection(&gobCriticalSection);

	m_bRestoreFadeVolume=true;
	
//	LeaveCriticalSection(&gobCriticalSection);
}

void CMusicTrack::StopOnNextMarker ()
{
//	EnterCriticalSection(&gobCriticalSection);

	const u_long ulDataRead=m_aobStream[m_iCurrentStream].m_ulDataRead;

	u_int uiNextMarkerIndex;
	
	if (m_aobStream[m_iCurrentStream].m_pobWaveFile)
		m_aobStream[m_iCurrentStream].m_pobWaveFile->GetNextMarker(ulDataRead,m_ulChangeEndOffset,uiNextMarkerIndex);

//	LeaveCriticalSection(&gobCriticalSection);
}

void CMusicTrack::SetTrackNotification (TRACK_NOTIFICATION eNotification)
{
//	EnterCriticalSection(&gobCriticalSection);

	m_eNotification=eNotification;

//	LeaveCriticalSection(&gobCriticalSection);
}

TRACK_NOTIFICATION CMusicTrack::GetTrackNotification ()
{
	TRACK_NOTIFICATION eNotification;

//	EnterCriticalSection(&gobCriticalSection);

	eNotification=m_eNotification;
	m_eNotification=NOTIFICATION_UNUSED;

//	LeaveCriticalSection(&gobCriticalSection);

	return eNotification;
}

void CMusicTrack::RestartSegment ()
{
//	EnterCriticalSection(&gobCriticalSection);

	m_bRestart=true;

//	LeaveCriticalSection(&gobCriticalSection);
}



//---------------------------------------------------------------------------------------------------------------------------------------------------




CInteractiveMusicManager::CInteractiveMusicManager ()
:	m_pobDirectSoundInterface(0)
,	m_ulBufferSize(0)
//,	m_hProcessThread(INVALID_HANDLE_VALUE)
{
//	InitializeCriticalSection(&gobCriticalSection);
}

CInteractiveMusicManager::~CInteractiveMusicManager ()
{
#ifndef _NO_DSOUND
	// Wait for thread to stop

	if (m_hProcessThread!=INVALID_HANDLE_VALUE)
	{
		m_bTerminateThread=true; // Signal thread to stop

		// Wait for the thread to stop

		u_long ulResult;

		do {
			ulResult=WaitForSingleObject(m_hProcessThread,INFINITE);

		} while(ulResult!=WAIT_OBJECT_0);

		CloseHandle(m_hProcessThread);
	}

	// Free music track allocations

	while(m_obMusicTrackList.size()>0)
	{
#ifdef INTERACTIVEMUSIC_PRINTF
		ntPrintf("CInteractiveMusicManager: Releasing music track %s\n",m_obMusicTrackList.back()->GetNameP());
#endif

		NT_DELETE( m_obMusicTrackList.back() );
		m_obMusicTrackList.pop_back();
	}

#ifdef INTERACTIVEMUSIC_PRINTF
	ntPrintf("CInteractiveMusicManager: Releasing resources\n");
#endif

#endif
}

void CInteractiveMusicManager::Initialise (long lChannels,u_long ulSamplesPerSec,long lBitsPerSample,int iMillisec)
{
#ifndef _NO_DSOUND
	// Make sure there is a directsound interface

	m_pobDirectSoundInterface=CAudioEngine::Get().GetDirectSoundP();

	if (!m_pobDirectSoundInterface)
	{
#ifdef INTERACTIVEMUSIC_PRINTF
		ntPrintf("CInteractiveMusicManager: Failed to add music track, no DirectSound interface\n");
#endif
		return;
	}

	// Set our format for the music

	m_stWaveFormat.wFormatTag=WAVE_FORMAT_PCM;
	m_stWaveFormat.nChannels=(WORD)lChannels;
	m_stWaveFormat.nSamplesPerSec=ulSamplesPerSec;
	m_stWaveFormat.wBitsPerSample=(WORD)lBitsPerSample;
	m_stWaveFormat.nBlockAlign=(WORD)((lChannels*lBitsPerSample)/8);
	m_stWaveFormat.nAvgBytesPerSec=m_stWaveFormat.nSamplesPerSec * m_stWaveFormat.nBlockAlign;
	m_stWaveFormat.cbSize=0;

	// Set default volume/pitch

	m_fGlobalVolume=1.0f;
	m_fGlobalPitch=1.0f;

	// Calculate our buffer size

	m_ulBufferSize=CSoundUtil::CalculatePacketSize(&m_stWaveFormat,iMillisec);

	// Set defaults

	m_bRequestPause=false;
	m_bRequestResume=false;

	m_bDebugRender=false;

	m_pobCrossover_SourceTrack=0;
	m_pobCrossover_TargetTrack=0;
	m_pobCrossover_Segment=0;
	m_fCrossover_TimeRemaining=0.0f;

#ifdef INTERACTIVEMUSIC_PRINTF
	ntPrintf("CInteractiveMusicManager: Initialised\n");
#endif

#endif
}

bool CInteractiveMusicManager::AddMusicTrack (const char* pcName)
{
#ifndef _NO_DSOUND

	if (!m_pobDirectSoundInterface) // Ignore if no DirectSound interface is available
		return false;

	if (m_ulBufferSize==0)
	{
#ifdef INTERACTIVEMUSIC_PRINTF
		ntPrintf("CInteractiveMusicManager: Failed to add music track, manager not initialised\n");
#endif
		return false;
	}

    // Create a new music track

	CMusicTrack* pobTrack=NT_NEW CMusicTrack(pcName);

	ntAssert(pobTrack);

	if (!pobTrack->Initialise(CAudioEngine::Get().GetDirectSoundP(),&m_stWaveFormat,m_ulBufferSize))
	{
#ifdef INTERACTIVEMUSIC_PRINTF
		ntPrintf("CInteractiveMusicManager: Failed to add music track %s\n",pcName);
#endif

		return false;
	}

	// Add it to list

	m_obMusicTrackList.push_back(pobTrack);

#ifdef INTERACTIVEMUSIC_PRINTF
	ntPrintf("CInteractiveMusicManager: Adding %s\n",pcName);
#endif

#endif

	return true;
}
#ifndef _NO_DSOUND

DWORD WINAPI MusicThreadUpdate (LPVOID lpParameter)
{
	CInteractiveMusicManager* pobMusicManager=(CInteractiveMusicManager*)lpParameter;
	
	while(pobMusicManager->ThreadUpdate())
	{
		Sleep(CInteractiveMusicManager::iTHREAD_SLEEP_TIME);
	}

#ifdef INTERACTIVEMUSIC_PRINTF
	ntPrintf("CInteractiveMusicManager: Exiting MusicThreadUpdate\n");
#endif

	return 0;
}

#endif

bool CInteractiveMusicManager::Begin ()
{
#ifndef _NO_DSOUND
	if (!m_pobDirectSoundInterface) // Ignore if no DirectSound interface is available
		return false;

	if (m_ulBufferSize==0)
	{
#ifdef INTERACTIVEMUSIC_PRINTF
		ntPrintf("CInteractiveMusicManager: Failed to begin, manager not initialised\n");
#endif
		return false;
	}

	// Create our thread

	m_bTerminateThread=false;

	m_hProcessThread=CreateThread(NULL,iTHREAD_STACK_SIZE,MusicThreadUpdate,this,NULL,NULL);

	ntAssert(m_hProcessThread);

	// Begin playback on our tracks

	for(ntstd::List<CMusicTrack*>::iterator obTrackIt=m_obMusicTrackList.begin();
		obTrackIt!=m_obMusicTrackList.end();
		++obTrackIt)
	{
		(*obTrackIt)->Begin();
	}
#endif
	return true;
}


void CInteractiveMusicManager::Update ()
{
#ifndef _NO_DSOUND
	if (!m_pobDirectSoundInterface)
		return;

	for(ntstd::List<CMusicTrack*>::iterator obTrackIt=m_obMusicTrackList.begin();
	obTrackIt!=m_obMusicTrackList.end();
		++obTrackIt)
	{
		(*obTrackIt)->MainUpdate();
	}

	if (m_pobCrossover_Segment)
	{
		if (m_pobCrossover_SourceTrack->GetTimeRemainingUntilTransition()<m_fCrossover_TimeRemaining)
		{
			m_pobCrossover_Segment->Initialise(); // Initialise the segment
			m_pobCrossover_TargetTrack->SetSegment(m_pobCrossover_Segment,STAGE_INTRO,STOP_FINISH);

			m_pobCrossover_Segment=0;

			ntPrintf("CInteractiveMusicManager: Triggering crossover segment %s on %s\n",
				m_pobCrossover_Segment->GetNameC(),
				m_pobCrossover_TargetTrack->GetNameP());
		}
	}


#ifndef _RELEASE	
	
	CInputKeyboard* pobKeyboard = CInputHardware::Get().GetKeyboardP();

	if (pobKeyboard->IsKeyPressed(KEYC_T,KEYM_SHIFT))
	{
		m_bDebugRender=!m_bDebugRender;

		//ntPrintf("CInteractiveMusicManager: DebugRender=%d\n",m_bDebugRender);
	}

	DebugRender();

#endif // _RELEASE

#endif
}

void CInteractiveMusicManager::DebugRender ()
{
	if (!m_bDebugRender) // Only render if its enabled!
		return;

	float fOffsetX=10.0f;
	float fOffsetY=10.0f;

	char acText [128];

	g_VisualDebug->Printf2D(fOffsetX, fOffsetY, 0xffffffff, 0, "CInteractiveMusicManager");
	fOffsetY+=12.0f;

	for(ntstd::List<CMusicTrack*>::iterator obTrackIt=m_obMusicTrackList.begin();
		obTrackIt!=m_obMusicTrackList.end();
		++obTrackIt)
	{
		CMusicTrack* pobTrack=(*obTrackIt);
		CMusicSegment* pobSegment=pobTrack->GetCurrentMusicSegmentP();

		if (pobSegment)
		{
			CMusicSample* pobSample=pobSegment->GetCurrentSampleP();

			if (pobSample)
				sprintf(acText,"%s: %s (%s) %.2f/%.2f Marker:%0.2f Exit:%0.2f",
					pobTrack->GetNameP(),
					ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( pobSample )),
					pobSegment->GetNameP(),
					pobTrack->GetPlayPosition(),
					pobTrack->GetDuration(),
					pobTrack->GetNextMarkerPosition(),
					pobTrack->GetTimeRemainingUntilTransition());
			else
				sprintf(acText,"%s: N/A",pobTrack->GetNameP());
		}
		else
		{
			sprintf(acText,"%s:",pobTrack->GetNameP());
		}

		g_VisualDebug->Printf2D(fOffsetX, fOffsetY, 0xffffffff, 0, acText);
		fOffsetY+=12.0f;
	}
}

bool CInteractiveMusicManager::ThreadUpdate ()
{
	for(ntstd::List<CMusicTrack*>::iterator obTrackIt=m_obMusicTrackList.begin();
		obTrackIt!=m_obMusicTrackList.end();
		++obTrackIt)
	{
		(*obTrackIt)->ThreadUpdate();
	}

	if (m_bTerminateThread)
	{
		//ntPrintf("\treceived signal to terminate thread...\n");

		// Mute the tracks

		for(ntstd::List<CMusicTrack*>::iterator obTrackIt=m_obMusicTrackList.begin();
			obTrackIt!=m_obMusicTrackList.end();
			++obTrackIt)
		{
			(*obTrackIt)->SetVolume(0.0f);
		}

		return false;
	}

	return true;
}

bool CInteractiveMusicManager::PlaySegment (const char* pcSegment,const char* pcTrack,TRACK_STOP eStopOption)
{
	if (!m_pobDirectSoundInterface) // Ignore if no DirectSound interface is available
		return false;

	// Check to make sure this segment isn't already being used

	CHashedString obSegment(pcSegment);
	CHashedString obTrack(pcTrack);
	CMusicTrack* pobTrack=NULL;

	for(ntstd::List<CMusicTrack*>::iterator obTrackIt=m_obMusicTrackList.begin(); obTrackIt!=m_obMusicTrackList.end(); ++obTrackIt)
	{
		bool bMatchingSegment=(obSegment.GetHash()==(*obTrackIt)->GetCurrentSegmentID() ? true : false);
		bool bMatchingTrack=((*obTrackIt)->GetID()==obTrack.GetHash() ? true : false);

		if (!bMatchingTrack)
		{
			if (bMatchingSegment)
			{
				#ifdef _DISPLAY_ERRORS
				ntPrintf("CInteractiveMusicManager: Failed to play %s, segment is already playing on another track %s\n",pcSegment,(*obTrackIt)->GetNameP());
				#endif
				return false;
			}
		}
		else
		{
			if (bMatchingSegment)
			{
				(*obTrackIt)->RestartSegment();
				return true;
			}

			pobTrack=(*obTrackIt);
		}
	}

	if (!pobTrack)
	{
#ifdef _DISPLAY_ERRORS
		ntPrintf("CInteractiveMusicManager: Failed to play %s, track %s not found\n",pcSegment,pcTrack);
#endif
		return false;
	}

	// Find the music segment

	CMusicSegment* pobSegment=GetMusicSegmentP(pcSegment);

	if (!pobSegment)
	{
#ifdef _DISPLAY_ERRORS
		ntPrintf("CInteractiveMusicManager: Unable to find music segment %s\n",pcSegment);
#endif
		return false;
	}

	pobSegment->Initialise(); // Initialise the segment

	pobTrack->SetSegment(pobSegment,STAGE_INTRO,eStopOption);

#ifdef INTERACTIVEMUSIC_PRINTF
	ntPrintf("CInteractiveMusicManager: Playing %s on %s\n",pcSegment,pcTrack);
#endif

	return true;
}

bool CInteractiveMusicManager::StopTrack (const char* pcTrackName,TRACK_STOP eStopOption,float fDuration)
{
	if (!m_pobDirectSoundInterface) // Ignore if no DirectSound interface is available
		return false;

	CMusicTrack* pobTrack=GetMusicTrackP(pcTrackName);

	if (!pobTrack)
	{
#ifdef INTERACTIVEMUSIC_PRINTF
		ntPrintf("CInteractiveMusicManager: Failed to stop %s, track not found\n",pcTrackName);
#endif

		return false;
	}
	
	pobTrack->Stop(eStopOption,fDuration);

#ifdef INTERACTIVEMUSIC_PRINTF
	ntPrintf("CInteractiveMusicManager: Stoping %s\n",pcTrackName);
#endif

	return true;
}

bool CInteractiveMusicManager::SetCrossoverTrigger (const char* pcSourceTrack, float fTimeRemaining, const char* pcTargetTrack, const char* pcCrossoverSegment)
{
	if (!m_pobDirectSoundInterface) // Ignore if no DirectSound interface is available
		return false;

	m_pobCrossover_SourceTrack=GetMusicTrackP(pcSourceTrack);
	m_pobCrossover_TargetTrack=GetMusicTrackP(pcTargetTrack);
	m_pobCrossover_Segment=GetMusicSegmentP(pcCrossoverSegment);
	m_fCrossover_TimeRemaining=fTimeRemaining;

	if (!m_pobCrossover_SourceTrack || !m_pobCrossover_TargetTrack || !m_pobCrossover_Segment)
	{
		m_pobCrossover_Segment=0;
		return false;
	}

	return true;
}

bool CInteractiveMusicManager::IsSegmentPlaying (const char* pcSegment,const char* pcTrack)
{
	if (!m_pobDirectSoundInterface) // Ignore if no DirectSound interface is available
		return false;

	CMusicTrack* pobTrack=GetMusicTrackP(pcTrack);

	ntAssert(pobTrack);

	CHashedString obSegment(pcSegment);
	
	if (pobTrack->GetCurrentSegmentID()==obSegment.GetHash())
		return true;

	return false;
}

TRACK_NOTIFICATION CInteractiveMusicManager::GetTrackNotification (const char* pcTrack)
{
	if (!m_pobDirectSoundInterface) // Ignore if no DirectSound interface is available
		return NOTIFICATION_UNUSED;

	CMusicTrack* pobTrack=GetMusicTrackP(pcTrack);
	
	ntAssert(pobTrack);

	return pobTrack->GetTrackNotification();
}

CMusicTrack* CInteractiveMusicManager::GetMusicTrackP (const char* pcTrack)
{
	ntAssert(pcTrack);

	CHashedString obTrack(pcTrack);

	for(ntstd::List<CMusicTrack*>::iterator obTrackIt=m_obMusicTrackList.begin();
		obTrackIt!=m_obMusicTrackList.end();
		++obTrackIt)
	{
		if ((*obTrackIt)->GetID()==obTrack.GetHash())
			return (*obTrackIt);
	}

	return NULL; // Not found
}


CMusicSegment* CInteractiveMusicManager::GetMusicSegmentP (const char* pcSegment)
{
	// Check that we have a string
	ntAssert( pcSegment );

	// Get the data object from this name
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( pcSegment );

	// Make sure we found something
	ntAssert_p( pDO, ( "CInteractiveMusicManager: Failed to find segment %s", pcSegment ) );

	// ...if so
	if ( pDO )
	{
		#ifndef _RELEASE
		ntAssert_p( strcmp( pDO->GetClassName(), "CMusicSegment" ) == 0,
			( "CInteractiveMusicManager: Error, %s is not a CMusicSegment type, but a %s\n", pcSegment, pDO->GetClassName() ) );
		#endif // _RELEASE
		
		return static_cast<CMusicSegment*>( pDO->GetBasePtr() );
	}

	return 0;
}

void CInteractiveMusicManager::SetTrackVolume (const char* pcTrackName,float fVolume)
{
	if (!m_pobDirectSoundInterface) // Ignore if no DirectSound interface is available
		return;

	CMusicTrack* pobMusicTrack=GetMusicTrackP(pcTrackName);

	ntAssert_p(pobMusicTrack,("CInteractiveMusicManager: Failed to set track volume, track %s not found",pcTrackName));

	if (pobMusicTrack)
		pobMusicTrack->SetVolume(fVolume);
}

void CInteractiveMusicManager::SetTrackPitch (const char* pcTrackName,float fPitch)
{
	if (!m_pobDirectSoundInterface) // Ignore if no DirectSound interface is available
		return;

	CMusicTrack* pobMusicTrack=GetMusicTrackP(pcTrackName);

	ntAssert_p(pobMusicTrack,("CInteractiveMusicManager: Failed to set track pitch, track %s not found",pcTrackName));
	
	if (pobMusicTrack)
		pobMusicTrack->SetPitch(fPitch);
}


float CInteractiveMusicManager::GetTrackVolume (const char* pcTrackName)
{
	if (!m_pobDirectSoundInterface) // Ignore if no DirectSound interface is available
		return 1.0f;

	CMusicTrack* pobMusicTrack=GetMusicTrackP(pcTrackName);

	ntAssert_p(pobMusicTrack,("CInteractiveMusicManager: Failed to get track volume, track %s not found",pcTrackName));

	if (pobMusicTrack)
		return pobMusicTrack->GetVolume();

	return 0.0f;
}

float CInteractiveMusicManager::GetTrackPitch (const char* pcTrackName)
{
	if (!m_pobDirectSoundInterface) // Ignore if no DirectSound interface is available
		return 1.0f;

	CMusicTrack* pobMusicTrack=GetMusicTrackP(pcTrackName);

	ntAssert_p(pobMusicTrack,("CInteractiveMusicManager: Failed to get track pitch, track %s not found",pcTrackName));

	if (pobMusicTrack)
		return pobMusicTrack->GetPitch();

	return 0.0f;
}

void CInteractiveMusicManager::DisplayDebugInfo ()
{
	if (!m_pobDirectSoundInterface)
	{
		ntPrintf("CInteractiveMusicManager:\n\tNo DirectSound interface\n");
	}
	else
	{
		ntPrintf("CInteractiveMusicManager: Volume=%0.2f  Pitch=%0.2f\n",m_fGlobalVolume,m_fGlobalPitch);

		for(ntstd::List<CMusicTrack*>::iterator obTrackIt=m_obMusicTrackList.begin();
			obTrackIt!=m_obMusicTrackList.end();
			++obTrackIt)
		{
			ntPrintf("\t%s : Volume=%0.2f  Pitch=%0.2f\n",
				(*obTrackIt)->GetNameP(),(*obTrackIt)->GetVolume(),(*obTrackIt)->GetPitch());
		}
	}
}

void CInteractiveMusicManager::StopAll (bool bOnMarker)
{
	if (bOnMarker)
	{
		for(ntstd::List<CMusicTrack*>::iterator obTrackIt=m_obMusicTrackList.begin(); obTrackIt!=m_obMusicTrackList.end(); ++obTrackIt)
		{
			(*obTrackIt)->Stop(STOP_ONMARKER);
		}
	}
	else
	{
		for(ntstd::List<CMusicTrack*>::iterator obTrackIt=m_obMusicTrackList.begin(); obTrackIt!=m_obMusicTrackList.end(); ++obTrackIt)
		{
			(*obTrackIt)->Stop(STOP_IMMEDIATE);
		}
	}
}


//---------------------------------------------------------------------------------------------------------------------------------------------------
