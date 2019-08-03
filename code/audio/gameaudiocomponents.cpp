
#include "audio/gameaudiocomponents.h"

#include "Physics/config.h"
#include "Physics/world.h"
#include "Physics/system.h"
#include "physics/physicsmaterial.h"
#include "physics/physicstools.h"

#include <hkcollide\castutil\hkWorldRayCastInput.h>
#include <hkcollide\castutil\hkWorldRayCastOutput.h>

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include "Physics/advancedcharactercontroller.h"
#endif

#include "audio/audiosystem.h"
#include "audio/audiomixer.h"
#include "audio/audioreverb.h"
#include "audio/ambientsound.h"
#include "audio/audiohelper.h"
#include "audio/audioconsole.h"
#include "audio/collisioneffectmanager.h"
#include "audio/imusic.h"
#include "audio/musiccontrol.h"

#include "camera/camman_public.h"
#include "camera/camman.h"

#include "core/visualdebugger.h"
#include "core/gatso.h"
#include "core/timer.h"

#include "anim/animator.h"
#include "anim/animation.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityai.h"
#include "game/attacks.h"
#include "game/randmanager.h"
#include "game/aicomponent.h"
#include "game/shellconfig.h"
#include "game/shellmain.h"

#include "objectdatabase/dataobject.h"
#include "gfx/levellighting.h"
#include "editable/enumlist.h"



// Comment these out to disable
#define _INTERACTIVE_MUSIC
#define _ENABLE_COMBAT_HIT_SOUNDS



#ifndef _RELEASE
//#define _SIMPLE_TEST
#endif // _RELEASE





START_CHUNKED_INTERFACE (GameAudioConfig, Mem::MC_MISC)
	PUBLISH_VAR_AS(m_obMixerProfile_Frontend,		MixerProfile_Frontend)
	PUBLISH_VAR_AS(m_obMixerProfile_LevelStart,		MixerProfile_LevelStart)
	PUBLISH_VAR_AS(m_obMixerProfile_LevelExit,		MixerProfile_LevelExit)
	PUBLISH_VAR_AS(m_obMixerProfile_GamePause,		MixerProfile_GamePause)
	PUBLISH_VAR_AS(m_obMixerProfile_OSDPause,		MixerProfile_OSDPause)

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE




START_CHUNKED_INTERFACE (FootstepEffect, Mem::MC_MISC)
	PUBLISH_VAR_AS(m_obSurfaceMaterial,		SurfaceMaterial)
	PUBLISH_VAR_AS(m_obSingleFootSound,		SingleFootSound)	
	PUBLISH_VAR_AS(m_obDoubleFootSound,		DoubleFootSound)	
	PUBLISH_VAR_AS(m_obSlideShortSound,		SlideShortSound)	
	PUBLISH_VAR_AS(m_obSlideMediumSound,	SlideMediumSound)
	PUBLISH_VAR_AS(m_obSlideLongSound,		SlideLongSound)
	PUBLISH_VAR_AS(m_obCustom1Sound,		Custom1Sound)	
	PUBLISH_VAR_AS(m_obCustom2Sound,		Custom2Sound)
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(FootstepProperties, Mem::MC_MISC)
	PUBLISH_PTR_CONTAINER_AS(m_obFootstepEffectList, FootstepEffectList)

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE





START_CHUNKED_INTERFACE (CombatHitSoundDefinition, Mem::MC_MISC)
	PUBLISH_VAR_AS(m_obAttackerEntityClump,			AttackerEntityClump)
	PUBLISH_VAR_AS(m_obReceiverEntityClump,			ReceiverEntityClump)
													
	PUBLISH_VAR_AS(m_obHit_SpeedFast,				Hit_SpeedFast)
	PUBLISH_VAR_AS(m_obHit_SpeedMedium,				Hit_SpeedMedium)
	PUBLISH_VAR_AS(m_obHit_PowerFast,				Hit_PowerFast)
	PUBLISH_VAR_AS(m_obHit_PowerMedium,				Hit_PowerMedium)
	PUBLISH_VAR_AS(m_obHit_RangeFast,				Hit_RangeFast)
	PUBLISH_VAR_AS(m_obHit_RangeMedium,				Hit_RangeMedium)
													
	PUBLISH_VAR_AS(m_obHitStagger_SpeedFast,		HitStagger_SpeedFast)
	PUBLISH_VAR_AS(m_obHitStagger_SpeedMedium,		HitStagger_SpeedMedium)
	PUBLISH_VAR_AS(m_obHitStagger_PowerFast,		HitStagger_PowerFast)
	PUBLISH_VAR_AS(m_obHitStagger_PowerMedium,		HitStagger_PowerMedium)
	PUBLISH_VAR_AS(m_obHitStagger_RangeFast,		HitStagger_RangeFast)
	PUBLISH_VAR_AS(m_obHitStagger_RangeMedium,		HitStagger_RangeMedium)
													
	PUBLISH_VAR_AS(m_obBlock_SpeedFast,				Block_SpeedFast)
	PUBLISH_VAR_AS(m_obBlock_SpeedMedium,			Block_SpeedMedium)
	PUBLISH_VAR_AS(m_obBlock_PowerFast,				Block_PowerFast)
	PUBLISH_VAR_AS(m_obBlock_PowerMedium,			Block_PowerMedium)
	PUBLISH_VAR_AS(m_obBlock_RangeFast,				Block_RangeFast)
	PUBLISH_VAR_AS(m_obBlock_RangeMedium,			Block_RangeMedium)
													
	PUBLISH_VAR_AS(m_obBlockStagger_SpeedFast,		BlockStagger_SpeedFast)
	PUBLISH_VAR_AS(m_obBlockStagger_SpeedMedium,	BlockStagger_SpeedMedium)
	PUBLISH_VAR_AS(m_obBlockStagger_PowerFast,		BlockStagger_PowerFast)
	PUBLISH_VAR_AS(m_obBlockStagger_PowerMedium,	BlockStagger_PowerMedium)
	PUBLISH_VAR_AS(m_obBlockStagger_RangeFast,		BlockStagger_RangeFast)
	PUBLISH_VAR_AS(m_obBlockStagger_RangeMedium,	BlockStagger_RangeMedium)
													
	PUBLISH_VAR_AS(m_obKO_SpeedFast,				KO_SpeedFast)
	PUBLISH_VAR_AS(m_obKO_SpeedMedium,				KO_SpeedMedium)
	PUBLISH_VAR_AS(m_obKO_PowerFast,				KO_PowerFast)
	PUBLISH_VAR_AS(m_obKO_PowerMedium,				KO_PowerMedium)
	PUBLISH_VAR_AS(m_obKO_RangeFast,				KO_RangeFast)
	PUBLISH_VAR_AS(m_obKO_RangeMedium,				KO_RangeMedium)
													
	PUBLISH_VAR_AS(m_obDeath_SpeedFast,				Death_SpeedFast)
	PUBLISH_VAR_AS(m_obDeath_SpeedMedium,			Death_SpeedMedium)
	PUBLISH_VAR_AS(m_obDeath_PowerFast,				Death_PowerFast)
	PUBLISH_VAR_AS(m_obDeath_PowerMedium,			Death_PowerMedium)
	PUBLISH_VAR_AS(m_obDeath_RangeFast,				Death_RangeFast)
	PUBLISH_VAR_AS(m_obDeath_RangeMedium,			Death_RangeMedium)
