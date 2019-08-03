/***************************************************************************************************
*
*	CLASS			CGuiE3SkinLoadProgress
*
*	DESCRIPTION		Skinned GUI element for indicating load in progress
*
***************************************************************************************************/

// Includes
#include "e3guiskinloadprogress.h"
#include "guimanager.h"
#include "guiutil.h"
#include "anim/hierarchy.h"
#include "core/timer.h"


#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiE3SkinLoadProgress(); }

// Register this class under it's XML tag
bool g_bLOAD_PROGRESS = CGuiManager::Register( "LOAD_PROGRESS", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinLoadProgress::CGuiE3SkinLoadProgress
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiE3SkinLoadProgress::CGuiE3SkinLoadProgress( void )
{
	m_pcImageName = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinLoadProgress::~CGuiE3SkinLoadProgress
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiE3SkinLoadProgress::~CGuiE3SkinLoadProgress( void )
{
	if (m_pcImageName)
		NT_DELETE_ARRAY(m_pcImageName);
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinLoadProgress::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiE3SkinLoadProgress::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnit::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "image" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcImageName );
		}

		if ( strcmp( pcTitle, "imagesize" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_fWidth, &m_fHeight );
		}

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinLoadProgress::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiE3SkinLoadProgress::ProcessEnd( void )
{
	// Call the base first
	CGuiUnitStatic::ProcessEnd();

	// We should have all attributes here so set up the image sprite

	// Overide default render space.
	m_eRenderSpace = CGuiUnit::RENDER_SCREENSPACE;

	float fBBWidth = CGuiManager::Get().BBWidth();
	float fBBHeight = CGuiManager::Get().BBHeight();

	CPoint Pos( m_BasePosition.X()*fBBWidth, m_BasePosition.Y()*fBBHeight, 0.0f );
	CPoint Size( m_fWidth*fBBWidth, m_fHeight*fBBHeight, 0.0f );

	// Set the vertical offset of our image
	switch ( m_eVerticalJustification )
	{
	case JUSTIFY_TOP:		Pos.Y() -= (Size.Y()/2.0f);														break;
	case JUSTIFY_MIDDLE:																					break;
	case JUSTIFY_BOTTOM:	Pos.Y() += (Size.Y()/2.0f);														break;
	default:				ntAssert( 0 );																	break;
	}

	// Set the horizontal offset of our image
	switch ( m_eHorizontalJustification )
	{
	case JUSTIFY_LEFT:		Pos.X() -= (Size.X()/2.0f);														break;
	case JUSTIFY_CENTRE:																					break;
	case JUSTIFY_RIGHT:		Pos.X() += (Size.X()/2.0f);														break;
	default:				ntAssert( 0 );																	break;
	}

	m_obImage.SetPosition( Pos );
	m_obImage.SetWidth( Size.X() );
	m_obImage.SetHeight( Size.Y() );
	m_obImage.SetColour( 0xd0ffffff );
	m_obImage.SetTexture( m_pcImageName );

	ntAssert ( m_obImage.GetTextureHeight() > 0.0f );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageStatic::Update()
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiE3SkinLoadProgress::Update( void )
{
	float fBBWidth = CGuiManager::Get().BBWidth();
	float fBBHeight = CGuiManager::Get().BBHeight();

	float fSysTime = _R(CTimer::Get().GetSystemTime());

	fSysTime = 1.0f + (0.1f * sin(fSysTime));
	CPoint Size( (m_fWidth*fSysTime)*fBBWidth, (m_fHeight*fSysTime)*fBBHeight, 0.0f );

	m_obImage.SetWidth( Size.X() );
	m_obImage.SetHeight( Size.Y() );

	// Call the base update
	return CGuiUnitStatic::Update();
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageStatic::Render()
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiE3SkinLoadProgress::Render( void )
{
	m_obImage.Render();

	return true;
}

