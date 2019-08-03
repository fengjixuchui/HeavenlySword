/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINRUNSCRIPT_H_
#define _GUISKINRUNSCRIPT_H_

// Includes
#include "gui/guiunit.h"
#include "gui/guiaction.h"

// Forward Declarations


/***************************************************************************************************
*
*	CLASS			CGuiSkinRunScript
*
*	DESCRIPTION		Button with inactive and active states
*
***************************************************************************************************/

class CGuiSkinRunScript : public CGuiUnit
{
public:

	// Construction Destruction
	CGuiSkinRunScript( void );
	virtual ~CGuiSkinRunScript( void );

	// 
protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );

	virtual void	UpdateIdle( void );

	void Execute();

	CGuiAction m_obScriptAction;
};

#endif // _GUISKINRUNSCRIPT_H_
