/***************************************************************************************************
*
*	DESCRIPTION		A usable screen unit that will control screen timeout
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINTIMER_H
#define _GUISKINTIMER_H

// Includes
#include "guiunitstatic.h"
#include "guiutil.h"
#include "gui/guiaction.h"

/***************************************************************************************************
*
*	CLASS			CGuiSkinTimer
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiSkinTimer : public CGuiUnitStatic
{
public:

	//! Construction Destruction
	CGuiSkinTimer( void );
	virtual ~CGuiSkinTimer( void ) {}

	//! We need one of these
	virtual bool	Update( void );

	//! So the timer can be reset if alowed
	bool	AnyAction( int iPads );	

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	CGuiTimer	m_obCountdownTime;

	//! Hold the initial time, so a reset can be done
	float m_fTime;

	//! To allow timer to be used on screens with multiple children
	int m_iTargetScreen;

	//! Can button presses reset me?
	bool m_bAllowReset;

	//Action - what to do next
	CGuiAction m_obAction;
};

#endif // _GUISKINTIMER_H

