/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface button unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

// Includes
#include "e3guiskinimagebutton.h"
#include "guimanager.h"
#include "guitext.h"
#include "guiflow.h"
#include "anim/hierarchy.h"
#include "guisound.h"



#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gfx/graphicsdevice.h"

// Convertion for bitwise enum values to strings - null terminated for converter
const CStringUtil::STRING_FLAG astrActionFlags[] = 
{	
	{ CGuiE3SkinImageButton::MOVE_SCREEN_ON,		"MOVE_SCREEN_ON"		},
	{ CGuiE3SkinImageButton::MOVE_SCREEN_BACK,		"MOVE_SCREEN_BACK"		},
	{ CGuiE3SkinImageButton::MOVE_SCREENTYPE_ON,	"MOVE_SCREENTYPE_ON"	},
	{ CGuiE3SkinImageButton::MOVE_SCREENTYPE_BACK,	"MOVE_SCREENTYPE_BACK"	},
	{ CGuiE3SkinImageButton::RESUME_GAME,			"RESUME_GAME"			},
	{ CGuiE3SkinImageButton::RELOAD_LEVEL,			"RELOAD_LEVEL"			},
	{ CGuiE3SkinImageButton::LOAD_LEVEL,			"LOAD_LEVEL"			},
	{ 0,											0						} 
};

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiE3SkinImageButton(); }

// Register this class under it's XML tag
bool g_bIMAGEBUTTON = CGuiManager::Register( "IMAGE_BUTTON", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::CGuiE3SkinImageButton
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiE3SkinImageButton::CGuiE3SkinImageButton( void )
:		m_iActionFlags(0)
,		m_pcActionParam(0)
,		m_iActionParam(0)
,		m_pcStringText(0)
,		m_fHeight(0.0f)
,		m_fWidth(0.0f)
,		m_pcImageName(0)
,		m_pcImageOffName(0)
,		m_pcBackgroundName(0)
{}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::~CGuiE3SkinImageButton
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiE3SkinImageButton::~CGuiE3SkinImageButton( void )
{
	if ( m_pcStringText ) 
		NT_DELETE_ARRAY( m_pcStringText );

	if ( m_pcImageName ) 
		NT_DELETE_ARRAY( m_pcImageName );

	if ( m_pcImageName ) 
		NT_DELETE_ARRAY( m_pcImageOffName );

	if ( m_pcBackgroundName ) 
		NT_DELETE_ARRAY( m_pcBackgroundName );

	if ( m_pcActionParam )
		NT_DELETE_ARRAY( m_pcActionParam );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiE3SkinImageButton::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnitButton::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "titlestringid" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcStringText );
		}

		if ( strcmp( pcTitle, "imageon" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcImageName );
		}

		if ( strcmp( pcTitle, "imageoff" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcImageOffName );
		}


		if ( strcmp( pcTitle, "background" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcBackgroundName );
		}

		if ( strcmp( pcTitle, "imagesize" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_fWidth, &m_fHeight );
		}

		if ( strcmp( pcTitle, "action" ) == 0 )
		{
			return GuiUtil::SetFlags( pcValue, &astrActionFlags[0], &m_iActionFlags );
		}

		if (m_iActionFlags)
		{
			return ProcessAction( pcTitle,  pcValue );
		}

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::ProcessAction
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiE3SkinImageButton::ProcessAction( const char* pcTitle, const char* pcValue )
{
	// We really shouldn't be here until we have some action flags
	ntAssert(m_iActionFlags);
	
	if ( (m_iActionFlags & MOVE_SCREENTYPE_BACK) || (m_iActionFlags & MOVE_SCREENTYPE_ON) )
	{
		if ( strcmp( pcTitle, "move" ) == 0 )
		{
			return GuiUtil::SetFlags( pcValue, &astrScreenFlags[0], &m_iActionParam );
		}
	}

	if ( (m_iActionFlags & MOVE_SCREEN_ON) || (m_iActionFlags & MOVE_SCREEN_BACK) )
	{
		if ( strcmp( pcTitle, "move" ) == 0 )
		{
			return GuiUtil::SetInt( pcValue, &m_iActionParam);
		}
	}

	if (m_iActionFlags & LOAD_LEVEL)
	{
		if ( strcmp( pcTitle, "levelname" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcActionParam );
		}
	}

	return false;
	
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiE3SkinImageButton::ProcessEnd( void )
{
	// Call the base first
	CGuiUnitButton::ProcessEnd();

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
	default:								ntAssert( 0 );																	break;
	}

	// Set the horizontal offset of our image
	switch ( m_eHorizontalJustification )
	{
	case JUSTIFY_LEFT:		Pos.X() -= (Size.X()/2.0f);														break;
	case JUSTIFY_CENTRE:																					break;
	case JUSTIFY_RIGHT:		Pos.X() += (Size.X()/2.0f);														break;
	default:								ntAssert( 0 );																	break;
	}

	m_fBackgroundAlpha = 0.0f;
	m_fImageAlpha = 0.0f;
	m_fImageOffAlpha = 0.0f;

	m_obImage.SetPosition( Pos );
	m_obImage.SetWidth( Size.X() );
	m_obImage.SetHeight( Size.Y() );
	m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageAlpha).GetNTColor() );
	m_obImage.SetTexture( m_pcImageName );

	m_obImageOff.SetPosition( Pos );
	m_obImageOff.SetWidth( Size.X() );
	m_obImageOff.SetHeight( Size.Y() );
	m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageOffAlpha).GetNTColor() );
	m_obImageOff.SetTexture( m_pcImageOffName );

	m_obBackground.SetPosition( Pos );
	m_obBackground.SetWidth( Size.X() );
	m_obBackground.SetHeight( Size.Y() );
	m_obBackground.SetColour( CVector(1.0f,1.0f,1.0f,m_fBackgroundAlpha).GetNTColor() );
	m_obBackground.SetTexture( m_pcBackgroundName );


	ntAssert ( m_obImage.GetTextureHeight() > 0.0f );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::Render()
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiE3SkinImageButton::Render( void )
{
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
	m_obImageOff.Render();
	m_obBackground.Render();
	m_obImage.Render();

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

#ifndef _GOLD_MASTER
	if ( m_pcStringText )
	{
		float fBBWidth = CGuiManager::Get().BBWidth();
		float fBBHeight = CGuiManager::Get().BBHeight();

		g_VisualDebug->Printf2D(m_BasePosition.X()*fBBWidth, m_BasePosition.Y()*fBBHeight, DC_WHITE,0,m_pcStringText);
	}
#endif

	return true;	
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::SelectAction()
*
*	DESCRIPTION		This button should do its stuff now
*
***************************************************************************************************/

bool	CGuiE3SkinImageButton::SelectAction( int iPads )
{
	UNUSED(iPads);

	// There are no actions on this button
	if (m_iActionFlags == 0)
		return false;

	bool bActedOn = false;

	if (m_iActionFlags & MOVE_SCREENTYPE_BACK)
	{
		if (CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);

		CGuiManager::Get().MoveBackScreenType( m_iActionParam );

		if (! CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);


		bActedOn = true;
	}

	if (m_iActionFlags & MOVE_SCREENTYPE_ON)
	{
		if (CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);

		CGuiManager::Get().MoveOnScreenType( m_iActionParam );

		if (! CGuiSoundManager::Get().SoundFirst() )
				CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);


		bActedOn = true;
	}

	if (m_iActionFlags & MOVE_SCREEN_BACK)
	{
		if (CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);

		CGuiManager::Get().MoveBackScreen( m_iActionParam );

		if (! CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);

		bActedOn = true;
	}

	if (m_iActionFlags & MOVE_SCREEN_ON)
	{
		if (CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);

		CGuiManager::Get().MoveOnScreen( m_iActionParam );

		if (! CGuiSoundManager::Get().SoundFirst() )
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);

		bActedOn = true;
	}

	if (m_iActionFlags & LOAD_LEVEL)
	{
		CGuiManager::LoadGameLevel_Name( m_pcActionParam, -1, 0 );

		// bActedOn false as we want our containing select to move us on a screen
		bActedOn = false;
	}

	return bActedOn;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::StartAction()
