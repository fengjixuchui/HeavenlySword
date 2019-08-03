/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface static unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

// Includes
#include "debugskinlevelswitch.h"
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

#include "gui/debug/debugskinlevelselect.h"

#include "game/shellconfig.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiDebugSkinLevelSwitch(); }

// Register this class under it's XML tag
bool g_bDEBUG_LEVELSWITCH = CGuiManager::Register( "LEVEL_SWITCH", &ConstructWrapper );


const CStringUtil::STRING_FLAG astrLoadFrom[] = 
{	
	{ CGuiDebugSkinLevelSwitch::LEVEL_SELECT,		"LEVEL_SELECT"		},	// Pulls the level name from the leveselect global
	{ CGuiDebugSkinLevelSwitch::DEFAULT_LEVEL,		"DEFAULT_LEVEL"		},	// Figures out the required level from various conditions
	{ 0,											0					} 
};


/***************************************************************************************************
*
*	FUNCTION		CGuiDebugSkinLevelSwitch::CGuiDebugSkinLevelSwitch
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiDebugSkinLevelSwitch::CGuiDebugSkinLevelSwitch( void )
{
	m_bLoadLevel = true;

	m_eLoadFrom = LEVEL_SELECT;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiDebugSkinLevelSwitch::~CGuiDebugSkinLevelSwitch
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiDebugSkinLevelSwitch::~CGuiDebugSkinLevelSwitch( void )
{
}
/*
bool CGuiDebugSkinLevelSwitch::Update( void )
{
	if (CGuiUnit::Update())
	{
        return true;
	}
	return false;
}
*/

extern int g_iDebugLevelLoadCounter;

bool ms_bFirstLoad = true;
void CGuiDebugSkinLevelSwitch::UpdateIdle()
{
	CGuiUnit::UpdateIdle();
//	user_warn_msg(("CGuiDebugSkinLevelSwitch::UpdateIdle"));

	g_iDebugLevelLoadCounter++;

	if (g_iDebugLevelLoadCounter <= g_ShellOptions->m_iGUIStartupLevelSkipCount)
	{
//			user_warn_msg(("CGuiDebugSkinLevelSwitch::UpdateIdle -> MoveOn"));
		CGuiManager::Get().MoveOnScreen();
//		SetStateExit();
		return;
	}

	if (m_bLoadLevel)
	{
		switch (m_eLoadFrom)
		{
		case LEVEL_SELECT:
			CGuiManager::LoadGameLevel_Name(CGuiDebugSkinLevelSelect::ms_obSelectedLevel.c_str(), -1, 0);
			break;

		case DEFAULT_LEVEL:
			CGuiManager::LoadDefaultGameLevel();
			break;
		}
		m_bLoadLevel = false;

//		SetStateExit();
		CGuiManager::Get().MoveOnScreen();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiDebugSkinLevelSwitch::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiDebugSkinLevelSwitch::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnit::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "loadfrom" ) == 0 )
		{
			return GuiUtil::SetFlags( pcValue, &astrLoadFrom[0], (int*)&m_eLoadFrom);
		}
		return false;
	}
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiDebugSkinLevelSwitch::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiDebugSkinLevelSwitch::ProcessEnd( void )
{
	//now call the base
	CGuiUnit::ProcessEnd();

	// Overide default render space.
	m_eRenderSpace = CGuiUnit::RENDER_SCREENSPACE;

	return true;
}
