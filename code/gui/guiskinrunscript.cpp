/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskinrunscript.h"
#include "gui/guimanager.h"
#include "gui/guiutil.h"

#include "lua/ninjalua.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinRunScript(); }

// Register this class under it's XML tag
bool g_bRUNSCRIPT = CGuiManager::Register( "RUNSCRIPT", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinRunScript::CGuiSkinRunScript
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinRunScript::CGuiSkinRunScript( void )
	:
	m_obScriptAction(CGuiAction::RUN_SCRIPT)
{}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinRunScript::~CGuiSkinRunScript
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinRunScript::~CGuiSkinRunScript( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinRunScript::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinRunScript::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnit::ProcessAttribute( pcTitle, pcValue ) )
	{
		return m_obScriptAction.ProcessAttribute(pcTitle, pcValue);
	}

	return true;
}

void CGuiSkinRunScript::UpdateIdle()
{
	CGuiUnit::UpdateIdle();

	Execute();

	SetStateDead();
}

void CGuiSkinRunScript::Execute()
{
	m_obScriptAction.TriggerAction(GetParentScreen());
}
