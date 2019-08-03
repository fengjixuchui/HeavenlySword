/***************************************************************************************************
*
*	DESCRIPTION		A screen unit that manages the user selection of button or toggle derived units
*
*	NOTES			
*
***************************************************************************************************/

#ifndef	_GUIUNITSELECT_H
#define	_GUIUNITSELECT_H

// Includes
#include "guiunit.h"

/***************************************************************************************************
*
*	CLASS			CGuiUnitSelect
*
*	DESCRIPTION		This screen unit is designed to hold a list of further screen units which may
*					selected from by the user.  For example this unit could manage a column of 
*					buttons from which a user may choose one to press, or a row of toggles from
*					which a user can set a range of options.
*
***************************************************************************************************/

class CGuiUnitSelect : public CGuiUnit
{
public:

	//! Movement Commands
	virtual bool	MoveLeftAction( int iPads );
	virtual bool	MoveRightAction( int iPads );
	virtual bool	MoveDownAction( int iPads );
	virtual bool	MoveUpAction( int iPads );

	//! Positive Actions
	virtual bool	StartAction( int iPads );
	virtual bool	SelectAction( int iPads );

	//! Negative Actions
	virtual bool	BackAction( int iPads );
	virtual bool	DeleteAction( int iPads );

	// Over ride this - for the kids
	virtual bool	BeginExit( bool bForce = false );

	virtual bool	Render( void );

protected:

	//! Construction Destruction - derive if you want one
	CGuiUnitSelect( void );
	virtual ~CGuiUnitSelect( void );

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessChild( CXMLElement* pobChild );
	virtual bool	ProcessEnd( void );

	// State specific update overrides
	virtual void	UpdateEnter( void );
	virtual void	UpdateIdle( void );
	virtual void	UpdateFocus( void );
	virtual void	UpdateFocusIn( void );
	virtual void	UpdateFocusOut( void );
	virtual void	UpdateExit( void );
	virtual void	UpdateFadeIn();
	virtual void	UpdateFadeOut();

	void			BuildPointerList();

	// item to remove. it is deleted.
	void			RemoveItem( const char* pcItem);
	
	//! Selectable unit lists
	ntstd::List< CGuiUnit* >	m_obSelectableUnits;

	//! Pointers to the selectable units in an array - easy to move around
	//! Created in ProcessEnd when all the children are collected
	CGuiUnit**			m_papobSelectableUnits;

	// What is the currently selected item
	int					m_iSelectedUnit;

	//! How are the items we are selecting between arranged
	int					m_iSelectionWidth;

	//! Do we loop if we get to the edge of the selections?
	bool				m_bLooping;

};

#endif // _GUIUNITSELECT_H
