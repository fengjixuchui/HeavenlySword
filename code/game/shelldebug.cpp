//------------------------------------------------------------------------------------------
//!
//!	\file shelldebug.cpp
//! Defines shell debug functionality only
//!
//------------------------------------------------------------------------------------------

#include "game/shelldebug.h"
#include "game/shellmain.h"
#include "game/shelllevel.h"
#include "game/entity.h"
#include "game/entitymanager.h"
#include "game/entitybrowser.h"
#include "game/movement.h"
#include "game/attacks.h"
#include "game/nsmanager.h"
#include "game/command.h"
#include "game/keybinder.h"
#include "game/jumpmenu.h"
#include "game/testmenu.h"
#include "game/capturesystem.h"
#include "game/chatterboxman.h"
#include "game/staticentity.h"
#include "game/inputcomponent.h"

#ifdef PLATFORM_PS3
#include "army/armymanager.h"
#include <netex/libnetctl.h>
#endif

#include "anim/animator.h"
#include "core/exportstruct_anim.h"
#include "core/gatso.h"
#include "core/OSDDisplay.h"
#include "core/debug_hud.h"

#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/debugcam.h"

#include "objectdatabase/dataobject.h"
#include "objectdatabase/neteditinterface.h"

#include "effect/functioncurve.h"
#include "effect/psystem_debug.h"
#include "effect/effect_trigger.h"
#include "effect/effect_special.h"
#include "effect/combateffect.h"

#include "gfx/shader.h"
#include "gfx/levelofdetail.h"
#include "gfx/sector.h"
#include "gfx/renderable.h"
#include "gfx/meshinstance.h"
#include "gfx/graphicsdevice.h"
#include "movies/movieinstance.h"
#include "movies/movieplayer.h"
#include "water/watermanager.h"

#include "ai/aiformationmanager.h"
#include "ai/aiinitialreactionman.h"				// Dario
#include "ai/aiuseobjectqueueman.h"					// Dario
#include "ai/aihearingman.h"						// Dario
#include "ai/ainavigationsystem/ainavigsystemman.h" // Dario
#include "ai/aibehaviourpool.h"
#include "ai/ainavgraphmanager.h"
#include "ai/aipatrolmanager.h"
#include "ai/aicoverpoint.h"

#include "physics/triggervolume.h"
#include "audio/audiomixer.h"

#ifndef _GOLD_MASTER

extern bool g_ResetExposure;
static RENDER_QUALITY_MODE s_renderMode = RENDER_QUALITY_BEST;

//------------------------------------------------------------------------------------------
//!
//!	LevelInfo
//! Version numbering struct
//!
//------------------------------------------------------------------------------------------
class LevelInfo
{
public:
	uint32_t		m_SaveCount;
	CHashedString	m_Comments;

	void PostConstruct()
	{
		ShellMain::Get().m_pDebugShell->m_LevelVersionNumber = m_SaveCount;
	}
};

START_CHUNKED_INTERFACE(LevelInfo, Mem::MC_MISC)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_SaveCount, 0, SaveCount );
	PUBLISH_VAR_WITH_DEFAULT_AS( m_Comments, "", Comments );
	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	ShellDebugCommandHelper
//!
//!	A placeholder object for shell commands that don't need the ShellDebug object's contents to work.
//! Examples would be function that just call through to external systems
//!	This way you can add commands without changing shelldebug.h (it forces big builds)
//!
//------------------------------------------------------------------------------------------
class ShellDebugCommandHelper
{
public:
#ifdef _HAVE_MEMORY_TRACKING
	COMMAND_RESULT CommandDumpMemoryChunkUsage();
	COMMAND_RESULT CommandDumpCompleteMemoryContents();
#endif

	int			m_iStubValue;			// in case compiler doesn't like classes with no members
};

