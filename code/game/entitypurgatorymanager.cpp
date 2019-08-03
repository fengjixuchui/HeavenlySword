//--------------------------------------------------
//!
//!	\file game/entitypurgatorymanager.cpp
//!	Definition of the Purgatory Manager
//!
//--------------------------------------------------


#include "objectdatabase/dataobject.h"
#include "game/entitypurgatorymanager.h"
#include "game/checkpointmanager.h"
#include "lua/ninjalua.h"

#include "camera/basiccamera.h"
#include "gfx/meshinstance.h"

#include "anim/animator.h"

#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/camtransition.h"

#include "gui/guiscreen.h"
#include "gui/guilua.h"
#include "gui/guimanager.h"
#include "gui/guisound.h"
#include "gui/purgatory/guiskincheckpointselect.h"

#include "core/hashcodes.h"

#define HASH_STRING_PURG_MAINMENU_CAMERA HASH_DECLARE_HASHED_STRING( 0xf00e6df4, "MainMenu" )
#define HASH_STRING_PURG_CHAPTERSELECT_CAMERA HASH_DECLARE_HASHED_STRING( 0xaae21df, "ChapterSelect" )
#define HASH_STRING_PURG_SPECIALFEATURES_CAMERA HASH_DECLARE_HASHED_STRING( 0x9dc8c2cc, "SpecialFeatures" )
#define HASH_STRING_PURG_OPTIONS_CAMERA HASH_DECLARE_HASHED_STRING( 0xe0773ce4, "Options" )

/*
void ForceLinkFunctionEntityPurgatoryManager()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionEntityPurgatoryManager() !ATTN!\n");
}
*/
START_CHUNKED_INTERFACE(SwordInfoDef, Mem::MC_ENTITY)
	PUBLISH_VAR_AS( m_iChapterNumber, ChapterNumber )
	PUBLISH_VAR_AS( m_obSwordChar, SwordChar )
	PUBLISH_VAR_AS( m_iCheckpointIndexOffset, CheckpointIndexOffset)
	PUBLISH_PTR_AS( m_pobCamera, MenuCamera )
	PUBLISH_PTR_AS( m_pobSword, Sword )
	PUBLISH_PTR_AS( m_pobNextSword, NextSword)
	PUBLISH_PTR_AS( m_pobPreviousSword, PreviousSword)
END_STD_INTERFACE


START_CHUNKED_INTERFACE(Object_PurgatoryManager, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	OVERRIDE_DEFAULT(CastShadows, "false")
	OVERRIDE_DEFAULT(RecieveShadows, "false")

	PUBLISH_PTR_AS( m_pobMenuCamera, MenuCamera )
	PUBLISH_PTR_AS( m_pobOptionsCamera, OptionsCamera )
	PUBLISH_PTR_AS( m_pobSpecialFeaturesCamera, SpecialFeaturesCamera )

	PUBLISH_PTR_CONTAINER_AS(m_obSwords, Swords)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//  Object_PurgatoryManager - Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START_INHERITED(Object_PurgatoryManager, CEntity)

	LUA_EXPOSED_METHOD(NotifyScreenEnter,		NotifyScreenEnter,		"Notify the manager that a new screen has been entered", "screen", "") 
	LUA_EXPOSED_METHOD(SetupPurgatoryMenu,		SetupPurgatoryMenu,		"setup gui", "screen", "") 

LUA_EXPOSED_END(Object_PurgatoryManager)


//--------------------------------------------------
//!
//! Object Fire State Machine
//!
//--------------------------------------------------
STATEMACHINE(OBJECT_PURGATORYMANAGER_FSM, Object_PurgatoryManager)
	//Not sure if i should be placing variables or methods here =)
	FSM::StateBase* m_pViewWaitNextState;	/// Next state after waiting for the view to come round.
	int m_iChapterCompleted;	/// Last chapter the player completed,
	int m_iChapterUnlocked;	/// Last chapter the player completed,

	static bool ViewReady()
	{
		return CamMan::Get().GetPrimaryView() != NULL;
	}

	OBJECT_PURGATORYMANAGER_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
		m_pViewWaitNextState = NULL;
		m_iChapterCompleted = -1;
		m_iChapterUnlocked = -1;
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
//				ntPrintf("State: DEFAULT  Event: ON_ENTER\n");
				ME->SetupInitialState();	// Sets the initial state
			}
			END_EVENT(true)
//			ON_EXIT { ntPrintf("State: DEFAULT  Event: ON_EXIT\n"); } END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(WAIT_FOR_VIEW)
		BEGIN_EVENTS
//			ON_ENTER { ntPrintf("State: WAIT_FOR_VIEW  Event: ON_ENTER\n"); } END_EVENT(true)
			ON_UPDATE
			{
				OBJECT_PURGATORYMANAGER_FSM& obParent = static_cast<OBJECT_PURGATORYMANAGER_FSM&>(FSM);
				if (obParent.ViewReady())
				{
					ntError(obParent.m_pViewWaitNextState);
					FSM.SetState(obParent.m_pViewWaitNextState);
					obParent.m_pViewWaitNextState = NULL;
				}
			}
			END_EVENT(true)
//			ON_EXIT { ntPrintf("State: WAIT_FOR_VIEW  Event: ON_EXIT\n"); } END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(LOCATION_SWITCH)
		BEGIN_EVENTS
			ON_ENTER
			{
//				ntPrintf("State: LOCATION_SWITCH  Event: ON_ENTER\n");
				ntError_p(CamMan::Get().GetPrimaryView(), ("View must be ready before entering this state"));
				ME->SwitchToCamera(ME->TargetLocation());
			}
			END_EVENT(true)
			ON_UPDATE
			{
				bool bInChapSelect = (ME->TargetLocation() == Object_PurgatoryManager::CHAPTERSELECT_CAMERA);
				if (ME->UpdateTransitions())
				{
					if (bInChapSelect && ME->DelayedSwordAnim())
					{
						OBJECT_PURGATORYMANAGER_FSM& obParent = static_cast<OBJECT_PURGATORYMANAGER_FSM&>(FSM);
						ME->TriggerDelayedSwordAnim(obParent.m_iChapterUnlocked);

						SET_STATE(WAIT_FOR_SWORD);
					}
					else
					{
					SET_STATE(IDLE);
				}
			}
			}
			END_EVENT(true)
