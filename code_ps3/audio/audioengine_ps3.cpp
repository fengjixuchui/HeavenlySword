/***************************************************************************************************
*
*	$Header:: /game/audioengine_pc.cpp 9     19/08/03 13:24 Harvey                                 $
*
*	Source file for CAudioEngine and related classes.
*
*	CHANGES
*
*	20/06/2003	Harvey	Created
*
***************************************************************************************************/


#include "audio/audioengine.h"
#include "audio/soundutil.h"



#ifndef _NO_DSOUND
#include <ks.h>
#include <ksmedia.h>
#endif

#ifndef _RELEASE

//#define _SHOW_SOUND_CREATION
//#define _SHOW_SOUND_RELEASE
#define _SHOW_ERROR_MESSAGES

#endif

#define _USE_LOW_QUALITY_3D





// Note: DirectSound default - X is right, Y is up, Z is forward

const float  g_afCoordinateConversion [3]={-1.0f,1.0f,1.0f};







/***************************************************************************************************
*
*	FUNCTION		CSound3DInterface::CSound3DInterface
*
*	DESCRIPTION		
*
***************************************************************************************************/

#ifndef _NO_DSOUND
CSound3DInterface::CSound3DInterface () :
	m_pobInterface3D(0),
	m_obPosition(CONSTRUCT_CLEAR),
	m_fMinDistance(DS3D_DEFAULTMINDISTANCE),
	m_fMaxDistance(DS3D_DEFAULTMAXDISTANCE),
	m_fDistanceFactor(DS3D_DEFAULTDISTANCEFACTOR),
	m_fDopplerFactor(DS3D_DEFAULTDOPPLERFACTOR),
	m_fRolloffFactor(DS3D_DEFAULTROLLOFFFACTOR)
{
}
#else
CSound3DInterface::CSound3DInterface ()
{
}
#endif // _NO_DSOUND


/***************************************************************************************************
*
*	FUNCTION		CSound3DInterface::~CSound3DInterface
*
*	DESCRIPTION		
*
***************************************************************************************************/

