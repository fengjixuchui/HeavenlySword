//------------------------------------------------------------------------------------------
//!
//!	\file animevents.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "anim/animevents.h"

#include "physics/system.h"
#include "objectdatabase/dataobject.h"
#include "anim/animator.h"
#include "anim/animation.h"
#include "core/visualdebugger.h"

// TODO Deano we need to decouple the anim events system from the 
// code that gets called, sounds like a job for a registeration API. Otherwise this has 
// to moved into game as it depends on everything!
#include "game/randmanager.h"			//! TODO Deano anim depends on game
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entitymanager.h"
#include "game/query.h"
#include "game/attacks.h"
#include "game/syncdcombat.h"
#include "audio/gameaudiocomponents.h"
#include "audio/audiomixer.h"

#include "game/luaattrtable.h" // Script events
#include "game/renderablecomponent.h"
#include "effect/effect_trigger.h"
#include "effect/effect_manager.h"
#include "effect/effect_error.h"
#include "effect/combateffect.h"
#include "game/messagehandler.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "blendshapes/anim/bsanimator.h"
#include "blendshapes/anim/blendshapes_anim_export.h"
#include "blendshapes/blendshapescomponent.h"

#include "game/chatterboxman.h"

START_CHUNKED_INTERFACE	(CAnimEventList, Mem::MC_ANIMATION )
	PUBLISH_PTR_CONTAINER_AS(m_obAnimEventList, AnimEventList)
#ifndef _RELEASE
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
#endif
	DECLARE_DELETE_CALLBACK( OnDelete )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(CAnimEventScript, Mem::MC_ANIMATION)
	IFLOAT		(CAnimEventScript, Offset)
	IFLOAT		(CAnimEventScript, BlendThreshold)
#ifdef _DEBUG_DISABLE_EVENT
	IBOOL		(CAnimEventScript, DebugDisable)	
#endif // _DEBUG_DISABLE_EVENT
	ISTRING		(CAnimEventScript, Function)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_DELETE_CALLBACK( OnDelete )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(CAnimEventMessage, Mem::MC_ANIMATION)
	IFLOAT		(CAnimEventMessage, Offset)
	IFLOAT		(CAnimEventMessage, BlendThreshold)
#ifdef _DEBUG_DISABLE_EVENT
	IBOOL		(CAnimEventMessage, DebugDisable)	
#endif // _DEBUG_DISABLE_EVENT
	ISTRING		(CAnimEventMessage, Message)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_DELETE_CALLBACK( OnDelete )
END_STD_INTERFACE

START_CHUNKED_INTERFACE (CAnimEventDemonAnimMessage, Mem::MC_ANIMATION)
	IFLOAT		(CAnimEventDemonAnimMessage, Offset)
	IFLOAT		(CAnimEventDemonAnimMessage, BlendThreshold)
	ISTRING		(CAnimEventDemonAnimMessage, AnimName)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_DELETE_CALLBACK( OnDelete )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(CAnimEventCameraShake, Mem::MC_ANIMATION)
	IFLOAT		(CAnimEventCameraShake, Offset)
	IFLOAT		(CAnimEventCameraShake, BlendThreshold)
#ifdef _DEBUG_DISABLE_EVENT
	IBOOL		(CAnimEventCameraShake, DebugDisable)	
#endif // _DEBUG_DISABLE_EVENT
	IFLOAT		(CAnimEventCameraShake, Time)
	IFLOAT		(CAnimEventCameraShake, Speed)
	IFLOAT		(CAnimEventCameraShake, Intensity)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_DELETE_CALLBACK( OnDelete )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(AnimEventEffect, Mem::MC_ANIMATION)
	IFLOAT		(AnimEventEffect, Offset)
	IFLOAT		(AnimEventEffect, BlendThreshold)
#ifdef _DEBUG_DISABLE_EVENT
	IBOOL		(AnimEventEffect, DebugDisable)	
#endif // _DEBUG_DISABLE_EVENT
	IBOOL		(AnimEventEffect, AutoDestruct)
	IREFERENCE	(AnimEventEffect, TriggerDef)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_DELETE_CALLBACK( OnDelete )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(CAnimEventSound, Mem::MC_ANIMATION)
	IFLOAT		(CAnimEventSound, Offset)
	IFLOAT		(CAnimEventSound, BlendThreshold)
#ifdef _DEBUG_DISABLE_EVENT
	IBOOL		(CAnimEventSound, DebugDisable)	
#endif // _DEBUG_DISABLE_EVENT
	ISTRING		(CAnimEventSound, ClumpName)
	IENUM		(CAnimEventSound, Channel, ENTITY_AUDIO_CHANNEL)
	ISTRING		(CAnimEventSound, SoundBank)
	ISTRING		(CAnimEventSound, SoundCue)
	IFLOAT		(CAnimEventSound, Volume)
	IFLOAT		(CAnimEventSound, Pitch)
	IFLOAT		(CAnimEventSound, Probability)
	IBOOL		(CAnimEventSound, StopSound)
	IBOOL		(CAnimEventSound, SendToChatterBox)
	IBOOL		(CAnimEventSound, HasSubtitles)
		
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_DELETE_CALLBACK( OnDelete )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(CAnimEventBSAnimPlay, Mem::MC_ANIMATION)
	IFLOAT		(CAnimEventBSAnimPlay, Offset)
	IFLOAT		(CAnimEventBSAnimPlay, BlendThreshold)
#ifdef _DEBUG_DISABLE_EVENT
	IBOOL		(CAnimEventBSAnimPlay, DebugDisable)	
#endif // _DEBUG_DISABLE_EVENT
	ISTRING		(CAnimEventBSAnimPlay, ShortName)
	IBOOL		(CAnimEventBSAnimPlay, HoldLastKeyframe)
	IBOOL		(CAnimEventBSAnimPlay, Looping)
	IFLOAT		(CAnimEventBSAnimPlay, Speed)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_DELETE_CALLBACK( OnDelete )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(CAnimEventBSAnimStop, Mem::MC_ANIMATION)
	IFLOAT		(CAnimEventBSAnimStop, Offset)
	IFLOAT		(CAnimEventBSAnimStop, BlendThreshold)
#ifdef _DEBUG_DISABLE_EVENT
	IBOOL		(CAnimEventBSAnimStop, DebugDisable)	
#endif // _DEBUG_DISABLE_EVENT
	ISTRING		(CAnimEventBSAnimStop, ShortName)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_DELETE_CALLBACK( OnDelete )
END_STD_INTERFACE

START_CHUNKED_INTERFACE (CAnimEventAudioChannelStop, Mem::MC_ANIMATION)
	IFLOAT		(CAnimEventAudioChannelStop,	Offset)
	IFLOAT		(CAnimEventAudioChannelStop,	BlendThreshold)