END_STD_INTERFACE






//-------------------------------------------- Game Audio Config --------------------------------------------

GameAudioConfig::GameAudioConfig ()
{
}

GameAudioConfig::~GameAudioConfig ()
{
	if (GameAudioManager::Exists())
		GameAudioManager::Get().RegisterGameAudioConfig(0);
}

void GameAudioConfig::PostConstruct ()
{
	if (GameAudioManager::Exists())
		GameAudioManager::Get().RegisterGameAudioConfig(this);
}





//-------------------------------------------- Game Audio Manager --------------------------------------------

GameAudioManager::GameAudioManager () :
	m_bFirstFrame(true),
	m_pGameAudioConfig(0)
{
	// Create the audio system
	NT_NEW AudioSystem ();

	// Initialise the audio layers and load in the layer profile
	NT_NEW AudioMixer ();

	// Initialise the audio reverb manager
	NT_NEW AudioReverbManager ();

	/*
	// Load the mixer profiles
	const char* pcMixerProfilePath = "content_neutral/audio/mixer_profiles.xml";

	if ( File::Exists( pcMixerProfilePath ) )
	{
		ntPrintf("GameAudioManager: Loading mixer profiles...\n");

		// Open the XML file in memory
		FileBuffer obFile( pcMixerProfilePath, true );

		if ( !ObjectDatabase::Get().LoadDataObject( &obFile, pcMixerProfilePath ) )
		{
			ntError_p( false, ( "Failed to parse XML file content_neutral/audio/mixer_profiles.xml" ) );
		}
	}
	*/

	// Initialise the ambient sound manager
	NT_NEW AmbientSoundManager ();

	NT_NEW AudioConsole ();

	#ifdef PLATFORM_PC
	//AudioSystem::Get().SetMediaPath("Z:\\HS\\content_pc\\audio");
	AudioSystem::Get().SetMediaPath("../../hs/content_pc/audio");
	#else // PLATFORM_PS3
	AudioSystem::Get().SetMediaPath("content_ps3/audio"); // Note: FIOS prefixes the path with app_home or dev_hdd0 etc automatically so it's not needed
	//AudioSystem::Get().SetMediaPath("/app_home/content_ps3/audio"); // Non-FIOS path
	#endif


	NT_NEW InteractiveMusicManager ();

	#ifdef _INTERACTIVE_MUSIC
	InteractiveMusicManager::Get().Init();
	#endif // _INTERACTIVE_MUSIC

	NT_NEW MusicControlManager();

	NT_NEW CollisionEffectManager();


	/*
	// Random test
	const unsigned int uiSamples=100;
	const unsigned int uiRange=3;
	
	unsigned int uiDistribution [uiRange];
	unsigned int uiLastResult=0;

	ntPrintf("Rand test - Range=%d Samples=%d\n",uiRange,uiSamples);

	for(unsigned int uiCount=0; uiCount<uiRange; ++uiCount)
		uiDistribution[uiCount]=0;

	for(unsigned int uiCount=0; uiCount<uiSamples; ++uiCount)
	{
		unsigned int uiResult=(drand() % (uiRange*100))/100;

		if (uiResult==uiLastResult)
		{
			uiResult+=((drand() % ((uiRange-1)*100))/100)+1;
			uiResult=uiResult % uiRange;
		}

		uiLastResult=uiResult;

		++uiDistribution[uiResult];

		ntPrintf("\tResult=%d\n",uiResult);
	}

	ntPrintf("Summary:\n");

	for(unsigned int uiCount=0; uiCount<uiRange; ++uiCount)
	{
		ntPrintf("\tNumber=%d  Hits=%d\n",uiCount,uiDistribution[uiCount]);
	}

	ntAssert(0);
	//*/
}

