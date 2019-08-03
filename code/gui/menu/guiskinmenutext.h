/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINMENUIMAGE_H
#define _GUISKINMENUIMAGE_H

// Includes
#include "gui/menu/guiskinmenuitem.h"
#include "gui/guitext.h"
#include "game/guiskinfader.h"

// Forward Declarations

/***************************************************************************************************
*
*	CLASS			CGuiSkinMenuText
*
*	DESCRIPTION		Static Menu Text
*
***************************************************************************************************/

class CGuiSkinMenuText : public CGuiSkinMenuItem
{
	typedef CGuiSkinMenuItem super;
	friend class CGuiSkinFader;
public:

	// Construction Destruction
	CGuiSkinMenuText( void );
	virtual ~CGuiSkinMenuText( void );

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	virtual void	SetStateFocusIn( void );
	virtual void	SetStateFocusOut( void );

	//! System
	virtual bool	Render( void );

	// Us
	bool			ProcessTitleAttribute(const char* pcText);
	bool			ProcessTitleIDAttribute(const char* pcText);
	virtual void	ActivationChange() {}

	void			CreateString();
	virtual void	CalculateExtents();

	CStringDefinition&	StringDef() { return m_obStrDef; }

	virtual void SetTextColour( const CVector &obColour );

	// Set the colour of the text depending on the selectable state.
	virtual void UpdateTextColourForSelectability( void );

private:

	union
	{
		const char* m_pcTitleID;
		const char* m_pcTitle;
	};

	CStringDefinition m_obStrDef;
	CString* m_pobStr;

	bool m_bLocalised;

	bool m_bConstructing;
};

#endif // _GUISKINMENUIMAGE_H