//			ON_EXIT { ntPrintf("State: LOCATION_SWITCH  Event: ON_EXIT\n"); } END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(IDLE)
		BEGIN_EVENTS
//			ON_ENTER { ntPrintf("State: IDLE  Event: ON_ENTER\n"); } END_EVENT(true)
			ON_UPDATE
			{
				//wait for a view switch request
				if (ME->TargetLocation() != Object_PurgatoryManager::INVALID_CAMERA)
					ME->ChangeCameraStyle();
			}
			END_EVENT(true)
//			ON_EXIT { ntPrintf("State: IDLE  Event: ON_EXIT\n"); } END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(TRIGGER_CUTSCENE)
		BEGIN_EVENTS
			ON_ENTER
			{
//				ntPrintf("State: TRIGGER_CUTSCENE  Event: ON_ENTER\n");
				OBJECT_PURGATORYMANAGER_FSM& obParent = static_cast<OBJECT_PURGATORYMANAGER_FSM&>(FSM);

				ntError_p(CamMan::Get().GetPrimaryView(), ("View must be ready before entering this state"));
				ntError_p(obParent.m_iChapterCompleted != -1, ("Cutscene to trigger has not been set"));

				ME->TriggerCutscene(obParent.m_iChapterCompleted);
			}
			END_EVENT(true)
			EVENT(Trigger)
			{
				OBJECT_PURGATORYMANAGER_FSM& obParent = static_cast<OBJECT_PURGATORYMANAGER_FSM&>(FSM);
				if ( ME->TriggerSwordAnim(obParent.m_iChapterUnlocked) )
				{
					SET_STATE(WAIT_FOR_SWORD);
				}
				else
				{
					SET_STATE(CUTSCENE_COMPLETE);
				}
			}
			END_EVENT(true)
//			ON_EXIT { ntPrintf("State: TRIGGER_CUTSCENE  Event: ON_EXIT\n"); } END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(WAIT_FOR_SWORD)
		BEGIN_EVENTS
//			ON_ENTER { ntPrintf("State: WAIT_FOR_SWORD  Event: ON_ENTER\n"); } END_EVENT(true)
			ON_UPDATE
			{
				if ( ME->SwordAnimComplete() )
				{
					SET_STATE(CUTSCENE_COMPLETE);
				}
			}
			END_EVENT(true)
//			ON_EXIT { ntPrintf("State: WAIT_FOR_SWORD  Event: ON_EXIT\n"); } END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(CUTSCENE_COMPLETE)
		BEGIN_EVENTS
			ON_ENTER
			{
//				ntPrintf("State: CUTSCENE_COMPLETE  Event: ON_ENTER\n");
				ME->OnCutsceneComplete();
				SET_STATE(IDLE);
			}
			END_EVENT(true)
//			ON_EXIT { ntPrintf("State: CUTSCENE_COMPLETE  Event: ON_EXIT\n"); } END_EVENT(true)
		END_EVENTS
	END_STATE

END_STATEMACHINE

//--------------------------------------------------
//!
//!	SwordCheckpointInfoDef::SwordCheckpointInfoDef
//!	Default constructor
//!
//--------------------------------------------------
SwordCheckpointInfoDef::SwordCheckpointInfoDef(const CheckpointInfoDef* pobDef, CMeshInstance* pobSelected, CMeshInstance* pobAvailable, CMeshInstance* pobLocked)
:
	m_iCheckpointNumber(pobDef->CheckpointNumber())
{
	m_apobMeshes[SELECTED] = pobSelected;
	m_apobMeshes[AVAILABLE] = pobAvailable;
	m_apobMeshes[LOCKED] = pobLocked;
}

//--------------------------------------------------
//!
//!	SwordCheckpointInfoDef::~SwordCheckpointInfoDef
//!	Default dtor
//!
//--------------------------------------------------
SwordCheckpointInfoDef::~SwordCheckpointInfoDef()
{
}

//--------------------------------------------------
//!
//!	SwordCheckpointInfoDef::SetMode
//!	
//!
//--------------------------------------------------
void SwordCheckpointInfoDef::SetMode(CHECKPOINT_MODE eMode)
{
	m_eMode = eMode;
	switch (m_eMode)
	{
	case SELECTED:
		ShowMesh(m_apobMeshes[SELECTED]);
		HideMesh(m_apobMeshes[AVAILABLE]);
		HideMesh(m_apobMeshes[LOCKED]);
		break;
	case AVAILABLE:
		HideMesh(m_apobMeshes[SELECTED]);
		ShowMesh(m_apobMeshes[AVAILABLE]);
		HideMesh(m_apobMeshes[LOCKED]);
		break;
	case LOCKED:
		HideMesh(m_apobMeshes[SELECTED]);
		HideMesh(m_apobMeshes[AVAILABLE]);
		ShowMesh(m_apobMeshes[LOCKED]);
		break;
	default:
		ntError(false);
	};
}

//--------------------------------------------------
//!
//!	SwordInfoDef::SwordInfoDef()
//!	Default constructor
//!
//--------------------------------------------------
void SwordCheckpointInfoDef::ShowMesh(CMeshInstance* pobMesh)
{
	if (pobMesh && !pobMesh->IsRendering())
		pobMesh->DisableRendering(false);
}

//--------------------------------------------------
//!
//!	SwordInfoDef::SwordInfoDef()
//!	Default constructor
//!
//--------------------------------------------------
void SwordCheckpointInfoDef::HideMesh(CMeshInstance* pobMesh)
{
	if (pobMesh && pobMesh->IsRendering())
		pobMesh->DisableRendering(true);
}

