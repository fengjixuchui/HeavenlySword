/***************************************************************************************************
*
*	DESCRIPTION		Functionality concerning strings and font use in the GUI
*
*	NOTES			This is the hidden away stuff - not to be included in anything other than
*					guitext.cpp
*
***************************************************************************************************/

#ifndef _GUITEXTINTERNALS_H
#define _GUITEXTINTERNALS_H

#include "gfx/texture.h"
#include "effect/screensprite.h"
#include "effect/worldsprite.h"

#include "guieffect.h"
#include "guiutil.h"
#include "guiunit.h"

// Foward declarations
class CClumpHeader;
class Transform;
class CMeshHeader;
class CMeshInstance;
class CFontManager;
class CFont;
class CFontDef;

/***************************************************************************************************
*
*	CLASS			CFont
*
*	DESCRIPTION		Will store the data for available fonts.  
*
***************************************************************************************************/

class CFont
{
public:

	// A per glyph information structure
	struct GLYPH_ATTR_3D
	{
		float fOffset;
		float fWidth;
		float fAdvance;
		WCHAR_T wcCode;
	};

	// A 2D font needs all the details above plus four floats for the texture
	// coordinates to look into the texture exported from font maker.
	struct GLYPH_ATTR_2D
	{
		float fLeft;
		float fTop;
		float fRight;
		float fBottom;
		short iOffset;
		short iWidth;
		short iAdvance;
		WCHAR_T wcCode;
	};

	
	enum FONT_TYPE {
		FONT_2D = 1,
		FONT_3D
	};

	// Construction Destruction
	CFont( int iType, const char * pcFile, const char * pcResource );
	~CFont( void );

	// Resources required by characters
	//bool	GetGlyphDetails3D( WCHAR_T wcCharacter, GLYPH_ATTR_3D** ppGlyphAtt, CMeshHeader** ppMeshHeader );
	bool	GetGlyphDetails2D( WCHAR_T wcCharacter, GLYPH_ATTR_2D** ppGlyphAtt, Texture::Ptr* ppTexture );

	// Details required by strings
	float	GetFontLineHeight( void )	{ return m_fFontHeight; }
	int		GetFontType( void )	{ return m_iFontType; }

protected:

	// All the glyph infomation
	uint32_t				m_uiVersion;
	float					m_fFontHeight;
	uint32_t				m_uiFontHeight;
	uint32_t				m_uiTextureWidth;
	uint32_t				m_uiTextureHeight;
	uint32_t				m_uiTextureBPP;
	uint32_t				m_uiNumGlyphs;
    WCHAR_T					m_wcLowchar_t;
    WCHAR_T					m_wcHighChar;
    GLYPH_ATTR_3D*			m_pstr3DGlyphs;
	GLYPH_ATTR_2D*			m_pstr2DGlyphs;

	char *					m_pcGlyphInfo;

	int m_iFontType;

	// Holds all the mesh data for a 3D font
	CClumpHeader*		m_pobClumpHeader;

	// Holds the texture data for a 2D font
	Texture::Ptr			m_pobTexture;

	friend class CFontManager;
};


