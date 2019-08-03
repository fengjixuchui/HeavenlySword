//-----------------------------------------------------------------------------
//!
//!	\file /game/credits.cpp
//!
//! Implementation of class to manage presentation of the credits screen.
//!
//-----------------------------------------------------------------------------

//-- Include files
#include "game/credits.h"
#include "gui/guitext.h"
#include "anim/hierarchy.h"
#include "gui/guimanager.h"
#include "gui/guisettings.h"
#include "objectdatabase/dataobject.h"

//-- Defines
//#define CREDITS_DEBUG
#define XML_CREDITS "XMLCredits"

//-- Statics
CHashedString CCredits::HashSN = CHashedString( "SN" );

//-- External functions
extern bool TryInstallXMLFile( const char* pcFile );

//-- XML credit data interface.
START_CHUNKED_INTERFACE( CXMLCredits, Mem::MC_MISC)
	PUBLISH_CONTAINER_AS( m_obXMLCredits, XMLCreditsDetail )
END_STD_INTERFACE

//-----------------------------------------------------------------------------
//!
//!	CCredits::CCredits
//! 
//-----------------------------------------------------------------------------
CCredits::CCredits() :
	m_bRunCredits( false ),
	m_fScrollSpeed( 30.0f ),
	m_bDisplayedAll( false ),
	m_fTopMargin( 200.0f ),
	m_fBottomMargin( 100.0f ),
	m_bAllowUpdate( false ),
	m_fInitialDelay( 0.0f )
{
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::Initialise
//! 
//-----------------------------------------------------------------------------
void CCredits::Initialise( void )
{
	m_bRunCredits = false;
	m_bDisplayedAll = false;
	m_fInitialDelay = 2.0f;
	m_bAllowUpdate = true;

	// Load credits data.
	TryInstallXMLFile( "data/gui/frontend/screens/specialfeatures/credits.xml" );
	m_pobXMLCredits = ObjectDatabase::Get().GetPointerFromName<CXMLCredits*>(CHashedString(XML_CREDITS));
	ntAssert_p( m_pobXMLCredits != NULL, ("Credits XML data could not be loaded.\n") );

	if ( NULL == m_pobXMLCredits )
	{
		return;
	}

	// First pass, create all credit lines and initialize positions.
	#ifdef CREDITS_DEBUG
		ntPrintf( "\n" );
		ntPrintf( "**********  XMLCredits Start **********\n" );
		ntPrintf( "\n" );
	#endif

	// Process credits data.  Create and position a credit line object for each entry.  Text is not created at this stage.
	int iXMLCreditLineCount = 0;
	for ( ntstd::List<CHashedString>::iterator obIt = m_pobXMLCredits->m_obXMLCredits.begin(); obIt != m_pobXMLCredits->m_obXMLCredits.end(); )
	{
		// Look for the SN, (S)ection (N)ame token.
		if ( (*obIt).Get() == CCredits::HashSN.Get() )
		{
			// Skip the SN token, next item is assumed to be the LAMS id for the section name.
			obIt++;

			if ( obIt != m_pobXMLCredits->m_obXMLCredits.end() )
			{
				// Add the section name.
				#ifdef CREDITS_DEBUG
					ntPrintf( "CREDITS SECTION NAME = %s.\n", ntStr::GetString( *obIt ) );
				#endif

				CCreditLine* pobCreditLine = NT_NEW CCreditLine;
				pobCreditLine->SetType( CCreditLine::SECTION_NAME );
				pobCreditLine->SetHashedString( (*obIt) );
				pobCreditLine->InitialisePosition( iXMLCreditLineCount++ );
				m_obCreditLines.push_back( pobCreditLine );
				obIt++;
			}

			// Assume a series of LAMS id's for credit names follows.  Does not deal with empty sections.
			// Keep reading credit names until the next section is found.
			while	( (obIt != m_pobXMLCredits->m_obXMLCredits.end()) && ((*obIt).Get() != CCredits::HashSN.Get()) )
			{
				#ifdef CREDITS_DEBUG
					ntPrintf( "    CREDITS LINE = %s.\n", ntStr::GetString( *obIt ) );
				#endif

				CCreditLine* pobCreditLine = NT_NEW CCreditLine;
				pobCreditLine->SetType( CCreditLine::PERSON_NAME );
				pobCreditLine->SetHashedString( (*obIt) );
				pobCreditLine->InitialisePosition( iXMLCreditLineCount++ );
				m_obCreditLines.push_back( pobCreditLine );
				obIt++;
			}

			// Ok, add blank line if this is the start of a new section.
			if ( (*obIt).Get() == CCredits::HashSN.Get() )
			{
				// Insert blank line.
				#ifdef CREDITS_DEBUG
					ntPrintf( "\n" );
				#endif

				CCreditLine* pobCreditLine = NT_NEW CCreditLine;
				pobCreditLine->SetType( CCreditLine::BLANK_LINE );
				pobCreditLine->InitialisePosition( iXMLCreditLineCount++ );
				m_obCreditLines.push_back( pobCreditLine );
			}
		}
		else
		{
			obIt++;
		}
	}

	// Finished with the XML credits data so destroy.
	ObjectDatabase::Get().DeleteContainer( ObjectDatabase::Get().GetPointerFromName<ObjectContainer*>( XML_CREDITS ) );
	m_pobXMLCredits = NULL;

	// Second pass, create string objects for credit lines that will be visible or just off screen at start.
	int iInitialiseStringCount = 0;
	for ( ntstd::List<CCreditLine*>::iterator obIt = m_obCreditLines.begin(); obIt != m_obCreditLines.end(); )
	{
		if ( (*obIt)->OnScreen() || (*obIt)->OnScreenSoon() )
		{
			(*obIt)->InitialiseString();
			iInitialiseStringCount++;
		}
		else
		{
			break;
		}

		obIt++;
	}

	#ifdef CREDITS_DEBUG
		ntPrintf( "XMLCredits - Credit lines found = %d.\n", iXMLCreditLineCount );
		ntPrintf( "\n" );
		ntPrintf( "Credit lines initialised with strings at start = %d.\n", iInitialiseStringCount );
		ntPrintf( "\n" );
		ntPrintf( "**********  XMLCredits End **********\n" );
		ntPrintf( "\n" );
	#endif
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::Update
//! 
//! Deal with rendering state and lifetime for credit lines on/off the screen.
//! 
//-----------------------------------------------------------------------------
void CCredits::Update( void )
{
	// Handle the initial delay.
	if	( m_fInitialDelay > 0.0f )
	{
		m_fInitialDelay -= CTimer::Get().GetGameTimeChange();

		m_bAllowUpdate = ( m_fInitialDelay < 0.0f );
	}

	if	( (false == m_bRunCredits) || (false == m_bAllowUpdate) )
	{
		return;
	}

	#ifdef CREDITS_DEBUG
		int iUpdatedCount = 0;
	#endif

	for ( ntstd::List<CCreditLine*>::iterator obIt = m_obCreditLines.begin(); obIt != m_obCreditLines.end(); )
	{
		if	( false == m_bRunCredits )
		{
			break;
		}

		(*obIt)->Update( m_fScrollSpeed * CTimer::Get().GetSystemTimeChange() );

		#ifdef CREDITS_DEBUG
			iUpdatedCount++;
		#endif

		if	(  (*obIt)->OffTop() )
		{
			(*obIt)->EnableRender( false );

			// Remove the credit line object.
			NT_DELETE( (*obIt) );

			// Remove from list.
			obIt = m_obCreditLines.erase( obIt );
		}
		else if	(  (*obIt)->OffBottom() )
		{
			// If nearly on screen, set the text ready for display.
			if ( (*obIt)->StringNeedsToBeInitialised() )
			{
				if ( (*obIt)->OnScreenSoon() )
				{
					(*obIt)->InitialiseString();
				}
			}

			(*obIt)->EnableRender( false );

			obIt++;
		}
		else
		{
			// On screen		
			(*obIt)->EnableRender( true );
			obIt++;
		}
	}

	// Detect end of credits.
	if	( m_obCreditLines.empty() )
	{
		m_bDisplayedAll = true;
	}

	#ifdef CREDITS_DEBUG
		ntPrintf( "Credit lines updated = %d.\n", iUpdatedCount );
	#endif
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::Render
//! 
//-----------------------------------------------------------------------------
void CCredits::Render( void )
{
	if	( (false == m_bRunCredits) )
	{
		return;
	}

	#ifdef CREDITS_DEBUG
		int iNumCreditsRendered = 0;
	#endif

	for ( ntstd::List<CCreditLine*>::iterator obIt = m_obCreditLines.begin(); obIt != m_obCreditLines.end(); obIt++ )
	{
		if	( false == m_bRunCredits )
		{
			break;
		}
		
		if	( (*obIt)->Render() )
		{
			#ifdef CREDITS_DEBUG
				iNumCreditsRendered++;
			#endif
		}
	}

	#ifdef CREDITS_DEBUG
		ntPrintf( "Credit lines rendered = %d.\n", iNumCreditsRendered );
	#endif
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::Run
//! 
//-----------------------------------------------------------------------------
void CCredits::Run( void )
{
	m_bRunCredits = true;
	m_bDisplayedAll = false;
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::Pause
//! 
//-----------------------------------------------------------------------------
void CCredits::Pause( void )
{
	m_bRunCredits = false;
	m_bDisplayedAll = false;
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::DisplayedAll
//! 
//-----------------------------------------------------------------------------
bool CCredits::DisplayedAll( void )
{
	return m_bDisplayedAll;
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::Terminate
//! 
//! \return	TRUE if the credits were Terminate.
//! 
//-----------------------------------------------------------------------------
bool CCredits::Terminate( void )
{
	// Only kill if the credits are running.
	if	( false == m_bRunCredits )
	{
		return false;
	}

	// Stop render and update.
	Pause();

	// Now destroy all resources.
	Destroy();

	return true;
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::Destroy
//! 
//-----------------------------------------------------------------------------
void CCredits::Destroy( void)
{
	m_bRunCredits = false;

	for ( ntstd::List<CCreditLine*>::iterator obIt = m_obCreditLines.begin(); obIt != m_obCreditLines.end(); )
	{
		// Remove the credit line object.
		NT_DELETE( ( *obIt ) );

		// Remove from list.
		obIt = m_obCreditLines.erase( obIt );
	}

	// Unload credits data.
	if ( m_pobXMLCredits )
	{
		ObjectDatabase::Get().DeleteContainer( ObjectDatabase::Get().GetPointerFromName<ObjectContainer*>( XML_CREDITS ) );
		m_pobXMLCredits = NULL;
	}
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::GetTopMargin
//! 
//-----------------------------------------------------------------------------
float CCredits::GetTopMargin( void )
{
	return m_fTopMargin;
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::GetBottomMargin
//! 
//-----------------------------------------------------------------------------
float CCredits::GetBottomMargin( void )
{
	return m_fBottomMargin;
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::GetScrollSpeed
//! 
//-----------------------------------------------------------------------------
float CCredits::GetScrollSpeed( void )
{
	return m_fScrollSpeed;
}

//-----------------------------------------------------------------------------
//!
//!	CCredits::Running
//! 
//! \return TRUE if the credits are being displayed. 
//! 
//-----------------------------------------------------------------------------
bool CCredits::Running( void )
{
	return m_bRunCredits;
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::CCreditLine
//! 
//-----------------------------------------------------------------------------
CCreditLine::CCreditLine() :
	m_pobBaseTransform( 0 ),
	m_BasePosition( CPoint( 0.0f, 0.0f, 0.0f ) ),
	m_pStr( 0 ),
	m_bRender( false ),
	m_fFontHeight( 0.0f )
{
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::~CCreditLine
//!
//-----------------------------------------------------------------------------
CCreditLine::~CCreditLine()
{
	// Destroy the string.
	if	( m_pStr )
	{
		CStringManager::Get().DestroyString( m_pStr );
	}

	// Deal with the base transform
	if ( m_pobBaseTransform )
	{
		m_pobBaseTransform->RemoveFromParent();
		NT_DELETE( m_pobBaseTransform );
	}
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::Update
//! 
//! Update the credit line position.
//! 
//-----------------------------------------------------------------------------
void CCreditLine::Update( const float fSpeed )
{
	// Calc and store new position for this credit line.
	CPoint obPosition( m_BasePosition.X(), m_BasePosition.Y() - fSpeed, m_BasePosition.Z() );
	m_BasePosition = obPosition;

	CMatrix obLocalMatrix( m_pobBaseTransform->GetLocalMatrix() );

	obLocalMatrix.SetTranslation( m_BasePosition );

	m_pobBaseTransform->SetLocalMatrix( obLocalMatrix );

	UpdateColour();
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::EnableRender
//! 
//! \param	bool, enable the render.
//! 
//-----------------------------------------------------------------------------
void CCreditLine::EnableRender( const bool bEnable )
{
	m_bRender = bEnable;
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::Initialise
//!
//! Set initial position, create string object.
//! 
//-----------------------------------------------------------------------------
void CCreditLine::InitialisePosition( const int iIndex )
{
	// Find out some info about the font to be used.
	CStringDefinition obStrDef;
	obStrDef.m_pobFont = CFontManager::Get().GetFont( CGuiManager::Get().GuiSettings()->BodyFont() );
	m_fFontHeight = obStrDef.m_pobFont->GetFontLineHeight();

	// Initial position, start credits in the center.
	float fX = 0.5f;
	float fY = 0.5f + ((float)iIndex * (m_fFontHeight/CGuiManager::Get().BBHeight()) );
	float fZ = 0.0f;

	// Create a point with the data
	CPoint obBasePoint( fX, fY, fZ );
	m_BasePosition = obBasePoint;

	// Create a matrix with the point
	CMatrix obBaseMatrix( CONSTRUCT_IDENTITY );
	obBaseMatrix.SetTranslation( obBasePoint );

	// Now set our transform object out of all that
	m_pobBaseTransform = NT_NEW Transform();
	m_pobBaseTransform->SetLocalMatrix( obBaseMatrix );

	// Position in screen space.
	m_BasePosition.X() *= CGuiManager::Get().BBWidth();
	m_BasePosition.Y() *= CGuiManager::Get().BBHeight();
	obBaseMatrix.SetTranslation( m_BasePosition );
	m_pobBaseTransform->SetLocalMatrix( obBaseMatrix );
	
	CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobBaseTransform );
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::InitialiseString
//!
//! Create string object.
//! 
//-----------------------------------------------------------------------------
void CCreditLine::InitialiseString( void )
{
	if ( m_pStr )
	{
		return;
	}

	if ( BLANK_LINE == m_eType )
	{
		// Do nothing.
	}
	else
	{
		// Get the string from LAMS.
		const WCHAR_T* pwcString = CStringManager::Get().GetResourceString( ntStr::GetString( m_HashedString ) );

		if ( pwcString )
		{
			CStringDefinition obStrDef;
			CVector obColour( 0.0f, 0.0f, 0.0f, 0.0f );

			// Select a font.
			switch	( m_eType )
			{
			case SECTION_NAME:
				obStrDef.m_pobFont = CFontManager::Get().GetFont( CGuiManager::Get().GuiSettings()->TitleFont() );
				obColour = CVector( 1.0f, 1.0f, 1.0f, 1.0f );
				break;

			default:
				obStrDef.m_pobFont = CFontManager::Get().GetFont( CGuiManager::Get().GuiSettings()->BodyFont() );
				obColour = CVector( 0.5f, 0.5f, 0.5f, 1.0f );
				break;
			}

			m_pStr = CStringManager::Get().MakeString( pwcString, obStrDef, m_pobBaseTransform, CGuiUnit::RENDER_SCREENSPACE );

			m_pStr->SetColour( obColour );
			UpdateColour();
		}
	}
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::OffTop
//!
//! Test to see if this credit line is off the top of the screen using screen
//! pixels.
//!
//! \return True if off bottom of the screen.
//!
//-----------------------------------------------------------------------------
const bool CCreditLine::OffTop( void )
{
	//ntPrintf("OffTop = %f.\n", CCredits::Get().GetTopMargin() + m_fFontHeight );

	if	( m_BasePosition.Y() < ( CCredits::Get().GetTopMargin() + m_fFontHeight ) )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::OffBottom
//!
//! Test to see if this credit line is off the bottom of the screen using screen
//! pixels.
//!
//! \return True if off bottom of the screen.
//!
//-----------------------------------------------------------------------------
const bool CCreditLine::OffBottom( void )
{
	//ntPrintf("OffBottom = %f.\n", CGuiManager::Get().BBHeight() - CCredits::Get().GetBottomMargin() );

	if	( m_BasePosition.Y() > ( CGuiManager::Get().BBHeight() - CCredits::Get().GetBottomMargin() ) )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::OnScreen
//!
//! Test to see if this credit line is on screen.
//!
//! \return True if on screen.
//!
//-----------------------------------------------------------------------------
const bool CCreditLine::OnScreen( void )
{
	if	( OffTop() || OffBottom() )
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::OnScreenSoon
//!
//! Test to see if the credit line will be in the display area soon.
//!
//! \return True if on screen soon.
//!
//-----------------------------------------------------------------------------
const bool CCreditLine::OnScreenSoon( void )
{
	float fDisplayBottom = CGuiManager::Get().BBHeight() - CCredits::Get().GetBottomMargin();

	if	( (m_BasePosition.Y() >= fDisplayBottom) &&  (m_BasePosition.Y() <= ( fDisplayBottom * 1.25f )) )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::Render
//!
//! \return True if rendered.
//!
//-----------------------------------------------------------------------------
const bool CCreditLine::Render( void )
{
	if	( m_bRender )
	{
		if	( m_pStr )
		{
			m_pStr->Render();
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::UpdateColour
//!
//! Modifies colour based on distance from the centre of the credits display
//! area.
//!
//-----------------------------------------------------------------------------
void CCreditLine::UpdateColour( void )
{
	if ( m_pStr )
	{
		// Change colourbased on distance from centre.
		float fDisplayTop = CCredits::Get().GetTopMargin();
		float fDisplayBottom = CGuiManager::Get().BBHeight() - CCredits::Get().GetBottomMargin();
		float fDisplayCenter = (fDisplayBottom + fDisplayTop) * 0.5f;

		float fColourVal;

		if	( m_BasePosition.Y() > fDisplayCenter )
		{
			// Below center of display area. -m_fFontHeight to force fade a bit earlier.
			fColourVal = 1.0f - ((m_BasePosition.Y() - (fDisplayCenter-m_fFontHeight)) / (fDisplayBottom - (fDisplayCenter-m_fFontHeight)));
	
		}
		else if ( m_BasePosition.Y() < fDisplayCenter )
		{
			// Above center of display area. +m_fFontHeight to prolong fade.
			fColourVal = (m_BasePosition.Y() - (fDisplayTop+m_fFontHeight)) / (fDisplayCenter - (fDisplayTop+m_fFontHeight));
		}
		else
		{
			fColourVal = 1.0f;
		}

		// Clamp
		fColourVal = max( 0.0f, fColourVal );
		fColourVal = min( 1.0f, fColourVal );

		// Set colour, use original rgb and modified alpha.
		CVector obTextColour( m_pStr->GetColour() );
		CVector obColour( obTextColour.X(), obTextColour.Y(), obTextColour.Z(), fColourVal );
		m_pStr->SetColour( obColour );
	}
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::SetType
//!
//! Set this credits lines type.
//!
//-----------------------------------------------------------------------------
void CCreditLine::SetType( const eType Type )
{
	m_eType = Type;
}

//-----------------------------------------------------------------------------
//!
//!	CCreditLine::SetHashedString
//!
//! Set the hashedstring for this credit line.
//!
//-----------------------------------------------------------------------------
void CCreditLine::SetHashedString( const CHashedString HashedString )
{
	m_HashedString = HashedString;
}