//--------------------------------------------------
//!
//!	SwordInfoDef::SwordInfoDef()
//!	Default constructor
//!
//--------------------------------------------------
SwordInfoDef::SwordInfoDef()
{
	m_eMode = LOCKED;
}

//--------------------------------------------------
//!
//!	SwordInfoDef::~SwordInfoDef()
//!	Default constructor
//!
//--------------------------------------------------
SwordInfoDef::~SwordInfoDef()
{
	for (CheckpointList::iterator obIt = m_obCheckpoints.begin(); obIt != m_obCheckpoints.end(); ++obIt)
	{
		NT_DELETE( *obIt );
	}
	m_obCheckpoints.clear();
}

//--------------------------------------------------
//!
//!	SwordInfoDef::SetMode()
//!	
//!
//--------------------------------------------------
void SwordInfoDef::SetMode(SWORD_MODE eMode, bool bUpdateVisibility)
{
	UNUSED(bUpdateVisibility);
	m_eMode = eMode;
	switch (m_eMode)
	{
	case LOCKED:
		if (bUpdateVisibility)
			m_pobSword->Hide();
		//fall through.
	case UNLOCKED:
		break;
	case AVAILABLE:
		{
			Message obMsg = CMessageHandler::Make( m_pobSword, "msg_animated_settoend" );
			obMsg.SetValue( "Locomoting", NinjaLua::LuaObject( CLuaGlobal::Get().State(), true) );
			m_pobSword->GetMessageHandler()->Receive( obMsg );
		}
		break;
	}
}

//--------------------------------------------------
//!
//!	SwordInfoDef::SetupCheckpointInfo(...)
//!	
//!
//--------------------------------------------------
SwordCheckpointInfoDef* SwordInfoDef::SetupCheckpointInfo(const CheckpointInfoDef* pobCP)
{
	char acBuff[128];
	char cSwordChar = SwordChar();
	int iCheckpointIndexOffset = CheckpointIndexOffset();
	int iCP = pobCP->CheckpointNumber();
	CRenderableComponent* pobRC = Sword()->GetRenderableComponent();
	ntError(pobRC);

	snprintf(acBuff, 128, "%c_light%.02d_selectedShape", cSwordChar, iCheckpointIndexOffset + iCP);
	CMeshInstance* pobSelected = const_cast<CMeshInstance*>(pobRC->GetMeshByMeshName(acBuff));
	snprintf(acBuff, 128, "%c_light%.02d_unlockedShape", cSwordChar, iCheckpointIndexOffset + iCP);
	CMeshInstance* pobAvailable = const_cast<CMeshInstance*>(pobRC->GetMeshByMeshName(acBuff));
	snprintf(acBuff, 128, "%c_light%.02d_lockedShape", cSwordChar, iCheckpointIndexOffset + iCP);
	CMeshInstance* pobLocked = const_cast<CMeshInstance*>(pobRC->GetMeshByMeshName(acBuff));

	return NT_NEW SwordCheckpointInfoDef(pobCP, pobSelected, pobAvailable, pobLocked );
}

//--------------------------------------------------
//!
//!	SwordInfoDef::SetupCheckpoints()
//!	
//!
//--------------------------------------------------
void SwordInfoDef::SetupCheckpoints(const ChapterInfoDef* pobChap)
{
	const ChapterInfoDef::CheckpointList& obCheckpoints = pobChap->Checkpoints();
	for (ChapterInfoDef::CheckpointList::const_iterator obCPIt = obCheckpoints.begin(); obCPIt != obCheckpoints.end(); ++obCPIt)
	{
		const CheckpointInfoDef* pobCPinfo = *obCPIt;

		//we dont want to add this checkpoint if its a 'final'
		if (pobCPinfo->Final() || pobCPinfo->EndGame())
			continue;

		SwordCheckpointInfoDef* pCP = SetupCheckpointInfo(pobCPinfo);

		//if we are freshly unlocked, we want to force the first checkpoint into an on state
		if (Mode() == UNLOCKED)
		{
			//if its the first one, then force it to available
			if (obCPIt == obCheckpoints.begin())
				pCP->SetMode(SwordCheckpointInfoDef::AVAILABLE);
			else
				pCP->SetMode(SwordCheckpointInfoDef::LOCKED);
		}
		else
		{
			int iChap = pobChap->ChapterNumber();
			int iCP = pobCPinfo->CheckpointNumber();

			//otherwise enable the checkpoint if we have data for it. 
			CheckpointData* pobData = CheckpointManager::Get().GetDataForCheckpoint(iChap, iCP);
			if (pobData)
			{
				pCP->SetMode(SwordCheckpointInfoDef::AVAILABLE);
			}
			else
			{
				pCP->SetMode(SwordCheckpointInfoDef::LOCKED);
			}
		}

		m_obCheckpoints.push_back(pCP);
	}

	//invalidate
	m_obCurrentCheckpoint = m_obCheckpoints.end();
}

