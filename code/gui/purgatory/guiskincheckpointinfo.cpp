/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskincheckpointinfo.h"
#include "gui/guimanager.h"
#include "gui/guiscreen.h"

#include "game/entitymanager.h"
#include "game/entitypurgatorymanager.h"
#include "objectdatabase/dataobject.h"
#include "game/checkpointmanager.h"

#include "hud/messagedata.h"

#include "gui/guisettings.h"
#include "core/timer.h"

#include "core/visualdebugger.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinCheckpointInfo(); }

// Register this class under it's XML tag
bool g_bSKINCHECKPOINTINFO = CGuiManager::Register( "CHECKPOINTINFO", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinCheckpointInfo::CGuiSkinCheckpointInfo
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinCheckpointInfo::CGuiSkinCheckpointInfo( void )
:
	m_pcImageTemplate(NULL),
	m_fYCurrentOffset(0.0f),
	m_pcLifeClockTitleId(NULL),
	m_pcLifeClockValueTitleId(NULL),
	m_pcLifeClockBestTitleId(NULL),
	m_pcLifeClockBestValueTitleId(NULL)
{
	m_pobMessageDataManager = NT_NEW MessageDataManager();

	m_obTitleStrDef.m_eHJustify = CStringDefinition::JUSTIFY_RIGHT;
        m_obTitleStrDef.m_eVJustify = CStringDefinition::JUSTIFY_MIDDLE;
	m_obTitleStrDef.m_pobFont = CFontManager::Get().GetFont("Body");//CStringManager::Get().GetLevelLanguage()->GetFont("TGS1");
	m_obTitleStrDef.m_pobMessageDataManager = m_pobMessageDataManager;

	m_obValueStrDef.m_eHJustify = CStringDefinition::JUSTIFY_RIGHT;
        m_obValueStrDef.m_eVJustify = CStringDefinition::JUSTIFY_MIDDLE;
	m_obValueStrDef.m_pobFont = CFontManager::Get().GetFont("Body");//CStringManager::Get().GetLevelLanguage()->GetFont("TGS1");
	m_obValueStrDef.m_pobMessageDataManager = m_pobMessageDataManager;

	m_pobMessageDataManager->CreateValue("LC_DAY", 0);
	m_pobMessageDataManager->CreateValue("LC_HR", 0);
	m_pobMessageDataManager->CreateValue("LC_MIN", 0);
	m_pobMessageDataManager->CreateValue("LC_SEC", 0);

	m_pobMessageDataManager->CreateValue("LCB_DAY", 0);
	m_pobMessageDataManager->CreateValue("LCB_HR", 0);
	m_pobMessageDataManager->CreateValue("LCB_MIN", 0);
	m_pobMessageDataManager->CreateValue("LCB_SEC", 0);

	m_bCheckpointSwitchFadeRunning = false;
	m_iCheckpointSwitchFadeDir = 1;
	m_fCheckpointSwitchFade = 0.0f;

	m_pobTitleUnit = NULL;
	m_pobImageUnit = NULL;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinCheckpointInfo::~CGuiSkinCheckpointInfo
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinCheckpointInfo::~CGuiSkinCheckpointInfo( void )
{
	NT_DELETE_ARRAY(m_pcLifeClockTitleId);
	NT_DELETE_ARRAY(m_pcLifeClockValueTitleId);
	NT_DELETE_ARRAY(m_pcLifeClockBestTitleId);
	NT_DELETE_ARRAY(m_pcLifeClockBestValueTitleId);
	SetImageTemplate(NULL);
	ClearInfo();
	NT_DELETE(m_pobMessageDataManager);
}

bool CGuiSkinCheckpointInfo::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "imagetemplate" ) == 0 )
		{
			SetImageTemplate(pcValue);
			return true;
		}
		if ( strcmp( pcTitle, "lifeclocktitleid" ) == 0 )
		{
			GuiUtil::SetString(pcValue, &m_pcLifeClockTitleId);
			return true;
		}
		if ( strcmp( pcTitle, "lifeclockvaluetitleid" ) == 0 )
		{
			GuiUtil::SetString(pcValue, &m_pcLifeClockValueTitleId);
			return true;
		}
		if ( strcmp( pcTitle, "lifeclockbesttitleid" ) == 0 )
		{
			GuiUtil::SetString(pcValue, &m_pcLifeClockBestTitleId);
			return true;
		}
		if ( strcmp( pcTitle, "lifeclockbestvaluetitleid" ) == 0 )
		{
			GuiUtil::SetString(pcValue, &m_pcLifeClockBestValueTitleId);
			return true;
		}

		return false;
	}

	return true;
}

