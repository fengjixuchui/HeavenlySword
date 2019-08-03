/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface static unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

// Includes
#include "debugskinlevelselect.h"
#include "gui/guimanager.h"
#include "gui/guiutil.h"
#include "anim/hierarchy.h"
#include "core/debug.h"

// FIX ME TOM only using visualdebugger until 2D font ready
#include "core/visualdebugger.h"

#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gfx/graphicsdevice.h"
#include "gfx/texturemanager.h"

#include "game/shellconfig.h"

#include "expat/xmlparse.h"
#include "core/io.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiDebugSkinLevelSelect(); }

// Register this class under it's XML tag
bool g_bDEBUG_LEVELSELECT = CGuiManager::Register( "LEVEL_SELECT", &ConstructWrapper );

ntstd::String CGuiDebugSkinLevelSelect::ms_obSelectedLevel = "";

/***************************************************************************************************
*
*	FUNCTION		CGuiDebugSkinLevelSelect::CGuiDebugSkinLevelSelect
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiDebugSkinLevelSelect::CGuiDebugSkinLevelSelect( void )
{
	m_bLevelsAvailable = true;

	m_pLevelSelect = 0;
	m_pScrollBar = 0;
	m_pLevelListView = 0;

	m_eRenderSpace = RENDER_SCREENSPACE;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiDebugSkinLevelSelect::~CGuiDebugSkinLevelSelect
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiDebugSkinLevelSelect::~CGuiDebugSkinLevelSelect( void )
{
	NT_DELETE(m_pLevelSelect);
}

int g_iDebugLevelLoadCounter = 0;

void CGuiDebugSkinLevelSelect::UpdateIdle( void )
{
	CGuiUnit::UpdateIdle();

	g_iDebugLevelLoadCounter++;	// be aware that this is called many times...

	if (g_iDebugLevelLoadCounter <= g_ShellOptions->m_iGUIStartupLevelSkipCount)
	{
		CGuiManager::Get().MoveOnScreen();
		return;
	}

	m_pLevelSelect->Update();
}

/***************************************************************************************************
*
*	FUNCTION		CGuiDebugSkinLevelSelect::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiDebugSkinLevelSelect::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	return CGuiUnit::ProcessAttribute( pcTitle, pcValue );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiDebugSkinLevelSelect::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiDebugSkinLevelSelect::ProcessEnd( void )
{
	//now call the base
	CGuiUnit::ProcessEnd();

	// Overide default render space...
	m_eRenderSpace = CGuiUnit::RENDER_SCREENSPACE;

#ifdef ENUM_FILE_SYS
	EnumerateLevels( "" );
#else
	EnumerateLevels( "LevelList.xml" );
#endif

	WindowRect rect = {0.25f, 0.33f, 0.5f, 0.33f};
	m_pLevelSelect = NT_NEW CPanelWindow(NULL);
	m_pLevelSelect->SetExtents(rect);
//	m_pLevelSelect->SetBackgroundColour(CVector(0.0f, 0.514f, 0.627f, 0.0f));
	m_pLevelSelect->HidePanel(true);	
	m_pLevelSelect->Create();

	WindowRect rectTitle = {0.0f, 0.0f, 1.0f, 0.08f};
	CTitleWindow* pTitle = NT_NEW CTitleWindow(m_pLevelSelect);
	pTitle->SetExtents(rectTitle);
//	pTitle->SetTitle("IDS_LEVELSELECT_TITLE");
	pTitle->SetTitle("Select Level");
//	pTitle->SetBackgroundColour(CVector(1.0f, 0.714f, 0.498f, 1.0f));
	pTitle->HidePanel(true);	
	pTitle->Create();
	m_pLevelSelect->AttachChild(pTitle);

	WindowRect rectMain = {0.0f, 0.1f, 0.95f, 0.9f};
	m_pLevelListView = NT_NEW CListViewWindow(m_pLevelSelect);
	m_pLevelListView->SetExtents(rectMain);
//	m_pLevelListView->SetBackgroundColour(CVector(1.0f, 0.933f, 0.875f, 1.0f));
	m_pLevelListView->HidePanel(true);	
	m_pLevelListView->Create();
	m_pLevelSelect->AttachChild(m_pLevelListView);

	WindowRect rectTrackerBar = {0.96f, 0.1f, 0.04f, 0.9f};
	m_pScrollBar = NT_NEW CScrollBarWindow(m_pLevelSelect);
	m_pScrollBar->SetExtents(rectTrackerBar);
	m_pScrollBar->SetBackgroundColour(CVector(1.0f, 0.855f, 0.749f, 1.0f));
	m_pScrollBar->Create();
	m_pLevelSelect->AttachChild(m_pScrollBar);

	ntstd::Vector<ntstd::String> items;
	for (LevelList::iterator it = m_obLevelList.begin(); it != m_obLevelList.end(); ++it)
	{
		items.push_back((*it).name + " [" + (*it).path + "]");
	}

	if (items.empty())
	{
		items.push_back("None available");
		m_bLevelsAvailable = false;
	}

	m_pLevelListView->SetContents(items);

	//
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiDebugSkinLevelSelect::EnumerateLevels()
*
*	DESCRIPTION		
*
***************************************************************************************************/

