/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskinmenutext.h"
#include "gui/guimanager.h"
#include "gui/guiscreen.h"
#include "gui/guisettings.h"

#define MAX_STRING_LENGTH 128

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinMenuText(); }

// Register this class under it's XML tag
bool g_bMENUTEXT = CGuiManager::Register( "MENUTEXT", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuText::CGuiSkinMenuText
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinMenuText::CGuiSkinMenuText( void )
:
	m_pcTitle(NULL),
	m_pobStr(NULL),
	m_bConstructing(true)
{
	const char* pcFont = CGuiManager::Get().GuiSettings()->BodyFont();
	m_obStrDef.m_pobFont = CFontManager::Get().GetFont(pcFont);//CStringManager::Get().GetLevelLanguage()->GetFont("TGS1");
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuText::~CGuiSkinMenuText
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinMenuText::~CGuiSkinMenuText( void )
{
	NT_DELETE_ARRAY(m_pcTitle);

	if (m_pobStr)
	{
		CStringManager::Get().DestroyString(m_pobStr);
		m_pobStr = NULL;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuText::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinMenuText::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "title" ) == 0 )
		{
			return ProcessTitleAttribute(pcValue);
		}
		else if ( strcmp( pcTitle, "titleid" ) == 0 )
		{
			return ProcessTitleIDAttribute(pcValue);
		}
		else if ( strcmp( pcTitle, "font" ) == 0 )
		{
			m_obStrDef.m_pobFont = GuiUtil::GetFontFromDescription(pcValue);
			return true;
		}
		return false;
	}

	if ( strcmp( pcTitle, "selectable" ) == 0 )
	{
		UpdateTextColourForSelectability();
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuText::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinMenuText::ProcessEnd( void )
{
	// Call the base first
	super::ProcessEnd();

	ntAssert(m_pcTitle);

	// Set the vertical offset of our image
	switch ( m_eVerticalJustification )
	{
		case JUSTIFY_TOP:		m_obStrDef.m_eVJustify = CStringDefinition::JUSTIFY_TOP;		break;
		case JUSTIFY_MIDDLE:	m_obStrDef.m_eVJustify = CStringDefinition::JUSTIFY_MIDDLE;	break;
		case JUSTIFY_BOTTOM:	m_obStrDef.m_eVJustify = CStringDefinition::JUSTIFY_BOTTOM;	break;
		default:				ntAssert( 0 );				break;
	}

	// Set the horizontal offset
	switch ( m_eHorizontalJustification )
	{
		case JUSTIFY_LEFT:		m_obStrDef.m_eHJustify = CStringDefinition::JUSTIFY_LEFT;		break;
		case JUSTIFY_CENTRE:	m_obStrDef.m_eHJustify = CStringDefinition::JUSTIFY_CENTRE;	break;
		case JUSTIFY_RIGHT:		m_obStrDef.m_eHJustify = CStringDefinition::JUSTIFY_RIGHT;	break;
		default:				ntAssert( 0 );				break;
	}

	m_bConstructing = false;

	//finally, create our string :)
	CreateString();

    return true;
}

bool CGuiSkinMenuText::ProcessTitleAttribute(const char* pcText)
{
	//reset text attrib
	NT_DELETE_ARRAY(m_pcTitle);
	m_pcTitle = NULL;

	//now set it
	bool bOk = GuiUtil::SetString(pcText, &m_pcTitle);

	m_bLocalised = false;

	//and create the string
	if (bOk)
	{
		CreateString();
	}
	return bOk;
}

bool CGuiSkinMenuText::ProcessTitleIDAttribute(const char* pcText)
{
	//reset text attrib
	NT_DELETE_ARRAY(m_pcTitleID);
	m_pcTitleID = NULL;

	//now set it
	bool bOk = GuiUtil::SetString(pcText, &m_pcTitleID);

	m_bLocalised = true;

	//and create the string
	if (bOk)
	{
		CreateString();
	}
	return bOk;
}

void CGuiSkinMenuText::CreateString()
{
	if (!m_pcTitle || m_bConstructing)
		return;

	//delete the existing string
	if (m_pobStr)
	{
		CStringManager::Get().DestroyString(m_pobStr);
		m_pobStr = NULL;
	}

	//now create a new one.
	if (m_bLocalised)
	{
		m_pobStr = CStringManager::Get().MakeString(m_pcTitleID, m_obStrDef, m_pobBaseTransform, m_eRenderSpace);
	}
	else
	{
	//	ntPrintf("GUI: Warning, using unlocalised string: \"%s\"", m_pcText );

		size_t iTextLength = strlen(m_pcTitle);

		WCHAR_T wcBuf[MAX_STRING_LENGTH] = {0};
		ntAssert(iTextLength <= MAX_STRING_LENGTH);

		mbstowcs(wcBuf, m_pcTitle, iTextLength);

		m_pobStr = CStringManager::Get().MakeString(wcBuf, m_obStrDef, m_pobBaseTransform, m_eRenderSpace);
	}

	ntAssert(m_pobStr);

	SetExtentsDirty();
	UpdateTextColourForSelectability();
}

void CGuiSkinMenuText::CalculateExtents()
{
	super::CalculateExtents();

	CPoint obTranslation = m_pobBaseTransform->GetWorldTranslation();

	m_obExtents.fWidth = m_pobStr->RenderWidth();
	m_obExtents.fHeight = m_pobStr->RenderHeight();

	switch ( m_eVerticalJustification )
	{
		case JUSTIFY_TOP:		m_obExtents.fY = obTranslation.Y(); break;
		case JUSTIFY_MIDDLE:	m_obExtents.fY = obTranslation.Y() - m_obExtents.fHeight/2.0f; break;
		case JUSTIFY_BOTTOM:	m_obExtents.fY = obTranslation.Y() - m_obExtents.fHeight; break;
		default:				ntAssert( 0 );				break;
	}

	// Set the horizontal offset
	switch ( m_eHorizontalJustification )
	{
		case JUSTIFY_LEFT:		m_obExtents.fX = obTranslation.X(); break;
		case JUSTIFY_CENTRE:	m_obExtents.fX = obTranslation.X() - m_obExtents.fWidth/2.0f; break;
		case JUSTIFY_RIGHT:		m_obExtents.fX = obTranslation.X() - m_obExtents.fWidth; break;
		default:				ntAssert( 0 );				break;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuText::Render
*
*	DESCRIPTION		Render our text
*
***************************************************************************************************/
bool CGuiSkinMenuText::Render()
{
	super::Render();

	if (m_pobStr)
	{
		if	( IsFading() )
		{
			CGuiSkinFader::FadeStringObject( m_pobStr, ScreenFade() );
		}

		m_pobStr->Render( );
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuText::SetTextColour
*
*	DESCRIPTION		Set render colour of the text object.
*
***************************************************************************************************/
void CGuiSkinMenuText::SetTextColour( const CVector &obColour )
{
	if (m_pobStr)
	{
		m_pobStr->SetColour( const_cast<CVector&>(obColour) );
		CGuiSkinFader::FadeStringObject( m_pobStr, ScreenFade() );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuText::UpdateTextColourForSelectability
*
*	DESCRIPTION		Set the text colour to reflect its selectable state.
*
***************************************************************************************************/
void CGuiSkinMenuText::UpdateTextColourForSelectability( void )
{
	if	( Selectable() )
	{
		SetTextColour( CGuiManager::Get().GuiSettings()->DefaultTextColour() );
	}
	else
	{
		SetTextColour( CGuiManager::Get().GuiSettings()->DefaultTextDisabledColour() );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuText::SetStateFocusIn
*
*	DESCRIPTION		Setup colour when selected in a menu select.
*
***************************************************************************************************/
void CGuiSkinMenuText::SetStateFocusIn( void )
{
	super::SetStateFocusIn();

	if	( Selectable() )
	{
		SetTextColour( CGuiManager::Get().GuiSettings()->DefaultTextSelectedColour() );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuText::SetStateFocusOut
*
*	DESCRIPTION		Setup colour when selected in a menu select.
*
***************************************************************************************************/
void CGuiSkinMenuText::SetStateFocusOut( void )
{
	super::SetStateFocusOut();

	if	( Selectable() )
	{
		SetTextColour( CGuiManager::Get().GuiSettings()->DefaultTextColour() );
	}
}

