/***************************************************************************************************
*
*	DESCRIPTION		The main manager of the flow of the game
*
*	NOTES			Looks after front end screens and the management of levels
*
***************************************************************************************************/

// Includes
#include "guimanager.h"
#include "guiscreen.h"
#include "guisound.h"
#include "guiinput.h"
#include "guiparse.h"
#include "guitext.h"
#include "guiresource.h"
#include "guiflow.h"
#include "tutorial.h"
#include "gui/guisettings.h"
#include "gui/menu/guiskinmenunavbar.h"
#include "gui/gameflowloader.h"

#include "objectdatabase/dataobject.h"
#include "game/shellconfig.h"
#include "game/shelllevel.h"
#include "game/shellmain.h"
#include "game/gameinfo.h"
#include "game/nsmanager.h"

#include "anim/hierarchy.h"
#include "gfx/display.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "gui/comboscreen/cscontrol.h"


//extern bool g_using_neutral;
extern bool TryInstallXMLFile( const char* pcFile );

#define SETTINGS_FILE "data/gui/frontend/guiSettings.xml"

// We want to do this number of frames to alow mesages/havok to settle at the start of each load
#define NUM_START_FRAMES 1

// Initialise the registered tag index to zero
int CGuiManager::m_iCurrentRegisteredTagIndex = 0;

// Initialise the fixed array
CGuiManager::TAG_REGISTER CGuiManager::m_astRegisteredTags[ MAX_REGISTERED_TAGS ] = { 0 };

int		CGuiManager::ms_iLevelsLoaded = 0;