// We need to instantiate this stub object for the command manager to invoke its commands...
static ShellDebugCommandHelper		g_sundryCommandsObject;

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandSetGameTimeMultiplier
*
*	DESCRIPTION		Command for toggling a bool member
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandToggleBoolMember(bool *pobBool)
{
	*pobBool = !*pobBool;
	ntPrintf(": %s", *pobBool ? "on" : "off");
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandIncrementInt
*
*	DESCRIPTION		Command for incrementing an int member
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandIncrementInt(int* pInt)
{
	*pInt++;
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandDecrementInt
*
*	DESCRIPTION		Command for decrementing an int member
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandDecrementInt(int* pInt)
{
	*pInt--;
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandRebuildCurve
*
*	DESCRIPTION		Command for rebuilding a curve's internal data from the object database data
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandRebuildCurve(const CHashedString& sCurveGuid)
{
	GameGUID obCurveGuid;
	DataObject *pCurveDO;
	
	obCurveGuid.SetFromString(ntStr::GetString(sCurveGuid));
	if (!obCurveGuid.IsNull())
	{
		pCurveDO = ObjectDatabase::Get().GetDataObjectFromGUID(obCurveGuid);
		if (strstr(pCurveDO->GetClassName(), "FunctionCurve_User"))
		{
			((FunctionCurve_User*)pCurveDO->GetBasePtr())->RebuildCurve();
			return CR_SUCCESS;
		}
		else
		{
			return CR_FAILED;
		}
	}
	else
	{
		return CR_FAILED;
	}
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandKillParticleSystem
*
*	DESCRIPTION		Kill all Psystems based off this definition
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandKillParticleSystem(const CHashedString& sPSystemGuid)
{
	GameGUID obPSystemGuid;
	DataObject *pPSystemDO;
	
	obPSystemGuid.SetFromString(ntStr::GetString(sPSystemGuid));
	if (!obPSystemGuid.IsNull())
	{
		pPSystemDO = ObjectDatabase::Get().GetDataObjectFromGUID(obPSystemGuid);
		if (strstr(pPSystemDO->GetClassName(), "PSystemComplexDef"))
		{
			PSystemDebugMan::KillAll((PSystemComplexDef*)pPSystemDO->GetBasePtr());
			return CR_SUCCESS;
		}
		else if (strstr(pPSystemDO->GetClassName(), "PSystemSimpleDef"))
		{	
			PSystemDebugMan::KillAll((PSystemSimpleDef*)pPSystemDO->GetBasePtr());
			return CR_SUCCESS;
		}
		else
		{
			return CR_FAILED;
		}
	}
	else
	{
		return CR_FAILED;
	}
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandSpawnFromTrigger
*
*	DESCRIPTION		Find this effect trigger and fire it
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandSpawnFromTrigger(const CHashedString& sTriggeruid)
{
	GameGUID obTriggerGuid;
	DataObject *pTriggerDO;
	
	obTriggerGuid.SetFromString(ntStr::GetString(sTriggeruid));
	if (!obTriggerGuid.IsNull())
	{
		pTriggerDO = ObjectDatabase::Get().GetDataObjectFromGUID(obTriggerGuid);
		((EffectTrigger*)pTriggerDO->GetBasePtr())->ForceTrigger();
		return CR_SUCCESS;
	}
	else
	{
		return CR_FAILED;
	}
}

// ------------------------------------ Anim Event Commands ----------------------------------------

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AnimEvent_SetEntity
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AnimEvent_SetEntity (const CHashedString& obEntityName)
{
	CEntity* pobEntity=CEntityManager::Get().FindEntity(obEntityName);
	
	if (pobEntity)
	{
		if (pobEntity->GetAnimator())
		{
			if (pobEntity->GetMovement() && pobEntity->GetMovement()->IsEnabled())
				pobEntity->GetMovement()->SetEnabled(false); // Disable movement controller
			
			pobEntity->GetAnimator()->RemoveAllAnimations(); // Clear all animations off
		}

		m_obAnimEventTargetEntity=obEntityName;

		// If this sector doesn't contain the entity, jump to sector containing first instance of entity:
		uint32_t entityAreaInfo = pobEntity->GetMappedAreaInfo();
		int32_t currentActiveArea = AreaManager::Get().GetCurrActiveArea();

		if ( !(entityAreaInfo & ( 1 << (currentActiveArea-1) )) )
		{
			ntPrintf("Warning - Attempting to warp to a sector containing entity: '%s'\n", ntStr::GetString(pobEntity->GetName()));
			bool bAreaFound = false;
			for( int32_t iAreaNumber = 1 ; iAreaNumber<=32 ; iAreaNumber++ )
			{
				ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
				if( entityAreaInfo & ( 1 << (iAreaNumber-1) ) )
				{
					bAreaFound = true;
					AreaManager::Get().ActivateArea( iAreaNumber, 0 );
					break;
				}
			}
			if( !bAreaFound )
			{
				ntPrintf("Error - Unable to warp to a sector containing entity: '%s'\n", ntStr::GetString(pobEntity->GetName()));
			}
		}

		// ----------

		// also update the entity browser
		CEntityBrowser::Get().SetCurrentEntity(pobEntity);

		return CR_SUCCESS;
	}

	return CR_FAILED;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AnimEvent_SetAnimation
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AnimEvent_SetAnimation (const CHashedString& obAnimationName)
{
	CEntity* pobEntity=CEntityManager::Get().FindEntity(m_obAnimEventTargetEntity);

	if (pobEntity && pobEntity->GetAnimator())
	{
		pobEntity->GetAnimator()->ClearAnimWeights();
		CAnimationPtr pobAnimation=pobEntity->GetAnimator()->CreateAnimation(obAnimationName);

		if (pobAnimation)
		{
			if ( pobAnimation->GetPriority() == 0 )
				pobAnimation->SetFlagBits( ANIMF_LOCOMOTING );

			pobAnimation->SetFlagBits(ANIMF_INHIBIT_AUTO_DESTRUCT);

			pobEntity->GetAnimator()->AddAnimation(pobAnimation);

			m_obAnimEventTargetAnim=obAnimationName;

			return CR_SUCCESS;
		}



		/*
		if (pobEntity->GetMovement() && pobEntity->GetMovement()->IsEnabled())
			pobEntity->GetMovement()->SetEnabled(false); // Disable movement controller

		pobEntity->GetAnimator()->RemoveAllAnimations(); // Clear all animations off

		CAnimationPtr pobAnimation=pobEntity->GetAnimator()->CreateAnimation(obAnimationName);

		//ntPrintf("Command_AnimEvent_SetAnimation:%s\n",*obAnimationName);

		if (pobAnimation)
		{
			pobAnimation->SetFlagBits(ANIMF_INHIBIT_AUTO_DESTRUCT);

			pobEntity->GetAnimator()->AddAnimation(pobAnimation);

			m_obAnimEventTargetAnim=obAnimationName;

			return CR_SUCCESS;
		}
		*/
	}

	return CR_FAILED;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AnimEvent_GenerateStrike
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AnimEvent_GetAnimationDuration (const CHashedString& obAnimationName)
{
	CEntityQuery obEntityQuery;
	CEntityManager::Get().FindEntitiesByType(obEntityQuery, CEntity::EntType_All);
	const QueryResultsContainerType& obResults = obEntityQuery.GetResults();
	for (QueryResultsContainerType::const_iterator obIt = obResults.begin(); obIt != obResults.end(); ++obIt)
	{
		// If this entity doesn't even have an animator, don't bother checking its animations.
		const CAnimator *pAnimator = (*obIt)->GetAnimator();
		if (pAnimator)
		{
			// Check to see if this entity actually has the animation we're looking for.
			const AnimShortCut* pAnimShortCut = (*obIt)->FindAnimShortCut(obAnimationName, true);
			if (pAnimShortCut != NULL)
			{
				CAnimationPtr pobRealAnim = CAnimation::Create( pAnimator, obAnimationName.GetHash(), pAnimShortCut, ntStr::GetString(obAnimationName) );
				ntstd::Ostringstream resultStream;
				resultStream << pobRealAnim->GetDuration();
				CommandManager::Get().StoreCommandResult(CHashedString(resultStream.str()));
				return CR_SUCCESS;
			}
		}
	}

	CommandManager::Get().StoreCommandResult(CHashedString("0.0"));
	return CR_FAILED;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AnimEvent_SetAnimationTime
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AnimEvent_SetAnimationTime (const float& fTime)
{
	CEntity* pobEntity=CEntityManager::Get().FindEntity(m_obAnimEventTargetEntity);

	if (pobEntity && pobEntity->GetAnimator())
	{
		const CAnimationPtr pobAnim=pobEntity->GetAnimator()->GetPlayingAnimation(m_obAnimEventTargetAnim.GetHash());

		if (pobAnim)
		{
			pobAnim->SetTime(fTime);
			pobEntity->GetAnimator()->GetAnimEventHandler().ResetProgress(pobAnim);
			return CR_SUCCESS;
		}
	}

	return CR_FAILED;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AnimEvent_SetAnimationSpeed
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AnimEvent_SetAnimationSpeed (const float& fSpeed)
{
	CEntity* pobEntity=CEntityManager::Get().FindEntity(m_obAnimEventTargetEntity);

	if (pobEntity && pobEntity->GetAnimator())
	{
		const CAnimationPtr pobAnim=pobEntity->GetAnimator()->GetPlayingAnimation(m_obAnimEventTargetAnim.GetHash());

		if (pobAnim)
			pobAnim->SetSpeed(fSpeed);

		return CR_SUCCESS;
	}

	return CR_FAILED;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AnimEvent_SetAnimationLoop
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AnimEvent_SetAnimationLoop (const bool& bLoop)
{
	CEntity* pobEntity=CEntityManager::Get().FindEntity(m_obAnimEventTargetEntity);

	if (pobEntity && pobEntity->GetAnimator())
	{
		const CAnimationPtr pobAnim=pobEntity->GetAnimator()->GetPlayingAnimation(m_obAnimEventTargetAnim.GetHash());

		if (pobAnim)
		{
			if (bLoop)
				pobAnim->SetFlagBits(ANIMF_LOOPING);
			else
				pobAnim->ClearFlagBits(ANIMF_LOOPING);

			return CR_SUCCESS;
		}
	}

	return CR_FAILED;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AnimEvent_DebugDisable
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AnimEvent_DebugDisable (const CHashedString& obAnimEventName)
{
	CEntity* pobEntity=CEntityManager::Get().FindEntity(m_obAnimEventTargetEntity);

	if (pobEntity && pobEntity->GetAnimator())
	{
		pobEntity->GetAnimator()->GetAnimEventHandler().ToggleDebugDisable(m_obAnimEventTargetAnim,obAnimEventName);

		return CR_SUCCESS;		
	}

	return CR_FAILED;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AnimEvent_GenerateStrike
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AnimEvent_GenerateStrike (const CHashedString& obStrikeName)
{
	CEntity* pobEntity=CEntityManager::Get().FindEntity(m_obAnimEventTargetEntity);

	if (!pobEntity)
	{
		pobEntity = CEntityManager::Get().GetPlayer();
	}

	if (pobEntity && pobEntity->GetAttackComponent())
	{
		CAttackLink* pobAttackLink = ObjectDatabase::Get().GetPointerFromName<CAttackLink*>(obStrikeName);

			// just make sure this is off
			CAttackComponent::m_bGlobablAutoBlockEnable = false;

			if (pobEntity->GetMovement() && !pobEntity->GetMovement()->IsEnabled())
				pobEntity->GetMovement()->SetEnabled(true); // The forced attack isn't going to do much otherwise
			
			pobEntity->GetAnimator()->RemoveAllAnimations(); // Clear all animations off

		if (pobAttackLink)
		{
			switch (m_eAnimEventAttackSimType)
			{	
#ifndef _RELEASE
			case BUTTON_SIM_COMBO:
				pobEntity->GetInputComponent()->PlaySequence(pobAttackLink);
				return CR_SUCCESS;
#endif
			case FORCE_ATTACK:
			default:
			CAttackComponent* pobAttackComponent = const_cast< CAttackComponent* >(pobEntity->GetAttackComponent());
			COMMAND_RESULT result = pobAttackComponent->ForceAttack(pobAttackLink) ? CR_SUCCESS : CR_FAILED;
			return result;
		}
	}
	}

	return CR_FAILED;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AnimEvent_GenerateStrike
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AnimEvent_SetAttackSimType (const int& iAttackSimType)
{
	m_eAnimEventAttackSimType = static_cast<AnimEventAttackSimType>(iAttackSimType);
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AnimEvent_RestoreControl
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AnimEvent_RestoreControl ()
{
	CEntity* pobEntity=CEntityManager::Get().FindEntity(m_obAnimEventTargetEntity);

	if (pobEntity && pobEntity->GetAnimator() && pobEntity->GetMovement())
	{
		pobEntity->GetAnimator()->RemoveAllAnimations();
		if (!pobEntity->GetMovement()->IsEnabled())
			pobEntity->GetMovement()->SetEnabled(true);

		return CR_SUCCESS;
	}

	return CR_FAILED;
}

// ------------------------------------- Audio Mixer Commands -----------------------------------------

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AudioMixer_SetProfile
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AudioMixer_SetProfile (const CHashedString& obProfileName)
{
	AudioMixer::Get().SetProfile(obProfileName, 0.1f);
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AudioMixer_SetProfileChangesEnabled
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AudioMixer_SetProfileChangesEnabled (const bool& bEnabled)
{
	FW_UNUSED(bEnabled);
	// TODO:
	// AudioMixer::Get().SetProfileChangesEnabled(bEnabled);
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AudioMixer_SetCategoryToManipulate
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AudioMixer_SetCategoryToManipulate (const CHashedString& obCategoryName)
{
	m_ob_AudioMixerCategoryToManipulate = obCategoryName;
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AudioMixer_SetCategoryMute
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AudioMixer_SetCategoryMute (const bool& bMute)
{
	if (m_ob_AudioMixerCategoryToManipulate.IsNull())
		return CR_FAILED;
	
	AudioMixer::Get().SetCategoryMute(m_ob_AudioMixerCategoryToManipulate, bMute);
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::Command_AudioMixer_SetCategoryPause
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::Command_AudioMixer_SetCategoryPause (const bool& bPause)
{
	if (m_ob_AudioMixerCategoryToManipulate.IsNull())
		return CR_FAILED;

	AudioMixer::Get().SetCategoryPause(m_ob_AudioMixerCategoryToManipulate, bPause);
	return CR_SUCCESS;
}

// ------------------------------------ NinjaSequence Commands ----------------------------------------

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandSetNinjaSequencePlaybackClip
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandSetNinjaSequencePlaybackClip (const CHashedString& obClipName)
{	
	NSManager::Get().SetPlaybackClipName( obClipName.GetDebugString() );
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandTriggerNinjaSequence
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandTriggerNinjaSequence (const CHashedString& obEntityName)
{
	CEntity* pobEntity=CEntityManager::Get().FindEntity(obEntityName);
	
	if (pobEntity)
	{
		CMessageSender::SendEmptyMessage( CHashedString("Trigger"), pobEntity->GetMessageHandler() );

		return CR_SUCCESS;
	}

	return CR_FAILED;
}

// ------------------------------------ Cube-map related command(s) ----------------------------------------

// Command for capturing cube-map image for a given axis
COMMAND_RESULT ShellDebug::Command_CaptureCubeMapAxis (const CHashedString& obAxisName)
{
	CDirection obCaptureDirection( 1.f, 0.f, 0.f );

	if( obAxisName == CHashedString("x") )
	{
		obCaptureDirection = CDirection( 1.f, 0.f, 0.f );
	}
	else if( obAxisName == CHashedString("-x") )
	{
		obCaptureDirection = CDirection( -1.f, 0.f, 0.f );
	}
	else if( obAxisName == CHashedString("y") )
	{
		obCaptureDirection = CDirection( 0.f, 1.f, 0.f );
	}
	else if( obAxisName == CHashedString("-y") )
	{
		obCaptureDirection = CDirection( 0.f, -1.f, 0.f );
	}
	else if( obAxisName == CHashedString("z") )
	{
		obCaptureDirection = CDirection( 0.f, 0.f, -1.f );
	}
	else if( obAxisName == CHashedString("-z") )
	{
		obCaptureDirection = CDirection( 0.f, 0.f, 1.f );
	}
	else
	{
		return CR_FAILED;
	}

	CamMan::GetPrimaryView()->GetDebugCamera()->SetFOV( 90.0f );
	CPoint curCamPosition = CamMan::GetPrimaryView()->GetDebugControllerMatrix().GetTranslation();
	CPoint camTarget = curCamPosition + obCaptureDirection;
	CamMan::GetPrimaryView()->SetDebugCameraPlacement(curCamPosition, camTarget);

	CSector::Get().DoForceDump();	
	return CR_SUCCESS;
}

//-----------------------------------------------------------------------------------------
//! 
//! ShellDebug::CommandCallScriptFunc
//! Command to call a lua script function
//!
//----------------------------------------------------------------------------------------
COMMAND_RESULT ShellDebug::CommandCallScriptFunc(const CHashedString& sFunc)
{
	NinjaLua::LuaObject lobjBind = CLuaGlobal::Get().State().GetGlobals()[sFunc];
	
	if(!lobjBind.IsFunction())
		return CR_FAILED;

	NinjaLua::LuaFunction lfuncBind(lobjBind);
	lfuncBind();

	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandSetGameTimeMultiplier
*
*	DESCRIPTION		Command for setting the game time multiplier E.g. x2.0
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandSetGameTimeMultiplier(const float& fMultiplier)
{
	if (fMultiplier == 1.0f)
	{
		CTimer::Get().ClearDebugTimeScalar();
	}
	else
	{
		CTimer::Get().SetDebugTimeScalar(fMultiplier);
	}
	// Indicate successfull
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandDebugQuitGame
*
*	DESCRIPTION		Debug command for quiting the game
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandDebugQuitGame()
{
	ShellMain::Get().RequestGameExit();
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::ToggleCodePause
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::ToggleCodePause()
{
	if ( g_ShellOptions->m_eFrontendMode != FRONTEND_TGS )
	{
		bool bPaused = ShellMain::Get().IsPausedByCode();
		ShellMain::Get().RequestPauseToggle( ShellMain::PM_INTERNAL, !bPaused );
	}
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::ToggleUserPause
*
*	DESCRIPTION		
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::ToggleUserPause()
{
	if ( g_ShellOptions->m_eFrontendMode == FRONTEND_NONE )
	{
		bool bPaused = ShellMain::Get().IsPausedByUser();
		ShellMain::Get().RequestPauseToggle( ShellMain::PM_USER, !bPaused );
	}
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandDumpGatso
*
*	DESCRIPTION		Dump current gatso usage
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandDumpGatso()
{
	CGatso::Dump();
	return CR_SUCCESS;
}

/***************************************************************************************************
*
*	FUNCTION		ShellDebugCommandHelper::CommandDumpMemoryChunkUsage
*
*	DESCRIPTION		Dump current memory usage in terms of how much each memory chunk is used
*
***************************************************************************************************/
#ifdef _HAVE_MEMORY_TRACKING
COMMAND_RESULT ShellDebugCommandHelper::CommandDumpMemoryChunkUsage()
{
	Mem::DumpSimpleChunkStats();
	return CR_SUCCESS;
}
#endif

/***************************************************************************************************
*
*	FUNCTION		ShellDebugCommandHelper::CommandDumpCompleteMemoryContents
*
*	DESCRIPTION		Dump a complete memory log to the "memory.log" file. Takes ages!
*
***************************************************************************************************/
#ifdef _HAVE_MEMORY_TRACKING
COMMAND_RESULT ShellDebugCommandHelper::CommandDumpCompleteMemoryContents()
{
	Mem::DumpMemoryTrackerMap();
	return CR_SUCCESS;
}
#endif

/***************************************************************************************************
*
*	FUNCTION		ShellDebug::CommandToggleSafeDebugLines
*
*	DESCRIPTION		This is used to toggle the rendering of the safe debug lines in the game.
*
***************************************************************************************************/
COMMAND_RESULT ShellDebug::CommandToggleSafeDebugLines()
{
	m_bRenderDebugSafeAreaLines = !m_bRenderDebugSafeAreaLines;
	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellDebug::CommandAdvanceContext
//! Command for setting the input context
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT ShellDebug::CommandAdvanceContext(const int& iVal)
{
	int iActualVal = iVal;
	iActualVal += INPUT_CONTEXT_MAX;

	// retreat the context (mod the maximum value)
	INPUT_CONTEXT eContext = INPUT_CONTEXT( ( CInputHardware::Get().GetContext() + iActualVal ) % INPUT_CONTEXT_MAX );
	CInputHardware::Get().SetContext( eContext );
	OSD::Add(OSD::DEBUG_CHAN, DC_WHITE, "changed context to: \"%s\"" , g_apcContextTitleStrings[eContext]);

	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellDebug::CommandSwitchDebugCameraMode
//!Switch debug camera mode
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT ShellDebug::CommandSwitchDebugCameraMode()
{
	// Temporary for testing PiP...
	if(CamMan::Get().GetView(1))
		CamMan::Get().GetView(1)->SwitchDebugCameraMode();
	else
		CamMan::GetPrimaryView()->SwitchDebugCameraMode();
	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellDebug::CommandToggleDebugRender
//! Toggle debug render
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT ShellDebug::CommandToggleDebugRender()
{
	CamMan::GetPrimaryView()->ToggleDebugRender();
	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellDebug::JumpToLevel
//! 
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT ShellDebug::JumpToLevel( const CHashedString& sLevelName, int nCheckpointID )
{
	const char* pcLevelNamePtr = ntStr::IsNull(sLevelName) ? 0 : ntStr::GetString(sLevelName);
	if ( g_ShellOptions->m_eFrontendMode == FRONTEND_NONE )
	{
		CGuiManager::Get().ResetLevel(pcLevelNamePtr, nCheckpointID);
	}
	else
	{
		CGuiManager::Get().OnComplete();
	}
	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	ShellDebug::RegisterDebugKeys
//! Register all global keys with the keybind manager
//!
//------------------------------------------------------------------------------------------
void	ShellDebug::RegisterDebugKeys()
{
	// Register some commands
	CommandBaseInput<const float&>* pobCommandGTM = CommandManager::Get().CreateCommand("SetGameTimeMultiplier", this, &ShellDebug::CommandSetGameTimeMultiplier, "Set the game time multiplier");

	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 0", 0.0f, KEYS_PRESSED, KEYC_0, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 1", 1.0f, KEYS_PRESSED, KEYC_1, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 2", 2.0f, KEYS_PRESSED, KEYC_2, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 3", 3.0f, KEYS_PRESSED, KEYC_3, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 4", 4.0f, KEYS_PRESSED, KEYC_4, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 5", 5.0f, KEYS_PRESSED, KEYC_5, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 6", 6.0f, KEYS_PRESSED, KEYC_6, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 7", 7.0f, KEYS_PRESSED, KEYC_7, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 8", 8.0f, KEYS_PRESSED, KEYC_8, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 9", 9.0f, KEYS_PRESSED, KEYC_9, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 20", 20.0f,	  KEYS_PRESSED, KEYC_0, KEYM_SHIFT);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 1/1", 1.0f,	  KEYS_PRESSED, KEYC_1, KEYM_SHIFT);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 1/2", 1.0f/2.0f, KEYS_PRESSED, KEYC_2, KEYM_SHIFT);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 1/3", 1.0f/3.0f, KEYS_PRESSED, KEYC_3, KEYM_SHIFT);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 1/4", 1.0f/4.0f, KEYS_PRESSED, KEYC_4, KEYM_SHIFT);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 1/5", 1.0f/5.0f, KEYS_PRESSED, KEYC_5, KEYM_SHIFT);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 1/6", 1.0f/6.0f, KEYS_PRESSED, KEYC_6, KEYM_SHIFT);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 1/7", 1.0f/7.0f, KEYS_PRESSED, KEYC_7, KEYM_SHIFT);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 1/8", 1.0f/8.0f, KEYS_PRESSED, KEYC_8, KEYM_SHIFT);
	KeyBindManager::Get().RegisterKey("global", pobCommandGTM, "Game Time Multiplier x 1/9", 1.0f/9.0f, KEYS_PRESSED, KEYC_9, KEYM_SHIFT);

	CommandBaseInput<const CHashedString&>* pobCommandCSF = CommandManager::Get().CreateCommand("CallScriptFunction", this, &ShellDebug::CommandCallScriptFunc, "Call a Script Function");
	KeyBindManager::Get().RegisterKey<CHashedString>("global", pobCommandCSF, "User Binding 1", "KeyBind1", KEYS_PRESSED, KEYC_KPAD_1, KEYM_CTRL);
	KeyBindManager::Get().RegisterKey<CHashedString>("global", pobCommandCSF, "User Binding 2", "KeyBind2", KEYS_PRESSED, KEYC_KPAD_2, KEYM_CTRL);
	KeyBindManager::Get().RegisterKey<CHashedString>("global", pobCommandCSF, "User Binding 3", "KeyBind3", KEYS_PRESSED, KEYC_KPAD_3, KEYM_CTRL);
	KeyBindManager::Get().RegisterKey<CHashedString>("global", pobCommandCSF, "User Binding 4", "KeyBind4", KEYS_PRESSED, KEYC_KPAD_4, KEYM_CTRL);
	KeyBindManager::Get().RegisterKey<CHashedString>("global", pobCommandCSF, "User Binding 5", "KeyBind5", KEYS_PRESSED, KEYC_KPAD_5, KEYM_CTRL);
	KeyBindManager::Get().RegisterKey<CHashedString>("global", pobCommandCSF, "User Binding 6", "KeyBind6", KEYS_PRESSED, KEYC_KPAD_6, KEYM_CTRL);
	KeyBindManager::Get().RegisterKey<CHashedString>("global", pobCommandCSF, "User Binding 7", "KeyBind7", KEYS_PRESSED, KEYC_KPAD_7, KEYM_CTRL);
	KeyBindManager::Get().RegisterKey<CHashedString>("global", pobCommandCSF, "User Binding 8", "KeyBind8", KEYS_PRESSED, KEYC_KPAD_8, KEYM_CTRL);
	KeyBindManager::Get().RegisterKey<CHashedString>("global", pobCommandCSF, "User Binding 9", "KeyBind9", KEYS_PRESSED, KEYC_KPAD_9, KEYM_CTRL);

	// Register some commands
	CommandBaseInput<const int&>* pobCommandAC = CommandManager::Get().CreateCommand("AdvanceContext", this, &ShellDebug::CommandAdvanceContext, "Advance the current context by n");
	KeyBindManager::Get().RegisterKey("global", pobCommandAC, "Step context forward", 1, KEYS_PRESSED, KEYC_TAB, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("global", pobCommandAC, "Step context backward", -1, KEYS_PRESSED, KEYC_TAB, KEYM_SHIFT);


	// Register some commands
	CommandBaseInput<bool*>* pobCommandToggleBool = CommandManager::Get().CreateCommand("ToggleBoolMember", this, &ShellDebug::CommandToggleBoolMember, "Toggle a bool member in a class");
//	CommandBaseInput<int*>* pobCommandIncrementInt = CommandManager::Get().CreateCommand("IncrementIntMember", this, &ShellDebug::CommandIncrementInt, "Increment an int member in a class");
//	CommandBaseInput<int*>* pobCommandDecrementInt = CommandManager::Get().CreateCommand("DecrementIntMember", this, &ShellDebug::CommandDecrementInt, "Decrement an int member in a class");


// ---- rendering ----
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Reload Shaders",				&ShaderManager::Get().m_bForceShadersReload, KEYS_PRESSED, KEYC_R, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle LOD",					&CLODManager::Get().m_bDebugRender, KEYS_PRESSED, KEYC_P, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Wireframe",			&CRendererSettings::bShowWireframe, KEYS_PRESSED, KEYC_W, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Bounding Boxes",		&CRendererSettings::bShowBoundingBoxes, KEYS_PRESSED, KEYC_B, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Exposure Sampling Area",	&CRendererSettings::bShowExposureSamplingArea, KEYS_PRESSED, KEYC_X, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Lighting Debug Info",	&CRendererSettings::bShowLightingInfo, KEYS_PRESSED, KEYC_L, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Levels Graph",			&CRendererSettings::bShowLevelsGraph, KEYS_PRESSED, KEYC_V, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle ALL Debug Primitives",	&CRendererSettings::bEnableDebugPrimitives, KEYS_PRESSED, KEYC_D, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Cloth",				&CRendererSettings::bEnableCloth, KEYS_PRESSED, KEYC_C, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Frustrum Culling",		&CRendererSettings::bEnableCulling, KEYS_PRESSED, KEYC_U, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Backface Culling",		&CRendererSettings::bEnableBackfaceCulling, KEYS_PRESSED, KEYC_A, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Lens Effects",			&CRendererSettings::bEnableLensEffects, KEYS_PRESSED, KEYC_F, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Sky",					&CRendererSettings::bEnableSky, KEYS_PRESSED, KEYC_Y, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Exposure",				&CRendererSettings::bEnableExposure, KEYS_PRESSED, KEYC_E, KEYM_NONE);

#ifdef PLATFORM_PC
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle GPU Exposure",			&CRendererSettings::bUseGPUExposure, KEYS_PRESSED, KEYC_J, KEYM_NONE);
#endif

	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Shadows",				&CRendererSettings::bEnableShadows, KEYS_PRESSED, KEYC_S, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Materials",			&CRendererSettings::bEnableMaterials, KEYS_PRESSED, KEYC_M, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Gamma Correction",		&CRendererSettings::bEnableGammaCorrection, KEYS_PRESSED, KEYC_G, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Depth Haze",			&CRendererSettings::bEnableDepthHaze, KEYS_PRESSED, KEYC_Z, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Lens Ghosting",		&CRendererSettings::bEnableLensGhost, KEYS_PRESSED, KEYC_N, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Skinning",				&CRendererSettings::bEnableSkinning, KEYS_PRESSED, KEYC_K, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Spherical Harmonics",	&CRendererSettings::bEnableSphericalHarmonics, KEYS_PRESSED, KEYC_H, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Alpha Blending",		&CRendererSettings::bUseAlphaBlending, KEYS_PRESSED, KEYC_T, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Parallax Mapping",		&CRendererSettings::bUseParallaxMapping, KEYS_PRESSED, KEYC_I, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle Debug D.O.F.",			&CRendererSettings::bGetDebugDOF, KEYS_PRESSED, KEYC_KPAD_8, KEYM_NONE);
	
#ifdef PLATFORM_PS3
	KeyBindManager::Get().RegisterKey("rendering", pobCommandToggleBool, "Toggle BSSkin Override",		&CRendererSettings::bLiveMaterialParamsEditing, KEYS_PRESSED, KEYC_B, KEYM_CTRL | KEYM_SHIFT);
#endif
	
	// Dump mapped keys
	CommandBaseNoInput* pobCommandDMK = CommandManager::Get().CreateCommandNoInput("DumpMappedKeys", KeyBindManager::GetP(), &KeyBindManager::DumpMappedKeys, "Dump the mapped keys");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandDMK, "Dump the mapped keys", KEYS_PRESSED, KEYC_P, KEYM_CTRL | KEYM_SHIFT);

	// Dump available keys
	CommandBaseNoInput* pobCommandDAK = CommandManager::Get().CreateCommandNoInput("DumpAvailableKeys", KeyBindManager::GetP(), &KeyBindManager::DumpAvailableKeys, "Dump the available keys");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandDAK, "Dump the available keys", KEYS_PRESSED, KEYC_SLASH, KEYM_CTRL | KEYM_SHIFT);

	// Quit the game
	CommandBaseNoInput* pobCommandQG = CommandManager::Get().CreateCommandNoInput("QuitGame", this, &ShellDebug::CommandDebugQuitGame, "Quit the game");
	KeyBindManager::Get().RegisterKeyNoInput("global", pobCommandQG, "Quit the game", KEYS_PRESSED, KEYC_ESCAPE, KEYM_NONE);

	// debug pause modes
	CommandBaseNoInput* pobCommandPC = CommandManager::Get().CreateCommandNoInput("ToggleCodePause", this, &ShellDebug::ToggleCodePause, "Toggle Code Pause");
	KeyBindManager::Get().RegisterKeyNoInput("global", pobCommandPC, "Toggle Code Pause", KEYS_PRESSED, KEYC_JOYPAD_SELECT, KEYM_NONE);

	CommandBaseNoInput* pobCommandPU = CommandManager::Get().CreateCommandNoInput("ToggleUserPause", this, &ShellDebug::ToggleUserPause, "Toggle User Pause");
	KeyBindManager::Get().RegisterKeyNoInput("global", pobCommandPU, "Toggle User Pause", KEYS_PRESSED, KEYC_JOYPAD_START, KEYM_NONE);

	// Toggle the jump menu
	CommandBaseNoInput* pobCommandTJM = CommandManager::Get().CreateCommandNoInput("ToggleJumpMenu", JumpMenu::GetP(), &JumpMenu::ToggleMenu, "Toggle the jump menu");
	KeyBindManager::Get().RegisterKeyNoInput("global", pobCommandTJM, "Toggle the jump menu", KEYS_PRESSED, KEYC_END, KEYM_NONE);

#if defined( PLATFORM_PS3 )
	CommandBaseNoInput* pobCommandADT = CommandManager::Get().CreateCommandNoInput("ToggleArmyDebug", ArmyManager::GetP(), &ArmyManager::ToggleDebugMode, "Toggle Army Debug");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandADT, "Toggle Army Debug", KEYS_PRESSED, KEYC_A, KEYM_CTRL);
#endif

	// Dump gatso usage
	CommandBaseNoInput* pobCommandTGD = CommandManager::Get().CreateCommandNoInput("DumpGatso", this, &ShellDebug::CommandDumpGatso, "Dump GATSO");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandTGD, "Dump GATSO", KEYS_PRESSED, KEYC_SLASH, KEYM_SHIFT);

#ifdef _HAVE_MEMORY_TRACKING	
	// [scee_st] Dump rough memory usage to TTY
	CommandBaseNoInput* pobCommandDMCU = CommandManager::Get().CreateCommandNoInput( "DumpMemoryChunkUsage", 
																					 &g_sundryCommandsObject,
																					 &ShellDebugCommandHelper::CommandDumpMemoryChunkUsage,
																					 "Dump chunk memory usage");
	KeyBindManager::Get().RegisterKeyNoInput(	"game",
												pobCommandDMCU,
												"Dump chunk memory usage",
												KEYS_PRESSED,
												KEYC_KPAD_DIVIDE,
												KEYM_SHIFT | KEYM_CTRL );
	// [scee_st] Dump absolutely everything memory-related to a file
	// NOTE: I have made this combination deliberately difficult to press as it takes a very long time...
	CommandBaseNoInput* pobCommandDCMU = CommandManager::Get().CreateCommandNoInput( "DumpCompleteMemoryUsage", 
																					 &g_sundryCommandsObject, 
																					 &ShellDebugCommandHelper::CommandDumpCompleteMemoryContents,
																					 "Dump full memory usage");
	KeyBindManager::Get().RegisterKeyNoInput(	"game",
												pobCommandDCMU,
												"Dump complete memory log (slow!)",
												KEYS_PRESSED,
												KEYC_KPAD_DIVIDE,
												KEYM_SHIFT | KEYM_CTRL | KEYM_ALT );
#endif	

	
	// Frame advance 
	CommandBaseInput<const float&>* pobCommandFA = CommandManager::Get().CreateCommand("AdvanceFrame", CTimer::GetP(), &CTimer::UpdateOneFrame, "Advance frame by \'f\'");
	KeyBindManager::Get().RegisterKey("game", pobCommandFA, "Advance by one frame", 1.0f, KEYS_PRESSED, KEYC_KPAD_PLUS, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("game", pobCommandFA, "Advance by quarter frame", 0.25f, KEYS_PRESSED, KEYC_KPAD_MINUS, KEYM_NONE);

	// Restart level
	CommandBaseInput<const CHashedString&>* pobCommandJTL = CommandManager::Get().CreateCommand("JumpToLevel", this, &ShellDebug::JumpToLevel, "Jump to level \'s\'");
	KeyBindManager::Get().RegisterKey<const CHashedString>("game", pobCommandJTL, "Restart level", ntStr::GetString(g_ShellOptions->m_dbgStartLevel), KEYS_PRESSED, KEYC_1, KEYM_CTRL);

// ---- Audio ----

//	KeyBindManager::Get().RegisterKey("sound", "ToggleBoolMember", "Toggle ChatterBox debug mode (Dario)", &CChatterBoxMan::Get().m_bRender,	KEYS_PRESSED, KEYC_C, KEYM_NONE);

// ---- Particle system control ----

	CommandManager::Get().CreateCommand("RebuildCurve", this, &ShellDebug::CommandRebuildCurve, "Rebuild a curve from its object database data");
	CommandManager::Get().CreateCommand("KillParticleSystem", this, &ShellDebug::CommandKillParticleSystem, "Immediately stop all running particle systems of a given type");
	CommandManager::Get().CreateCommand("SpawnFromTrigger", this, &ShellDebug::CommandSpawnFromTrigger, "Spawn a particle system from a given effect trigger");

// ---- Anim event control ----

	CommandManager::Get().CreateCommand("AnimEvent_SetEntity", this, &ShellDebug::Command_AnimEvent_SetEntity, "");
	CommandManager::Get().CreateCommand("AnimEvent_SetAnimation", this, &ShellDebug::Command_AnimEvent_SetAnimation, "");
	CommandManager::Get().CreateCommand("AnimEvent_GetAnimationDuration", this, &ShellDebug::Command_AnimEvent_GetAnimationDuration, "");
	CommandManager::Get().CreateCommand("AnimEvent_SetAnimationTime", this, &ShellDebug::Command_AnimEvent_SetAnimationTime, "");
	CommandManager::Get().CreateCommand("AnimEvent_SetAnimationSpeed", this, &ShellDebug::Command_AnimEvent_SetAnimationSpeed, "");
	CommandManager::Get().CreateCommand("AnimEvent_SetAnimationLoop", this, &ShellDebug::Command_AnimEvent_SetAnimationLoop, "");
	CommandManager::Get().CreateCommand("AnimEvent_GenerateStrike", this, &ShellDebug::Command_AnimEvent_GenerateStrike, "");
	CommandManager::Get().CreateCommand("AnimEvent_SetAttackSimType", this, &ShellDebug::Command_AnimEvent_SetAttackSimType, "");
	CommandManager::Get().CreateCommand("AnimEvent_DebugDisable", this, &ShellDebug::Command_AnimEvent_DebugDisable, "");
	CommandManager::Get().CreateCommandNoInput("AnimEvent_RestoreControl", this, &ShellDebug::Command_AnimEvent_RestoreControl, "");

// --- Audio Mixer control ---
	CommandManager::Get().CreateCommand("AudioMixer_SetProfile", this, &ShellDebug::Command_AudioMixer_SetProfile, "");
	CommandManager::Get().CreateCommand("AudioMixer_SetProfileChangesEnabled", this, &ShellDebug::Command_AudioMixer_SetProfileChangesEnabled, "");
	CommandManager::Get().CreateCommand("AudioMixer_SetCategoryToManipulate", this, &ShellDebug::Command_AudioMixer_SetCategoryToManipulate, "");
	CommandManager::Get().CreateCommand("AudioMixer_SetCategoryMute", this, &ShellDebug::Command_AudioMixer_SetCategoryMute, "");
	CommandManager::Get().CreateCommand("AudioMixer_SetCategoryPause", this, &ShellDebug::Command_AudioMixer_SetCategoryPause, "");

// ---- NinjaSequence control ----
	
	CommandManager::Get().CreateCommand("SetNinjaSequencePlaybackClip", this, &ShellDebug::CommandSetNinjaSequencePlaybackClip, "");
	CommandManager::Get().CreateCommand("TriggerNinjaSequence", this, &ShellDebug::CommandTriggerNinjaSequence, "");

// ---- Cubemap capture ----
	CommandManager::Get().CreateCommand("CaptureCubeMapAxis", this, &ShellDebug::Command_CaptureCubeMapAxis, "");

// ---- Camera ----

	// Switch debug camera mode
	CommandBaseNoInput* pobCommandSDCM = CommandManager::Get().CreateCommandNoInput("SwitchDebugCameraMode", this, &ShellDebug::CommandSwitchDebugCameraMode, "Switch debug camera mode");
	KeyBindManager::Get().RegisterKeyNoInput("camera", pobCommandSDCM, "Switch debug camera mode", KEYS_PRESSED, KEYC_SPACE, KEYM_NONE);

	// Toggle debug render
	CommandBaseNoInput* pobCommandTDR = CommandManager::Get().CreateCommandNoInput("ToggleDebugRender", this, &ShellDebug::CommandToggleDebugRender, "Toggle debug render");
	KeyBindManager::Get().RegisterKeyNoInput("camera", pobCommandTDR, "Toggle debug render", KEYS_PRESSED, KEYC_D, KEYM_NONE);


// ---- Combat ----

	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Auto Block",				&CAttackComponent::m_bGlobablAutoBlockEnable,	KEYS_PRESSED, KEYC_B, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle grid rendering",			&AttackDebugger::Get().m_bRenderProtractors,		KEYS_PRESSED, KEYC_P, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Reset player one grid",			&AttackDebugger::Get().m_bResetPlayerOneGrid,		KEYS_PRESSED, KEYC_F, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Attack state display",	&AttackDebugger::Get().m_bShowCurrentAttackState,	KEYS_PRESSED, KEYC_A, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Recovery state display",	&AttackDebugger::Get().m_bShowCurrentRecoveryState, KEYS_PRESSED, KEYC_R, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Attack class display",	&AttackDebugger::Get().m_bShowCurrentAttackClass,	KEYS_PRESSED, KEYC_C, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Show stance display",		&AttackDebugger::Get().m_bShowCurrentStance,		KEYS_PRESSED, KEYC_S, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle show windows display",	&AttackDebugger::Get().m_bShowAttackWindows,		KEYS_PRESSED, KEYC_W, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Health state display",	&AttackDebugger::Get().m_bShowHealth,				KEYS_PRESSED, KEYC_H, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Incapacity time display",	&AttackDebugger::Get().m_bShowIncapacityTime,		KEYS_PRESSED, KEYC_I, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Battle mode",				&AttackDebugger::Get().m_bBattleMode,				KEYS_PRESSED, KEYC_M, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Style level display",		&AttackDebugger::Get().m_bShowStyleLevel,			KEYS_PRESSED, KEYC_K, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Attack movement display",	&AttackDebugger::Get().m_bShowAttackMovementType,	KEYS_PRESSED, KEYC_E, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Targetting display",		&AttackDebugger::Get().m_bRenderTargeting,			KEYS_PRESSED, KEYC_T, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Root display",			&AttackDebugger::Get().m_bRenderRoots,				KEYS_PRESSED, KEYC_L, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Evade display",			&AttackDebugger::Get().m_bRenderRoots,				KEYS_PRESSED, KEYC_D, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle One hit kills",			(bool*)&g_ShellOptions->m_bOneHitKills,		KEYS_PRESSED, KEYC_Y, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("combat", pobCommandToggleBool, "Toggle Strike proximity angle display", &AttackDebugger::Get().m_bShowStrikeProximityCheckAngle, KEYS_PRESSED, KEYC_X, KEYM_NONE);

	CommandBaseNoInput* pobCommandACOB = CommandManager::Get().CreateCommandNoInput("AttackComponentOneBackward", &AttackDebugger::Get(), &AttackDebugger::AttackComponentOneBackward, "Step back P1 attack component");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandACOB, "Step back P1 attack component", KEYS_PRESSED, KEYC_LEFT_ARROW, KEYM_CTRL);

	CommandBaseNoInput* pobCommandACOF = CommandManager::Get().CreateCommandNoInput("AttackComponentOneForward", &AttackDebugger::Get(), &AttackDebugger::AttackComponentOneBackward, "Advance P1 attack component");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandACOF, "Advance P1 attack component", KEYS_PRESSED, KEYC_RIGHT_ARROW, KEYM_CTRL);

	CommandBaseNoInput* pobCommandACTB = CommandManager::Get().CreateCommandNoInput("AttackComponentTwoBackward", &AttackDebugger::Get(), &AttackDebugger::AttackComponentTwoBackward, "Step back P2 attack component");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandACTB, "Step back P2 attack component", KEYS_PRESSED, KEYC_LEFT_ARROW, KEYM_SHIFT);

	CommandBaseNoInput* pobCommandACTF = CommandManager::Get().CreateCommandNoInput("AttackComponentTwoForward", &AttackDebugger::Get(), &AttackDebugger::AttackComponentTwoBackward, "Advance P2 attack component");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandACTF, "Advance P2 attack component", KEYS_PRESSED, KEYC_RIGHT_ARROW, KEYM_SHIFT);

	CommandBaseNoInput* pobCommandTAZ = CommandManager::Get().CreateCommandNoInput("ToggleAutoCounter", &AttackDebugger::Get(), &AttackDebugger::ToggleAutoCounter, "Toggle Auto Counter");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandTAZ, "Toggle Auto Counter", KEYS_PRESSED, KEYC_C, KEYM_SHIFT);

	CommandBaseNoInput* pobCommandTNS = CommandManager::Get().CreateCommandNoInput("ToggleNinaSimulator", &AttackDebugger::Get(), &AttackDebugger::ToggleNinaSimulator, "Toggle Nina Simulator");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandTNS, "Toggle Nina Simulator", KEYS_PRESSED, KEYC_N, KEYM_NONE);

	CommandBaseNoInput* pobCommandISL = CommandManager::Get().CreateCommandNoInput("IncrementStyleLevels", &AttackDebugger::Get(), &AttackDebugger::IncrementStyleLevels, "Incrememnt Style Levels");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandISL, "Increment Style Levels", KEYS_PRESSED, KEYC_J, KEYM_NONE);

	CommandBaseNoInput* pobCommandDSL = CommandManager::Get().CreateCommandNoInput("DecrementStyleLevels", &AttackDebugger::Get(), &AttackDebugger::DecrementStyleLevels, "Decrememnt Style Levels");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandDSL, "Decrement Style Levels", KEYS_PRESSED, KEYC_J, KEYM_SHIFT);


	CommandBaseNoInput* pobCommandTGZ = CommandManager::Get().CreateCommandNoInput("ToggleGridDisplay", &AttackDebugger::Get(), &AttackDebugger::ToggleGridDisplay, "Toggle Grid Display");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandTGZ, "Toggle Grid Display", KEYS_PRESSED, KEYC_V, KEYM_NONE);

	CommandBaseNoInput* pobCommandSSV = CommandManager::Get().CreateCommandNoInput("ToggleSuperSafeVolumeDisplay", &AttackDebugger::Get(), &AttackDebugger::ToggleSuperSafeVolumeDisplay, "Toggle Super Safe Volume Display");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandSSV, "Toggle Grid Display", KEYS_PRESSED, KEYC_S, KEYM_SHIFT);

	CommandBaseNoInput* pobCommandTC = CommandManager::Get().CreateCommandNoInput("ToggleCapture", &CaptureSystem::Get(), &CaptureSystem::ToggleCaptureMode, "Toggle Capture");
	KeyBindManager::Get().RegisterKeyNoInput("global", pobCommandTC, "Toggle Capture", KEYS_PRESSED, KEYC_INSERT, KEYM_SHIFT);

	CommandBaseNoInput* pobCommandDSC = CommandManager::Get().CreateCommandNoInput("DoScreenshot", &CaptureSystem::Get(), &CaptureSystem::DoScreenShot, "Create Screenshot");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandDSC, "Create Screenshot", KEYS_PRESSED, KEYC_INSERT, KEYM_CTRL);
	
	// Toggle the combat test menu
	CommandBaseNoInput* pobCommandTTM = CommandManager::Get().CreateCommandNoInput("ToggleTestMenu", TestMenu::GetP(), &TestMenu::ToggleMenu, "Toggle the test menu");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandTTM, "Toggle the test menu", KEYS_PRESSED, KEYC_HOME, KEYM_NONE);
	
	CommandBaseNoInput* pobCommandTSMTM = CommandManager::Get().CreateCommandNoInput("ToggleSingleMoveTestMode", TestMenu::GetP(), &TestMenu::ToggleSingleMoveMode, "Toggle single move/full combo mode for combat test menu");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandTSMTM, "Toggle single move/full combo mode for combat test menu", KEYS_PRESSED, KEYC_S, KEYM_ALT);

	CommandBaseNoInput* pobCommandDCA = CommandManager::Get().CreateCommandNoInput("DumpCategorizedAttacks", TestMenu::GetP(), &TestMenu::DumpCategorizedAttacks, "Dump player's categorized attacks links to output");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandDCA, "Dump player's categorized attacks links to output", KEYS_PRESSED, KEYC_D, KEYM_ALT);
	CommandBaseNoInput* pobCommandDebugSafeRegion = CommandManager::Get().CreateCommandNoInput("ToggleSafeRegion", this, &ShellDebug::CommandToggleSafeDebugLines, "Toggle Debug Safe Lines");
	KeyBindManager::Get().RegisterKeyNoInput("global", pobCommandDebugSafeRegion, "Toogle Debug Safe Lines", KEYS_PRESSED, KEYC_Z, KEYM_CTRL );

	CommandBaseNoInput* pobCommandMBW = CommandManager::Get().CreateCommandNoInput("MovePlayerBackwards", TestMenu::GetP(), &TestMenu::MoveBackwards, "Move player backwards");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandMBW, "Move player backwards", KEYS_PRESSED, KEYC_COMMA, KEYM_NONE);

	CommandBaseNoInput* pobCommandMFW = CommandManager::Get().CreateCommandNoInput("MovePlayerForwards", TestMenu::GetP(), &TestMenu::MoveForwards, "Move player forwards");
	KeyBindManager::Get().RegisterKeyNoInput("combat", pobCommandMFW, "Move player backwards", KEYS_PRESSED, KEYC_PERIOD, KEYM_NONE);

	// Last thing to do when the global keys have been registered....
	// turn off global state
	CommandManager::Get().DisableGlobalState();
	KeyBindManager::Get().DisableGlobalState();
}

//------------------------------------------------------------------------------------------
//!
//!	ShellDebug::RegisterDebugKeysForLevel
//! Register all level based debug keys with the keybind manager. These will be deleted when the level is deleted.
//!
//------------------------------------------------------------------------------------------
void	ShellDebug::RegisterDebugKeysForLevel()
{ 
#ifndef _GOLD_MASTER

// ---- AI ----

// Gav defined m_bDebug out of existance in release
#if !defined(_RELEASE)
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle Formation debug info",	&AIFormationManager::Get().m_bDebug,	KEYS_PRESSED, KEYC_F, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle AI Nav render",			&CAINavGraphManager::Get().m_bRender,	KEYS_PRESSED, KEYC_R, KEYM_NONE);

	KeyBindManager::Get().RegisterKey("global", "ToggleBoolMember", "Toggle ultimo kill 1", &AIFormationManager::Get().m_bKillAllEntities,	KEYS_PRESSED, KEYC_K, KEYM_CTRL);

	CommandBaseNoInput* pobCommandDAC = CommandManager::Get().CreateCommandNoInput("AdvanceDebugAttackContext", &AIFormationManager::Get(), &AIFormationManager::AdvanceDebugAttackContext, "Advance debug attack context");
	KeyBindManager::Get().RegisterKeyNoInput("ai", pobCommandDAC, "Advance debug attack context", KEYS_PRESSED, KEYC_A, KEYM_NONE);
#endif

	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle AI Navigation Graph (Dario)", &CAINavigationSystemMan::Get().m_bRenderNavigGraph,	KEYS_PRESSED, KEYC_N, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle AI Patrol Graph (Dario)", &CAINavigationSystemMan::Get().m_bRenderPatrolGraph,	KEYS_PRESSED, KEYC_L, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle Render AI Avoidance (Dario)", &CAINavigationSystemMan::Get().m_bRenderAIAvoidance,	KEYS_PRESSED, KEYC_B, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle Formation Slot Changing", &CAINavigationSystemMan::Get().m_bFormationSlotChanging,	KEYS_PRESSED, KEYC_F, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle Render Wall Avoidance (Dario)", &CAINavigationSystemMan::Get().m_bRenderWallAvoidance,	KEYS_PRESSED, KEYC_W, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle Render Total Steering Action (Dario)", &CAINavigationSystemMan::Get().m_bRenderTotalSteeringAction,	KEYS_PRESSED, KEYC_T, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle Render AI's cone of view (Dario)", &CAINavigationSystemMan::Get().m_bRenderViewCones,	KEYS_PRESSED, KEYC_V, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle Render AI's cone of view (Dario)", &CAINavigationSystemMan::Get().m_bRenderKnowledge,	KEYS_PRESSED, KEYC_K, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle Render AI World Volumes (Dario)", &CAINavigationSystemMan::Get().m_bRenderAIWorldVolumes,	KEYS_PRESSED, KEYC_M, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle Render Demo Text (Dario)", &CAINavigationSystemMan::Get().m_bRenderVideoText,	KEYS_PRESSED, KEYC_X, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle AI Vol. Collision (Dario)", &CAINavigationSystemMan::Get().m_bCollisionWithAIVolumes,	KEYS_PRESSED, KEYC_C, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle New AI Navigation System (Dario)", &CAINavigationSystemMan::Get().m_bNewNavigationSystem,	KEYS_PRESSED, KEYC_S, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle Render Hearing System (Dario)", &CAIHearingMan::Get().m_bSoundInfoRender,	KEYS_PRESSED, KEYC_H, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle CAIInitialReactionMan debug mode (Dario)", &CAIInitialReactionMan::Get().m_bInitialReactionRender,	KEYS_PRESSED, KEYC_I, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle CAIQueueManager debug mode (Dario)", &CAIQueueManager::Get().m_bDebugRender,	KEYS_PRESSED, KEYC_Q, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("ai", "ToggleBoolMember", "Toggle AI Moving Volley (Dario)", &CAINavigationSystemMan::Get().m_bMovingVolleyDebugRender,	KEYS_PRESSED, KEYC_Z, KEYM_NONE);

	
// ---- Entity browser ----

	KeyBindManager::Get().RegisterKey("game", "ToggleBoolMember", "Ent browser- help",	&CEntityBrowser::Get().m_bHelp,			KEYS_PRESSED, KEYC_I, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("game", "ToggleBoolMember", "Ent browser- anim only",&CEntityBrowser::Get().m_bDisplayAnimOnly,	KEYS_PRESSED, KEYC_S, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("game", "ToggleBoolMember", "Ent browser- render transforms",&CEntityBrowser::Get().m_bRenderTransforms, KEYS_PRESSED, KEYC_T, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("game", "ToggleBoolMember", "Ent browser- render usepoints",&CEntityBrowser::Get().m_bRenderUsePoints, KEYS_PRESSED, KEYC_U, KEYM_NONE);
	KeyBindManager::Get().RegisterKey("game", "ToggleBoolMember", "Ent browser- toggle debug axis",&CEntityBrowser::Get().m_bDebugAxis, KEYS_PRESSED, KEYC_A, KEYM_SHIFT);
	KeyBindManager::Get().RegisterKey("game", "ToggleBoolMember", "Ent browser- toggle collision info",&CEntityBrowser::Get().m_bRenderCollision, KEYS_PRESSED, KEYC_C, KEYM_SHIFT);

	CommandBaseNoInput* pobCommandEEB = CommandManager::Get().CreateCommandNoInput("EBSelectPreviousEntity", &CEntityBrowser::Get(), &CEntityBrowser::ToggleEntityBrowser, "Ent browser - toggle");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandEEB, "Ent browser - select previous entity", KEYS_PRESSED, KEYC_E, KEYM_CTRL);

	CommandBaseNoInput* pobCommandSPE = CommandManager::Get().CreateCommandNoInput("EBSelectPreviousEntity", &CEntityBrowser::Get(), &CEntityBrowser::SelectPreviousEntity, "Ent browser - select previous entity");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandSPE, "Ent browser - select previous entity", KEYS_PRESSED, KEYC_LEFT_ARROW, KEYM_NONE);
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandSPE, "Ent browser - select previous entity", KEYS_HELD, KEYC_LEFT_ARROW, KEYM_SHIFT);

	CommandBaseNoInput* pobCommandSNE = CommandManager::Get().CreateCommandNoInput("EBSelectNextEntity", &CEntityBrowser::Get(), &CEntityBrowser::SelectNextEntity, "Ent browser - select next entity");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandSNE, "Ent browser - select next entity", KEYS_PRESSED, KEYC_RIGHT_ARROW, KEYM_NONE);
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandSNE, "Ent browser - select next entity", KEYS_HELD, KEYC_RIGHT_ARROW, KEYM_SHIFT);

	CommandBaseNoInput* pobCommandSPA = CommandManager::Get().CreateCommandNoInput("EBSelectPreviousAnim", &CEntityBrowser::Get(), &CEntityBrowser::SelectPreviousAnim, "Ent browser - select previous anim");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandSPA, "Ent browser - select previous anim", KEYS_PRESSED, KEYC_UP_ARROW, KEYM_NONE);
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandSPA, "Ent browser - select previous anim", KEYS_HELD, KEYC_UP_ARROW, KEYM_SHIFT);

	CommandBaseNoInput* pobCommandSNA = CommandManager::Get().CreateCommandNoInput("EBSelectNextEntity", &CEntityBrowser::Get(), &CEntityBrowser::SelectNextAnim, "Ent browser - select next anim");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandSNA, "Ent browser - select next anim", KEYS_PRESSED, KEYC_DOWN_ARROW, KEYM_NONE);
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandSNA, "Ent browser - select next anim", KEYS_HELD, KEYC_DOWN_ARROW, KEYM_SHIFT);

	CommandBaseNoInput* pobCommandTMC = CommandManager::Get().CreateCommandNoInput("EBToggleMovementController", &CEntityBrowser::Get(), &CEntityBrowser::ToggleMovementController, "Ent browser - toggle movement controller");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandTMC, "Ent browser - toggle movement controller", KEYS_PRESSED, KEYC_Z, KEYM_NONE);

	CommandBaseNoInput* pobCommandSSA = CommandManager::Get().CreateCommandNoInput("EBStartSelectedAnim", &CEntityBrowser::Get(), &CEntityBrowser::StartSelectedAnim, "Ent browser - start selected anim");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandSSA, "Ent browser - start selected anim", KEYS_PRESSED, KEYC_A, KEYM_NONE);

	CommandBaseNoInput* pobCommandWDA = CommandManager::Get().CreateCommandNoInput("EBStartSelectedAnimWithAttackDuration", &CEntityBrowser::Get(), &CEntityBrowser::StartSelectedAnimWithAttackDuration, "Ent browser - start selected anim with attack duration");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandWDA, "Ent browser - start selected anim with attack duration", KEYS_PRESSED, KEYC_PERIOD, KEYM_CTRL);
	