GameAudioManager::~GameAudioManager ()
{
	InteractiveMusicManager::Kill();
	MusicControlManager::Kill();

	AudioConsole::Kill();

	CollisionEffectManager::Kill();
	AmbientSoundManager::Kill();
	AudioReverbManager::Kill();
	AudioMixer::Kill();
	AudioSystem::Kill();
}

void GameAudioManager::LoadResources ()
{
	if (!g_ShellOptions->m_bAudioEngine)
		return;

	#ifdef _SIMPLE_TEST
	AudioSystem::Get().LoadEventFile("examples.fev",0);
	#endif // _SIMPLE_TEST

	char acPath [MAX_PATH];

	// ----- Load audio config -----

	Util::SetToNeutralResources();
	Util::GetFiosFilePath( "audio/audioconfig.xml", acPath );

	if ( File::Exists( acPath ) )
	{
		// Open the XML file in memory
		FileBuffer obFile( acPath, true );

		if ( ObjectDatabase::Get().LoadDataObject( &obFile, acPath ) )
		{
			ntPrintf("GameAudioManager: Loaded audio config - %s\n",acPath);
		}
		else
		{
			ntPrintf("GameAudioManager: Failed to load audio config - %s\n",acPath);
		}
	}
	else
	{
		ntPrintf("GameAudioManager: Unable to find %s\n",acPath);
	}

	// ----- Load the mixer profiles -----

	char acMixerProfilePath [MAX_PATH];
	Util::SetToNeutralResources();
	Util::GetFiosFilePath( "audio/mixer_profiles.xml", acMixerProfilePath );

	if ( File::Exists( acMixerProfilePath ) )
	{
		// Open the XML file in memory
		FileBuffer obFile( acMixerProfilePath, true );

		if ( ObjectDatabase::Get().LoadDataObject( &obFile, "audio/mixer_profiles.xml" ) )
		{
			ntPrintf("GameAudioManager: Loaded mixer profile - %s\n",acMixerProfilePath);			
		}
		else
		{
			ntPrintf("GameAudioManager: Failed to load mixer profile - %s\n",acMixerProfilePath);
		}
	}
	else
	{
		ntPrintf("GameAudioManager: Unable to find %s\n",acMixerProfilePath);
	}

	AudioMixer::Get().InitCategories();

	#ifdef _INTERACTIVE_MUSIC

	// ----- Load music resources -----

	/*
	char acMusicBankPath [MAX_PATH];
	Util::SetToPlatformResources();
	Util::GetFiosFilePath( "audio//music.wb", acMusicBankPath );

	InteractiveMusicManager::Get().LoadMusicBank(acMusicBankPath,true);
	*/

	// Set the media path for the music system

	char acMusicPath [MAX_PATH];
	Util::SetToNeutralResources();

	#ifdef PLATFORM_PC
	Util::GetFiosFilePath( "../../hs/content_neutral/audio/music", acMusicPath );
	#else // PLATFORM_PS3
	strcpy(acMusicPath, "content_neutral/audio/music");
	#endif

	InteractiveMusicManager::Get().SetMediaPath(acMusicPath); // Set the media path for the music files
	
	/*
	// Load the music xml file
	char acMusicDefPath [MAX_PATH];
	Util::SetToNeutralResources();
	Util::GetFiosFilePath( "audio/music.xml", acMusicDefPath );

	if ( File::Exists( acMixerProfilePath ) )
	{
		// Open the XML file in memory
		FileBuffer obFile( acMusicDefPath, true );

		if ( ObjectDatabase::Get().LoadDataObject( &obFile, acMusicDefPath ) )
		{
			ntPrintf("GameAudioManager: Loaded music defs - %s\n",acMusicDefPath);			
		}
		else
		{
			ntPrintf("GameAudioManager: Failed to load music defs - %s\n",acMusicDefPath);
		}
	}
	else
	{
		ntPrintf("GameAudioManager: Unable to find %s\n",acMusicDefPath);
	}
	*/

	//g_AudioDecoder=NT_NEW AudioDecoder("test.mp3");

	/*
	AudioDecoder obDecoder("Z:\\hs\\content_pc\\audio\\test.mp3",false);

	if (obDecoder.IsValid())
	{
		uint8_t* pBuffer=NT_NEW uint8_t [64*1024*1024];

		double dTime=CTimer::Get().GetSystemTime();

		obDecoder.GetData(pBuffer,16*1024*1024);

		dTime=CTimer::Get().GetSystemTime() - dTime;

		ntPrintf("Time elapsed=%f\n",dTime);
	}
	*/

	#endif // _INTERACTIVE_MUSIC
}

