/***************************************************************************************************
*
*   $Header:: /game/audiomanager_pc.cpp 6     19/08/03 13:24 Harvey                                $
*
*	Source file for CAudioManager and related classes.
*
*	CHANGES		
*
*	02/07/2003	Harvey	Created
*
***************************************************************************************************/

#include "audio/audiomanager.h"
#include "audio/audiolayer.h"

#include "anim/transform.h"
#include "core/timer.h"

#include "game/randmanager.h"
#include "game/LuaGlobal.h"
#include "game/entitymanager.h"

#include "objectdatabase/dataobject.h"

#include "game/shell.h" // debugging
#include "input/inputhardware.h" // debugging
#include "core/visualdebugger.h" // debugging



// Finished Callback Hack - JML Added 09-12-05
#include "game/entity.h"
#include "game/entity.inl"
#include "game/messagehandler.h"

START_STD_INTERFACE	(CI3DL2Reverb)
	IINT		(CI3DL2Reverb, Room)
	IINT		(CI3DL2Reverb, RoomHF)
	IFLOAT		(CI3DL2Reverb, RoomRolloffFactor)
	IFLOAT		(CI3DL2Reverb, DecayTime)
	IFLOAT		(CI3DL2Reverb, DecayHFRatio)
	IINT		(CI3DL2Reverb, Reflections)
	IFLOAT		(CI3DL2Reverb, ReflectionsDelay)
	IINT		(CI3DL2Reverb, Reverb)
	IFLOAT		(CI3DL2Reverb, ReverbDelay)
	IFLOAT		(CI3DL2Reverb, Diffusion)
	IFLOAT		(CI3DL2Reverb, Density)
	IFLOAT		(CI3DL2Reverb, HFReference)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE






#ifndef _RELEASE

//#define _SHOW_DEBUG_MESSAGES
//#define _SHOW_WARNING_MESSAGES - Harvey, can you make warning messages enabled from a .config file flag please? Thanks! :)

#endif // _RELEASE


#define _ENABLE_EFFECTS
#define _ALLOW_CHANNEL_MASK




CI3DL2Reverb::CI3DL2Reverb ()
{
#ifndef _NO_DSOUND
	m_iRoom=DSFX_I3DL2REVERB_ROOM_DEFAULT;
	m_iRoomHF=DSFX_I3DL2REVERB_ROOMHF_DEFAULT;
	m_fRoomRolloffFactor=DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_DEFAULT;
	m_fDecayTime=DSFX_I3DL2REVERB_DECAYTIME_DEFAULT;
	m_fDecayHFRatio=DSFX_I3DL2REVERB_DECAYHFRATIO_DEFAULT;
	m_iReflections=DSFX_I3DL2REVERB_REFLECTIONS_DEFAULT;
	m_fReflectionsDelay=DSFX_I3DL2REVERB_REFLECTIONSDELAY_DEFAULT;
	m_iReverb=DSFX_I3DL2REVERB_REVERB_DEFAULT;
	m_fReverbDelay=DSFX_I3DL2REVERB_REVERBDELAY_DEFAULT;
	m_fDiffusion=DSFX_I3DL2REVERB_DIFFUSION_DEFAULT;
	m_fDensity=DSFX_I3DL2REVERB_DENSITY_MAX;
	m_fHFReference=DSFX_I3DL2REVERB_HFREFERENCE_DEFAULT;
#endif
}

void CI3DL2Reverb::PostConstruct ()
{
	Validate();
}

bool CI3DL2Reverb::EditorChangeValue(CallBackParameter /*pcItem*/, CallBackParameter /*pcValue*/)
{
	Validate();

	return true;
}


void CI3DL2Reverb::Set (AUDIO_EFFECT_I3DL2REVERB& stI3DL2Reverb)
{
	stI3DL2Reverb.lRoom=				m_iRoom;
	stI3DL2Reverb.lRoomHF=				m_iRoomHF; 
	stI3DL2Reverb.fRoomRolloffFactor=	m_fRoomRolloffFactor;
	stI3DL2Reverb.fDecayTime=			m_fDecayTime;
	stI3DL2Reverb.fDecayHFRatio=		m_fDecayHFRatio;
	stI3DL2Reverb.lReflections=			m_iReflections;
	stI3DL2Reverb.fReflectionsDelay=	m_fReflectionsDelay;
	stI3DL2Reverb.lReverb=				m_iReverb;
	stI3DL2Reverb.fReverbDelay=			m_fReverbDelay; 
	stI3DL2Reverb.fDiffusion=			m_fDiffusion;
	stI3DL2Reverb.fDensity=				m_fDensity;
	stI3DL2Reverb.fHFReference=			m_fHFReference;
}