#ifndef _RELEASE
	CommandBaseNoInput* pobCommandDSL = CommandManager::Get().CreateCommandNoInput("EBDumpSuperLog", &CEntityBrowser::Get(), &CEntityBrowser::DumpSuperLog, "Ent browser - dump super log");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandDSL, "Ent browser - dump super log", KEYS_PRESSED, KEYC_L, KEYM_NONE);
#endif

	CommandBaseNoInput* pobCommandDAE = CommandManager::Get().CreateCommandNoInput("EBToggleDebugAnimEvents", &CEntityBrowser::Get(), &CEntityBrowser::ToggleDebugAnimEvents, "Ent browser - toggle debug anim events");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandDAE, "Ent browser - toggle debug anim events", KEYS_PRESSED, KEYC_E, KEYM_NONE);

	CommandBaseNoInput* pobCommandRAEL = CommandManager::Get().CreateCommandNoInput("EBRebuildAnimEventLists", &CEntityBrowser::Get(), &CEntityBrowser::RebuildAnimEventLists, "Ent browser - rebuild anim event lists");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandRAEL, "Ent browser - rebuild anim event lists", KEYS_PRESSED, KEYC_R, KEYM_NONE);

	CommandBaseNoInput* pobCommandTSD = CommandManager::Get().CreateCommandNoInput("EBToggleSkinningDebug", &CEntityBrowser::Get(), &CEntityBrowser::ToggleSkinningDebug, "Ent browser - toggle skinning debug");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandTSD, "Ent browser - toggle skinning debug", KEYS_PRESSED, KEYC_B, KEYM_NONE);

	CommandBaseNoInput* pobCommandLDS = CommandManager::Get().CreateCommandNoInput("AdvanceLuaDebugState", &CEntityBrowser::Get(), &CEntityBrowser::AdvanceLuaDebugState, "Advance Lua debug state");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandLDS, "Advance Lua debug state", KEYS_PRESSED, KEYC_S, KEYM_CTRL);

 	CommandBaseNoInput* pobCommandEMR = CommandManager::Get().CreateCommandNoInput("EntityManagerReport", &CEntityManager::Get(), &CEntityManager::GenerateReport, "Entity manager report");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCommandEMR, "Entity manager report", KEYS_PRESSED, KEYC_E, KEYM_SHIFT | KEYM_CTRL);