*
*	DESCRIPTION		This button should do its stuff now
*
***************************************************************************************************/

bool	CGuiE3SkinImageButton::StartAction( int iPads )
{
	return SelectAction(iPads);
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::SetStateFocusIn
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageButton::SetStateFocusIn( void )
{
	CGuiUnit::SetStateFocusIn();
	m_obFadeTime.Set( ANIM_BLEND_TIME );
	m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageOffAlpha).GetNTColor() );
	m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,0.0f).GetNTColor() );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::UpdateFocusIn
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageButton::UpdateFocusIn( void )
{
	// Take away the time change
	m_obFadeTime.Update();

	// If we have finished 
	if ( m_obFadeTime.Passed() )
	{
		SetStateFocus();

		// Make sure we don't do a frame with FOCUS states settings
		m_fImageOffAlpha = 0.0f;
		m_fImageAlpha = 1.0f;
		m_fBackgroundAlpha = 0.0f;	

		m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageAlpha).GetNTColor() );
		m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageOffAlpha).GetNTColor() );
		m_obBackground.SetColour( CVector(1.0f,1.0f,1.0f,m_fBackgroundAlpha).GetNTColor() );
		return;
	}
	
	m_fImageAlpha = CMaths::SmoothStep( 1.0f - m_obFadeTime.NormalisedTime() );	
	m_fImageOffAlpha = CMaths::SmoothStep( m_obFadeTime.NormalisedTime() );

	m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageAlpha).GetNTColor() );
	m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageOffAlpha).GetNTColor() );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::SetStateFocusOut
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageButton::SetStateFocusOut( void )
{
	CGuiUnit::SetStateFocusOut();
	m_obFadeTime.Set( ANIM_BLEND_TIME );
	m_obBackground.SetColour( CVector(1.0f,1.0f,1.0f,m_fBackgroundAlpha).GetNTColor() );
	m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,0.0f).GetNTColor() );
	m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,1.0f).GetNTColor() );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::UpdateFocusOut
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageButton::UpdateFocusOut( void )
{
	// Take away the time change
	m_obFadeTime.Update();

	// If we have finished 
	if ( m_obFadeTime.Passed() )
	{
		SetStateIdle();

		// Make sure we don't do a frame with Idle states settings
		m_fBackgroundAlpha = 0.0f;	
		m_fImageAlpha = 0.0f;	
		m_fImageOffAlpha = 1.0f;

		m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageAlpha).GetNTColor() );
		m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageOffAlpha).GetNTColor() );
		m_obBackground.SetColour( CVector(1.0f,1.0f,1.0f,m_fBackgroundAlpha).GetNTColor() );
		return;
	}	

	m_fImageAlpha =  CMaths::SmoothStep( m_obFadeTime.NormalisedTime() );	
	m_fImageOffAlpha = CMaths::SmoothStep( 1.0f - m_obFadeTime.NormalisedTime() );

	m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageAlpha).GetNTColor() );
	m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageOffAlpha).GetNTColor() );

	m_obBackground.SetColour( CVector(1.0f,1.0f,1.0f,  m_fBackgroundAlpha*CMaths::SmoothStep( m_obFadeTime.NormalisedTime() ) ).GetNTColor() );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::SetStateFocus
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageButton::SetStateFocus( void )
{
	CGuiUnit::SetStateFocus();
	m_obFadeTime.Set( ANIM_BLEND_CYCLE );
	m_bFadeIn = true;

	m_fBackgroundAlpha = 0.0f;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::UpdateFocus
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageButton::UpdateFocus( void )
{
	CGuiUnit::UpdateFocus();
	
	// Take away the time change
	m_obFadeTime.Update();

	// If we have finished 
	if ( m_obFadeTime.Passed() )
	{
		m_obFadeTime.Reset();
		m_bFadeIn = !m_bFadeIn;
	}

	if (m_bFadeIn)
		m_fBackgroundAlpha = CMaths::SmoothStep( 1.0f - m_obFadeTime.NormalisedTime() ) * 0.5f + 0.5f;
	else
		m_fBackgroundAlpha = CMaths::SmoothStep(  m_obFadeTime.NormalisedTime() ) * 0.5f + 0.5f;	

	m_obBackground.SetColour( CVector(1.0f,1.0f,1.0f,m_fBackgroundAlpha).GetNTColor() );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::SetStateEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageButton::SetStateEnter( void )
{
	CGuiUnit::SetStateEnter();
	m_obFadeTime.Set( ANIM_BLEND_TIME );
	
	m_fImageOffAlpha = 0.0f;
	m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageOffAlpha).GetNTColor() );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::UpdateEnter
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageButton::UpdateEnter( void )
{
	// Take away the time change
	m_obFadeTime.Update();

	// If we have finished 
	if ( m_obFadeTime.Passed() )
	{
		SetStateIdle();
		m_fImageOffAlpha = 1.0f;
		m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageOffAlpha).GetNTColor() );
		return;
	}
	m_fImageOffAlpha = CMaths::SmoothStep( 1.0f - m_obFadeTime.NormalisedTime() );	

	m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageOffAlpha).GetNTColor() );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::SetStateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageButton::SetStateExit( void )
{
	CGuiUnit::SetStateExit();
	m_obFadeTime.Set( ANIM_BLEND_TIME );
	
	m_obImage.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageAlpha).GetNTColor() );
	m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageOffAlpha).GetNTColor() );
	m_obBackground.SetColour( CVector(1.0f,1.0f,1.0f,m_fBackgroundAlpha).GetNTColor() );

}

