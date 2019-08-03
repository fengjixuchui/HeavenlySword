/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskincheckpointselect.h"
#include "gui/guimanager.h"
#include "gui/guiscreen.h"
#include "gui/guilua.h"
#include "gui/guisound.h"

#include "game/entitymanager.h"
#include "game/entitypurgatorymanager.h"
#include "objectdatabase/dataobject.h"

#include "gui/purgatory/guiskincheckpointinfo.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinCheckpointSelect(); }

// Register this class under it's XML tag
bool g_bSKINCHECKPOINTSELECT = CGuiManager::Register( "CHECKPOINTSELECT", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinCheckpointSelect::CGuiSkinCheckpointSelect
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinCheckpointSelect::CGuiSkinCheckpointSelect( void )
:
	m_obOnSelectAction(CGuiAction::RUN_SCRIPT)
{
//	m_obPagingArrows.reserve(2);
	m_bFirstUpdate = true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinCheckpointSelect::~CGuiSkinCheckpointSelect
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinCheckpointSelect::~CGuiSkinCheckpointSelect( void )
{
}

bool CGuiSkinCheckpointSelect::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if (!m_obOnSelectAction.ProcessAttribute( pcTitle, pcValue ) )
		{
			if ( strcmp( pcTitle, "onselect" ) == 0 )
			{
				return m_obOnSelectAction.ProcessAttribute( "script", pcValue );
			}
		}

		return false;
	}

	return true;
}

bool CGuiSkinCheckpointSelect::ProcessChild( CXMLElement* pobChild )
{
	super::ProcessChild(pobChild);
	
	ntAssert( pobChild );

	// Add this as our controls
//	m_obPagingArrows.push_back( ( CGuiUnit* )pobChild );

	return true;
}

bool CGuiSkinCheckpointSelect::ProcessEnd( void )
{
	super::ProcessEnd();

	//make sure we have our two controls
//	ntAssert(m_obPagingArrows.size() == 2);


	return true;
}

bool CGuiSkinCheckpointSelect::Update( void )
{
	if (super::Update())
	{
		// Update all the children
	//	m_obPagingArrows[0]->Update();
	//	m_obPagingArrows[1]->Update();

		if (m_bFirstUpdate)
		{
			m_bFirstUpdate = false;
		}

		return true;
	}

	return false;
}

bool CGuiSkinCheckpointSelect::Render( void )
{
	if (super::Render())
	{
		// Render all the children
	//	m_obPagingArrows[0]->Render();
	//	m_obPagingArrows[1]->Render();

		return true;
	}

	return false;
}


bool CGuiSkinCheckpointSelect::StartAction( int iPads )
{
	return SelectAction(iPads);
}

bool CGuiSkinCheckpointSelect::SelectAction( int iPads )
{
	UNUSED(iPads);

	CEntity* pobEnt = CEntityManager::Get().FindEntity("PurgatoryManager");
	if (pobEnt)
	{
		Object_PurgatoryManager* pobPurg = (Object_PurgatoryManager*)pobEnt;

		//Save the loadlevel info into lua
		NinjaLua::LuaObject obStore = CGuiLua::GetGlobalStore();

		obStore.Set("selectedChapter", pobPurg->CurrentSelectedChapter());
		obStore.Set("selectedCheckpoint", pobPurg->CurrentSelectedCheckpoint());

		GetParentScreen()->AllowRender(false);
	}

	//m_obOnSelectAction.TriggerAction(GetParentScreen());

	return true;
}

bool CGuiSkinCheckpointSelect::MoveLeftAction( int iPads )
{
	super::MoveLeftAction(iPads);

	CEntity* pobEnt = CEntityManager::Get().FindEntity("PurgatoryManager");
	if (pobEnt)
	{
		Object_PurgatoryManager* pobPurg = (Object_PurgatoryManager*)pobEnt;
		bool bSwitched = pobPurg->SwitchChapter(Object_PurgatoryManager::PREVIOUS_CHAPTER);

		if (bSwitched)
		{
			CGuiSoundManager::Get().PlaySound( CGuiSound::ACTION_CHANGE_CHAPTER );
		}
	}

	return true;
}

bool CGuiSkinCheckpointSelect::MoveRightAction( int iPads )
{
	super::MoveRightAction(iPads);

	CEntity* pobEnt = CEntityManager::Get().FindEntity("PurgatoryManager");
	if (pobEnt)
	{
		Object_PurgatoryManager* pobPurg = (Object_PurgatoryManager*)pobEnt;
		bool bSwitched = pobPurg->SwitchChapter(Object_PurgatoryManager::NEXT_CHAPTER);

		if (bSwitched)
		{
			CGuiSoundManager::Get().PlaySound( CGuiSound::ACTION_CHANGE_CHAPTER) ;
		}
	}

	return true;
}