#ifndef _GOLD_MASTER
	CommandBaseNoInput *pobCommandMoviesSkip = CommandManager::Get().CreateCommandNoInput( "SkipCurrentMovies", &MoviePlayer::Get(), &MoviePlayer::SkipCurrentMoviesCallback, "Skip all currently playing movies." );
	KeyBindManager::Get().RegisterKeyNoInput( "game", pobCommandMoviesSkip, "Skip all currently playing movies.", KEYS_PRESSED, KEYC_SPACE, KEYM_NONE );
#endif

#ifdef _REGISTER_GUINOTIFY_KEYS
	if (!CGuiNotifyKeys::Exists())
		NT_NEW CGuiNotifyKeys();
	CGuiNotifyKeys::Get().Register();
#endif

#endif
	
	KeyBindManager::Get().RegisterKey("game", "ToggleBoolMember", "Toggle Trigger Volume rendering",	&CTriggerVolumeManager::Get().m_bDebugRender,	KEYS_PRESSED, KEYC_T, KEYM_CTRL);
	KeyBindManager::Get().RegisterKey("sound", "ToggleBoolMember", "Toggle ChatterBox debug mode (Dario)", &CChatterBoxMan::Get().m_bRender,	KEYS_PRESSED, KEYC_C, KEYM_NONE);
}