void GameAudioManager::Update ()
{
	if (!g_ShellOptions->m_bAudioEngine)
		return;

	if (m_bFirstFrame)
	{
		InteractiveMusicManager::Get().SetPause(false);

		m_bFirstFrame=false;
	}

	MusicControlManager::Get().Update();

#ifndef _RELEASE // Debugging stuff

	CInputKeyboard &obKeyboard=CInputHardware::Get().GetKeyboard();
	UNUSED( obKeyboard );

	#ifdef _INTERACTIVE_MUSIC
	
	if (obKeyboard.IsKeyPressed(KEYC_F1,KEYM_CTRL | KEYM_ALT)) InteractiveMusicManager::Get().SetIntensity(0.1f);
	if (obKeyboard.IsKeyPressed(KEYC_F2,KEYM_CTRL | KEYM_ALT)) InteractiveMusicManager::Get().SetIntensity(0.2f);
	if (obKeyboard.IsKeyPressed(KEYC_F3,KEYM_CTRL | KEYM_ALT)) InteractiveMusicManager::Get().SetIntensity(0.3f);
	if (obKeyboard.IsKeyPressed(KEYC_F4,KEYM_CTRL | KEYM_ALT)) InteractiveMusicManager::Get().SetIntensity(0.4f);
	if (obKeyboard.IsKeyPressed(KEYC_F5,KEYM_CTRL | KEYM_ALT)) InteractiveMusicManager::Get().SetIntensity(0.5f);
	if (obKeyboard.IsKeyPressed(KEYC_F6,KEYM_CTRL | KEYM_ALT)) InteractiveMusicManager::Get().SetIntensity(0.6f);
	if (obKeyboard.IsKeyPressed(KEYC_F7,KEYM_CTRL | KEYM_ALT)) InteractiveMusicManager::Get().SetIntensity(0.7f);
	if (obKeyboard.IsKeyPressed(KEYC_F8,KEYM_CTRL | KEYM_ALT)) InteractiveMusicManager::Get().SetIntensity(0.8f);
	if (obKeyboard.IsKeyPressed(KEYC_F9,KEYM_CTRL | KEYM_ALT)) InteractiveMusicManager::Get().SetIntensity(0.9f);
	if (obKeyboard.IsKeyPressed(KEYC_F10,KEYM_CTRL | KEYM_ALT)) InteractiveMusicManager::Get().SetIntensity(1.0f);
	if (obKeyboard.IsKeyPressed(KEYC_F11,KEYM_CTRL | KEYM_ALT)) InteractiveMusicManager::Get().SetIntensity(0.0f);
	#endif // _INTERACTIVE_MUSIC


	#ifdef _SIMPLE_TEST
	if (obKeyboard.IsKeyPressed(KEYC_Y))
	{
		AudioSystem::Get().SimpleTest();
	}
	#endif // _SIMPLE_TEST

#endif // _RELEASE

	CGatso::Start("AudioMixer:DoWork");
	AudioMixer::Get().DoWork();
	CGatso::Stop("AudioMixer:DoWork");

	CGatso::Start("AudioReverbManager::DoWork");
	AudioReverbManager::Get().Update();
	CGatso::Stop("AudioReverbManager::DoWork");

	CGatso::Start("AmbientSoundManager:Update");
	AmbientSoundManager::Get().Update();
	CGatso::Stop("AmbientSoundManager:Update");

	CGatso::Start("AudioSystem:DoWork");

	CMatrix camPos( CONSTRUCT_IDENTITY );
	if ( ShellMain::Get().HaveLoadedLevel() )
		camPos = CamMan_Public::GetCurrMatrix();

	AudioSystem::Get().SetListenerProperties(0,camPos);
	AudioSystem::Get().DoWork();
	CGatso::Stop("AudioSystem:DoWork");

	AudioConsole::Get().Update();
}


void GameAudioManager::DebugRender ()
{
	AudioConsole::Get().RenderUpdate();
}

void GameAudioManager::OnFrontend ()
{
	//printf("***** GameAudioManager::OnFrontend *****\n");

	//AudioMixer::Get().PauseAll(false); // Ensure everything is unpaused
	
	MusicControlManager::Get().Suspend(); // Ensure the music control manager is suspended

	AudioSystem::Get().Sound_StopAll(false); // Stop all existing sounds

	AmbientSoundManager::Get().SetActive(false); // Disable ambient sounds

	if (m_pGameAudioConfig)
		AudioMixer::Get().SetProfile(m_pGameAudioConfig->m_obMixerProfile_Frontend,1.0f);
}

void GameAudioManager::OnLevelStart ()
{
	//printf("***** GameAudioManager::OnLevelStart *****\n");

	MusicControlManager::Get().Resume(); // Re-activate the music control manager half a second after the level has started

	AmbientSoundManager::Get().SetActive(true); // This should only be active when a game level is running

	if (m_pGameAudioConfig)
		AudioMixer::Get().SetProfile(m_pGameAudioConfig->m_obMixerProfile_LevelStart,1.0f);
}

void GameAudioManager::OnLevelExit ()
{
	//printf("***** GameAudioManager::OnLevelExit *****\n");

	MusicControlManager::Get().Suspend(); // Suspend the music control manager on level exit

	AudioSystem::Get().Sound_ResetCallbacks(); // Clear all callbacks that may have been set in the previous level

	AmbientSoundManager::Get().SetActive(false); // We are exiting the game level so ensure the ambient sound stuff is disabled

	InteractiveMusicManager::Get().StopAll(0.1f);

	if (m_pGameAudioConfig)
		AudioMixer::Get().SetProfile(m_pGameAudioConfig->m_obMixerProfile_LevelExit,0.1f);
}

void GameAudioManager::OnPause (bool bActive)
{
	//printf("***** GameAudioManager::OnPause - %d *****\n",bActive);

	if (m_pGameAudioConfig)
	{
		if (bActive)
		{
			AudioMixer::Get().SetProfile(m_pGameAudioConfig->m_obMixerProfile_GamePause,0.2f); // We make the pause profile active at the same time
		}
		else
		{
			AudioMixer::Get().PauseAll(false); // Unpause everything

			AudioMixer::Get().SetPreviousProfile(0.2f); // Go back to the previous profile
		}
	}
}

