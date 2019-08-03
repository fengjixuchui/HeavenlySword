/***************************************************************************************************
*
*	FILE			audiobindings.cpp
*
*	DESCRIPTION		
*
***************************************************************************************************/

#include "audiobindings.h"

#include "game/luaglobal.h"
#include "game/luaattrtable.h"
#include "game/luaexptypes.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "anim/hierarchy.h"

#include "audio/gameaudiocomponents.h"
#include "audio/audiomixer.h"
#include "audio/ambientsound.h"
#include "audio/audiosystem.h"
#include "audio/imusic.h"
#include "audio/musiccontrol.h"

#include "game/playeroptions.h"


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.SetAmbience( string, bool )
// DESCRIPTION: Activate a pre-defined ambient sound
//-------------------------------------------------------------------------------------------------
static void Audio_SetAmbience( const char* pcName, bool bEnable )
{
	if (bEnable)
		AmbientSoundManager::Get().Activate(pcName);
	else
		AmbientSoundManager::Get().Deactivate(pcName);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.IsAmbienceEnabled( string )
// DESCRIPTION: Determine if an ambient sound
//-------------------------------------------------------------------------------------------------
static bool Audio_IsAmbienceEnabled ( const char* pcName )
{
	return AmbientSoundManager::Get().IsActive(pcName);

}



//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.PlayEntitySound
// DESCRIPTION: 
//-------------------------------------------------------------------------------------------------
static int Audio_PlayEntitySound (NinjaLua::LuaState* pobState)
{
	// Audio_PlayerEntitySound(entity,CHANNEL,SPATIALIZATION,soundbank,soundcue)

	NinjaLua::LuaStack obArgs( pobState );

	CEntity* pobEntity=0;

	if (obArgs[1].Is<CEntity*>())
	{
		pobEntity=obArgs[1].Get<CEntity*>();
	}
	else
	{
		NinjaLua::LuaObject obNewSelf( obArgs[1] );
		LuaAttributeTable* pAttrTable = LuaAttributeTable::GetFromLuaState( *obNewSelf.GetState() );

		if (pAttrTable)
		{
			pobEntity=pAttrTable->GetEntity();
		}
	}

	if (!pobEntity)
	{
		ntPrintf("Audio_PlayEntitySound: Invalid entity specified\n");
		return 0;
	}

	if (!obArgs[2].IsInteger())
	{
		ntPrintf("Audio_PlayEntitySound: Argument #2 is invalid\n");
		return 0;
	}

	if (!obArgs[3].IsInteger())
	{
		ntPrintf("Audio_PlayEntitySound: Argument #3 is invalid\n");
		return 0;
	}

	if (!obArgs[4].IsString())
	{
		ntPrintf("Audio_PlayEntitySound: Argument #4 is invalid\n");
		return 0;
	}

	if (!obArgs[5].IsString())
	{
		ntPrintf("Audio_PlayEntitySound: Argument #5 is invalid\n");
		return 0;
	}

	if (pobEntity)
		//pobEntity->GetEntityAudioChannel()->Play((ENTITY_AUDIO_CHANNEL)obArgs[2].GetInteger(),(SPATIALIZATION)obArgs[3].GetInteger(),obArgs[4].GetString(),obArgs[5].GetString());
		pobEntity->GetEntityAudioChannel()->Play((ENTITY_AUDIO_CHANNEL)obArgs[2].GetInteger(),obArgs[4].GetString(),obArgs[5].GetString());
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.StopEntitySound
// DESCRIPTION: Stop a particular channel on an entity
//-------------------------------------------------------------------------------------------------
static int Audio_StopEntitySound (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack obArgs( pobState );

	if (obArgs[1].Is<CEntity*>() && obArgs[2].IsInteger())
	{
		CEntity* pobEntity=obArgs[1].Get<CEntity*>();
		
		if (pobEntity)
		{
			pobEntity->GetEntityAudioChannel()->Stop((ENTITY_AUDIO_CHANNEL)obArgs[2].GetInteger());
			return 0;
		}
	}

	ntPrintf("Audio.StopEntitySound: Invalid arguments\n");

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.PlayChatterVO
// DESCRIPTION: Play some chatterbox sound
//-------------------------------------------------------------------------------------------------
static void Audio_ChatterVO(CEntity* pEnt, const char* pcCue, const char* pcBank, NinjaLua::LuaObject msg)
{
	/*
	if(!pEnt)
	{
		ntPrintf("Audio_PlayEntitySound: Invalid entity specified\n");
		return;
	}

	pEnt->GetEntityAudioChannel()->SetCallback(msg);
	pEnt->GetEntityAudioChannel()->Play(CHANNEL_VOICE_HIGHPRI, SPATIALIZATION_HRTF, pcBank, pcCue);
	*/

	UNUSED(pEnt);
	UNUSED(pcCue);
	UNUSED(pcBank);
	UNUSED(msg);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.LoadProject
// DESCRIPTION: Load an FMOD fev file.
//-------------------------------------------------------------------------------------------------
static int Audio_LoadProject (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack obArgs( pobState );

	if (obArgs[1].IsString())
	{
		int iGroupID=0;

		if (obArgs[2].IsInteger())
		{
			iGroupID=obArgs[2].GetInteger();
		}

		AudioSystem::Get().LoadProject(obArgs[1].GetString(),iGroupID);
	}
	#ifndef _RELEASE
	else
	{
		ntPrintf("Audio.LoadProject: Invalid arguments\n");
	}
	#endif // _RELEASE

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.LoadEventFile
// DESCRIPTION: Load an FMOD fev file.
//-------------------------------------------------------------------------------------------------
static int Audio_LoadEventFile (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack obArgs( pobState );

	if (obArgs[1].IsString())
	{
		int iGroupID=0;

		if (obArgs[2].IsInteger())
		{
			iGroupID=obArgs[2].GetInteger();
		}

		AudioSystem::Get().LoadProject(obArgs[1].GetString(),iGroupID);
	}
	#ifndef _RELEASE
	else
	{
		ntPrintf("Audio.LoadProject: Invalid arguments\n");
	}
	#endif // _RELEASE

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.UnloadProject
// DESCRIPTION: Unload a particular project.
//-------------------------------------------------------------------------------------------------
static int Audio_UnloadProject (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack obArgs( pobState );

	if (obArgs[1].IsString())
	{
		AudioSystem::Get().UnloadProject(obArgs[1].GetString());
	}
	#ifndef _RELEASE
	else
	{
		ntPrintf("Audio.UnloadProject: Invalid arguments\n");
	}
	#endif // _RELEASE

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.UnloadProjectGroup
// DESCRIPTION: Unload a particular type of project.
//-------------------------------------------------------------------------------------------------
static int Audio_UnloadProjectGroup (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack obArgs( pobState );

	if (obArgs[1].IsInteger())
	{
		AudioSystem::Get().UnloadProjectGroup(obArgs[2].GetInteger());
	}
	#ifndef _RELEASE
	else
	{
		ntPrintf("Audio.UnloadProjectGroup: Invalid argument\n");
	}
	#endif // _RELEASE


	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.UnloadAllProjects
// DESCRIPTION: Unloads all projects and event data.
//-------------------------------------------------------------------------------------------------
static int Audio_UnloadAllProjects (NinjaLua::LuaState* pobState)
{
	AudioSystem::Get().UnloadAllProjects();

	return 0;
}




//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.LoadEventGroup
// DESCRIPTION: Cache an event group.
//-------------------------------------------------------------------------------------------------
static void Audio_LoadEventGroup (const char* pcEventGroup)
{
	AudioSystem::Get().LoadEventGroup(pcEventGroup);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.UnloadEventGroup
// DESCRIPTION: Load an FMOD fev file.
//-------------------------------------------------------------------------------------------------
static void Audio_UnloadEventGroup (const char* pcEventGroup)
{
	AudioSystem::Get().UnloadEventGroup(pcEventGroup);
}

 

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.PlayEvent
// DESCRIPTION: Play an FMOD event relative to an entity. Returns an ID of the sound instance.
//-------------------------------------------------------------------------------------------------
static unsigned int Audio_PlayEvent (const char* pcGroupName,const char* pcEventName)
{
	CEntity* pobEnt = CLuaGlobal::Get().GetTarg();
	
	if (pobEnt)
	{
		unsigned int id;

		if (AudioSystem::Get().Sound_Prepare(id,pcGroupName,pcEventName))
		{
			if (pobEnt)
				AudioSystem::Get().Sound_SetPosition(id, pobEnt->GetPosition());

			AudioSystem::Get().Sound_Play(id);

			return id;
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.StopEvent
// DESCRIPTION: Stop an instance that might currently be playing.
//-------------------------------------------------------------------------------------------------
static void Audio_StopEvent (unsigned int id)
{
	AudioSystem::Get().Sound_Stop(id);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.InitMixer()
// DESCRIPTION: Initialise the mixer.
//-------------------------------------------------------------------------------------------------
static int Audio_InitMixer (NinjaLua::LuaState* /*pobState*/)
{
	AudioMixer::Get().InitCategories();
	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.SetMixerProfile( string, float )
// DESCRIPTION: Apply a mixer profile
//-------------------------------------------------------------------------------------------------
static int Audio_SetMixerProfile (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack obArgs( pobState );

	if (!obArgs[1].IsString())
	{
		ntPrintf("Invalid argument #1 in Audio.SetMixerProfile\n");
		return 0;
	}
	
	const char* pcMixerProfile=obArgs[1].GetString();
	float fTransitionDuration=0.0f;
	
	if (obArgs[2].IsNumber())
	{
		fTransitionDuration=obArgs[2].GetFloat();
	}


	AudioMixer::Get().SetProfile(pcMixerProfile,fTransitionDuration);

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.SetMixerPause( string, bool )
// DESCRIPTION: Pause or unpause a set of categories.
//-------------------------------------------------------------------------------------------------
static int Audio_SetMixerPause (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack obArgs( pobState );

	if (!obArgs[1].IsString())
	{
		ntPrintf("Invalid argument #1 in Audio.SetMixerPause\n");
		return 0;
	}
	
	const char* pcMixerProfile=obArgs[1].GetString();

	bool bPause=true;
	
	if (obArgs[2].IsBoolean())
	{
		bPause=obArgs[2].GetBoolean();
	}

	AudioMixer::Get().SetProfilePause(pcMixerProfile,bPause);

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.GetNumSpeakers()
// DESCRIPTION: Return the number of speakers that are configured for the platform. This value is
//              obtained from the operating system.
//-------------------------------------------------------------------------------------------------
static int Audio_GetNumSpeakers ()
{
	return AudioSystem::Get().GetSpeakerCount();
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Audio.GetLanguage
// DESCRIPTION: Get the language configuration.
//-------------------------------------------------------------------------------------------------
static int Audio_GetLanguage (NinjaLua::LuaState* pobState)
{
	pobState->Push( Language::GetAudioLanguageName( CPlayerOptions::Get().GetLiveAudioLanguage() ) );
	return 1;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.LoadBank( string )
// DESCRIPTION: Load a music bank.
//-------------------------------------------------------------------------------------------------
static int Music_LoadBank (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack obArgs( pobState );

	/*
	if (obArgs[1].IsString())
	{
		InteractiveMusicManager::Get().LoadMusicBank(obArgs[1].GetString(),true);
	}
	#ifndef _RELEASE
	else
	{
		ntPrintf("Music.LoadBank: Invalid argument\n");
	}
	#endif // _RELEASE
	*/

	return 0;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SetIntensity( float )
// DESCRIPTION: Set the global music intensity.
//-------------------------------------------------------------------------------------------------
static int Music_SetIntensity (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack obArgs( pobState );

	if (obArgs[1].IsNumber())
	{
		InteractiveMusicManager::Get().SetIntensity(obArgs[1].GetFloat());
	}
	#ifndef _RELEASE
	else
	{
		ntPrintf("Music.SetIntensity: Invalid argument\n");
	}
	#endif // _RELEASE

	return 0;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SuspendTrackingElements( void )
// DESCRIPTION: Suspends tracking element update.
//-------------------------------------------------------------------------------------------------
static void Music_SuspendTrackingElements(void)
{
	if (!MusicControlManager::Exists())
		return;

	MusicControlManager::Get().Suspend();
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SleepTrackingElements( float )
// DESCRIPTION: Suspends tracking element update for a given duration.
//-------------------------------------------------------------------------------------------------
static void Music_SleepTrackingElements(float fDuration)
{
	if (!MusicControlManager::Exists())
		return;

	MusicControlManager::Get().Suspend(fDuration);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.ResumeTrackingElements( void )
// DESCRIPTION: Resumes tracking element update (from sleep or suspend).
//-------------------------------------------------------------------------------------------------
static void Music_ResumeTrackingElements(void)
{
	if (!MusicControlManager::Exists())
		return;

	MusicControlManager::Get().Resume();
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.EnableAllTrackingElements( void )
// DESCRIPTION: Enables all tracking elements.
//-------------------------------------------------------------------------------------------------
static void Music_EnableAllTrackingElements(void)
{
	if (!MusicControlManager::Exists())
		return;

	MusicControlManager::Get().EnableAllRules(true);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.DisableAllTrackingElements( void )
// DESCRIPTION: Disables all tracking elements.
//-------------------------------------------------------------------------------------------------
static void Music_DisableAllTrackingElements(void)
{
	if (!MusicControlManager::Exists())
		return;

	MusicControlManager::Get().EnableAllRules(false);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.EnableTrackingElement( string )
// DESCRIPTION: Enables an element by name.
//-------------------------------------------------------------------------------------------------
static bool Music_EnableTrackingElement(const char* pcName)
{
	if (!pcName || !MusicControlManager::Exists())
		return false;

	return MusicControlManager::Get().EnableRule(pcName, true);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.DisableTrackingElement( string )
// DESCRIPTION: Disables an element by name.
//-------------------------------------------------------------------------------------------------
static bool Music_DisableTrackingElement(const char* pcName)
{
	if (!pcName || !MusicControlManager::Exists())
		return false;

	return MusicControlManager::Get().EnableRule(pcName, false);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.EnableTrackingElementGroup( string )
// DESCRIPTION: Enables tracking elements by group label.
//-------------------------------------------------------------------------------------------------
static bool Music_EnableTrackingElementGroup(const char* pcGroupLabel)
{
	if (!pcGroupLabel || !MusicControlManager::Exists())
		return false;

	return MusicControlManager::Get().EnableRuleGroup(pcGroupLabel, true);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.DisableTrackingElementGroup( string )
// DESCRIPTION: Disables tracking elements by group label.
//-------------------------------------------------------------------------------------------------
static bool Music_DisableTrackingElementGroup(const char* pcGroupLabel)
{
	if (!pcGroupLabel || !MusicControlManager::Exists())
		return false;

	return MusicControlManager::Get().EnableRuleGroup(pcGroupLabel, false);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SetTrackingElementInput( string, string )
// DESCRIPTION: Changes an element's input.
//-------------------------------------------------------------------------------------------------
static void Music_SetTrackingElementInput(const char* pcElement, const char* pcInput)
{
	if (!pcElement|| !pcInput || !MusicControlManager::Exists())
		return;

	MusicControlManager::MusicControlRule* pobRule = MusicControlManager::Get().GetRule(pcElement);
	MusicControlManager::MusicControlInput* pobInput = MusicControlManager::Get().GetInput(pcInput);
	if (!pobRule || !pobInput)
		return;

	pobRule->SetInput(pobInput);
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SetTrackingElementRuleThreshold( string, float, bool )
// DESCRIPTION: Changes to a threshold or inverted threshold based rule and sets the threshold value.
//-------------------------------------------------------------------------------------------------
static void Music_SetTrackingElementRuleThreshold(const char* pcElement, float fThreshold, bool bInvert)
{
	if (!pcElement || !MusicControlManager::Exists())
		return;

	MusicControlManager::MusicControlRule* pobRule = MusicControlManager::Get().GetRule(pcElement);
	if (!pobRule)
		return;

	pobRule->SetThreshold(fThreshold, bInvert);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SetTrackingElementRuleRange( string, float, float, bool )
// DESCRIPTION: Changes to a range or inverted range based rule and sets the min. and max. range extents.
//-------------------------------------------------------------------------------------------------
static void Music_SetTrackingElementRuleRange(const char* pcElement, float fMin, float fMax, bool bInvert)
{
	if (!pcElement || !MusicControlManager::Exists())
		return;

	MusicControlManager::MusicControlRule* pobRule = MusicControlManager::Get().GetRule(pcElement);
	if (!pobRule)
		return;

	pobRule->SetRange(fMin, fMax, bInvert);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SetTrackingElementRuleDelta( string, float, float )
// DESCRIPTION: Changes to a delta based rule and sets the activation rate of change via a supplied delta and time window.
//-------------------------------------------------------------------------------------------------
static void Music_SetTrackingElementRuleDelta(const char* pcElement, float fDelta, float fTime)
{
	if (!pcElement || !MusicControlManager::Exists())
		return;

	MusicControlManager::MusicControlRule* pobRule = MusicControlManager::Get().GetRule(pcElement);
	if (!pobRule)
		return;

	pobRule->SetDelta(fDelta, fTime);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SetTrackingElementActionIntensityFixed( string, float, bool )
// DESCRIPTION: Changes to a fixed intensity or fixed sustained intensity action and sets the music intensity value.
//-------------------------------------------------------------------------------------------------
static void Music_SetTrackingElementActionIntensityFixed(const char* pcElement, float fValue, bool bSustained)
{
	if (!pcElement || !MusicControlManager::Exists())
		return;

	MusicControlManager::MusicControlAction* pobAction = MusicControlManager::Get().GetAction(pcElement);
	if (!pobAction)
		return;

	pobAction->SetFixedIntensity(fValue, bSustained);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SetTrackingElementActionIntensityRelative( string, float )
// DESCRIPTION: Changes to a relative intensity action and sets the music intensity value delta.
//-------------------------------------------------------------------------------------------------
static void Music_SetTrackingElementActionIntensityRelative(const char* pcElement, float fValue)
{
	if (!pcElement || !MusicControlManager::Exists())
		return;

	MusicControlManager::MusicControlAction* pobAction = MusicControlManager::Get().GetAction(pcElement);
	if (!pobAction)
		return;

	pobAction->SetRelativeIntensity(fValue);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SetTrackingElementActionIntensityRange( string, float, float )
// DESCRIPTION: Changes to a scaled intensity action and sets the associated music intensity range.
//-------------------------------------------------------------------------------------------------
static void Music_SetTrackingElementActionIntensityRange(const char* pcElement, float fMin, float fMax)
{
	if (!pcElement || !MusicControlManager::Exists())
		return;

	MusicControlManager::MusicControlAction* pobAction = MusicControlManager::Get().GetAction(pcElement);
	if (!pobAction)
		return;

	pobAction->SetIntensityRange(fMin, fMax);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SetTrackingElementActionInfluenceRange( string, float, float )
// DESCRIPTION: Sets action influenced intensity range.
//-------------------------------------------------------------------------------------------------
static void Music_SetTrackingElementActionInfluenceRange(const char* pcElement, float fMin, float fMax)
{
	if (!pcElement || !MusicControlManager::Exists())
		return;

	MusicControlManager::MusicControlAction* pobAction = MusicControlManager::Get().GetAction(pcElement);
	if (!pobAction)
		return;

	pobAction->SetInfluenceRange(fMin, fMax);
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC: Music.SetInputParameter( string, string, float )
// DESCRIPTION: Sets a named input parameter.
//-------------------------------------------------------------------------------------------------
static void Music_SetInputParameter(const char* pcInput, const char* pcParam, float fValue)
{
	if (!pcInput || !pcParam || !MusicControlManager::Exists())
		return;

	MusicControlManager::MusicControlInput* pobInput = MusicControlManager::Get().GetInput(pcInput);
	if (!pobInput)
		return;

	pobInput->SetParameter(CHashedString(pcParam), fValue);
}





/***************************************************************************************************
*
*	FUNCTION		CAudioBindings::Register
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CAudioBindings::Register()
{
	NinjaLua::LuaObject obLuaGlobals = CLuaGlobal::Get().State().GetGlobals();

	obLuaGlobals.Set("Audio", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));

	// FMOD resource handling
	obLuaGlobals["Audio"].RegisterRaw(	"LoadProject",			Audio_LoadProject );
	obLuaGlobals["Audio"].RegisterRaw(	"LoadEventFile",		Audio_LoadEventFile ); // Legacy - need to remove after alpha
	obLuaGlobals["Audio"].RegisterRaw(	"UnloadProject",		Audio_UnloadProject );
	obLuaGlobals["Audio"].RegisterRaw(	"UnloadProjectGroup",	Audio_UnloadProjectGroup );
	obLuaGlobals["Audio"].RegisterRaw(	"UnloadAllProjects",	Audio_UnloadAllProjects );

	obLuaGlobals["Audio"].Register(		"LoadEventGroup",		Audio_LoadEventGroup );
	obLuaGlobals["Audio"].Register(		"UnloadEventGroup",		Audio_UnloadEventGroup );

	// FMOD events
	obLuaGlobals["Audio"].Register(		"PlayEvent",			Audio_PlayEvent );
	obLuaGlobals["Audio"].Register(		"StopEvent",			Audio_StopEvent );

	obLuaGlobals["Audio"].RegisterRaw(	"PlayEntitySound",		Audio_PlayEntitySound);   // This needs to be moved onto an entity.audio component
	obLuaGlobals["Audio"].RegisterRaw(	"StopEntitySound",		Audio_StopEntitySound);
	obLuaGlobals["Audio"].Register(		"PlayChatterVO",		Audio_ChatterVO);

	// Ambient sounds
	obLuaGlobals["Audio"].Register( 	"SetAmbience", 			Audio_SetAmbience);
	obLuaGlobals["Audio"].Register(		"IsAmbienceEnabled",	Audio_IsAmbienceEnabled);

	// Audio mixer
	obLuaGlobals["Audio"].RegisterRaw(	"InitMixer",			Audio_InitMixer);
	obLuaGlobals["Audio"].RegisterRaw(	"SetMixerProfile",		Audio_SetMixerProfile);
	obLuaGlobals["Audio"].RegisterRaw(	"SetMixerPause",		Audio_SetMixerPause);
	obLuaGlobals["Audio"].Register(		"GetNumSpeakers",		Audio_GetNumSpeakers);
	obLuaGlobals["Audio"].RegisterRaw(	"GetLanguage",			Audio_GetLanguage);

	// Music bindings
	obLuaGlobals.Set("Music", NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State()));
	obLuaGlobals["Music"].RegisterRaw(	"LoadBank",				Music_LoadBank );
	obLuaGlobals["Music"].RegisterRaw(	"SetIntensity",			Music_SetIntensity );

	// Music control
	obLuaGlobals["Music"].Register(	"SuspendTrackingElements",						Music_SuspendTrackingElements);
	obLuaGlobals["Music"].Register(	"SleepTrackingElements",						Music_SleepTrackingElements);
	obLuaGlobals["Music"].Register(	"ResumeTrackingElements",						Music_ResumeTrackingElements);
	obLuaGlobals["Music"].Register(	"EnableAllTrackingElements",					Music_EnableAllTrackingElements);
	obLuaGlobals["Music"].Register(	"DisableAllTrackingElements",					Music_DisableAllTrackingElements);
	obLuaGlobals["Music"].Register(	"EnableTrackingElement",						Music_EnableTrackingElement);
	obLuaGlobals["Music"].Register(	"DisableTrackingElement",						Music_DisableTrackingElement);
	obLuaGlobals["Music"].Register(	"EnableTrackingElementGroup",					Music_EnableTrackingElementGroup);
	obLuaGlobals["Music"].Register(	"DisableTrackingElementGroup",					Music_DisableTrackingElementGroup);
	obLuaGlobals["Music"].Register(	"SetTrackingElementInput",						Music_SetTrackingElementInput);
	obLuaGlobals["Music"].Register(	"SetTrackingElementRuleThreshold",				Music_SetTrackingElementRuleThreshold);
	obLuaGlobals["Music"].Register(	"SetTrackingElementRuleRange",					Music_SetTrackingElementRuleRange);
	obLuaGlobals["Music"].Register(	"SetTrackingElementRuleDelta",					Music_SetTrackingElementRuleDelta);
	obLuaGlobals["Music"].Register(	"SetTrackingElementActionIntensityFixed",		Music_SetTrackingElementActionIntensityFixed);
	obLuaGlobals["Music"].Register(	"SetTrackingElementActionIntensityRelative",	Music_SetTrackingElementActionIntensityRelative);
	obLuaGlobals["Music"].Register(	"SetTrackingElementActionIntensityRange",		Music_SetTrackingElementActionIntensityRange);
	obLuaGlobals["Music"].Register(	"SetTrackingElementActionInfluenceRange",		Music_SetTrackingElementActionInfluenceRange);
	obLuaGlobals["Music"].Register(	"SetInputParameter",							Music_SetInputParameter);
}
