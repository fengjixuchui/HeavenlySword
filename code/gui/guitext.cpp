/***************************************************************************************************
*
*	DESCRIPTION		Functionality concerning strings and font use in the GUI
*
*	NOTES			This is the main text interface code avaialbe to the user
*
***************************************************************************************************/

// Includes
#include "gui/guitext.h"
#include "gui/guitextinternals.h"
#include "anim/transform.h"

// For game data
#include "hud/hudmanager.h"		
#include "hud/messagedata.h"

#define PRESENTATION_START_TAG	( 0x003C )	// <
#define PRESENTATION_END_TAG	( 0x003E )	// >
#define PRESENTATION_CLOSE_TAG	( 0x002F )	// /
#define STRING_IDENTIFIER_TAG	( 0x0024 )	// $
#define GAME_DATA_TAG			( 0x0025 )	// %
#define SPECIAL_CHAR_TAG		( 0x005C )  // 

#define WORLDSPACE_TEXT_SCALE ( 0.1f )

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* FontConstructWrapper( void ) { return NT_NEW CFontDef(); }

// Register this class under it's XML tag
bool g_bFONT = CGuiManager::Register( "FONT", &FontConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CFontDef::CFontDef
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CFontDef::CFontDef( void )
{
}

CFontDef::CFontDef( CFont::FONT_TYPE eFontType, const char* pcGlyphInfoFile, const char* pcRenderableFile ) 
{ 
	m_iFontType = (int)eFontType; 
	
	char* pcTempString = NT_NEW char[ strlen( pcGlyphInfoFile) + 1 ];
	strcpy( pcTempString, pcGlyphInfoFile );
	m_pcGlyphInfoFile = pcTempString;

	pcTempString = NT_NEW char[ strlen( pcRenderableFile) + 1 ];
	strcpy( pcTempString, pcRenderableFile );
	m_pcRenderableFile = pcTempString;
};

/***************************************************************************************************
*
*	FUNCTION		CFontDef::~CFontDef
*
*	DESCRIPTION		Desruction
*
***************************************************************************************************/

CFontDef::~CFontDef( void )
{
	NT_DELETE_ARRAY(m_pcName); 
	NT_DELETE_ARRAY(m_pcGlyphInfoFile); 
	NT_DELETE_ARRAY(m_pcRenderableFile);
}


/***************************************************************************************************
*
*	FUNCTION		CFontDef::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CFontDef::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CXMLElement::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "name" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcName );
		}

		if ( strcmp( pcTitle, "type" ) == 0 )
		{
			return GuiUtil::SetFlags(pcValue, &astrFontType[0], &m_iFontType);
		}

		if ( strcmp( pcTitle, "glyph" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcGlyphInfoFile );
		}

		if ( strcmp( pcTitle, "resource" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcRenderableFile );
		}

		return false;
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CFontDef::ProcessEnd
*
*	DESCRIPTION		Called when we reach the end on the defining XML tag.  We know that we are fully 
*					defined here so we can do any final setup.
*
***************************************************************************************************/

bool CFontDef::ProcessEnd( void )
{
	ntAssert( m_pcName );
	ntAssert( m_pcGlyphInfoFile );
	ntAssert( m_pcRenderableFile );

	return true;
}

/***************************************************************************************************
*
*	The bits below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiFontList(); }

// Register this class under it's XML tag
bool g_bFONTLIST	= CGuiManager::Register( "FONTLIST", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiFontList::CGuiFontList
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiFontList::CGuiFontList( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CGuiFontList::~CGuiFontList
*
*	DESCRIPTION		Destruction - the destruction of child elements is dealt with by the underlying
*					CXMLElement class.
*
***************************************************************************************************/

CGuiFontList::~CGuiFontList( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CGuiFontList::ProcessAttribute
*
*	DESCRIPTION
*
***************************************************************************************************/

bool CGuiFontList::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CXMLElement::ProcessAttribute( pcTitle, pcValue ) )
	{
		/*if ( strcmp( pcTitle, "filename" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcFileName );
		}

		else if ( strcmp( pcTitle, "skipback" ) == 0 )
		{
			return GuiUtil::SetInt( pcValue, &m_iSkipback );
		}

		else if ( strcmp( pcTitle, "flags" ) == 0 )
		{
			return GuiUtil::SetFlags( pcValue, &astrScreenFlags[0], &m_iScreenFlags );
		}

		return false;*/
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiFontList::ProcessChild
*
*	DESCRIPTION
*
***************************************************************************************************/

bool CGuiFontList::ProcessChild( CXMLElement* pobChild )
{
	// Drop out if the data is shonky
	ntAssert( pobChild );

	m_obFontList.push_back( ( CFontDef* )pobChild );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiFontList::ProcessEnd
*
*	DESCRIPTION		Called when we reach the end on the defining XML tag.  We know that we are fully 
*					defined here so we can do any final setup.
*
***************************************************************************************************/

bool CGuiFontList::ProcessEnd( void )
{
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CString::CString
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/

CString::CString( const WCHAR_T* pwcStringContents, CStringDefinition& obDefinition, Transform* pobTransform, CGuiUnit::RENDER_SPACE eRenderSpace )
{
	// Initialise the bounds exceeded flag
	m_bExceedsBounds = false;

	// Save the definition details
	m_obStringDefinition = obDefinition;

	m_eRenderSpace = eRenderSpace;

	// Set up a basic character definition
	CCharacterDefinition obCharDef;

	obCharDef.m_fCharacterScale = ( m_eRenderSpace == CGuiUnit::RENDER_WORLDSPACE ) ? WORLDSPACE_TEXT_SCALE : obDefinition.m_fOverallScale;
	m_fStringScale = obCharDef.m_fCharacterScale;

	obCharDef.m_iCharType = obDefinition.m_pobFont->GetFontType();
	obCharDef.m_eRenderSpace = eRenderSpace;

	// Create and initialise a transform for this string using the defined offsets
	CPoint obBasePoint( obDefinition.m_fXOffset, obDefinition.m_fYOffset, obDefinition.m_fZOffset );	
	CMatrix obBaseMatrix( CONSTRUCT_IDENTITY );
	obBaseMatrix.SetTranslation( obBasePoint );
	m_pobTransform = NT_NEW Transform();
	m_pobTransform->SetLocalMatrix( obBaseMatrix );

	// Set this tranform as a child of the one we have been handed
	pobTransform->AddChild( m_pobTransform );

	// Build all the characters
	BuildCharacters( pwcStringContents, obCharDef );

	// Layout all the characters
	LayoutCharacters();
}


/***************************************************************************************************
*
*	FUNCTION		CString::~CString
*
*	DESCRIPTION		Destructor
*
***************************************************************************************************/

CString::~CString( void )
{
	// Go through and destroy all our characters
	for( ntstd::List< CCharacter* >::iterator obIt = m_obCharacters.begin(); obIt != m_obCharacters.end(); ++obIt)
	{
		NT_DELETE( ( *obIt ) );
	}

	// Clear the pointers from the list
	m_obCharacters.clear();

	// Remove string transform
	m_pobTransform->RemoveFromParent();
	NT_DELETE( m_pobTransform );
}


/***************************************************************************************************
*
*	FUNCTION		CString::BuildCharacters
*
*	DESCRIPTION		This is a recursive function that scans through a WCHAR_T string and builds the
*					defined CCharacters to add to the member list.  The 'recursiveness' allows us
*					to endlessly embed string identifiers within strings.
*
*					This is all written to maximise readability and maintainability at this stage - 
*					i'm sure this code could be reduced but i don't think it will be an issue.
*
***************************************************************************************************/

void CString::BuildCharacters( const WCHAR_T* pwcChars, CCharacterDefinition& obCharDef )
{
	// Loop through the given string until we reach the nullness
	for ( int iChar = 0; pwcChars[iChar]; iChar++ )
	{
		if ( pwcChars[iChar] == PRESENTATION_START_TAG )
		{
			// This can set or unset a value - check if we are 'unsetting'
			if ( ( pwcChars[iChar + 1] ) && ( pwcChars[iChar + 1] == PRESENTATION_CLOSE_TAG ) )
			{
				// Set up the string in which to catch our info
				WCHAR_T pwcTagDetails[MAX_PATH] = { 0 };

				int iStart = iChar+1;
				do iChar++; while ( pwcChars[iChar] != PRESENTATION_END_TAG );
				wcsncpy( pwcTagDetails, &(pwcChars[iStart]), iChar-iStart );

				int iPresentFlag = 0;
				// Find what this tag means to a character
				GuiUtil::SetFlags( pwcTagDetails, &astrMarkupFlags[0], &iPresentFlag);

				// Make sure this is set - otherwise there is something screwy with our resources
				ntAssert ( obCharDef.m_iCharacterFlags & iPresentFlag );
				obCharDef.m_iCharacterFlags &= ~iPresentFlag;
				
			}

			else
			{
				// Here we could either have a number or some markup tag
				// Check for a number first because it can fail, signifying markup

				// Set up the string in which to catch our info
				WCHAR_T pwcTagDetails[MAX_PATH] = { 0 };

				int iStart = iChar+1;
				do iChar++; while ( pwcChars[iChar] != PRESENTATION_END_TAG );
				wcsncpy( pwcTagDetails, &(pwcChars[iStart]), iChar-iStart );

				WCHAR_T pwcScaleFormat[] = { '%', 'f', 0x0000 };
				float fScalar = 0.0f;

				if (  swscanf( (const wchar_t*)&pwcTagDetails[0], (const wchar_t*)pwcScaleFormat, &fScalar ) == 1 )	
				{
					// Check that the value is sensible then store it
					ntAssert ( fScalar > 0.0f );
					obCharDef.m_fCharacterScale = ( m_eRenderSpace == CGuiUnit::RENDER_WORLDSPACE ) ? WORLDSPACE_TEXT_SCALE * fScalar : fScalar;
				}

				else 
				{
					int iPresentFlag = 0;
					// Find what this tag means to a character
					GuiUtil::SetFlags( pwcTagDetails, &astrMarkupFlags[0], &iPresentFlag);

					// Make sure this is not currently set - otherwise there is something screwy with our resources
					ntAssert ( !( obCharDef.m_iCharacterFlags & iPresentFlag ) );
					obCharDef.m_iCharacterFlags |= iPresentFlag;
				}
			}
		}

		else if ( pwcChars[iChar] == STRING_IDENTIFIER_TAG )
		{
			// Set up the string in which to catch our info
			WCHAR_T pwcNestedStringID[MAX_PATH] = { 0 };

			int iStart = iChar+1;
			do iChar++; while ( pwcChars[iChar] != STRING_IDENTIFIER_TAG );
			wcsncpy( pwcNestedStringID, &(pwcChars[iStart]), iChar-iStart );

			// Get the extra contents to add to this string
			const WCHAR_T* pwcNestedStringContents = CStringManager::Get().GetResourceString( pwcNestedStringID );//CLanguage::Get().GetResourceString( pwcNestedStringID );
			ntAssert( pwcNestedStringContents );

			// Recurse - we just keep adding characters to this strings internal list
			BuildCharacters( pwcNestedStringContents, obCharDef );
		}

		else if ( pwcChars[iChar] == SPECIAL_CHAR_TAG )
		{
			
			// Set up the string in which to catch our info
			WCHAR_T pwcSpecialID[MAX_PATH] = { 0 };

			int iStart = iChar+1;
			do iChar++; while ( pwcChars[iChar] != SPECIAL_CHAR_TAG );
			wcsncpy( pwcSpecialID, &(pwcChars[iStart]), iChar-iStart );
		
			// Find what this special character is
			obCharDef.m_wcCharacter = 0;
			GuiUtil::SetEnum( pwcSpecialID, &astrSpecialChars[0], &obCharDef.m_iSpecialCharacter);
			obCharDef.m_bSpecialCharacter = true;
			// Set up the character
			obCharDef.m_pobFont = m_obStringDefinition.m_pobFont;
			CCharacter* pobCharacter = NT_NEW CCharacter( obCharDef );

			// Pop it on the list, or is that push
			m_obCharacters.push_back( pobCharacter );

			// Make sure the carriage return flag is cleared
			obCharDef.m_iCharacterFlags &= ~CCharacterDefinition::CHARACTER_LINE_BREAK;

			// Make sure the special charater is cleared
			obCharDef.m_iSpecialCharacter = 0;
			obCharDef.m_bSpecialCharacter = false;
		}

		else if ( pwcChars[iChar] == GAME_DATA_TAG )
		{
			// Set up the string in which to catch our info
			char pcNestedString[MAX_PATH] = { 0 };
			
			int iPos = 0;

			// drop the first % tag
			iChar++;

			do
			{
				pcNestedString[iPos] = (char)(pwcChars[iChar] & 0x000000FF);
				iChar++;	
				iPos++;
			}
			while ( pwcChars[iChar] != GAME_DATA_TAG );
			
			// Get the extra contents to add to this string
			WCHAR_T pwcNestedStringContents[MAX_PATH] = { 0 };

			MessageDataManager* pobMDH = NULL;
			if (m_obStringDefinition.m_pobMessageDataManager)
				pobMDH = m_obStringDefinition.m_pobMessageDataManager;
			else if ( CHud::Exists() && CHud::Get().GetMessageDataManager() )
				pobMDH = CHud::Get().GetMessageDataManager();

			if ( pobMDH )
			{
				CHashedString obKey(pcNestedString);
				if ( pobMDH->ToStringW( obKey, pwcNestedStringContents, MAX_PATH ) )		
				{
					// Recurse - we just keep adding characters to this strings internal list
					BuildCharacters( pwcNestedStringContents, obCharDef );
				}
			}
		}

		else
		{
			// Set up the character
			obCharDef.m_wcCharacter = pwcChars[iChar];
			obCharDef.m_pobFont = m_obStringDefinition.m_pobFont;
			CCharacter* pobCharacter = NT_NEW CCharacter( obCharDef );

			// Pop it on the list, or is that push
			m_obCharacters.push_back( pobCharacter );

			// Make sure the carriage return flag is cleared
			obCharDef.m_iCharacterFlags &= ~CCharacterDefinition::CHARACTER_LINE_BREAK;
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CString::LayoutCharacters
*
*	DESCRIPTION		OK - this is how the layout works.  The script folk give us a base transform and
*					a point which describes the offset of the string from that transform.  
*
*					The point has a different effect depending on the string style justification.
*					Hopefully this point will be the most useful point with which to position a 
*					string justified in the given manner.  If the string is justified top-left then
*					the point will decribed the top-left point from which the string 'hangs'.  If 
*					the string is justified middle centre then the point will describe the position
*					about which the string is positioned.
*
***************************************************************************************************/

void CString::LayoutCharacters( void )
{
	// Dynamically format the string if necessary
	if ( m_obStringDefinition.m_bDynamicFormat )
		DynamicallyFormat();

	// Create a point with which to position our characters
	CPoint obCharacterOffset( 0.0f, 0.0f, 0.0f );

	// Set the vertical offset of our first character
	switch ( m_obStringDefinition.m_eVJustify )
	{
	case CStringDefinition::JUSTIFY_TOP:
		if (m_eRenderSpace == CGuiUnit::RENDER_WORLDSPACE)
			obCharacterOffset.Z() = 0.0f;
		else
			obCharacterOffset.Y() = 0.0f;
		break;
	case CStringDefinition::JUSTIFY_MIDDLE:	
		if (m_eRenderSpace == CGuiUnit::RENDER_WORLDSPACE)
			obCharacterOffset.Z() = -( GetStringHeight( m_obCharacters.begin() ) * 0.5f );
		else
			obCharacterOffset.Y() = -( GetStringHeight( m_obCharacters.begin() ) * 0.5f );
		break;
	case CStringDefinition::JUSTIFY_BOTTOM:	
		if (m_eRenderSpace == CGuiUnit::RENDER_WORLDSPACE)
			obCharacterOffset.Z() = - GetStringHeight( m_obCharacters.begin() );
		else
			obCharacterOffset.Y() = - GetStringHeight( m_obCharacters.begin() );
		break;
	default:								
		ntAssert( 0 );																						
		break;
	}

	// Need to account for the height of the string when justifying text
	float fLineHeight = m_obStringDefinition.m_pobFont->GetFontLineHeight() * m_obStringDefinition.m_fLineSpacingMultiplier * m_fStringScale;

	if (m_eRenderSpace == CGuiUnit::RENDER_WORLDSPACE)
		obCharacterOffset.Z() += 0.5f * fLineHeight;
	else
		obCharacterOffset.Y() += 0.5f * fLineHeight;

	// 

	// Now go through our list of characters on set them in position
	for( ntstd::List< CCharacter* >::iterator obCharacter = m_obCharacters.begin(); obCharacter != m_obCharacters.end(); ++obCharacter)
	{
		// If this is a new line - or the first - set a horizontal offset based on justification
		if ( ( ( *obCharacter )->CharacterStartsLine() ) || ( obCharacter == m_obCharacters.begin() ) )
		{
			switch ( m_obStringDefinition.m_eHJustify )
			{
			case CStringDefinition::JUSTIFY_LEFT:	
				obCharacterOffset.X() = 0.0f;										
				break;
			case CStringDefinition::JUSTIFY_CENTRE:	
				obCharacterOffset.X() = -( GetLineLength( obCharacter ) * 0.5f);	
				break;
			case CStringDefinition::JUSTIFY_RIGHT:	
				obCharacterOffset.X() = -GetLineLength( obCharacter );				
				break;
			default:								
				ntAssert( 0 );														
				break;
			}
		}

		// If this is a new line then drop down to the next line
		if ( ( *obCharacter )->CharacterStartsLine() )
		{
			if (m_eRenderSpace == CGuiUnit::RENDER_WORLDSPACE)
				obCharacterOffset.Z() += m_obStringDefinition.m_pobFont->GetFontLineHeight() * m_obStringDefinition.m_fLineSpacingMultiplier * m_fStringScale;
			else
				obCharacterOffset.Y() += m_obStringDefinition.m_pobFont->GetFontLineHeight() * m_obStringDefinition.m_fLineSpacingMultiplier * m_fStringScale;
		}

		// Place this character
		( *obCharacter )->PlaceCharacter( m_pobTransform, obCharacterOffset );

		// Move across to the horizontal position of the next character
		obCharacterOffset.X() += GetEffectiveCharacterWidth( obCharacter );
		
	}
}


/***************************************************************************************************
*
*	FUNCTION		CString::DynamicallyFormat
*
*	DESCRIPTION		This function checks the line lengths of our string against the width of the 
*					given bounding area.  If any line exceeds the width it tries to amiliorate the
*					situation with extra line breaks.  
*
*					The height of the given bounds is ignored.  If this is exceeded it will be 
*					picked up elsewhere.
*
***************************************************************************************************/

void CString::DynamicallyFormat( void )
{
	// Make sure we are only doing this when we should be
	ntAssert( m_obStringDefinition.m_bDynamicFormat );

	// Run through all our characters and reformat if necessary
	for ( ntstd::List<CCharacter*>::iterator obIt = m_obCharacters.begin(); obIt != m_obCharacters.end(); obIt = GetNextLineStart( obIt ) )
	{
		// If this line is too long try and add a new break
		if ( GetLineLength( obIt ) > m_obStringDefinition.m_fBoundsWidth )
		{
			InsertDynamicBreak( obIt );
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CString::InsertDynamicBreak
*
*	DESCRIPTION		This will break the string at the first space inside the string bounding width.
*					It may be that it is not possible to break the line as required - for example 
*					if there is a long word taking one whole line.
*				
*					This function does not need to flag any failure to break the line.  Any 
*					unavoidable bounding breakage will be picked up when the string is positioned.
*
***************************************************************************************************/

void CString::InsertDynamicBreak( ntstd::List< CCharacter* >::iterator obStart )
{
	// We need to find the point where the string goes out of bounds
	float fLineLength = 0.0f;

	// Create a new iterator - we'll need the one give for reference later on
	ntstd::List<CCharacter*>::iterator obCharacter = obStart;

	// Count along the string until we exceed the given width
	for( ; obCharacter != m_obCharacters.end(); obCharacter++ )
	{
		// Find the line length so far
		fLineLength += GetEffectiveCharacterWidth( obCharacter );

		// Drop out when we have just gone too far
		if ( fLineLength >= m_obStringDefinition.m_fBoundsWidth ) break;
	}

	// Now travel back along the string
	for( ; obCharacter != obStart; obCharacter-- )
	{
		// Find the first space character
		if ( ( *obCharacter )->CharacterIsSpace() )
		{
			// Move back again to the next character
			obCharacter++;

			// Set a new break on this character
			( *obCharacter )->SetAsLineBreak();

			// Move back to the space
			obCharacter--;

			// Get rid of the space
			m_obCharacters.erase( obCharacter );

			// Drop out of here
			break;
		}
	}

	// If we haven't found anything it doesn't matter - just drop out
	return;
}


/***************************************************************************************************
*
*	FUNCTION		CString::GetNextLineStart
*
*	DESCRIPTION		Returns a pointer to the start of the next line - if there is one.  If not it
*					will return an iterator pointing to the list end.
*
***************************************************************************************************/

ntstd::List<CCharacter*>::iterator CString::GetNextLineStart( ntstd::List< CCharacter* >::iterator obCharacter )
{
	if ( obCharacter == m_obCharacters.end() )
		return obCharacter;

	// Count through till we get to the end or to a break - skip the first character
	for ( obCharacter++; obCharacter != m_obCharacters.end(); obCharacter++ )
	{
		if ( ( *obCharacter )->CharacterStartsLine() ) 
			break;
	}

	return obCharacter;
}


/***************************************************************************************************
*
*	FUNCTION		CString::GetLineLength
*
*	DESCRIPTION		Finds the length of the line starting at the given character.  The line either
*					ends at the end of the string or at the next character with a breakpoint marker.
*
***************************************************************************************************/
float CString::GetLineLength( ntstd::List<CCharacter*>::iterator obCharacter )
{
	// Initialise our return value
	float fLineLength = 0.0f;

	// Find the point we moving to
	ntstd::List<CCharacter*>::iterator obEnd = GetNextLineStart( obCharacter );

	// Count along the string until we reach a break or the end
	for( ; obCharacter != obEnd; obCharacter++ )
	{
		// Add the character width
		fLineLength += GetEffectiveCharacterWidth( obCharacter );
	}

	return fLineLength;
}

/***************************************************************************************************
*
*	FUNCTION		CString::GetEffectiveCharacterWidth
*
*	DESCRIPTION		Gets the width of the character pointed to by the iterator.  If the character
*					is a space it uses the word spacing multiplier specified in the strings style.
*
***************************************************************************************************/

float CString::GetEffectiveCharacterWidth( ntstd::List< CCharacter* >::iterator obCharacter )
{
	// The return value
	float fEffectiveCharacterWidth = 0.0f;

	// For space characters
	if ( ( *obCharacter )->CharacterIsSpace() )
		fEffectiveCharacterWidth = ( ( *obCharacter )->GetTotalCharacterWidth() * m_obStringDefinition.m_fWordSpacingMultiplier );

	// For all other characters
	else
		fEffectiveCharacterWidth = ( *obCharacter )->GetTotalCharacterWidth();

	// Give it to them
	return fEffectiveCharacterWidth;
}


/***************************************************************************************************
*
*	FUNCTION		CString::GetStringHeight
*
*	DESCRIPTION		Gets the height of the string starting from the given iterator
*
***************************************************************************************************/

float CString::GetStringHeight( ntstd::List< CCharacter* >::iterator obCharacter )
{
	// We need to count the number of lines in this string - there is always one
	int iLines = 1;

	// Get the height of a line from the font
	float fLineHeight = m_obStringDefinition.m_pobFont->GetFontLineHeight();

	// Loop through all the characters and count those with new line markers
	for ( ; obCharacter != m_obCharacters.end(); obCharacter++ )
	{
		if ( ( *obCharacter )->CharacterStartsLine() )
			iLines++;
	}

	// Lines * Height of each line * Style line height multiplier
	return ( iLines * fLineHeight * m_obStringDefinition.m_fLineSpacingMultiplier * m_fStringScale);
}	


/***************************************************************************************************
*
*	FUNCTION		CString::Render
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CString::Render( void )
{
	// Run through all our characters and render
	for ( ntstd::List<CCharacter*>::iterator obIt = m_obCharacters.begin(); obIt != m_obCharacters.end(); obIt++ )
	{
		(*obIt)->Render();
	}
}	

/***************************************************************************************************
*
*	FUNCTION		CString::SetColour
*
*	DESCRIPTION		Change the colour of our text for special effects
*
***************************************************************************************************/

void CString::SetColour( CVector& obColour )
{
	// Run through all our characters and render
	for ( ntstd::List<CCharacter*>::iterator obIt = m_obCharacters.begin(); obIt != m_obCharacters.end(); obIt++ )
	{
		(*obIt)->SetColour( obColour );
	}
}	

/***************************************************************************************************
*
*	FUNCTION		CString::GetColour
*
*	DESCRIPTION		Get the text colour.
*
***************************************************************************************************/

const CVector CString::GetColour( void )
{
	if	( false == m_obCharacters.empty() )
{
		// Just get the colour of the first character.
		CCharacter *pobCharacter = m_obCharacters.front();

		return pobCharacter->GetColour();
	}

	// Return the colour as white if no chars.
	return CVector( 1.0f, 1.0f, 1.0f, 1.0f );
}	

/***************************************************************************************************
*
*	FUNCTION		CString::RenderWidth
*
*	DESCRIPTION		
*
***************************************************************************************************/
float CString::RenderWidth()
{
	float fMaxWidth = 0.0f;
	float fWidth = 0.0f;

	// Start point
	ntstd::List<CCharacter*>::iterator obCharacter = m_obCharacters.begin();

	// Find the point we moving to
	ntstd::List<CCharacter*>::iterator obEnd = GetNextLineStart( obCharacter );

	while ( obCharacter != m_obCharacters.end() )
	{
		fWidth = GetLineLength ( obCharacter );
		if ( fWidth > fMaxWidth )
		{
			fMaxWidth = fWidth;
		}
		
		obCharacter = obEnd;
		obEnd = GetNextLineStart( obCharacter );
	}

	return fMaxWidth;
}

/***************************************************************************************************
*
*	FUNCTION		CString::RenderHeight
*
*	DESCRIPTION		
*
***************************************************************************************************/
float CString::RenderHeight()
{
	return GetStringHeight ( m_obCharacters.begin() );
}

/***************************************************************************************************
*
*	FUNCTION		CStringManager::CStringManager
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/

CStringManager::CStringManager( void )
{
	m_pcLevelName = NULL;
	m_pobLevelStrings = m_pobGeneralStrings = NULL;

	SetLanguage( NT_SUBTITLE_EURO_ENGLISH );
}

/**
	Change level to the indicated level
*/
void CStringManager::LevelChange( const char *pcLanguagePath )
{
	// campf - map HS levels to LAMS data
	//
	//
	
	char acPath[255];
	acPath[0]=0;

	//printf("CStringManager::LevelChange() %s\n", pcLanguagePath);

	if ((stricmp(pcLanguagePath, "ch0/initialbattle") == 0) || (stricmp(pcLanguagePath, "armydemo/armydemo") == 0))
	{
		sprintf(acPath, "gui\\Prologue\\%s.inf", GetLanguageName());
	}
	else if (stricmp(pcLanguagePath, "purgatory/purgatory") == 0)
	{
        sprintf(acPath, "gui\\FRONT_END\\%s.inf", GetLanguageName());
	}
	else if (stricmp(pcLanguagePath, "ch1/fort") == 0)
	{
		sprintf(acPath, "gui\\Chapter_1\\%s.inf", GetLanguageName());
	}
	else if ((stricmp(pcLanguagePath, "ch2/walkways") == 0) || (stricmp(pcLanguagePath, "ch2b/walkways") == 0))
	{
		sprintf(acPath, "gui\\Chapter_2\\%s.inf", GetLanguageName());
	}
	else if (stricmp(pcLanguagePath, "ch3/temple") == 0)
	{
		sprintf(acPath, "gui\\Chapter_3\\%s.inf", GetLanguageName());
	}
	else if ((stricmp(pcLanguagePath, "ch4/escape") == 0) || (stricmp(pcLanguagePath, "CH4NEW/CH4") == 0))
	{
		sprintf(acPath, "gui\\Chapter_4\\%s.inf", GetLanguageName());
	}
	else if (stricmp(pcLanguagePath, "ch5/battlefield") == 0)
	{
		sprintf(acPath, "gui\\Chapter_5\\%s.inf", GetLanguageName());
	}
	else if (stricmp(pcLanguagePath, "ch6/kingbattle") == 0)
	{
		sprintf(acPath, "gui\\Chapter_6\\%s.inf", GetLanguageName());
	}

	if (acPath[0])
	{
		// store this for later
		m_pcLevelName = pcLanguagePath;

		delete m_pobLevelStrings;
		m_pobLevelStrings = NT_NEW CLanguage(acPath);
	}

	// Re-load the correct font for this 'region'
	CFontManager::Get().FreeFonts();
	CFontManager::Get().Initialise(m_strRegion);
}

/**
	Sets the current language and reloads all string/font resources.

	@note You will need to re-make any existing strings to see the changes.
*/
void CStringManager::SetLanguage(SubtitleLanguage iLanguage)
{
	m_eCurrentLangauge = iLanguage;

	char acStringPath[255];
	char *strFontPath;

	switch (m_eCurrentLangauge)
	{
		case NT_SUBTITLE_JAPANESE:
			strFontPath = "gui\\japanese\\fonts";
			m_strRegion = "japanese";
			break;

		case NT_SUBTITLE_CHINESE:
			strFontPath = "gui\\chinese\\fonts";
			m_strRegion = "chinese";
			break;

		case NT_SUBTITLE_KOREAN:
			strFontPath = "gui\\korean\\fonts";
			m_strRegion = "korean";
			break;

		default:
			strFontPath = "gui\\europe\\fonts";
			m_strRegion = "europe";
			break;
	}

	delete m_pobGeneralStrings;
	delete m_pobLevelStrings;

	m_pobGeneralStrings = NULL;
	m_pobLevelStrings = NULL;

	sprintf(acStringPath, "gui\\general\\%s.inf", GetLanguageName());

	// Reload all the strings in this new language
	m_pobGeneralStrings = NT_NEW CLanguage(acStringPath);

	// Only reload the level if we had one in the first place
	if (m_pcLevelName)
	{
		LevelChange(m_pcLevelName);
	}
}


/***************************************************************************************************
*
*	FUNCTION		CStringManager::~CStringManager
*
*	DESCRIPTION		Destructor
*
***************************************************************************************************/

CStringManager::~CStringManager( void )
{
	// Clear up any remaining strings if they are about
	for ( ntstd::List< CString* >::iterator obIt = m_obStrings.begin(); obIt != m_obStrings.end(); ++obIt )
	{
		NT_DELETE( ( *obIt ) );
	}

	// Remove the pointers
	m_obStrings.clear();

	// Clear up the string table
	delete m_pobGeneralStrings;
	delete m_pobLevelStrings;
}


/***************************************************************************************************
*
*	FUNCTION		CStringManager::Update
*
*	DESCRIPTION		This will have to deal with any updates that are required to all strings - such
*					as checks that need to be made for button representations.
*
***************************************************************************************************/

void CStringManager::Update( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CStringManager::MakeString
*
*	DESCRIPTION		
*
***************************************************************************************************/

CString* CStringManager::MakeString( const char* pcStringID, CStringDefinition& obStringDef, Transform* pobTransform, CGuiUnit::RENDER_SPACE eRenderSpace )
{
	// Get the string contents from the ID
	const WCHAR_T* pwcStringContents = GetResourceString( pcStringID );//CLanguage::Get().GetResourceString( pcStringID );

	// Make a new string
	return CStringManager::MakeString(pwcStringContents, obStringDef, pobTransform, eRenderSpace);
}

CString* CStringManager::MakeString( const WCHAR_T* pwcStringContents, CStringDefinition& obStringDef, Transform* pobTransform, CGuiUnit::RENDER_SPACE eRenderSpace  )
{
	// Make a new string
	CString* pobNewString = NT_NEW CString( pwcStringContents, obStringDef, pobTransform, eRenderSpace );

	// Keep hold of the string so we can manage them - as our job title indicates
	m_obStrings.push_front( pobNewString );

	return pobNewString;
}

/***************************************************************************************************
*
*	FUNCTION		CStringManager::DestroyString
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CStringManager::DestroyString( CString* pobString )
{
	// Look for the string in our list
	for ( ntstd::List< CString* >::iterator obIt = m_obStrings.begin(); obIt != m_obStrings.end(); ++obIt )
	{
		if ( ( *obIt ) == pobString ) 
		{
			// If we have it fre and remove
			NT_DELETE( ( *obIt ) );
			m_obStrings.erase( obIt );
			return;
		}
	}

	// We haven't found it if we are here
	ntAssert( 0 );
}

/**
	campf - This new function replaces CLanguage::GetResourceString() in terms of usage since
	there are now two strings files, both must be interrogated.
*/
const WCHAR_T* CStringManager::GetResourceString( const char* pcIdentifier )
{
	const WCHAR_T *pcString = NULL;
	
	// First look into the general strings file
	pcString = m_pobGeneralStrings->GetResourceString(pcIdentifier);
	if (!pcString)
	{
		// Then, look into the per level strings file
		if (m_pobLevelStrings)
		{
			pcString = m_pobLevelStrings->GetResourceString(pcIdentifier);
		}
	}

	#ifndef _MASTER
	if (pcString)
	{
		if (pcString[0] == 0)
		{
			// The string is 0 length, this means the translastion hasn't yet been made for that string, print a message saying so
			pcString = m_pobGeneralStrings->GetResourceString("GENERAL_MISSING_STRING");

			ntPrintf("CStringManager::GetResourceString(): no translations for %s in %s\n", pcIdentifier, CStringManager::Get().GetGeneralLanguage()->m_acLanguagePath);
		}
	}
	else
	{
		// The string ID supplied isn't present in the database, fetch a string saying so, instead of asserting
		pcString = m_pobGeneralStrings->GetResourceString("STRING_NOT_IN_LAMS");

		ntPrintf("CStringManager::GetResourceString(): string %s doesn't exist in LAMS!\n", pcIdentifier);
	}
	#endif

	return pcString;
}
/**
	campf - This new function replaces CLanguage::GetResourceString() in terms of usage since
	there are now two strings files, both must be interrogated.
*/
const WCHAR_T* CStringManager::GetResourceString( const WCHAR_T* wpcIdentifier )
{
	const WCHAR_T *pcString = NULL;

	// First look into the general strings file
	pcString = m_pobGeneralStrings->GetResourceString(wpcIdentifier);
	if (!pcString)
	{
		// Then, look into the per level strings file
		if (m_pobLevelStrings)
		{
			pcString = m_pobLevelStrings->GetResourceString(wpcIdentifier);
		}
	}

	#ifndef _MASTER
	if (pcString)
	{
		if (pcString[0] == 0)
		{
			// The string is 0 length, this means the translastion hasn't yet been made for that string, print a message saying so
			pcString = m_pobGeneralStrings->GetResourceString("GENERAL_MISSING_STRING");

			// convert to char from widechar
			char strBuffer[255];
			const int iLen = wcslen(wpcIdentifier);
			int i;
			for (i=0; i<iLen; i++) strBuffer[i] = (char)wpcIdentifier[i];
			strBuffer[i] = 0;

			ntPrintf("CStringManager::GetResourceString(): no translations for %s in %s\n", strBuffer, CStringManager::Get().GetGeneralLanguage()->m_acLanguagePath);
		}
	}
	else
	{
		// The string ID supplied isn't present in the database, fetch a string saying so, instead of asserting
		pcString = m_pobGeneralStrings->GetResourceString("STRING_NOT_IN_LAMS");

		// convert to char from widechar
		char strBuffer[255];
		const int iLen = wcslen(wpcIdentifier);
		int i;
		for (i=0; i<iLen; i++) strBuffer[i] = (char)wpcIdentifier[i];
		strBuffer[i] = 0;

		ntPrintf("CStringManager::GetResourceString(): string %s doesn't exist in LAMS!\n", strBuffer);
	}
	#endif

	return pcString;
}

/***************************************************************************************************
*
*	FUNCTION		CFontManager::CFontManager
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/

CFontManager::CFontManager( void )
{
	// Do special setup for special charaters
	CCharacterDefinition::Initialise();
}


/***************************************************************************************************
*
*	FUNCTION		CFontManager::~CFontManager
*
*	DESCRIPTION		Destructor
*
***************************************************************************************************/

CFontManager::~CFontManager( void )
{
	// Do special destruction for special charaters
	CCharacterDefinition::Release();

	FreeFonts();
}

/**
	Free all the loaded fonts.
*/
void CFontManager::FreeFonts(void)
{
	// Clear up any remaining strings if they are about
	for ( FontMapIt obIt = m_obFonts.begin(); obIt != m_obFonts.end(); ++obIt )
	{
		NT_DELETE( obIt->second );
	}

	// Remove the pointers
	m_obFonts.clear();
}

/***************************************************************************************************
*
*	FUNCTION		CFontManager::Update
*
*	DESCRIPTION		This will have to deal with any updates that are required to all strings - such
*					as checks that need to be made for button representations.
*
***************************************************************************************************/

void CFontManager::Update( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CFontManager::MakeFont
*
*	DESCRIPTION		
*
***************************************************************************************************/

/*CFont* CFontManager::MakeFont( void )
{
	// Make a new font
	CFont* pobNewFont = NT_NEW CFont( CFont::FONT_3D, "gui\\font.3dg", "data\\gui\\font.clump" );

	// Keep hold of the font so we can manage them - as our job title indicates
	m_obFonts[ GuiUtil::CalculateHash( "default" ) ] =  pobNewFont;

	return pobNewFont;
}*/

/***************************************************************************************************
*
*	FUNCTION		CFontManager::MakeFont
*
*	DESCRIPTION		
*
***************************************************************************************************/

CFont* CFontManager::MakeFont(CFontDef* pobFontDef, const char *strRegion)
{
	FontMapIt obIt = m_obFonts.find( GuiUtil::CalculateHash( pobFontDef->m_pcName ) );
	if ( obIt != m_obFonts.end() )
	{
		ntPrintf("Gui: Already have font %s\n", pobFontDef->m_pcName);
		return obIt->second;
	}

	// Make a new font

	// campf - temporarily build the font path dynamically from the config file path and font name (from fonts.xml)
	//
	// NOTE the first part of this if statement needs to go soon because those are debug fonts
	char acGlyph[255], acTexture[255];

	if ((stricmp(pobFontDef->m_pcName, "alan den") == 0) || 
		(stricmp(pobFontDef->m_pcName, "comic") == 0))
	{
		sprintf(acGlyph, "gui\\%s", pobFontDef->m_pcGlyphInfoFile);
		sprintf(acTexture, "gui\\%s", pobFontDef->m_pcRenderableFile);
	}
	else
	{
		// campf - japan is only allowed the body font
		if (CStringManager::Get().GetLanguage() == NT_SUBTITLE_JAPANESE ||
			CStringManager::Get().GetLanguage() == NT_SUBTITLE_KOREAN ||
			CStringManager::Get().GetLanguage() == NT_SUBTITLE_CHINESE)
		{
			if (strcmp( pobFontDef->m_pcName, "Body" ) != 0)
			{
				// if its not minion, don't allow it!
				return NULL;
			}
		}

		sprintf(acGlyph, "gui\\%s\\%s", strRegion, pobFontDef->m_pcGlyphInfoFile);
		sprintf(acTexture, "gui\\%s\\%s", strRegion, pobFontDef->m_pcRenderableFile);
	}

	CFont* pobNewFont = NT_NEW CFont( pobFontDef->m_iFontType, acGlyph, acTexture );

	// Keep hold of the font so we can manage them - as our job title indicates
	m_obFonts[ GuiUtil::CalculateHash( pobFontDef->m_pcName ) ] =  pobNewFont;

	return pobNewFont;
}

/***************************************************************************************************
*
*	FUNCTION		CFontManager::DestroyFont
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CFontManager::DestroyFont( CFont* pobFont )
{
	// Look for the string in our list
	for ( FontMapIt obIt = m_obFonts.begin(); obIt != m_obFonts.end(); ++obIt )
	{
		if ( obIt->second == pobFont ) 
		{
			// If we have it free and remove
			NT_DELETE( obIt->second );
			m_obFonts.erase( obIt );
			return;
		}
	}

	// We haven't found it if we are here
	ntAssert( 0 );
}

/***************************************************************************************************
*
*	FUNCTION		CFontManager::Initialise
*
*	DESCRIPTION		Get a named font from the manager
*
***************************************************************************************************/
CFont* CFontManager::GetFont( const char * pcFontName )
{
	FontMapIt obIt = m_obFonts.find( GuiUtil::CalculateHash( pcFontName ) );

	if ( obIt->second )
	{
#ifdef DEBUG_GUI
		ntPrintf("Gui: Found font %s\n", pcFontName);
#endif
		return obIt->second;
	}
	else
	{
#ifdef DEBUG_GUI
		ntPrintf("Gui: Error: Not found font %s\n", pcFontName);
#endif
		return 0;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CFontManager::Initialise
*
*	DESCRIPTION		Read in the font definitions from xml
*
***************************************************************************************************/

void CFontManager::Initialise(const char *strRegion)
{	
	// Import the font definitions
	CGuiFontList* pobFontList;
	pobFontList = static_cast< CGuiFontList* >( CXMLParse::Get().CreateTree( "gui\\fonts.xml" ) );

	for ( ntstd::List< CFontDef* >::iterator obIt = pobFontList->m_obFontList.begin(); obIt != pobFontList->m_obFontList.end(); ++obIt )
	{
		MakeFont(*obIt, strRegion);
	}
	
	NT_DELETE ( pobFontList );
}

