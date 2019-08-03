/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINMENUSELECT_H_
#define _GUISKINMENUSELECT_H_

// Includes
#include "gui/guiskinselect.h"
#include "effect/screensprite.h"

// Forward Declarations

/***************************************************************************************************
*
*	CLASS			CGuiSkinMenuSelect
*
*	DESCRIPTION		Select
*
***************************************************************************************************/

class CGuiSkinMenuSelect : 	public CGuiSkinTestSelect
{
	typedef CGuiSkinTestSelect super;
public:
	CGuiSkinMenuSelect(void);
	virtual ~CGuiSkinMenuSelect(void);

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	virtual bool Render();

	virtual bool	MoveDownAction( int iPads );
	virtual bool	MoveUpAction( int iPads );

private:

	void			SyncToContents();

	void			CalcCursorPosition();

	void			LoadBackground();
	void			PositionBackground();

	void			SaveCurrentPosition();
	void			LoadCurrentPosition();

	virtual bool	Update();

	ScreenSprite m_obImage;

	float m_fCursorWidth;
	float m_fCursorHeight;
	float m_fCursorXOffset;

	ScreenSprite m_aobBackground[9];
};

#endif //_GUISKINMENUSELECT_H_
