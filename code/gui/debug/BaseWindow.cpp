/***************************************************************************************************
*
*	DESCRIPTION		Base windows class
*
*	NOTES			
*
***************************************************************************************************/

#include "basewindow.h"
#include "gui/guimanager.h"

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::CBaseWindow
*
*	DESCRIPTION		Construction
*
*	INPUTS			pParent - Parent window. NULL indicates that this is a top level window.
*
***************************************************************************************************/

CBaseWindow::CBaseWindow( CBaseWindow* pParent )
	: m_pParent(pParent)
{
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::~CBaseWindow
*
*	DESCRIPTION		Destruction. Deletes child windows.
*
***************************************************************************************************/

CBaseWindow::~CBaseWindow( void )
{
	// iterate over children and free them
	for (WindowList::iterator it = m_Children.begin(); it != m_Children.end(); ++it)
	{
		NT_DELETE( (*it) );
	}

	m_Children.clear();
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::UpdateScreenExtents
*
*	DESCRIPTION		Calculates screen extents. Also updates extents of child windows.
*
*	NOTES			If we are a top level window, our parent is considered to be the backbuffer.
*					The 'OnExtentsChanged' callback is triggered from here
*
***************************************************************************************************/

void CBaseWindow::UpdateScreenExtents()
{
	WindowRect parentExtents;

	//determine our parent extents
	if (m_pParent)
	{
		parentExtents = m_pParent->GetScreenExtents();
	}
	else
	{
		parentExtents.fX = parentExtents.fY = 0.0f;
		parentExtents.fWidth = CGuiManager::Get().BBWidth();
		parentExtents.fHeight = CGuiManager::Get().BBHeight();
	}

	//calculate our screen extents
	m_ScreenExtents.fX = parentExtents.fX + m_Extents.fX * parentExtents.fWidth;
	m_ScreenExtents.fY = parentExtents.fY + m_Extents.fY * parentExtents.fHeight;
	m_ScreenExtents.fWidth = parentExtents.fWidth * m_Extents.fWidth;
	m_ScreenExtents.fHeight = parentExtents.fHeight * m_Extents.fHeight;

	//trigger callback
	OnExtentsChanged();

	//Update children
	for (WindowList::iterator it = m_Children.begin(); it != m_Children.end(); ++it)
	{
		(*it)->UpdateScreenExtents();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::Create
*
*	DESCRIPTION		Contructs the window.
*
*	NOTES			The 'OnCreate' callback is triggered from here
*
***************************************************************************************************/

void CBaseWindow::Create()
{
	OnCreate();
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::Update
*
*	DESCRIPTION		Updates the window and its children.
*
*	NOTES			The 'OnUpdate' callback is triggered from here
*
***************************************************************************************************/

void CBaseWindow::Update( void )
{
	OnUpdate();

	for (WindowList::iterator it = m_Children.begin(); it != m_Children.end(); ++it)
	{
		(*it)->Update();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::Render
*
*	DESCRIPTION		Renders the window and its children.
*
*	NOTES			The 'OnRender' callback is triggered from here
*
***************************************************************************************************/

void CBaseWindow::Render( void )
{
	OnRender();

	for (WindowList::iterator it = m_Children.begin(); it != m_Children.end(); ++it)
	{
		(*it)->Render();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::GetScreenExtents
*
*	RESULT			The window's screen extents.
*
***************************************************************************************************/

const WindowRect& CBaseWindow::GetScreenExtents()
{
	return m_ScreenExtents;
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::SetExtents
*
*	INPUTS			rect - The window's new normalised screen extents.
*
***************************************************************************************************/

void CBaseWindow::SetExtents(const WindowRect& rect)
{
	m_Extents = rect;
	UpdateScreenExtents();
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::SetWidth
*
*	INPUTS			w - New window width (normailised)
*
***************************************************************************************************/

void CBaseWindow::SetWidth(float w)
{
	m_Extents.fWidth = w;
	UpdateScreenExtents();
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::GetWidth
*
*	RESULT			The window's width (normalised).
*
***************************************************************************************************/

float CBaseWindow::GetWidth()
{
	return 	m_Extents.fWidth;
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::SetHeight
*
*	INPUTS			h - New window height (normailised)
*
***************************************************************************************************/

void CBaseWindow::SetHeight(float h)
{
	m_Extents.fHeight = h;
	UpdateScreenExtents();
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::GetHeight
*
*	RESULT			The window's height (normalised).
*
***************************************************************************************************/

float CBaseWindow::GetHeight()
{
	return m_Extents.fHeight;
}

/***************************************************************************************************
*
*	FUNCTION		CBaseWindow::AttachChild
*
*	INPUTS			pChild - Child window to be attached.
*
***************************************************************************************************/

void CBaseWindow::AttachChild(CBaseWindow* pChild)
{
	m_Children.push_back(pChild);
}
