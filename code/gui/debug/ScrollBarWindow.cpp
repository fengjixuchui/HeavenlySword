/***************************************************************************************************
*
*	DESCRIPTION		ScrollBar windows class
*
*	NOTES			
*
***************************************************************************************************/

#include "scrollbarwindow.h"

#include "gfx/texturemanager.h"

#include "core/visualdebugger.h"
#include "gui/guimanager.h"

/***************************************************************************************************
*
*	FUNCTION		CScrollBarWindow::CScrollBarWindow
*
*	DESCRIPTION		Construction
*
*	INPUTS			pParent - Parent window. NULL indicates that this is a top level window.
*
***************************************************************************************************/

CScrollBarWindow::CScrollBarWindow(CBaseWindow* pParent)
	: CPanelWindow(pParent)
{
	//silly defaults
	m_fTrackerWidth = 10.0f;
	m_fTrackerHeight = 10.0f;
}

/***************************************************************************************************
*
*	FUNCTION		CScrollBarWindow::~CScrollBarWindow
*
*	DESCRIPTION		Destruction.
*
***************************************************************************************************/

CScrollBarWindow::~CScrollBarWindow(void)
{
}

/***************************************************************************************************
*
*	FUNCTION		CScrollBarWindow::OnCreate
*
*	DESCRIPTION		Creation callback. Sets up the tracker.
*
***************************************************************************************************/

void CScrollBarWindow::OnCreate()
{
	CPanelWindow::OnCreate();

	UpdateTrackerSize();

	m_Tracker.SetColour( CVector(1.0f, 0.714f, 0.498f, 1.0f).GetNTColor() );
	m_Tracker.SetTexture( TextureManager::CreateProcedural2DTexture(32, 32, GF_ARGB8, CVector(1.0f, 1.0f, 1.0f, 1.0f)) );
}

/***************************************************************************************************
*
*	FUNCTION		CScrollBarWindow::OnExtentsChanged
*
*	DESCRIPTION		Callback. Update tracker extents.
*
***************************************************************************************************/

void CScrollBarWindow::OnExtentsChanged()
{
	CPanelWindow::OnExtentsChanged();

	UpdateTrackerSize();
}

/***************************************************************************************************
*
*	FUNCTION		CScrollBarWindow::OnRender
*
*	DESCRIPTION		Callback. Displays the tracker
*
***************************************************************************************************/

void CScrollBarWindow::OnRender()
{
	CPanelWindow::OnRender();

	m_Tracker.Render();
}

/***************************************************************************************************
*
*	FUNCTION		CScrollBarWindow::UpdateTrackerSize
*
*	DESCRIPTION		Recalculates tracker size. Called after resize operations
*
***************************************************************************************************/

void CScrollBarWindow::UpdateTrackerSize()
{
	m_fTrackerWidth = m_ScreenExtents.fWidth*0.8f;
	m_fTrackerHeight = 10.0f;

	m_Tracker.SetWidth( m_fTrackerWidth );
	m_Tracker.SetHeight( m_fTrackerHeight );

	SetTrackerPosition(m_fTrackerPosition);
}

/***************************************************************************************************
*
*	FUNCTION		CScrollBarWindow::SetTrackerPosition
*
*	DESCRIPTION		Set the location of the tracker along the scrollbar
*
*	INPUTS			val - Position as a normalised value (0.0-1.0)
*
***************************************************************************************************/

void CScrollBarWindow::SetTrackerPosition(float val)
{
	m_fTrackerPosition = val;

	float fX = m_ScreenExtents.fX + m_ScreenExtents.fWidth*0.5f;	//middle
	float fY = m_ScreenExtents.fY + (m_ScreenExtents.fHeight - m_fTrackerHeight)*m_fTrackerPosition + m_fTrackerHeight*0.5f;

	m_Tracker.SetPosition( CPoint( fX, fY, 0.0f) );
}
