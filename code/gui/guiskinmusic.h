/***************************************************************************************************
*
*	DESCRIPTION		A usable screen unit that sontains video playback functionality
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINMUSIC_H
#define _GUISKINMUSIC_H

// Includes
#include "guiunit.h"

/***************************************************************************************************
*
*	CLASS			CGuiSkinMusic
*
*	DESCRIPTION		Test skin for developing the functionality of the abstract screen elements.
*
***************************************************************************************************/

class CGuiSkinMusic : public CGuiUnit
{
public:

	// Construction Destruction
	CGuiSkinMusic( void );
	virtual ~CGuiSkinMusic( void );

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	// State Setting code
	virtual void	SetStateIdle( void );
	virtual void	SetStateDead( void );

	int m_iMusicType;

	static int s_iUseCount;	// Count our uses so that we don't restart if multiple screens have a music item
};

#endif // _GUISKINMUSIC_H
