/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINCHECKPOINTSELECT_H
#define _GUISKINCHECKPOINTSELECT_H

// Includes
#include "gui/guiunit.h"
#include "gui/guiaction.h"

// Forward Declarations

/***************************************************************************************************
*
*	CLASS			CGuiSkinCheckpointSelect
*
*	DESCRIPTION		Represents the checkpoint selection component on the chapter select screen.
*
***************************************************************************************************/

class CGuiSkinCheckpointSelect : public CGuiUnit
{
	typedef CGuiUnit super;
public:
	
	// Construction Destruction
	CGuiSkinCheckpointSelect( void );
	virtual ~CGuiSkinCheckpointSelect( void );

		//! Movement Commands
	virtual bool	MoveLeftAction( int iPads );
	virtual bool	MoveRightAction( int iPads );
	virtual bool	MoveDownAction( int iPads );
	virtual bool	MoveUpAction( int iPads );

	//! Positive Actions
	virtual bool	StartAction( int iPads );
	virtual bool	SelectAction( int iPads );

	//! Negative Actions
	virtual bool	BackAction( int iPads ) { UNUSED(iPads); return true; }
	virtual bool	DeleteAction( int iPads ) { UNUSED(iPads); return true; }

	// called by the frontend when a chapter chapter change occurs
	void			OnChapterSwitchBegin();
	void			OnChapterSwitchEnd();
protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessChild( CXMLElement* pobChild );
	virtual bool	ProcessEnd( void );

	// main
	virtual bool	Update( void );
	virtual bool	Render( void );

	void			UpdateCheckpointInfo();

private:
	//! Control buttons
	//ntstd::Vector< CGuiUnit* > m_obPagingArrows;

	CGuiAction m_obOnSelectAction;
	bool m_bFirstUpdate;
};

#endif // _GUISKINCHECKPOINTSELECT_H