/***************************************************************************************************
*
*	CLASS			CCharacterDefinition
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CCharacterDefinition
{
public:

	CCharacterDefinition() 
	:	m_iCharacterFlags (0)
	,	m_bSpecialCharacter (false)
	,	m_iSpecialCharacter ( 0 )
	,	m_wcCharacter (0)
	,	m_fCharacterScale (1.0f)
	,	m_pobFont ( 0 )
	{};

	// Presentation flags
	enum CHARACTER_FLAG
	{
		CHARACTER_BOLD			= ( 1 << 0 ),			// Present in Bold
		CHARACTER_ITALIC		= ( 1 << 1 ),			// Present in Italics
		CHARACTER_UNDERLINE		= ( 1 << 2 ),			// Underline this character
		CHARACTER_EMPHASISED	= ( 1 << 3 ),			// Emphasise this character
		CHARACTER_LINE_BREAK	= ( 1 << 4 ),			// Line break before this character
	};

	// Special characters
	enum SPECIAL_CHARACTER
	{
		SPECIAL_TRIANGLE	,			// Triangle
		SPECIAL_CIRCLE		,			// Circle
		SPECIAL_CROSS		,			// Cross
		SPECIAL_SQUARE		,			// Square
		SPECIAL_FORWARD		,			// Forward  - user defined
		SPECIAL_BACK		,			// Back - user defined
		SPECIAL_END
	};

	// Special glyph infomation
	static CFont::GLYPH_ATTR_2D m_astrSpecialGlyphs[ CCharacterDefinition::SPECIAL_END ];
	// The textures the special glyphs
	static Texture::Ptr			m_aobTexture[ CCharacterDefinition::SPECIAL_END ];

	static void Initialise(void);
	static void Release(void);

	// To hold the presentation flags
	int		m_iCharacterFlags;

	// A special character?
	bool	m_bSpecialCharacter;
	int		m_iSpecialCharacter;

	// What is the character?
	WCHAR_T	m_wcCharacter;

	// What scale should this character be displayed at
	float	m_fCharacterScale;

	// Helper functions for users of this class
	//static	CHARACTER_FLAG	CharacterFlagFromString( const WCHAR_T* pwcTagDetails );

	// What font is this character?
	CFont*	m_pobFont;

	//! Renderspace for this character
	CGuiUnit::RENDER_SPACE m_eRenderSpace;

	int m_iCharType;
};

// Convertion for bitwise enum values to strings - null terminated for converter
const CStringUtil::STRING_FLAG_W astrMarkupFlags[] = 
{	
	{ CCharacterDefinition::CHARACTER_BOLD,			(const WCHAR_T*)L"B"		},
	{ CCharacterDefinition::CHARACTER_ITALIC,		(const WCHAR_T*)L"I"		},
	{ CCharacterDefinition::CHARACTER_UNDERLINE,	(const WCHAR_T*)L"U"		},
	{ CCharacterDefinition::CHARACTER_EMPHASISED,	(const WCHAR_T*)L"EM"		},
	{ CCharacterDefinition::CHARACTER_LINE_BREAK,	(const WCHAR_T*)L"BR"		},
	{ 0,											0							} 
};

// Convertion for bitwise enum values to strings - null terminated for converter
const CStringUtil::STRING_FLAG_W astrSpecialChars[] = 
{	
	{ CCharacterDefinition::SPECIAL_TRIANGLE,		(const WCHAR_T*)L"TRIANGLE"	},
	{ CCharacterDefinition::SPECIAL_CIRCLE,			(const WCHAR_T*)L"CIRCLE"	},
	{ CCharacterDefinition::SPECIAL_CROSS,			(const WCHAR_T*)L"CROSS"	},
	{ CCharacterDefinition::SPECIAL_SQUARE,			(const WCHAR_T*)L"SQUARE"	},
	{ CCharacterDefinition::SPECIAL_FORWARD,		(const WCHAR_T*)L"FORWARD"	},
	{ CCharacterDefinition::SPECIAL_BACK,			(const WCHAR_T*)L"BACK"		},
	{ 0,											0							} 
};

/***************************************************************************************************
*
*	CLASS			CCharacter
*
*	DESCRIPTION		This class must provide the standard interface to all characters that may be
*					used.  The string class should not know whether the characters it holds are
*					2D or 3D or button images.
*
*					These characters should hold enough data to format and arrange the whole string.
*					There should be no need to refer to the orignal character string for formating
*					information once an array of these characters has been put together.
*
*					This class is patched up as a 3D mesh only at the moment - the 3D/2D divide will
*					have to be put in later when we have the capacity to do 2D!
*
***************************************************************************************************/

class CCharacter
{
public:

	//! Construction Destruction
	CCharacter( CCharacterDefinition& obDefintion );
	~CCharacter( void );

	//! Set the transform and add it to the world
	void	PlaceCharacter( Transform* pobStringTransform, const CPoint& obPosition );

	//! Where would a subsequent character need to be placed
	float	GetTotalCharacterWidth( void );

	//! Is this character a space?  Required for dynamic formating
	bool	CharacterIsSpace( void )		{ return m_bSpaceCharacter; }

	//! Does this character start a new line
	bool	CharacterStartsLine( void )		{ return ( ( m_obCharacterDefinition.m_iCharacterFlags & CCharacterDefinition::CHARACTER_LINE_BREAK ) == CCharacterDefinition::CHARACTER_LINE_BREAK  ); }