void GameAudioManager::OnOSD (bool bActive)
{
	//printf("GameAudioManager::OnOSD\n");

	if (m_pGameAudioConfig)
	{
		if (bActive)
		{
			AudioMixer::Get().SetProfile(m_pGameAudioConfig->m_obMixerProfile_OSDPause,0.5f); // We make the pause profile active at the same time
		}
		else
		{
			AudioMixer::Get().SetPreviousProfile(0.5f); // Go back to the previous profile
		}
	}
}

void GameAudioManager::TriggerCombatHitSound (const CEntity* pobAttacker,const CEntity* pobReceiver,COMBAT_STATE eCombatState,ATTACK_CLASS eAttackClass)
{
#ifdef _ENABLE_COMBAT_HIT_SOUNDS

	//printf("TriggerCombatHitSound: 0x%x  CS=%d  AC=%d\n",pobReceiver,eCombatState,eAttackClass);

	const unsigned int uiAttackerClumpHash = pobAttacker->GetClumpFileHash();
	const unsigned int uiReceiverClumpHash = pobReceiver->GetClumpFileHash();
	
	for(ntstd::List<CombatHitSoundDefinition*>::iterator obIt=m_obCombatHitSoundDefList.begin(); obIt!=m_obCombatHitSoundDefList.end(); ++obIt)
	{
		if ((uiAttackerClumpHash==(*obIt)->GetAttackerHash().GetHash() || (*obIt)->GetAttackerHash().IsNull()) &&
            (uiReceiverClumpHash==(*obIt)->GetReceiverHash().GetHash() || (*obIt)->GetReceiverHash().IsNull())) // Found an appropriate definition
		{
			(*obIt)->TriggerSound(pobReceiver,eCombatState,eAttackClass);
			return;
		}
	}

#else

	UNUSED(pobReceiver);
	UNUSED(eCombatState);
	UNUSED(eAttackClass);

#endif // _ENABLE_COMBAT_HIT_SOUNDS
}

void GameAudioManager::AddCombatHitSoundDef (CombatHitSoundDefinition* pobSoundDef)
{
	for(ntstd::List<CombatHitSoundDefinition*>::iterator obIt=m_obCombatHitSoundDefList.begin(); obIt!=m_obCombatHitSoundDefList.end(); ++obIt)
	{
		if (pobSoundDef==(*obIt)) // Prevent adding the same def multiple times
			return;
	}

	m_obCombatHitSoundDefList.push_back(pobSoundDef); // Add it
}

void GameAudioManager::RemoveCombatHitSoundDef (CombatHitSoundDefinition* pobSoundDef)
{
	m_obCombatHitSoundDefList.remove(pobSoundDef);
}


//-------------------------------------------- Entity Audio Channel --------------------------------------------


EntityAudioChannel::EntityAudioChannel (const CEntity* pobParent) :
	m_pobParentEntity(pobParent),
	m_pobFootstepProperties(0),
	m_pCallbackFunc(0),
	m_pCallbackData(0),
	m_fAudioRadius(0.0f)
{
	ntAssert(pobParent);
	ntAssert(pobParent->GetRootTransformP());

	m_pobRootTransform=pobParent->GetRootTransformP();

	for(int i=0; i<TOTAL_CHANNELS; ++i)
	{
		m_astChannelInfo[i].id=0;
		m_astChannelInfo[i].iGroupIndex=-1;
		m_astChannelInfo[i].iEventIndex=-1;
	}
}

void EntityAudioChannel::RegisterFootstepDefinition (const CHashedString& obName)
{
	m_pobFootstepProperties=ObjectDatabase::Get().GetPointerFromName<FootstepProperties*>(obName);
	m_dFootstepTime=0.0;
}

void EntityAudioChannel::ProcessFootstepEvent (FOOTSTEP eType,float fVolume)
{
	if (!m_pobFootstepProperties)
		return;

	double dGameTime=CTimer::Get().GetGameTime();

	if (dGameTime > m_dFootstepTime && m_pobFootstepProperties->ProcessEvent(eType,m_pobParentEntity,fVolume))
	{
		m_dFootstepTime=dGameTime + 0.1f; // Ensure that the footstep sounds can't occur at intervals faster than 0.1 seconds
	}
}

bool EntityAudioChannel::Play (ENTITY_AUDIO_CHANNEL eChannel,const CKeyString& obGroup,const CKeyString& obEvent,const CPoint& obPosition,float fVolume,float fPitch)
{
	if (!DoChannelLogic(eChannel,obGroup.GetValue(),obEvent.GetValue()))
		return false; // Channel logic has decided to skip this play request

	unsigned int id;

	if (AudioSystem::Get().Sound_Prepare(id,obGroup.GetString(),obEvent.GetString()))
	{
		CPoint obNewPosition=obPosition * m_pobRootTransform->GetWorldMatrix();

		if (m_pCallbackFunc) // We want to set a completion callback on this sound
		{
			AudioSystem::Get().Sound_SetCallback(id,m_pCallbackFunc,m_pCallbackData);
			m_pCallbackFunc=0;
			m_pCallbackData=0;
		}

		AudioSystem::Get().Sound_SetPosition(id,obNewPosition,m_fAudioRadius);
		AudioSystem::Get().Sound_SetVolume(id,fVolume);
		AudioSystem::Get().Sound_SetPitch(id,fPitch);
		
		if (AudioSystem::Get().Sound_Play(id)) // If the sound plays successfully, update the channel with information about this sound instance
		{
			m_astChannelInfo[eChannel].id=id;
			m_astChannelInfo[eChannel].iGroupIndex=obGroup.GetValue();
			m_astChannelInfo[eChannel].iEventIndex=obEvent.GetValue();
		}

		return true;
	}

	return false; // Unable to play the sound, possibly due to it not existing or we have reached the instance limit
}

