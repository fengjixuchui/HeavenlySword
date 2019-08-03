/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskininputaction.h"
#include "gui/guimanager.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinInputAction(); }

// Register this class under it's XML tag
bool g_bINPUTACTION = CGuiManager::Register( "INPUTACTION", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinInputAction::CGuiSkinInputAction
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinInputAction::CGuiSkinInputAction( void )
	: m_iAcceptsInput(CGuiInput::ACCEPTS_NONE)
{}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinInputAction::~CGuiSkinInputAction
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinInputAction::~CGuiSkinInputAction( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinInputAction::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinInputAction::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnit::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "acceptsinput" ) == 0 )
		{
			return GuiUtil::SetFlags(pcValue, &CGuiInput::ms_astrAcceptsInputFlags[0], &m_iAcceptsInput);
		}

		return m_obAction.ProcessAttribute(pcTitle, pcValue);
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinInputAction::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinInputAction::ProcessEnd( void )
{
	// Call the base first
	CGuiUnit::ProcessEnd();

	return true;
}

bool CGuiSkinInputAction::Action()
{
	return m_obAction.TriggerAction(GetParentScreen());
}

bool CGuiSkinInputAction::AnyAction( int iPads )
{
	UNUSED(iPads);
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_ANY))
		return Action();
	return false;
}

bool CGuiSkinInputAction::MoveLeftAction( int iPads )
{
	UNUSED(iPads);
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_LEFT))
		return Action();
	return false;
}

bool CGuiSkinInputAction::MoveRightAction( int iPads )
{
	UNUSED(iPads);
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_RIGHT))
		return Action();
	return false;
}

bool CGuiSkinInputAction::MoveDownAction( int iPads )
{
	UNUSED(iPads);
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_DOWN))
		return Action();
	return false;
}

bool CGuiSkinInputAction::MoveUpAction( int iPads )
{
	UNUSED(iPads);
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_UP))
		return Action();
	return false;
}

bool CGuiSkinInputAction::StartAction( int iPads )
{
	UNUSED(iPads);
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_START))
		return Action();
	return false;
}

bool CGuiSkinInputAction::SelectAction( int iPads )
{
	UNUSED(iPads);
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_SELECT))
		return Action();
	return false;
}

bool CGuiSkinInputAction::BackAction( int iPads )
{
	UNUSED(iPads);
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_BACK))
		return Action();
	return false;
}

bool CGuiSkinInputAction::DeleteAction( int iPads )
{
	UNUSED(iPads);
	if (CGuiInput::AcceptsInput(m_iAcceptsInput, CGuiInput::ACCEPTS_DELETE))
		return Action();
	return false;
}
