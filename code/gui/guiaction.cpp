/***************************************************************************************************
*
*	DESCRIPTION		
*	NOTES
*
***************************************************************************************************/

// Includes
#include "guiaction.h"
#include "gui/guimanager.h"
#include "gui/guiutil.h"
#include "gui/guilua.h"
#include "gui/guiscreen.h"
#include "game/shellmain.h"

// Convertion for bitwise enum values to strings - null terminated for converter

const CStringUtil::STRING_FLAG astrActionFlags[] = 
{
	{ CGuiAction::NO_ACTION,	"NO_ACTION"		},
	{ CGuiAction::MOVE_ON,		"MOVE_ON"		},
	{ CGuiAction::MOVE_BACK,	"MOVE_BACK"		},
	{ CGuiAction::EXIT,			"EXIT"			},
	{ CGuiAction::RUN_SCRIPT,	"RUN_SCRIPT"	},
	{ 0,						0				}
};

// Convertion for bitwise enum values to strings - null terminated for converter
const CStringUtil::STRING_FLAG astrLoadFromFlags[] = 
{
	{ CGuiAction::FROM_INLINE,	"INLINE"	},
	{ CGuiAction::FROM_FILE,	"FILE"		},
	{ 0,						0			} 
};

CGuiAction::CGuiAction(ACTION_TYPES eAction)
	:
	m_eAction(eAction),
	m_pcParam(NULL),
	m_eExecFrom(FROM_INLINE),
	m_pcScript(NULL)
{}

CGuiAction::~CGuiAction()
{
	NT_DELETE_ARRAY(m_pcParam);
	NT_DELETE_ARRAY(m_pcScript);
}

bool CGuiAction::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( strcmp( pcTitle, "action" ) == 0 )
	{
		return GuiUtil::SetFlags( pcValue, &astrActionFlags[0], (int*)&m_eAction);
	}
	else if ( strcmp( pcTitle, "param" ) == 0 )
	{
		return GuiUtil::SetString(pcValue, &m_pcParam);
	}
	else if ( strcmp( pcTitle, "from" ) == 0 )
	{
		return GuiUtil::SetFlags( pcValue, &astrLoadFromFlags[0], (int*)&m_eExecFrom);

	}
	else if ( strcmp( pcTitle, "script" ) == 0 )
	{
		return GuiUtil::SetString(pcValue, &m_pcScript);
	}
	return false;
}

bool CGuiAction::TriggerAction(CGuiScreen* pobContext)
{
	switch (m_eAction)
	{
	case NO_ACTION:
	//	ntPrintf("TriggerAction: NO_ACTION\n");
		return true;

	case MOVE_ON:
	//	ntPrintf("TriggerAction: MOVE_ON %s\n", m_pcParam?m_pcParam:"");
		if (m_pcParam == NULL)
			return CGuiManager::Get().MoveOnScreen();
		else
			return CGuiManager::Get().MoveOnScreenGroup(m_pcParam);

	case MOVE_BACK:
	//	ntPrintf("TriggerAction: MOVE_BACK %s\n", m_pcParam?m_pcParam:"");
		if (m_pcParam == NULL)
			return CGuiManager::Get().MoveBackScreen();
		else
			return CGuiManager::Get().MoveBackScreenGroup(m_pcParam);

	case EXIT:
	//	ntPrintf("TriggerAction: EXIT\n" );
		ShellMain::Get().RequestGameExit();
		return true;

	case RUN_SCRIPT:
	//	ntPrintf("TriggerAction: RUN_SCRIPT, %s\n", m_pcScript);

		Util::SetToNeutralResources();

		CGuiLua::PushContext(pobContext);
		if (m_eExecFrom == FROM_INLINE)
		{
			NinjaLua::DoBuffer(CLuaGlobal::Get().State(), m_pcScript, strlen(m_pcScript), "RUN_SCRIPT action");
		}
		else 
		{
			CLuaGlobal::Get().InstallFile(m_pcScript);
		}
		CGuiLua::PopContext();

		return true;
	};

	return false;
}
