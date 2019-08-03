/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINMENUTOGGLE_H_
#define _GUISKINMENUTOGGLE_H_

// Includes
#include "gui/menu/guiskinmenutext.h"
#include "game/guiskinfader.h"

// Forward Declarations


/***************************************************************************************************
*
*	CLASS			CGuiSkinMenuToggle
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiSkinMenuToggle : public CGuiSkinMenuText
{
	typedef CGuiSkinMenuText super;
	friend class CGuiSkinFader;
public:

	// Construction Destruction
	CGuiSkinMenuToggle( void );
	virtual ~CGuiSkinMenuToggle( void );

	// Flags to modify button action

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	//system
	virtual bool	MoveLeftAction( int iPads );
	virtual bool	MoveRightAction( int iPads );
	virtual bool	StartAction( int iPads );
	virtual bool	SelectAction( int iPads );

	virtual void	GetExtents(GuiExtents& obExtents);

	// us
	virtual bool	Render();

	void			Toggle();

	void	ProcessValueBasePositionValue( const char* pcValue );

	virtual void SetTextColour( const CVector &obColour );


private:
	const char*			m_pcOnString;
	const char*			m_pcOffString;
	const char*			m_pcCallback;

	bool				m_bOn;

	void				CreateValueString();
	CString*			m_pobValueString;
	CString*			m_pOtherString;

	float				m_fValuePositionX;
	float				m_fGapBetweenText;

	GuiExtents			m_obExtents;
};

#endif // _GUISKINMENUTOGGLE_H_