//--------------------------------------------------
//!
//!	SwordInfoDef::SelectCheckpoint
//!	
//!
//--------------------------------------------------
bool SwordInfoDef::SelectCheckpoint(SELECT_CHECKPOINT eCheckpoint)
{
	if (m_obCurrentCheckpoint != m_obCheckpoints.end())
		(*m_obCurrentCheckpoint)->SetMode(SwordCheckpointInfoDef::AVAILABLE);

	bool bResult = true;

	CheckpointList::iterator obNext;
	switch (eCheckpoint)
	{
	case NEXT_CHECKPOINT:
		obNext = m_obCurrentCheckpoint;
		++obNext;
		if (obNext != m_obCheckpoints.end() && (*obNext)->Mode() != SwordCheckpointInfoDef::LOCKED)
		{
			m_obCurrentCheckpoint = obNext;
		}
		else
		{
			bResult = false;
		}
		break;
	case PREVIOUS_CHECKPOINT:
		if (m_obCurrentCheckpoint != m_obCheckpoints.begin())
		{
			obNext = m_obCurrentCheckpoint;
			--obNext;
			if ((*obNext)->Mode() != SwordCheckpointInfoDef::LOCKED)
			{
				m_obCurrentCheckpoint = obNext;
			}
			else
			{
				bResult = false;
			}
		}
		else
		{
			bResult = false;
		}
		break;
	case NEWEST_CHECKPOINT:
		for (obNext = m_obCheckpoints.begin(); obNext != m_obCheckpoints.end(); ++obNext)
		{
			if ((*obNext)->Mode() == SwordCheckpointInfoDef::LOCKED)
				break;
			m_obCurrentCheckpoint = obNext;
		}
		break;
	case FIRST_CHECKPOINT:
		m_obCurrentCheckpoint = m_obCheckpoints.begin();
		break;
	};

	(*m_obCurrentCheckpoint)->SetMode(SwordCheckpointInfoDef::SELECTED);
	return bResult;
}

//--------------------------------------------------
//!
//!	SwordInfoDef::SelectedCheckpoint()
//!	
//!
//--------------------------------------------------
int SwordInfoDef::SelectedCheckpoint()
{
	ntError(m_obCurrentCheckpoint != m_obCheckpoints.end());
	return (*m_obCurrentCheckpoint)->CheckpointNumber();
}

//--------------------------------------------------
//!
//!	Object_PurgatoryManager::Object_PurgatoryManager()
//!	Default constructor
//!
//--------------------------------------------------
Object_PurgatoryManager::Object_PurgatoryManager()
{
	m_eType = EntType_Object;

	m_pobGameInfo = NULL;
	m_pobCurrentSword = NULL;
	m_pobCutsceneSword = NULL;

	m_pobTargetScreen = NULL;

	m_bTransitionActive = false;

	m_eTargetLocation = INVALID_CAMERA;

	m_bDelayedSwordAnim = false;
	m_bSetupMenuRequired = false;
	m_iChapterSelectTransitionGracePeriod = 15;
	
	ATTACH_LUA_INTERFACE(Object_PurgatoryManager);
}

//--------------------------------------------------
//!
//!	Object_PurgatoryManager::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_PurgatoryManager::OnPostConstruct()
{
	CEntity::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();

	// Create and attach the statemachine
	OBJECT_PURGATORYMANAGER_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) OBJECT_PURGATORYMANAGER_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_PurgatoryManager::~Object_PurgatoryManager()
//!	Default destructor
//!
//--------------------------------------------------
Object_PurgatoryManager::~Object_PurgatoryManager()
{
	DETACH_FSM();
}

//--------------------------------------------------
//!
//!	DumpCheckpointInfo()
//!	Dump diagnostic info for save game data state
//!
//--------------------------------------------------
void DumpCheckpointInfo()
{
#ifndef _GOLD_MASTER

	GameInfoDef* pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
	const ChapterInfoDef* pobChapter = pobGameInfo->InitialBattleChapter();

	ntPrintf("Initial Battle: %p\n", pobChapter);

	const GameInfoDef::ChapterList& obCList = pobGameInfo->Chapters();
	for (GameInfoDef::ChapterList::const_iterator obIt = obCList.begin(); obIt != obCList.end(); ++obIt)
	{
		pobChapter = *obIt;
		ntPrintf("Chapter %d\n", pobChapter->ChapterNumber());

		const ChapterInfoDef::CheckpointList& obCPList = pobChapter->Checkpoints();
		for (ChapterInfoDef::CheckpointList::const_iterator obCPIt = obCPList.begin(); obCPIt != obCPList.end(); ++obCPIt)
		{
			const CheckpointInfoDef* pobCP = *obCPIt;
			ntPrintf("\tCheckpoint %d", pobCP->CheckpointNumber());

			if (pobCP->EndGame())
				ntPrintf(" [EndGame]");

			if (pobCP->Final())
				ntPrintf(" [Final]");

			ntPrintf(": ");

			CheckpointData* pobData = CheckpointManager::Get().GetDataForCheckpoint(pobChapter->ChapterNumber(), pobCP->CheckpointNumber());
			if (pobData)
			{
				ntPrintf("Data exists [%p]", pobData);
				if (pobData->m_GenericData.IsNewlyHitCheckpoint())
					ntPrintf(" [New]");
			}
			else
				ntPrintf("No data");
			ntPrintf("\n");
		}
	}

	int c = -1, cp = -1;
	CheckpointManager::Get().GetLastCheckpointNumberReachedByPlayer(c, cp);
	ntPrintf("Last Checkpoint Number Reached By Player: Chapter %d, Checkpoint %d\n", c, cp);

#endif
}

//--------------------------------------------------
//!
//!	Object_PurgatoryManager::SetupInitialState()
//!	
//!
//--------------------------------------------------

