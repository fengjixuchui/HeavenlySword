/***************************************************************************************************
*
*   $Header:: /game/audiocue_pc.cpp 4     19/08/03 13:24 Harvey                                    $
*
*	Source file for CSoundBank and related classes.
*
*	CHANGES		
*
*	02/07/2003	Harvey	Created
*
***************************************************************************************************/

#include "audio/audiocue.h"
#include "game/randmanager.h"

/***************************************************************************************************
*
*	FUNCTION		CAudioCue::CAudioCue
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAudioCue::CAudioCue (AUDIO_CUE_PARAMETERS* pstParam) :
	m_pstCueParameters(pstParam),
	m_iCurrentSound(0)
{
	Shuffle();
}

/***************************************************************************************************
*
*	FUNCTION		CAudioCue::~CAudioCue
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAudioCue::~CAudioCue ()
{
}

/***************************************************************************************************
*
*	FUNCTION		CAudioCue::Shuffle
*
*	DESCRIPTION		Creates a shuffled list of sounds.
*
***************************************************************************************************/

void CAudioCue::Shuffle ()
{
	if (m_pstCueParameters->m_ePlaybackMode!=PLAYBACK_SHUFFLED) // Playback mode is not shuffled
		return;

	const u_short usWaveCount=m_pstCueParameters->m_ucTotalSounds;
	
	SOUND_ENTRY* pstActual [SOUND_POOL_SIZE];

	for(int iCount=0; iCount<usWaveCount; iCount++)
		pstActual[iCount]=&m_pstCueParameters->m_astSoundEntry[iCount];

	int iCurrent=0;

	while(iCurrent<usWaveCount)
	{
		int iIndex=drand() % usWaveCount;

		if (pstActual[iIndex])
		{
			m_pstShuffled[iCurrent]=pstActual[iIndex];
			pstActual[iIndex]=NULL;
			iCurrent++;

			//ntPrintf("shuffled -> %d\n",iIndex);
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAudioCue::GetSound
*
*	DESCRIPTION		Get a wave and wavebank reference.
*
***************************************************************************************************/

void CAudioCue:: GetSound (u_int& uiBankID,u_int& uiSoundID)
{
	ntAssert(m_pstCueParameters);

	switch(m_pstCueParameters->m_ePlaybackMode)
	{
		case PLAYBACK_NORMAL :
		{
			uiSoundID=m_pstCueParameters->m_astSoundEntry[m_iCurrentSound].uiWaveFileID;
			uiBankID=m_pstCueParameters->m_astSoundEntry[m_iCurrentSound].uiWaveBankID;
			m_iCurrentSound=(m_iCurrentSound+1)%m_pstCueParameters->m_ucTotalSounds;
			break;
		}

		case PLAYBACK_RANDOM :
		{
			int iRand=(u_short)((drand() % 100)+1);

			int iCurrent=0;

			for(int iCount=0; iCount<m_pstCueParameters->m_ucTotalSounds; iCount++)
			{
				int iWeight=m_pstCueParameters->m_astSoundEntry[iCount].usWeight;
				
				if (iWeight>0 && iRand>iCurrent && iRand<=iCurrent+iWeight)
				{
					m_iCurrentSound=iCount;
					uiSoundID=m_pstCueParameters->m_astSoundEntry[m_iCurrentSound].uiWaveFileID;
					uiBankID=m_pstCueParameters->m_astSoundEntry[m_iCurrentSound].uiWaveBankID;
					return;
				}
				
				iCurrent+=iWeight;
			}
			break;
		}

		case PLAYBACK_RANDOM_NOWEIGHTS :
		{
			m_iCurrentSound=(drand() % (m_pstCueParameters->m_ucTotalSounds * 10))/10;

			uiSoundID=m_pstCueParameters->m_astSoundEntry[m_iCurrentSound].uiWaveFileID;
			uiBankID=m_pstCueParameters->m_astSoundEntry[m_iCurrentSound].uiWaveBankID;
			break;
		}

		case PLAYBACK_SHUFFLED :
		{
			uiSoundID=m_pstShuffled[m_iCurrentSound]->uiWaveFileID;
			uiBankID=m_pstShuffled[m_iCurrentSound]->uiWaveBankID;
            m_iCurrentSound=(m_iCurrentSound+1)%m_pstCueParameters->m_ucTotalSounds;
			break;
		}
	}
}












/***************************************************************************************************
*
*	FUNCTION		CSoundBank::CSoundBank
*
*	DESCRIPTION		
*
***************************************************************************************************/

CSoundBank::CSoundBank ()
{
}

/***************************************************************************************************
*
*	FUNCTION		CSoundBank::~CSoundBank
*
*	DESCRIPTION		
*
***************************************************************************************************/

CSoundBank::~CSoundBank ()
{
	ClearData();
}

/***************************************************************************************************
*
*	FUNCTION		CSoundBank::Open
*
*	DESCRIPTION		Load in a soundbank.
*
***************************************************************************************************/

bool CSoundBank::Open (const char* pcPath)
{
#if !defined(_NO_DSOUND)
	HANDLE hFileHandle=CreateFile(pcPath,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

	if (hFileHandle==INVALID_HANDLE_VALUE)
		return false;

	strcpy(m_acPath,pcPath); // Copy the path, so we can reload later

	GetFileTime(hFileHandle,0,0,&m_obModifiedTime); // Get the files modified time
	
	if (!ReadData(hFileHandle)) // Read data from sound bank
		return false;

	CloseHandle(hFileHandle); // Close file handle
#endif
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CSoundBank::Reload
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CSoundBank::Reload ()
{
#if !defined(_NO_DSOUND)
	HANDLE hFileHandle=CreateFile(m_acPath,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

	if (hFileHandle==INVALID_HANDLE_VALUE)
		return false;

	FILETIME obCurrentModifiedTime;

	GetFileTime(hFileHandle,0,0,&obCurrentModifiedTime); // Get the files modified time

	if (CompareFileTime(&obCurrentModifiedTime,&m_obModifiedTime)<=0) // File has not changed
	{
		CloseHandle(hFileHandle); // Close file handle

		return false;
	}

	m_obModifiedTime=obCurrentModifiedTime; // Update the modified time

	if (!ReadData(hFileHandle)) // Read data from sound bank
	{
		CloseHandle(hFileHandle); // Close file handle

		return false;
	}

	CloseHandle(hFileHandle); // Close file handle
#endif
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CSoundBank::FindAudioCueP
*
*	DESCRIPTION		Find an audio cue inside this sound bank.
*
***************************************************************************************************/

CAudioCue* CSoundBank::FindAudioCueP (u_int uiID)
{
#if !defined(_NO_DSOUND)
	if (uiID)
	{
		for(ntstd::List<CAudioCue*>::iterator obCueIt=m_obAudioCueList.begin(); obCueIt!=m_obAudioCueList.end(); ++obCueIt)
		{
			if ((*obCueIt)->GetID()==uiID)
				return *obCueIt;
		}
	}
#endif
	return 0; // Unable to find this audio cue
}

/***************************************************************************************************
*
*	FUNCTION		CSoundBank::GetVolumeControl
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CSoundBank::GetVolumeControl (const char* pcName,float fInput,float &fVolume)
{
#if !defined(_NO_DSOUND)
	ntAssert(pcName);
	ntAssert(fInput>=0.0f && fInput<=1.0f); // Make sure input is valid

	const u_int uiID=CHashedString(pcName).GetValue();

	for(int iCount=0; iCount<(int)m_stHeader.usVolumeControlCount; ++iCount)
	{
		if (m_pstVolumeControl[iCount].m_uiID==uiID) // We have found our parameter control
		{
			const int iTotalSamples=(int)m_pstVolumeControl[iCount].m_ucTotalPoints;

			if (fInput<=m_pstVolumeControl[iCount].m_afInput[0]) // Input starts before first input
			{
				fVolume=m_pstVolumeControl[iCount].m_afVolume[0];
				return true;
			}

			if (fInput>=m_pstVolumeControl[iCount].m_afInput[iTotalSamples-1]) // Input starts after last
			{
				fVolume=m_pstVolumeControl[iCount].m_afVolume[iTotalSamples-1];
				return true;
			}

			for(int iSample=0; iSample<iTotalSamples-1; ++iSample) // Find out which two points the input lies between
			{
				float fCurrentInput=m_pstVolumeControl[iCount].m_afInput[iSample];
				float fNextInput=m_pstVolumeControl[iCount].m_afInput[iSample+1];

				if (fInput>=fCurrentInput && fInput<=fNextInput) // Interpolate
				{
					float fCurrentVolume=m_pstVolumeControl[iCount].m_afVolume[iSample];
					float fDiff=m_pstVolumeControl[iCount].m_afVolume[iSample+1]-fCurrentVolume;
					float fRatio=(fInput-fCurrentInput)/(fNextInput-fCurrentInput);

					fVolume=fCurrentVolume+(fDiff*fRatio);

					return true;
				}
			}
		}
	}
#endif
	return false; // Failed to find volume control
}

/***************************************************************************************************
*
*	FUNCTION		CSoundBank::GetPitchControl
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CSoundBank::GetPitchControl (const char* pcName,float fInput,float &fPitch)
{
#if !defined(_NO_DSOUND)
	ntAssert(pcName);
	ntAssert(fInput>=0.0f && fInput<=1.0f); // Make sure input is valid

	const u_int uiID=CHashedString(pcName).GetValue();

	for(int iCount=0; iCount<(int)m_stHeader.usPitchControlCount; ++iCount)
	{
		if (m_pstPitchControl[iCount].m_uiID==uiID) // We have found our parameter control
		{
			const int iTotalSamples=(int)m_pstPitchControl[iCount].m_ucTotalPoints;

			if (fInput<=m_pstPitchControl[iCount].m_afInput[0]) // Input starts before first input
			{
				fPitch=m_pstPitchControl[iCount].m_afPitch[0];
				return true;
			}

			if (fInput>=m_pstPitchControl[iCount].m_afInput[iTotalSamples-1]) // Input starts after last
			{
				fPitch=m_pstPitchControl[iCount].m_afPitch[iTotalSamples-1];
				return true;
			}

			for(int iSample=0; iSample<iTotalSamples-1; ++iSample) // Find out which two points the input lies between
			{
				float fCurrentInput=m_pstPitchControl[iCount].m_afInput[iSample];
				float fNextInput=m_pstPitchControl[iCount].m_afInput[iSample+1];

				if (fInput>=fCurrentInput && fInput<=fNextInput) // Interpolate
				{
					float fCurrentPitch=m_pstPitchControl[iCount].m_afPitch[iSample];
					float fDiff=m_pstPitchControl[iCount].m_afPitch[iSample+1]-fCurrentPitch;
					float fRatio=(fInput-fCurrentInput)/(fNextInput-fCurrentInput);

					fPitch=fCurrentPitch+(fDiff*fRatio);

					return true;
				}
			}
		}
	}
#endif
	return false; // Failed to find volume control
}

/***************************************************************************************************
*
*	FUNCTION		CSoundBank::GetParamEqControl
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CSoundBank::GetParamEqControl (const char* pcName,float fInput,AUDIO_EFFECT_PARAMEQ* pstParamEq)
{
#if !defined(_NO_DSOUND)
	ntAssert(pcName);
	ntAssert(fInput>=0.0f && fInput<=1.0f); // Make sure input is valid

	const u_int uiID=CHashedString(pcName).GetValue();

	for(int iCount=0; iCount<(int)m_stHeader.usParamEqControlCount; ++iCount)
	{
		if (m_pstParamEqControl[iCount].m_uiID==uiID) // We have found our parameter control
		{
			const int iTotalSamples=(int)m_pstParamEqControl[iCount].m_ucTotalPoints;
			const int iLastSample=iTotalSamples-1;

			if (fInput<=m_pstParamEqControl[iCount].m_afInput[0]) // Input starts before first input
			{
				pstParamEq->fCenter=m_pstParamEqControl[iCount].m_afCenter[0];
				pstParamEq->fBandwidth=m_pstParamEqControl[iCount].m_afBandwidth[0];
				pstParamEq->fGain=m_pstParamEqControl[iCount].m_afGain[0];
				return true;
			}

			if (fInput>=m_pstParamEqControl[iCount].m_afInput[iLastSample]) // Input starts after last
			{
				pstParamEq->fCenter=m_pstParamEqControl[iCount].m_afCenter[iLastSample];
				pstParamEq->fBandwidth=m_pstParamEqControl[iCount].m_afBandwidth[iLastSample];
				pstParamEq->fGain=m_pstParamEqControl[iCount].m_afGain[iLastSample];
				return true;
			}

			for(int iSample=0; iSample<iLastSample; ++iSample) // Find out which two points the input lies between
			{
				float fCurrentInput=m_pstParamEqControl[iCount].m_afInput[iSample];
				float fNextInput=m_pstParamEqControl[iCount].m_afInput[iSample+1];

				if (fInput>=fCurrentInput && fInput<=fNextInput) // Interpolate
				{
					AUDIO_PARAMEQ_CONTROL* pstThisControl=&m_pstParamEqControl[iCount];
					float fRatio=(fInput-fCurrentInput)/(fNextInput-fCurrentInput);
					
					pstParamEq->fCenter=
						pstThisControl->m_afCenter[iSample]+((pstThisControl->m_afCenter[iSample+1]-pstThisControl->m_afCenter[iSample])*fRatio);
					pstParamEq->fBandwidth=
						pstThisControl->m_afBandwidth[iSample]+((pstThisControl->m_afBandwidth[iSample+1]-pstThisControl->m_afBandwidth[iSample])*fRatio);
					pstParamEq->fGain=
						pstThisControl->m_afGain[iSample]+((pstThisControl->m_afGain[iSample+1]-pstThisControl->m_afGain[iSample])*fRatio);

					return true;
				}
			}
		}
	}
#endif
	return false; // Failed to find volume control
}

/***************************************************************************************************
*
*	FUNCTION		CSoundBank::ReadData
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CSoundBank::ReadData (void* hFileHandle)
{
#if !defined(_NO_DSOUND)
	// Clear any existing data
	
	ClearData();

	// Read the header

	u_long ulBytesRead;

    if (!ReadFile(hFileHandle,&m_stHeader,HEADER_SIZE,&ulBytesRead,NULL))
	{
		ntPrintf("CSoundBank: Failed to read header\n");
		return false;
	}

	// Read the cue data

	const u_short usCueCount=m_stHeader.usCueCount;

	m_pstAudioCue=NT_NEW AUDIO_CUE_PARAMETERS [usCueCount];

	if (!ReadFile(hFileHandle,m_pstAudioCue,CUE_SIZE * usCueCount,&ulBytesRead,NULL))
	{
		ntPrintf("CSoundBank: Failed to read cue data\n");
		return false;
	}

	// Read the volume control data

	const u_short usVolumeControlCount=m_stHeader.usVolumeControlCount;

	if (usVolumeControlCount>0)
	{
		m_pstVolumeControl=NT_NEW AUDIO_VOLUME_CONTROL [usVolumeControlCount];

		if (!ReadFile(hFileHandle,m_pstVolumeControl,VOLUME_CONTROL_SIZE * usVolumeControlCount,&ulBytesRead,NULL))
		{
			ntPrintf("CSoundBank: Failed to read volume control data\n");
			return false;
		}
	}

	// Read the pitch control data

	const u_short usPitchControlCount=m_stHeader.usPitchControlCount;

	if (usPitchControlCount>0)
	{
		m_pstPitchControl=NT_NEW AUDIO_PITCH_CONTROL [usPitchControlCount];

		if (!ReadFile(hFileHandle,m_pstPitchControl,PITCH_CONTROL_SIZE * usPitchControlCount,&ulBytesRead,NULL))
		{
			ntPrintf("CSoundBank: Failed to read pitch control data\n");
			return false;
		}
	}

	// Read the parameter EQ data

	const u_short usParamEqControlCount=m_stHeader.usParamEqControlCount;

	if (usParamEqControlCount>0)
	{
		m_pstParamEqControl=NT_NEW AUDIO_PARAMEQ_CONTROL [usParamEqControlCount];

		if (!ReadFile(hFileHandle,m_pstPitchControl,PARAMEQ_CONTROL_SIZE * usParamEqControlCount,&ulBytesRead,NULL))
		{
			ntPrintf("CSoundBank: Failed to read parameter eq control data\n");
			return false;
		}
	}

	// Create our audio cues

	for(int iCount=0; iCount<usCueCount; ++iCount)
	{
		CAudioCue* pobCue=NT_NEW CAudioCue(&m_pstAudioCue[iCount]);

		ntAssert(pobCue);

		m_obAudioCueList.push_back(pobCue);
	}
#endif
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CSoundBank::ClearData
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CSoundBank::ClearData ()
{
#if !defined(_NO_DSOUND)
	// Delete CAudioCue objects
	
	while(m_obAudioCueList.size()>0)
	{
		NT_DELETE( m_obAudioCueList.back() );
		m_obAudioCueList.pop_back();
	}

	// Delete allocations

	if (m_pstAudioCue)
	{
		NT_DELETE_ARRAY( m_pstAudioCue );
		m_pstAudioCue=0;
	}

	if (m_pstVolumeControl)
	{
		NT_DELETE_ARRAY( m_pstVolumeControl );
		m_pstVolumeControl=0;
	}

	if (m_pstPitchControl)
	{
		NT_DELETE_ARRAY( m_pstPitchControl );
		m_pstPitchControl=0;
	}

	if (m_pstParamEqControl)
	{
		NT_DELETE_ARRAY( m_pstParamEqControl );
		m_pstParamEqControl=0;
	}
#endif
}