bool CGuiSkinCheckpointSelect::MoveDownAction( int iPads )
{
	bool bResult = super::MoveDownAction(iPads);

	if ( !m_obPadTimer.Passed() )
		return bResult;

	CEntity* pobEnt = CEntityManager::Get().FindEntity("PurgatoryManager");
	if (pobEnt)
	{
		Object_PurgatoryManager* pobPurg = (Object_PurgatoryManager*)pobEnt;
		bool bSwitched = pobPurg->SwitchCheckpoint(Object_PurgatoryManager::PREVIOUS_CHECKPOINT);

		if (bSwitched)
		{
			CGuiSoundManager::Get().PlaySound( CGuiSound::ACTION_CHANGE_CHECKPOINT );
			UpdateCheckpointInfo();

			bResult = true;
			m_obPadTimer.Set( m_fPadTime );
		}
	}

	return bResult;
}

bool CGuiSkinCheckpointSelect::MoveUpAction( int iPads )
{
	bool bResult = super::MoveUpAction(iPads);

	if ( !m_obPadTimer.Passed() )
		return bResult;

	CEntity* pobEnt = CEntityManager::Get().FindEntity("PurgatoryManager");
	if (pobEnt)
	{
		Object_PurgatoryManager* pobPurg = (Object_PurgatoryManager*)pobEnt;
		bool bSwitched = pobPurg->SwitchCheckpoint(Object_PurgatoryManager::NEXT_CHECKPOINT);

		if (bSwitched)
		{
			CGuiSoundManager::Get().PlaySound( CGuiSound::ACTION_CHANGE_CHECKPOINT );
			UpdateCheckpointInfo();

			bResult = true;
			m_obPadTimer.Set( m_fPadTime );
		}
	}

	return bResult;
}

void CGuiSkinCheckpointSelect::UpdateCheckpointInfo()
{
	CGuiUnit* pobCheckpointInfoUnit = GetParentScreen()->FindChildUnit("checkpointinfo");
	ntAssert_p(pobCheckpointInfoUnit, ("Failed to find \"checkpointinfo\" field on this FE screen\n"));

	//sync to current data
	CGuiSkinCheckpointInfo* pobTmp = static_cast<CGuiSkinCheckpointInfo*>(pobCheckpointInfoUnit);
	pobTmp->UpdateCheckpointInfo();
}

void CGuiSkinCheckpointSelect::OnChapterSwitchBegin()
{
	CGuiUnit* pobNavUnit = GetParentScreen()->FindChildUnit("navbar");
	ntError_p(pobNavUnit, ("Failed to find \"navbar\" TEXT field on this FE screen\n"));

	//Hide the chapter name
	pobNavUnit->SetAttribute("descriptiontitleid", "");

	//hide controls aswell
	pobNavUnit->SetAttribute("showselect", "false");
	pobNavUnit->SetAttribute("showback", "false");

	//now propagate thees changes
	pobNavUnit->SetData("syncattributes", NULL);

	//Additionally we need to tell the checkpointinfo to sync

	//find checkpointinfo
	CGuiUnit* pobCheckpointInfoUnit = GetParentScreen()->FindChildUnit("checkpointinfo");
	ntAssert_p(pobCheckpointInfoUnit, ("Failed to find \"checkpointinfo\" field on this FE screen\n"));

	//start sync to current data
	CGuiSkinCheckpointInfo* pobTmp = static_cast<CGuiSkinCheckpointInfo*>(pobCheckpointInfoUnit);
	pobTmp->OnChapterSwitchBegin();
}

void CGuiSkinCheckpointSelect::OnChapterSwitchEnd()
{
	//look up the purg manager and read the currently selected chapter
	CEntity* pobEnt = CEntityManager::Get().FindEntity("PurgatoryManager");
	ntError_p(pobEnt, ("Failed to find PurgatoryManager\n"));
	Object_PurgatoryManager* pobPurg = static_cast<Object_PurgatoryManager*>(pobEnt);
	int iChapter = pobPurg->CurrentSelectedChapter();

	//now look up the chapter's data
	GameInfoDef* pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
	ntError_p(pobGameInfo, ("GameInfo object not found") );
	const ChapterInfoDef* pobChapterInfo = pobGameInfo->GetChapter(iChapter);
	ntError_p(pobChapterInfo, ("Failed to find chapter %d in GameInfo", iChapter) );

	//find nav bar
	CGuiUnit* pobNavUnit = GetParentScreen()->FindChildUnit("navbar");
	ntError_p(pobNavUnit, ("Failed to find \"navbar\" TEXT field on this FE screen\n"));

	//update the chapter name
	pobNavUnit->SetAttribute("descriptiontitleid",  ntStr::GetString(pobChapterInfo->ChapterTitleID()));

	//and show the controls
	pobNavUnit->SetAttribute("showselect", "true");
	pobNavUnit->SetAttribute("showback", "true");

	//now propagate thees changes
	pobNavUnit->SetData("syncattributes", NULL);

	//Additionally we need to tell the checkpointinfo to sync

	//find checkpointinfo
	CGuiUnit* pobCheckpointInfoUnit = GetParentScreen()->FindChildUnit("checkpointinfo");
	ntAssert_p(pobCheckpointInfoUnit, ("Failed to find \"checkpointinfo\" field on this FE screen\n"));

	//end sync to current data
	CGuiSkinCheckpointInfo* pobTmp = static_cast<CGuiSkinCheckpointInfo*>(pobCheckpointInfoUnit);
	pobTmp->OnChapterSwitchEnd();

}