void CI3DL2Reverb::Validate ()
{
	// Make sure all the values are nice and legal

#ifndef _NO_DSOUND
	if (m_iRoom<DSFX_I3DL2REVERB_ROOM_MIN)
		m_iRoom=DSFX_I3DL2REVERB_ROOM_MIN;
		
	if (m_iRoom>DSFX_I3DL2REVERB_ROOM_MAX)
		m_iRoom=DSFX_I3DL2REVERB_ROOM_MAX;

	if (m_iRoomHF<DSFX_I3DL2REVERB_ROOMHF_MIN)
		m_iRoomHF=DSFX_I3DL2REVERB_ROOMHF_MIN;

	if (m_iRoomHF>DSFX_I3DL2REVERB_ROOMHF_MAX)
		m_iRoomHF=DSFX_I3DL2REVERB_ROOMHF_MAX;

	if (m_fRoomRolloffFactor<DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_MIN)
		m_fRoomRolloffFactor=DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_MIN;

	if (m_fRoomRolloffFactor>DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_MAX)
		m_fRoomRolloffFactor=DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_MAX;

	if (m_fDecayTime<DSFX_I3DL2REVERB_DECAYTIME_MIN)
		m_fDecayTime=DSFX_I3DL2REVERB_DECAYTIME_MIN;

	if (m_fDecayTime>DSFX_I3DL2REVERB_DECAYTIME_MAX)
		m_fDecayTime=DSFX_I3DL2REVERB_DECAYTIME_MAX;

	if (m_fDecayHFRatio<DSFX_I3DL2REVERB_DECAYHFRATIO_MIN)
		m_fDecayHFRatio=DSFX_I3DL2REVERB_DECAYHFRATIO_MIN;

	if (m_fDecayHFRatio>DSFX_I3DL2REVERB_DECAYHFRATIO_MAX)
		m_fDecayHFRatio=DSFX_I3DL2REVERB_DECAYHFRATIO_MAX;

	if (m_iReflections<DSFX_I3DL2REVERB_REFLECTIONS_MIN)
		m_iReflections=DSFX_I3DL2REVERB_REFLECTIONS_MIN;

	if (m_iReflections>DSFX_I3DL2REVERB_REFLECTIONS_MAX)
		m_iReflections=DSFX_I3DL2REVERB_REFLECTIONS_MAX;

	if (m_fReflectionsDelay<DSFX_I3DL2REVERB_REFLECTIONSDELAY_MIN)
		m_fReflectionsDelay=DSFX_I3DL2REVERB_REFLECTIONSDELAY_MIN;

	if (m_fReflectionsDelay>DSFX_I3DL2REVERB_REFLECTIONSDELAY_MAX)
		m_fReflectionsDelay=DSFX_I3DL2REVERB_REFLECTIONSDELAY_MAX;

	if (m_iReverb<DSFX_I3DL2REVERB_REVERB_MIN)
		m_iReverb=DSFX_I3DL2REVERB_REVERB_MIN;

	if (m_iReverb>DSFX_I3DL2REVERB_REVERB_MAX)
		m_iReverb=DSFX_I3DL2REVERB_REVERB_MAX;

	if (m_fReverbDelay<DSFX_I3DL2REVERB_REVERBDELAY_MIN)
		m_fReverbDelay=DSFX_I3DL2REVERB_REVERBDELAY_MIN;

	if (m_fReverbDelay>DSFX_I3DL2REVERB_REVERBDELAY_MAX)
		m_fReverbDelay=DSFX_I3DL2REVERB_REVERBDELAY_MAX;

	if (m_fDiffusion<DSFX_I3DL2REVERB_DIFFUSION_MIN)
		m_fDiffusion=DSFX_I3DL2REVERB_DIFFUSION_MIN;

	if (m_fDiffusion>DSFX_I3DL2REVERB_DIFFUSION_MAX)
		m_fDiffusion=DSFX_I3DL2REVERB_DIFFUSION_MAX;

	if (m_fDensity<DSFX_I3DL2REVERB_DENSITY_MIN)
		m_fDensity=DSFX_I3DL2REVERB_DENSITY_MIN;

	if (m_fDensity>DSFX_I3DL2REVERB_DENSITY_MAX)
		m_fDensity=DSFX_I3DL2REVERB_DENSITY_MAX;

	if (m_fHFReference<DSFX_I3DL2REVERB_HFREFERENCE_MIN)
		m_fHFReference=DSFX_I3DL2REVERB_HFREFERENCE_MIN;

	if (m_fHFReference>DSFX_I3DL2REVERB_HFREFERENCE_MAX)
		m_fHFReference=DSFX_I3DL2REVERB_HFREFERENCE_MAX;
#endif
}







/***************************************************************************************************
*
*	FUNCTION		CAudioObject::CAudioObject
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAudioObject::CAudioObject (const CAudioObjectID& obID,CAudioCue* pobCue,CAudioLayer* pobAudioLayer,SPATIALIZATION eSpatialType) :
	m_pobAudioLayer(pobAudioLayer),
	m_uiFlags(0),
	m_pobAudioStream(0),
	m_fVolume(1.0f),
	m_fPitch(1.0f),
	m_fFadeVolume(1.0f),
	m_fFadeTime(0.0f),
	m_eState(STATE_UNINITIALISED),
	m_eSpatialization(eSpatialType),
	m_pCallbackFunc(0),
	m_pCallbackData(0)
{
	ntAssert(pobCue);
	ntAssert(pobAudioLayer);

	m_obID=obID;
	m_fStartTime=CAudioManager::Get().GetTime();

	AUDIO_CUE_PARAMETERS* pstParam=pobCue->GetParametersP();

	// Take a copy of useful information
	m_uiCueID=pstParam->m_uiID;
	m_uiCategoryID=pstParam->m_uiCategoryID;
	m_usPriority=pstParam->m_usPriority;
	m_fFadeInTime=pstParam->m_fFadeIn;
	m_fFadeOutTime=pstParam->m_fFadeOut;
	m_eFadeInCurve=pstParam->m_eFadeInCurve;
	m_eFadeOutCurve=pstParam->m_eFadeOutCurve;
	m_ucPlayCount=pstParam->m_ucPlayCount;
	m_fDelayTime=pstParam->m_fDelayTime;
	m_fMinDist=pstParam->m_fMinDist;
	m_fMaxDist=pstParam->m_fMaxDist;

	// Create our sound instance

	u_int uiBankID;
	u_int uiSoundID;
	
	pobCue->GetSound(uiBankID,uiSoundID);

#ifdef _ALLOW_CHANNEL_MASK
	if (pstParam->m_ulChannelMask==0) // This is a normal sound
	{
		m_pobAudioStream=CAudioEngine::Get().CreateStream(uiBankID,uiSoundID,(m_eSpatialization==SPATIALIZATION_HRTF ? true : false));
	}
	else // This is a channel specific sound
	{
		if (m_eSpatialization==SPATIALIZATION_HRTF)
			m_eSpatialization=SPATIALIZATION_ATTENUATED; // HRTF is only permitted on single channel sounds

		m_pobAudioStream=CAudioEngine::Get().CreateStream(uiBankID,uiSoundID,pstParam->m_ulChannelMask);
	}
#else
	m_pobAudioStream=CAudioEngine::Get().CreateStream(uiBankID,uiSoundID,(m_eSpatialization==SPATIALIZATION_HRTF ? true : false));
#endif // _ALLOW_CHANNEL_MASK

	if (!m_pobAudioStream)
	{
		m_iSoundID=0;
		
		return;
	}

	m_iSoundID=m_pobAudioStream->GetAllocationID();

#ifdef _ENABLE_EFFECTS

//	Util::DisableFPUZeroDivException();
	Util::DisableFPUOverflowException();
	Util::DisableFPUInvalidException();

	if ((pstParam->m_ucFlags & FLAG_REVERB) && (pstParam->m_ucFlags & FLAG_PARAMEQ))
	{
		m_pobAudioStream->GetFXInterface().SetEffects(FX_PARAMEQ | FX_REVERB);

		m_uiFlags|=AUDIO_OBJECT_FLAGS::FLAG_REVERB | AUDIO_OBJECT_FLAGS::FLAG_PARAMEQ;
	}
	else if (pstParam->m_ucFlags & FLAG_REVERB)
	{
		m_pobAudioStream->GetFXInterface().SetEffects(FX_REVERB);
		m_uiFlags|=AUDIO_OBJECT_FLAGS::FLAG_REVERB;
	}
	else if (pstParam->m_ucFlags & FLAG_PARAMEQ)
	{
		m_pobAudioStream->GetFXInterface().SetEffects(FX_PARAMEQ);
		m_uiFlags|=AUDIO_OBJECT_FLAGS::FLAG_PARAMEQ;
	}

//	Util::EnableFPUZeroDivException();
	Util::EnableFPUOverflowException();
	Util::EnableFPUInvalidException();

#endif // _ENABLE_EFFECTS

	// Setup volume and pitch

	if (pstParam->m_ucFlags & FLAG_VOLUME_VARIATION)
		m_fBaseVolume=drandf(pstParam->m_fMaxVolume-pstParam->m_fMinVolume)+pstParam->m_fMinVolume;
	else
		m_fBaseVolume=pstParam->m_fVolume;
		
	if (pstParam->m_ucFlags & FLAG_PITCH_VARIATION)
		m_fBasePitch=drandf(pstParam->m_fMaxPitch-pstParam->m_fMinPitch)+pstParam->m_fMinPitch;
	else
		m_fBasePitch=pstParam->m_fPitch;

	// Setup 3d parameters

	m_pobTransform=NULL;

#ifndef _NO_DSOUND

	if (m_eSpatialization==SPATIALIZATION_HRTF)
	{
		if (pstParam->m_fMinDist!=DS3D_DEFAULTMINDISTANCE)
			m_pobAudioStream->SetMinDistance(pstParam->m_fMinDist);

		if (pstParam->m_fMaxDist!=DS3D_DEFAULTMAXDISTANCE)
			m_pobAudioStream->SetMaxDistance(pstParam->m_fMaxDist);
	}

#endif // _NO_DSOUND

	// Determine if this sound is supposed to loop

	if (pstParam->m_ucPlayCount==0)
		m_uiFlags|=AUDIO_OBJECT_FLAGS::FLAG_LOOPING;

	// This audio object is now initialised	
	m_eState=STATE_INITIALISED;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioObject::~CAudioObject
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAudioObject::~CAudioObject ()
{
	if (m_eState==STATE_FADING_IN || m_eState==STATE_PLAYING || m_eState==STATE_FADING_OUT)
	{
		m_pobAudioStream->Stop();
	}

	// Tidy this up.  Remove the lua message callback?
	if(m_pCallbackFunc)
	{
		m_pCallbackFunc(m_pCallbackData);
	}
	// Now dead, and should be removed. - JML
	/*else if(m_lobjFinishMsg.IsTable() && CEntityManager::Exists())
	{
		CEntity* pEnt = 0;
		m_lobjFinishMsg.Get("To", pEnt);
		if(pEnt)
		{
			CMessageHandler* pHandler = pEnt->GetMessageHandler();
			if(pHandler)
				pHandler->Receive(m_lobjFinishMsg);
		}

		m_lobjFinishMsg.AssignNil(CLuaGlobal::Get().State());
	} */
}

