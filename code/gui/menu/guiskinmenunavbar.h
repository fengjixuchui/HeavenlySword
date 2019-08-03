/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINMENUNAVBAR_H_
#define _GUISKINMENUNAVBAR_H_

// Includes
#include "gui/guiunit.h"
#include "gui/guitext.h"
#include "effect/screensprite.h"

// Forward Declarations

//#define DEBUG_NAV_FADE

/***************************************************************************************************
*
*	CLASS			GuiNavigationBar
*
*	DESCRIPTION		
*
***************************************************************************************************/

class GuiNavigationBar
{
public:

	enum SLOT
	{
		SLOT_SELECT,
		SLOT_BACK,
		SLOT_DESCRIPTION,
		NUM_SLOTS
	};

	// Construction Destruction
	GuiNavigationBar( void );
	~GuiNavigationBar( void );

	void ShowScriptOverlay(bool bShow);
	void ShowTextSlot(SLOT eSlot, bool bShow, const char* pcID = NULL);

	void Render();
	void Update();

	void NotifyFade();

protected:

	void CreateString( SLOT eSlot, const char* pcID );
	void DestroyString( SLOT eSlot );

	void CreateOverlay();

	void CreateTransform();
	void DestroyTransform();

	bool m_abShow[NUM_SLOTS];
	CStringDefinition m_aobStringDef[NUM_SLOTS];
	CString* m_apobString[NUM_SLOTS];

	bool m_bShowScriptOverlay;

	//overlay along the bottom
	ScreenSprite m_aobScriptOverlay[2];

	//position
	Transform* m_pobTransform;

	float m_afFadeTimers[NUM_SLOTS];
	bool m_abFadeWaiting[NUM_SLOTS];
	bool m_abFadeDir[NUM_SLOTS];	// true = in, false == out :/

};

/***************************************************************************************************
*
*	CLASS			CGuiSkinMenuNavigationBar
*
*	DESCRIPTION		The XML 'frontend' to the navbar object
*
***************************************************************************************************/

class CGuiSkinMenuNavigationBar: public CGuiUnit
{
	typedef CGuiUnit super;

public:

	// Construction Destruction
	CGuiSkinMenuNavigationBar( void );
	virtual ~CGuiSkinMenuNavigationBar( void );

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	virtual void	UpdateFadeIn();
	virtual void	UpdateFadeOut();

	virtual bool	SetData(const char* pcName, void* pvData);

	void			SyncAttributes();

	bool m_abShow[GuiNavigationBar::NUM_SLOTS];
	const char* m_pcDescTitleID;
	bool m_bShowScriptOverlay;

	bool m_bNotifiedFadeIn;
	bool m_bNotifiedFadeOut;
};

#endif // _GUISKINMENUNAVBAR_H_
