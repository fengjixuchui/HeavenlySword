//-------------------------------------------------------
//!
//!	\file gui\debug\PanelWindow.h
//!	Panel class.
//!
//-------------------------------------------------------

#ifndef _PANELWINDOW_H
#define _PANELWINDOW_H

#include "gui/debug/basewindow.h"
#include "effect/screensprite.h"

//------------------------------------------------------------------------------------------
//!
//!	CPanelWindow
//!	Basic panel window. Displays a single colour panel.
//!
//------------------------------------------------------------------------------------------

class CPanelWindow : public CBaseWindow
{
public:
	// ctor
	CPanelWindow(CBaseWindow* pParent);
	// dtor.
	virtual ~CPanelWindow(void);

	// Get/Set panel colour
	void SetBackgroundColour(const CVector& col);
	const CVector& GetBackgroundColour();

	void HidePanel(bool bHide);

protected:

	// Callbacks.
	virtual void OnCreate();
	virtual void OnRender();
	virtual void OnExtentsChanged();
	virtual void OnSetBackgroundColour();

private:
	ScreenSprite m_Panel;	// Our onscreen panel
	CVector m_Colour;		// Current colour

	bool m_bHidePanel;		// Render the panel?
};

#endif //_PANELWINDOW_H