//-----------------------------------------------------
//!
//! Setups the debug logging system, there are number of
//! different channels, each one can be sent to a number
//! of different places.
//!
//-----------------------------------------------------
void	ShellDebug::SetupLogging( void )
{
	// make sure we are in the content_pc directory
	Util::SetToPlatformResources();

	// redirect Core output to core.log file
	m_pCoreLogFile = Debug::CreateNewLogFile( "core.log" );
	Debug::SetLogToFile( Debug::DCU_CORE, m_pCoreLogFile );
	Debug::SetLogToConsole( Debug::DCU_CORE, false );

	m_pLuaLogFile = Debug::CreateNewLogFile( "lua.log" );
	Debug::SetLogToFile( Debug::DCU_LUA, m_pLuaLogFile );
	Debug::SetLogToConsole( Debug::DCU_LUA, false );

	m_pGameLogFile = Debug::CreateNewLogFile( "game.log" );
	Debug::SetLogToFile( Debug::DCU_GAME, m_pGameLogFile );
	Debug::SetLogToConsole( Debug::DCU_GAME, true );

	m_pGfxLogFile = Debug::CreateNewLogFile( "gfx.log" );
	Debug::SetLogToConsole( Debug::DCU_GRAPHICS, false );
	Debug::SetLogToFile( Debug::DCU_GRAPHICS, m_pGfxLogFile );

	m_pAILogFile = Debug::CreateNewLogFile( "ai.log" );
	Debug::SetLogToConsole( Debug::DCU_AI, false );
	Debug::SetLogToFile( Debug::DCU_AI, m_pAILogFile );

	m_pAICBLogFile = Debug::CreateNewLogFile( "ai_cb.log" );
	Debug::SetLogToConsole( Debug::DCU_AI_CHBOX, false );
	Debug::SetLogToFile( Debug::DCU_AI_CHBOX, m_pAICBLogFile );

	m_pAIBehaveLogFile = Debug::CreateNewLogFile( "ai_behave.log" );
	Debug::SetLogToConsole( Debug::DCU_AI_BEHAVE, false );
	Debug::SetLogToFile( Debug::DCU_AI_BEHAVE, m_pAICBLogFile );

	// resource logs

	m_pTextureLogFile = Debug::CreateNewLogFile( "texture.log" );
	Debug::SetLogToConsole( Debug::DCU_TEXTURE, false );
	Debug::SetLogToFile( Debug::DCU_TEXTURE, m_pTextureLogFile );

	m_pClumpLogFile = Debug::CreateNewLogFile( "clump.log" );
	Debug::SetLogToConsole( Debug::DCU_CLUMP, false );
	Debug::SetLogToFile( Debug::DCU_CLUMP, m_pClumpLogFile );

	m_pAnimLogFile = Debug::CreateNewLogFile( "anim.log" );
	Debug::SetLogToConsole( Debug::DCU_ANIM, false );
	Debug::SetLogToFile( Debug::DCU_ANIM, m_pAnimLogFile );

	m_pResourceLogFile = Debug::CreateNewLogFile( "resource.log" );
	Debug::SetLogToConsole( Debug::DCU_RESOURCES, false );
	Debug::SetLogToFile( Debug::DCU_RESOURCES, m_pResourceLogFile );

	m_pAreaLoadLogFile = Debug::CreateNewLogFile( "areaload.log" );
	Debug::SetLogToConsole( Debug::DCU_AREA_LOAD, false );
	Debug::SetLogToFile( Debug::DCU_AREA_LOAD, m_pAreaLoadLogFile );




	// user log profiles. 
	// Feel free to override any of the normal settings above
	// in your personal log settings i.e. if you want to see graphics log on console
	// whack a 	Debug::SetLogToConsole( Debug::DCU_GRAPHICS, true ); in you USER_XXX
	// section
#if defined( USER_DEANC )
	// redirect Deano's log to the console
	m_pUserLogFile = Debug::CreateNewLogFile( "deano.log" );
	Debug::SetLogToConsole( Debug::DCU_USER_DEANO, true );
	Debug::SetLogToFile( Debug::DCU_USER_DEANO, m_pUserLogFile );
	Debug::SetLogToConsole( Debug::DCU_CORE, true );
	Debug::SetLogToConsole( Debug::DCU_GRAPHICS, true );
	m_pUserLogFile = 0;
#endif

	Debug::SetCurrentChannel( Debug::DCU_GAME );

	// make sure we are in the content_neutral directory
	Util::SetToNeutralResources();
}

