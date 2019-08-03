/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the event unit.
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINEVENT_H
#define _GUISKINEVENT_H

// Includes
#include "gui/guiaction.h"
#include "guiunit.h"

/***************************************************************************************************
*
*	CLASS			CGuiSkinEvent
*
*	DESCRIPTION		Test skin for developing the abstract screen elements.
*
***************************************************************************************************/

class CGuiSkinEvent : public CGuiUnit
{
public:

	// Construction Destruction
	CGuiSkinEvent( void );
	virtual ~CGuiSkinEvent( void );

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	//! For setting up specific attributes

	virtual void	SetStateEnter( void );
	virtual void	SetStateIdle( void );
	virtual void	SetStateExit( void );

	virtual void	UpdateEnter( void );
	virtual void	UpdateIdle( void );
	virtual void	UpdateExit( void );

private:

	enum eEVENT_TYPE
	{
		ET_NONE,

		ET_BEGINENTER,
		ET_BEGINIDLE,
		ET_BEGINEXIT,

		ET_UPDATEENTER,
		ET_UPDATEIDLE,
		ET_UPDATEEXIT,
	};

	// Get the event enum from a event string.
	eEVENT_TYPE GetEventType( const char * pcEvent );
	
	// Perform action for the event.
	bool ProcessEvent( eEVENT_TYPE eEventType );

	CGuiAction m_obScriptAction;

	eEVENT_TYPE m_eEventType;
};

#endif // _GUISKINEVENT_H