/***************************************************************************************************
*
*	FUNCTION		CAudioObject::Update
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioObject::Update ()
{
	if (!m_pobAudioStream)
	{
		ntPrintf("Warning: Attempting to update audio object with no valid stream!\n");
		return false;
	}
    
	if (!CAudioEngine::Get().IsPlaying(m_iSoundID)) // Check to see if the sound is still playing
	{
		SetFinished();
		m_pobAudioStream=0; // Audio stream is no longer valid
		return false;
	}

	float fVolume=m_fBaseVolume*m_fVolume*m_pobAudioLayer->GetVolume()*m_fFadeVolume;
	float fPitch=m_fBasePitch*m_fPitch*m_pobAudioLayer->GetPitch();

	switch(m_eState)
	{
		case STATE_WAITING:
		{
			if (CAudioManager::Get().GetTime()>m_fStartTime) // Its time to start playing
			{
				// This sound is positioned relative to a transform
				if (m_pobTransform)
				{
					CMatrix obTemp=m_pobTransform->GetWorldMatrix() * m_obOffsetMatrix;
					m_pobAudioStream->SetPosition(obTemp.GetTranslation());
				}

				if (m_fFadeInTime>0.0f) // We want to fade in
				{
					m_pobAudioStream->SetVolume(0.0f);

					m_fFadeVolume=0.0f;
					m_eState=STATE_FADING_IN;
				}
				else // Play immediately
				{
					m_pobAudioStream->SetVolume(fVolume);
					m_eState=STATE_PLAYING;
				}

				m_pobAudioStream->SetPitch(fPitch);
				m_pobAudioStream->Play(m_ucPlayCount);

				return true;
			}

			break;
		}

		case STATE_FADING_IN:
		{
			m_fFadeTime+=CTimer::Get().GetGameTimeChange();

			if (m_fFadeTime>m_fFadeInTime)
				m_fFadeTime=m_fFadeInTime;

			switch(m_eFadeInCurve)
			{
				case CURVE_LINEAR:
				{
					m_fFadeVolume=m_fFadeTime/m_fFadeInTime;
					break;
				}

				case CURVE_EXPONENTIAL:
				{
					float fTemp=m_fFadeTime/m_fFadeInTime;
					m_fFadeVolume=fTemp*fTemp;
					break;
				}

				case CURVE_LOGARITHMIC:
				{
					float fTemp=1.0f-(m_fFadeTime/m_fFadeInTime);
					m_fFadeVolume=1.0f-(fTemp*fTemp);
					break;
				}
			}

			if (m_fFadeVolume>=1.0f)
			{
				m_fFadeVolume=1.0f;
				m_eState=STATE_PLAYING;
			}

			//ntPrintf("Fading in: %f\n",m_fFadeVolume);

			break;
		}

		case STATE_PLAYING:
		{
			break;
		}

		case STATE_FADING_OUT:
		{
			m_fFadeTime+=CTimer::Get().GetGameTimeChange();

			if (m_fFadeTime>m_fFadeOutTime)
				m_fFadeTime=m_fFadeOutTime;

			switch(m_eFadeOutCurve)
			{
				case CURVE_LINEAR:
				{
					m_fFadeVolume=1.0f-(m_fFadeTime/m_fFadeOutTime);
					break;
				}

				case CURVE_EXPONENTIAL:
				{
					float fTemp=1.0f-(m_fFadeTime/m_fFadeOutTime);
					m_fFadeVolume=fTemp*fTemp;
					break;
				}

				case CURVE_LOGARITHMIC:
				{
					float fTemp=m_fFadeTime/m_fFadeOutTime;
					m_fFadeVolume=1.0f-(fTemp*fTemp);
					break;
				}
			}

			if (m_fFadeVolume<=0.0f)
			{
				m_fFadeVolume=0.0f;
				m_eState=STATE_PENDING_STOP;
				m_pobAudioStream->Stop();
			}

			//ntPrintf("Fading out: %f\n",m_fFadeVolume);

			break;
		}

		case STATE_PENDING_STOP:
		{
			break;
		}

		case STATE_FINISHED:
		{
			return false;

			break;
		}
	}

	if (!CAudioEngine::Get().IsPlaying(m_iSoundID)) // Check to see if the sound is still playing
	{
		SetFinished();
		m_pobAudioStream=0; // Audio stream is no longer valid
		return false;
	}

	// This sound is positioned relative to a transform
	if (m_pobTransform)
	{
		CMatrix obTemp=m_pobTransform->GetWorldMatrix() * m_obOffsetMatrix;
		m_obWorldPosition=obTemp.GetTranslation();

		if (m_eSpatialization==SPATIALIZATION_HRTF)
			m_pobAudioStream->SetPosition(obTemp.GetTranslation());
	}

	// Distance attenuation
	
	if (m_eSpatialization==SPATIALIZATION_ATTENUATED)
	{
		CDirection obDiff(m_obWorldPosition-CAudioManager::Get().GetListenerPosition());
		
		float fFactor=(m_fMaxDist-obDiff.Length())/(m_fMaxDist-m_fMinDist);

		if (fFactor<0.0f)
			fFactor=0.0f;
		if (fFactor>1.0f)
			fFactor=1.0f;

		fVolume*=fFactor;
	}

	// Update volume/pitch
	if (m_pobAudioStream->GetVolume()!=fVolume) // Set new volume if it has changed
		m_pobAudioStream->SetVolume(fVolume);

	if (m_pobAudioStream->GetPitch()!=fPitch)
		m_pobAudioStream->SetPitch(fPitch);
		


	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioObject::Play
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioObject::Play ()
{
	if (m_eState!=STATE_INITIALISED)
	{
		return;
	}

	if (m_fDelayTime>0.0f) // We want to have a delay before this audio object plays
	{
		m_fStartTime+=m_fDelayTime;
		m_eState=STATE_WAITING;
	}
	else if (m_fFadeInTime>0.0f) // We want to fade in
	{
		m_pobAudioStream->SetVolume(0.0f);
		m_pobAudioStream->SetPitch(m_fBasePitch*m_fPitch*m_pobAudioLayer->GetPitch());
		m_pobAudioStream->Play(m_ucPlayCount);

		m_fFadeVolume=0.0f;
		m_eState=STATE_FADING_IN;
	}
	else // Play immediately
	{
		m_pobAudioStream->SetVolume(m_fBaseVolume*m_fVolume*m_pobAudioLayer->GetVolume()*m_fFadeVolume);
		m_pobAudioStream->SetPitch(m_fBasePitch*m_fPitch*m_pobAudioLayer->GetPitch());
		m_pobAudioStream->Play(m_ucPlayCount);

		m_eState=STATE_PLAYING;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAudioObject::Stop
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioObject::Stop ()
{
	if (m_pobAudioStream)
	{
		switch(m_eState)
		{
			case STATE_WAITING:
			{
				SetFinished();
				break;
			}
		
			case STATE_FADING_IN:
			{
				m_eState=STATE_FADING_OUT;
				
				switch(m_eFadeOutCurve)
				{
					case CURVE_LINEAR:
					{
						m_fFadeTime=m_fFadeOutTime * (1.0f-m_fFadeVolume);
						break;
					}

					case CURVE_EXPONENTIAL:
					{
						m_fFadeTime=m_fFadeOutTime * (1.0f-fsqrtf(m_fFadeVolume));
						break;
					}

					case CURVE_LOGARITHMIC:
					{
						m_fFadeTime=m_fFadeOutTime * (fsqrtf(1.0f-m_fFadeVolume));
						break;
					}
				}

				break;
			}

			case STATE_PLAYING:
			{
				if (m_fFadeOutTime>0.0f) // We want to fade out
				{
					m_eState=STATE_FADING_OUT;
					m_fFadeTime=(1.0f-m_fFadeVolume)*m_fFadeOutTime;
				}
				else
				{
					m_pobAudioStream->Stop();
					m_eState=STATE_PENDING_STOP;
				}

				break;
			}
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAudioObject::Pause
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioObject::Pause ()
{
	if (m_pobAudioStream)
	{
		m_pobAudioStream->Pause();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAudioObject::Resume
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioObject::Resume ()
{
	if (m_pobAudioStream)
	{
		m_pobAudioStream->Resume();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAudioObject::SetPosition
*
*	DESCRIPTION		Set a static position for this sound.
*
***************************************************************************************************/