bool CGuiSkinCheckpointInfo::ProcessChild( CXMLElement* pobChild )
{
	super::ProcessChild(pobChild);
	
	ntAssert( pobChild );

	return true;
}

bool CGuiSkinCheckpointInfo::ProcessEnd( void )
{
	super::ProcessEnd();

	ntAssert(m_pcImageTemplate);
	ntAssert(m_pcLifeClockTitleId);
	ntAssert(m_pcLifeClockValueTitleId);
	ntAssert(m_pcLifeClockBestTitleId);
	ntAssert(m_pcLifeClockBestValueTitleId);

	//fill cache
	m_pobTitleUnit = GetParentScreen()->FindChildUnit("checkpointname");
	ntError_p(m_pobTitleUnit, ("Failed to find \"checkpointname\" TEXT field on this FE screen\n"));

	m_pobImageUnit = GetParentScreen()->FindChildUnit("checkpointimage");
	ntError_p(m_pobImageUnit, ("Failed to find \"checkpointimage\" IMAGE field on this FE screen\n"));

	return true;
}

bool CGuiSkinCheckpointInfo::Update( void )
{
	if (super::Update())
	{
		if (m_bCheckpointSwitchFadeRunning)
		{
			float fTimerChange = CTimer::Get().GetSystemTimeChange() * (1.0f / CGuiManager::Get().GuiSettings()->ScreenFadeTime());

			m_fCheckpointSwitchFade += m_iCheckpointSwitchFadeDir * fTimerChange;

			if (m_iCheckpointSwitchFadeDir == 1)	//fade in
			{
				if (m_fCheckpointSwitchFade > 1.0f)
				{
					m_fCheckpointSwitchFade = 1.0f;
					m_bCheckpointSwitchFadeRunning = false;
				}
			}
			else	//fade out
			{
				if (m_fCheckpointSwitchFade < 0.0f)
				{
					m_fCheckpointSwitchFade = 0.0f;
					m_bCheckpointSwitchFadeRunning = false;
				}
			}

			//poke fade value into title and image
			m_pobTitleUnit->SetFade(m_fCheckpointSwitchFade);
			m_pobImageUnit->SetFade(m_fCheckpointSwitchFade);

			if (!m_bCheckpointSwitchFadeRunning && m_fCheckpointSwitchFade == 1.0f)
			{
				m_pobTitleUnit->SetFadeComplete();
				m_pobImageUnit->SetFadeComplete();
	//			m_pobTitleUnit->SetAttribute("autofade", "true");
	//			m_pobImageUnit->SetAttribute("autofade", "true");
	//			m_bAutoFade = true;
			}
		}


		for( ntstd::List< CXMLElement* >::iterator obIt = m_obChildren.begin(); obIt != m_obChildren.end(); ++obIt)
		{
			((CGuiUnit*)( *obIt ))->Update();
		}

		return true;
	}

	return false;
}

bool CGuiSkinCheckpointInfo::Render( void )
{
	if (super::Render())
	{
		for( ntstd::List< CXMLElement* >::iterator obIt = m_obChildren.begin(); obIt != m_obChildren.end(); ++obIt)
		{
			((CGuiUnit*)( *obIt ))->Render();
		}

	//	g_VisualDebug->Printf2D(45, 500, 0xffffffff, 0, "m_fCheckpointSwitchFade = %f", m_fCheckpointSwitchFade);
	//	g_VisualDebug->Printf2D(45, 530, 0xffffffff, 0, "ScreenFade() = %f", ScreenFade());
	//	g_VisualDebug->Printf2D(45, 560, 0xffffffff, 0, "m_bAutoFade = %d", m_bAutoFade);

        //now render our components
		CVector obColour(1.0f, 1.0f, 1.0f, m_fCheckpointSwitchFade);

		for (InfoItemList::iterator obIt = m_obInfoItemList.begin(); obIt != m_obInfoItemList.end(); ++obIt)
		{
			(*obIt)->m_pobTitle->SetColour(obColour);
			(*obIt)->m_pobTitle->Render();
			(*obIt)->m_pobValue->SetColour(obColour);
			(*obIt)->m_pobValue->Render();
		}

		return true;
	}

	return false;
}