void EntityAudioChannel::Stop (ENTITY_AUDIO_CHANNEL eChannel)
{
	AudioSystem::Get().Sound_Stop(m_astChannelInfo[eChannel].id);
	m_astChannelInfo[eChannel].id=0;
	m_astChannelInfo[eChannel].iGroupIndex=-1;
	m_astChannelInfo[eChannel].iEventIndex=-1;
}

bool EntityAudioChannel::DoChannelLogic (ENTITY_AUDIO_CHANNEL eChannel,int iGroupIndex,int iEventIndex)
{
	switch(eChannel)
	{
		/*
		case CHANNEL_ACTION:
		{
			Stop(CHANNEL_ACTION_LOOPING); // Stop any sound that might be playing on the action looping channel
			break;
		}
		*/

		case CHANNEL_ACTION_STOPLOOPING:
		{
			Stop(CHANNEL_ACTION_LOOPING); // Stop whatever is currently playing in the action looping channel
			break;
		}

		case CHANNEL_ACTION_LOOPING:
		{
			/*
			if (!IsPlaying(CHANNEL_ACTION_LOOPING)) // Action looping sound is not playing
			{
				Stop(CHANNEL_ACTION_LOOPING);
			}
			else // Action looping sound is playing
			{
				if (iGroupIndex==m_astChannelInfo[CHANNEL_ACTION_LOOPING].iGroupIndex && iEventIndex==m_astChannelInfo[CHANNEL_ACTION_LOOPING].iEventIndex)
				{
					return false; // The same sound is already playing, so ignore this sound request and let it keep playing
				}

				Stop(CHANNEL_ACTION_LOOPING);
			}
			*/

			if (IsPlaying(CHANNEL_ACTION_LOOPING)) // Action looping sound is already playing
			{
				if (iGroupIndex==m_astChannelInfo[CHANNEL_ACTION_LOOPING].iGroupIndex && iEventIndex==m_astChannelInfo[CHANNEL_ACTION_LOOPING].iEventIndex)
				{
					return false; // The same sound is already playing, so ignore this sound request and let it keep playing
				}

				Stop(CHANNEL_ACTION_LOOPING); // We want to play a new action looping sound, so stop existing
			}

			break;
		}

		case CHANNEL_VOICE_HIGHPRI:
		{
			if (IsPlaying(CHANNEL_VOICE_HIGHPRI)) // We already have something playing in the voice high priority channel, so ignore
				return false;
			
			// Stop all voice channels below high priority
			Stop(CHANNEL_VOICE);
			Stop(CHANNEL_VOICE_NORMAL);
			break;	
		}

		case CHANNEL_VOICE_NORMAL:
		{
			if (IsPlaying(CHANNEL_VOICE_HIGHPRI) || IsPlaying(CHANNEL_VOICE_NORMAL)) // We already have something playing in either the high/normal priority channels, so ignore
				return false;

			// Stop all voice channels below normal priority
			Stop(CHANNEL_VOICE);
			break;
		}

		case CHANNEL_VOICE:
		{
			if (IsPlaying(CHANNEL_VOICE_HIGHPRI) || IsPlaying(CHANNEL_VOICE_NORMAL)) // We already have something playing in either the high/normal priority channels, so ignore
				return false;

			// Check to see if the same sound is already playing
			if (iGroupIndex==m_astChannelInfo[CHANNEL_VOICE].iGroupIndex && iEventIndex==m_astChannelInfo[CHANNEL_VOICE].iEventIndex)
			{
				if (IsPlaying(CHANNEL_VOICE))
					return false; // The same sound is already playing, so ignore this sound request and let it keep playing
			}

			Stop(CHANNEL_VOICE); // Stop anything the entity might currently be saying
			//obPosition.Y()=1.4f;
			break;
		}
		case CHANNEL_IMPACT:
		{
			Stop(CHANNEL_ACTION);
			Stop(CHANNEL_ACTION_LONG);
			Stop(CHANNEL_ACTION_STOPLOOPING);
			Stop(CHANNEL_ACTION_LOOPING);
			Stop(CHANNEL_VOICE);
			break;
		}
		default:
			break;
	}

	return true; // It's okay to go ahead and play the sound
}

bool EntityAudioChannel::IsPlaying (ENTITY_AUDIO_CHANNEL eChannel) const
{
	return AudioSystem::Get().Sound_IsPlaying(m_astChannelInfo[eChannel].id);
}



//---------------------------------------------------------------------------------------------------------------------------------------------------

FootstepProperties::FootstepProperties () :
	m_pobDefaultEffect(0)
{
}

void FootstepProperties::PostConstruct(void)
{
	// Find the default effect
	for(ntstd::List<FootstepEffect*>::iterator obIt=m_obFootstepEffectList.begin(); obIt!=m_obFootstepEffectList.end(); ++obIt)
	{
		if ((*obIt)->m_obSurfaceMaterial.IsNull())
		{
			m_pobDefaultEffect=(*obIt);
			break;
		}
	}
}

