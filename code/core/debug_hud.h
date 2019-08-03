/***************************************************************************************************
*
* debug_hud.h
*
* A simple menu system and text/graph output system for complex debug display info
*
***************************************************************************************************/

#ifndef _DEBUG_HUD_H
#define _DEBUG_HUD_H

struct DebugHudItem
{
	enum TYPE
	{
		DHI_NONE = 0,
		DHI_TEXT,
		DHI_TEXT_CALLBACK,
		DHI_DRAW_CALLBACK,
	} eType;

	typedef bool (*Callback)( DebugHudItem* );
	typedef bool (*CallbackSelected)( DebugHudItem*, bool );
	typedef void (*CallbackText)( DebugHudItem*, char outText[256], unsigned int& outColour );
	typedef void (*CallbackDraw)( DebugHudItem*, float *fInOutX, float *fInOutY, float* pOutWidth, float* pOutHeight );

	union
	{
		const char* pText;		//!< text to display to user
		CallbackText pTextCallback; //!< called when its text needs filling in
		CallbackDraw pDrawCallback; //!< called to draw anything..
	};
	int iIndex;	//!< index to identify each item -numbers is unselectable

	union
	{
		unsigned int uiColour; // text its a colour
		void* pUserData; // for callbacks its a user pointer
	}; // user data 

	//! function to call when highlighted if null, then a simple visual highlight will be used
	Callback pHighLightedFunc; 
	//! function to call when selected, null move to sub-menu else do nothing
	CallbackSelected pSelectedFunc; 

	void* pSelectedUserUserData; // for select funcs callbacks its a user pointer

	DebugHudItem* pChildMenu;
};

class DebugHUD : public Singleton<DebugHUD>
{
public:
	DebugHUD();

	//! tell the system to use this menu list
	void UseMenu( DebugHudItem* pMenu );

	void Update();	

	void DoUserInterface();
private:
	float m_fStartX;
	float m_fStartY;

	static const int MAX_MENU_LEVELS = 4;
	int m_iSelected[MAX_MENU_LEVELS];
	int m_SelectedLevel;

	DebugHudItem* GetSelected();
	DebugHudItem* GetSelectedAtLevel( int level );
	DebugHudItem* GetSelectedFrom( DebugHudItem* pMenuItem, int iSelected );

	void SelectFirstValid( DebugHudItem* pMenuItem );
	void SelectPrev();
	void SelectNext();

	void DrawMenu(  DebugHudItem* pMenuItem, float fX, float fY, int iLevel );

	DebugHudItem* m_pStartMenuItem;
	DebugHudItem* m_pParent;
};

#endif // end _DEBUG_HUD_H
