/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINMENUITEM_H_
#define _GUISKINMENUITEM_H_

// Includes
#include "gui/guiunit.h"

// Forward Declarations


/***************************************************************************************************
*
*	CLASS			CGuiSkinMenuItem
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiSkinMenuItem: public CGuiUnit
{
	typedef CGuiUnit super;
public:

	// Construction Destruction
	CGuiSkinMenuItem( void );
	virtual ~CGuiSkinMenuItem( void );

	// Is this item selectable.
	bool			Selectable() const;

	//Extents
	void SetExtentsDirty() { m_bDirtyExtents = true; }
	void GetExtents(GuiExtents& obExtents);

	// Flags to modify button action
protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	//callbacks
	virtual void	UpdateFocusIn( void );
	virtual void	UpdateFocusOut( void );

	//callbacks to be implemented
	virtual void	ActivationChange() = 0;

	bool			Active() const	{ return m_bActive; }

	void			SetSelectable( const bool m_bSelectable );

	bool			ExtentsDirty() const { return m_bDirtyExtents; }
	virtual void	CalculateExtents() { m_bDirtyExtents = false; }

	GuiExtents m_obExtents;

private:
	bool m_bActive;
	bool m_bSelectable;
	bool m_bDirtyExtents;
};

#endif // _GUISKINMENUITEM_H_
