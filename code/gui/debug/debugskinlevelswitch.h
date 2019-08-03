/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef GUIDEBUGSKINLEVELSWITCH_H
#define GUIDEBUGSKINLEVELSWITCH_H

// Includes
#include "gui/guiunitselect.h"
#include "effect/screensprite.h"
#include "gui/guitext.h"

#include "core/nt_std.h"

/***************************************************************************************************
*
*	CLASS			CGuiDebugSkinLevelSwitch
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiDebugSkinLevelSwitch : public CGuiUnit
{
public:

	// Construction Destruction
	CGuiDebugSkinLevelSwitch( void );
	virtual	~CGuiDebugSkinLevelSwitch( void );

	//bool			Update( void );
	void UpdateIdle();

	enum LOADFROM_METHOD
	{
		LEVEL_SELECT	= ( 1 << 0 ),
		DEFAULT_LEVEL	= ( 1 << 1 )
	};

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	LOADFROM_METHOD m_eLoadFrom;

	bool m_bLoadLevel;
};

#endif // GUIDEBUGSKINLEVELSWITCH_H