#ifdef _DEBUG_DISABLE_EVENT
	IBOOL		(CAnimEventAudioChannelStop,	DebugDisable)	
#endif // _DEBUG_DISABLE_EVENT
	IENUM		(CAnimEventAudioChannelStop,	Channel, ENTITY_AUDIO_CHANNEL)
	DECLARE_DELETE_CALLBACK( OnDelete )
END_STD_INTERFACE

START_CHUNKED_INTERFACE (CAnimEventFootstep, Mem::MC_ANIMATION)
	IFLOAT		(CAnimEventFootstep,			Offset)
	IFLOAT		(CAnimEventFootstep,			BlendThreshold)
	IFLOAT		(CAnimEventFootstep,			Volume)
	IENUM		(CAnimEventFootstep,			Footstep,	FOOTSTEP)
#ifdef _DEBUG_DISABLE_EVENT
	IBOOL		(CAnimEventFootstep,			DebugDisable)	
#endif // _DEBUG_DISABLE_EVENT	
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(AnimEventMixerProfile, Mem::MC_ANIMATION)
	IFLOAT		(AnimEventMixerProfile, Offset)
	IFLOAT		(AnimEventMixerProfile, BlendThreshold)
#ifdef _DEBUG_DISABLE_EVENT
	IBOOL		(AnimEventMixerProfile, DebugDisable)	
#endif // _DEBUG_DISABLE_EVENT
	ISTRING		(AnimEventMixerProfile, Profile)
	IFLOAT		(AnimEventMixerProfile, TransitionDuration)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_DELETE_CALLBACK( OnDelete )
END_STD_INTERFACE


START_CHUNKED_INTERFACE	(AnimEventCombatAttack, Mem::MC_ANIMATION)
	PUBLISH_VAR_AS( m_fOffset, Offset )
	PUBLISH_VAR_AS( m_fBlendThreshold, BlendThreshold )
	PUBLISH_PTR_AS( m_pAttackData, AttackData )
	PUBLISH_VAR_AS( m_bUseStrike,  UseStrike )
	PUBLISH_VAR_AS( m_bUseWake,    UseWake )
	PUBLISH_VAR_AS( m_fRadius,	   Radius )
	PUBLISH_VAR_AS( m_fDebrisImpulse, DebrisImpulse )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	CAnimEvent::PostConstruct