//-----------------------------------------------------------------------------------------
//! 
//! Close all the log resources
//!
//----------------------------------------------------------------------------------------
void ShellDebug::ReleaseLogs( void )
{
	NT_DELETE( m_pUserLogFile );
	m_pUserLogFile = 0;
	NT_DELETE( m_pGfxLogFile );
	m_pGfxLogFile = 0;
	NT_DELETE( m_pGameLogFile );
	m_pGameLogFile = 0;
	NT_DELETE( m_pLuaLogFile );
	m_pLuaLogFile = 0;
	NT_DELETE( m_pCoreLogFile );
	m_pCoreLogFile = 0;
	NT_DELETE( m_pAILogFile );
	m_pAILogFile = 0;
	NT_DELETE( m_pAICBLogFile );
	m_pAILogFile = 0;
	NT_DELETE( m_pAIBehaveLogFile );
	m_pAILogFile = 0;

	NT_DELETE( m_pTextureLogFile );
	m_pTextureLogFile = 0;
	NT_DELETE( m_pClumpLogFile );
	m_pClumpLogFile = 0;
	NT_DELETE( m_pAnimLogFile );
	m_pAnimLogFile = 0;
	NT_DELETE( m_pResourceLogFile );
	m_pResourceLogFile = 0;
	NT_DELETE( m_pAreaLoadLogFile );
	m_pAreaLoadLogFile = 0;

#if defined( USER_DEANC )
	Debug::SetLogToFile( Debug::DCU_USER_DEANO, 0 );
#endif

	Debug::SetLogToFile( Debug::DCU_TEXTURE, 0 );
	Debug::SetLogToFile( Debug::DCU_CLUMP, 0 );
	Debug::SetLogToFile( Debug::DCU_ANIM, 0 );
	Debug::SetLogToFile( Debug::DCU_RESOURCES, 0 );
	Debug::SetLogToFile( Debug::DCU_AREA_LOAD, 0 );

	Debug::SetLogToFile( Debug::DCU_GRAPHICS, 0 );
	Debug::SetLogToFile( Debug::DCU_GAME, 0 );
	Debug::SetLogToFile( Debug::DCU_LUA, 0 );
	Debug::SetLogToFile( Debug::DCU_CORE, 0 );
}

