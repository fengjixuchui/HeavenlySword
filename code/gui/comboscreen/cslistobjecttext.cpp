/***************************************************************************************************
*
*	DESCRIPTION		The provides an interface for objects that are to appear in the list
*
*	NOTES			
*
***************************************************************************************************/

#include "cslistobjecttext.h"
#include "gui/guisettings.h"

/***************************************************************************************************
*
*	FUNCTION		CSListObjectText::CSListObjectText
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CSListObjectText::CSListObjectText( void )
: m_pStr(NULL)
{
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectText::~CSListObjectText
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CSListObjectText::~CSListObjectText( void )
{
	if( NULL != m_pStr )
	{
		CStringManager::Get().DestroyString( m_pStr );
		m_pStr = NULL;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectText::Render
*
*	DESCRIPTION		This draws the text.
*
***************************************************************************************************/

void CSListObjectText::Render( void )
{
	if( NULL != m_pStr )
	{
		m_pStr->Render();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectText::Update
*
*	DESCRIPTION		This repositions the text if need be.
*
***************************************************************************************************/

void CSListObjectText::Update( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectText::SetTextString
*
*	DESCRIPTION		This draws the text.
*
***************************************************************************************************/

void CSListObjectText::SetTextString( const char* cDisplayText, const CStringDefinition::STRING_JUSTIFY_HORIZONTAL eHJustify, const CStringDefinition::STRING_JUSTIFY_VERTICAL eVJustify )
{
	if( NULL != m_pStr )
	{
		CStringManager::Get().DestroyString( m_pStr );
		m_pStr = NULL;
	}

	CStringDefinition StrDef;
	StrDef.m_fBoundsHeight = m_fHeight;
	StrDef.m_fBoundsWidth = m_fWidth;
	StrDef.m_fXOffset = m_fX;
	StrDef.m_fYOffset = m_fY;
	StrDef.m_fZOffset = 0.0f;
	const char* pcFont = CGuiManager::Get().GuiSettings()->BodyFont();
	StrDef.m_pobFont = CFontManager::Get().GetFont(pcFont);
	StrDef.m_eHJustify = eHJustify;//CStringDefinition::JUSTIFY_LEFT;
	StrDef.m_eVJustify = eVJustify;//CStringDefinition::JUSTIFY_MIDDLE;

	m_pStr = CStringManager::Get().MakeString( cDisplayText, StrDef, m_pOffsetTransform, CGuiUnit::RENDER_SCREENSPACE );
	ntAssert( NULL != m_pStr );

	m_fWidth = m_pStr->RenderWidth();
	m_fHeight = m_pStr->RenderHeight();
}
