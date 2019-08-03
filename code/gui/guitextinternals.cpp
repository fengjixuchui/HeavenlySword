/***************************************************************************************************
*
*	DESCRIPTION		Functionality concerning strings and font use in the GUI
*
*	NOTES			This is the hidden away stuff - not to be included in anything other than
*					guitext.cpp
*
***************************************************************************************************/

// Includes
#include "guitextinternals.h"
#include "guiresource.h"
#include "gfx/clump.h"
#include "guimanager.h"
#include "gfx/listspace.h"
#include "gfx/meshinstance.h"
#include "core/exportstruct_clump.h"
#include "gfx/renderersettings.h"
#include "gfx/sector.h"
#include "gfx/texturemanager.h"
#include "gfx/renderer.h"
#include "gui/guitext.h"
#include "game/shellconfig.h"

static const WCHAR_T UnicodeSpace = 0x0020;

// Glyph information for button actions
CFont::GLYPH_ATTR_2D CCharacterDefinition::m_astrSpecialGlyphs[ CCharacterDefinition::SPECIAL_END ] = 
{
/*									{	left,	top,	right,	bottom,	offset,	width,	advance,	code }	*/
/* SPECIAL_TRIANGLE				*/	{	0.0f,	0.0f,	1.0f,	1.0f,	0,		256,	257,		L'\0'	},
/* SPECIAL_CIRCLE				*/	{	0.0f,	0.0f,	1.0f,	1.0f,	0,		256,	257,		L'\0'	},
/* SPECIAL_CROSS				*/	{	0.0f,	0.0f,	1.0f,	1.0f,	0,		256,	257,		L'\0'	},
/* SPECIAL_SQUARE				*/	{	0.0f,	0.0f,	1.0f,	1.0f,	0,		256,	257,		L'\0'	},	
};

// Textures for button actions
Texture::Ptr CCharacterDefinition::m_aobTexture[ CCharacterDefinition::SPECIAL_END ];

/***************************************************************************************************
*
*	FUNCTION		CCharacterDefinition::Initialise/Release
*
*	DESCRIPTION		Static functions to deal with the special textures for the action buttons
*
***************************************************************************************************/

void CCharacterDefinition::Initialise(void)
{
	m_aobTexture[SPECIAL_TRIANGLE]	= TextureManager::Get().LoadTexture_Neutral( "hud/control_triangle_colour_alpha_nomip.dds" );
	m_aobTexture[SPECIAL_CIRCLE]	= TextureManager::Get().LoadTexture_Neutral( "hud/control_circle_colour_alpha_nomip.dds" );
	m_aobTexture[SPECIAL_CROSS]		= TextureManager::Get().LoadTexture_Neutral( "hud/control_cross_colour_alpha_nomip.dds" );
	m_aobTexture[SPECIAL_SQUARE]	= TextureManager::Get().LoadTexture_Neutral( "hud/control_square_colour_alpha_nomip.dds" );
}

void CCharacterDefinition::Release(void)
{
	m_aobTexture[SPECIAL_TRIANGLE].Reset();
	m_aobTexture[SPECIAL_CIRCLE].Reset();
	m_aobTexture[SPECIAL_CROSS].Reset();
	m_aobTexture[SPECIAL_SQUARE].Reset();
}

/***************************************************************************************************
*
*	FUNCTION		CCharacter::CCharacter
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/

CCharacter::CCharacter( CCharacterDefinition& obDefinition )
:	m_pstr3DGlyphDetails ( 0 )
,	m_pstr2DGlyphDetails ( 0 )
,	m_pobTransform ( 0 )
,	m_pobMeshHeader	( 0 )
,	m_pobRenderable ( 0 )
,	m_pobSprite ( 0 )
,	m_pobWorldSprite ( 0 )
,	m_pobEffectGui ( 0 )
,	m_obCharColour(1.0f, 1.0f, 1.0f, 1.0f)
,	m_bSpaceCharacter ( false )
{
	// Store the definition details
	m_obCharacterDefinition = obDefinition;

	// Check if this character is a space - if so it will have no mesh
	m_bSpaceCharacter = ( m_obCharacterDefinition.m_wcCharacter == UnicodeSpace );

	// Check if this character is a action icon
	if ( !m_obCharacterDefinition.m_bSpecialCharacter )
	{
		#ifndef _MASTER
		// Get the pointers to the glyph details and mesh for this character
		if (CFont::FONT_3D == obDefinition.m_iCharType )
		{
			//obDefinition.m_pobFont->GetGlyphDetails3D( m_obCharacterDefinition.m_wcCharacter, &m_pstr3DGlyphDetails, &m_pobMeshHeader );
			ntAssert(0);
		}
		else
		#endif
		{
			obDefinition.m_pobFont->GetGlyphDetails2D( m_obCharacterDefinition.m_wcCharacter, &m_pstr2DGlyphDetails, &m_pobTexture );
		}
	}
	else
	{
		// This is a special character, i.e. a button image
		int iSpecialIndex = m_obCharacterDefinition.m_iSpecialCharacter;

		// If we have SPECIAL_FORWARD or SPECIAL_BACK we need to ask the system which is which
		if (iSpecialIndex == CCharacterDefinition::SPECIAL_BACK)
		{
			iSpecialIndex = g_ShellOptions->m_bGuiXOSwap?CCharacterDefinition::SPECIAL_CROSS:CCharacterDefinition::SPECIAL_TRIANGLE;
		}
		else if (iSpecialIndex == CCharacterDefinition::SPECIAL_FORWARD)
		{
			iSpecialIndex = g_ShellOptions->m_bGuiXOSwap?CCharacterDefinition::SPECIAL_CIRCLE:CCharacterDefinition::SPECIAL_CROSS;
		}
				
		m_pobTexture = m_obCharacterDefinition.m_aobTexture[iSpecialIndex];
		m_pstr2DGlyphDetails = &( m_obCharacterDefinition.m_astrSpecialGlyphs[iSpecialIndex] );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CCharacter::~CCharacter
*
*	DESCRIPTION		Destructor
*
***************************************************************************************************/