//!	Make sure that we have some reasonable values here
//!
//------------------------------------------------------------------------------------------
void CAnimEvent::PostConstruct( void )
{
#ifndef _RELEASE

	// Sort out the offset values
	if ( m_fOffset < EPSILON )
		m_fOffset = 0.0f;
	else if ( m_fOffset > 1.0f )
		m_fOffset = 1.0f;

	// Sort out the blend values
	if ( m_fBlendThreshold < EPSILON )
		m_fBlendThreshold = 0.0f;
	else if ( m_fBlendThreshold > 1.0f )
		m_fBlendThreshold = 1.0f;

#ifdef _DEBUG_EVENT_DISABLE

	m_bDebugDisable=false;

#endif // _DEBUG_EVENT_DISABLE

#endif // _RELEASE
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEvent::OnDelete
//!
//------------------------------------------------------------------------------------------
void CAnimEvent::OnDelete()
{
#	ifndef _RELEASE
		// We don't store back-pointers to parents, so just go through each CAnimEventMonitor object and check if it holds this ael.
		for (	CAnimEventMonitor *monitor = Registered< CAnimEventMonitor >::GetFirst();
				monitor != NULL;
				monitor = monitor->GetNext() )
		{
			monitor->RemoveEventIfPresent( this );
		}
#	endif
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventBSAnimPlay
//!	Trigger
//!
//------------------------------------------------------------------------------------------
void CAnimEventBSAnimPlay::Trigger( CEntity* pobEntity )
{
	ntError( pobEntity );

	int iFlags = 0;
	iFlags |= m_bHoldLastKeyframe ? kBSAF_NoDelete : 0;
	iFlags |= m_bLooping ? kBSAF_Looping : 0;

	BlendShapesComponent* pBSComp = pobEntity->GetBlendShapesComponent();
	//user_warn_p( pBSComp, (__FUNCTION__"target entity %s has no blendshapes component",ntStr::GetString(pobEntity->GetName())) );
	if ( pBSComp )
	{
		pBSComp->GetBSAnimator()->Play( m_obShortName, m_fSpeed, iFlags );
	}
}


void CAnimEventBSAnimPlay::PostConstruct( void )
{

}

//------------------------------------------------------------------------------------------
//!
//! CAnimEventBSAnimStop
//!	Trigger
//!
//------------------------------------------------------------------------------------------
void CAnimEventBSAnimStop::Trigger( CEntity* pobEntity )
{
	ntError( pobEntity );

	BlendShapesComponent* pBSComp = pobEntity->GetBlendShapesComponent();
//	user_warn_p( pBSComp, (__FUNCTION__"target entity has no blendshapes component") );
	if ( pBSComp )
	{
		pBSComp->GetBSAnimator()->Stop( m_obShortName );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventSound::CAnimEventSound
//!	Construction
//!
//------------------------------------------------------------------------------------------
CAnimEventSound::CAnimEventSound () :
	m_eChannel(CHANNEL_ACTION),
	m_fVolume(1.0f),
	m_fPitch(0.0f),
	m_fProbability(1.0f),
	m_bStopSound(false),
	m_bSendToChatterBox(false),
	m_bHasSubtitles(false)
{
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventSound::PostConstruct
//!
//------------------------------------------------------------------------------------------
void CAnimEventSound::PostConstruct ()
{
	// Call the base functionality
	CAnimEvent::PostConstruct();

#ifndef _RELEASE

	if ((m_obSoundBank.IsNull() && !m_obSoundCue.IsNull()) || (!m_obSoundBank.IsNull() && m_obSoundCue.IsNull()))
	{
		if (m_obSoundCue.IsNull())
			ntPrintf("CAnimEventSound: %s has an invalid sound cue (%s/%s)\n", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )),ntStr::GetString(m_obSoundCue),ntStr::GetString(m_obSoundBank));
	}

#endif // _RELEASE
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventSound::EditorChangeValue
//!
//------------------------------------------------------------------------------------------
bool CAnimEventSound::EditorChangeValue (CallBackParameter, CallBackParameter)
{
#ifndef _RELEASE

	CAnimEvent::PostConstruct();

	/*
	if (!m_obSoundCue.IsNull() && !m_obSoundBank.IsNull() && CAudioManager::Get().Verify(CHashedString(m_obSoundBank),CHashedString(m_obSoundCue))==false)
	{
		user_warn_p(false,( "CAnimEventSound: Warning, sound cue %s (%s) does not exist!\n",ntStr::GetString(m_obSoundCue),ntStr::GetString(m_obSoundBank)));
		return false;
	}
	*/
#endif // _RELEASE

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventSound::Trigger
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventSound::Trigger (CEntity* pobEntity)
{
	if (!m_obClumpName.IsNull() && m_obClumpName.GetValue()!=pobEntity->GetClumpFileHash())
		return;

	if (m_obSoundBank.IsNull())
		return;

	if (m_fProbability<1.0f)
	{
		if (m_fProbability<grandf(1.0f))
		{
			return;
		}
	}

	if (m_bSendToChatterBox)
	{
		// To be processed by the ChatterBox
		CChatterBoxMan::Get().TriggerAnimEventSound(pobEntity, this);
		return;
	}

	if (pobEntity && pobEntity->GetEntityAudioChannel()) // Play sound on the entity channel
	{
		pobEntity->GetEntityAudioChannel()->Play(m_eChannel,m_obSoundBank,m_obSoundCue,CPoint(0.0f,0.8f,0.0f),m_fVolume,m_fPitch);
	}
	else // Play sound normally relative to the entity
	{
		unsigned int id;

		if (AudioSystem::Get().Sound_Prepare(id,m_obSoundBank.GetString(),m_obSoundCue.GetString()))
		{
			AudioSystem::Get().Sound_SetVolume(id,m_fVolume);
			AudioSystem::Get().Sound_SetPitch(id,m_fPitch);
			AudioSystem::Get().Sound_SetPosition(id,pobEntity->GetPosition());
			AudioSystem::Get().Sound_Play(id);
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventSound::Stop
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventSound::Stop(CEntity* pobEntity)
{
	if (m_bStopSound)
	{
		/*
		EntityAudioChannel* pobAudio = pobEntity->GetEntityAudioChannel();
		ntAssert_p( pobAudio, ("Trying to play a sound on an entity ('%s') which doesn't have an audio component!", pobEntity->GetName().c_str() ) );
		pobAudio->Stop(m_eChannel);
		*/

		if (pobEntity->GetEntityAudioChannel())
		{
			pobEntity->GetEntityAudioChannel()->Stop(m_eChannel);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
CAnimEventAudioChannelPlay::CAnimEventAudioChannelPlay () :
	m_eChannel(CHANNEL_ACTION),
	m_obPosition(CONSTRUCT_CLEAR),
	m_fVolume(1.0f),
	m_fPitch(0.0f),
	m_fProbability(1.0f)
{
}

void CAnimEventAudioChannelPlay::PostConstruct ()
{
#ifndef _RELEASE

	CAnimEvent::PostConstruct();

	if ((m_obSoundBank.IsNull() && !m_obSoundCue.IsNull()) || (!m_obSoundBank.IsNull() && m_obSoundCue.IsNull()))
	{
		if (strcmp(*m_obSoundCue,"NULL") && strcmp(*m_obSoundBank,"NULL"))
			ntPrintf("CAnimEventAudioChannelPlay: %s has an invalid sound cue (%s/%s)\n", ObjectDatabase::Get().GetNameFromPointer( this ),*m_obSoundCue,*m_obSoundBank);
	}

	m_bDebugMute=false; // This should always be false when an animevent is loaded

#endif // _RELEASE
}

void CAnimEventAudioChannelPlay::EditorChangeValue ()
{
#ifndef _RELEASE

	CAnimEvent::PostConstruct();

	if (!m_obSoundCue.IsNull() && !m_obSoundBank.IsNull() && CAudioManager::Get().Verify(m_obSoundBank,m_obSoundCue)==false)
	{
		user_warn_p(false,( "CAnimEventAudioChannelPlay: Warning, sound cue %s (%s) does not exist!\n",*m_obSoundCue,*m_obSoundBank));
	}

#endif // _RELEASE
}

void CAnimEventAudioChannelPlay::Trigger (CEntity* pobEntity)
{
	if (pobEntity && pobEntity->GetEntityAudioChannel())

#ifndef _RELEASE

	if (m_bDebugMute) // This sound trigger has been temporarily disabled
		return;

	if (m_obSoundCue.IsNull() || m_obSoundBank.IsNull())
		return;

#endif // _RELEASE

	if (m_fProbability<1.0f && m_fProbability<grandf(1.0f))
	{
		return;
	}

	if (pobEntity && pobEntity->GetEntityAudioChannel())
	{
		pobEntity->GetEntityAudioChannel()->Play(m_eChannel,,m_obSoundBank,m_obSoundCue,m_obPosition,m_fVolume,m_fPitch);
	}
}

void CAnimEventAudioChannelPlay::Stop (CEntity*)
{
}
*/

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CAnimEventAudioChannelStop::CAnimEventAudioChannelStop () :
	m_eChannel(CHANNEL_ACTION)
{
}

void CAnimEventAudioChannelStop::PostConstruct ()
{
#ifndef _RELEASE
	CAnimEvent::PostConstruct();
#endif // RELEASE
}

void CAnimEventAudioChannelStop::EditorChangeValue ()
{
#ifndef _RELEASE
	CAnimEvent::PostConstruct();
#endif // RELEASE
}

void CAnimEventAudioChannelStop::Trigger (CEntity* pobEntity)
{
	if (pobEntity && pobEntity->GetEntityAudioChannel())
	{
		pobEntity->GetEntityAudioChannel()->Stop(m_eChannel);
	}
}

void CAnimEventAudioChannelStop::Stop (CEntity* /*pobEntity*/)
{
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CAnimEventFootstep::CAnimEventFootstep () :
	m_fVolume(1.0f),
	m_eFootstep(LEFT_STEP)
{
}

void CAnimEventFootstep::Trigger (CEntity* pobEntity)
{
	if (pobEntity->GetEntityAudioChannel())
	{
		pobEntity->GetEntityAudioChannel()->ProcessFootstepEvent(m_eFootstep,m_fVolume);
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
//!
//!	AnimEventMixerProfile::AnimEventMixerProfile
//!	
//!
//------------------------------------------------------------------------------------------
AnimEventMixerProfile::AnimEventMixerProfile ()	:
	m_fTransitionDuration(0.0f)
{

}

//------------------------------------------------------------------------------------------
//!
//!	AnimEventMixerProfile::Trigger
//!	
//!
//------------------------------------------------------------------------------------------
void AnimEventMixerProfile::Trigger (CEntity* /*pobEntity*/)
{
	AudioMixer::Get().SetProfile(m_obProfile,m_fTransitionDuration);
}











//------------------------------------------------------------------------------------------
//!
//!	CAnimEventScript::CAnimEventScript
//!	
//!
//------------------------------------------------------------------------------------------
CAnimEventScript::CAnimEventScript()
{
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventScript::Trigger
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventScript::Trigger( CEntity* pobEntity )
{
	// find/call the event function
	CLuaGlobal::Get().SetTarg( pobEntity );

#ifndef _RELEASE
	if (!CLuaGlobal::Get().State().GetGlobals()[m_obFunction].IsFunction())
	{
		user_warn_p(false,( "CAnimEventScript: Warning, %s is not a valid lua function\n", ntStr::GetString(m_obFunction)));
		return;
	}

	//ntAssert_p( CLuaGlobal::Get().GetState().GetGlobal( *m_obFunction ).IsFunction(), ("Animevent cannot find function '%s'", *m_obFunction ) );
#endif // _RELEASE

	NinjaLua::LuaFunction obFunc ( CLuaGlobal::Get().State().GetGlobals()[ m_obFunction ] );

	if (pobEntity->HasAttributeTable())
		obFunc(pobEntity->GetAttributeTable()->GetLuaObjectWrapper()); // Pass in the entity as the argument to the lua function (if the entity has an attribute table!)
	else
		obFunc();

	CLuaGlobal::Get().SetTarg( 0 );

	// Tell the world what we are upto
	// ntPrintf( "Using %s, function: %s at %0.2f.\n", GetNameC(), *m_obFunction, m_fOffset );
}







//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMessage::CAnimEventMessage
//!	
//!
//------------------------------------------------------------------------------------------
CAnimEventMessage::CAnimEventMessage()
{
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMessage::Trigger
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventMessage::Trigger( CEntity* pobEntity )
{
	//send the stored message to the entity
	CMessageSender::SendEmptyMessage( m_obMessage, pobEntity->GetMessageHandler() );
}


CAnimEventDemonAnimMessage::CAnimEventDemonAnimMessage()
{
}


void CAnimEventDemonAnimMessage::Trigger( CEntity* pobEntity )
{
	//Send the stored message to the entity.
	if(pobEntity->GetMessageHandler())
	{
		Message obDemonAnimMessage(msg_demon_playanim);
		obDemonAnimMessage.SetString("AnimName", m_obAnimName);
		pobEntity->GetMessageHandler()->QueueMessage(obDemonAnimMessage);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventCameraShake::CAnimEventCameraShake
//!	
//!
//------------------------------------------------------------------------------------------
CAnimEventCameraShake::CAnimEventCameraShake() :
	m_fTime(0.0f),
	m_fIntensity(0.0f),
	m_fSpeed(0.0f)
{
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventCameraShake::Trigger
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventCameraShake::Trigger (CEntity* /*pobEntity*/)
{
	//TODO: Apply Camera Shake.
	CamMan::Get().Shake(m_fTime, m_fIntensity, m_fSpeed);
}







//------------------------------------------------------------------------------------------
//!
//!	AnimEventEffect::AnimEventEffect
//!	Construction
//!
//------------------------------------------------------------------------------------------
AnimEventEffect::AnimEventEffect() :
	m_bAutoDestruct(false),
	m_pobTriggerDef(0)
{
}


//------------------------------------------------------------------------------------------
//!
//!	AnimEventEffect::~AnimEventEffect
//!	Construction
//!
//------------------------------------------------------------------------------------------
AnimEventEffect::~AnimEventEffect()
{
	while (!m_cleanups.empty())
	{
		NT_DELETE_CHUNK( Mem::MC_ANIMATION, m_cleanups.back() );
		m_cleanups.pop_back();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	AnimEventEffect::Trigger
//!	
//!
//------------------------------------------------------------------------------------------
void AnimEventEffect::Trigger(CEntity* pParentEnt)
{
	if (m_pobTriggerDef == 0)
	{
#ifndef _RELEASE
		char aErrors[MAX_PATH];
		sprintf( aErrors, "AnimEventEffect %s has no trigger definition\n", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )) );
		EffectErrorMSG::AddDebugError( aErrors );
#endif
		return;
	}

	u_int iNewEffectID = m_pobTriggerDef->ForceTrigger(pParentEnt);

	if (m_bAutoDestruct)
	{
		EntIDListNode* pNode = NT_NEW_CHUNK( Mem::MC_ANIMATION ) EntIDListNode;
		pNode->iID = iNewEffectID;
		pNode->pEnt = pParentEnt;

		m_cleanups.push_back( pNode );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	AnimEventEffect::Stop
//!	To create an instance of what we describe
//!
//------------------------------------------------------------------------------------------
void AnimEventEffect::Stop(CEntity* pParentEnt)
{
	if (m_bAutoDestruct)
	{
		for(	ntstd::List<EntIDListNode*, Mem::MC_ANIMATION>::iterator it = m_cleanups.begin();
				it != m_cleanups.end(); ++it )
		{
			if ((*it)->pEnt == pParentEnt)
			{
				EffectManager::Get().KillEffectWhenReady( (*it)->iID );
				NT_DELETE_CHUNK( Mem::MC_ANIMATION, *it );
				m_cleanups.erase( it );
				return;
			} 
		}
		ntPrintf( "stop called without a corresponding trigger Parent Entity Name: %s\n", pParentEnt->GetName().c_str() );
	}
}








//------------------------------------------------------------------------------------------
//!
//!	CAnimEventList::CAnimEventList
//!	Construction
//!
//------------------------------------------------------------------------------------------
CAnimEventList::CAnimEventList( void )
{
}


#ifndef _RELEASE
//------------------------------------------------------------------------------------------
//!
//!	CAnimEventList::PostConstruct
//!	Check the interfaces of the serialised items and make sure they are of a valid type
//!
//------------------------------------------------------------------------------------------
void CAnimEventList::PostConstruct( void )
{
	// Loop through all our contained animation events and check that the interfaces are OK
	using namespace ntstd;
	List<CAnimEvent*, Mem::MC_ANIMATION>::const_iterator obEnd = m_obAnimEventList.end();
	for ( List<CAnimEvent*, Mem::MC_ANIMATION>::const_iterator obIt = m_obAnimEventList.begin(); obIt != obEnd; ++obIt )
	{
		// Get the data object from pointer
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( *obIt );

		// Make sure that we get a data object
		if ( !pDO )
		{
			ntError_p( 0, ( "Broken anim event in list %s.\n", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )) ) );
		}

		// Check that we have a data object and the interface is acceptable - obviously
		// this list needs to be updated with each animation event type we add...
		else if ( ( strcmp( pDO->GetClassName(), "AnimEventEffect" ) != 0 )
				  &&
				  ( strcmp( pDO->GetClassName(), "AnimEventCombatAttack" ) != 0 )
				  &&
				  ( strcmp( pDO->GetClassName(), "CAnimEventCameraShake" ) != 0 )
				  &&
				  ( strcmp( pDO->GetClassName(), "CAnimEventMessage" ) != 0 )
				  &&
				  ( strcmp( pDO->GetClassName(), "CAnimEventDemonAnimMessage" ) != 0 )
				  &&
				  ( strcmp( pDO->GetClassName(), "CAnimEventScript" ) != 0 )
				  &&
				  ( strcmp( pDO->GetClassName(), "CAnimEventLayer" ) != 0 )
				  &&
				  ( strcmp( pDO->GetClassName(), "CAnimEventMusic" ) != 0 )
				  &&
				  ( strcmp( pDO->GetClassName(), "CAnimEventSound" ) != 0 )
				  &&
				  ( strcmp( pDO->GetClassName(), "CAnimEventAudioChannelStop" ) != 0 ) 
				  &&
				  ( strcmp( pDO->GetClassName(), "CAnimEventFootstep" ) != 0 ) 
				  &&
				  ( strcmp( pDO->GetClassName(), "CAnimEventBSAnimPlay" ) != 0 ) 
				  &&
				  ( strcmp( pDO->GetClassName(), "CAnimEventBSAnimStop" ) != 0 ) )

		{
			ntError_p( 0, ( "%s in the Anim Event List %s is not a valid anim event.\n", ntStr::GetString(pDO->GetName()), ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( this )) ) );
		}
	}
}
#endif

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventList::OnDelete
//!
//------------------------------------------------------------------------------------------
void CAnimEventList::OnDelete()
{
#	ifndef _RELEASE
		// We don't store back-pointers to parents, so just go through each CAnimEventMonitor object and check if it holds this ael.
		for (	CAnimEventMonitor *monitor = Registered< CAnimEventMonitor >::GetFirst();
				monitor != NULL;
				monitor = monitor->GetNext() )
		{
			monitor->RemoveEventListIfPresent( this );
		}
#	endif
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventList::RemoveEventIfPresent
//!
//------------------------------------------------------------------------------------------
void CAnimEventList::RemoveEventIfPresent( CAnimEvent *ae ) const
{
	for (	ntstd::List< CAnimEvent *, Mem::MC_ANIMATION >::iterator it = m_obAnimEventList.begin();
			it != m_obAnimEventList.end(); )
	{
		if ( ae == *it )
		{
			it = m_obAnimEventList.erase( it );
		}
		else
		{
			++it;
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMonitor::CAnimEventMonitor
//!	Constrcution
//!
//------------------------------------------------------------------------------------------
CAnimEventMonitor::CAnimEventMonitor( u_int uiAnimNameHash, const CAnimEventList* pobAnimEventList,CEntity* pobEntity )
:	m_uiAnimNameHash( uiAnimNameHash ),
	m_pobAnimEventList( pobAnimEventList ),
	m_pobEntity( pobEntity ),
	m_bActive( false )
{
	// Make sure we have an anim event list
	ntAssert( pobAnimEventList );

	m_Animations.reserve( 2 );
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMonitor::Update
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventMonitor::Update( void )
{
	// If we are not currently active...
	if ( !m_bActive || m_pobAnimEventList == NULL )
		return;

	for ( HandledAnimVector::iterator anim_it = m_Animations.begin(); anim_it != m_Animations.end(); ++anim_it )
	{
		// Find the percentage we are currently through the animation
		anim_it->m_CurrentProgress = anim_it->m_Anim->GetTime() / anim_it->m_Anim->GetDuration();

		// If the position has changed...
		if ( ( anim_it->m_Anim->GetSpeed() > 0.0f ) && ( anim_it->m_CurrentProgress != anim_it->m_PreviousProgress ) )
		{
			// Find the current blend weight of the animation
			const float fBlendWeight = anim_it->m_Anim->GetBlendWeight();

			// Loop through all our anim events...
			ntstd::List<CAnimEvent*, Mem::MC_ANIMATION>::const_iterator obEnd = m_pobAnimEventList->m_obAnimEventList.end();
			for( ntstd::List<CAnimEvent*, Mem::MC_ANIMATION>::const_iterator obIt = m_pobAnimEventList->m_obAnimEventList.begin(); obIt != obEnd; ++obIt )
			{
				// If we are progressing through the animation normally...
				if ( anim_it->m_PreviousProgress < anim_it->m_CurrentProgress )
				{
					// If the offset of our event falls between the current and last update
					if ( ( ( *obIt )->m_fOffset >= anim_it->m_PreviousProgress ) && ( ( *obIt )->m_fOffset < anim_it->m_CurrentProgress ) )
					{
						// ...and the blend weight is above the desired threshold
						if ( fBlendWeight >= ( *obIt )->m_fBlendThreshold )
						{
							#ifdef _DEBUG_DISABLE_EVENT
							if (!(*obIt)->m_bDebugDisable)
							#endif // _DEBUG_DISABLE_EVENT

							// Trigger the event
							( *obIt )->Trigger( m_pobEntity );
						}
					}
				}

				// Previous progress is higher than current progress - this indicates the anim is looping - assumes
				// that we are not playing animations backwards
				else 
				{
					// If the event falls over the loop gap
					if ( ( ( *obIt )->m_fOffset >= anim_it->m_PreviousProgress ) || ( ( *obIt )->m_fOffset < anim_it->m_CurrentProgress ) )
					{
						// ...and the blend weight is above the desired threshold
						if ( fBlendWeight >= ( *obIt )->m_fBlendThreshold )
						{
							#ifdef _DEBUG_DISABLE_EVENT
							if (!(*obIt)->m_bDebugDisable)
							#endif // _DEBUG_DISABLE_EVENT

							// Trigger the event
							( *obIt )->Trigger( m_pobEntity );
						}
					}
				}
			}

			// Store the progress fro next time around
			anim_it->m_PreviousProgress = anim_it->m_CurrentProgress;
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMonitor::SetAnimation
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventMonitor::SetAnimation( const CAnimationPtr& pobAnimation )
{
	ntError( pobAnimation->GetShortNameHash() == m_uiAnimNameHash );

	// Set the animation member - previous and current progress are initialised to 0.0 in the ctor.
	HandledAnim handled_anim;
	handled_anim.m_Anim = pobAnimation;

	// Hold the pointer to the running instance of our animation
	m_Animations.push_back( handled_anim );

	// Activate our events
	m_bActive = true;

	//ntPrintf("Monitor:%x  Adding animation %x (%d)- List size=%d\n",this,pobAnimation->GetShortNameHash(),pobAnimation->IsActive(),m_Animations.size());
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMonitor::RemoveAnimation
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventMonitor::RemoveAnimation( const CAnimationPtr &anim )
{
	// Stop all the events associated with our animation
	// XXX TODO NOTE: We currently don't track which events are generated by which animations held by this handler,
	// so we have to stop all events. This may be a problem.
	// Example: Player evades twice in quick succession, we have two evade anims on the animator, but they
	// are the same animation being played so they are both held by a single CAnimEventMonitor object;
	// In this case, when the first evade finishes, all outstanding events from *both* evade animations will be
	// stopped. While this isn't technically correct, it may well be "good enough". [ARV].
	if ( m_pobAnimEventList != NULL )
	{
		ntstd::List<CAnimEvent*, Mem::MC_ANIMATION>::const_iterator obEnd = m_pobAnimEventList->m_obAnimEventList.end();
		for ( ntstd::List<CAnimEvent*, Mem::MC_ANIMATION>::const_iterator obIt = m_pobAnimEventList->m_obAnimEventList.begin(); obIt != obEnd; ++obIt )
			( *obIt )->Stop( m_pobEntity );
	}

	// ntPrintf("removing anim #%d\n",m_uiAnimNameHash);

	for ( HandledAnimVector::iterator anim_it = m_Animations.begin(); anim_it != m_Animations.end(); ++anim_it )
	{
		if ( anim_it->m_Anim = anim )
		{
			m_Animations.erase( anim_it );
			break;
		}
	}

	m_bActive = ( m_Animations.size() > 0 );

	//ntPrintf("Monitor:%x  Removing animation %x (%d) - List size=%d\n",this,anim->GetShortNameHash(),anim->IsActive(),m_Animations.size());
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMonitor::RemoveAllAnimations
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventMonitor::RemoveAllAnimations()
{
	// Deactivate the animation
	m_bActive = false;

	// Stop all the events associated with our animation
	if ( m_pobAnimEventList != NULL )
	{
		ntstd::List<CAnimEvent*, Mem::MC_ANIMATION>::const_iterator obEnd = m_pobAnimEventList->m_obAnimEventList.end();
		for ( ntstd::List<CAnimEvent*, Mem::MC_ANIMATION>::const_iterator obIt = m_pobAnimEventList->m_obAnimEventList.begin(); obIt != obEnd; ++obIt )
			( *obIt )->Stop( m_pobEntity );
	}

	// ntPrintf("removing anim #%d\n",m_uiAnimNameHash);

	for ( HandledAnimVector::iterator anim_it = m_Animations.begin(); anim_it != m_Animations.end(); ++anim_it )
	{
		anim_it->m_Anim = CAnimationPtr( NULL );	// Maybe not necessary. Being paranoid. [ARV].
	}

	m_Animations.clear();

	//ntPrintf("Monitor: %x  RemoveAllAnimations\n",this);
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMonitor::ResetProgress
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventMonitor::ResetProgress ()
{
	for ( HandledAnimVector::iterator anim_it = m_Animations.begin(); anim_it != m_Animations.end(); ++anim_it )
	{
		anim_it->m_CurrentProgress = anim_it->m_Anim->GetTime() / anim_it->m_Anim->GetDuration();
		anim_it->m_PreviousProgress = anim_it->m_CurrentProgress;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMonitor::RemoveEventListIfPresent
//!	
//------------------------------------------------------------------------------------------
void CAnimEventMonitor::RemoveEventListIfPresent( CAnimEventList *ael )
{
	if ( m_pobAnimEventList == ael )
	{
		m_pobAnimEventList = NULL;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMonitor::ToggleDebugDisable
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventMonitor::ToggleDebugDisable (const CHashedString& obEventName)
{
#ifdef _DEBUG_DISABLE_EVENT

	ntstd::List<CAnimEvent*, Mem::MC_ANIMATION>::const_iterator obEnd = m_pobAnimEventList->m_obAnimEventList.end();

	for( ntstd::List<CAnimEvent*, Mem::MC_ANIMATION>::const_iterator obIt = m_pobAnimEventList->m_obAnimEventList.begin(); obIt != obEnd; ++obIt )
	{
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( (*obIt) );
		
		if ( pDO && pDO->GetName()==obEventName )
		{
			(*obIt)->m_bDebugDisable=!(*obIt)->m_bDebugDisable;
			return;
		}
	}

#else

	UNUSED(obEventName);

#endif // _DEBUG_DISABLE_EVENT
}












//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::CAnimEventHandler
//!	Construction
//!
//------------------------------------------------------------------------------------------
CAnimEventHandler::CAnimEventHandler( void )
:	m_obAnimEventMonitorList(),
	m_pobParentEntity( 0 ),
	m_uiAnimDoneHash( 0 ),
	m_pcAnimDoneCallback( 0 ),
	m_bDebugRender( false )
{
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::~CAnimEventHandler
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CAnimEventHandler::~CAnimEventHandler( void )
{
	RemoveAllMonitors();
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::SetEntity
//!	Set our parent entity pointer
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::SetEntity( CEntity* pobParentEntity )
{
	m_pobParentEntity = pobParentEntity;
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::AddAnimEventMonitor
//!	Add a monitor to our full list
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::AddAnimEventMonitor( u_int uiAnimNameHash, const CAnimEventList* pobAnimEventList )
{
	ntAssert( uiAnimNameHash );

	// We can only have animation events if we have a parent entity pointer
	if ( m_pobParentEntity != NULL && uiAnimNameHash )
	{
		// If we have a name hash build an event monitor for this animation
		m_obAnimEventMonitorList.push_back( NT_NEW_CHUNK( Mem::MC_ANIMATION ) CAnimEventMonitor( uiAnimNameHash, pobAnimEventList, m_pobParentEntity ) );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::RemoveAnimEventMonitor
//!	Remove any monitors associated with a particular anim.
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::RemoveAnimEventMonitor( u_int uiAnimNameHash )
{
	if ( uiAnimNameHash )
	{
		// Remove monitors from the update list
		
		{
			if(!m_obMonitorUpdateList.empty())
			{
				ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::iterator obIt = m_obMonitorUpdateList.begin();

				while( obIt != m_obMonitorUpdateList.end() )
				{
					if ((*obIt)->GetAnimNameHash()==uiAnimNameHash )
					{
						obIt=m_obMonitorUpdateList.erase(obIt);
					}
					else
					{
						++obIt;
					}
				}
			}
		}

		// Remove monitors from the anim event monitor list and delete them

		{
			if(!m_obMonitorUpdateList.empty())
			{
				ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::iterator obIt = m_obAnimEventMonitorList.begin();

				while( obIt != m_obAnimEventMonitorList.end() )
				{
					if ((*obIt)->GetAnimNameHash()==uiAnimNameHash )
					{
						NT_DELETE_CHUNK( Mem::MC_ANIMATION, *obIt );
						obIt=m_obAnimEventMonitorList.erase(obIt);
					}
					else
					{
						++obIt;
					}
				}
			}
		}
		
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::Update
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::Update( void )
{
	if (!m_pobParentEntity || !m_pobParentEntity->GetAnimator()->IsEnabled())
		return;

	// Update active monitors only
	ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::const_iterator obEnd = m_obMonitorUpdateList.end();
	for( ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::const_iterator obIt = m_obMonitorUpdateList.begin(); obIt != obEnd; ++obIt )
	{
		( *obIt )->Update();
	}

#ifndef _RELEASE
	if (m_bDebugRender)
	{
		const u_long ulColour=0xffffff00;
		const u_long ulElapsedColour=0xffff9900;
		const float fV_SPACING=11.0f;
		
		float fX=10.0f;
		float fY=10.0f;

		if (m_obMonitorUpdateList.size()==0)
		{
			g_VisualDebug->Printf2D(fX,fY, ulColour, 0, "No anim events active");
			fY+=fV_SPACING;
		}
		else
		{
			ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::const_iterator obEnd = m_obMonitorUpdateList.end();

			for( ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::const_iterator obIt = m_obMonitorUpdateList.begin(); obIt != obEnd; ++obIt )
			{
				const CAnimEventMonitor *monitor = *obIt;

				int32_t anim_counter = 0;

				for (	CAnimEventMonitor::HandledAnimVector::const_iterator anim_it = monitor->m_Animations.begin();
						anim_it != monitor->m_Animations.end();
						++anim_it )
				{
					const CAnimEventMonitor::HandledAnim &anim_data = *anim_it;

					if ( anim_data.m_Anim->GetBlendWeight() > 0.0f )
					{
						g_VisualDebug->Printf2D(fX,fY, ulColour, 0, "%s  (%1.2f)",  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( monitor->m_pobAnimEventList )), anim_data.m_CurrentProgress);
						fY+=fV_SPACING;

						for( ntstd::List<CAnimEvent*, Mem::MC_ANIMATION>::const_iterator obIt2 = monitor->m_pobAnimEventList->m_obAnimEventList.begin();
							obIt2 != monitor->m_pobAnimEventList->m_obAnimEventList.end();
							++obIt2 )
						{
							u_long ulAnimEventColour;

							if ( anim_data.m_CurrentProgress >= (*obIt2)->m_fOffset )
							{
								ulAnimEventColour=ulElapsedColour;
							}
							else
							{
								ulAnimEventColour=ulColour;
							}
							
							g_VisualDebug->Printf2D(fX,fY, ulAnimEventColour, 0, "%1.2f  %s on anim %i", (*obIt2)->m_fOffset,  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(*obIt2)), anim_counter);
							fY+=fV_SPACING;
						}
						
						fY+=fV_SPACING;
					}

					anim_counter++;
				}
			}
		}
	}
#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::SetAnimation
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::SetAnimation( const CAnimationPtr& pobAnimation )
{
	// Get the hash name of this animation
	const u_int uiHash = pobAnimation->GetShortNameHash();

	ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::const_iterator obEnd = m_obAnimEventMonitorList.end();
	for( ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::const_iterator obIt = m_obAnimEventMonitorList.begin(); obIt != obEnd; ++obIt )
	{
		if ( *obIt != NULL && (*obIt)->GetAnimNameHash() == uiHash )
		{
			( *obIt )->SetAnimation( pobAnimation );

			AddToUpdateList(*obIt);
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::ClearAnimations
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::ClearAnimations ( void )
{
	// If we don't have a parent entity we won't have ny animations
	if ( m_pobParentEntity == NULL )
		return;

	/*
	// Loop through all our animations and remove them all
	ntstd::List<CAnimEventMonitor*>::iterator obEnd = m_obAnimEventMonitorList.end();
	for( ntstd::List<CAnimEventMonitor*>::iterator obIt=m_obAnimEventMonitorList.begin(); obIt != obEnd; ++obIt )
	{
		if ( ( *obIt )->IsActive() )
			( *obIt )->RemoveAnimation();
	}
	*/

	// Loop through and delete all our monitors
	while(!m_obMonitorUpdateList.empty())
	{
		m_obMonitorUpdateList.back()->RemoveAllAnimations();
		m_obMonitorUpdateList.pop_back();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::RemoveAnimation
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::RemoveAnimation ( const CAnimationPtr& pobAnimation )
{
	// If we don't have a parent entity we won't have ny animations
	if ( m_pobParentEntity == NULL )
		return;

	// Get the hash of the animation to remove
	const u_int uiHash = pobAnimation->GetShortNameHash();

	// Update active monitors only
	ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::iterator obEnd = m_obMonitorUpdateList.end();
	for( ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::iterator obIt = m_obMonitorUpdateList.begin(); obIt != obEnd; )
	{
		if ((*obIt)->GetAnimNameHash()==uiHash)
		{
			(*obIt)->RemoveAnimation( pobAnimation );

			//NT_DELETE( *obIt );
			obIt=m_obMonitorUpdateList.erase(obIt);
		}
		else
		{
			++obIt;
		}
	}

	// We made need to send a message on completion of this particular aniamtion
	if ( m_uiAnimDoneHash == uiHash )
	{
		// If we have a message handler then send the message
		if ( m_pobParentEntity->GetMessageHandler() && m_pcAnimDoneMessageString == "NO_MESSAGE")
		{
			//old style, just send msg_animdone
			m_pobParentEntity->GetMessageHandler()->ReceiveMsg<msg_animdone>();
		}
		else if (m_pobParentEntity->GetMessageHandler() && m_pcAnimDoneMessageString != "NO_MESSAGE")
		{
			//new style, send the string that was specified in the call to GenerateAnimDoneMessage
			CMessageSender::SendEmptyMessage( m_pcAnimDoneMessageString.c_str(), m_pobParentEntity->GetMessageHandler() );
		}

		if (m_pcAnimDoneCallback)
			m_pcAnimDoneCallback(m_pobParentEntity);

		// Clear the 'animation done' hash
		m_uiAnimDoneHash = 0;
		// And the animation done message
		m_pcAnimDoneMessageString = "NO_MESSAGE";
		m_pcAnimDoneCallback = 0; 
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::ResetProgress
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::ResetProgress (const CAnimationPtr& pobAnimation )
{
	// Get the hash of the animation to remove
	const u_int uiHash = pobAnimation->GetShortNameHash();

	ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::iterator obEnd = m_obMonitorUpdateList.end();
	for( ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::iterator obIt = m_obMonitorUpdateList.begin(); obIt != obEnd; obIt++ )
	{
		if ((*obIt)->GetAnimNameHash()==uiHash)
		{
			(*obIt)->ResetProgress();
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::ToggleDebugDisable
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::ToggleDebugDisable (const CHashedString& obAnimName,const CHashedString& obEventName)
{
#ifdef _DEBUG_DISABLE_EVENT

	for( ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::iterator obIt=m_obMonitorUpdateList.begin(); obIt!=m_obMonitorUpdateList.end(); ++obIt)
	{
		if (obAnimName.GetValue()==(*obIt)->GetAnimNameHash())
		{
			(*obIt)->ToggleDebugDisable(obEventName);
			return;
		}
	}

#else

	UNUSED(obAnimName);
	UNUSED(obEventName);

#endif // _DEBUG_DISABLE_EVENT
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::RemoveAllMonitors
//!	
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::RemoveAllMonitors( void )
{
	m_obMonitorUpdateList.clear();

	// Loop through and free all our monitors
	while(!m_obAnimEventMonitorList.empty())
	{
		NT_DELETE_CHUNK( Mem::MC_ANIMATION, m_obAnimEventMonitorList.back() );
		m_obAnimEventMonitorList.pop_back();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::GenerateAnimDoneMessage
//!
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::GenerateAnimDoneMessage( const CHashedString& pcAnimName )
{
	// Check the validity of our data
	ntAssert( !ntStr::IsNull(pcAnimName) );

	// Store the hash of the animations name
	m_uiAnimDoneHash = ntStr::GetHashKey(pcAnimName);
	// Set the member message string to null so there is no confusion over what to do in Remove
	m_pcAnimDoneMessageString = "NO_MESSAGE";
}


//for new style anim callbacks that supply a specific message string
void CAnimEventHandler::GenerateAnimDoneMessage( const CHashedString& pcAnimName , const char* pcMessageString )
{
	// Check the validity of our data
	ntAssert( !ntStr::IsNull(pcAnimName) );
	ntAssert( pcMessageString );

	// Store the hash of the animations name
	m_uiAnimDoneHash = ntStr::GetHashKey( pcAnimName );
	// Store the message string to be sent upon completion of the animation
	m_pcAnimDoneMessageString = pcMessageString;

	//the animeventhandler can only handle one anim at a time?
}

//for new style anim callbacks that supply a specific message string
void CAnimEventHandler::GenerateAnimDoneMessageAndCallback( const CHashedString& pcAnimName , const char* pcMessageString, CompletionCallback pcCallback )
{
	// Check the validity of our data
	ntAssert( !ntStr::IsNull(pcAnimName) );	

	// Store the hash of the animations name
	m_uiAnimDoneHash = ntStr::GetHashKey( pcAnimName );
	// Store the message string to be sent upon completion of the animation
	if (pcMessageString)
		m_pcAnimDoneMessageString = pcMessageString;

	m_pcAnimDoneCallback = pcCallback;

	//the animeventhandler can only handle one anim at a time?	
}

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler::AddToUpdateList
//!
//------------------------------------------------------------------------------------------
void CAnimEventHandler::AddToUpdateList (CAnimEventMonitor* pobMonitor)
{
	// Update active monitors only
	ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::const_iterator obEnd = m_obMonitorUpdateList.end();
	for( ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION>::const_iterator obIt = m_obMonitorUpdateList.begin(); obIt != obEnd; ++obIt )
	{
		if (pobMonitor==*obIt )
			return;
	}

	m_obMonitorUpdateList.push_back(pobMonitor);
}

//------------------------------------------------------------------------------------------
//!  public virtual  Trigger
//!
//!  @param [in, out]  pEnt CEntity * 
//!
//!
//!  @author GavB @date 22/11/2006
//------------------------------------------------------------------------------------------
void AnimEventCombatAttack::Trigger(CEntity* pEnt)
{
	// Make sure the attack data is all fine
	ntAssert( m_pAttackData );

	// If required - then send out the strikes
	if( m_bUseStrike )
	{
		// Method to find entities. 
		CEntityQuery obQuery;

		// Attack only those inrange
		CEQCProximitySphere obSphereTest;
		obSphereTest.Set (pEnt->GetPosition(), m_fRadius);

		// Find all the entities that match our needs
		CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_Character );

		// Parse all those entities found
		for(QueryResultsContainerType::iterator obIt=obQuery.GetResults().begin(); obIt!=obQuery.GetResults().end(); ++obIt)
		{
			CEntity* pTargetEnt = (*obIt);
			ntError( pTargetEnt );

			// Make sure that someone is not trying to hit themself
			if ( pEnt == pTargetEnt )
				continue;

			// If the target has an attack component we can issue them with a strike
			if ( pTargetEnt->GetAttackComponent() )
			{
				// Sort out an originating position - use an attacker if we have it
				CPoint obOriginatorPos = pEnt->GetPosition();

				// Issue a strike - using the thrown entity as the originator - but the reaction position 
				// relative to the character that issued the attack
				CStrike* pobStrike = NT_NEW CStrike( 0, 
													pTargetEnt, 
													m_pAttackData, 
													1.0f, 
													1.0f, 
													false, 
													false, 
													false,
													false,
													false,
													false,
													0,
													obOriginatorPos );

				// Post the strike
				SyncdCombat::Get().PostStrike( pobStrike );
			}

			// Otherwise we can still send the entity a message that it may be able to handle
			else if ( pTargetEnt->GetMessageHandler() )
			{
				// Send the message
				CMessageSender::SendEmptyMessage( msg_object_strike, pTargetEnt->GetMessageHandler() );
			}
		}
	}

	// Perhap the wake effect could alse be triggered
	if( m_bUseWake && pEnt->GetAttackComponent() && pEnt->GetAttackComponent()->GetAttackDefinition() )
	{
		// Find the attack def for the host entity
		const CAttackDefinition* pAttackDef = pEnt->GetAttackComponent()->GetAttackDefinition();

		// Within the attack def is the combat effect def...
		if( pAttackDef->m_pCombatEffectsDef )
		{
			pAttackDef->m_pCombatEffectsDef->SetupEffectsTrigger( pEnt, NULL, m_pAttackData, m_pAttackData->GetAttackTime(1.0f) );
		}
	}

	// If required create an explosion based on the debris impulse value
	if( m_fDebrisImpulse > 0.0f )
	{
		Physics::CExplosionParams params;
		params.m_fPush = m_fDebrisImpulse;
		params.m_fPushDropoff = m_fDebrisImpulse / 2.0f;
		params.m_fRadius = m_fRadius;
		params.m_pobOriginator = pEnt;
		params.m_obPosition = pEnt->GetPosition();
		pEnt->GetPhysicsSystem()->Lua_AltExplosion( params );
	}
}
