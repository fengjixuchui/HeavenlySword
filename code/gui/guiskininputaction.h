/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKININPUTACTION_H_
#define _GUISKININPUTACTION_H_

// Includes
#include "gui/guiunit.h"
#include "gui/guiaction.h"

// Forward Declarations


/***************************************************************************************************
*
*	CLASS			CGuiSkinInputAction
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiSkinInputAction : public CGuiUnit
{
public:

	// Construction Destruction
	CGuiSkinInputAction( void );
	virtual ~CGuiSkinInputAction( void );

	// 

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

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

	//! Indiscriminate Actions 
	virtual bool	AnyAction( int iPads );

	bool Action();

	CGuiAction m_obAction;
	int m_iAcceptsInput;
};

#endif // _GUISKININPUTACTION_H_
