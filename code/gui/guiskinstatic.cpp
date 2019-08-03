/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface static unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

// Includes
#include "guiskinstatic.h"
#include "guimanager.h"
#include "guitext.h"
#include "anim/hierarchy.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinTestStatic(); }

// Register this class under it's XML tag
bool g_bSTATIC = CGuiManager::Register( "STATIC", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestStatic::CGuiSkinTestStatic
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinTestStatic::CGuiSkinTestStatic( void )
{
	m_pcStringTextID = 0;
	m_pcFontName = 0;

	m_fDuration = 0.0f;
}


CGuiSkinTestStatic::CGuiSkinTestStatic( const char* pcStringID, const char* pcFont, CPoint& obPos, float fDuration )
{
	// Set our values
	char* pcTempString = NT_NEW char[ strlen( pcStringID ) + 1 ];
	strcpy( pcTempString, pcStringID );
	m_pcStringTextID = pcTempString;

	char* pcTempStringB = NT_NEW char[ strlen( pcFont ) + 1 ];
	strcpy( pcTempStringB, pcFont );
	m_pcFontName = pcTempStringB;

	m_fDuration = fDuration;

	m_eRenderSpace = RENDER_SCREENSPACE;
	m_eHorizontalJustification = JUSTIFY_CENTRE;
	m_eVerticalJustification = JUSTIFY_MIDDLE;

	// TGS hack

	char strPosition[1024];
	sprintf( strPosition, "%f, %f, 0.0", obPos.X(), obPos.Y() );

	ProcessBasePositionValue( strPosition );

	ProcessEnd();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestStatic::~CGuiSkinTestStatic
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinTestStatic::~CGuiSkinTestStatic( void )
{
	NT_DELETE_ARRAY( m_pcStringTextID );
	CStringManager::Get().DestroyString( m_pobString );
	NT_DELETE_ARRAY( m_pcFontName );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestStatic::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinTestStatic::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnitStatic::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "titlestringid" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcStringTextID );
		}

		if ( strcmp( pcTitle, "font" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcFontName );
		}

		if ( strcmp( pcTitle, "duration" ) == 0 )
		{
			return GuiUtil::SetFloat( pcValue, &m_fDuration );
		}

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestStatic::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinTestStatic::ProcessEnd( void )
{
	// Call the base first
	CGuiUnitStatic::ProcessEnd();

	// Set up our string
	CStringDefinition obStringDef;
	obStringDef.m_fZOffset = 0.0f;
	obStringDef.m_fXOffset = 0.0f;

	obStringDef.m_pobFont = CFontManager::Get().GetFont( m_pcFontName );
	m_pobString = CStringManager::Get().MakeString( m_pcStringTextID, obStringDef, m_pobBaseTransform, m_eRenderSpace );
	
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestStatic::Render
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool	CGuiSkinTestStatic::Render( void )
{
	if ( m_eUnitState != STATE_DEAD )
		m_pobString->Render();

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestStatic::SetStateFocusIn
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiSkinTestStatic::SetStateIdle( void )
{
	CGuiUnitStatic::SetStateIdle();
	if ( m_fDuration > 0.0f )
	{
		m_obTimer.Set(m_fDuration);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestStatic::UpdateFocusIn
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiSkinTestStatic::UpdateIdle( void )
{
	// Take away the time change
	if ( m_fDuration > 0.0f )
	{
		m_obTimer.Update();

		// If we have finished 
		if ( m_obTimer.Passed() )
		{
			SetStateExit();
		}
	}
}
