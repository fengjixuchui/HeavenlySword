//-------------------------------------------------------
//!
//!	\file gui\debug\TitleWindow.h
//!	Titlebar class.
//!
//-------------------------------------------------------

#ifndef _TITLEWINDOW_H
#define _TITLEWINDOW_H

#include "gui/debug/panelwindow.h"

#include "gui/guitext.h"

//------------------------------------------------------------------------------------------
//!
//!	CTitleWindow
//!	Title window. Displays a text string.
//!
//------------------------------------------------------------------------------------------
class CTitleWindow : public CPanelWindow
{
public:
	// ctor
	CTitleWindow(CBaseWindow* pParent);
	// dtor
	virtual ~CTitleWindow(void);
	
	// Set the text on this window
	void SetTitle(const char* title);

private:

	// Callbacks
	virtual void OnRender();

	// String management
	void FreeString();

	CStringDefinition m_obStrDef;


	char* m_szTitle;		// Window text string
};

#endif //_TITLEWINDOW_H