void CGuiSkinCheckpointInfo::UpdateFadeIn()
{
	super::UpdateFadeIn();

	m_fCheckpointSwitchFade = ScreenFade();

	//poke fade value into title and image
	m_pobTitleUnit->SetFade(m_fCheckpointSwitchFade);
	m_pobImageUnit->SetFade(m_fCheckpointSwitchFade);

	if (m_fCheckpointSwitchFade >= 1.0f)
	{
		m_pobTitleUnit->SetFadeComplete();
		m_pobImageUnit->SetFadeComplete();
	}
}

void CGuiSkinCheckpointInfo::UpdateFadeOut()
{
	super::UpdateFadeOut();

	m_fCheckpointSwitchFade = ScreenFade();

	//poke fade value into title and image
	m_pobTitleUnit->SetFade(m_fCheckpointSwitchFade);
	m_pobImageUnit->SetFade(m_fCheckpointSwitchFade);

	if (m_fCheckpointSwitchFade <= 0.0f)
	{
		m_pobTitleUnit->SetFadeComplete();
		m_pobImageUnit->SetFadeComplete();
	}
}

void CGuiSkinCheckpointInfo::SetImageTemplate(const char* pcImageTemplate)
{
	NT_DELETE_ARRAY(m_pcImageTemplate);
	m_pcImageTemplate = NULL;

	if (pcImageTemplate)
{
		GuiUtil::SetString(pcImageTemplate, &m_pcImageTemplate);
	}
}

void CGuiSkinCheckpointInfo::UnpackLifeclock(double dLifeclock, int& iHours, int& iMinutes, int& iSeconds)
		{
	double dSeconds = dLifeclock;

	iHours = (int)dSeconds / 3600;
	dSeconds -= iHours * 3600.0;

	iMinutes = (int)dSeconds / 60;
	dSeconds -= iMinutes * 60.0;

	iSeconds = (int)dSeconds;
}

void CGuiSkinCheckpointInfo::ClearInfo()
{
	for (InfoItemList::iterator obIt = m_obInfoItemList.begin(); obIt != m_obInfoItemList.end(); ++obIt)
	{
		CStringManager::Get().DestroyString((*obIt)->m_pobTitle);
		CStringManager::Get().DestroyString((*obIt)->m_pobValue);
		NT_DELETE(*obIt);
	}
	m_obInfoItemList.clear();

	m_fYCurrentOffset = 0.0f;
}

void CGuiSkinCheckpointInfo::AddInfo(const char* pcName, const char* pcValue)
{
	InfoItem* pobInfo = new InfoItem();

	m_obTitleStrDef.m_fXOffset = 0.0f;
	m_obTitleStrDef.m_fYOffset = m_fYCurrentOffset;
	pobInfo->m_pobTitle = CStringManager::Get().MakeString(pcName, m_obTitleStrDef, m_pobBaseTransform, m_eRenderSpace);

	m_fYCurrentOffset += pobInfo->m_pobTitle->RenderHeight();

	//we need to convert the value as it will not actually be stored in LAMS
/*	size_t iTextLength = strlen(pcValue);
#define MAX_STRING_LENGTH 128
	WCHAR_T wcBuf[MAX_STRING_LENGTH] = {0};
	ntAssert(iTextLength <= MAX_STRING_LENGTH);
	mbstowcs(wcBuf, pcValue, iTextLength);
*/
	m_obValueStrDef.m_fXOffset = 0.0f;
	m_obValueStrDef.m_fYOffset = m_fYCurrentOffset;
//	pobInfo->m_pobValue = CStringManager::Get().MakeString(wcBuf, m_obValueStrDef, m_pobBaseTransform, m_eRenderSpace);
	pobInfo->m_pobValue = CStringManager::Get().MakeString(pcValue, m_obValueStrDef, m_pobBaseTransform, m_eRenderSpace);

	m_fYCurrentOffset += pobInfo->m_pobValue->RenderHeight();

	m_obInfoItemList.push_back(pobInfo);
}

