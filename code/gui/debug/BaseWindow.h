//-------------------------------------------------------
//!
//!	\file gui\debug\BaseWindow.h
//!	Baseclass for all debug windows.
//!
//-------------------------------------------------------

#ifndef _BASEWINDOW_H
#define _BASEWINDOW_H

#include "core/nt_std.h"

//extents are relative to the parent
struct WindowRect
{
	float fX;
	float fY;
	float fWidth;
	float fHeight;
};

//------------------------------------------------------------------------------------------
//!
//!	CBaseWindow
//!	Base class for all 'debug' windows. It has some logic to manage its children.
//!
//------------------------------------------------------------------------------------------

class CBaseWindow
{
public:
	// ctor
	CBaseWindow(CBaseWindow* pParent);
	// dtor. destroys it's children.
	virtual ~CBaseWindow(void);

	// Creates window. Should be called once all properties have been set.
	void Create();

	// Update, also updates children. To be called once per frame.
	void Update();
	// Render, also renders children. Intended to be called once per frame.
	void Render();

	// Set window extents. The values represent percentages of parent extents.
	void SetExtents(const WindowRect& rect);

	// Get/Set Window width (percent of parent width)
	void SetWidth(float w);
	float GetWidth();

	// Get/Set Window height (percent of parent height)
	void SetHeight(float h);
	float GetHeight();

	// Attach a child to be managed by this window.
	void AttachChild(CBaseWindow* pChild);

protected:

	// Callbacks. They are called during the correspondingly named methods above.
	virtual void OnCreate() {};
	virtual void OnUpdate() {};
	virtual void OnRender() {};
	virtual void OnExtentsChanged() {};

	// Get screen extents (actual screen coords)
	const WindowRect& GetScreenExtents();
	// Calcuate our screen extents and those of our children
	void UpdateScreenExtents();

	WindowRect m_Extents;							// Normalised extents (% of parent window)
	WindowRect m_ScreenExtents;						// Screen coords

	CBaseWindow* m_pParent;							// Parent window

	typedef ntstd::List<CBaseWindow*> WindowList;	
	WindowList m_Children;							// Our child windows
};

#endif //_BASEWINDOW_H