CCharacter::~CCharacter( void )
{
	if ( !m_bSpaceCharacter ) 
	{
		if ( m_obCharacterDefinition.m_iCharType == CFont::FONT_3D)
		{
			// Remove the character mesh from the world
			CSector::Get().GetRenderables().RemoveRenderable( m_pobRenderable );
			NT_DELETE( m_pobRenderable );
		}
		else
		{	
			if ( m_obCharacterDefinition.m_eRenderSpace == CGuiUnit::RENDER_SCREENSPACE)
			{
				NT_DELETE( m_pobSprite );
			}

			if ( m_obCharacterDefinition.m_eRenderSpace == CGuiUnit::RENDER_WORLDSPACE)
			{
				ntPrintf("dtor for worldspace char %c\n", m_pstr2DGlyphDetails->wcCode);

				NT_DELETE ( m_pobWorldSprite );		
				//NT_DELETE ( m_pobEffectGui );
				m_pobEffectGui->KillMeWhenReady();
			}
		}

		// Remove the character from the string transform
		m_pobTransform->RemoveFromParent();
		NT_DELETE( m_pobTransform );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CCharacter::PlaceCharacter
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CCharacter::PlaceCharacter( Transform* pobStringTransform, const CPoint& obPosition )
{
	if ( !m_bSpaceCharacter )
	{
		if ( m_obCharacterDefinition.m_iCharType == CFont::FONT_3D)
		{

			// Create a matrix with the point
			CMatrix obPositionMatrix( CONSTRUCT_IDENTITY );
			obPositionMatrix.SetTranslation( obPosition );

			// Create and initialise a transform for this string
			m_pobTransform = NT_NEW Transform();
			m_pobTransform->SetLocalMatrix( obPositionMatrix );

			// Set this characters transform as the child of the parent string's
			pobStringTransform->AddChild( m_pobTransform );

			// Stick it in the world
			m_pobRenderable = NT_NEW CMeshInstance( m_pobTransform, m_pobMeshHeader, true, true, true );
			CSector::Get().GetRenderables().AddRenderable( m_pobRenderable );
		}
		else
		{
			if ( m_obCharacterDefinition.m_eRenderSpace == CGuiUnit::RENDER_SCREENSPACE)
			{
				// Create a matrix with the point
				CMatrix obPositionMatrix( CONSTRUCT_IDENTITY );
				CPoint obPos = obPosition;
				obPos.X() += (float)(m_pstr2DGlyphDetails->iWidth) * m_obCharacterDefinition.m_fCharacterScale * 0.5f;

				obPositionMatrix.SetTranslation( obPos );

				// Create and initialise a transform for this string
				m_pobTransform = NT_NEW Transform();
				m_pobTransform->SetLocalMatrix( obPositionMatrix );

				// Set this characters transform as the child of the parent string's
				pobStringTransform->AddChild( m_pobTransform );

				m_pobSprite = NT_NEW ScreenSprite();
				m_pobSprite->SetTexture( m_pobTexture );
				m_pobSprite->SetColour( CVector(1.0f,1.0f,1.0f,1.0f).GetNTColor() );
				m_pobSprite->SetUV( m_pstr2DGlyphDetails->fTop, m_pstr2DGlyphDetails->fLeft, m_pstr2DGlyphDetails->fBottom, m_pstr2DGlyphDetails->fRight);
				m_pobSprite->SetWidth( (float)(m_pstr2DGlyphDetails->iWidth) * m_obCharacterDefinition.m_fCharacterScale);
				if ( m_obCharacterDefinition.m_bSpecialCharacter )
					m_pobSprite->SetHeight( (float)m_pstr2DGlyphDetails->iWidth * m_obCharacterDefinition.m_fCharacterScale);
				else
					m_pobSprite->SetHeight( (m_obCharacterDefinition.m_pobFont->GetFontLineHeight()) * m_obCharacterDefinition.m_fCharacterScale );			
			}

			if ( m_obCharacterDefinition.m_eRenderSpace == CGuiUnit::RENDER_WORLDSPACE)
			{
				// Create a matrix with the point
				CMatrix obPositionMatrix( CONSTRUCT_IDENTITY );
				CPoint obPos = obPosition;
				obPos.X() += (float)(m_pstr2DGlyphDetails->iWidth) * m_obCharacterDefinition.m_fCharacterScale * 0.5f;


				obPositionMatrix.SetTranslation( obPos );

				// Create and initialise a transform for this string
				m_pobTransform = NT_NEW Transform();
				m_pobTransform->SetLocalMatrix( obPositionMatrix );

				// Set this characters transform as the child of the parent string's
				pobStringTransform->AddChild( m_pobTransform );

				m_pobWorldSprite = NT_NEW WorldSprite();
				m_pobWorldSprite->SetTexture( m_pobTexture );
				m_pobWorldSprite->SetColour( CVector(1.0f,1.0f,1.0f,1.0f).GetNTColor() );
				m_pobWorldSprite->SetUV( m_pstr2DGlyphDetails->fTop, m_pstr2DGlyphDetails->fLeft, m_pstr2DGlyphDetails->fBottom, m_pstr2DGlyphDetails->fRight);
				m_pobWorldSprite->SetWidth( (float)m_pstr2DGlyphDetails->iWidth * m_obCharacterDefinition.m_fCharacterScale);
				if ( m_obCharacterDefinition.m_bSpecialCharacter )
					m_pobWorldSprite->SetHeight( (float)m_pstr2DGlyphDetails->iWidth * m_obCharacterDefinition.m_fCharacterScale);
				else
					m_pobWorldSprite->SetHeight( m_obCharacterDefinition.m_pobFont->GetFontLineHeight() * m_obCharacterDefinition.m_fCharacterScale );

				CPoint Pos;
				Pos = m_pobTransform->GetWorldTranslation();
				m_pobWorldSprite->SetPosition( Pos );
				
				m_pobEffectGui = NT_NEW EffectGui( m_pobWorldSprite );
			}
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CCharacter::GetTotalCharacterWidth
*
*	DESCRIPTION		
*
***************************************************************************************************/

float CCharacter::GetTotalCharacterWidth( void )
{
	// Have a new one
	float fTotalCharacterWidth = 0.0f;

	if ( CFont::FONT_3D == m_obCharacterDefinition.m_pobFont->GetFontType())
	{
		// Add this characters offset to the width
		fTotalCharacterWidth += m_pstr3DGlyphDetails->fOffset;
		// Add this characters advance to the width
		fTotalCharacterWidth += m_pstr3DGlyphDetails->fAdvance;
	}
	else
	{
		fTotalCharacterWidth += (float)m_pstr2DGlyphDetails->iOffset;
		fTotalCharacterWidth += (float)m_pstr2DGlyphDetails->iAdvance;
	}

	fTotalCharacterWidth *= m_obCharacterDefinition.m_fCharacterScale;

	// Send it packing
	return fTotalCharacterWidth;
}

/***************************************************************************************************
*
*	FUNCTION		CCharacter::Render
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CCharacter::Render( void )
{
	if ( !m_bSpaceCharacter && m_obCharacterDefinition.m_iCharType == CFont::FONT_2D )
	{
		
		if ( m_obCharacterDefinition.m_eRenderSpace == CGuiUnit::RENDER_SCREENSPACE)
		{	
			Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

			CPoint Pos;
			Pos = m_pobTransform->GetWorldTranslation();
			m_pobSprite->SetColour( m_obCharColour );

			// integerise coordinates to prevent 'thinning' of font due to bilinear filtering
			Pos.X() = floor(Pos.X());
			Pos.Y() = floor(Pos.Y());

			m_pobSprite->SetPosition( Pos );

			m_pobSprite->Render();

			Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
		}

		// Worldspace are rendered via the EffectGui object.
	}
}


/***************************************************************************************************
*
*	FUNCTION		CLanguage::CStringLookup::CStringLookup
*
*	DESCRIPTION		Constructor
*
***************************************************************************************************/

CLanguage::CStringLookup::CStringLookup( void )					
{ 
	m_uiHashValue = 0; 
	m_pwcStringResource = 0; 
}


/***************************************************************************************************
*
*	FUNCTION		CLanguage::CStringLookup::CStringLookup
*
*	DESCRIPTION		Copy Constructor
*
***************************************************************************************************/

CLanguage::CStringLookup::CStringLookup( const CStringLookup& obOther ) 
{ 
	m_uiHashValue = obOther.m_uiHashValue; 
	m_pwcStringResource = obOther.m_pwcStringResource; 
}


/***************************************************************************************************
*
*	FUNCTION		CLanguage::CStringLookup::operator=
*
*	DESCRIPTION		Equals Operator
*
***************************************************************************************************/

CLanguage::CStringLookup& CLanguage::CStringLookup::operator=( const CStringLookup& obOther )
{
	m_uiHashValue = obOther.m_uiHashValue;
	m_pwcStringResource = obOther.m_pwcStringResource;

	return *this;
}


/***************************************************************************************************
*
*	FUNCTION		CLanguage::CStringLookup::Swap
*
*	DESCRIPTION		Equals Operator
*
***************************************************************************************************/

void CLanguage::CStringLookup::Swap( CStringLookup& obOther )
{
	CStringLookup obTemp( obOther );
	obOther = *this;
	*this = obTemp;
}

/**
	Make the specified font for this language.
*/
/*CFont* CLanguage::MakeFont(CFontDef* pobFontDef )
{
	FontMapIt obIt = m_obFonts.find( GuiUtil::CalculateHash( pobFontDef->m_pcName ) );
	if ( obIt != m_obFonts.end() )
	{
		ntPrintf("Gui: Already have font %s\n", pobFontDef->m_pcName);
		return obIt->second;
	}

	// Make a new font

	// campf - temporarily build the font path dynamically from the config file path and font name (from fonts.xml)
	char acGlyph[255], acTexture[255];
	
	if ((stricmp(pobFontDef->m_pcName, "alan den") == 0) || 
		(stricmp(pobFontDef->m_pcName, "comic") == 0))
	{
		sprintf(acGlyph, "gui\\%s", pobFontDef->m_pcGlyphInfoFile);
		sprintf(acTexture, "gui\\%s", pobFontDef->m_pcRenderableFile);
	}
	else
	{
		sprintf(acGlyph, "%s\\%s", m_acLanguagePath, pobFontDef->m_pcGlyphInfoFile);
		sprintf(acTexture, "%s\\%s", m_acLanguagePath, pobFontDef->m_pcRenderableFile);
	}


	CFont* pobNewFont = NT_NEW CFont( pobFontDef->m_iFontType, acGlyph, acTexture );

	// Keep hold of the font so we can manage them - as our job title indicates
	m_obFonts[ GuiUtil::CalculateHash( pobFontDef->m_pcName ) ] =  pobNewFont;

	return pobNewFont;
}*/

/**
*/
/*void CLanguage::DestroyFont(CFont* pobFont)
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

	ntAssert(0);
}*/

/**
	Load all the font for this language.
*/
/*void CLanguage::LoadFonts(void)
{
	// Import the font definitions
	CGuiFontList* pobFontList;
	pobFontList = static_cast< CGuiFontList* >( CXMLParse::Get().CreateTree( "gui\\fonts.xml" ) );

	for ( ntstd::List< CFontDef* >::iterator obIt = pobFontList->m_obFontList.begin(); obIt != pobFontList->m_obFontList.end(); ++obIt )
	{
		MakeFont(*obIt);
	}

	NT_DELETE ( pobFontList );
}*/

/**
	Get a pointer to a font in this language.
*/
/*CFont* CLanguage::GetFont( const char * pcFontName )
{
	FontMapIt obIt = m_obFonts.find( GuiUtil::CalculateHash( pcFontName ) );
	if ( obIt != m_obFonts.end() && obIt->second )
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
}*/

/**
	Get a pointer to a font in this language.
*/
/*CFont* CLanguage::GetFont( const u_int uiHash )
{
	FontMapIt obIt = m_obFonts.find( uiHash );
	if ( obIt != m_obFonts.end() && obIt->second )
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
}*/


/***************************************************************************************************
*
*	FUNCTION		CLanguage::CLanguage
*
*	DESCRIPTION		Construction
*
*					The string file is imported as a global resource - therefore always present in
*					the same place in memory.  The file itself contains unicode string identifiers,
*					the code takes these, converts them to a char strings and then produces a hash
*					look up value for them.  The strings to output are left where they are in the 
*					file but we actually write a null termination over the end of them so they can
*					be used directly later on.
*
***************************************************************************************************/

CLanguage::CLanguage( const char *pcLanguagePath )
{
	strcpy(m_acLanguagePath, pcLanguagePath);

	// Load up the strings file
	
#ifdef PLATFORM_PS3

	int iByteFileSize = 0;
	uint16_t* pFileData = (uint16_t*)CGuiResource::Get().GetResource( pcLanguagePath, &iByteFileSize );

	// on PS3 our wchar type is 32bit signed quantities, so we have to replace our resource, and byte swap
	int8_t *text = reinterpret_cast< int8_t * >( pFileData );
		for ( int32_t i=0;i<iByteFileSize;i+=2,text+=2 )
		{
			int8_t temp = text[ 0 ];
			text[ 0 ] = text[ 1 ];
			text[ 1 ] = temp;
		}

	// now expand out
	m_stringsData = NT_NEW WCHAR_T[iByteFileSize>>1];
	for ( int32_t i=0; i<(iByteFileSize>>1); i++ )
		m_stringsData[i] = (WCHAR_T)pFileData[i];

	// now fake up these so rest of this function can work
	WCHAR_T* pData = m_stringsData;
	//iByteFileSize *= 2;

#else

	int iByteFileSize = 0;		
	WCHAR_T* pData = (WCHAR_T*)CGuiResource::Get().GetResource( pcLanguagePath, &iByteFileSize );

#	endif

	// Ok. We've got our file, along with iByteFileSyze, and pData. Let's use it!
	int iwchar_tFileSize = iByteFileSize>>1;

	// Set a pointer to the files last WCHAR_T
	WCHAR_T* pLastData = &pData[iwchar_tFileSize - 1];

	// The first character should say that this is little endian unicode
	user_error_p ( pData[0] == 0xfeff, ( "This isn't unicode, get out of here" ) );

	pData++;
	iwchar_tFileSize--;

	// There should be a header on this file for localization - check that
	for( int iCount = 0; &pData[iCount] <= pLastData; ++iCount )
	{
		if ( IsCarriageReturn( &pData[iCount] ) )
		{
			// Save the position of the beginning of line two
			WCHAR_T*	pFirstIdentifier = &pData[iCount];
			pFirstIdentifier += 2;

			// Move back through any white space
			while ( IsWhiteSpace( &pData[iCount - 1] ) ) iCount--;

			// Null terminate the localisation header and check it
			pData[iCount] = 0x0000;
			ntAssert( wcscmp( (const wchar_t*)pData, L"[strings]" ) == 0 );

			// Update the size of the data remaining. 
			iwchar_tFileSize -= pFirstIdentifier - pData;

			// Move the pointer forward and leave
			pData = pFirstIdentifier;

			break;
		}
	}

	// Check the number of carriage returns in the file to find maximum number of strings
	m_iMaxStrings = 0;
	for ( int iwchar_t = 0; iwchar_t < iwchar_tFileSize; ++iwchar_t )
	{
		if ( &pData[iwchar_t] == pLastData || IsCarriageReturn( &pData[iwchar_t] ) ) 
		{
			int jwchar_t = iwchar_t;
			while (jwchar_t > 0 && ! IsCarriageReturn( &pData[jwchar_t - 1] ) ) --jwchar_t;

			// Ignore blank lines
			if ((iwchar_t - jwchar_t) > 1)
				m_iMaxStrings++;	
		}
	}
	
	// We have to have some strings..
	ntAssert( m_iMaxStrings > 0 );

	// Create and array of CStringLookups large enough to contain the max possible number of strings
	m_pstrStringLookup = NT_NEW CStringLookup[m_iMaxStrings];

	for ( int iString = 0; iString < m_iMaxStrings;  )
	{
		// Move to the beginning of a string identifier
		while ( pData <= pLastData ) 
		{
			if ( IsWhiteSpace( pData ) ) pData++;
			else if ( IsCarriageReturn( pData ) ) pData += 2;
			else break;
		}

		// Get the identifier string and hash value it
		for( int iCount = 0; &pData[iCount] <= pLastData; ++iCount )
		{
			// Look for the first equals sign on this line
			if ( IsEqualsOperator( &pData[iCount] ) )
			{
				// Move back through any white space
				while ( IsWhiteSpace( &pData[iCount - 1] ) ) iCount--;

				// Null terminate the identifier string
				pData[iCount] = 0x0000;

				// Save the hash value for this identifier and move our data pointer
				m_pstrStringLookup[iString].m_uiHashValue = GuiUtil::CalculateHash( pData );
				pData = &pData[iCount];

				// Drop out
				break;
			}
		}

		// Get a pointer to the unicode resource string
		for( int iCount = 0; &pData[iCount] <= pLastData; ++iCount )
		{
			if ( IsDoubleQuote( &pData[iCount] ) )
			{
				m_pstrStringLookup[iString].m_pwcStringResource = &pData[iCount + 1];
				break;
			}
		}

		// Terminate the string we have just set the pointer to
		for( int iCount = 0; &pData[iCount] <= pLastData; ++iCount )
		{
			if ( IsCarriageReturn( &pData[iCount] ) )
			{
				// Move back through any white space
				while ( IsWhiteSpace( &pData[iCount - 1] ) ) iCount--;

				// The previous character should be a double quote
				ntAssert_p( IsDoubleQuote( &pData[iCount - 1] ), ( "This string is not properly terminated" ) );

				// Null terminate the resource string by writing over the double quote
				pData[iCount - 1] = 0x0000;
				pData = &pData[iCount];

				// You've got to find a way, Say what you want to say, Breakout
				break;
			}
		}

		// Ignore blank lines
		if ( m_pstrStringLookup[iString].m_uiHashValue )
			iString++;
	}

	// Sort the items into hash value order
	SortEntries( m_pstrStringLookup, 0, m_iMaxStrings - 1 );

	#ifndef _MASTER
	// Check through our array to ensure we do not have duplicate keys
	for ( int iString = 1; iString < m_iMaxStrings; iString++ )
	{
		ntAssert_p( m_pstrStringLookup[iString].m_uiHashValue > m_pstrStringLookup[iString - 1].m_uiHashValue, ( "Our resources are in the wrong order" ) );
		ntAssert_p( ( ( m_pstrStringLookup[iString].m_uiHashValue != m_pstrStringLookup[iString - 1].m_uiHashValue ) || ( m_pstrStringLookup[iString].m_uiHashValue == 0 ) ), ( "We have two hash values the same" ) );
	}
	#endif

	//LoadFonts();
}


/***************************************************************************************************
*
*	FUNCTION		CLanguage::~CLanguage
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/


CLanguage::~CLanguage( void )
{
	/*for ( FontMapIt obIt = m_obFonts.begin(); obIt != m_obFonts.end(); ++obIt )
	{
		NT_DELETE( obIt->second );
	}

	// Remove the pointers
	m_obFonts.clear();*/

	CGuiResource::Get().ReleaseResource(m_acLanguagePath);

	// Get rid of all those bits and bobs
	NT_DELETE_ARRAY( m_pstrStringLookup );

	// get rid of converted version of strings.inf
#ifdef PLATFORM_PS3
	NT_DELETE_ARRAY(m_stringsData);
#endif
}



/***************************************************************************************************
*	
*	FUNCTION		CLanguage::SortEntries
*
*	DESCRIPTION		Sorts the array of CStringLookups from smallest to largest hash value so a 
*					binary sort may be used to find the string resource. Its recursive.
*
*	INPUT			Takes the array values you want to sort between - for the whole array this would
*					be 0 and <array size> - 1.  As offset from the pointer to the array.
*
***************************************************************************************************/

void CLanguage::SortEntries( CStringLookup* pstrStringLookups, int iArrayStart, int iArrayEnd )
{	
	int		iLow				= iArrayStart;
	int		iHigh				= iArrayEnd;
	int		iMidIndex			= ( iArrayStart + iArrayEnd ) / 2;
	u_int	uiSortHashValue		= pstrStringLookups[ iMidIndex ].m_uiHashValue;

	do
	{
		while ( pstrStringLookups[ iLow ].m_uiHashValue < uiSortHashValue ) iLow++;
		while ( pstrStringLookups[ iHigh ].m_uiHashValue > uiSortHashValue ) iHigh--;

		if ( iLow <= iHigh )
		{
			pstrStringLookups[ iLow ].Swap( pstrStringLookups[ iHigh ] );
			iLow++;
			iHigh--;
		}

	} while ( iLow < iHigh );

	if ( iHigh > iArrayStart )	SortEntries( pstrStringLookups, iArrayStart, iHigh );
	if ( iLow < iArrayEnd )		SortEntries( pstrStringLookups, iLow, iArrayEnd );
}


/***************************************************************************************************
*
*	FUNCTION		CLanguage::GetResourceString
*
*	DESCRIPTION		What this whole class is based around - finding string contents from a given ID.
*					IDs can be in char or WCHAR_T format but they may only be made from ascii encoded
*					characters.
*
***************************************************************************************************/

const WCHAR_T* CLanguage::GetResourceString( const char* pcIdentifier )
{
	// Make sure we have a valid identifier
	ntAssert( pcIdentifier );

	// Hash the request identifier
	u_int uiRequiredHashValue = GuiUtil::CalculateHash( pcIdentifier );

	// Use the hash, return the string
	return GetResourceString( uiRequiredHashValue );
}


/***************************************************************************************************
*
*	FUNCTION		CLanguage::GetResourceString
*
*	DESCRIPTION		What this whole class is based around - finding string contents from a given ID.
*					IDs can be in char or WCHAR_T format but they may only be made from ascii encoded
*					characters.
*
***************************************************************************************************/

const WCHAR_T* CLanguage::GetResourceString( const WCHAR_T* pwcIdentifier )
{
	// Make sure we have a valid identifier
	ntAssert( pwcIdentifier );

	// Hash the request identifier
	u_int uiRequiredHashValue = GuiUtil::CalculateHash( pwcIdentifier );

	// Use the hash, return the string
	return GetResourceString( uiRequiredHashValue );
}


/***************************************************************************************************
*
*	FUNCTION		CLanguage::GetResourceString
*
*	DESCRIPTION		Takes a char string identifier, calculates the hash value for that identifier
*					then does a binary search for that hash value in the string array.
*
***************************************************************************************************/

const WCHAR_T* CLanguage::GetResourceString( u_int uiRequiredHashValue )
{
	// Set our search parameters
	int iFirst	= 0;
	int	iLast	= m_iMaxStrings - 1;

	// Binary search
	while ( iFirst <= iLast ) 
	{
		// Get the midpoint
		int iMidpoint = ( iFirst + iLast) / 2;

		// If the value here is too small look in the division above
		if ( uiRequiredHashValue > m_pstrStringLookup[iMidpoint].m_uiHashValue )
		{
           iFirst = ++iMidpoint;
		}

		// If the value here is too big look in the division below
		else if ( uiRequiredHashValue < m_pstrStringLookup[iMidpoint].m_uiHashValue )
		{
			iLast = --iMidpoint;
		}

		// Otherwise we must have found it - Bingo
		else
		{
			return m_pstrStringLookup[iMidpoint].m_pwcStringResource;
		}
	}

	// If we have got here then we haven't found anything
	return 0;
}


/***************************************************************************************************
*
*	FUNCTION		CFont::CFont
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CFont::CFont( int iType, const char * pcFile, const char * pcResource  )
:	m_uiVersion		( 0 )
,	m_fFontHeight	( 0 )
,	m_uiFontHeight	( 0 )
,	m_uiTextureWidth( 0 )
,	m_uiTextureHeight( 0 )
,	m_uiTextureBPP	( 0 )
,	m_uiNumGlyphs	( 0 )
,	m_wcLowchar_t	( 0 )
,	m_wcHighChar	( 0 )
,	m_pstr3DGlyphs	( 0 )
,	m_pstr2DGlyphs	( 0 )
,	m_pcGlyphInfo	( 0 )
,   m_iFontType		( iType )
,	m_pobClumpHeader( 0 )
,	m_pobTexture	( 0 )
{
	// Load up the glyph data
	Util::SetToPlatformResources();
	uint8_t* pData = CGuiResource::Get().GetResource( pcFile, 0, true );
	Util::SetToNeutralResources();
	user_warn_p( pData, ("Can not find %s.\n", pcFile) );
	if ( pData == NULL )
	{
		return;
	}

	// Take a note of the glyph file we loaded so it can be unloaded later
	char* pcTempString = NT_NEW char[ strlen( pcFile ) + 1 ];
	strcpy( pcTempString, pcFile );
	m_pcGlyphInfo = pcTempString;

	// Move through the file to get the data
	if (FONT_2D == m_iFontType)
	{
		m_uiVersion = *( ( uint32_t* )pData );
		pData += sizeof( uint32_t );
	}

	if (FONT_3D == m_iFontType)
	{
		m_fFontHeight = *( ( float* )pData );
		pData += sizeof( float );
	}
	else
	{
		m_uiFontHeight = *( ( uint32_t* )pData );
		m_fFontHeight = (float)m_uiFontHeight;
		pData += sizeof( uint32_t );
	}

	if (FONT_2D == m_iFontType)
	{
		m_uiTextureWidth = *( ( uint32_t* )pData );
		pData += sizeof( uint32_t );

		m_uiTextureHeight = *( ( uint32_t* )pData );
		pData += sizeof( uint32_t );

		m_uiTextureBPP = *( ( uint32_t* )pData );
		pData += sizeof( uint32_t );
	}

	m_wcLowchar_t = *( ( WCHAR_T* )pData );
	pData += sizeof( WCHAR_T );

	m_wcHighChar = *( ( WCHAR_T* )pData );
	pData += sizeof( WCHAR_T );

	m_uiNumGlyphs = *( ( uint32_t* )pData );
	pData += sizeof( uint32_t );

	if (FONT_3D == m_iFontType)
	{
		m_pstr3DGlyphs = ( GLYPH_ATTR_3D* )pData;
		// Try to build a clump header from the given filename
		m_pobClumpHeader = CClumpLoader::Get().LoadClump_Neutral( pcResource, true );
		ntAssert( m_pobClumpHeader );
	}
	else
	{
		m_pstr2DGlyphs = ( GLYPH_ATTR_2D* )pData;
		m_pobTexture = TextureManager::Get().LoadTexture_Neutral( pcResource, true );	
		ntAssert( m_pobTexture );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CFont::~CFont
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CFont::~CFont( void )
{
	// If we have a clump header - drop it
	if ( m_pobClumpHeader )
		CClumpLoader::Get().UnloadClump( m_pobClumpHeader );

	// Drop the glyph definition file
	CGuiResource::Get().ReleaseResource( m_pcGlyphInfo );

	NT_DELETE_ARRAY ( m_pcGlyphInfo );
}


/***************************************************************************************************
*
*	FUNCTION		CFont::GetGlyphDetails
*
*	DESCRIPTION		This version for 3D fonts only
*
***************************************************************************************************/

/*bool CFont::GetGlyphDetails3D( WCHAR_T wcCharacter, GLYPH_ATTR_3D** ppGlyphAtt, CMeshHeader** ppMeshHeader )
{
	// Check the input
	ntAssert( ppGlyphAtt );
	ntAssert( ppMeshHeader );
	ntAssert( m_iFontType == FONT_3D );

	// Set up a return value
	bool bGlyphAttFound = false;

	// The first character is null and used if nothing else is found
	int iFirst	= 1;
	int	iLast	= m_uiNumGlyphs - 1;

	while ( iFirst <= iLast ) 
	{
		// Get the midpoint
		int iMidpoint = ( iFirst + iLast) / 2;

		// If the value here is too small look in the division above
		if ( wcCharacter > m_pstr3DGlyphs[iMidpoint].wcCode )
		{
           iFirst = ++iMidpoint;
		}

		// If the value here is too big look in the division below
		else if ( wcCharacter < m_pstr3DGlyphs[iMidpoint].wcCode )
		{
			iLast = --iMidpoint;
		}

		// Otherwise we must have found it - Bingo
		else
		{
			*ppGlyphAtt = &m_pstr3DGlyphs[iMidpoint];
			bGlyphAttFound = true;
			break;
		}
	}

	// If we didn't find it use the null character at zero
	if ( !bGlyphAttFound )
		*ppGlyphAtt = &m_pstr3DGlyphs[0]; 

	// Now we can look for the mesh header for this glyph
	char pcMeshName[MAX_PATH] = { 0 };

	// A space character has not mesh
	if ( wcCharacter == UnicodeSpace )
	{
		// Set the mesh pointer and return
		*ppMeshHeader = 0;
		return bGlyphAttFound;
	}

	else
	{
		// If we found the glyph details above - look for that character
		if ( bGlyphAttFound )
			sprintf( pcMeshName, "glyph%dShape", wcCharacter );

		// Otherwise find the NULL character mesh
		else
			sprintf( pcMeshName, "glyphNULLShape" );

		// Hash the string value
		CHashedString obSearchHash( pcMeshName );

		// Unfortunately this is a horrible linear search for now
		for ( int iMesh = 0; iMesh < m_pobClumpHeader->m_iNumberOfMeshes; iMesh++ )
		{
			if ( m_pobClumpHeader->m_pobMeshHeaderArray[ (uint32_t)iMesh ].m_obNameHash == obSearchHash )
			{
				// Set the mesh pointer and return
				*ppMeshHeader = &m_pobClumpHeader->m_pobMeshHeaderArray[ (uint32_t)iMesh ];
				return bGlyphAttFound;
			}
		}
	}

	// If we are here we have failed to find a mesh
	ntAssert( 0 );
	return false;
}*/

/***************************************************************************************************
*
*	FUNCTION		CFont::GetGlyphDetails
*
*	DESCRIPTION		This version for 2D fonts only
*
***************************************************************************************************/

bool CFont::GetGlyphDetails2D( wchar_t wcCharacter, GLYPH_ATTR_2D** ppGlyphAtt, Texture::Ptr* ppTexture )
{
	UNUSED (ppTexture);

	// Check the input
	ntAssert( ppGlyphAtt );
	ntAssert( ppTexture );
	ntAssert( m_iFontType == FONT_2D );

	// Set up a return value
	bool bGlyphAttFound = false;

	// The first character is null and used if nothing else is found
	int iFirst	= 1;
	int	iLast	= m_uiNumGlyphs - 1;

	while ( iFirst <= iLast ) 
	{
		// Get the midpoint
		int iMidpoint = ( iFirst + iLast) / 2;

		// If the value here is too small look in the division above
		if ( wcCharacter > m_pstr2DGlyphs[iMidpoint].wcCode )
		{
           iFirst = ++iMidpoint;
		}

		// If the value here is too big look in the division below
		else if ( wcCharacter < m_pstr2DGlyphs[iMidpoint].wcCode )
		{
			iLast = --iMidpoint;
		}

		// Otherwise we must have found it - Bingo
		else
		{
			*ppGlyphAtt = &m_pstr2DGlyphs[iMidpoint];
			bGlyphAttFound = true;
			break;
		}
	}

	// If we didn't find it use the null character at zero
	if ( !bGlyphAttFound )
		*ppGlyphAtt = &m_pstr2DGlyphs[0]; 

	*ppTexture = m_pobTexture;
	
	// If we are here we have failed to find a mesh
	//ntAssert( bGlyphAttFound );
	return bGlyphAttFound;
}