	//! Set this character as a line break
	void	SetAsLineBreak( void )			{ m_obCharacterDefinition.m_iCharacterFlags |= CCharacterDefinition::CHARACTER_LINE_BREAK; }


	void Render();

	void SetColour ( CVector& obColour ) { m_obCharColour = obColour; };
	const CVector& GetColour( void ) { return m_obCharColour; }

protected:

	// Character properties
	CCharacterDefinition	m_obCharacterDefinition;

	// The properties of our glyph
	CFont::GLYPH_ATTR_3D*	m_pstr3DGlyphDetails;
	CFont::GLYPH_ATTR_2D*	m_pstr2DGlyphDetails;

	// The characters transform relative to a string
	Transform*				m_pobTransform;

	// The mesh header describing the glyph
	CMeshHeader*			m_pobMeshHeader;

	// The texture for this glyph
	Texture::Ptr			m_pobTexture;

	// The renderable for the character.
	CMeshInstance*			m_pobRenderable;

	ScreenSprite*			m_pobSprite;

	WorldSprite*			m_pobWorldSprite;
	EffectGui*				m_pobEffectGui;

	CVector m_obCharColour;

	// Special cases abound for the space character - it has no mesh
	bool					m_bSpaceCharacter;

};


/***************************************************************************************************
*
*	CLASS			CLanguage
*
*	DESCRIPTION		This class sets itself up to lookup the localised string resources.  And it's 
*					really good.
*
***************************************************************************************************/

class CLanguage
{
public:

	//! Construction Destruction
	CLanguage( const char *pcFilename );
	~CLanguage( void );

	//! Get a pointer to the identified string contents
	const WCHAR_T* GetResourceString( const char* pcIdentifier );
	const WCHAR_T* GetResourceString( const WCHAR_T* pcIdentifier );

	// Path of the language this strings table represents
	char m_acLanguagePath[64];

	/*typedef ntstd::Map< u_int, CFont* > FontMap;
	typedef ntstd::Map< u_int, CFont* >::iterator FontMapIt;

	//! The fonts that we are managing
	FontMap	m_obFonts;

	CFont *MakeFont(CFontDef* pobFontDef );
	void DestroyFont(CFont* pobFont);
	CFont *GetFont( const char * pcFontName );
	CFont *GetFont( const u_int uiHash );
	void LoadFonts(void);*/

protected:

	// Nested class to enable storage and sorting
	class CStringLookup
	{
	public:

		// Construction
		CStringLookup( void );
		CStringLookup( const CStringLookup& obOther );

		// Operations
		CStringLookup& operator=( const CStringLookup& obOther );
		void Swap( CStringLookup& obOther );
			
		// Members
		u_int			m_uiHashValue;
		const WCHAR_T*	m_pwcStringResource;
	};

	// Helpers
	bool IsWhiteSpace		( WCHAR_T* pwcChar ) const	{ return ( ( pwcChar[0] == 0x0009 ) || ( pwcChar[0] == 0x0020 ) ); }
	bool IsCarriageReturn	( WCHAR_T* pwcChar ) const	{ return ( ( pwcChar[0] == 0x000D ) && ( pwcChar[1] == 0x000A ) ); }
	bool IsEqualsOperator	( WCHAR_T* pwcChar ) const	{ return ( pwcChar[0] == 0x003D ); }
	bool IsDoubleQuote		( WCHAR_T* pwcChar ) const	{ return ( pwcChar[0] == 0x0022 ); }

	// Sort all our strings into hash value order
	void	SortEntries( CStringLookup* pstrStringLookups, int iArrayStart, int iArrayEnd );

	// Find a string given its hash value
	const WCHAR_T* GetResourceString( u_int uiRequiredHashValue );

	// Hash lookup values - Moved to GuiUtil
	//u_int	CalculateHash( const WCHAR_T* pwcIdentifier );
	//u_int	CalculateHash( const char* pcIdentifier );

	// Our array of string lookups
	CStringLookup*	m_pstrStringLookup;
	int				m_iMaxStrings;

#ifdef PLATFORM_PS3
	WCHAR_T*		m_stringsData;
#endif
};

#endif // _GUITEXTINTERNALS_H