//-----------------------------------------------------------------------------------------
//! 
//! Init some debug vars
//!
//----------------------------------------------------------------------------------------
ShellDebug::ShellDebug() :
	m_pUserLogFile(0),

	m_pTextureLogFile(0),
	m_pClumpLogFile(0),
	m_pAnimLogFile(0),
	m_pResourceLogFile(0),
	m_pAreaLoadLogFile(0),

	m_pGfxLogFile(0),
	m_pGameLogFile(0),
	m_pLuaLogFile(0),
	m_pExecLogFile(0),
	m_pCoreLogFile(0),
	m_pAILogFile(0),
	m_pAICBLogFile(0),
	m_pAIBehaveLogFile(0),

	m_bSingleStep( false ),
	m_bDebugRenderShell( false ),
	m_bRenderDebugSafeAreaLines( false )
{
	// Satisfy the compiler on builds where none of the simple commands are referenced
	g_sundryCommandsObject.m_iStubValue = 0;

	// yes evil but also so very very simple....
	m_AppVersionNumber[0] = 
#include "version_num.h"
	;
	m_AppVersionNumber[1] = 
#include "minor_version_num.h"
	;
	m_LevelVersionNumber = 0;	
}

//-----------------------------------------------------------------------------------------
//! 
//! ShellDebug::ForceMaterials
//!
//----------------------------------------------------------------------------------------
void ShellDebug::ForceMaterials( RENDER_QUALITY_MODE eRenderMode )
{
	// Update the effects that poke material properties
	CEffectSpecial::Clear();

	// get ALL entities
	CEQCAlways obOverOptimisticQuery;
	CEntityQuery obQuery;
	obQuery.AddClause( obOverOptimisticQuery );
	CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_All );

	// either force lambert or go back to the exported version (signified by 0)
	CMaterial const* pobMaterial = 0;
	if( !CRendererSettings::bEnableMaterials )
	{
		pobMaterial = ShaderManager::Get().FindMaterial( ( eRenderMode == RENDER_QUALITY_VERTEX_LIT ) ? "null" : "lambert" );
		ntAssert( pobMaterial );
	}

	// force this material on all entites
	for( QueryResultsContainerType::const_iterator obIt = obQuery.GetResults().begin();
			obIt != obQuery.GetResults().end(); ++obIt )
	{
		if	(
			( ( *obIt )->GetRenderableComponent() != 0 ) &&
			( ( *obIt )->IsAreaResFixedUp() )
			)
		{
			( *obIt )->GetRenderableComponent()->ForceMaterial( pobMaterial );
		}
	}
	for( ntstd::List<Static*>::const_iterator obIt = CEntityManager::Get().GetStaticEntityList().begin();
			obIt != CEntityManager::Get().GetStaticEntityList().end(); ++obIt )
	{
		if	(
			( ( *obIt )->GetRenderableComponent() != 0 ) &&
			( ( *obIt )->IsAreaResFixedUp() )
			)
		{
			( *obIt )->GetRenderableComponent()->ForceMaterial( pobMaterial );
		}
	}
	// force material on all renderables
	for( ListSpace::RenderableListType::const_iterator obIt = CSector::Get().GetRenderables().GetRenderableList().begin(); 
		 obIt != CSector::Get().GetRenderables().GetRenderableList().end(); ++obIt )
	{
		if ( ( *obIt )->GetRenderableType() == CRenderable::RT_MESH_INSTANCE )
		{
			CMeshInstance* pMeshInstance = (CMeshInstance*)((const_cast<CRenderable*>(*obIt) ));
			pMeshInstance->ForceMaterial( pobMaterial );
		}
	}

	// Update the effects that poke material properties
	CEffectSpecial::Build();

#ifdef PLATFORM_PS3
	// Update materials on all army renderables
	ArmyManager::Get().ForceMaterials( pobMaterial );
#endif
}

//-----------------------------------------------------------------------------------------
//! 
//! ShellDebug::UpdateGlobal
//! Check for debug keyboard input that just affects global settings
//!
//----------------------------------------------------------------------------------------
void	ShellDebug::UpdateGlobal()
{
	if (ShellMain::Get().HaveLoadedLevel())
		g_VisualDebug->SetDefaultCamera( CamMan::GetPrimaryView() );

#ifdef	DEBUG_KEYBOARD

	// Process the registered keys
	KeyBindManager::Get().ProcessKeys();

	if(CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_S, KEYM_SHIFT ))
		m_bDebugRenderShell = !m_bDebugRenderShell;

	// Debug Keyboard reads
	//-------------------------------

	CInputKeyboard* pobKeyboard = CInputHardware::Get().GetKeyboardP();

#ifdef PLATFORM_PC

	// KEY: ESCAPE			-		Quit if the user pressed escape
	
	if ( pobKeyboard->IsKeyPressed(KEYC_ESCAPE) )
	{
		SendMessage( (HWND)GraphicsDevice::Get().m_Platform.GetHwnd(), WM_CLOSE, 0, 0);
	}

#endif

	// KEY: SHIFT-SLASH		-		Display Gatsos

	if ( pobKeyboard->IsKeyPressed( KEYC_SLASH, KEYM_SHIFT ) )
		CGatso::Dump();

	// KEY: NUMBERS			-		Adjust game time scalar to speed up or slow down things

	if ( pobKeyboard->IsKeyPressed( KEYC_KPAD_PLUS ) ) // HC: I'm not sure why this was commented out, but I re-enabled this because it's needed!
		CTimer::Get().UpdateOneFrame();

	if ( pobKeyboard->IsKeyPressed( KEYC_KPAD_MINUS ) ) // Frame stepping at 25% the normal time step
		CTimer::Get().UpdateOneFrame(0.25);

	if ( pobKeyboard->IsKeyPressed( KEYC_PAGE_DOWN ) ) // Allow a single step of a paused level
		m_bSingleStep = true;
	else
		m_bSingleStep = false;

#endif // DEBUG_KEYBOARD

	CNetEditInterface::Get().Update();

	DebugHUD::Get().Update();

#ifdef PLATFORM_PC
	Debug::FlushLog();
#endif

	FXMaterialManager::Get().DebugUpdate();
}

//-----------------------------------------------------------------------------------------
//! 
//! ShellDebug::UpdateLevel
//! Check for debug input, that requires / affects level specific objects
//!
//----------------------------------------------------------------------------------------
void	ShellDebug::UpdateLevel()
{
	PAD_NUMBER iWhichPad = CamMan::Get().GetDebugCameraPadNumber();

	// Debug Pad reads
	//-------------------------------
	if ( CInputHardware::Get().GetPadContext() == PAD_CONTEXT_DEBUG )
	{
		// swap debug pad input mode
		if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_ENTER ) )
			CInputHardware::Get().SetPadContext( PAD_CONTEXT_GAME );

		// swap keyboard input contexts
		if ( CInputHardware::Get().GetPad( iWhichPad ).GetPressed() & PAD_LEFT )
		{
			INPUT_CONTEXT eContext = CInputHardware::Get().GetContext();
			
			if( eContext != INPUT_CONTEXT_GAME )
			{
				CInputHardware::Get().SetContext( INPUT_CONTEXT_GAME );
				OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "changed context to: \"%s\"" , g_apcContextTitleStrings[INPUT_CONTEXT_GAME]);
			}
			else
			{
				CInputHardware::Get().SetContext( INPUT_CONTEXT_CAMERA_DEBUG );
				OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "changed context to: \"%s\"" , g_apcContextTitleStrings[INPUT_CONTEXT_CAMERA_DEBUG]);
			}
		}

		// swap camera mode
		if(CInputHardware::Get().GetPad(iWhichPad).GetPressed() & PAD_RIGHT)
		{
			if(CamMan::Get().GetView(1))
				CamMan::Get().GetView(1)->SwitchDebugCameraMode();
			else
				CamMan::GetPrimaryView()->SwitchDebugCameraMode();
		}

		if(CInputHardware::Get().GetPad(iWhichPad).GetPressed() & PAD_UP)
			CamMan::GetPrimaryView()->SwitchDebugCameraStyle();

		// debug render cameras
		if(CInputHardware::Get().GetPad(iWhichPad).GetPressed() & PAD_START)
			CamMan::GetPrimaryView()->ToggleDebugRender();

		// weird white button processing.
		static INPUT_CONTEXT eLastNonRenderingContext = INPUT_CONTEXT_GAME;
		if( ( CInputHardware::Get().GetPad( iWhichPad ).GetPressed() & PAD_TOP_2 ) != 0 )
		{
			eLastNonRenderingContext = CInputHardware::Get().GetContext();
			CInputHardware::Get().SetContext( INPUT_CONTEXT_RENDER_DEBUG );
		}
		if( ( CInputHardware::Get().GetPad( iWhichPad ).GetReleased() & PAD_TOP_2 ) != 0 )
			CInputHardware::Get().SetContext( eLastNonRenderingContext );

		// accelerate time of day
		static const float fTimeSpeed = 1.0f / 3.0f;

		float fTime = LevelLighting::Get().GetTimeOfDay();
		fTime += CInputHardware::Get().GetPad( iWhichPad ).GetButtonFrac( PAD_ANALOG_TOP_4 )*fTimeSpeed*0.25f;
		fTime -= CInputHardware::Get().GetPad( iWhichPad ).GetButtonFrac( PAD_ANALOG_TOP_3 )*fTimeSpeed*0.25f;

		if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_KPAD_PLUS ) )
		{
			fTime += 1.0f / 60.0f;	// Advance 1 minute
		}
		if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_KPAD_MINUS ) )
		{
			fTime -= 1.0f / 60.0f;	// Go back 1 minute
		}

		// Set the time back
		LevelLighting::Get().SetTimeOfDay( fTime );
	}
	else if ( CInputHardware::Get().GetPadContext() == PAD_CONTEXT_GAME )
	{
		// swap debug pad input mode
		if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_ENTER ) )
			CInputHardware::Get().SetPadContext( PAD_CONTEXT_DEBUG );
	}

#ifdef	DEBUG_KEYBOARD

	// Debug Keyboard reads
	//-------------------------------
	CInputKeyboard* pobKeyboard = CInputHardware::Get().GetKeyboardP();

	// camera debug interface
	if( CInputHardware::Get().GetContext() == INPUT_CONTEXT_CAMERA_DEBUG )
	{
		if( pobKeyboard->IsKeyPressed( KEYC_SPACE ) ) 
			CamMan::GetPrimaryView()->SwitchDebugCameraMode();
		if( pobKeyboard->IsKeyPressed( KEYC_D ) )
			CamMan::GetPrimaryView()->ToggleDebugRender();
	}

	// global quality stuff
	static double dSysTimeSilent = 0.0f;
	bool bSwitchMaterials = false;

#ifdef PLATFORM_PS3
	if( pobKeyboard->IsKeyPressed( KEYC_DELETE ) )
	{
		bSwitchMaterials = true;
		CRendererSettings::iDebugLayer = (CRendererSettings::iDebugLayer + 1) %  3;
	}