/***************************************************************************************************
*
*	FUNCTION		CGuiE3SkinImageButton::UpdateExit
*
*	DESCRIPTION		
*
***************************************************************************************************/
void	CGuiE3SkinImageButton::UpdateExit( void )
{	
	// Take away the time change
	m_obFadeTime.Update();

	// If we have finished 
	if ( m_obFadeTime.Passed() )
	{
		SetStateDead();
		m_fBackgroundAlpha = m_fImageAlpha = m_fImageOffAlpha = 0.0f;
		m_obImageOff.SetColour( CVector(1.0f,1.0f,1.0f,m_fImageOffAlpha).GetNTColor() );
		m_obBackground.SetColour( CVector(1.0f,1.0f,1.0f,m_fBackgroundAlpha).GetNTColor() );
		return;
	}
	m_fImageOffAlpha = CMaths::SmoothStep( m_obFadeTime.NormalisedTime() );	

	m_obImage.SetColour( CVector( 1.0f,1.0f,1.0f,m_fImageAlpha*CMaths::SmoothStep(m_obFadeTime.NormalisedTime() ) ).GetNTColor() );
	m_obImageOff.SetColour( CVector( 1.0f,1.0f,1.0f,m_fImageOffAlpha ).GetNTColor() );
	m_obBackground.SetColour( CVector( 1.0f,1.0f,1.0f, m_fBackgroundAlpha*CMaths::SmoothStep(m_obFadeTime.NormalisedTime() ) ).GetNTColor() );
}