void Object_PurgatoryManager::SetupInitialState()
{
	ntPrintf("Object_PurgatoryManager::SetupInitialState\n");

	ntError_p(m_pobMenuCamera, ("PurgatoryManager needs MenuCamera to be set") );
	ntError_p(m_pobOptionsCamera, ("PurgatoryManager needs OptionsCamera to be set") );
	ntError_p(m_pobSpecialFeaturesCamera, ("PurgatoryManager needs SpecialFeaturesCamera to be set") );

	m_pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
	ntError_p(m_pobGameInfo, ("GameInfo object not found") );
	
//	ntError_p(m_obSwords.size() == (size_t)m_pobGameInfo->NumChapters(), ("PurgatoryManager needs %d Swords set", m_pobGameInfo->NumChapters()) );
	DumpCheckpointInfo();

	int iLastChapterComplete = 0;
	int iLastCheckpointComplete = 0;

	const ChapterInfoDef* pobUnlockedChapter = NULL;
	const ChapterInfoDef* pobCompletedChapter = NULL;

	bool bHaveCheckpointData = CheckpointManager::Get().GetLastCheckpointNumberReachedByPlayer(iLastChapterComplete, iLastCheckpointComplete);
	if (bHaveCheckpointData)
	{
		//Figure out if we have unlocked a chapter since last coming in here.
		//grab the chapter
		pobCompletedChapter = m_pobGameInfo->GetChapter(iLastChapterComplete);
		ntError_p(pobCompletedChapter, ("Failed to find chapter %d in GameInfo", iLastChapterComplete) );

		//and the checkpoint
		const CheckpointInfoDef* pobCPInfo = pobCompletedChapter->GetCheckpoint(iLastCheckpointComplete);
		ntError_p(pobCPInfo, ("Failed to find checkpoint %d for chapter %d in GameInfo", iLastCheckpointComplete, iLastChapterComplete) );

		if (pobCPInfo->Final() && !pobCPInfo->EndGame())	// looking good sofar, as the cp we are upto is a 'final' checkpoint, though not the endgame checkpoint
		{
			CheckpointData* pobData = CheckpointManager::Get().GetDataForCheckpoint(iLastChapterComplete, iLastCheckpointComplete);
			ntError_p(pobData, ("Missing checkpoint data for chapter %d checkpoint %d", iLastChapterComplete, iLastCheckpointComplete));

			if (pobData->m_GenericData.IsNewlyHitCheckpoint())
			{
				//at this point we know that we should unlock the next chapter
				const CheckpointInfoDef* pobCPNext = pobCPInfo->NextCheckpoint();
				ntError_p(pobCPNext, ("Checkpoint %d for chapter %d does not have a next checkpoint. Check GameInfo.xml", pobCPInfo->CheckpointNumber(), pobCompletedChapter->ChapterNumber()) );

				ntError_p(pobCPNext != pobCPInfo, ("Checkpoint\'s within the same chapter"));

				pobUnlockedChapter = m_pobGameInfo->FindChapterContainingCheckpoint(pobCPNext);
			}
		}
	}


	//Now, setup the swords
	for (SwordList::iterator obIt = m_obSwords.begin(); obIt != m_obSwords.end(); ++obIt)
	{
		SwordInfoDef* pobSwordInfo = *obIt;
		const ChapterInfoDef* pobChap = m_pobGameInfo->GetChapter(pobSwordInfo->ChapterNumber());

		user_warn_p(pobChap!=NULL, ("Chapter information not available for Chapter %d. Please check GameInfo.xml\n", pobSwordInfo->ChapterNumber()));
		
		if (pobChap)
		{
			//now, if this is the newly unlocked chapter, process it accordingly
			if (pobChap == pobUnlockedChapter)
			{
				pobSwordInfo->SetMode(SwordInfoDef::UNLOCKED);
				pobSwordInfo->SetupCheckpoints(pobChap);
				pobSwordInfo->SelectCheckpoint(SwordInfoDef::FIRST_CHECKPOINT);
			}
			else
			{
				//check if the checkpoint manager has a datapoint for atleast our first checkpoint
				ntError_p(pobChap->Checkpoints().begin() != pobChap->Checkpoints().end(), ("Chapter %d has no checkpoints defined. see GameInfo.xml", pobChap->NumCheckpoints()));

				//Note too that I'm assuming that the first defined is the first checkpoint in the chapter
				CheckpointInfoDef* pobCP = *(pobChap->Checkpoints().begin());
				ntError(pobCP);	// just to be sure :)

				
				bool bChapterIsOpen = false;
				
				//Do we have a checkpoint already stored for the current chapter?
				if (CheckpointManager::Get().GetDataForCheckpoint(pobChap->ChapterNumber(), pobCP->CheckpointNumber()))
				{	
					//...if so, this chapter should be available for selection.		
					bChapterIsOpen= true;
				}
				else
				{
					//We have not yet hit a checkpoint for this chapter - however did we complete the chapter before?
					//If so, this chapter should be available.
					int iPrevChapterNum = pobSwordInfo->ChapterNumber()-1;
					
					//Get the chapter previous to this....
					const ChapterInfoDef* pobPreviousChapter = m_pobGameInfo->GetChapter(iPrevChapterNum);
					ntError_p(pobPreviousChapter,("no chapter info for %s\n", iPrevChapterNum));
					



					//If the last checkpoint in that chapter had been hit, then this current chapter must now be available
					if (CheckpointManager::Get().GetDataForCheckpoint(iPrevChapterNum,pobPreviousChapter->NumCheckpoints() - 1))
						bChapterIsOpen= true;
					/*
					//Get the last checkpoint reached by the player within that chapter
					if (CheckpointManager::Get().GetLastCheckpointNumberReachedByPlayer(iLastChapterWithCompletedCheckpoint, iLastCheckpointReachedInPreviousChapter))
					{				
						//If it's the same as the last checkpoint listed for the chapter then we can assume the player's previously completed that chapter
						//IE: The current chapter should be available for selection.
						if ((iLastChapterWithCompletedCheckpoint == iPrevChapterNum) && (iLastCheckpointReachedInPreviousChapter == (pobPreviousChapter->NumCheckpoints() - 1)))
							bChapterIsOpen= true;
					}
					*/
				}
				
					
				//If the chapter has been unlocked, ensure the sword is available for display.
				if (bChapterIsOpen)
				{

					pobSwordInfo->SetMode(SwordInfoDef::AVAILABLE);
					pobSwordInfo->SetupCheckpoints(pobChap);
					pobSwordInfo->SelectCheckpoint(SwordInfoDef::FIRST_CHECKPOINT);
				}
				else
				{
					//..otherwise, it's locked out for now
					pobSwordInfo->SetMode(SwordInfoDef::LOCKED);
				}
			}
		}
		else
		{
			pobSwordInfo->SetMode(SwordInfoDef::LOCKED);
		}
		
		if (pobSwordInfo->Mode() != SwordInfoDef::LOCKED)
		{
			m_pobCurrentSword = pobSwordInfo;
			
		}
	}

	//do we need to delay the sword anim?
	if (pobUnlockedChapter && pobCompletedChapter)
	{
		const ChapterInfoDef* pobInitialChapterInfo = m_pobGameInfo->InitialBattleChapter();
		ntError_p(pobInitialChapterInfo, ("Failed to find InitialBattle chapter in GameInfo") );

		if (pobInitialChapterInfo->ChapterNumber() == pobCompletedChapter->ChapterNumber())
		{
			m_bDelayedSwordAnim = true;
			//in this case we also want to have a different menu
			m_bSetupMenuRequired = true;
		}
	}

	//Move to the required gui screen
	MoveToInitialScreen();

	//Do we need to trigger a cutscene?
 	if (pobUnlockedChapter)
	{
		OBJECT_PURGATORYMANAGER_FSM* pFSM = static_cast<OBJECT_PURGATORYMANAGER_FSM*>(m_pFSM);

		pFSM->m_pViewWaitNextState = OBJECT_PURGATORYMANAGER_FSM::TRIGGER_CUTSCENE::GetInstance();
		pFSM->m_iChapterUnlocked = pobUnlockedChapter->ChapterNumber();
		pFSM->m_iChapterCompleted = pobCompletedChapter->ChapterNumber();

		EXTERNALLY_SET_STATE(OBJECT_PURGATORYMANAGER_FSM, WAIT_FOR_VIEW);

		BlockGuiInput(true);
	}
	else
	{
		//otherwise we simply want to go to the IDLE state
		EXTERNALLY_SET_STATE(OBJECT_PURGATORYMANAGER_FSM, IDLE);
	}

	CheckpointManager::Get().ClearAllCheckpointHitFlags();
}