#ifdef _DEBUG_GUI_UNIT_STATE
extern float g_fUnitStateDebugX;
extern float g_fUnitStateDebugY;
#endif

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::CGuiManager
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiManager::CGuiManager( void )
{
	m_fFilterTimer = 0.0f;
	
	m_pobPrimaryScreen = NULL;
	m_pobSecondaryScreen = NULL;
	
	Initialise();

	m_obFilter.SetTexture( GuiSettings()->FilterTexture() );
	m_obFilter.SetColour( CVector(1.0f,1.0f,1.0f,0.0f).GetNTColor() );
	m_obFilter.SetPosition( CPoint(BBWidth()*0.5f, BBHeight()*0.5f, 0.0f) );
	m_obFilter.SetWidth( BBWidth() );
	m_obFilter.SetHeight( BBHeight() );

	m_pobNavBar = NT_NEW GuiNavigationBar();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiManager::~CGuiManager
*
*	DESCRIPTION		Desruction
*
***************************************************************************************************/

CGuiManager::~CGuiManager( void )
{
	//Unload settings
	ObjectDatabase::Get().DeleteContainer( ObjectDatabase::Get().GetPointerFromName<ObjectContainer*>( SETTINGS_FILE ) );
	m_pobGuiSettings = NULL;

	CGuiLua::Uninitialise();

	// Destroy any screens
	if (m_pobSecondaryScreen)
		DestroyScreen(m_pobSecondaryScreen);
	if (m_pobPrimaryScreen)
		DestroyScreen(m_pobPrimaryScreen);

	if (m_pobNavBar)
		NT_DELETE(m_pobNavBar);

	// Kill the game flow layout
	NT_DELETE( m_pobGameFlow );

	// Kill the transform, after removing screens as GuiUnits may be using the transform
	if (m_pobCamTransform->GetParent())
		m_pobCamTransform->RemoveFromParent();
	NT_DELETE( m_pobCamTransform );

	// Kill the singltons that this item created
	CXMLParse::Kill();
	CTutorialManager::Kill();
	CFontManager::Kill();
	CStringManager::Kill();
	CGuiSoundManager::Kill();
	CGuiResource::Kill();
	CGuiInput::Kill();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiManager::Initialise
*
*	DESCRIPTION		Guess what this does
*
***************************************************************************************************/

void CGuiManager::Initialise( void )
{	
	//Load gui defaults
	TryInstallXMLFile( SETTINGS_FILE );
	m_pobGuiSettings = ObjectDatabase::Get().GetPointerFromName<GuiSettingsDef*>(CHashedString("GuiSettings"));

	// Create and initialise a GUI input filter
	NT_NEW CGuiInput();
	CGuiInput::Get().Initialise();

	// Create a resource manager
	NT_NEW CGuiResource();

	// Create a parser
	NT_NEW CXMLParse();

	// Create a manager for the text
	NT_NEW CStringManager;

	// Create a manager for the fonts
	NT_NEW CFontManager;

	// Create a manager for the sounds
	NT_NEW CGuiSoundManager;

	// Create a manager for the tutorial
	NT_NEW CTutorialManager;

	// Create a point for camera relative menu items to be hung from
	// We must do this before creating the first screen as the tranform is
	// needed when the GuiUnits are created

	// Translation of menu relative to camera
	CPoint obBasePoint( 0.0f, 0.0f, 8.0f );

	// Create a matrix with the point
	CMatrix obBaseMatrix( CONSTRUCT_IDENTITY );
	obBaseMatrix.SetTranslation( obBasePoint );

	// Now set our transform object out of all that
	m_pobCamTransform = NT_NEW Transform();
	m_pobCamTransform->SetLocalMatrix( obBaseMatrix );

	// Set this transform as a child of the world root
	CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobCamTransform );

	// Set tbe back buffer info for 2D screen elements
	m_fBBWidth = DisplayManager::Get().GetInternalWidth();
	m_fBBHeight = DisplayManager::Get().GetInternalHeight();

	CGuiSoundManager::Get().Initialise();

	CGuiLua::Initialise();
  
	// Import the screen flow definition
	const char* szGameFlowPath = NULL;

	switch (g_ShellOptions->m_eFrontendMode)
		{
	case FRONTEND_NONE:
		szGameFlowPath = "gui\\simpleflow.xml";
		break;
	case FRONTEND_E3:
			szGameFlowPath = "gui\\gameflowp.xml";
		break;
	case FRONTEND_LEVEL_SELECT:
		szGameFlowPath = "gui\\gameflow_levelselect.xml";
		break;
	case FRONTEND_FINAL:
		szGameFlowPath = "gui\\frontend\\frontend.xml";
		break;
	case FRONTEND_TGS:
		szGameFlowPath = "gui\\tgsgameflow.xml";
		break;
	default:
		ntAssert(false && "Invalid Frontend mode");
	};

	ntPrintf("Using gameflow %s\n", szGameFlowPath);
	m_pobGameFlow = CGameflowLoader::Load(szGameFlowPath);

	m_bFirstFrame = true;

	// Start up our first screen
	if ( g_ShellOptions->m_eFrontendMode != FRONTEND_NONE )
	{
		m_bDoReset = false;
	}

	m_pobPrimaryScreen = CreateScreen( m_pobGameFlow );

	//load the combo list data file, well try to anyway.	
	//TryInstallXMLFile( "data/gui/frontend/combolistdata.xml" );


}

void	CGuiManager::LoadGameLevel_Chapter( int nChapterNumber, int nCheckpointID )
{
	GameInfoDef* pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
	ntAssert_p(pobGameInfo, ("GameInfo object not found") );

	const ChapterInfoDef* pobChap = pobGameInfo->GetChapter(nChapterNumber);
	ntAssert_p(pobChap, ("Failed to find chapter %d", nChapterNumber) );

	LoadGameLevel_Name(ntStr::GetString(pobChap->ChapterPath()), nChapterNumber, nCheckpointID);
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::LoadGameLevel_Name
*
*	DESCRIPTION		Pass through to ShellMain request of level load
*
***************************************************************************************************/
void	CGuiManager::LoadGameLevel_Name( const char* pcLevelName, int nChapterNumber, int nCheckpointID )
{
	// 'Orrible nastyness to test chapter related code without being forced to use purgatory
	if ( nChapterNumber == -1 && pcLevelName)
	{
		if		( !strncmp( pcLevelName, "ch0/initialbattle",	17 ) )	
			nChapterNumber = 0;
		else if ( !strncmp( pcLevelName, "ch1/fort",			8 ) )	
			nChapterNumber = 1;
		else if ( !strncmp( pcLevelName, "ch2/walkways",		12 ) )	
			nChapterNumber = 2;
		else if ( !strncmp( pcLevelName, "ch3/temple",			10 ) )	
			nChapterNumber = 3;
		else if ( !strncmp( pcLevelName, "ch4/escape",			10 ) )	
			nChapterNumber = 4;
		else if ( !strncmp( pcLevelName, "ch5/battlefield",		15 ) )	
			nChapterNumber = 5;
		else if ( !strncmp( pcLevelName, "ch6/kingbattle",		14 ) )	
			nChapterNumber = 6;
	}
	
	if ( pcLevelName )
	{
		// load
		ShellMain::Get().RequestLevelLoad( pcLevelName, nChapterNumber, nCheckpointID );
	}

	ms_iLevelsLoaded++;
}

void CGuiManager::LoadDefaultGameLevel()
{
	switch (g_ShellOptions->m_eFrontendMode)
	{
	case FRONTEND_NONE:
		CGuiManager::LoadGameLevel_Name( g_ShellOptions->m_dbgStartLevel.c_str(), -1, g_ShellOptions->m_dbgStartCheckpoint );
		break;

	case FRONTEND_E3:
		CGuiManager::LoadGameLevel_Name( g_ShellOptions->m_dbgStartLevel.c_str(), -1, g_ShellOptions->m_dbgStartCheckpoint );
		break;

	case FRONTEND_LEVEL_SELECT:
		if (g_ShellOptions->m_bSkipLevelSelect && ms_iLevelsLoaded < 1)
			CGuiManager::LoadGameLevel_Name( g_ShellOptions->m_dbgStartLevel.c_str(), -1, g_ShellOptions->m_dbgStartCheckpoint );
		break;

	case FRONTEND_FINAL:
		break;

	case FRONTEND_TGS:
		CGuiManager::LoadGameLevel_Name( g_ShellOptions->m_dbgStartLevel.c_str(), -1, g_ShellOptions->m_dbgStartCheckpoint );
		break;

	default:
		ntError_p(false, ("Invalid frontend mode"));
	};
}

void CGuiManager::UnloadGameLevel()
{
	if (ShellMain::Get().HaveLoadedLevel())
		ShellMain::Get().RequestLevelUnload();	
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::Update
*
*	DESCRIPTION		This updates it, please call me every frame
*
***************************************************************************************************/

void CGuiManager::Update( void )
{
	if (m_bFirstFrame)
	{
		//CGuiSoundManager::Get().PlayMusic( CGuiSound::MUSIC_MAINMENU );

		m_bMainMenuMusic = true;
		m_bPauseMenuMusic = false;

		// Don't try this again
		m_bFirstFrame = false;
	}

	// Update the input filter
	CGuiInput::Get().Update();

	// Update the tutorial system
	if (ShellMain::Get().HaveLoadedLevel())
	CTutorialManager::Get().Update();

#ifdef DEBUG_GUI
	if ( ShellMain::Get().IsPausedByUser() )
		g_VisualDebug->Printf2D(10,10,DC_RED,0,"GUI- USER PAUSE");
	if ( ShellMain::Get().IsPausedByCode() )
		g_VisualDebug->Printf2D(10,22,DC_RED,0,"GUI- CODE PAUSE");
	if ( ShellMain::Get().IsPausedBySystem() )
		g_VisualDebug->Printf2D(10,34,DC_RED,0,"GUI- OS PAUSE");
#endif

	// Update transform (Incase camera changed)
	UpdateTransform();

	//Process pending MoveTo screens
	if ( m_obPendingScreens.size() > 0 )
	{
		for( ntstd::List< CScreenHeader* >::iterator obIt = m_obPendingScreens.begin(); obIt != m_obPendingScreens.end(); )
		{
			// Update the screen
#ifdef DEBUG_GUI
			ntPrintf("Gui: Attempting to resolve screen move request - %s\n", (*obIt)->GetDefinitionFileNameP());
#endif
			if ( MoveScreen( ( *obIt ) ) )
			{
				obIt = m_obPendingScreens.erase( obIt );
			}
			else
			{
				++obIt;
			}
		}
	}

	//process input. old + new style :/
	ProcessInput();

#ifdef _DEBUG_GUI_UNIT_STATE
	g_fUnitStateDebugX = 100.0f;
	g_fUnitStateDebugY = 100.0f;
#endif

	//if we have a secondary screen, update it until it is ready for destruction
	if (m_pobSecondaryScreen && m_pobSecondaryScreen->pobScreen->Update() == false)
	{
		DestroyScreen(m_pobSecondaryScreen);
#ifdef DEBUG_GUI
		ntPrintf("Gui: Destroying secondary screen\n");
#endif
		m_pobSecondaryScreen = NULL;
	}

	if (m_pobPrimaryScreen && m_pobPrimaryScreen->pobScreen->Update() == false)
	{
		DestroyScreen(m_pobPrimaryScreen);
#ifdef DEBUG_GUI
		ntPrintf("Gui: Destroying primary screen\n");
#endif
		m_pobPrimaryScreen = NULL;
	}


#ifdef DEBUG_GUI
	g_VisualDebug->Printf2D(10, 450.0f, DC_RED, 0, "Primary Screen: %s", m_pobPrimaryScreen ? m_pobPrimaryScreen->pobScreenHeader->GetFilename() : "NULL");
	g_VisualDebug->Printf2D(10, 465.0f, DC_RED, 0, "Secondary Screen: %s", m_pobSecondaryScreen ? m_pobSecondaryScreen->pobScreenHeader->GetFilename() : "NULL");
#endif

	UpdateFilter();

	if (m_pobNavBar)
		m_pobNavBar->Update();
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::ProcessInput
*
*	DESCRIPTION		Deals with pad input. If the screen includes as 'acceptsinput' attrib then we 
*						allow the screen to handle it. otherwise (demo...) handle it out here.
*
***************************************************************************************************/

void CGuiManager::ProcessInput()
	{
	if (!m_pobPrimaryScreen)
		return;

	if (m_pobPrimaryScreen->pobScreen->NewStyleInput())
		{
		m_pobPrimaryScreen->pobScreen->ProcessInput();
		}
		else
		{
		// If this is a HUD, only the start button can be used
		// and not during a NS/cutscene
		if ( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_HUD )
		{	
			if ( !NSManager::Get().IsNinjaSequencePlaying() )
			{
				m_pobPrimaryScreen->pobScreen->StartAction( CGuiInput::Get().StartPress() );

				// Any indicriminate actions? - suitable for preventing level restart
				m_pobPrimaryScreen->pobScreen->AnyAction( CGuiInput::Get().AnyAction(true) );
			}
		}
		// If this is an ATTRACT screen, any action can be used
		else if ( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_ATTRACT )
		{	
			m_pobPrimaryScreen->pobScreen->StartAction( CGuiInput::Get().AnyAction(true) );
		}
		// If this is a SCREEN_NO_SKIP screen, no action can be used i.e. a timer splash or a load screen
		else if ( ! (m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_NO_SKIP ))
		{
			// All other screens

			// Register the positive actions first
			m_pobPrimaryScreen->pobScreen->StartAction( CGuiInput::Get().StartPress() );
			m_pobPrimaryScreen->pobScreen->SelectAction( CGuiInput::Get().SelectPress() );

			// Movement actions next
			m_pobPrimaryScreen->pobScreen->MoveUpAction( CGuiInput::Get().MoveUp() );
			m_pobPrimaryScreen->pobScreen->MoveDownAction( CGuiInput::Get().MoveDown() );
			m_pobPrimaryScreen->pobScreen->MoveLeftAction( CGuiInput::Get().MoveLeft() );
			m_pobPrimaryScreen->pobScreen->MoveRightAction( CGuiInput::Get().MoveRight() );

			// Finally the negative actions - which we apparently dont want on the pause screen
			if (!( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_PAUSE ) )
			{
				m_pobPrimaryScreen->pobScreen->BackAction( CGuiInput::Get().Back() );
				m_pobPrimaryScreen->pobScreen->DeleteAction( CGuiInput::Get().Delete() );
			}

			// Any indicriminate actions? - suitable for preventing/returning from attract mode
			m_pobPrimaryScreen->pobScreen->AnyAction( CGuiInput::Get().AnyAction(true) );
		}
	}
	}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::UpdateFilter
*
*	DESCRIPTION		Updates the filter effect. Deals with fading in and out depending on the current
*					whether the active screens want it active
*
***************************************************************************************************/

void CGuiManager::UpdateFilter()
{
	SCREEN_INFO* pobScreen = m_pobPrimaryScreen ? m_pobPrimaryScreen : m_pobSecondaryScreen;
	if (!pobScreen)
		return;

	enum { FADEIN, FADEOUT } eFilterState;
	eFilterState = pobScreen->pobScreen->ShowFilter() ? FADEIN : FADEOUT;

	switch (eFilterState)
	{
	case FADEIN:
		m_fFilterTimer += CTimer::Get().GetSystemTimeChange();
		if (m_fFilterTimer > 1.0f)
		{
			m_fFilterTimer = 1.0f;
		}
		break;
	case FADEOUT:
		m_fFilterTimer -= CTimer::Get().GetSystemTimeChange();
		if (m_fFilterTimer < 0.0f)
		{
			m_fFilterTimer = 0.0f;
		}
		break;
	default:
		ntError(false);
	};
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::Render
*
*	DESCRIPTION		This renders it, please call me every frame
*
***************************************************************************************************/

void CGuiManager::Render( void )
{
	if (m_fFilterTimer > 0.0f)
	{
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
		m_obFilter.SetColour( CVector(1.0f,1.0f,1.0f,m_fFilterTimer).GetNTColor() );
		m_obFilter.Render();
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
	}

	// Call render on all our screens 
	if (m_pobSecondaryScreen)
		m_pobSecondaryScreen->pobScreen->Render();
	if (m_pobPrimaryScreen)
		m_pobPrimaryScreen->pobScreen->Render();

	//render the navbar
	if (m_pobNavBar)
		m_pobNavBar->Render();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiManager::UpdateTransform
*
*	DESCRIPTION		Updates the transform on GUI elements, required by camera space
*					elements in the event that the camera changes.
*
***************************************************************************************************/

void CGuiManager::UpdateTransform( void )
{
	if (ShellMain::Get().HaveLoadedLevel() == false)
		return;

	CamView* pobCamView = CamMan::GetPrimaryView();
	ntAssert( pobCamView );

	// If the camera is the same as our last update then we don't need to be here
	if (pobCamView == m_pobCamView)
		return;

	// We're going to put on a new camera transform, so hold onto it to check next time.
	m_pobCamView = pobCamView;

	Transform* pCamTrans = (Transform*)pobCamView->GetViewTransform();

	if ( m_pobCamTransform->GetParent() )
		m_pobCamTransform->RemoveFromParent();
	pCamTrans->AddChild( m_pobCamTransform );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiManager::RemoveTransform
*
*	DESCRIPTION		Updates the transform on GUI elements, required by camera space
*					elements in the event that the camera changes.
*
***************************************************************************************************/

void CGuiManager::RemoveTransform( void )
{
	if ( m_pobCamTransform->GetParent() )
	{
		m_pobCamTransform->RemoveFromParent();
	
		// Set this transform as a child of the world root
		CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobCamTransform );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::CreateScreen
*
*	DESCRIPTION		
*
***************************************************************************************************/

CGuiManager::SCREEN_INFO* CGuiManager::CreateScreen( CScreenHeader* pobScreenHeader )
{
	ntPrintf("CreateScreen: %s\n", pobScreenHeader->GetDefinitionFileNameP());
	// Check we've been given something useful
	ntAssert( pobScreenHeader );

	// Create a new structure to hold the screen with it's header
	SCREEN_INFO* pstrScreen = NT_NEW SCREEN_INFO;

	// Link to the header
	pstrScreen->pobScreenHeader = pobScreenHeader;

	// Build the screen
	CGuiScreen::ms_pobCurrentScreenHeader = pobScreenHeader;
	pstrScreen->pobScreen = ( CGuiScreen* )CXMLParse::Get().CreateTree( pobScreenHeader->GetDefinitionFileNameP() );
	CGuiScreen::ms_pobCurrentScreenHeader = NULL;

	// Let the screen know about its flags
	pstrScreen->pobScreen->SetScreenFlags( pstrScreen->pobScreenHeader->GetScreenFlags() );

	// Update the transform, incase it needs to be camera relative
	UpdateTransform();

	// Give them the lot back
	return pstrScreen;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiManager::DestroyScreen
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiManager::DestroyScreen( SCREEN_INFO* pstrScreen )
{
	ntPrintf("DestroyScreen: %s\n", pstrScreen->pobScreenHeader->GetDefinitionFileNameP());
	// Check we've been given something useful
	ntAssert( pstrScreen );

	// Delete the actual screen
	NT_DELETE( pstrScreen->pobScreen );

	// Now free the struct that holds it all - the screen header we
	// leave alone - it's held elsewhere
	NT_DELETE( pstrScreen );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiManager::MoveOnScreen
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiManager::MoveOnScreen( int iOption )
{
	// I don't think this is necissary as all back screens clean themselves up so we 
	// can come here with more than one screen - TMcK

	// See if there is a screen to go  to
	ntError_p(m_pobPrimaryScreen, ("Gui: No primary screen available"));
	CScreenHeader* pobNextScreen = m_pobPrimaryScreen->pobScreenHeader->GetNextScreenP( iOption );

	return RequestMoveScreen ( pobNextScreen );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiManager::MoveBackScreen
*
*	DESCRIPTION		Called when back is requested on a screen.  iScreensBack can be used to force a
*					move of a specific number of screens - ignoring the SCREEN_NO_BACK setting
*
***************************************************************************************************/

bool CGuiManager::MoveBackScreen( int iScreensBack )
{
	// I don't think this is necissary as all back screens clean themselves up so we 
	// can come here with more than one screen - TMcK

	ntError_p(m_pobPrimaryScreen, ("Gui: No primary screen available"));

	if ( !( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_NO_BACK ) || iScreensBack)
	{
		// Make sure there is a screen to go back to
		CScreenHeader* pobBackScreen = m_pobPrimaryScreen->pobScreenHeader->GetBackScreenP( iScreensBack );
		return RequestMoveScreen ( pobBackScreen );
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::MoveOnScreenType
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CGuiManager::MoveOnScreenType( int iScreenFlag )
{
	// No type, just do normal move on
	if (! iScreenFlag )
	{
		return MoveOnScreen();
	}

	// Search for a screen to go to
	ntError_p(m_pobPrimaryScreen, ("Gui: No primary screen available"));
	CScreenHeader* pobNextScreen = m_pobPrimaryScreen->pobScreenHeader->FindForwardScreenP(iScreenFlag);

	return RequestMoveScreen ( pobNextScreen );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::MoveBackScreenType
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CGuiManager::MoveBackScreenType( int iScreenFlag )
{
	// No type, just do normal move oback
	if (! iScreenFlag )
	{
		return MoveBackScreen();
	}

	// Make sure there is a screen to go back to
	ntError_p(m_pobPrimaryScreen, ("Gui: No primary screen available"));
	CScreenHeader* pobBackScreen = m_pobPrimaryScreen->pobScreenHeader->FindBackScreenP(iScreenFlag);	

	return RequestMoveScreen ( pobBackScreen );
}

bool CGuiManager::MoveOnScreenGroup( const char* pcGroup )
{
	ntAssert(pcGroup);

	// Search for a screen to go to
	ntError_p(m_pobPrimaryScreen, ("Gui: No primary screen available"));
	CScreenHeader* pobNextScreen = m_pobPrimaryScreen->pobScreenHeader->FindForwardScreenP(pcGroup);	

	return RequestMoveScreen ( pobNextScreen );
}

bool CGuiManager::MoveBackScreenGroup( const char* pcGroup )
{
	ntAssert(pcGroup);

	// Make sure there is a screen to go back to
	ntError_p(m_pobPrimaryScreen, ("Gui: No primary screen available"));
	CScreenHeader* pobBackScreen = m_pobPrimaryScreen->pobScreenHeader->FindBackScreenP(pcGroup);

	return RequestMoveScreen ( pobBackScreen );
}

bool CGuiManager::SkipForwardScreen( int iCount, bool bUsePath... )
{
	ntError_p(m_pobPrimaryScreen, ("Gui: No primary screen available"));
	CScreenHeader* pobCurrentScreen = m_pobPrimaryScreen->pobScreenHeader;

	if (bUsePath)
	{
		va_list tag;
		va_start(tag, bUsePath);

		for (int i = 0; i < iCount; i++)
		{
			const char* pcTag = va_arg(tag, const char *);
			ntAssert(pcTag);

			CScreenHeader* pobNextScreen = pobCurrentScreen->FindForwardScreenP(pcTag);
			if (pobNextScreen)
				pobCurrentScreen = pobNextScreen;
			else
			{
				ntAssert_p(false, ("Could not find next screen in path: %s", pcTag));
				va_end(tag);
				return false;
			}
		}
		va_end(tag);
	}
	else
	{
		for (int i = 0; i < iCount; i++)
		{
			CScreenHeader* pobNextScreen = pobCurrentScreen->GetNextScreenP();
			if (pobNextScreen)
				pobCurrentScreen = pobNextScreen;
			else
			{
				ntAssert_p(false, ("Could not find next screen..."));
				return false;
			}
		}
	}

	return RequestMoveScreen ( pobCurrentScreen );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::RequestMoveScreen
*
*	DESCRIPTION		Request a move to a new screen
*
***************************************************************************************************/
bool CGuiManager::RequestMoveScreen(CScreenHeader* pobNewScreen)
{
	if ( ntstd::find( m_obPendingScreens.begin(), m_obPendingScreens.end(), pobNewScreen ) == m_obPendingScreens.end() )
	{
#ifdef DEBUG_GUI
		ntPrintf("Gui: Move screen request - %s\n", pobNewScreen->GetDefinitionFileNameP());
#endif
		m_obPendingScreens.push_back(pobNewScreen);
	}
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::MoveScreen
*
*	DESCRIPTION		Do the grunt work of changing screens
*
***************************************************************************************************/
bool CGuiManager::MoveScreen(CScreenHeader* pobNewScreen)
{
	// If we have a screen to go to
	if ( pobNewScreen )
	{
		//printf("MoveScreen -> %d\n",pobNewScreen->GetScreenFlags());

		// Option to disable the attract mode
		if ( (pobNewScreen->GetScreenFlags() & CScreenHeader::SCREEN_ATTRACT) 
			&& (! g_ShellOptions->m_bGuiAttractMode ) )
			{
			return false;
		}

		bool bAdd = false;
		if (!m_pobPrimaryScreen)
		{
			bAdd = true;
		}
		else
		{
			if (m_pobPrimaryScreen->pobScreen->CrossFadeExiting())
			{
				if (!m_pobSecondaryScreen)
			{
					m_pobPrimaryScreen->pobScreen->BeginExit();
					m_pobSecondaryScreen = m_pobPrimaryScreen;
					m_pobPrimaryScreen = NULL;
					bAdd = true;
				}
			}
			else
			{
				//is the primary still alive?
				if ( !m_pobPrimaryScreen->pobScreen->IsDead() )
				{
					if ( m_pobPrimaryScreen->pobScreen->IsInteractive() )
			{
						// Start to get rid of the current one
						m_pobPrimaryScreen->pobScreen->BeginExit(true);
					}
			}
				else
			{
					//let it die...
					ntError_p(m_pobSecondaryScreen == NULL, ("Gui: Secondary screen still active"));
					m_pobSecondaryScreen = m_pobPrimaryScreen;
					m_pobPrimaryScreen = NULL;
					bAdd = true;
				}
			}
			}

		if ( bAdd )
			{
			ntError_p(m_pobPrimaryScreen == NULL, ("Gui: Already have a primary screen while attempting to create a new one :/"));
			m_pobPrimaryScreen = CreateScreen( pobNewScreen );
			ntError_p(m_pobPrimaryScreen, ("Gui: Failed to create next screen :/"));

			PostCreateScreen();
			}
		else
			{
			RequestMoveScreen(pobNewScreen);
			return false;
		}
		return true;
			}
	return false;
		}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::PostCreateScreen
*
*	DESCRIPTION		Called once a screen is created. Uses the flag system to determine what needs to be done.
*
***************************************************************************************************/

void CGuiManager::PostCreateScreen()
		{
	ntError_p(m_pobPrimaryScreen, ("Gui: no primary screen in PostCreateScreen"));

			// If this new screen is a HUD screen unpause game.
			// This function could get really complex and messy if we keep checking
			// all these flags.  MUST make sure that doesn't happen.
	if ( ( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_HUD ) )
			{
				ShellMain::Get().RequestPauseToggle(ShellMain::PM_USER,false);

				// Also need to stop any menu music
				if (m_bMainMenuMusic)
				{
					m_bMainMenuMusic = false;
					CGuiSoundManager::Get().StopMusic( CGuiSound::MUSIC_MAINMENU );
				}

				if (m_bPauseMenuMusic)
				{
					m_bPauseMenuMusic = false;
					CGuiSoundManager::Get().StopMusic( CGuiSound::MUSIC_PAUSEMENU );
				}

				// We've come into the game now, make sure we know to do a reset next time were at the start screen
				m_bDoReset = true;
			}

			// If this new screen is a SCREEN_ROOTMENU screen play the menu music
	if ( ( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_ROOTMENU ) )
			{
				// Just incase pause music already playing
				if (m_bPauseMenuMusic)
				{
					m_bPauseMenuMusic = false;
					CGuiSoundManager::Get().StopMusic( CGuiSound::MUSIC_PAUSEMENU );
				}

				if (!m_bMainMenuMusic)
				{
					m_bMainMenuMusic = true;
					//CGuiSoundManager::Get().PlayMusic( CGuiSound::MUSIC_MAINMENU );
				}
			}

			// If this new screen is a SCREEN_PAUSE screen play the pause menu music
	if ( ( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_PAUSE ) )
			{
				// TGS music
				CLuaGlobal::CallLuaFunc("OnPauseMenuEnter");

				// Just incase menu music already playing
				if (m_bMainMenuMusic)
				{
					m_bMainMenuMusic = false;
					CGuiSoundManager::Get().StopMusic( CGuiSound::MUSIC_MAINMENU );
				}

				if (!m_bPauseMenuMusic)
				{
					m_bPauseMenuMusic = true;
					//CGuiSoundManager::Get().PlayMusic( CGuiSound::MUSIC_PAUSEMENU );
				}
			}

	if ( ( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_LOAD ) )
			{
				// Make sure we stop any menu music
				if (m_bMainMenuMusic)
				{
					m_bMainMenuMusic = false;
					CGuiSoundManager::Get().StopMusic( CGuiSound::MUSIC_MAINMENU );
				}

				if (m_bPauseMenuMusic)
				{
					m_bPauseMenuMusic = false;
					CGuiSoundManager::Get().StopMusic( CGuiSound::MUSIC_PAUSEMENU );
				}
			}

	// few checks of the old screen
	if (m_pobSecondaryScreen)
	{
			// If the old screen is a SCREEN_LOAD screen attempt to restart level
		if ( ( m_pobSecondaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_LOAD ) )
			{
				if (m_bDoReset)
				{
					// null parameter signfies a reload of last
					ResetLevel(0);
				}
			}

			// If the old screen was a HUD then pause game
		if ( ( m_pobSecondaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_HUD ) )
			{
				ShellMain::Get().RequestPauseToggle(ShellMain::PM_USER,true);
			}

			//TGS -  If the old screen was a PAUSE and the new screen is a HUD change music mix
		if ( ( m_pobSecondaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_PAUSE ) 
			&& ( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_HUD ) )
			{
				// TGS music
				CLuaGlobal::CallLuaFunc("OnPauseMenuReturnToGame");
			}

			//TGS -  If the old screen was a PAUSE and the new screen is a LOAD change music mix
		if ( ( m_pobSecondaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_PAUSE ) 
		&& ( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_LOAD ) )
			{
				// TGS music
				CLuaGlobal::CallLuaFunc("OnPauseMenuLeaveGame");
			}

			//TGS -  If the old screen was a HUD and the new screen is a COMPLETE change music mix
		if ( ( m_pobSecondaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_HUD ) 
		&& ( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_COMPLETE ) )
			{
				// TGS music
				CLuaGlobal::CallLuaFunc("OnGameEnd");
			}

		//Some audio hooks aswell
		const int iNewScreenFlags = m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags();
		const int iOldScreenFlags = m_pobSecondaryScreen->pobScreenHeader->GetScreenFlags();

		if (!(iOldScreenFlags & CScreenHeader::SCREEN_ROOTMENU) && (iNewScreenFlags & CScreenHeader::SCREEN_ROOTMENU)) // We have entered the front end
		{
			GameAudioManager::Get().OnFrontend();

			CGuiSoundManager::Get().PlayMusic( CGuiSound::MUSIC_MAINMENU );
		}
		else if ((iOldScreenFlags & CScreenHeader::SCREEN_HUD) && (iNewScreenFlags & CScreenHeader::SCREEN_PAUSE)) // We have gone from in-game to pause menu
		{
			GameAudioManager::Get().OnPause(true);
		}
		else if ((iOldScreenFlags & CScreenHeader::SCREEN_PAUSE) && (iNewScreenFlags & CScreenHeader::SCREEN_HUD)) // We have gone from pause menu to in-game
		{
			GameAudioManager::Get().OnPause(false);
		}
		else if (!(iOldScreenFlags & CScreenHeader::SCREEN_HUD) && (iNewScreenFlags & CScreenHeader::SCREEN_HUD)) // We have gone from pause menu to in-game
			{
			GameAudioManager::Get().OnLevelStart();
			}
		else if (
			((iOldScreenFlags & CScreenHeader::SCREEN_HUD) && (iNewScreenFlags & CScreenHeader::SCREEN_COMPLETE)) || // We have gone from game to complete
			((iOldScreenFlags & CScreenHeader::SCREEN_PAUSE) && (iNewScreenFlags & CScreenHeader::SCREEN_COMPLETE)) || // We have gone from pause menu to complete
			((iOldScreenFlags & CScreenHeader::SCREEN_PAUSE) && (iNewScreenFlags & CScreenHeader::SCREEN_LOAD)) // We have gone from pause menu to loading screen
			)
		{
			GameAudioManager::Get().OnLevelExit();
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::Register
*
*	DESCRIPTION		This function is called before main to register the Gui Elements that the 
*					manager is able to construct on request.
*
***************************************************************************************************/

bool CGuiManager::Register( const char* pcTagName, CXMLElement* ( *ContructionWrapper ) ( void ) )
{
	// Hold the tag which indicates what to construct along with a pointer to a function wrapper
	m_astRegisteredTags[ m_iCurrentRegisteredTagIndex ].pcTag = pcTagName;
	m_astRegisteredTags[ m_iCurrentRegisteredTagIndex ].ContructionWrapper = ContructionWrapper;

	// Proceed through the array
	m_iCurrentRegisteredTagIndex++;

	ntAssert ( m_iCurrentRegisteredTagIndex < MAX_REGISTERED_TAGS );

	// Return an arbitrary value for the dummy initialisation
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiManager::CreateGuiElement
*
*	DESCRIPTION		Returns a fresh new screen element based on it's XML tag.
*
***************************************************************************************************/

CXMLElement* CGuiManager::CreateGuiElement( const char* pcTag )
{
	// Check they're not passing us any bobbins
	ntAssert( pcTag );

	// Find the element we want and build one
	for ( int iTag = 0; iTag < MAX_REGISTERED_TAGS; iTag++ )
	{
		if ( m_astRegisteredTags[ iTag ].pcTag )
		{
			if ( strcmp( m_astRegisteredTags[ iTag ].pcTag, pcTag ) == 0 )
			{
				// Call the linked constructor wrapper
				return ( *m_astRegisteredTags[ iTag ].ContructionWrapper )();
			}
		}

		// There are no gaps in the array so we can exit here
		else 
		{
			// Things are bad if we are here
			ntAssert ( 0 );
			return 0;
		}
	}

	// Things are also bad if we are here
	ntAssert ( 0 );
	return 0;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::AddGuiElement
*
*	DESCRIPTION		Add a newly created Gui element to the current screen.
*
***************************************************************************************************/
bool CGuiManager::AddGuiElement(CGuiUnit* pobNewElement)
{
	return m_pobPrimaryScreen->pobScreen->AddGuiElement(pobNewElement);
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::RemoveGuiElement
*
*	DESCRIPTION		Remove specified gui element if still available.
*
***************************************************************************************************/
bool CGuiManager::RemoveGuiElement(CGuiUnit* pobElement)
{
	return m_pobPrimaryScreen->pobScreen->RemoveGuiElement(pobElement);
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::OnComplete
*
*	DESCRIPTION		Temporay function to move to a complete screen following the end NS of the E3 demo
*
***************************************************************************************************/

void CGuiManager::OnComplete( void )
{
	ntError_p(m_pobPrimaryScreen, ("Gui: No primary screen available"));

	if ( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_HUD )
	{
#ifdef DEBUG_GUI
		ntPrintf("Move on to Complete screen, please\n");
#endif

		MoveOnScreenType(CScreenHeader::SCREEN_COMPLETE);
	}
	else
	{
		MoveBackScreenType(CScreenHeader::SCREEN_LOAD);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::ResetLevel
*
*	DESCRIPTION		Reset level - without reloading resources
*
***************************************************************************************************/

bool CGuiManager::ResetLevel( const char* pcLevelName, int nCheckpointID)
{
#ifdef DEBUG_GUI
	ntPrintf("GUI: Do level reset\n");
#endif

	if ( pcLevelName )
	{
	// Specific level
		LoadGameLevel_Name(pcLevelName, -1, nCheckpointID);
	}
	else
	{
		// Or the one we last used
		LoadGameLevel_Name(	ShellMain::Get().GetCurrRunningLevel()->GetLevelName(), 
							-1, 
							ShellMain::Get().GetCurrRunningLevel()->GetStartCheckpointID());
	}

	m_bDoReset = false;

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiManager::IsActive()
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CGuiManager::IsActive(void)
{
	if ( g_ShellOptions->m_eFrontendMode == FRONTEND_NONE )
		return false;

	ntError_p(m_pobPrimaryScreen, ("Gui: No primary screen available"));

	// No screen, then best not claim to be active
	if ( !m_pobPrimaryScreen->pobScreenHeader )
		return false;

	// Need to update game underneath pause and hud screens
	if ( ( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_HUD ) ||
		( m_pobPrimaryScreen->pobScreenHeader->GetScreenFlags() & CScreenHeader::SCREEN_PAUSE ) )
	{
		return false;
	}
	
	if (m_pobPrimaryScreen->pobScreen->GameUpdateAllowed())
	{
		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiManager:::GetComboScreenMoveInfoXXXXXList
*
*	DESCRIPTION		
*
***************************************************************************************************/
// This are access for the xml data required for the combo list move data.
ntstd::List< CS_MOVE_INFO_PARENT* >*	CGuiManager::GetComboScreenMoveInfoBasicList( void )
{
	return &m_BasicMoveInfoList;
}

ntstd::List< CS_MOVE_INFO_PARENT* >*	CGuiManager::GetComboScreenMoveInfoSpeedList( void )
{
	return &m_SpeedMoveInfoList;
}

ntstd::List< CS_MOVE_INFO_PARENT* >*	CGuiManager::GetComboScreenMoveInfoRangeList( void )
{
	return &m_RangeMoveInfoList;
}

ntstd::List< CS_MOVE_INFO_PARENT* >*	CGuiManager::GetComboScreenMoveInfoPowerList( void )
{
	return &m_PowerMoveInfoList;
}

ntstd::List< CS_MOVE_INFO_PARENT* >*	CGuiManager::GetComboScreenMoveInfoAerialList( void )
{
	return &m_AerialMoveInfoList;
}

ntstd::List< CS_MOVE_INFO_PARENT* >*	CGuiManager::GetComboScreenMoveInfoSuperStyleList( void )
{
	return &m_SuperStyleMoveInfoList;
}