#endif

	if( pobKeyboard->IsKeyPressed( KEYC_Q ) || (g_ShellOptions->m_renderQuality != s_renderMode) )
	{
		if ( pobKeyboard->IsKeyPressed( KEYC_Q ) )
			g_ShellOptions->ChangeRenderQuality();
		s_renderMode = g_ShellOptions->m_renderQuality;
		
		CRendererSettings::iShadowQuality = (( s_renderMode < RENDER_QUALITY_LOW_SHADOWS ) ? 3 : 1 );
		CRendererSettings::bEnableShadows = (( s_renderMode < RENDER_QUALITY_NO_SHADOWS ) ? true : false );
		CRendererSettings::bEnableMaterials = (( s_renderMode < RENDER_QUALITY_FASTEST ) ? true : false );
		CRendererSettings::bEnableLensEffects = (( s_renderMode < RENDER_QUALITY_FASTEST ) ? true : false );
		CRendererSettings::bEnableDepthHaze = (( s_renderMode < RENDER_QUALITY_FASTEST ) ? true : false );
		bSwitchMaterials = true;

		// display the message for 2 seconds
		dSysTimeSilent = CTimer::Get().GetSystemTime() + 2.0;
	}
	if( CTimer::Get().GetSystemTime() < dSysTimeSilent )
	{
		// quality settings
		char const* apcQuality[] = { "best", "low quality shadows", "no shadows", "fastest possible", "vertex lighting" };
		g_VisualDebug->Printf2D(
			g_VisualDebug->GetDebugDisplayWidth() * 0.5f,
			g_VisualDebug->GetDebugDisplayHeight() * 0.5f,
			NTCOLOUR_ARGB( 0xff, 0x7f, 0xff, 0x7f ), DTF_ALIGN_HCENTRE | DTF_ALIGN_VCENTRE, 
			"rendering quality: %s", apcQuality[s_renderMode] 
		);
	}

	// renderer debug interface
	if	(
		( CInputHardware::Get().GetContext() == INPUT_CONTEXT_RENDER_DEBUG  ) ||
		( !CRendererSettings::bEnableDebugPrimitives  )
		)
	{
		// print help and toggle switches
		if( pobKeyboard->IsKeyPressed( KEYC_H ) )
			ntPrintf( "--\nRenderer Debug Keys\n--\n" );

		bool bOldExposure = CRendererSettings::bUseGPUExposure;

		if( pobKeyboard->IsKeyPressed( KEYC_M ) || pobKeyboard->IsKeyPressed( KEYC_K ) )
			bSwitchMaterials = true;
		if( pobKeyboard->IsKeyPressed( KEYC_H ) )
		{
			ntPrintf( "D-PAD UP/DOWN - Manual Exposure Control (When Auto Exposure Is Off!)\n" );
			ntPrintf( "--\n" );
		}
		if( pobKeyboard->IsKeyPressed( KEYC_O ) )
			EffectManager::ToggleRender();

		if( pobKeyboard->IsKeyPressed( KEYC_O, KEYM_CTRL ) )
			CombatEffectsDefinition::s_bCombatEffectsDisabled = !CombatEffectsDefinition::s_bCombatEffectsDisabled;

		if( pobKeyboard->IsKeyPressed( KEYC_P, KEYM_CTRL ) )
			CLODManager::Get().ForceLODOffset( CLODManager::Get().GetForcedLODOffset()+1 );
		else if ( pobKeyboard->IsKeyPressed( KEYC_P, KEYM_SHIFT ) )
			CLODManager::Get().ForceLODOffset( CLODManager::Get().GetForcedLODOffset()-1 );

		if (bOldExposure != CRendererSettings::bUseGPUExposure)
			g_ResetExposure = true;

		if( pobKeyboard->IsKeyPressed( KEYC_SPACE ) )
		{
			if (FXMaterialManager::Exists())
				FXMaterialManager::Get().ForceRecompile();

			if (EffectResourceMan::Exists())
				EffectResourceMan::Get().ForceRecompile();
		}

		Renderer::Get().DebugRender(sfDebugLeftBorder, sfContextTopBorder, sfDebugLineSpacing);
	}

	// perform the material change if necessary
	if( bSwitchMaterials )
		ForceMaterials( s_renderMode );
#endif
}

//-----------------------------------------------------------------------------------------
//! 
//! ShellDebug::Render
//!
//----------------------------------------------------------------------------------------
void	ShellDebug::Render()
{
	float fWidth = DisplayManager::Get().GetInternalWidth();
	float fHeight = DisplayManager::Get().GetInternalHeight();

#ifdef PLATFORM_PS3
	if (g_ShellOptions->m_bNetworkAvailable == false)
	{
		g_VisualDebug->Printf2D(fWidth*0.5f, 40.0f, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "WARNING! NETWORK UNAVAILABLE" );
	}
#endif

	if( m_bRenderDebugSafeAreaLines )
	{
		// safe area is internal 85% of width and height;
		float fLeft = fWidth*0.075f;
		float fRight = fWidth-fLeft;
		float fTop = fHeight*0.075f;
		float fBottom = fHeight-fTop;

		g_VisualDebug->RenderLine( CPoint( fLeft, fTop, 0.0f ),		CPoint( fRight, fTop, 0.0f ), DC_RED, DPF_DISPLAYSPACE  ); 
		g_VisualDebug->RenderLine( CPoint( fRight, fTop, 0.0f ),	CPoint( fRight, fBottom, 0.0f ), DC_RED, DPF_DISPLAYSPACE );
		g_VisualDebug->RenderLine( CPoint( fLeft, fBottom, 0.0f ),	CPoint( fRight, fBottom, 0.0f ), DC_RED, DPF_DISPLAYSPACE );
		g_VisualDebug->RenderLine( CPoint( fLeft, fTop, 0.0f ),		CPoint( fLeft, fBottom, 0.0f ), DC_RED, DPF_DISPLAYSPACE );
	}

	// Print some debug text
	if(m_bDebugRenderShell)
	{
		float fX = sfDebugLeftBorder;
		float fY = sfDebugTopBorder;

		// Display version information
		g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0xff,0xff), 0, "HS VERSION %i.%i LEVEL VERSION %i",m_AppVersionNumber[0], m_AppVersionNumber[1], m_LevelVersionNumber  );
		fY += sfDebugLineSpacing*1.0f;

		// Display context information
		g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xd0,0xd0,0xd0), 0, "Context: %s", g_apcContextTitleStrings[CInputHardware::Get().GetContext()]); 
		fY += sfDebugLineSpacing*1.0f;

		// Obtain time of day info
		float fHours = 0.f;
		float fMins = 0.f;
		if (LevelLighting::Exists())
			LevelLighting::Get().GetTimeOfDay( fHours, fMins );

		// Obtain Player pos info
		CPoint obPlayerPos(CONSTRUCT_CLEAR);
		if	(
			(CEntityManager::Exists()) &&
			(CEntityManager::Get().GetPlayer())
			)
		{
			obPlayerPos = CEntityManager::Get().GetPlayer()->GetPosition();
		}

		// Display the general info
		g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xd0,0xd0,0xd0), 0, "Game Time: %4.1f x %.2f ToD:%02.0f:%02.0f PP:X:%04.2f Y:%04.2f Z:%04.2f", CTimer::Get().GetGameTime(), CTimer::Get().GetGameTimeScalar(), fHours, fMins, obPlayerPos.X(), obPlayerPos.Y(), obPlayerPos.Z() );
		fY += sfDebugLineSpacing*1.0f;

		// Display the pause status of the game
		if ( ShellMain::Get().IsPausedByCode() )
		{
			g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "CODE PAUSE" );
			fY += sfDebugLineSpacing*1.0f;
		}

		if ( ShellMain::Get().IsPausedByUser() )
		{
			g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "USER PAUSE" );
			fY += sfDebugLineSpacing*1.0f;
		}

		if ( ShellMain::Get().IsPausedBySystem() )
		{
			g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SYSTEM PAUSE" );
			fY += sfDebugLineSpacing*1.0f;
		}

		switch (ShellMain::Get().GetShellState())
		{
		case ShellMain::SS_STARTUP:			g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_STARTUP" );		break;
		case ShellMain::SS_CHECK_GAME_DATA:	g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_CHECK_GAME_DATA" );break;
		case ShellMain::SS_LOAD_PLAYER_OPTIONS:	g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_LOAD_PLAYER_OPTIONS" );break;
		case ShellMain::SS_LOAD_PLAYER_PROGRESSION:	g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_LOAD_PLAYER_PROGRESSION" );break;
		case ShellMain::SS_CHECK_SYS_CACHE:	g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_CHECK_SYS_CACHE" );break;
		case ShellMain::SS_INSTALL_FE:		g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_INSTALL_FE" );break;
		case ShellMain::SS_CREATE_GAME:		g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_CREATE_GAME" );	break;
		case ShellMain::SS_INSTALL_GLOBALS:		g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_INSTALL_GLOBALS" );break;
		case ShellMain::SS_RUNNING_EMPTY:	g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_RUNNING_EMPTY" );	break;
		case ShellMain::SS_CREATE_LEVEL:	g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_CREATE_LEVEL" );	break;
		case ShellMain::SS_RUNNING_LEVEL:	g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_RUNNING_LEVEL" );	break;
		case ShellMain::SS_SHUTDOWN_LEVEL:	g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_SHUTDOWN_LEVEL" );	break;
		case ShellMain::SS_SHUTDOWN_GAME:	g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_SHUTDOWN_GAME" );	break;
		case ShellMain::SS_EXIT:			g_VisualDebug->Printf2D(fX, fY, NTCOLOUR_ARGB(0xff,0xff,0,0), 0, "SS_EXIT" );			break;
		}
		fY += sfDebugLineSpacing*1.0f;

		// put out our timings
		float fFrameIntervalSecs = 1.0f / GraphicsDevice::Get().GetGameRefreshRate();

		float RSXPercent = (Renderer::Get().GetGPUFrameTime() / fFrameIntervalSecs) * 100.0f;
		float CELLPercent = (Renderer::Get().GetCPUFrameTime() / fFrameIntervalSecs) * 100.0f;

		float RSX_fps = 1.f / Renderer::Get().GetGPUFrameTime();
		float CELL_fps = 1.f / Renderer::Get().GetCPUFrameTime();

		const char* pBound = "Within Budget";
		uint32_t col = NTCOLOUR_ARGB(0xff,0xd0,0xd0,0xd0);
		if ((RSXPercent > 101.0f) || (CELLPercent > 101.0f))
		{
			col = NTCOLOUR_ARGB(0xff,0xff,0,0);
			
			if (RSXPercent > CELLPercent)
				pBound = "Rendering Bound";
			else
				pBound = "Code Bound";
		}

		g_VisualDebug->Printf2D(fX, fY, col, 0, "CELL: %.0f FPS (%.2f%%) RSX: %.0f FPS (%.2f%%).  %s",
			CELL_fps, CELLPercent, RSX_fps, RSXPercent, pBound );

		fY += sfDebugLineSpacing*1.0f;

#ifdef PLATFORM_PS3
		if (g_ShellOptions->m_bNetworkAvailable == true)
		{
			static bool s_bRetrieved = false;
			static char s_ip_addresss[32];

			if (s_bRetrieved == false)
			{
				int ret;
				union CellNetCtlInfo info;
				// NB this will assert if cable disconnected from a DEBUG build...
				ret = cellNetCtlGetInfo( CELL_NET_CTL_INFO_IP_ADDRESS, &info );
				ntAssert_p( ret >= 0, ("Could not retrieve IP address of game network") );

				NT_MEMCPY( s_ip_addresss, info.ip_address, sizeof(info.ip_address) );
				s_bRetrieved = true;
			}

			g_VisualDebug->Printf2D(fWidth*0.5f, 40.0f, NTCOLOUR_ARGB( 0xff, 0xff, 0xff, 0xff ), 0, "GAME IP: %s", s_ip_addresss );
		}
#endif
	}

	AttackDebugger::Get().Update(CTimer::Get().GetGameTimeChange());

	if (CAINavGraphManager::Exists())
		CAINavGraphManager::Get().DebugRender();
	
	if (AIPatrolManager::Exists())
		AIPatrolManager::Get().DebugRender();
	
	if (AICoverManager::Exists())
		AICoverManager::Get().DebugRender();

	if (CEntityBrowser::Exists())
		CEntityBrowser::Get().DebugRender();

	if (GameAudioManager::Exists())
		GameAudioManager::Get().DebugRender();

	if (NSManager::Exists())
		NSManager::Get().DebugRender();

	// Update the OSD
	OSD::Update(CTimer::Get().GetSystemTimeChange());

	if (WaterManager::Exists())
		WaterManager::Get().DebugRender();

	// Check if we have to reload shaders
	if ( ShaderManager::Get().m_bForceShadersReload )
	{
		ShaderManager::Get().m_bForceShadersReload = false;
		ShaderManager::Get().ReloadShaders();
		ForceMaterials(s_renderMode);
	}
}

#endif // _GOLD_MASTER