//--------------------------------------------------
//!
//!	Object_PurgatoryManager::StringToCameraStyle
//!	
//!
//--------------------------------------------------
Object_PurgatoryManager::CAMERA Object_PurgatoryManager::StringToCameraStyle(CHashedString obStyle)
{
	if (obStyle == CHashedString(HASH_STRING_PURG_MAINMENU_CAMERA))
	{
		return MAINMENU_CAMERA;
	}
	else if (obStyle == CHashedString(HASH_STRING_PURG_CHAPTERSELECT_CAMERA))
	{
		return CHAPTERSELECT_CAMERA;
	}
	else if (obStyle == CHashedString(HASH_STRING_PURG_SPECIALFEATURES_CAMERA))
	{
		return SPECIALFEATURES_CAMERA;
	}
	else if (obStyle == CHashedString(HASH_STRING_PURG_OPTIONS_CAMERA))
	{
		return OPTIONS_CAMERA;
	}

	ntError_p(false, ("StringToCameraStyle: unknown camera style hash"));
	return MAINMENU_CAMERA;
}

//--------------------------------------------------
//!
//!	Object_PurgatoryManager::ChangeCameraStyle
//!	
//!
//--------------------------------------------------
void Object_PurgatoryManager::ChangeCameraStyle()
{
	BlockGuiInput(true);

	OBJECT_PURGATORYMANAGER_FSM* pFSM = static_cast<OBJECT_PURGATORYMANAGER_FSM*>(m_pFSM);
	if (pFSM->ViewReady())
	{
		EXTERNALLY_SET_STATE(OBJECT_PURGATORYMANAGER_FSM, LOCATION_SWITCH);
	}
	else
	{
		pFSM->m_pViewWaitNextState = OBJECT_PURGATORYMANAGER_FSM::LOCATION_SWITCH::GetInstance();
		EXTERNALLY_SET_STATE(OBJECT_PURGATORYMANAGER_FSM, WAIT_FOR_VIEW);
	}
}

//--------------------------------------------------
//!
//!	Object_PurgatoryManager::RequestCameraStyleChange
//!	
//!
//--------------------------------------------------
void Object_PurgatoryManager::RequestCameraStyleChange(CAMERA eCamera)
{
	m_eTargetLocation = eCamera;
}

//--------------------------------------------------
//!
//!	Object_PurgatoryManager::SwitchToCamera
//!	
//!
//--------------------------------------------------
void Object_PurgatoryManager::SwitchToCamera(CAMERA eCamera)
{
	// if we have a target screen, we want it to fade out
//	if (m_pobTargetScreen)
//		m_pobTargetScreen->BeginFadeOut(false);

	switch (eCamera)
	{
	case MAINMENU_CAMERA:
		m_pobMenuCamera->Activate();
		break;
	case OPTIONS_CAMERA:
		CGuiSoundManager::Get().PlaySound( CGuiSound::ACTION_CAMERA_PAN_OPTIONS );
		m_pobOptionsCamera->Activate();
		break;
	case SPECIALFEATURES_CAMERA:
		CGuiSoundManager::Get().PlaySound( CGuiSound::ACTION_CAMERA_PAN_SPECIAL_FEATURES );
		m_pobSpecialFeaturesCamera->Activate();
		break;
	case CHAPTERSELECT_CAMERA:
		ntError(m_pobCurrentSword);
		CGuiSoundManager::Get().PlaySound( CGuiSound::ACTION_CAMERA_PAN_CHAPTER );
		m_pobCurrentSword->Camera()->Activate();
		break;
	default:
		ntError(false);
	}

	m_bTransitionActive = true;
}

