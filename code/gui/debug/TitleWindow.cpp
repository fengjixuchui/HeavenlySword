/***************************************************************************************************
*
*	DESCRIPTION		Title window class
*
*	NOTES			
*
***************************************************************************************************/

#include "titlewindow.h"

#include "gui/guiutil.h"

#include "anim/hierarchy.h"

#include "core/visualdebugger.h"

/***************************************************************************************************
*
*	FUNCTION		CTitleWindow::CTitleWindow
*
*	DESCRIPTION		Construction
*
*	INPUTS			pParent - Parent window. NULL indicates that this is a top level window.
*
***************************************************************************************************/

CTitleWindow::CTitleWindow(CBaseWindow* pParent)
	: CPanelWindow(pParent)
	, m_szTitle(0)
{
	m_obStrDef.m_pobFont = CFontManager::Get().GetFont("Comic");//CStringManager::Get().GetGeneralLanguage()->GetFont("Comic");

	m_obStrDef.m_eVJustify = CStringDefinition::JUSTIFY_TOP;
	m_obStrDef.m_eHJustify = CStringDefinition::JUSTIFY_LEFT;
}

/***************************************************************************************************
*
*	FUNCTION		CTitleWindow::~CTitleWindow
*
*	DESCRIPTION		Destruction.
*
***************************************************************************************************/

CTitleWindow::~CTitleWindow(void)
{
	FreeString();
}

/***************************************************************************************************
*
*	FUNCTION		CTitleWindow::SetTitle
*
*	DESCRIPTION		Set the text string
*
*	INPUTS			title - new string
*
***************************************************************************************************/

void CTitleWindow::SetTitle(const char* title)
{
	FreeString();

	int length = strlen(title);
	m_szTitle = NT_NEW_ARRAY char[length+1];

	strcpy(m_szTitle, title);
	m_szTitle[length] = 0;
}

/***************************************************************************************************
*
*	FUNCTION		CTitleWindow::FreeString
*
*	DESCRIPTION		Frees the internal string.
*
***************************************************************************************************/

void CTitleWindow::FreeString()
{
	if (m_szTitle)
	{
		NT_DELETE_ARRAY(m_szTitle);
		m_szTitle = 0;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CTitleWindow::OnRender
*
*	DESCRIPTION		Callback. Displays the text string
*
***************************************************************************************************/

void CTitleWindow::OnRender()
{
	CPanelWindow::OnRender();

	float fTextHeight = 15.0f;	//guessing

	float fX = m_ScreenExtents.fX + 10.0f;
	float fY = m_ScreenExtents.fY + m_ScreenExtents.fHeight*0.5f - fTextHeight*0.5f;

	GuiUtil::RenderString(fX, fY, &m_obStrDef, m_szTitle);

//	g_VisualDebug->Printf2D(fX, fY, DC_BLUE, 0, m_szTitle);
}
