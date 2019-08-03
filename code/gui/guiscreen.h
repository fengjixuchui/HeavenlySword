/***************************************************************************************************
*
*	DESCRIPTION		Describes the 'pages' of the GUI
*
*	NOTES			
*
***************************************************************************************************/

#ifndef	_GUISCREEN_H
#define	_GUISCREEN_H

// Includes
#include "guiunit.h"
#include "lua/ninjalua.h"

// Forward declarations
class CGuiUnit;
class CScreenHeader;

/***************************************************************************************************
*
*	CLASS			CGuiScreen
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiScreen : public CGuiUnit
{
public:

	//! Construction Destruction
	CGuiScreen( void );
	~CGuiScreen( void );

	//! Movement Commands
	virtual bool	MoveLeftAction( int iPads );
	virtual bool	MoveRightAction( int iPads );
	virtual bool	MoveDownAction( int iPads );
	virtual bool	MoveUpAction( int iPads );

	//! Positive Actions
	virtual bool	StartAction( int iPads );
	virtual bool	SelectAction( int iPads );

	//! Negative Actions
	virtual bool	BackAction( int iPads );
	virtual bool	DeleteAction( int iPads );

	//! Indiscriminate Actions 
	virtual bool	AnyAction( int iPads );


	// Over ride this - for the kids
	virtual bool	BeginExit( bool bForce = false );

	virtual bool	Render( void );

	void			SetScreenFlags( int iFlags ) { m_iScreenFlags = iFlags; }; 

	bool			AddGuiElement( CGuiUnit* pobChild );
	bool			RemoveGuiElement( CGuiUnit* pobChild );

	void			ProcessInput();
	bool			NewStyleInput() const	{ return m_iAcceptsInput != 0; }

	CScreenHeader*	GetScreenHeader()	{ return m_pobScreenHeader; }
	void			SetScreenHeader(CScreenHeader* pobHdr)	{ m_pobScreenHeader = pobHdr; }

	static CScreenHeader* ms_pobCurrentScreenHeader;
	static CGuiScreen* ms_pobCurrentScreen;

	//! Does this screen require the game to be updated?
	bool			GameUpdateAllowed() const	{ return m_bUpdateGame; }

	virtual float	ScreenFade() { return m_fScreenFade; }
	virtual bool	IsFading() const { return m_bFadeRunning; }

//	void BeginFadeIn(bool bReset);
//	void BeginFadeOut(bool bReset);
	void UpdateFade();

	bool			ShowFilter() const { return m_bShowFilter; }

	void			ContinueFade() { m_bHoldFadeIn = false; }

//	bool			CrossFadeEntering() const { return m_bCrossFadeEntering; }
	bool			CrossFadeExiting() const { return m_bCrossFadeExiting; }

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessChild( CXMLElement* pobChild );
	virtual bool	ProcessEnd( void );

	// State specific update overrides
	virtual void	UpdateEnter( void );
	virtual void	UpdateIdle( void );
	virtual void	UpdateFocus( void );
	virtual void	UpdateFocusIn( void );
	virtual void	UpdateFocusOut( void );
	virtual void	UpdateExit( void );

	virtual void	UpdateFadeIn( void );
	virtual void	UpdateFadeOut( void );

	//State
	virtual void	SetStateFadeIn( void );
	virtual void	SetStateFadeOut( void );


	//! Debug activities
	void DebugPadAction( const char* pcAction, int iPads );

	//! Screen Unit List
	ntstd::List< CGuiUnit* >	m_obScreenUnits;

	int m_iScreenFlags;
	int m_iAcceptsInput;

	CScreenHeader* m_pobScreenHeader;

	bool m_bUpdateGame;

	float m_fScreenFadeDestination;

	bool m_bShowFilter;
	bool m_bHoldFadeIn;

//	bool m_bCrossFadeEntering;	/// Previous screen to this screen should overlap
	bool m_bCrossFadeExiting;	/// This screen to next screen should overlap

public:
	HAS_LUA_INTERFACE();
};

LV_DECLARE_USERDATA(CGuiScreen);

#endif // _GUISCREEN_H