#ifdef ENUM_FILE_SYS
void CGuiDebugSkinLevelSelect::EnumerateLevels(const char* szRootPath)
{
	const char* szLevelPath = "levels\\";

	char szBuffer[MAX_PATH] = {0};

	WIN32_FIND_DATA data;
	memset(&data, 0, sizeof(data));

    //build initial path
	strcpy(szBuffer, szLevelPath);
	strcat(szBuffer, szRootPath);
	strcat(szBuffer, "*.*");

	//Go
	HANDLE hFileSearch = FindFirstFile(szBuffer, &data);
	if (hFileSearch != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0)
					continue;

				//build subdir path
				strcpy(szBuffer, szRootPath);
				strcat(szBuffer, data.cFileName);
				strcat(szBuffer, "\\");
				//Enum level
				EnumerateLevels(szBuffer);
			}
			else
			{
				int len = strlen(data.cFileName);
				if (len < 4)
					continue;

				if ( 0 != strcmp(data.cFileName + len - 4, ".man") )
					continue;

				//build level path
				strcpy(szBuffer, szRootPath);
				strcat(szBuffer, data.cFileName);

				szBuffer[strlen(szBuffer)-4] = 0;	// get rid of '.man'

				// convert \ to /
				char* pChr = szBuffer;
				while (*pChr++)
					if (*pChr == '\\') *pChr = '/';

				//ntPrintf("Found %s\n", szBuffer);
				m_obLevelList.push_back(szBuffer);
			}
		}
		while (FindNextFile(hFileSearch, &data));
	}
}
#else
void CGuiDebugSkinLevelSelect::StartElementHandler(void *pvUserData, const char *el, const char **attr)
{
	if ( strcmp(el, "LEVEL") != 0)
		return;

	LevelList* pLevelList= (LevelList*)pvUserData;
	LevelInfo info;

	while (*attr)
	{
		const char* szAttrName = *attr;
		attr++;

		const char* szAttrVal = *attr;
		attr++;

		if( strcmp("name", szAttrName) == 0 )
		{
			info.name = szAttrVal;
		}
		else if( strcmp("path", szAttrName) == 0 )
		{
			info.path = szAttrVal;
		}
	}

	pLevelList->push_back(info);
}

void CGuiDebugSkinLevelSelect::EndElementHandler(void *pvUserData, const char *el)
{
	UNUSED(pvUserData);	UNUSED(el);
}

void CGuiDebugSkinLevelSelect::CommentHandler( void* pvUserData, const char* pcComment )
{
	UNUSED(pvUserData);	UNUSED(pcComment);
}

void CGuiDebugSkinLevelSelect::EnumerateLevels(const char* szLevelsFile)
{
	char acLevelListPath [MAX_PATH];
	Util::SetToNeutralResources();
	Util::GetFiosFilePath( szLevelsFile, acLevelListPath );

	if (File::Exists(acLevelListPath))
	{
		FileBuffer obFile( acLevelListPath, true );

		XML_Parser obParser = XML_ParserCreate( 0 );

		XML_SetUserData( obParser, &m_obLevelList );

		XML_SetElementHandler( obParser, StartElementHandler, EndElementHandler );

		XML_SetCommentHandler( obParser, CommentHandler );

		if ( !XML_Parse( obParser, *obFile, obFile.GetSize(), 1 ) )
		{
			user_warn_msg( ("Parse ntError at line %d:\n%s (see log for filename)\n", XML_GetCurrentLineNumber( obParser ), XML_ErrorString( XML_GetErrorCode( obParser ) ) ) );
		}

		XML_ParserFree(obParser);
	}
	else
	{
		user_warn_msg( ("File %s could not be found\n", szLevelsFile) );
	}
}
#endif

/***************************************************************************************************
*
*	FUNCTION		CGuiPurgatorySkinLevelSelect::Render()
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiDebugSkinLevelSelect::Render( void )
{
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	float fPos = m_pLevelListView->GetSelection()/((float)m_obLevelList.size()-1);
	m_pScrollBar->SetTrackerPosition(fPos);

	m_pLevelSelect->Render();

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

	return CGuiUnit::Render();
}


bool CGuiDebugSkinLevelSelect::MoveDownAction( int iPads )
{
	CGuiUnit::MoveDownAction(iPads);
	m_pLevelListView->MoveSelection(1);

	return true;
}

bool CGuiDebugSkinLevelSelect::MoveUpAction( int iPads )
{
	CGuiUnit::MoveUpAction(iPads);
	m_pLevelListView->MoveSelection(-1);

	return true;
}

bool CGuiDebugSkinLevelSelect::AnyAction( int iPads )
{
	UNUSED(iPads);
	PAD_NUMBER pad = PAD_0;

	if (CInputHardware::Get().GetPad( pad ).GetPressed() & PAD_TOP_2)
	{
		m_pLevelListView->MoveSelection(-10);
	}
	if (CInputHardware::Get().GetPad( pad ).GetPressed() & PAD_TOP_4)
	{
		m_pLevelListView->MoveSelection(10);
	}

	return false;
}

bool CGuiDebugSkinLevelSelect::StartAction( int iPads )
{
	return SelectAction(iPads);
}

bool CGuiDebugSkinLevelSelect::SelectAction( int iPads )
{
	UNUSED(iPads);

	if (m_bLevelsAvailable)
	{
		int iIndex = m_pLevelListView->GetSelectedItemIndex();
		ms_obSelectedLevel = m_obLevelList[iIndex].path;
	}
	else
	{
		return true;
	}

	return false;
}
