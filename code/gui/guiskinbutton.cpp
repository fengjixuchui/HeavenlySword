/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface button unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

// Includes
#include "guiskinbutton.h"
#include "guimanager.h"
#include "guitext.h"
#include "anim/hierarchy.h"


/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinTestButton(); }

// Register this class under it's XML tag
bool g_bBUTTON = CGuiManager::Register( "BUTTON", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestButton::CGuiSkinTestButton
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinTestButton::CGuiSkinTestButton( void )
{
	m_pcStringTextID = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestButton::~CGuiSkinTestButton
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinTestButton::~CGuiSkinTestButton( void )
{
	NT_DELETE_ARRAY( m_pcStringTextID );
	CStringManager::Get().DestroyString( m_pobString );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestButton::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinTestButton::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnitButton::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "titlestringid" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcStringTextID );
		}

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestButton::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinTestButton::ProcessEnd( void )
{
	// Call the base first
	CGuiUnitButton::ProcessEnd();

	// Set up our string
	CStringDefinition obStringDef;
	obStringDef.m_fZOffset = 0.0f;
	obStringDef.m_fXOffset = 0.0f;
	m_pobString = CStringManager::Get().MakeString( m_pcStringTextID, obStringDef, m_pobHierarchy->GetTransform( 0 ), m_eRenderSpace );

	return true;
}


// Temp so we can see whats selected.  Can remove if we get Focus anims
void	CGuiSkinTestButton::UpdateFocusIn( void )
{
	CGuiUnit::UpdateFocusIn();

	CPoint Pos = m_BasePosition;
	Pos.Z() -= 0.5f; 
	CMatrix local = m_pobBaseTransform->GetLocalMatrix();
	local.SetTranslation( Pos );
	m_pobBaseTransform->SetLocalMatrix( local );
}

void	CGuiSkinTestButton::UpdateFocusOut( void )
{
	CGuiUnit::UpdateFocusOut();

	CPoint Pos = m_BasePosition;
	Pos.Z() += 0.5f; 
	CMatrix local = m_pobBaseTransform->GetLocalMatrix();
	local.SetTranslation( Pos );
	m_pobBaseTransform->SetLocalMatrix( local );
}

