//-------------------------------------------------------
//!
//!	\file gui\debug\ScrollBarWindow.h
//!	Scrollbar class.
//!
//-------------------------------------------------------

#ifndef _SCROLLBARWINDOW_H
#define _SCROLLBARWINDOW_H

#include "gui/debug/panelwindow.h"

//------------------------------------------------------------------------------------------
//!
//!	CScrollBarWindow
//!	Scrollbar...
//!
//------------------------------------------------------------------------------------------
class CScrollBarWindow : public CPanelWindow
{
public:
	// ctor
	CScrollBarWindow(CBaseWindow* pParent);
	// dtor
	virtual ~CScrollBarWindow(void);

	// Set the trackers position along the scrollbar
	void SetTrackerPosition(float val);

private:
	// Callbacks
	virtual void OnCreate();
	virtual void OnRender();
	virtual void OnExtentsChanged();

	// Resize tracker (tab on scrollbar :)
	void UpdateTrackerSize();

	float m_fTrackerPosition;	// Trackers position along the scrollbar (0.0-1.0)
	float m_fTrackerWidth;		// 
	float m_fTrackerHeight;		//

	ScreenSprite m_Tracker;		// On screen rep of tracker.
};

#endif //_SCROLLBARWINDOW_H