void CAudioObject::SetPosition (const CPoint& obPosition)
{
	m_obWorldPosition=obPosition;

	if (m_eSpatialization==SPATIALIZATION_HRTF && m_pobAudioStream)
	{
		m_pobAudioStream->Get3DInterface().SetPosition(obPosition,true);
		m_pobTransform=0;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAudioObject::SetPosition
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioObject::SetPosition (const Transform* pobTransform,const CPoint& obPosition)
{
	ntAssert_p(pobTransform,("CAudioObject: Invalid transform passed in to SetPosition\n"));

	m_pobTransform=pobTransform;
	m_obOffsetMatrix.SetIdentity();
	m_obOffsetMatrix.SetTranslation(obPosition);

	if (m_eSpatialization==SPATIALIZATION_HRTF)
	{
		CMatrix obTemp=m_pobTransform->GetWorldMatrix() * m_obOffsetMatrix;
		m_pobAudioStream->Get3DInterface().SetPosition(obTemp.GetTranslation(),true);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAudioObject::SetI3DL2Reverb
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioObject::SetI3DL2Reverb (AUDIO_EFFECT_I3DL2REVERB* pstI3DL2Reverb)
{
#ifdef _ENABLE_EFFECTS

	ntAssert(pstI3DL2Reverb);

	if (m_uiFlags & AUDIO_OBJECT_FLAGS::FLAG_REVERB)
	{
		m_pobAudioStream->GetFXInterface().SetReverb(pstI3DL2Reverb);
	}

#else

	UNUSED(pstI3DL2Reverb);

#endif // _ENABLE_EFFECTS
}

/***************************************************************************************************
*
*	FUNCTION		CAudioObject::SetParamEq
*
*	DESCRIPTION		Set a pointer to a position, which will be used to position this audio object.
*
***************************************************************************************************/

void CAudioObject::SetParamEq (AUDIO_EFFECT_PARAMEQ *pstParamEq)
{
#ifdef _ENABLE_EFFECTS

	ntAssert(pstParamEq);

	if (m_uiFlags & AUDIO_OBJECT_FLAGS::FLAG_PARAMEQ)
	{
		m_pobAudioStream->GetFXInterface().SetParamEq(pstParamEq);
	}

#else

	UNUSED(pstParamEq);

#endif // _ENABLE_EFFECTS
}

/***************************************************************************************************
*
*	FUNCTION		CAudioObject::SetName
*
*	DESCRIPTION		Used for debugging.
*
***************************************************************************************************/
void CAudioObject::SetName (const CKeyString& obSoundCue,const CKeyString& obSoundBank)
{
	m_obSoundCue=obSoundCue;
	m_obSoundBank=obSoundBank;
}


// Finished Callback Hack - JML Added 09-12-05
void CAudioObject::SetFinished()
{
	m_eState = STATE_FINISHED;
}



















/***************************************************************************************************
*
*	FUNCTION		CAudioManager::CAudioManager
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAudioManager::CAudioManager () :
	m_uiMaximum2dSounds(12),
	m_uiMaximum3dSounds(48),
	m_bDebugRender(false),
	m_iNextSoundBankToUpdate(0)
{
	m_obListenerMatrix.SetIdentity();

	if (CAudioEngine::Get().GetDirectSoundP())
	{
		m_bEnabled=true;
	}
	else
	{
		ntPrintf("CAudioManager: Disabling audio manager, no DirectSound interface available\n");
		m_bEnabled=false;
	}

	// Obtain a reference to the global lua state
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	// Create the global Audio Events table. 
	NinjaLua::LuaObject obTable = NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State());
	obGlobals.Set("AudioEvents", obTable);

	// Set reverb defaults
	
#ifndef _NO_DSOUND
	m_stI3DL2Reverb.lRoom=DSFX_I3DL2REVERB_ROOM_DEFAULT;
	m_stI3DL2Reverb.lRoomHF=DSFX_I3DL2REVERB_ROOMHF_DEFAULT;
	m_stI3DL2Reverb.fRoomRolloffFactor=DSFX_I3DL2REVERB_ROOMROLLOFFFACTOR_DEFAULT;
	m_stI3DL2Reverb.fDecayTime=DSFX_I3DL2REVERB_DECAYTIME_DEFAULT;
	m_stI3DL2Reverb.fDecayHFRatio=DSFX_I3DL2REVERB_DECAYHFRATIO_DEFAULT;
	m_stI3DL2Reverb.lReflections=DSFX_I3DL2REVERB_REFLECTIONS_DEFAULT;
	m_stI3DL2Reverb.fReflectionsDelay=DSFX_I3DL2REVERB_REFLECTIONSDELAY_DEFAULT;
	m_stI3DL2Reverb.lReverb=DSFX_I3DL2REVERB_REVERB_DEFAULT;
	m_stI3DL2Reverb.fReverbDelay=DSFX_I3DL2REVERB_REVERBDELAY_DEFAULT;
	m_stI3DL2Reverb.fDiffusion=DSFX_I3DL2REVERB_DIFFUSION_DEFAULT;
	m_stI3DL2Reverb.fDensity=DSFX_I3DL2REVERB_DENSITY_MAX;
	m_stI3DL2Reverb.fHFReference=DSFX_I3DL2REVERB_HFREFERENCE_DEFAULT;
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::~CAudioManager
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAudioManager::~CAudioManager ()
{
	ReleaseAll();
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetConfig
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioManager::SetConfig (u_int uiMax2dSounds,u_int uiMax3dSounds)
{
	m_uiMaximum2dSounds=uiMax2dSounds;
	m_uiMaximum3dSounds=uiMax3dSounds;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::RegisterSoundBank
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::RegisterSoundBank (const char* pcPath)
{
	CSoundBank* pobSoundBank=NT_NEW CSoundBank();


	if (!pobSoundBank->Open(pcPath))
	{
		NT_DELETE( pobSoundBank );
		return false;
	}

	m_obSoundBankList.push_back(pobSoundBank);

	ntPrintf("CAudioManager: Registering sound bank %s (%d cues)\n",pcPath,pobSoundBank->GetTotalCues());

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::ReleaseSoundBank
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::ReleaseSoundBank (const char* pcFriendlyName)
{
	CHashedString obID(pcFriendlyName);
	const u_int uiID=obID.GetValue();

	for(ntstd::List<CSoundBank*>::iterator obSoundBankIt=m_obSoundBankList.begin();
		obSoundBankIt!=m_obSoundBankList.end();
		++obSoundBankIt)
	{
		if ((*obSoundBankIt)->GetID()==uiID)
		{
			NT_DELETE( *obSoundBankIt );
			obSoundBankIt=m_obSoundBankList.erase(obSoundBankIt);
			return true;
		}
	}

	ntPrintf("CAudioManager: Unable to release %s, soundbank not found\n",pcFriendlyName);
	
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::ReleaseAll
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioManager::ReleaseAll ()
{
	while(!m_obAudioObjectList.empty())
	{
		NT_DELETE( m_obAudioObjectList.back() );
		m_obAudioObjectList.pop_back();
	}

	while(!m_obSoundBankList.empty())
	{
		NT_DELETE( m_obSoundBankList.back() );
		m_obSoundBankList.pop_back();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::FindSoundBankP
*
*	DESCRIPTION		
*
***************************************************************************************************/

CSoundBank* CAudioManager::FindSoundBankP (u_int uiID)
{
	for(ntstd::List<CSoundBank*>::iterator obSoundBankIt=m_obSoundBankList.begin();
		obSoundBankIt!=m_obSoundBankList.end();
		++obSoundBankIt)
	{
		if ((*obSoundBankIt)->GetID()==uiID)
			return *obSoundBankIt;
	}

	return NULL; // Unable to find this soundbank
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetListener
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioManager::SetListener (const CMatrix& obMatrix)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return;

	const CPoint& obPosition=obMatrix.GetTranslation();
	
	CAudioEngine::Get().SetListenerPosition(obPosition.X(),obPosition.Y(),obPosition.Z());

	CDirection obForward=CDirection(0.0f,0.0f,1.0f) * obMatrix;

	CDirection obUp=CDirection(0.0f,1.0f,0.0f) * obMatrix;

	CAudioEngine::Get().SetListenerOrientation(
		obForward.X(),obForward.Y(),obForward.Z(),
		obUp.X(),obUp.Y(),obUp.Z());

	m_obListenerMatrix=obMatrix;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetListener
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioManager::SetI3DL2Reverb (CI3DL2Reverb* pobI3DL2Reverb)
{
	ntAssert(pobI3DL2Reverb);

	m_pobI3DL2Reverb=pobI3DL2Reverb;

	m_pobI3DL2Reverb->Set(m_stI3DL2Reverb);
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::Update
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioManager::Update (float fTime)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return;

	m_fTime=fTime;

	CAudioEngine::Get().DoWork();

	for(ntstd::List<CAudioObject*>::iterator obObjectIt=m_obAudioObjectList.begin();
		obObjectIt!=m_obAudioObjectList.end(); )
	{
		if (!(*obObjectIt)->Update()) // Destroy audio objects which have expired
		{
			#ifdef _SHOW_DEBUG_MESSAGES
			ntPrintf("CAudioManager: Destroying audio object #%d\n",(*obObjectIt)->GetID());
			#endif

			NT_DELETE( *obObjectIt );
			obObjectIt=m_obAudioObjectList.erase(obObjectIt);
		}
		else
		{
			++obObjectIt;
		}
	}

#ifndef _RELEASE // Debugging features (disabled on release)

	CInputKeyboard &obKeyboard=CInputHardware::Get().GetKeyboard();

	if (obKeyboard.IsKeyPressed(KEYC_A,KEYM_ALT))
		m_bDebugRender=!m_bDebugRender;

	if (CGameShell::Get().GetOptions().GetLiveAudioEditing())
	{
		u_int iSoundBankIndex=0;

		for(ntstd::List<CSoundBank*>::iterator obIt=m_obSoundBankList.begin(); obIt!=m_obSoundBankList.end(); ++obIt)
		{
			if (iSoundBankIndex==m_iNextSoundBankToUpdate) // We only ever update one sound bank per frame
			{
				if ((*obIt)->Reload())
				{
					ntPrintf("CAudioManager: Reloading sound bank %s\n",(*obIt)->GetPath());
				}

				break;
			}

			++iSoundBankIndex;
		}

		++m_iNextSoundBankToUpdate;

		if (m_iNextSoundBankToUpdate>=m_obSoundBankList.size())
			m_iNextSoundBankToUpdate=0;		
	}

	if (m_bDebugRender)
	{
		const unsigned long ulCOLOUR = 0x99ffff77;

		float fX=10.0f;
		float fY=10.0f;
		const float fY_SPACING=11.0f;

		g_VisualDebug->Printf2D(fX,fY, ulCOLOUR, 0, "Audio Console - Live editing %s",
			(CGameShell::Get().GetOptions().GetLiveAudioEditing() ? "enabled" : "disabled"));

		fY+=fY_SPACING * 2.0f;

		g_VisualDebug->Printf2D(fX,fY, ulCOLOUR, 0, "Cue (%d)",m_obAudioObjectList.size());
		g_VisualDebug->Printf2D(fX+360.0f,fY, ulCOLOUR, 0, "Volume Pitch 3D");
		fY+=fY_SPACING;

		for(ntstd::List<CAudioObject*>::iterator obIt=m_obAudioObjectList.begin(); obIt!=m_obAudioObjectList.end(); ++obIt )
		{
			g_VisualDebug->Printf2D(fX,fY, ulCOLOUR, 0, "%s:%s",
				ntStr::GetString((*obIt)->GetSoundCueName()),
				ntStr::GetString((*obIt)->GetSoundBankName()));

			g_VisualDebug->Printf2D(fX+360.0f,fY, ulCOLOUR, 0, "%.2f   %.2f  %c",
				(*obIt)->GetActualVolume(),
				(*obIt)->GetActualPitch(),
				((*obIt)->Is3d() ? 'Y' : 'N'));

			fY+=fY_SPACING;
		}
	}

#endif // _RELEASE
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::Create
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAudioManager::Create (CAudioObjectID& obID,const char* pcString,SPATIALIZATION eSpatialType)
{
	// Extract sound bank & sound cue from the string
	char acSoundBank [32];
	char acSoundCue [32];

	int iOffset=0;
	int iStrLen=strlen(pcString);

	while(pcString[iOffset]!=':' && iOffset<iStrLen)
	{
		++iOffset;
	}

	if (iOffset==iStrLen)
	{
		#ifdef _SHOW_WARNING_MESSAGES
		ntPrintf("Warning - %s is not a valid sound cue",pcString);
		#endif // _SHOW_WARNING_MESSAGES

		return false;
	}

	strncpy(acSoundBank,pcString,iOffset);
	acSoundBank[iOffset]='\0';

	++iOffset;

	strncpy(acSoundCue,pcString+iOffset,iStrLen-iOffset);
	acSoundCue[iStrLen-iOffset]='\0';

	//ntPrintf("Playing %s (%s)\n",acSoundCue,acSoundBank);

	CKeyString obBank(acSoundBank);
	CKeyString obCue(acSoundCue);

	return Create(obID,obBank,obCue,eSpatialType);
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::Create
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::Create (CAudioObjectID& obID,const CKeyString& obBank,const CKeyString& obCue,SPATIALIZATION eSpatialType)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
	{
		obID.Invalidate();
		return false;
	}

	CSoundBank *pobSoundBank=FindSoundBankP(obBank.GetHash());

	if (!pobSoundBank)
	{
		#ifdef _SHOW_WARNING_MESSAGES
		if (!obBank.IsNull())
			ntPrintf("Failed to play sound cue %s (%s), invalid sound bank\n", ntStr::GetString(obCue),ntStr::GetString(obBank));
		#endif // _SHOW_WARNING_MESSAGES
		obID.Invalidate();
		return false;
	}

	CAudioCue* pobCue=pobSoundBank->FindAudioCueP(obCue.GetHash());
	
	if (!pobCue)
	{
		#ifdef _SHOW_WARNING_MESSAGES
		if (!obCue.IsNull())
			ntPrintf("Failed to play sound cue %s (%s), invalid sound cue\n", ntStr::GetString(obCue),ntStr::GetString(obBank));
		#endif // _SHOW_WARNING_MESSAGES
		obID.Invalidate();
		return false;
	}
	
	// Create our audio object and add it to our list

	if (CanPlayAudioCue(pobCue,eSpatialType!=SPATIALIZATION_NONE ? true : false)) // Check to see if its okay to play this audio cue
	{
		CAudioLayer* pobLayer=CAudioLayerManager::Get().FindAudioLayer(pobCue->GetParametersP()->m_uiCategoryID);
		
		if (!pobLayer)
		{
			ntAssert(pobLayer);

			#ifdef _SHOW_WARNING_MESSAGES
			ntPrintf("Failed to play sound cue %s (%s), invalid layer\n", ntStr::GetString(obCue), ntStr::GetString(obBank));
			#endif // _SHOW_WARNING_MESSAGES
			obID.Invalidate();
			return false;
		}

		// Generate a new ID

		++m_obGeneratedID.m_iID;

		if (!m_obGeneratedID.m_iID) // ID is never zero
			++m_obGeneratedID.m_iID;

		// Create the audio object

		CAudioObject* pobAudioObject=NT_NEW CAudioObject(m_obGeneratedID,pobCue,pobLayer,eSpatialType);

		if (!pobAudioObject->IsInitialised())
		{
			NT_DELETE( pobAudioObject );

			obID.Invalidate();

			#ifdef _SHOW_WARNING_MESSAGES
			ntPrintf("Failed to play sound cue %s (%s), unable to create stream\n", ntStr::GetString(obCue), ntStr::GetString(obBank));
			#endif // _SHOW_WARNING_MESSAGES

			return false;
		}

#ifndef _RELEASE

		pobAudioObject->SetName(obCue,obBank);

#endif // _RELEASE

		// Set the reverb if this sound is 3d

		pobAudioObject->SetI3DL2Reverb(&m_stI3DL2Reverb);

		m_obAudioObjectList.push_back(pobAudioObject);

		obID=m_obGeneratedID;

		#ifdef _SHOW_DEBUG_MESSAGES
		ntPrintf("CAudioManager: Creating audio object #%d (%s, %s)\n",m_obGeneratedID.m_iID,*obBank,*obCue);
		#endif // _SHOW_DEBUG_MESSAGES

		return true;
	}
//	else
//	{
//		#ifdef _SHOW_WARNING_MESSAGES
//		ntPrintf("CAudioManager: Failed to play sound cue %s, CanPlayAudioCue returned false\n",*obCue);
//		#endif // _SHOW_WARNING_MESSAGES
//	}

	// We have failed to play this sound cue
	
	obID.Invalidate();

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::Play
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::Play (const CAudioObjectID& obID)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	CAudioObject* pobAudioObject=GetAudioObjectP(obID);

	if (pobAudioObject)
	{
		pobAudioObject->Play();
		return true;
	}
	
	return false;
}

// Finished Callback  - JML Added 09-12-05
bool CAudioManager::SetCallback(const CAudioObjectID& obID, NinjaLua::LuaObject& pFinishMsg)
{
	CAudioObject* pobAudioObject=GetAudioObjectP(obID);

	if (pobAudioObject)
	{
		pobAudioObject->SetFinishCallBack(pFinishMsg);
		return true;
	}
	
	return false;
}


// Migrated to C++
bool CAudioManager::SetCallback(const CAudioObjectID& obID, AudioCallback pFunc, void* pData)
{
	CAudioObject* pobAudioObject=GetAudioObjectP(obID);

	if (pobAudioObject)
	{
		pobAudioObject->SetFinishCallBack(pFunc, pData);
		return true;
	}
	
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::Stop
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::Stop (const CAudioObjectID& obID)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	CAudioObject* pobAudioObject=GetAudioObjectP(obID);

	if (pobAudioObject)
	{
		pobAudioObject->Stop();
		return true;
	}

	#ifdef _SHOW_DEBUG_MESSAGES
	ntPrintf("CAudioManager: Failed to stop audio object #%d, since it does not exist\n",obID.m_iID);
	#endif

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetVolume
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::SetVolume (const CAudioObjectID& obID,float fVolume)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	CAudioObject* pobAudioObject=GetAudioObjectP(obID);

	if (pobAudioObject)
	{
		pobAudioObject->SetVolume(fVolume);
		return true;
	}
	
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetPitch
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::SetPitch (const CAudioObjectID& obID,float fPitch)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	CAudioObject* pobAudioObject=GetAudioObjectP(obID);

	if (pobAudioObject)
	{
		pobAudioObject->SetPitch(fPitch);
		return true;
	}
	
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetPosition
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::SetPosition (const CAudioObjectID& obID,const CPoint& obPosition)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	CAudioObject* pobAudioObject=GetAudioObjectP(obID);

	if (pobAudioObject)
	{
		pobAudioObject->SetPosition(obPosition);
		return true;
	}
	
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetPosition
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::SetPosition (const CAudioObjectID& obID,const Transform* pobTransform,const CPoint& obPosition)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	CAudioObject* pobAudioObject=GetAudioObjectP(obID);

	if (pobAudioObject)
	{
		pobAudioObject->SetPosition(pobTransform,obPosition);
		return true;
	}
	
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetVolumeControl
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::SetVolumeControl (const CAudioObjectID& obID,const char* pcBank,const char* pcParamName,float fValue)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	CHashedString obTemp(pcBank);

	CSoundBank *pobSoundBank=FindSoundBankP(obTemp.GetValue());

	if (pobSoundBank)
	{
		float fVolume;
		
		if (pobSoundBank->GetVolumeControl(pcParamName,fValue,fVolume))
		{
			CAudioObject* pobAudioObject=GetAudioObjectP(obID);

			if (pobAudioObject)
			{
				pobAudioObject->SetVolume(fVolume);
				return true;
			}
			else
			{
				ntPrintf("CAudioManager: Specified audio object does not exist\n");
			}
		}
		else
		{
			ntPrintf("CAudioManager: Unable to find volume control %s in sound bank %s\n",pcParamName,pcBank);
		}
	}
	else
	{
		ntPrintf("CAudioManager: Unable to find sound bank %s\n",pcBank);
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetPitchControl
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::SetPitchControl (const CAudioObjectID& obID,const char* pcBank,const char* pcParamName,float fValue)
{
 	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	CHashedString obTemp(pcBank);

	CSoundBank *pobSoundBank=FindSoundBankP(obTemp.GetValue());

	if (pobSoundBank)
	{
		float fPitch;
		
		if (pobSoundBank->GetPitchControl(pcParamName,fValue,fPitch))
		{
			CAudioObject* pobAudioObject=GetAudioObjectP(obID);

			if (pobAudioObject)
			{
				pobAudioObject->SetPitch(fPitch);
				return true;
			}
			else
			{
				ntPrintf("CAudioManager: Specified audio object does not exist\n");
			}
		}
		else
		{
			ntPrintf("CAudioManager: Unable to find pitch control %s in sound bank %s\n",pcParamName,pcBank);
		}
	}
	else
	{
		ntPrintf("CAudioManager: Unable to find sound bank %s\n",pcBank);
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetPitchControl
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::SetParamEqControl (const CAudioObjectID& obID,const char* pcBank,const char* pcParamName,float fValue)
{
 	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	CHashedString obTemp(pcBank);

	CSoundBank *pobSoundBank=FindSoundBankP(obTemp.GetValue());

	if (pobSoundBank)
	{
		AUDIO_EFFECT_PARAMEQ stParam;
		
		if (pobSoundBank->GetParamEqControl(pcParamName,fValue,&stParam))
		{
			CAudioObject* pobAudioObject=GetAudioObjectP(obID);

			if (pobAudioObject)
			{
				pobAudioObject->SetParamEq(&stParam);
				return true;
			}
			else
			{
				ntPrintf("CAudioManager: Specified audio object does not exist\n");
			}
		}
		else
		{
			ntPrintf("CAudioManager: Unable to find pitch control %s in sound bank %s\n",pcParamName,pcBank);
		}
	}
	else
	{
		ntPrintf("CAudioManager: Unable to find sound bank %s\n",pcBank);
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetDelay
*
*	DESCRIPTION		Temporary until delay variation is implemented. Lets you override the
*					delay defined by the sound cue.
*
***************************************************************************************************/

bool CAudioManager::SetDelay (const CAudioObjectID& obID,float fTime) 
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	CAudioObject* pobAudioObject=GetAudioObjectP(obID);

	if (pobAudioObject)
	{
		pobAudioObject->SetDelay(fTime);
		return true;
	}
	
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::SetCategory
*
*	DESCRIPTION		Temporary until delay variation is implemented. Lets you override the
*					delay defined by the sound cue.
*
***************************************************************************************************/
bool CAudioManager::SetCategory (const CAudioObjectID& obID,const char* pcCategory)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	CAudioObject* pobAudioObject=GetAudioObjectP(obID);

	if (pobAudioObject)
	{
		unsigned int uiID= ntStr::GetHashKey(CHashedString(pcCategory));

		if (CAudioLayerManager::Get().FindAudioLayer(uiID)) // Check to see if this is a valid category
		{
			pobAudioObject->SetCategory(uiID);
		}

		return true;
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::StopAll
*
*	DESCRIPTION		Note, this will actually signal all sounds to stop AND remove the audio
*					objects.
*
***************************************************************************************************/

void CAudioManager::StopAll ()
{
	while(!m_obAudioObjectList.empty())
	{
		m_obAudioObjectList.back()->Stop();
		NT_DELETE( m_obAudioObjectList.back() );
		m_obAudioObjectList.pop_back();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::IsPlaying
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioManager::IsPlaying (const CAudioObjectID& obID)
{
	if (GetAudioObjectP(obID))
		return true;

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::GetAudioObjectP
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAudioObject* CAudioManager::GetAudioObjectP (const CAudioObjectID& obID)
{
	for(ntstd::List<CAudioObject*>::iterator obObjectIt=m_obAudioObjectList.begin();
		obObjectIt!=m_obAudioObjectList.end();
		++obObjectIt)
	{
		if ((*obObjectIt)->GetID()==obID)
			return (*obObjectIt);
	}

	return NULL;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::CanPlayAudioCue
*
*	DESCRIPTION		This deals with the advance resource management of play requests.
*
***************************************************************************************************/

bool CAudioManager::CanPlayAudioCue (CAudioCue* pobCue,bool b3d)
{
	if (!m_bEnabled) // Ignore if AudioManager is disabled
		return false;

	ntAssert(pobCue);

	// Check to see if we have reached audio engine limit

	if (CAudioEngine::Get().GetActiveStreams()==CAudioEngine::Get().GetMaxStreams())
		return false;

	// Check concurrent parameters

	AUDIO_CUE_PARAMETERS *pstParam=pobCue->GetParametersP();
	const float fStartTime=GetTime()+pstParam->m_fDelayTime; // Determine the projected start time of this cue
	const u_char ucMaxConcurrentStart=pstParam->m_ucMaxConcurrentStart;
	const u_char ucMaxConcurrentPlaying=pstParam->m_ucMaxConcurrentPlaying;

	// Examine whats currently playing

	u_int uiTotalObjects=0;
	u_short usPriority=pstParam->m_usPriority;
	CAudioObject* pobFirstInstance=NULL;
	CAudioObject* pobLowestPriorityInstance=NULL;
	u_char ucConcurrentPlaying=0;
	u_char ucConcurrentStarts=0;

	for(ntstd::List<CAudioObject*>::iterator obObjectIt=m_obAudioObjectList.begin(); // Iterate through the audio object list
		obObjectIt!=m_obAudioObjectList.end();
		++obObjectIt)
	{
		if (pstParam->m_uiID==(*obObjectIt)->GetCueID()) // This is the same cue
		{
			++ucConcurrentPlaying;

			if (fStartTime==(*obObjectIt)->GetStartTime()) // This cue has the same start time
				++ucConcurrentStarts;

			if (!pobFirstInstance)
				pobFirstInstance=(*obObjectIt); // Make a note of the first instance of a matching cue
		}

		if ((*obObjectIt)->Is3d()==b3d && !(*obObjectIt)->IsFinished()) // This object is also 2D/3D
		{
			++uiTotalObjects; // Keep track of how many are 2D/3D

			if ((*obObjectIt)->GetPriority()<usPriority) // Keep track of the lowest priority object
			{
				usPriority=(*obObjectIt)->GetPriority();
				pobLowestPriorityInstance=(*obObjectIt);
			}
		}
	}

	// Check concurrent audio cues

	if (ucMaxConcurrentStart>0 && ucConcurrentStarts>=ucMaxConcurrentStart) // Check to see if we have exceeded the limit of cues that are scheduled to start at the same time
	{
		return false;
	}

	if (ucMaxConcurrentPlaying>0 && ucConcurrentPlaying>=ucMaxConcurrentPlaying) // Check to see if we have exceeded the maximum number that can play simultaneously
	{
		// Note: Commented this bit out since it's not working for some reason :(
		//pobFirstInstance->Stop(); // Stop the first matching cue that was playing, so that this new cue can replace it
		//return true;
		return false;
	}

	// Check to see if we have reached our limit of 2D/3D sounds
	
	if ((b3d && m_uiMaximum3dSounds>0 && uiTotalObjects>=m_uiMaximum3dSounds) ||
		(!b3d && m_uiMaximum2dSounds>0 && uiTotalObjects>=m_uiMaximum2dSounds))
	{
		if (pobLowestPriorityInstance) // We have found an object with a lower priority, terminate it and accept request
		{
			pobLowestPriorityInstance->Stop();

			return true;
		}
		else // No sound of a lower priority was found, therefore deny request to play this cue
		{
			return false;
		}
	}

	//ntPrintf("ucConcurrentStarts=%d ucConcurrentPlaying=%d\n",ucConcurrentStarts,ucConcurrentPlaying);

	// We haven't reached any limits, its safe to play

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CAudioManager::Verify
*
*	DESCRIPTION		Verify whether a particular sound cue exists.
*
***************************************************************************************************/

bool CAudioManager::Verify (const CHashedString& obBank,const CHashedString& obCue)
{
	CSoundBank* pobBank=FindSoundBankP(ntStr::GetHashKey(obBank));

	if (pobBank)
	{
		if (pobBank->FindAudioCueP(ntStr::GetHashKey(obCue)))
			return true;
	}

	return false;
}



void CAudioManager::PlayDebugSound(int i)
{
	CAudioObjectID obID;
	switch(i)
	{
		case 1:
		{
			CAudioManager::Get().Create( obID, "miscellaneous_sb:frank1", SPATIALIZATION_NONE );
			break;
		}
		case 2:
		{
			CAudioManager::Get().Create( obID, "miscellaneous_sb:frank2", SPATIALIZATION_NONE );
			break;
		}
		default:
		{
			CAudioManager::Get().Create( obID, "miscellaneous_sb:frank3", SPATIALIZATION_NONE );
			break;
		}
	}
	CAudioManager::Get().Play( obID );	
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::GlobalTrigger
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioManager::GlobalTrigger( const char* pcEvent )
{
	// Obtain a reference to the global lua state
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	// Get a reference to the audio event table
	NinjaLua::LuaObject& obAudioEvents = obGlobals["AudioEvents"];
	
	// If the audio events table isn't valid, then ntAssert
	ntAssert( !obAudioEvents.IsNil() );

	// Is there a function for the event?
	NinjaLua::LuaObject& obAudioEvent = obAudioEvents[pcEvent];

	// If the audio event isn't a function, then return now
	if( !obAudioEvent.IsFunction() )
		return;

	// Typecast the audio event to a function
	NinjaLua::LuaFunction obFunction(obAudioEvent);

	// Call the bound function function. 
	obFunction( pcEvent );
}

/***************************************************************************************************
*
*	FUNCTION		CAudioManager::GlobalTriggerF
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioManager::GlobalTriggerF( const char* pcFormat, ... )
{
	va_list args;
	va_start( args, pcFormat );

	// Find the length of the string
	int iLen = _vscprintf( pcFormat, args ) + 1; // terminating '\0'

	// Allocate stack space for the string
	char *pcStringBuffer = (char*) _alloca( iLen );
	
	// Make the string		
	vsprintf( pcStringBuffer, pcFormat, args );

	// Call the tigger
	GlobalTrigger( pcStringBuffer );
}