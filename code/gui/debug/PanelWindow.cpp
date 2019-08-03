/***************************************************************************************************
*
*	DESCRIPTION		Panel windows class
*
*	NOTES			
*
***************************************************************************************************/

#include "game/pch.h"
#include ".\panelwindow.h"

#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gfx/graphicsdevice.h"
#include "gfx/texturemanager.h"

#include "core/visualdebugger.h"
#include "game/randmanager.h"

#include "gui/guimanager.h"

/***************************************************************************************************
*
*	FUNCTION		CPanelWindow::CPanelWindow
*
*	DESCRIPTION		Construction
*
*	INPUTS			pParent - Parent window. NULL indicates that this is a top level window.
*
***************************************************************************************************/

CPanelWindow::CPanelWindow(CBaseWindow* pParent)
	: CBaseWindow(pParent)
	, m_Colour(1.0f, 1.0f, 1.0f, 1.0f)
	, m_bHidePanel(false)
{
}

/***************************************************************************************************
*
*	FUNCTION		CPanelWindow::~CPanelWindow
*
*	DESCRIPTION		Destruction.
*
***************************************************************************************************/

CPanelWindow::~CPanelWindow(void)
{
}

/***************************************************************************************************
*
*	FUNCTION		CPanelWindow::OnCreate
*
*	DESCRIPTION		Creation callback. Sets up the screen panel.
*
***************************************************************************************************/

void CPanelWindow::OnCreate()
{
	CBaseWindow::OnCreate();

	OnSetBackgroundColour();
	OnExtentsChanged();

	m_Panel.SetTexture( TextureManager::CreateProcedural2DTexture(32, 32, GF_ARGB8, CVector(1.0f, 1.0f, 1.0f, 1.0f)) );
}

/***************************************************************************************************
*
*	FUNCTION		CPanelWindow::OnSetBackgroundColour
*
*	DESCRIPTION		Callback. Updates our screen colour.
*
***************************************************************************************************/

void CPanelWindow::OnSetBackgroundColour()
{
	m_Panel.SetColour( m_Colour.GetNTColor() );
}

/***************************************************************************************************
*
*	FUNCTION		CPanelWindow::OnRender
*
*	DESCRIPTION		Callback. Displays the panel
*
***************************************************************************************************/

void CPanelWindow::OnRender()
{
	CBaseWindow::OnRender();

	if (!m_bHidePanel)
		m_Panel.Render();
}

/***************************************************************************************************
*
*	FUNCTION		CPanelWindow::OnExtentsChanged
*
*	DESCRIPTION		Callback. Updates our screen object's extents.
*
***************************************************************************************************/

void CPanelWindow::OnExtentsChanged()
{
	CBaseWindow::OnExtentsChanged();

	m_Panel.SetPosition( CPoint(m_ScreenExtents.fX + m_ScreenExtents.fWidth*0.5f, m_ScreenExtents.fY + m_ScreenExtents.fHeight*0.5f, 0.0f) );
	m_Panel.SetWidth( m_ScreenExtents.fWidth );
	m_Panel.SetHeight( m_ScreenExtents.fHeight );
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::SetBackgroundColour
*
*	INPUTS			col - New window colour
*
***************************************************************************************************/

void CPanelWindow::SetBackgroundColour(const CVector& col)
{
	m_Colour = col;
	OnSetBackgroundColour();
}

/***************************************************************************************************
*
*	FUNCTION		CPanelWindow::GetBackgroundColour
*
*	RESULT			The window's colour.
*
***************************************************************************************************/

const CVector& CPanelWindow::GetBackgroundColour()
{
	return m_Colour;
}

/***************************************************************************************************
*
*	FUNCTION		CPanelWindow::HidePanel
*
*	INPUTS			bHide
*
***************************************************************************************************/

void CPanelWindow::HidePanel(bool bHide)
{
	m_bHidePanel = bHide;
}
