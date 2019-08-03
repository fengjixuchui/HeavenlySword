/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface static unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

// Includes
#include "e3guiskinstatic.h"
#include "guimanager.h"
#include "guiutil.h"
#include "anim/hierarchy.h"

// FIX ME TOM only using visualdebugger until 2D font ready
#include "core/visualdebugger.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiE3SkinStatic(); }

// Register this class under it's XML tag
bool g_bE3_STATIC = CGuiManager::Register( "E3_STATIC", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinStatic::CGuiE3SkinStatic
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiE3SkinStatic::CGuiE3SkinStatic( void )
{
	m_pcStringText = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinStatic::~CGuiE3SkinStatic
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiE3SkinStatic::~CGuiE3SkinStatic( void )
{
	if (m_pcStringText)
		NT_DELETE (m_pcStringText);
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinStatic::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiE3SkinStatic::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnitStatic::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "titlestringid" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcStringText);
		}

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinStatic::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiE3SkinStatic::ProcessEnd( void )
{
	// Call the base first
	CGuiUnitStatic::ProcessEnd();

	// We should have all attributes here so set up the image sprite

	// Overide default render space.
	m_eRenderSpace = CGuiUnit::RENDER_SCREENSPACE;

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinStatic::Render()
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiE3SkinStatic::Render( void )
{
#ifndef _GOLD_MASTER
	if ( m_pcStringText )
	{
		float fBBWidth = CGuiManager::Get().BBWidth();
		float fBBHeight = CGuiManager::Get().BBHeight();

		g_VisualDebug->Printf2D(m_BasePosition.X()*fBBWidth, m_BasePosition.Y()*fBBHeight, DC_WHITE,0,m_pcStringText);
	}
#endif

	return true;
}

