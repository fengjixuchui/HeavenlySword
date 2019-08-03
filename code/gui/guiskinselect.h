/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface select unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

#ifndef _GUISKINSELECT_H
#define _GUISKINSELECT_H

// Includes
#include "guiunitselect.h"

// Forward Declarations

/***************************************************************************************************
*
*	CLASS			CGuiSkinTestSelect
*
*	DESCRIPTION		Test skin for developing the functionality of the abstract screen elements.
*
***************************************************************************************************/

class CGuiSkinTestSelect : public CGuiUnitSelect
{
public:
	
	// Construction Destruction
	CGuiSkinTestSelect( void );
	virtual ~CGuiSkinTestSelect( void );

		//! Movement Commands
	//virtual bool	MoveLeftAction( int iPads );
	//virtual bool	MoveRightAction( int iPads );
	virtual bool	MoveDownAction( int iPads );
	virtual bool	MoveUpAction( int iPads );

	//! Positive Actions
	virtual bool	StartAction( int iPads );
	virtual bool	SelectAction( int iPads );

	//! Negative Actions
	virtual bool	BackAction( int iPads );
	virtual bool	DeleteAction( int iPads );

protected:

};

#endif // _GUISKINSELECT_H