bool FootstepProperties::EditorChangeValue(CallBackParameter /*obItem*/,CallBackParameter /*obValue*/)
{
	PostConstruct();

	return true;
}

unsigned int FootstepProperties::ProcessEvent (FOOTSTEP eType,const CEntity* pobEntity,float fVolume)
{
	Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*)(pobEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER ));

	if (ctrl && ctrl->IsOnGround()) // Make sure the character is actually on the ground
	{
		// Get the material of the surface the entity is currently standing on
		Physics::psPhysicsMaterial* pMaterial=GetSurfaceMaterial(pobEntity->GetPosition());

		// Find the appropriate effect definition
		FootstepEffect* pobEffect=0;

		if (pMaterial)
		{
			unsigned int uiSurfaceMaterial=pMaterial->GetMaterialId();

			for(ntstd::List<FootstepEffect*>::iterator obIt=m_obFootstepEffectList.begin(); obIt!=m_obFootstepEffectList.end(); ++obIt)
			{
				if ((*obIt)->m_obSurfaceMaterial.GetHash() == uiSurfaceMaterial)
				{
					pobEffect=*obIt; // Found it
					break;
				}
			}
		}
		
		if (!pobEffect) // Unable to find effect definition, so use the default
			pobEffect=m_pobDefaultEffect;

		if (!pobEffect) // We still don't have an effect definition, so unable to process this event
			return 0;

		// Get the appropriate sound string
		const char* pcSound;

		switch(eType)
		{
			case LEFT_STEP:
			case RIGHT_STEP:
				pcSound=pobEffect->m_obSingleFootSound.c_str(); break;

			case LEFT_STEP_HEAVY:
			case RIGHT_STEP_HEAVY:
				pcSound=pobEffect->m_obDoubleFootSound.c_str(); break;

			case LEFT_SLIDE_SHORT:
			case RIGHT_SLIDE_SHORT:
				pcSound=pobEffect->m_obSlideShortSound.c_str(); break;

			case LEFT_SLIDE_MEDIUM:
			case RIGHT_SLIDE_MEDIUM:
				pcSound=pobEffect->m_obSlideMediumSound.c_str(); break;

			case LEFT_SLIDE_LONG:
			case RIGHT_SLIDE_LONG:
				pcSound=pobEffect->m_obSlideLongSound.c_str(); break;

			case CUSTOM1:
				pcSound=pobEffect->m_obCustom1Sound.c_str(); break;

			case CUSTOM2:
				pcSound=pobEffect->m_obCustom2Sound.c_str(); break;

			default:
				pcSound=0; break; // Shouldn't ever get here...
		}

		unsigned int id;

		if (AudioSystem::Get().Sound_Prepare(id,pcSound)) // Fire off our sound effect
		{
			CDirection obVelocity(ctrl->GetLinearVelocity()); // Get the velocity of the character

			// Get the foot position of the character
			CPoint obFootPosition(pobEntity->GetPosition()); // Use the root position for now
			//obFootPosition=pobEntity->GetCharacterTransformP(CHARACTER_BONE_L_BALL)->GetWorldTranslation(); // Left foot position
			//obFootPosition=pobEntity->GetCharacterTransformP(CHARACTER_BONE_R_BALL)->GetWorldTranslation(); // Right foot position

			float fAudioRadius;

			if (pobEntity->GetEntityAudioChannel())
				fAudioRadius=pobEntity->GetEntityAudioChannel()->GetAudioRadius();
			else
				fAudioRadius=0.0f;

			// Fire off the sound
			AudioSystem::Get().Sound_SetPosition(id,obFootPosition,fAudioRadius);
			AudioSystem::Get().Sound_SetParameterValue(id,"speed",obVelocity.Length());
			AudioSystem::Get().Sound_SetVolume(id,fVolume);
			AudioSystem::Get().Sound_Play(id);
		}

		return id; // Return the instance ID of the sound
	}

	return 0; // Unable to trigger the effect
}