CSound3DInterface::~CSound3DInterface ()
{
#ifndef _NO_DSOUND
	if (m_pobInterface3D)
	{
		m_pobInterface3D->Release();
		m_pobInterface3D=NULL;
	}
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CSound3DInterface::Initialise
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CSound3DInterface::Initialise (IDirectSoundBuffer8 *pobBuffer)
{
	ntAssert(pobBuffer);
	UNUSED(pobBuffer);

#ifndef _NO_DSOUND
	if (pobBuffer->QueryInterface(IID_IDirectSound3DBuffer8,(LPVOID*)&m_pobInterface3D)==DS_OK)
		return true;
#endif

#ifdef _SHOW_ERROR_MESSAGES
	ntPrintf("CSound3DInterface: Failed to create 3d interface\n");
#endif // _SHOW_ERROR_MESSAGES

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CSound3DInterface::SetPosition
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CSound3DInterface::SetPosition (const CPoint& obPosition,bool bImmediate)
{
	ntAssert(m_pobInterface3D);
	UNUSED(obPosition);
	UNUSED(bImmediate);

#ifndef _NO_DSOUND
	HRESULT hResult;

	hResult=m_pobInterface3D->SetPosition(
				obPosition.X()*g_afCoordinateConversion[0],
				obPosition.Y()*g_afCoordinateConversion[1],
				obPosition.Z()*g_afCoordinateConversion[2],
				bImmediate ? DS3D_IMMEDIATE : DS3D_DEFERRED);

	ntAssert_p(hResult==DS_OK,("CSound3DInterface: Failed to set position\n"));

	m_obPosition=obPosition;
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CSound3DInterface::SetMinDistance
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CSound3DInterface::SetMinDistance (float fMinDist,bool bImmediate)
{
	ntAssert(m_pobInterface3D);
	UNUSED(fMinDist);
	UNUSED(bImmediate);

#ifndef _NO_DSOUND
	HRESULT hResult;
	
	hResult=m_pobInterface3D->SetMinDistance(fMinDist,(bImmediate ? DS3D_IMMEDIATE : DS3D_DEFERRED));

	ntAssert_p(hResult==DS_OK,("CSound3DInterface: Failed to set min distance\n"));

	m_fMinDistance=fMinDist;
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CSound3DInterface::SetMaxDistance
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CSound3DInterface::SetMaxDistance (float fMaxDist,bool bImmediate)
{
	ntAssert(m_pobInterface3D);
	UNUSED(fMaxDist);
	UNUSED(bImmediate);

#ifndef _NO_DSOUND
	HRESULT hResult;

	hResult=m_pobInterface3D->SetMaxDistance(fMaxDist,(bImmediate ? DS3D_IMMEDIATE : DS3D_DEFERRED));

	ntAssert_p(hResult==DS_OK,("CSound3DInterface: Failed to set max distance\n"));

	m_fMaxDistance=fMaxDist;
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CSound3DInterface::SetRolloffFactor
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CSound3DInterface::SetRolloffFactor (float fRolloff,bool bImmediate)
{
	ntAssert(m_pobInterface3D);

	UNUSED(fRolloff);
	UNUSED(bImmediate);
}

/***************************************************************************************************
*
*	FUNCTION		CSound3DInterface::SetDistanceFactor
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CSound3DInterface::SetDistanceFactor (float fDistanceFactor,bool bImmediate)
{
	ntAssert(m_pobInterface3D);

	UNUSED(fDistanceFactor);
	UNUSED(bImmediate);
}

/***************************************************************************************************
*
*	FUNCTION		CSound3DInterface::SetDopplerFactor
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CSound3DInterface::SetDopplerFactor (float fDopplerFactor,bool bImmediate)
{
	ntAssert(m_pobInterface3D);

	UNUSED(fDopplerFactor);
	UNUSED(bImmediate);
}










/***************************************************************************************************
*
*	FUNCTION		CSoundFXInterface::CSoundFXInterface
*
*	DESCRIPTION		
*
***************************************************************************************************/

CSoundFXInterface::CSoundFXInterface () :
	m_pobSoundBuffer(0),
	m_uiEffectFlags(0),
	m_pobInterfaceParamEq(0),
	m_pobInterfaceReverb(0)
{
}

/***************************************************************************************************
*
*	FUNCTION		CSoundFXInterface::~CSoundFXInterface
*
*	DESCRIPTION		
*
***************************************************************************************************/

CSoundFXInterface::~CSoundFXInterface ()
{
#ifndef _NO_DSOUND
	if (m_pobInterfaceParamEq)
	{
		m_pobInterfaceParamEq->Release();
		m_pobInterfaceParamEq=NULL;
	}

	if (m_pobInterfaceReverb)
	{
		m_pobInterfaceReverb->Release();
		m_pobInterfaceReverb=NULL;
	}
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CSoundFXInterface::Initialise
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CSoundFXInterface::Initialise (IDirectSoundBuffer8* pobBuffer)
{
	ntAssert(pobBuffer);
	
	m_pobSoundBuffer=pobBuffer;

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CSoundFXInterface::SetEffects
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CSoundFXInterface::SetEffects (u_int uiEffectFlags)
{
	UNUSED(uiEffectFlags);

#ifndef _NO_DSOUND
	ntAssert(m_pobSoundBuffer);

	if (uiEffectFlags==0) // We are disabling effects
	{
		m_pobSoundBuffer->SetFX(0,NULL,NULL);
		m_pobInterfaceParamEq=NULL;
		m_pobInterfaceReverb=NULL;
		return true;
	}

	// Enable effects

	u_long aulResult [2];

	if ((uiEffectFlags & FX_PARAMEQ) && (uiEffectFlags & FX_REVERB))
	{
		DSEFFECTDESC astEffectDesc [2]={
			{ sizeof(DSEFFECTDESC), 0, GUID_DSFX_STANDARD_PARAMEQ, NULL, NULL },		// Parameter EQ
			{ sizeof(DSEFFECTDESC), 0, GUID_DSFX_STANDARD_I3DL2REVERB, NULL, NULL }		// I3DL2 Reverb
		};

		if (m_pobSoundBuffer->SetFX(2,astEffectDesc,aulResult)!=DS_OK)
		{
			#ifdef _SHOW_ERROR_MESSAGES
			ntPrintf("CSoundFXInterface: Failed to activate effects\n");
			#endif // _SHOW_ERROR_MESSAGES
			return false;
		}
	}
	else if (uiEffectFlags & FX_PARAMEQ)
	{
		DSEFFECTDESC astEffectDesc [1]={
			{ sizeof(DSEFFECTDESC), 0, GUID_DSFX_STANDARD_PARAMEQ, NULL, NULL },		// Parameter EQ
		};

		if (m_pobSoundBuffer->SetFX(1,astEffectDesc,aulResult)!=DS_OK)
		{
			#ifdef _SHOW_ERROR_MESSAGES
			ntPrintf("CSoundFXInterface: Failed to activate effects\n");
			#endif // _SHOW_ERROR_MESSAGES
			return false;
		}
	}
	else if (uiEffectFlags & FX_REVERB)
	{
		DSEFFECTDESC astEffectDesc [1]={
			{ sizeof(DSEFFECTDESC), 0, GUID_DSFX_STANDARD_I3DL2REVERB, NULL, NULL }		// I3DL2 Reverb
		};

		if (m_pobSoundBuffer->SetFX(1,astEffectDesc,aulResult)!=DS_OK)
		{
			#ifdef _SHOW_ERROR_MESSAGES
			ntPrintf("CSoundFXInterface: Failed to activate effects\n");
			#endif // _SHOW_ERROR_MESSAGES
			return false;
		}
	}

	// Obtain interfaces for effects

	if ((uiEffectFlags & FX_PARAMEQ) && m_pobSoundBuffer->GetObjectInPath(GUID_DSFX_STANDARD_PARAMEQ,0,IID_IDirectSoundFXParamEq8,(LPVOID*)&m_pobInterfaceParamEq)!=DS_OK)
	{
		#ifdef _SHOW_ERROR_MESSAGES
		ntPrintf("CSoundFXInterface: Failed to obtain parameter EQ interface\n");
		#endif // _SHOW_ERROR_MESSAGES
		return false;
	}

	if ((uiEffectFlags & FX_REVERB) && m_pobSoundBuffer->GetObjectInPath(GUID_DSFX_STANDARD_I3DL2REVERB,0,IID_IDirectSoundFXI3DL2Reverb8,(LPVOID*)&m_pobInterfaceReverb)!=DS_OK)
	{
		#ifdef _SHOW_ERROR_MESSAGES
		ntPrintf("CSoundFXInterface: Failed to obtain reverb interface\n");
		#endif // _SHOW_ERROR_MESSAGES
		return false;
	}

	m_uiEffectFlags=uiEffectFlags;

#endif

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CSoundFXInterface::SetParamEq
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CSoundFXInterface::SetParamEq (AUDIO_EFFECT_PARAMEQ* pstParamEq)
{
	UNUSED(pstParamEq);

#ifndef _NO_DSOUND
	ntAssert(m_pobInterfaceParamEq);
	ntAssert(m_uiEffectFlags & FX_PARAMEQ);

	HRESULT hResult;

	hResult=m_pobInterfaceParamEq->SetAllParameters((DSFXParamEq*)pstParamEq);

	ntAssert(hResult==DS_OK); // Note: If it asserts here, theres a good chance fCenter exceeds 1/3 of the soundbuffer frequency
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CSoundFXInterface::GetParamEq
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CSoundFXInterface::GetParamEq (AUDIO_EFFECT_PARAMEQ* pstParamEq)
{
	UNUSED(pstParamEq);

#ifndef _NO_DSOUND
	ntAssert(m_pobInterfaceParamEq);
	ntAssert(m_uiEffectFlags & FX_PARAMEQ);

	HRESULT hResult;
	
	hResult=m_pobInterfaceParamEq->GetAllParameters((DSFXParamEq*)pstParamEq);

	ntAssert(hResult==DS_OK);
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CSoundFXInterface::SetReverb
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CSoundFXInterface::SetReverb (AUDIO_EFFECT_I3DL2REVERB* pstReverb)
{
	UNUSED(pstReverb);

#ifndef _NO_DSOUND
	ntAssert(m_pobInterfaceReverb);
	ntAssert(m_uiEffectFlags & FX_REVERB);

	HRESULT hResult;

	hResult=m_pobInterfaceReverb->SetQuality(DSFX_I3DL2REVERB_QUALITY_MIN);

	ntAssert(hResult==DS_OK);

	hResult=m_pobInterfaceReverb->SetAllParameters((DSFXI3DL2Reverb*)pstReverb);

	ntAssert(hResult==DS_OK);
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CSoundFXInterface::GetReverb
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CSoundFXInterface::GetReverb (AUDIO_EFFECT_I3DL2REVERB* pstReverb)
{
	UNUSED(pstReverb);

#ifndef _NO_DSOUND
	ntAssert(m_pobInterfaceReverb);
	ntAssert(m_uiEffectFlags & FX_REVERB);

	HRESULT hResult;

	hResult=m_pobInterfaceReverb->GetAllParameters((DSFXI3DL2Reverb*)pstReverb);

	ntAssert(hResult==DS_OK);
#endif
}
















/***************************************************************************************************
*
*	FUNCTION		CSoundBuffer::CSoundBuffer
*
*	DESCRIPTION		
*
***************************************************************************************************/
//
//CSoundBuffer::CSoundBuffer () :
//	m_pobSoundBuffer(0),
//	m_pstWaveFormat(0),
//	m_uiFlags(0),
//	m_fVolume(1.0f),
//	m_fPitch(1.0f),
//	m_ulStatus(0)
//{
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::~CSoundBuffer
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//CSoundBuffer::~CSoundBuffer ()
//{
//#ifndef _NO_DSOUND
//	if (m_pobSoundBuffer)
//	{
//		m_pobSoundBuffer->Release();
//		m_pobSoundBuffer=NULL;
//	}
//#endif
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::Initialise
//*
//*	DESCRIPTION		Create a sound buffer based on a wave file object.
//*
//***************************************************************************************************/
//
//bool CSoundBuffer::Initialise (IDirectSound8* pobDirectSound,CWaveFile* pobWaveFile,bool b3d,bool bEffects)
//{
//	UNUSED(pobDirectSound);
//	UNUSED(pobWaveFile);
//	UNUSED(b3d);
//	UNUSED(bEffects);
//
//#ifndef _NO_DSOUND
//
//	m_pstWaveFormat=pobWaveFile->GetWaveFormatP();
//
//	if (b3d && m_pstWaveFormat->nChannels>1)
//	{
//		#ifdef _SHOW_ERROR_MESSAGES
//		ntPrintf("CSoundBuffer: Failed to initialise buffer, only mono files may be 3d\n");
//		#endif // _SHOW_ERROR_MESSAGES
//		return false;
//	}
//
//	DSBUFFERDESC stBufferDesc;
//	ZeroMemory(&stBufferDesc,sizeof(DSBUFFERDESC));
//	stBufferDesc.dwSize          = sizeof(DSBUFFERDESC);
//	stBufferDesc.dwFlags         = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS;
//	stBufferDesc.dwBufferBytes   = pobWaveFile->GetBufferSize();
//	stBufferDesc.lpwfxFormat     = pobWaveFile->GetWaveFormatP();
//
//	if (b3d)
//	{
//		#ifdef _USE_LOW_QUALITY_3D
//		stBufferDesc.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
//		#else
//		stBufferDesc.guid3DAlgorithm = DS3DALG_HRTF_FULL;
//		#endif	
//
//		stBufferDesc.dwFlags|=DSBCAPS_CTRL3D; // Flag the buffer for 3d
//	}
//	else
//	{
//		stBufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;
//	}
//
//	if (bEffects)
//		stBufferDesc.dwFlags|=DSBCAPS_CTRLFX; // This buffer will support effects
//
//	HRESULT hResult=pobDirectSound->CreateSoundBuffer(&stBufferDesc,(LPDIRECTSOUNDBUFFER*)&m_pobSoundBuffer,NULL);
//
//	if (hResult!=DS_OK) // An ntError has occurred
//	{
//		DisplayErrorMessage(hResult);
//
//		return false;
//	}
//
//	// Initialise buffer interfaces
//
//	if (b3d)
//		m_obInterface3D.Initialise(m_pobSoundBuffer);
//
//	if (bEffects)
//		m_obInterfaceFX.Initialise(m_pobSoundBuffer);
//
//#endif
//
//	return true;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::Initialise
//*
//*	DESCRIPTION		Create a sound buffer that has channels directed to specific speakers.
//*
//***************************************************************************************************/
//
//bool CSoundBuffer::Initialise (IDirectSound8* pobDirectSound,CWaveFile* pobWaveFile,u_long ulChannelMask)
//{
//	UNUSED(pobDirectSound);
//	UNUSED(pobWaveFile);
//	UNUSED(ulChannelMask);
//
//#ifndef _NO_DSOUND
//
//	m_pstWaveFormat=pobWaveFile->GetWaveFormatP();
//
//	WAVEFORMATEXTENSIBLE stWaveFormatEx;
//	NT_MEMCPY(&stWaveFormatEx.Format,m_pstWaveFormat,sizeof(WAVEFORMATEX));
//	stWaveFormatEx.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
//	stWaveFormatEx.Format.cbSize=22;
//	stWaveFormatEx.Samples.wValidBitsPerSample=m_pstWaveFormat->wBitsPerSample;
//	stWaveFormatEx.dwChannelMask=ulChannelMask;
//	stWaveFormatEx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;
//
//	DSBUFFERDESC stBufferDesc;
//	ZeroMemory(&stBufferDesc,sizeof(DSBUFFERDESC));
//	stBufferDesc.dwSize          = sizeof(DSBUFFERDESC);
//	stBufferDesc.dwFlags         = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS;
//	stBufferDesc.dwBufferBytes   = pobWaveFile->GetBufferSize();
//	stBufferDesc.lpwfxFormat     = (LPWAVEFORMATEX)&stWaveFormatEx;
//	stBufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;
//
//	HRESULT hResult=pobDirectSound->CreateSoundBuffer(&stBufferDesc,(LPDIRECTSOUNDBUFFER*)&m_pobSoundBuffer,NULL);
//
//	if (hResult!=DS_OK) // Failed to create buffer, display ntError message
//	{
//		DisplayErrorMessage(hResult);
//
//		return false;
//	}
//
//#endif
//
//	return true;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::Initialise
//*
//*	DESCRIPTION		Create a standard sound buffer.
//*
//***************************************************************************************************/
//
//bool CSoundBuffer::Initialise (IDirectSound8* pobDirectSound,WAVEFORMATEX* pstWaveFormat,u_long ulBufferSize)
//{
//	UNUSED(pobDirectSound);
//	UNUSED(pstWaveFormat);
//	UNUSED(ulBufferSize);
//
//#ifndef _NO_DSOUND
//
//	ntAssert(pstWaveFormat);
//	
//	m_pstWaveFormat=pstWaveFormat;
//
//	DSBUFFERDESC stBufferDesc;
//	ZeroMemory(&stBufferDesc,sizeof(DSBUFFERDESC));
//	stBufferDesc.dwSize          = sizeof(DSBUFFERDESC);
//	stBufferDesc.dwFlags         = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS;
//	stBufferDesc.dwBufferBytes   = ulBufferSize;
//	stBufferDesc.lpwfxFormat     = m_pstWaveFormat;
//	stBufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;
//
//	HRESULT hResult=pobDirectSound->CreateSoundBuffer(&stBufferDesc,(LPDIRECTSOUNDBUFFER*)&m_pobSoundBuffer,NULL);
//
//	if (hResult!=DS_OK) // Failed to create buffer, display ntError message
//	{
//		DisplayErrorMessage(hResult);
//
//		return false;
//	}
//
//#endif
//
//	return true;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::DisplayErrorMessage
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//void CSoundBuffer::DisplayErrorMessage (HRESULT hResult)
//{
//	UNUSED(hResult);
//
//#ifndef _NO_DSOUND
//#ifdef _SHOW_ERROR_MESSAGES
//
//	switch(hResult)
//	{
//		case DSERR_ALLOCATED:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - DSERR_ALLOCATED\n");
//			break;
//
//		case DSERR_BADFORMAT:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - DSERR_BADFORMAT\n");
//			break;
//
//		case DSERR_BUFFERTOOSMALL:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - DSERR_BUFFERTOOSMALL\n");
//			break;
//
//		case DSERR_CONTROLUNAVAIL:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - DSERR_CONTROLUNAVAIL\n");
//			break;
//
//		case DSERR_DS8_REQUIRED:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - DSERR_DS8_REQUIRED\n");
//			break;
//
//		case DSERR_INVALIDCALL:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - DSERR_INVALIDCALL\n");
//			break;
//
//		case DSERR_INVALIDPARAM:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - DSERR_INVALIDPARAM\n");
//			break;
//
//		case DSERR_NOAGGREGATION:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - DSERR_NOAGGREGATION\n");
//			break;
//
//		case DSERR_OUTOFMEMORY:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - DSERR_OUTOFMEMORY\n");
//			break;
//
//		case DSERR_UNINITIALIZED:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - DSERR_UNINITIALIZED\n");
//			break;
//
//		case DSERR_UNSUPPORTED:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - DSERR_UNSUPPORTED\n");
//			break;
//
//		default:
//			ntPrintf("CSoundBuffer: Failed to create sound buffer - unknown ntError\n");
//			break;
//	}
//
//#endif // _SHOW_ERROR_MESSAGES
//#endif // _NO_DSOUND
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::Play
//*
//*	DESCRIPTION		Calling this resumes to play cursor from its current position.
//*
//***************************************************************************************************/
//
//void CSoundBuffer::Play (bool bLoop)
//{
//	UNUSED(bLoop);
//
//#ifndef _NO_DSOUND
//	ntAssert(m_pobSoundBuffer);
//
//	u_long ulFlags=0;
//		
//	if (bLoop)
//	{
//		m_uiFlags|=BUFFER_FLAG_LOOP;
//		ulFlags|=DSBPLAY_LOOPING;
//	}
//	else
//	{
//		m_uiFlags&=~BUFFER_FLAG_LOOP;
//	}
//
//	m_pobSoundBuffer->Play(0,0,ulFlags);
//#endif
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::Stop
//*
//*	DESCRIPTION		Calling this halts the play cursor.
//*
//***************************************************************************************************/
//
//void CSoundBuffer::Stop ()
//{
//#ifndef _NO_DSOUND
//	ntAssert(m_pobSoundBuffer);
//
//	m_pobSoundBuffer->Stop();
//#endif
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::GetStatus
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//u_long CSoundBuffer::GetStatus ()
//{
//#ifndef _NO_DSOUND
//	ntAssert(m_pobSoundBuffer);
//
//	m_pobSoundBuffer->GetStatus(&m_ulStatus);
//#endif
//
//	return m_ulStatus;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::Lock
//*
//*	DESCRIPTION		Lock a portion of the sound buffer so that we can write to it.
//*
//***************************************************************************************************/
//
//bool CSoundBuffer::Lock (void** pvBufferOut,u_long ulOffset,u_long ulSize)
//{
//	UNUSED(pvBufferOut);
//	UNUSED(ulOffset);
//	UNUSED(ulSize);
//
//#ifndef _NO_DSOUND
//	ntAssert(m_pobSoundBuffer);
//
//	u_long ulTemp;
//
//	HRESULT hResult=m_pobSoundBuffer->Lock(ulOffset,ulSize,pvBufferOut,&ulTemp,NULL,NULL,0);
//
//	if (hResult==DS_OK)
//		return true;
//
//	if (hResult==DSERR_BUFFERLOST) // Buffer was lost, try again
//	{
//		m_pobSoundBuffer->Restore();
//		
//		if (m_pobSoundBuffer->Lock(ulOffset,ulSize,pvBufferOut,&ulTemp,NULL,NULL,0)==DS_OK)
//			return true;
//	}
//
//#ifdef _SHOW_ERROR_MESSAGES
//	ntPrintf("CSoundBuffer: Failed to lock buffer\n");
//#endif // _SHOW_ERROR_MESSAGES
//
//#endif // _NO_DSOUND
//
//	return false;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::Unlock
//*
//*	DESCRIPTION		Unlock a portion of the sound buffer to prevent writing to it.
//*
//***************************************************************************************************/
//
//bool CSoundBuffer::Unlock (void* pvBufferIn,u_long ulSize)
//{
//	UNUSED(pvBufferIn);
//	UNUSED(ulSize);
//
//#ifndef _NO_DSOUND
//	ntAssert(pvBufferIn);
//	ntAssert(m_pobSoundBuffer);
//
//	if (m_pobSoundBuffer->Unlock(pvBufferIn,ulSize,NULL,0)==DS_OK)
//		return true;
//
//#ifdef _SHOW_ERROR_MESSAGES
//	ntPrintf("CSoundBuffer: Failed to unlock buffer\n");
//#endif // _SHOW_ERROR_MESSAGES
//
//#endif
//	return false;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::SetPlayCursor
//*
//*	DESCRIPTION		Set the play cursor position.
//*
//***************************************************************************************************/
//
//bool CSoundBuffer::SetPlayCursor (u_long ulPlayCursor)
//{
//	UNUSED(ulPlayCursor);
//	
//#ifndef _NO_DSOUND
//	ntAssert(m_pobSoundBuffer);
//
//	if (m_pobSoundBuffer->SetCurrentPosition(ulPlayCursor)==DS_OK)
//		return true;
//
//#ifdef _SHOW_ERROR_MESSAGES
//	ntPrintf("CSoundBuffer: Failed to set play cursor position\n");
//#endif // _SHOW_ERROR_MESSAGES
//
//#endif
//
//	return false;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::GetPlayCursor
//*
//*	DESCRIPTION		Get play and write positions in the buffer.
//*
//***************************************************************************************************/
//
//u_long CSoundBuffer::GetPlayCursor ()
//{	
//#ifndef _NO_DSOUND
//	ntAssert(m_pobSoundBuffer);
//
//	u_long ulPlayCursor;
//
//	if (m_pobSoundBuffer->GetCurrentPosition(&ulPlayCursor,NULL)==DS_OK)
//		return ulPlayCursor;
//
//#ifdef _SHOW_ERROR_MESSAGES
//	ntPrintf("CSoundBuffer: Failed to get cursors\n");
//#endif // _SHOW_ERROR_MESSAGES
//
//#endif
//
//	return 0;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::SetVolume
//*
//*	DESCRIPTION		Set the volume on this buffer. E.g. 0.5 means play at half volume.
//*
//***************************************************************************************************/
//
//void CSoundBuffer::SetVolume (float fVolume)
//{
//	UNUSED(fVolume);
//	
//#ifndef _NO_DSOUND
//
//	ntAssert(m_pobSoundBuffer);
//
//	long lVolume=CSoundUtil::ConvertVolume(fVolume);
//	
//	HRESULT hResult=m_pobSoundBuffer->SetVolume(lVolume);
//	
//	switch(hResult)
//	{
//		case DS_OK:
//			m_fVolume=fVolume;
//			break;
//
//#ifdef _SHOW_ERROR_MESSAGES
//		case DSERR_CONTROLUNAVAIL:
//			ntPrintf("CSoundBuffer: Failed to set volume on sound buffer - control unavailable\n");
//			break;
//
//		case DSERR_GENERIC:
//			ntPrintf("CSoundBuffer: Failed to set volume on sound buffer - unknown\n");
//			break;
//		
//		case DSERR_INVALIDPARAM:
//			ntPrintf("CSoundBuffer: Failed to set volume on sound buffer - invalid parameter\n");
//			break;
//
//		case DSERR_PRIOLEVELNEEDED:
//			ntPrintf("CSoundBuffer: Failed to set volume on sound buffer - priority level needed\n");
//			break;
//
//		default:
//			ntPrintf("CSoundBuffer: Failed to set volume on sound buffer - unknown ntError\n");
//			break;
//#endif // _SHOW_ERROR_MESSAGES
//
//	}
//#endif
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CSoundBuffer::SetPitch
//*
//*	DESCRIPTION		Set the pitch on this buffer. E.g. 2.0 means play twice as fast.
//*
//***************************************************************************************************/
//
//void CSoundBuffer::SetPitch (float fPitch)
//{
//	UNUSED(fPitch);
//	
//#ifndef _NO_DSOUND
//
//	ntAssert(m_pobSoundBuffer);
//
//	u_long ulFrequency=CSoundUtil::ConvertFrequency(fPitch,m_pstWaveFormat->nSamplesPerSec);
//	
//	HRESULT hResult=m_pobSoundBuffer->SetFrequency(ulFrequency);
//
//	switch(hResult)
//	{
//		case DS_OK:
//			m_fPitch=fPitch;
//			break;
//
//#ifdef _SHOW_ERROR_MESSAGES
//		case DSERR_CONTROLUNAVAIL:
//			ntPrintf("CSoundBuffer: Failed to set pitch on sound buffer - control unavailable\n");
//			break;
//
//		case DSERR_GENERIC:
//			ntPrintf("CSoundBuffer: Failed to set pitch on sound buffer - unknown\n");
//			break;
//		
//		case DSERR_INVALIDPARAM:
//			ntPrintf("CSoundBuffer: Failed to set pitch on sound buffer - invalid parameter\n");
//			break;
//
//		case DSERR_PRIOLEVELNEEDED:
//			ntPrintf("CSoundBuffer: Failed to set pitch on sound buffer - priority level needed\n");
//			break;
//
//		default:
//			ntPrintf("CSoundBuffer: Failed to set pitch on sound buffer - unknown ntError\n");
//			break;
//#endif // _SHOW_ERROR_MESSAGES
//	}
//#endif
//}
//
//
//
//









/***************************************************************************************************
*
*	FUNCTION		CAudioStream::CAudioStream
*
*	DESCRIPTION		
*
***************************************************************************************************/

//CAudioStream::CAudioStream (CRITICAL_SECTION* pobCriticalSection) :
//	m_pobSoundBuffer(0),
//	m_pobWaveFile(0),
//	m_eState(STREAM_UNINITIALISED),
//	m_pobCriticalSection(pobCriticalSection)
//{
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::~CAudioStream
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//CAudioStream::~CAudioStream ()
//{
//	if (m_pobSoundBuffer)
//	{
//		NT_DELETE( m_pobSoundBuffer ); // Release sound buffer
//	}
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::Initialise
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//bool CAudioStream::Initialise (IDirectSound8* pobDirectSound,CWaveFile* pobWaveFile,bool b3d)
//{
//	ntAssert(pobDirectSound);
//	ntAssert(pobWaveFile);
//	ntError_p(!m_pobSoundBuffer,("CAudioStream: Error - this stream has already been initialised\n"));
//
//	// ----- Create the buffer -----
//
//	m_pobWaveFile=pobWaveFile;
//
//	m_pobSoundBuffer=NT_NEW CSoundBuffer ();
//
//	ntAssert(m_pobSoundBuffer);
//
//	if (!m_pobSoundBuffer->Initialise(pobDirectSound,m_pobWaveFile,b3d,true))
//	{
//		//ntPrintf("CAudioStream: Failed to initialise sound stream\n");
//
//		return false;
//	}
//
//	return Prepare();
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::Initialise
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//bool CAudioStream::Initialise (IDirectSound8* pobDirectSound,CWaveFile* pobWaveFile,u_long ulChannelMask)
//{
//	ntAssert_p(pobWaveFile,("CAudioStream: Error - invalid wavefile"));
//	ntAssert_p(!m_pobSoundBuffer,("CAudioStream: Error - this stream has already been initialised\n"));
//
//	// ----- Create the buffer -----
//
//	m_pobWaveFile=pobWaveFile;
//
//	m_pobSoundBuffer=NT_NEW CSoundBuffer ();
//
//	ntAssert(m_pobSoundBuffer);
//
//	if (!m_pobSoundBuffer->Initialise(pobDirectSound,pobWaveFile,ulChannelMask))
//	{
//		#ifdef _SHOW_ERROR_MESSAGES
//		ntPrintf("CAudioStream: Failed to initialise audio stream\n");
//		#endif // _SHOW_ERROR_MESSAGES
//
//		return false;
//	}
//
//	return Prepare();
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::Prepare
//*
//*	DESCRIPTION		Prepare our sound buffer for streaming.
//*
//***************************************************************************************************/
//
//bool CAudioStream::Prepare ()
//{
//	const u_long ulBufferSize=m_pobWaveFile->GetBufferSize();
//	
//	m_ulEndOffset=m_pobWaveFile->GetDataSize(); // The default end offset is essentially the end of the wave file
//	
//	m_ulPacketSize=ulBufferSize >> 1;
//	m_ucSilence=(m_pobWaveFile->GetWaveFormatP()->wBitsPerSample == 8 ? 128 : 0 );
//
//	m_uiPlayCount=0;
//	m_ulPosition=0;
//	m_ulPlayCursorPosition=0;
//	m_ulLastPosition=0;
//	m_ulLastPlayCursorPosition=0;
//
//	m_iAllocationID=CSoundUtil::GenerateAllocationID();
//
//	// Write initial packet to the buffer
//
//	void* pvBuffer=NULL;
//	
//	if (!m_pobSoundBuffer->Lock(&pvBuffer,0,m_ulPacketSize)) // Allow writing to the buffer
//		return false;
//
//	// Get any cached data (if available)
//
//	if (m_pobWaveFile->GetCachedData(pvBuffer))
//	{
//		if (m_ulEndOffset<m_ulPacketSize)
//			m_ulDataRead=m_ulEndOffset;
//		else
//			m_ulDataRead=m_ulPacketSize; // First packet is ready, so set position to packet two
//
//		m_iNextPacket=1; // Next packet is packet two
//	}
//	else // We are not using zero-latency, so read the first packet
//	{
//		m_ulDataRead=0;
//		m_iNextPacket=1;
//
//		ReadFromFile(pvBuffer,m_ulPacketSize);
//	}
//
//	if (!m_pobSoundBuffer->Unlock(pvBuffer,m_ulPacketSize)) // Prevent writing to the buffer
//		return false;
//
//	// Buffer has now been initialised
//
//	m_bLastPacketSubmitted=false;
//	m_eState=STREAM_INITIALISED;
//
//	return true;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::ReadFromFile
//*
//*	DESCRIPTION		Read wave data from a wave file object to an address.
//*
//***************************************************************************************************/
//
//void CAudioStream::ReadFromFile (void* pvDestination,u_long ulPacketSize)
//{
//	const u_long ulBytesRemaining=m_ulEndOffset-m_ulDataRead;
//
//	if (ulBytesRemaining<ulPacketSize) // Amount remaining is less than the packetsize, therefore we have reached the end of the wave data
//	{
//		// Read the last remaining fragment into the buffer
//		
//		if (ulBytesRemaining>0)
//		{
//			m_pobWaveFile->GetWaveData(pvDestination,m_ulDataRead,ulBytesRemaining);
//		}
//
//		// In this bit, we need to decide whether the remainder of the buffer needs to be filled with silence (if we are finishing)
//		// or fill it with a portion of the beginning of the wave data (if we are looping)
//
//		if (m_uiPlayCount==0 || // We are looping infinitely
//            m_uiPlayedCount<(m_uiPlayCount-1)) // We are looping a fixed amount of times and we still need to loop
//		{
//			m_pobWaveFile->GetWaveData((uint8_t*)pvDestination+ulBytesRemaining,0,m_ulPacketSize-ulBytesRemaining);
//
//			m_ulDataRead=m_ulPacketSize-ulBytesRemaining;
//		}
//		else  // We are finishing
//		{
//			FillMemory((uint8_t*)pvDestination+ulBytesRemaining,m_ulPacketSize-ulBytesRemaining,m_ucSilence); // Fill the remainder of the buffer with silence
//
//			m_bLastPacketSubmitted=true; // Set flag so we know last packet has been submitted
//
//			//m_ulDataRead=m_pobWaveFile->GetDataSize(); // All data has been read
//			m_ulDataRead+=ulPacketSize;
//		}
//
//		m_uiPlayedCount++; // Increment the played count
//	}
//	else // Read in a full packet
//	{
//		if (m_bLastPacketSubmitted) // Last packet was already submitted, fill remaining packets with silence
//		{
//			FillMemory((uint8_t*)pvDestination,m_ulPacketSize,m_ucSilence);
//		}
//		else
//		{
//			m_pobWaveFile->GetWaveData(pvDestination,m_ulDataRead,ulPacketSize);
//		}
//
//		m_ulDataRead+=ulPacketSize;
//	}
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::Process
//*
//*	DESCRIPTION		Feed the sound buffer with wave data.
//*
//***************************************************************************************************/
//
//bool CAudioStream::Process ()
//{
//	// Get the current state of this stream
//
//	STREAM_STATE eState;
//
//	EnterCriticalSection(m_pobCriticalSection);
//	eState=m_eState;
//	LeaveCriticalSection(m_pobCriticalSection);
//
//	if (!m_pobSoundBuffer || eState==STREAM_STOPPED)
//		return false;
//
//	if (eState==STREAM_UNINITIALISED || eState==STREAM_INITIALISED) // Hold processing until we start playing
//		return true;
//
//	// ----- Get play and write cursor positions -----
//
//	m_ulPlayCursorPosition=m_pobSoundBuffer->GetPlayCursor();
//
//	// ----- Write sound data to the buffer -----
//
//	void* pvBuffer=NULL;
//
//	if (m_ulPlayCursorPosition>=m_ulPacketSize) // We are in the second segment
//	{
//		if (m_iNextPacket==0 && m_pobSoundBuffer->Lock(&pvBuffer,0,m_ulPacketSize)) // We are ready to read data into the first segment
//		{
//			ReadFromFile(pvBuffer,m_ulPacketSize);
//
//			m_pobSoundBuffer->Unlock(pvBuffer,m_ulPacketSize);
//
//			m_iNextPacket=1;
//		}
//	}
//	else if (m_ulPlayCursorPosition<m_ulPacketSize) // We are in the first segment
//	{
//		if (m_iNextPacket==1 && m_pobSoundBuffer->Lock(&pvBuffer,m_ulPacketSize,m_ulPacketSize)) // We are ready to read into the second segment
//		{
//			ReadFromFile(pvBuffer,m_ulPacketSize);
//
//			m_pobSoundBuffer->Unlock(pvBuffer,m_ulPacketSize);
//
//			m_iNextPacket=0;
//		}
//	}
//
//	// ----- Calculate position in wave data -----
//	
//	const u_long ulBufferSize=m_ulPacketSize<<1;
//
//	m_ulLastPosition=m_ulPosition;
//
//	u_long ulBytesPlayed;
//
//	if (m_ulLastPlayCursorPosition>m_ulPlayCursorPosition)
//		ulBytesPlayed=(ulBufferSize-m_ulLastPlayCursorPosition)+m_ulPlayCursorPosition;
//	else
//		ulBytesPlayed=m_ulPlayCursorPosition-m_ulLastPlayCursorPosition;
//	
//	m_ulPosition+=ulBytesPlayed;
//
//	if (m_ulPosition>m_ulEndOffset)
//	{
//		if (m_bLastPacketSubmitted) // We are not looping, this means we've reached the padded silence
//		{
//			m_ulPosition=m_ulEndOffset;
//
//			StopStream(); // Signal the stream object to stop
//		}
//		else // We are looping
//		{
//			m_ulPosition-=m_ulEndOffset;
//		}
//	}
//
//	m_ulLastPlayCursorPosition=m_ulPlayCursorPosition;
//
//	return true;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::Play
//*
//*	DESCRIPTION		Kick off the sound buffer. Specifying a play count of zero means we loop
//*					infinitely.
//*
//***************************************************************************************************/
//
//void CAudioStream::Play (u_int uiPlayCount)
//{
//	ntAssert(m_pobSoundBuffer); // Make sure we have a sound buffer
//
//	STREAM_STATE eState;
//
//	EnterCriticalSection(m_pobCriticalSection);
//	eState=m_eState;
//	LeaveCriticalSection(m_pobCriticalSection);
//
//	if (eState==STREAM_INITIALISED) // We can only start playing from an initialised state!
//	{
//		m_pobSoundBuffer->Play(true); // NOTE: The sound buffer ALWAYS loops for streams, but that doesn't necessarily mean the stream loops.
//
//		m_uiPlayCount=uiPlayCount;
//		m_uiPlayedCount=0;
//		
//		EnterCriticalSection(m_pobCriticalSection);
//		m_eState=STREAM_PLAYING;
//		LeaveCriticalSection(m_pobCriticalSection);
//	}
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::Stop
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//void CAudioStream::Stop ()
//{
//	ntAssert(m_pobSoundBuffer);
//
//	StopStream();
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::Pause
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//void CAudioStream::Pause ()
//{
//	ntAssert(m_pobSoundBuffer);
//
//	EnterCriticalSection(m_pobCriticalSection);
//	STREAM_STATE eState=m_eState;
//	LeaveCriticalSection(m_pobCriticalSection);
//	
//	if (eState==STREAM_PLAYING)
//	{
//		m_pobSoundBuffer->Stop();
//
//		EnterCriticalSection(m_pobCriticalSection);
//		m_eState=STREAM_PAUSED;
//		LeaveCriticalSection(m_pobCriticalSection);
//	}
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::Resume
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//void CAudioStream::Resume ()
//{
//	ntAssert(m_pobSoundBuffer);
//
//	EnterCriticalSection(m_pobCriticalSection);
//	STREAM_STATE eState=m_eState;
//	LeaveCriticalSection(m_pobCriticalSection);
//
//	if (eState==STREAM_PAUSED)
//	{
//		m_pobSoundBuffer->Play(true);
//
//		EnterCriticalSection(m_pobCriticalSection);
//		m_eState=STREAM_PLAYING;
//		LeaveCriticalSection(m_pobCriticalSection);
//	}
//
//	
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::StopStream
//*
//*	DESCRIPTION		Safely stop the stream and set states. Note this can currently be called both
//*					inside and outside the main thread!
//*
//***************************************************************************************************/
//
//void CAudioStream::StopStream ()
//{
//	EnterCriticalSection(m_pobCriticalSection);
//	STREAM_STATE eState=m_eState;
//	LeaveCriticalSection(m_pobCriticalSection);
//
//	if (eState!=STREAM_STOPPED) // Make sure the stream hasn't already been stopped somewhere else!
//	{
//		EnterCriticalSection(m_pobCriticalSection);
//		m_eState=STREAM_STOPPED;
//		LeaveCriticalSection(m_pobCriticalSection);
//
//		m_pobSoundBuffer->Stop();
//	}
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::SetPosition
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//void CAudioStream::SetPosition (const CPoint& obPosition)
//{
//	ntAssert(m_pobSoundBuffer);
//
//	/* HC: Commented out old version, so that setposition is ALWAYS deferred for performance improvements
//	EnterCriticalSection(m_pobCriticalSection);
//	STREAM_STATE eState=m_eState;
//	LeaveCriticalSection(m_pobCriticalSection);
//
//	m_pobSoundBuffer->Get3DInterface().SetPosition(obPosition,(eState==STREAM_PLAYING ? false : true));
//	*/
//
//	m_pobSoundBuffer->Get3DInterface().SetPosition(obPosition);
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::SetMinDistance
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//void CAudioStream::SetMinDistance (float fMinDist)
//{
//	ntAssert(m_pobSoundBuffer);
//
//	EnterCriticalSection(m_pobCriticalSection);
//	STREAM_STATE eState=m_eState;
//	LeaveCriticalSection(m_pobCriticalSection);
//
//	m_pobSoundBuffer->Get3DInterface().SetMinDistance(fMinDist,(eState==STREAM_PLAYING ? false : true));
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CAudioStream::SetMaxDistance
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//void CAudioStream::SetMaxDistance (float fMaxDist)
//{
//	ntAssert(m_pobSoundBuffer);
//
//	EnterCriticalSection(m_pobCriticalSection);
//	STREAM_STATE eState=m_eState;
//	LeaveCriticalSection(m_pobCriticalSection);
//
//	m_pobSoundBuffer->Get3DInterface().SetMaxDistance(fMaxDist,(eState==STREAM_PLAYING ? false : true));
//}
//
//
//
//




















/***************************************************************************************************
*
*	FUNCTION		ThreadUpdate
*
*	DESCRIPTION		
*
***************************************************************************************************/
#ifndef _NO_DSOUND

DWORD WINAPI AudioEngineThreadUpdate (LPVOID lpParameter)
{
	CAudioEngine* pobAudioEngine=(CAudioEngine*)lpParameter;
	
	while(pobAudioEngine->ProcessThread())
	{
		Sleep(CAudioEngine::iTHREAD_SLEEP_TIME);
	}

	return 0;
}

#endif










/***************************************************************************************************
*
*	FUNCTION		CAudioEngine::CAudioEngine
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAudioEngine::CAudioEngine ()
{
	m_pobDirectSound=NULL;
	m_pobListener=NULL;
	m_iActiveStreams=0;
#ifndef _NO_DSOUND
	m_hThread=INVALID_HANDLE_VALUE;
#endif
}

///***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::~CAudioEngine
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/

CAudioEngine::~CAudioEngine ()
{
#ifndef _NO_DSOUND
	// Release the listener

	if (m_pobListener)
	{
		m_pobListener->Release();
		m_pobListener=NULL;
	}

	// Terminate our thread

	if (m_hThread!=INVALID_HANDLE_VALUE)
	{
		m_bTerminateThread=true; // Signal thread to stop

		// Wait for the thread to stop

		u_long ulResult;

		do {
			ulResult=WaitForSingleObject(m_hThread,INFINITE);

		} while(ulResult!=WAIT_OBJECT_0);

		CloseHandle(m_hThread);

		// Delete all remaining streams and critical sections
		
		for(int iCount=0; iCount<iMAX_STREAMS; iCount++)
		{
			if (m_apobAudioStream[iCount])
			{
				NT_DELETE( m_apobAudioStream[iCount] );
			}

			DeleteCriticalSection(&m_aobCriticalSection[iCount]);
		}
	}

	// Release our directsound interface

	if (m_pobDirectSound)
	{
		m_pobDirectSound->Release();

		// Uninitalize COM library

		CoUninitialize();
	}
#endif
}

///***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::Initialise
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/

bool CAudioEngine::Initialise ()
{
#ifndef _NO_DSOUND
	// Initialise COM library (this is necessary to use effects)

	CoInitialize(NULL);

	// Create DirectSound interface

	if (FAILED(DirectSoundCreate8(NULL,&m_pobDirectSound,NULL)))
	{
		#ifdef _SHOW_ERROR_MESSAGES
		ntPrintf("CAudioEngine: Failed to create DirectSound interface\n");
		#endif // _SHOW_ERROR_MESSAGES

		m_pobDirectSound=0;
		return false;
	}

	// Get a pointer to the listener

	DSBUFFERDESC             stBufferDesc;
	LPDIRECTSOUNDBUFFER      pobPrimary;  // Cannot be IDirectSoundBuffer8.

	ZeroMemory(&stBufferDesc, sizeof(DSBUFFERDESC));
	stBufferDesc.dwSize = sizeof(DSBUFFERDESC);
	stBufferDesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

	if (SUCCEEDED(m_pobDirectSound->CreateSoundBuffer(&stBufferDesc,&pobPrimary,NULL)))
	{
		pobPrimary->QueryInterface(IID_IDirectSound3DListener8,(LPVOID*)&m_pobListener);//(LPVOID*)m_pobListener);

		pobPrimary->Release();
	}
	else
	{
		return false;
	}

	// Set cooperative level

	m_pobDirectSound->SetCooperativeLevel( (HWND)GraphicsDevice::Get().m_Platform.GetHwnd(),DSSCL_PRIORITY);

	// Check allocations

	ntAssert(m_pobDirectSound);
	ntAssert(m_pobListener);


	// Create worker thread

	m_bTerminateThread=false;

	for(int iCount=0; iCount<iMAX_STREAMS; iCount++)
	{
		InitializeCriticalSection(&m_aobCriticalSection[iCount]); // Initialise our critical sections

		m_apobAudioStream[iCount]=NULL;
		m_abDeleteStream[iCount]=false;
	}

	m_hThread=CreateThread(NULL,iTHREAD_STACK_SIZE,AudioEngineThreadUpdate,this,NULL,NULL);

	ntAssert(m_hThread);
#endif
	return true;
}

///***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::ProcessThread
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/

bool CAudioEngine::ProcessThread ()
{
#ifndef _NO_DSOUND
	if (m_bTerminateThread) // We have received a signal from the main thread to terminate this thread
		return false;

	for(int iCount=0; iCount<iMAX_STREAMS; iCount++)
	{
		if (m_apobAudioStream[iCount]!=NULL && m_abDeleteStream[iCount]==false)
		{
			if (!m_apobAudioStream[iCount]->Process())
			{
				m_abDeleteStream[iCount]=true; // Flag this slot for deletion
			}
		}
	}	
#endif
	return true;
}

///***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::DetectSpeakerConfig
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/

void CAudioEngine::DetectSpeakerConfig ()
{
#ifndef _NO_DSOUND
	m_eSpeakerConfig=SPEAKER_CONFIG_STEREO;
#endif
}

///***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::SetListenerPosition
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
#ifdef _NO_DSOUND
void CAudioEngine::SetListenerPosition (float ,float ,float ) {}
#else
void CAudioEngine::SetListenerPosition (float fX,float fY,float fZ)
{
	ntAssert(m_pobListener);

	m_pobListener->SetPosition(
		g_afCoordinateConversion[0]*fX,
		g_afCoordinateConversion[1]*fY,
		g_afCoordinateConversion[2]*fZ,
		DS3D_DEFERRED);
}
#endif

///***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::SetListenerVelocity
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
#ifdef _NO_DSOUND
void CAudioEngine::SetListenerVelocity (float ,float ,float ) {}
#else
void CAudioEngine::SetListenerVelocity (float fVelX,float fVelY,float fVelZ)
{
	ntAssert(m_pobListener);

	m_pobListener->SetVelocity(
		g_afCoordinateConversion[0]*fVelX,
		g_afCoordinateConversion[1]*fVelY,
		g_afCoordinateConversion[2]*fVelZ,
		DS3D_DEFERRED);
}
#endif

///***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::SetListenerOrientation
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
#ifdef _NO_DSOUND
void CAudioEngine::SetListenerOrientation (float ,float ,float, float ,float ,float ) {}
#else
void CAudioEngine::SetListenerOrientation (float fForwardX,float fForwardY,float fForwardZ,float fUpX,float fUpY,float fUpZ)
{
	ntAssert(m_pobListener);

	m_pobListener->SetOrientation(
		g_afCoordinateConversion[0]*fForwardX,
		g_afCoordinateConversion[1]*fForwardY,
		g_afCoordinateConversion[2]*fForwardZ,
		g_afCoordinateConversion[0]*fUpX,
		g_afCoordinateConversion[1]*fUpY,
		g_afCoordinateConversion[2]*fUpZ,
		DS3D_DEFERRED);
}
#endif

//***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::DoWork
//*
//*	DESCRIPTION		Process any streams that are active and update any deferred settings.
//*
//***************************************************************************************************/

void CAudioEngine::DoWork ()
{
#ifndef _NO_DSOUND
	if (!m_pobDirectSound || !m_pobListener) // Engine isn't initialised, so do nothing
		return;

	// Free up slots for completed audio streams

	for(int iCount=0; iCount<iMAX_STREAMS; ++iCount)
	{
		if (m_abDeleteStream[iCount]==true) // This stream has been flagged for deletion by the thread
		{
			if (m_apobAudioStream[iCount]) // Make sure it has a valid stream
			{
				CAudioStream* pobAudioStream=m_apobAudioStream[iCount];

				m_apobAudioStream[iCount]=NULL;
				m_abDeleteStream[iCount]=false;

				NT_DELETE( pobAudioStream );

				--m_iActiveStreams;
			}
		}
	}

	ntAssert(m_iActiveStreams>=0 && m_iActiveStreams<=iMAX_STREAMS); // Make sure number of active streams is valid

	// Commit any deferred settings on the listener

	m_pobListener->CommitDeferredSettings();
#endif
}

///***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::CreateStream
//*
//*	DESCRIPTION		Prepare a sound object.
//*
//***************************************************************************************************/

CAudioStream* CAudioEngine::CreateStream (u_int uiWaveBankID,u_int uiWaveFileID,bool b3d)
{
#ifndef _NO_DSOUND
	if (!m_pobDirectSound)
		return NULL;

	// Find a free slot for this stream

	for(int iCount=0; iCount<iMAX_STREAMS; ++iCount)
	{
		if (m_apobAudioStream[iCount]==NULL) // Found a free slot
		{
			CAudioStream* pobStream=NT_NEW CAudioStream(&m_aobCriticalSection[iCount]);

			ntAssert(pobStream);

			CWaveFile* pobWaveFile=CAudioResourceManager::Get().FindWaveFileP(uiWaveBankID,uiWaveFileID);

			if (pobWaveFile && pobStream->Initialise(m_pobDirectSound,pobWaveFile,b3d))
			{
				m_apobAudioStream[iCount]=pobStream; // Assign this stream to the free slot

				++m_iActiveStreams;

				#ifdef _SHOW_SOUND_CREATION
				ntPrintf("CAudioEngine: Creating sound instance ID# %d\n",pobStream->GetAllocationID());
				#endif

				return pobStream;
			}

			// Failed to create this stream

			#ifdef _SHOW_ERROR_MESSAGES
			ntPrintf("CAudioEngine: Failed to create stream, unable to initialise stream and/or find wavefile\n");
			#endif // _SHOW_ERROR_MESSAGES

			NT_DELETE( pobStream );

			return NULL;
		}
	}

	#ifdef _SHOW_ERROR_MESSAGES
	ntPrintf("CAudioEngine: Failed to create stream, unable to find a free slot\n");
	#endif // _SHOW_ERROR_MESSAGES
#endif
	return NULL;
}

///***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::CreateStream
//*
//*	DESCRIPTION		Prepare a sound object.
//*
//***************************************************************************************************/

CAudioStream*	CAudioEngine::CreateStream (u_int uiWaveBankID,u_int uiWaveFileID,u_long ulChannelMask)
{
#ifndef _NO_DSOUND
	if (!m_pobDirectSound)
		return NULL;

	// Find a free slot for this stream

	for(int iCount=0; iCount<iMAX_STREAMS; iCount++)
	{
		if (m_apobAudioStream[iCount]==NULL) // Found a free slot
		{
			CAudioStream* pobStream=NT_NEW CAudioStream(&m_aobCriticalSection[iCount]);

			ntAssert(pobStream);

			CWaveFile* pobWaveFile=CAudioResourceManager::Get().FindWaveFileP(uiWaveBankID,uiWaveFileID);

			if (pobWaveFile && pobStream->Initialise(m_pobDirectSound,pobWaveFile,ulChannelMask))
			{
				m_apobAudioStream[iCount]=pobStream; // Assign this stream to the free slot

				++m_iActiveStreams;

				#ifdef _SHOW_SOUND_CREATION
				ntPrintf("CAudioEngine: Creating sound instance ID# %d\n",pobStream->GetAllocationID());
				#endif

				return pobStream;
			}

			// Failed to create this stream

			#ifdef _SHOW_ERROR_MESSAGES
			ntPrintf("CAudioEngine: Failed to create stream, unable to initialise stream and/or find wavefile\n");
			#endif // _SHOW_ERROR_MESSAGES

			NT_DELETE( pobStream );

			return NULL;
		}
	}

	#ifdef _SHOW_ERROR_MESSAGES
	ntPrintf("CAudioEngine: Failed to create stream, unable to find a free slot\n");
	#endif // _SHOW_ERROR_MESSAGES
#endif
	return NULL;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioEngine::CreateStream
*
*	DESCRIPTION		Prepare a sound object.
*
***************************************************************************************************/

CAudioStream* CAudioEngine::CreateStream (const char* pcWaveBank,const char* pcWaveFile,bool b3d)
{
	const u_int uiWaveBankID=CHashedString(pcWaveBank).GetValue();
	const u_int uiWaveFileID=CHashedString(pcWaveFile).GetValue();

	return CreateStream(uiWaveBankID,uiWaveFileID,b3d);
}


///***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::IsPlaying
//*
//*	DESCRIPTION		Check to see if a stream is still playing.
//*
//***************************************************************************************************/

bool CAudioEngine::IsPlaying (int iAllocationID)
{
#ifndef _NO_DSOUND
	for(int iCount=0; iCount<iMAX_STREAMS; ++iCount)
	{
		if (m_apobAudioStream[iCount])
		{
			if (m_apobAudioStream[iCount]->GetAllocationID()==iAllocationID)
				return true;
		}
	}
#endif
	return false;
}

///***************************************************************************************************
//*
//*	FUNCTION		CAudioEngine::DisplayDebugInfo
//*
//*	DESCRIPTION		Show debugging information.
//*
//***************************************************************************************************/

void CAudioEngine::DisplayDebugInfo ()
{
#ifndef _RELEASE

	if (!m_pobDirectSound)
	{
		ntPrintf("CAudioEngine:\n  No DirectSound interface\n");
	}
	else
	{
		ntPrintf("CAudioEngine:\n");
		ntPrintf("  Active streams=%d\n",m_iActiveStreams);

/*		DSCAPS stDSCaps;
		m_pobDirectSound->GetCaps(&stDSCaps);
		ntPrintf("  MaxHwMixingAllBuffers=%d\n",stDSCaps.dwMaxHwMixingAllBuffers);
		ntPrintf("  MaxHwMixingStaticBuffers=%d\n",stDSCaps.dwMaxHwMixingStaticBuffers);
		ntPrintf("  MaxHwMixingStreamingBuffers=%d\n",stDSCaps.dwMaxHwMixingStreamingBuffers);
		ntPrintf("  FreeHwMixingAllBuffers=%d\n",stDSCaps.dwFreeHwMixingAllBuffers);
		ntPrintf("  FreeHwMixingStaticBuffers=%d\n",stDSCaps.dwFreeHwMixingStaticBuffers);
		ntPrintf("  FreeHwMixingStreamingBuffers=%d\n",stDSCaps.dwFreeHwMixingStreamingBuffers);
		ntPrintf("  MaxHw3DAllBuffers=%d\n",stDSCaps.dwMaxHw3DAllBuffers);
		ntPrintf("  MaxHw3DStaticBuffers=%d\n",stDSCaps.dwMaxHw3DStaticBuffers);
		ntPrintf("  MaxHw3DStreamingBuffers=%d\n",stDSCaps.dwMaxHw3DStreamingBuffers);
		ntPrintf("  FreeHw3DAllBuffers=%d\n",stDSCaps.dwFreeHw3DAllBuffers);
		ntPrintf("  FreeHw3DStaticBuffers=%d\n",stDSCaps.dwFreeHw3DStaticBuffers);
		ntPrintf("  FreeHw3DStreamingBuffers=%d\n",stDSCaps.dwFreeHw3DStreamingBuffers);
		ntPrintf("  TotalHwMemBytes=%d\n",stDSCaps.dwTotalHwMemBytes);
		ntPrintf("  FreeHwMemBytes=%d\n",stDSCaps.dwFreeHwMemBytes);
		ntPrintf("  MaxContigFreeHwMemBytes=%d\n",stDSCaps.dwMaxContigFreeHwMemBytes); */
	}

#endif // _RELEASE
}
