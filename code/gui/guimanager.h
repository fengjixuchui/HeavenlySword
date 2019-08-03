/***************************************************************************************************
*
*	DESCRIPTION		The main manager of the flow of the game
*
*	NOTES			Looks after front end screens and the management of levels
*
***************************************************************************************************/

#ifndef	_GUIMANAGER_H
#define	_GUIMANAGER_H

// Includes
#include "gfx/camera.h"
#include "gfx/listspace.h"
#include "anim/transform.h"
#include "gui/guiutil.h"
#include "gui/guiunit.h"
#include "effect/screensprite.h"
#include "core/nt_std.h"

// Forward declarations
class CGuiScreen;
class CXMLElement;
class CScreenHeader;
class CamView;
class GuiSettingsDef;
class GuiNavigationBar;

struct CS_MOVE_INFO_PARENT;

// For debugging GUI behaviour
//#define DEBUG_GUI_INPUT
//#define DEBUG_GUI

/***************************************************************************************************
*
*	CLASS			CGuiManager
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiManager : public Singleton<CGuiManager>
{
public:

	//! Construction Destruction
	CGuiManager( void );
	~CGuiManager( void );

	// This is a fake call that show what needs to be done to unload and reload a level
	static void	LoadGameLevel_Name( const char* pcLevelName, int nChapterNumber, int nCheckpointID );
	static void	LoadGameLevel_Chapter( int nChapterNumber, int nCheckpointID );

	static void	LoadDefaultGameLevel();
	static void	UnloadGameLevel();

	//! To move us on when the E3 demo is complete
	void			OnComplete( void );

	//! Go
	void	Update( void );
	void	Render( void );

	//! Transform stuff for camera relative elements
	void			UpdateTransform( void );
	void			RemoveTransform( void );
	Transform*		GetCamTransform (void) { return m_pobCamTransform; };

	//! To register items which can be built by this manager
	static bool		Register( const char* pcTagName, CXMLElement* ( *ContructionWrapper ) ( void ) );

	//! To create items from their XML tag 
	CXMLElement*	CreateGuiElement( const char* pcTag );

	//! TO BE MOVED TO PROTECTED WHEN THERE IS A MESSAGE SYSTEM
	bool			MoveOnScreen( int iOption = 0 );
	bool			MoveBackScreen( int iScreensBack = 0 );

	bool			MoveOnScreenType( int iScreenFlag );
	bool			MoveBackScreenType( int iScreenFlag );

	bool			MoveOnScreenGroup( const char* pcGroup );
	bool			MoveBackScreenGroup( const char* pcGroup );

	bool			SkipForwardScreen( int iCount, bool bUsePath... );	//count and path to take

	float			BBWidth(void) { return m_fBBWidth; };
	float			BBHeight(void) { return m_fBBHeight; };

	void			SetUserPaused( bool bPaused );

	bool		IsActive( void );

	bool			AddGuiElement(CGuiUnit* pobNewElement);
	bool			RemoveGuiElement(CGuiUnit* pobElement);

	static int		ms_iLevelsLoaded;	// number of levels loaded so far

	bool			ResetLevel( const char* pcLevelName = 0, int nCheckpointID = 0 );

	ntstd::List< CS_MOVE_INFO_PARENT* >*	GetComboScreenMoveInfoBasicList( void );

	ntstd::List< CS_MOVE_INFO_PARENT* >*	GetComboScreenMoveInfoSpeedList( void );

	ntstd::List< CS_MOVE_INFO_PARENT* >*	GetComboScreenMoveInfoRangeList( void );

	ntstd::List< CS_MOVE_INFO_PARENT* >*	GetComboScreenMoveInfoPowerList( void );
	ntstd::List< CS_MOVE_INFO_PARENT* >*	GetComboScreenMoveInfoAerialList( void );

	ntstd::List< CS_MOVE_INFO_PARENT* >*	GetComboScreenMoveInfoSuperStyleList( void );

	const GuiSettingsDef* GuiSettings() const { return m_pobGuiSettings; }
	GuiNavigationBar* NavigationBar() { return m_pobNavBar; }

protected:

	void			UpdateFilter();
	
	float m_fFilterTimer;
	ScreenSprite m_obFilter;

	//! Get Ready
	void	Initialise( void ); // (moved on 30/01/04 by WD, as doesnt need to be called externally anymore)	

	//! Possible screen elements are registered on startup.
	//! We store pointers to constructor wrappers here incase
	//! we fancy using one of them.
	struct TAG_REGISTER
	{
		const char* pcTag;
		CXMLElement* ( *ContructionWrapper ) ( void );
	};

	//! We link the screens themselves with the screenheader information
	//! which is imported in a seperate file describing the whole game flow
	struct SCREEN_INFO
	{
		CGuiScreen*		pobScreen;
		CScreenHeader*	pobScreenHeader;
	};
	
	//! Utility helpers
	SCREEN_INFO*	CreateScreen( CScreenHeader* pobScreenHeader );
	void			DestroyScreen( SCREEN_INFO* pstrScreen );
	bool			MoveScreen(CScreenHeader* pobNewScreen);

	bool			RequestMoveScreen(CScreenHeader* pobNewScreen);

	void			ProcessInput();
	void			PostCreateScreen();

	//! So we need an array of tags and something to count them with
	static int m_iCurrentRegisteredTagIndex;
	static const int MAX_REGISTERED_TAGS = 40;
	static TAG_REGISTER m_astRegisteredTags[ MAX_REGISTERED_TAGS ];

	//! The Screens
	SCREEN_INFO* m_pobPrimaryScreen;	// The currently active (or becoming active) screen
	SCREEN_INFO* m_pobSecondaryScreen;	// The previously primary screen. remains valid while it fades out (if we are cross fading)
	//! Screens that are waiting to activate
	ntstd::List< CScreenHeader* >	m_obPendingScreens;

	//! The root of the game flow heirarchy
	CScreenHeader*			m_pobGameFlow;

	// Last camera view we had, so we only update transforms when necissary
	CamView* m_pobCamView;

	Transform* m_pobCamTransform;

	// BackBuffer size for placing 2D screen space elements		
	float m_fBBHeight;
	float m_fBBWidth;

	// Flags so we know the state of the Gui's music
	// Not sure that it should be here
	// but seems the best place as music across the whole front end system not just particular screens
	bool m_bMainMenuMusic;
	bool m_bPauseMenuMusic;

	// Allow Gui to do some first frame setup; so things like music don't try to play while the rest of the game is loading
	bool m_bFirstFrame;

	// Bool so that we know when to do a reset
	bool m_bDoReset;

	GuiSettingsDef* m_pobGuiSettings;
	GuiNavigationBar* m_pobNavBar;
	//Lists to hold the Combo Screens Data
	ntstd::List< CS_MOVE_INFO_PARENT* >	m_BasicMoveInfoList;
	ntstd::List< CS_MOVE_INFO_PARENT* >	m_SpeedMoveInfoList;
	ntstd::List< CS_MOVE_INFO_PARENT* >	m_RangeMoveInfoList;
	ntstd::List< CS_MOVE_INFO_PARENT* >	m_PowerMoveInfoList;
	ntstd::List< CS_MOVE_INFO_PARENT* >	m_AerialMoveInfoList;
	ntstd::List< CS_MOVE_INFO_PARENT* >	m_SuperStyleMoveInfoList;

};

#endif // _GUIMANAGER_H

