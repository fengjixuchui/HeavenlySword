/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskinmenuitem.h"
#include "gui/guimanager.h"
#include "gui/guitext.h"

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuItem::CGuiSkinMenuItem
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinMenuItem::CGuiSkinMenuItem( void )
:	m_bActive(false)
,	m_bSelectable( true )
{}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuItem::~CGuiSkinMenuItem
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinMenuItem::~CGuiSkinMenuItem( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuItem::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinMenuItem::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "type" ) == 0 )
		{
			ntPrintf("Found TYPE in gui Element. please remove :)\n");
			return true;
		}
		else if ( strcmp( pcTitle, "selectable" ) == 0 )
		{
			GuiUtil::SetBool(pcValue, &m_bSelectable);			
			return true;
		}
		return false;
	}

	if ( strcmp( pcTitle, "moveposition" ) == 0 )
	{
		SetExtentsDirty();
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuItem::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinMenuItem::ProcessEnd( void )
{
	// Call the base first
	return super::ProcessEnd();
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuItem::UpdateFocusIn
*
*	DESCRIPTION		Gained focus
*
***************************************************************************************************/

void CGuiSkinMenuItem::UpdateFocusIn( void )
{
	super::UpdateFocusIn();

	m_bActive = true;
	ActivationChange();
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuItem::UpdateFocusOut
*
*	DESCRIPTION		Lost focus
*
***************************************************************************************************/

void CGuiSkinMenuItem::UpdateFocusOut( void )
{
	super::UpdateFocusOut();

	m_bActive = false;
	ActivationChange();
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuItem::RenderMenuText
*
*	DESCRIPTION		Wraps text with selection indicator
*
***************************************************************************************************/
/*
void CGuiSkinMenuItem::RenderMenuText(const char* pcString)
{
	CStringDefinition obStrDef;
	obStrDef.m_pobFont = CFontManager::Get().GetFont("Alan Den");

	// Set the vertical offset of our image
	switch ( m_eVerticalJustification )
	{
		case JUSTIFY_TOP:		obStrDef.m_eVJustify = CStringDefinition::JUSTIFY_TOP;		break;
		case JUSTIFY_MIDDLE:	obStrDef.m_eVJustify = CStringDefinition::JUSTIFY_MIDDLE;	break;
		case JUSTIFY_BOTTOM:	obStrDef.m_eVJustify = CStringDefinition::JUSTIFY_BOTTOM;	break;
		default:				ntAssert( 0 );				break;
	}

	// Set the horizontal offset
	switch ( m_eHorizontalJustification )
	{
		case JUSTIFY_LEFT:		obStrDef.m_eHJustify = CStringDefinition::JUSTIFY_LEFT;		break;
		case JUSTIFY_CENTRE:	obStrDef.m_eHJustify = CStringDefinition::JUSTIFY_CENTRE;	break;
		case JUSTIFY_RIGHT:		obStrDef.m_eHJustify = CStringDefinition::JUSTIFY_RIGHT;	break;
		default:				ntAssert( 0 );				break;
	}

	const char* pcFormat = Active() ? "-%s-" : "%s";

	char cBuf[128] = {0};
	int len = sprintf(cBuf, pcFormat, pcString);

	WCHAR_T wcBuf[128] = {0};
	mbstowcs(wcBuf, cBuf, len);

	CString* pStr = CStringManager::Get().MakeString(wcBuf, obStrDef, m_pobBaseTransform, m_eRenderSpace);
	pStr->Render( );
	CStringManager::Get().DestroyString(pStr);
}*/

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuItem::Selectable
*
*	DESCRIPTION		Returns true if the item is selectable.
*
***************************************************************************************************/
bool CGuiSkinMenuItem::Selectable() const
{
	return m_bSelectable;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuItem::SetSelectable
*
*	DESCRIPTION		Set the selectable state of the menu item.
*
***************************************************************************************************/
void CGuiSkinMenuItem::SetSelectable( const bool bSelectable )
{
	m_bSelectable = bSelectable;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuItem::GetExtents
*
*	DESCRIPTION		Gets the extents of this screen object. Extents are recalculated if required.
*
***************************************************************************************************/
void CGuiSkinMenuItem::GetExtents(GuiExtents& obExtents)
{
	if ( m_bDirtyExtents )
		CalculateExtents();

	obExtents = m_obExtents;
}