void CGuiSkinCheckpointInfo::OnChapterSwitchBegin()
{
//	m_pobTitleUnit->SetAttribute("autofade", "false");
//	m_pobImageUnit->SetAttribute("autofade", "false");
//	m_bAutoFade = false;

	m_bCheckpointSwitchFadeRunning = true;
	m_iCheckpointSwitchFadeDir = -1;
}

void CGuiSkinCheckpointInfo::OnChapterSwitchEnd()
{
//	m_pobTitleUnit->SetAttribute("autofade", "false");
//	m_pobImageUnit->SetAttribute("autofade", "false");
//	m_bAutoFade = false;

	UpdateCheckpointInfo();

	if (IsFading())
		return;

	m_bCheckpointSwitchFadeRunning = true;
	m_iCheckpointSwitchFadeDir = 1;
}

void CGuiSkinCheckpointInfo::UpdateCheckpointInfo()
{
	CEntity* pobEnt = CEntityManager::Get().FindEntity("PurgatoryManager");
	ntAssert_p(pobEnt, ("Failed to find PurgatoryManager\n"));

	Object_PurgatoryManager* pobPurg = static_cast<Object_PurgatoryManager*>(pobEnt);
	int iChapter = pobPurg->CurrentSelectedChapter();
	int iCheckpoint = pobPurg->CurrentSelectedCheckpoint();

	//Grab gameinfo
	GameInfoDef* pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
	ntAssert_p(pobGameInfo, ("GameInfo object not found") );

	//locate the chapter
	const ChapterInfoDef* pobChapterInfo = pobGameInfo->GetChapter(iChapter);
	ntAssert_p(pobChapterInfo, ("Failed to find chapter %d in GameInfo", iChapter) );

	//locate the checkpoint
	const CheckpointInfoDef* pobCPInfo = pobChapterInfo->GetCheckpoint(iCheckpoint);
	ntAssert_p(pobCPInfo, ("Failed to find checkpoint %d for chapter %d in GameInfo", iCheckpoint, iChapter) );

	//update the chapter name control
	m_pobTitleUnit->SetAttribute("titleid", ntStr::GetString(pobCPInfo->CheckpointTitleID()));

	//Setup the checkpoint Image
	char buf[256];

	// build image path
	ntAssert(m_pcImageTemplate);
	sprintf(buf, m_pcImageTemplate, iChapter, iCheckpoint);

	//assign to image
	m_pobImageUnit->SetAttribute("image", buf);

	//Now update the checkpoint stats
	ClearInfo();

	CheckpointData* pobData = CheckpointManager::Get().GetDataForCheckpoint(iChapter, iCheckpoint);
	int iHours = 0, iMinutes = 0, iSeconds = 0;

	//First lifeclock delta
	if (pobData)
	{
		double dLifeClockDelta = pobData->m_GlobalData.GetLifeClockDelta();
		UnpackLifeclock(dLifeClockDelta, iHours, iMinutes, iSeconds);
	}

	m_pobMessageDataManager->SetValue("LC_HR", iHours);
	m_pobMessageDataManager->SetValue("LC_MIN", iMinutes);
	m_pobMessageDataManager->SetValue("LC_SEC", iSeconds);

	AddInfo(m_pcLifeClockBestTitleId, m_pcLifeClockBestValueTitleId);

	//And now overall lifeclock
	if (pobData)
	{
		double dLifeClock = CheckpointManager::Get().GetLifeClockToThisCheckpoint(pobData);
		UnpackLifeclock(dLifeClock, iHours, iMinutes, iSeconds);
	}

	m_pobMessageDataManager->SetValue("LCB_HR", iHours);
	m_pobMessageDataManager->SetValue("LCB_MIN", iMinutes);
	m_pobMessageDataManager->SetValue("LCB_SEC", iSeconds);

	AddInfo(m_pcLifeClockTitleId, m_pcLifeClockValueTitleId);
}
