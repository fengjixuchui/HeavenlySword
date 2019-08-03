/***************************************************************************************************
*
*	DESCRIPTION		A usable screen unit that contains movie playback functionality
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskinmovie.h"
#include "guimanager.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinMovie(); }

// Register this class under it's XML tag
bool g_bMOVIE = CGuiManager::Register( "MOVIE", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMovie::CGuiSkinMovie
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinMovie::CGuiSkinMovie( void ) : 
	m_fPixelWidth( CGuiManager::Get().BBWidth() ),
	m_fPixelHeight( CGuiManager::Get().BBHeight() ),
	m_fPixelX( CGuiManager::Get().BBWidth()*0.5f ),
	m_fPixelY( CGuiManager::Get().BBHeight()*0.5f ),
	m_State( GM_IDLE ),
	m_pobGuiMovie( NULL ),
	m_pcFileName( NULL ),
	m_bMovieEndMoveBack( false )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMovie::~CGuiSkinMovie
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinMovie::~CGuiSkinMovie( void )
{
	if ( m_pobGuiMovie )
	{
		NT_DELETE( m_pobGuiMovie );
		m_pobGuiMovie = NULL;
	}

	if ( m_pcFileName )
	{	
		NT_DELETE_ARRAY( m_pcFileName );
		m_pcFileName = NULL;
	}	
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMovie::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinMovie::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "filename" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcFileName );
		}
		else if ( strcmp( pcTitle, "pixelsize" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_fPixelWidth, &m_fPixelHeight );
		}
		else if ( strcmp( pcTitle, "pixelposition" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_fPixelX, &m_fPixelY );
		}
		else if ( strcmp( pcTitle, "movieendmoveback" ) == 0 )
		{
			return GuiUtil::SetBool( pcValue, &m_bMovieEndMoveBack );
		}

		return false;
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMovie::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinMovie::ProcessEnd( void )
{
	// Call the base first
	super::ProcessEnd();

	// Signal the object is ready for movie playback to start.
	m_State = GM_READY;

	return true;
}

void CGuiSkinMovie::UpdateIdle( void )
{
	super::UpdateIdle();

	if ( GM_READY == m_State )
	{
		// It's possible that movie memory is still in use when a new movie is requested.
		// Keep calling start until the movie has been created, then change state.
		if ( NULL == m_pobGuiMovie )
		{
			m_pobGuiMovie = NT_NEW CGuiMovie;
		}

		if ( m_pobGuiMovie )
		{
			if ( m_pobGuiMovie->Start( m_pcFileName ) )
			{
				m_pobGuiMovie->SetSize( m_fPixelWidth, m_fPixelHeight );
				m_pobGuiMovie->SetPosition( m_fPixelX, m_fPixelY );

				m_State = GM_PLAYING;
			}
		}
	}
	else if	( GM_PLAYING == m_State )
	{
		if ( false == m_pobGuiMovie->IsPlaying() )
		{
			if ( m_bMovieEndMoveBack )
			{
				CGuiManager::Get().MoveBackScreen();
			}
			
			m_State = GM_IDLE;
		}
	}
	else if ( GM_IDLE )
	{
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMovie::Render
*
*	DESCRIPTION		Render the movie object.
*
***************************************************************************************************/
bool CGuiSkinMovie::Render( void )
{
	super::Render();

	if ( GM_PLAYING == m_State )
	{
		if ( m_pobGuiMovie )
		{
			m_pobGuiMovie->Render();
		}
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMovie::AnyAction
*
*	DESCRIPTION		Handle the move back action.
*
***************************************************************************************************/
bool CGuiSkinMovie::AnyAction( int iPads )
{
	// Player has requested to exit the movie, so disable the go back at end of movie behaviour.
	m_bMovieEndMoveBack = false;

	return super::AnyAction( iPads );
}