//--------------------------------------------------
//!
//!	Object_PurgatoryManager::SwitchChapter
//!	
//!
//--------------------------------------------------
bool Object_PurgatoryManager::SwitchChapter(SWITCH_CHAPTER eChapter)
{
	ntError(m_pobCurrentSword);
	SwordInfoDef* pobSwitch = NULL;
	switch (eChapter)
	{
	case NEXT_CHAPTER:
		pobSwitch = m_pobCurrentSword->NextSword();
		break;
	case PREVIOUS_CHAPTER:
		pobSwitch = m_pobCurrentSword->PreviousSword();
		break;
	}

	if (pobSwitch->Mode() != SwordInfoDef::LOCKED)
	{
		m_pobCurrentSword = pobSwitch;
		RequestCameraStyleChange(CHAPTERSELECT_CAMERA);

		//get the chapter select widget and tell it the chapter is changing
		ntError_p(m_pobTargetScreen, ("GUI: target screen not available when starting chapter switch :("));

		CGuiUnit* pobChapterSelectUnit = m_pobTargetScreen->FindChildUnit("chapterselect");
		ntError_p(pobChapterSelectUnit, ("Failed to find \"chapterselect\" field on target screen\n"));

		CGuiSkinCheckpointSelect* pobChapSel = static_cast<CGuiSkinCheckpointSelect*>(pobChapterSelectUnit);
		//notify
		pobChapSel->OnChapterSwitchBegin();

		return true;
	}

	return false;
}

//--------------------------------------------------
//!
//!	Object_PurgatoryManager::SwitchCheckpoint
//!	
//!
//--------------------------------------------------
bool Object_PurgatoryManager::SwitchCheckpoint(SWITCH_CHECKPOINT eCheckpoint)
{
	ntError(m_pobCurrentSword);

	bool bResult = false;
	switch (eCheckpoint)
	{
	case NEXT_CHAPTER:
		bResult = m_pobCurrentSword->SelectCheckpoint(SwordInfoDef::NEXT_CHECKPOINT);
		break;
	case PREVIOUS_CHAPTER:
		bResult = m_pobCurrentSword->SelectCheckpoint(SwordInfoDef::PREVIOUS_CHECKPOINT);
		break;
	}

	return bResult;
}

void Object_PurgatoryManager::NotifyScreenEnter(CGuiScreen* pobScreen, CHashedString obStyle)
{

	m_pobTargetScreen = pobScreen;
	m_eTargetLocation = StringToCameraStyle(obStyle);

}

/*void Object_PurgatoryManager::NotifyScreenExit(CGuiScreen* pobScreen)
{
	//Screen is closing...

	UNUSED(pobScreen);
}*/



bool Object_PurgatoryManager::UpdateTransitions()
{
	if (!m_bTransitionActive)
		return false;

	const CamTransition* pobTrans = CamMan::Get().GetPrimaryView()->ActiveTransition();

	//we really should have a screen pointer here...
	ntError(m_pobTargetScreen);

	//active if it exists
	if (pobTrans)
	{
		
		float fTransTotalTime = pobTrans->GetControlTransTotalTime();
		float fTransTotal = pobTrans->GetControlTransTotal();

		float fFadeLen = m_pobTargetScreen ? m_pobTargetScreen->ScreenFadeLength() : 0.025f;

		if (fTransTotal + fFadeLen < fTransTotalTime)
			return false;
	}

	if (m_pobTargetScreen)
	{
		
		m_pobTargetScreen->ContinueFade();

		//we dont want to clear our target if we're in chapter select mode
		if (m_eTargetLocation != CHAPTERSELECT_CAMERA)
		{
			m_pobTargetScreen = NULL;
		}
		else
		{	//OK, we are in chapter select mode, so double check that the transition to new chapter camera has completed
			//If not, return 'transition not complete' to the client
			//To address: HS-5254
			if (CamMan::Get().GetPrimaryView()->ActiveTransition())
			{
				return (false);
			}
			else
			{
				//..we've transitioned to the new chapter and can signify it accordingly
				// if we have a 'chapterselect' widget then we are in the chapter select screen.
				// so, get the widget and tell it the chapter is done changing
				CGuiUnit* pobChapterSelectUnit = m_pobTargetScreen->FindChildUnit("chapterselect");
				if (pobChapterSelectUnit)
				{
					
					CGuiSkinCheckpointSelect* pobChapSel = static_cast<CGuiSkinCheckpointSelect*>(pobChapterSelectUnit);
					//notify
					pobChapSel->OnChapterSwitchEnd();

					

					//Horrible hack..the edge-case (again to address: HS-5254 issues)
					//Appears to be a 'grace' period needed to allow the currently selected chapter to fade down
					//before deeming the transition as fully complete.
					//To be reviewed - and cleaned - by Mike Blaha
					if (m_iChapterSelectTransitionGracePeriod > 0)
					{
						m_iChapterSelectTransitionGracePeriod --;
						return (false);
					}
				}
			
				m_iChapterSelectTransitionGracePeriod = 15;
			}
		}
	}
	
	m_bTransitionActive = false;
	BlockGuiInput(false);
	m_eTargetLocation = INVALID_CAMERA;

	//finally unpause (just in case)
//	ShellMain::Get().RequestPauseToggle( ShellMain::PM_USER, false );

	return true;
}

void Object_PurgatoryManager::BlockGuiInput(bool bBlock)
{
	CGuiInput::Get().BlockInput(bBlock);
}


void Object_PurgatoryManager::TriggerCutscene(int iChapterCompleted)
{
	const ChapterInfoDef* pobChap = m_pobGameInfo->GetChapter(iChapterCompleted);
	ntError_p(pobChap, ("Failed to locate chapter info for chapter %d", iChapterCompleted) );

	const char* pcTrig = ntStr::GetString(pobChap->CompletionCutsceneTrigger());

	NinjaLua::LuaObject obFunc = CLuaGlobal::Get().State().GetGlobals()[pcTrig];
	CLuaGlobal::Get().SetMessage( 0 );

	ntError_p(!obFunc.IsNil(), ("Failed to locate chapter %d cutscene trigger %s", iChapterCompleted, pcTrig) );

	NinjaLua::LuaFunction obTrig(obFunc);
	obTrig();
}

void Object_PurgatoryManager::OnCutsceneComplete()
{
	BlockGuiInput(false);
	RequestCameraStyleChange(m_eTargetLocation);

	//position character
	CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
	if (pobPlayer)
	{
		pobPlayer->SetPosition(CPoint(0,0,0));
	}
}