Physics::psPhysicsMaterial* FootstepProperties::GetSurfaceMaterial (const CPoint& obFootPosition)
{
	Physics::RaycastCollisionFlag flags; 
	flags.base = 0;
	flags.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
	flags.flags.i_collide_with = Physics::LARGE_INTERACTABLE_BIT;

	hkWorldRayCastInput input;
	hkWorldRayCastOutput output;

	input.m_filterInfo = flags.base;

	input.m_from(0)=obFootPosition.X();
	input.m_from(1)=obFootPosition.Y() + 0.1f;
	input.m_from(2)=obFootPosition.Z();

	input.m_to(0)=input.m_from(0);
	input.m_to(1)=obFootPosition.Y() - 0.5f;
	input.m_to(2)=input.m_from(2);

	Physics::CPhysicsWorld::Get().GetHavokWorldP()->castRay(input,output);

	if (output.hasHit())
		return Physics::Tools::GetMaterial(output);

	return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------

CombatHitSoundDefinition::CombatHitSoundDefinition ()
{
	GameAudioManager::Get().AddCombatHitSoundDef(this);
}

CombatHitSoundDefinition::~CombatHitSoundDefinition ()
{
	if (GameAudioManager::Exists())
		GameAudioManager::Get().RemoveCombatHitSoundDef(this);
}

void CombatHitSoundDefinition::TriggerSound (const CEntity* pobReceiver,COMBAT_STATE eCombatState,ATTACK_CLASS eAttackClass)
{
	switch(eCombatState)
	{
		case CS_RECOILING: // Hit
		{
			switch(eAttackClass)
			{
				case AC_SPEED_FAST: PlaySound(pobReceiver,m_obHit_SpeedFast); break;
				case AC_SPEED_MEDIUM: PlaySound(pobReceiver,m_obHit_SpeedMedium); break;
				case AC_POWER_FAST: PlaySound(pobReceiver,m_obHit_PowerFast); break;
				case AC_POWER_MEDIUM: PlaySound(pobReceiver,m_obHit_PowerMedium); break;
				case AC_RANGE_FAST: PlaySound(pobReceiver,m_obHit_RangeFast); break;
				case AC_RANGE_MEDIUM: PlaySound(pobReceiver,m_obHit_RangeMedium); break;
				default: break;
			}

			break;
		}

		case CS_IMPACT_STAGGERING: // Hit stagger
		{
			switch(eAttackClass)
			{
				case AC_SPEED_FAST: PlaySound(pobReceiver,m_obHitStagger_SpeedFast); break;
				case AC_SPEED_MEDIUM: PlaySound(pobReceiver,m_obHitStagger_SpeedMedium); break;
				case AC_POWER_FAST: PlaySound(pobReceiver,m_obHitStagger_PowerFast); break;
				case AC_POWER_MEDIUM: PlaySound(pobReceiver,m_obHitStagger_PowerMedium); break;
				case AC_RANGE_FAST: PlaySound(pobReceiver,m_obHitStagger_RangeFast); break;
				case AC_RANGE_MEDIUM: PlaySound(pobReceiver,m_obHitStagger_RangeMedium); break;
				default: break;
			}

			break;
		}

		case CS_DEFLECTING: // Block
		{
			switch(eAttackClass)
			{
				case AC_SPEED_FAST: PlaySound(pobReceiver,m_obBlock_SpeedFast); break;
				case AC_SPEED_MEDIUM: PlaySound(pobReceiver,m_obBlock_SpeedMedium); break;
				case AC_POWER_FAST: PlaySound(pobReceiver,m_obBlock_PowerFast); break;
				case AC_POWER_MEDIUM: PlaySound(pobReceiver,m_obBlock_PowerMedium); break;
				case AC_RANGE_FAST: PlaySound(pobReceiver,m_obBlock_RangeFast); break;
				case AC_RANGE_MEDIUM: PlaySound(pobReceiver,m_obBlock_RangeMedium); break;
				default: break;
			}

			break;
		}

		case CS_BLOCK_STAGGERING: // Block stagger
		{
			switch(eAttackClass)
			{
				case AC_SPEED_FAST: PlaySound(pobReceiver,m_obBlock_SpeedFast); break;
				case AC_SPEED_MEDIUM: PlaySound(pobReceiver,m_obBlock_SpeedMedium); break;
				case AC_POWER_FAST: PlaySound(pobReceiver,m_obBlock_PowerFast); break;
				case AC_POWER_MEDIUM: PlaySound(pobReceiver,m_obBlock_PowerMedium); break;
				case AC_RANGE_FAST: PlaySound(pobReceiver,m_obBlock_RangeFast); break;
				case AC_RANGE_MEDIUM: PlaySound(pobReceiver,m_obBlock_RangeMedium); break;
				default: break;
			}

			break;
		}

		case CS_KO: // KO'ed
		case CS_DEAD: // KO'ed
		{
			switch(eAttackClass)
			{
				case AC_SPEED_FAST: PlaySound(pobReceiver,m_obKO_SpeedFast); break;
				case AC_SPEED_MEDIUM: PlaySound(pobReceiver,m_obKO_SpeedMedium); break;
				case AC_POWER_FAST: PlaySound(pobReceiver,m_obKO_PowerFast); break;
				case AC_POWER_MEDIUM: PlaySound(pobReceiver,m_obKO_PowerMedium); break;
				case AC_RANGE_FAST: PlaySound(pobReceiver,m_obKO_RangeFast); break;
				case AC_RANGE_MEDIUM: PlaySound(pobReceiver,m_obKO_RangeMedium); break;
				default: break;
			}

			break;
		}

		case CS_DYING: // Dead
		{
			switch(eAttackClass)
			{
				case AC_SPEED_FAST: PlaySound(pobReceiver,m_obDeath_SpeedFast); break;
				case AC_SPEED_MEDIUM: PlaySound(pobReceiver,m_obDeath_SpeedMedium); break;
				case AC_POWER_FAST: PlaySound(pobReceiver,m_obDeath_PowerFast); break;
				case AC_POWER_MEDIUM: PlaySound(pobReceiver,m_obDeath_PowerMedium); break;
				case AC_RANGE_FAST: PlaySound(pobReceiver,m_obDeath_RangeFast); break;
				case AC_RANGE_MEDIUM: PlaySound(pobReceiver,m_obDeath_RangeMedium); break;
				default: break;
			}

			break;
		}

		default:
			break;
	}
}

void CombatHitSoundDefinition::PlaySound (const CEntity* pobEntity,ntstd::String& obSound)
{
	//printf("CombatHitSoundDefinition: Trigger -> 0x%x  sound=%s\n",entity,sound.c_str());
	
	unsigned int id;

	if (AudioSystem::Get().Sound_Prepare(id,obSound.c_str()))
	{
		AudioSystem::Get().Sound_SetPosition(id,pobEntity->GetPosition());
		AudioSystem::Get().Sound_Play(id);
	}
}




