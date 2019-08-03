/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface button unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

// Includes
#include "guiskinevent.h"
#include "guimanager.h"
#include "guitext.h"


/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinEvent(); }

// Register this class under it's XML tag
bool g_bEVENT = CGuiManager::Register( "EVENT", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinEvent::CGuiSkinEvent
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinEvent::CGuiSkinEvent( void )
	:
	m_obScriptAction(CGuiAction::RUN_SCRIPT),
	m_eEventType(ET_NONE)
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinEvent::~CGuiSkinEvent
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinEvent::~CGuiSkinEvent( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinEvent::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinEvent::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnit::ProcessAttribute( pcTitle, pcValue ) )
	{
		if (!m_obScriptAction.ProcessAttribute(pcTitle, pcValue))
		{
			m_eEventType = GetEventType( pcTitle );

			switch	( m_eEventType )
			{
				case ET_NONE:
					break;
				default:
					return m_obScriptAction.ProcessAttribute("script", pcValue);
			}
		
			return false;
		}
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinEvent::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinEvent::ProcessEnd( void )
{
	// Call the base first
	CGuiUnit::ProcessEnd();

	return true;
}

/*
bool CGuiSkinEvent::BeginEnter( bool bForce )
{
	ProcessEvent( ET_BEGINEXIT );

	CGuiUnit::BeginExit( bForce );

	// TODO : Maybe more logic here to make it safe.

	return true;
}
bool CGuiSkinEvent::BeginIdle( bool bForce )
{
	ProcessEvent( ET_BEGINIDLE );

	CGuiUnit::BeginIdle( bForce );

	// TODO : Maybe more logic here to make it safe.

	return true;
}

bool CGuiSkinEvent::BeginExit( bool bForce )
{
	ProcessEvent( ET_BEGINEXIT );

	CGuiUnit::BeginExit( bForce );

	// TODO : Maybe more logic here to make it safe.

	return true;
}

void CGuiSkinEvent::UpdateIdle( void )
{
	ProcessEvent( ET_UPDATEIDLE );

	CGuiUnit::UpdateIdle();
}

*/

void CGuiSkinEvent::SetStateEnter( void )
{
	CGuiUnit::SetStateEnter();
	ProcessEvent( ET_BEGINENTER );
}

void CGuiSkinEvent::SetStateIdle( void )
{
	CGuiUnit::SetStateIdle();
	ProcessEvent( ET_BEGINIDLE);
}

void CGuiSkinEvent::SetStateExit( void )
{
	CGuiUnit::SetStateExit();
	ProcessEvent( ET_BEGINEXIT );
}

void CGuiSkinEvent::UpdateEnter( void )
{
	CGuiUnit::UpdateEnter();
	ProcessEvent( ET_UPDATEENTER );
}

void CGuiSkinEvent::UpdateIdle( void )
{
	CGuiUnit::UpdateIdle();
	ProcessEvent( ET_UPDATEIDLE );
}

void CGuiSkinEvent::UpdateExit( void )
{
	CGuiUnit::UpdateExit();
	ProcessEvent( ET_UPDATEEXIT );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinEvent::ProcessEvent
*
*	DESCRIPTION		Perform action required for the event.
*
***************************************************************************************************/

bool CGuiSkinEvent::ProcessEvent( eEVENT_TYPE eEventType )
{
	if (m_eEventType == eEventType)
	{
		m_obScriptAction.TriggerAction(GetParentScreen());
		return true;
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinEvent::GetEventType
*
*	DESCRIPTION		Gets the event type enum from event string.
*					Returns ET_NONE if pcEvent is invalid.
*
***************************************************************************************************/

CGuiSkinEvent::eEVENT_TYPE CGuiSkinEvent::GetEventType( const char * pcEvent )
{
	if ( strcmp( pcEvent, "beginenter" ) == 0 )
	{
		return ET_BEGINENTER;
	}
	else if	( strcmp( pcEvent, "beginidle" ) == 0 )
	{
		return ET_BEGINIDLE;
	}
	else if	( strcmp( pcEvent, "beginexit" ) == 0 )
	{
		return ET_BEGINEXIT;
	}
	else if	( strcmp( pcEvent, "updateenter" ) == 0 )
	{
		return ET_UPDATEENTER;
	}
	else if	( strcmp( pcEvent, "updateidle" ) == 0 )
	{
		return ET_UPDATEIDLE;
	}
	else if	( strcmp( pcEvent, "updateexit" ) == 0 )
	{
		return ET_UPDATEEXIT;
	}

	return ET_NONE;
}