SwordInfoDef* Object_PurgatoryManager::LocateSword(int iChapter)
{
	for (SwordList::iterator obIt = m_obSwords.begin(); obIt != m_obSwords.end(); ++obIt)
	{
		SwordInfoDef* pobSwordInfo = *obIt;
		if (pobSwordInfo->ChapterNumber() == iChapter)
		{
			return pobSwordInfo;
		}
	}
	return NULL;
}

void Object_PurgatoryManager::MoveToInitialScreen()
{
	//Here we want to go to either the purgatory menu or switch to chapterselect.
	//We need to go to the purg menu when
	// 1. First time completion of the initial chapter
	// 2. First time game complete
	// 3. Regular entry into purgatory (see below)
	//We want to go directly to chapter select when
	// 1. Player has completed a chapter for the first time (except initial chapter)

	enum { INIT_MAINMENU, INIT_CHAPTERSELECT } eTarget = INIT_MAINMENU;

	int iLastChapterComplete = 0;
	int iLastCheckpointComplete = 0;
	bool bHaveCheckpointData = CheckpointManager::Get().GetLastCheckpointNumberReachedByPlayer(iLastChapterComplete, iLastCheckpointComplete);

	if (bHaveCheckpointData)
	{
	CheckpointData* pobData = CheckpointManager::Get().GetDataForCheckpoint(iLastChapterComplete, iLastCheckpointComplete);
		ntError_p(pobData, ("Failed to find checkpoint data for %d %d", iLastChapterComplete, iLastCheckpointComplete) );

	//Is this checkpoint freshly hit?
	if ( pobData->m_GenericData.IsNewlyHitCheckpoint() )
	{
		//Yes. so check that it is a 'final' checkpoint and not endgame

		//locate the chapter
		const ChapterInfoDef* pobLastChapterInfo = m_pobGameInfo->GetChapter(iLastChapterComplete);
			ntError_p(pobLastChapterInfo, ("Failed to find chapter %d in GameInfo", iLastChapterComplete) );

		//locate the checkpoint
		const CheckpointInfoDef* pobLastCPInfo = pobLastChapterInfo->GetCheckpoint(iLastCheckpointComplete);
			ntError_p(pobLastCPInfo, ("Failed to find checkpoint %d for chapter %d in GameInfo", iLastCheckpointComplete, iLastChapterComplete) );

		if ( pobLastCPInfo->Final() && !pobLastCPInfo->EndGame() )
		{
			//yes. now check that it isnt the initial chapter
			
			//locate the initial chapter
			const ChapterInfoDef* pobInitialChapterInfo = m_pobGameInfo->InitialBattleChapter();
				ntError_p(pobInitialChapterInfo, ("Failed to find InitialBattle chapter in GameInfo") );

			//compare chapter numbers
			if ( pobLastChapterInfo->ChapterNumber() != pobInitialChapterInfo->ChapterNumber() )
			{
				//we have a winner! In this case we want to go to the chapter select screen
				eTarget = INIT_CHAPTERSELECT;
			}
		}
	}
	}

	switch (eTarget)
	{
	case INIT_MAINMENU:
		//nothing to do here =) just wait for the normal screen switch to occur
		break;
	case INIT_CHAPTERSELECT:
		{
			NinjaLua::LuaObject obStore = CGuiLua::GetGlobalStore();
			obStore.Set("purgatoryMoveOnScreen", true);	// The screen's script will find this and move on. this will inturn set out camera style
		}
		break;
	default:
		ntError_p(false, ("Unknown screen target"));
	};
}

bool Object_PurgatoryManager::TriggerSwordAnim(int iChapterUnlocked)
{
	//we need to check if the trigger needs to take place.
	//first test if we have delayed anim trigger
	if (m_bDelayedSwordAnim)
		return false;

	//next we can check sword mode to be unlocked
	m_pobCutsceneSword = LocateSword(iChapterUnlocked);
	ntError(m_pobCutsceneSword);

	if (m_pobCutsceneSword->Mode() != SwordInfoDef::UNLOCKED)
	{
		m_pobCutsceneSword = NULL;
		return false;
	}

	Message message(PlayAnim);
	m_pobCutsceneSword ->Sword()->GetMessageHandler()->QueueMessage(message);

	m_pobCutsceneSword->Camera()->Activate();

	return true;
}

void Object_PurgatoryManager::TriggerDelayedSwordAnim(int iChapterUnlocked)
{
	BlockGuiInput(true);

	m_bDelayedSwordAnim = false;
	if (!TriggerSwordAnim(iChapterUnlocked))
	{
		ntPrintf("Gui: Failed to play sword anim for chapter %d\n", iChapterUnlocked);
	}
}

bool Object_PurgatoryManager::SwordAnimComplete()
{
	CEntity* pobSword = m_pobCutsceneSword->Sword();
	if( pobSword->GetAnimator() && !pobSword->GetAnimator()->IsPlayingAnimation() )
	{
		m_pobCutsceneSword = NULL;
		return true;
	}
	return false;
}

void Object_PurgatoryManager::SetupPurgatoryMenu(CGuiScreen* pobScreen)
{
	if (!m_bSetupMenuRequired)
		return;

	CGuiUnit* pobMenu = pobScreen->FindChildUnit("menu");
	ntError_p(pobMenu, ("Gui: Failed to find menu component on purgatorymenu screen."));

	pobMenu->SetAttribute("removeitem", "gotolatest");

	CGuiUnit* pobBtn = pobMenu->FindChildUnit("chapterselect");
	ntError_p(pobBtn, ("Gui: Failed to find chapterselect button component in purgatorymenu screen menu."));

	pobBtn->SetAttribute( "bgimage", "gui/frontend/textures/common/fr_menuglow_colour_alpha_nomip.dds" );

	m_bSetupMenuRequired = false;
}
