/***************************************************************************************************
*
*	DESCRIPTION		Functionality concerning strings and font use in the GUI
*
*	NOTES			This is the main text interface code avaialbe to the user
*
***************************************************************************************************/

#ifndef _GUITEXT_H
#define _GUITEXT_H

#include "guitextinternals.h"
#include "guiparse.h"
#include "guimanager.h"
#include "game/languages.h"

// Forward declarations
class Transform;
class CStringManager;
class CFontManager;
class CGuiFontList;
class CFontDef;
class MessageDataManager;

// Convertion for enum values to strings - null terminated for converter
const CStringUtil::STRING_FLAG astrFontType[] = 
{	
	{ CFont::FONT_2D,				"FONT_2D"			},
	{ CFont::FONT_3D,				"FONT_3D"			},
	{ 0,							0					} 
};

// XML element to hold list of fonts
class CGuiFontList : public CXMLElement
{
public:


	//! Construction Destruction
	CGuiFontList( void );
	~CGuiFontList( void );

protected:

	//! Font List
	ntstd::List< CFontDef* >	m_obFontList;
	
	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );
	virtual bool	ProcessChild( CXMLElement* pobChild );

	friend class CFontManager;
	friend class CLanguage;
};

/***************************************************************************************************
*
*	CLASS			CStringDefinition
*
*	DESCRIPTION		A majority of these items will need to be moved out into some sort of style 
*					class - but that probably won't be necessary for the prototype.
*
***************************************************************************************************/

class CStringDefinition
{
public:

	// Initialisation
	CStringDefinition( void ) : m_fBoundsWidth ( 0.0f ),
								m_fBoundsHeight ( 0.0f ),
								m_fXOffset ( 0.0f ),
								m_fYOffset ( 0.0f ),
								m_fZOffset ( 0.0f ),
								m_fLineSpacingMultiplier ( 1.0f ),
								m_fWordSpacingMultiplier ( 1.0f ),
								m_eVJustify ( JUSTIFY_MIDDLE ),
								m_eHJustify ( JUSTIFY_CENTRE ),
								m_bDynamicFormat ( true ) ,
								m_fOverallScale ( 1.0f ) ,
								m_pobFont	(0),
								m_pobMessageDataManager	(0) {}

	// Justification - this needs to be wrapped 
	// up in some style information
	enum STRING_JUSTIFY_VERTICAL
	{
		JUSTIFY_TOP,
		JUSTIFY_MIDDLE,
		JUSTIFY_BOTTOM,
	};

	enum STRING_JUSTIFY_HORIZONTAL
	{
		JUSTIFY_LEFT,
		JUSTIFY_CENTRE,
		JUSTIFY_RIGHT,
	};

	// Size of string bounds
	float	m_fBoundsWidth;
	float	m_fBoundsHeight;	

	// Offset of the point around which the bounds are placed.
	// The bounds positioning about the point depends on the justication
	// For example for top/left justified text the point will position
	// the top/left corner of the bounding box.
	float	m_fXOffset;
	float	m_fYOffset;
	float	m_fZOffset;

	// Spacing control
	float	m_fLineSpacingMultiplier;
	float	m_fWordSpacingMultiplier;

	// String justification
	STRING_JUSTIFY_VERTICAL		m_eVJustify;
	STRING_JUSTIFY_HORIZONTAL	m_eHJustify;

	// Formatting
	bool	m_bDynamicFormat;

	// Scaling 
	float m_fOverallScale;

	CFont* m_pobFont;

	// set this to override the default (HUD) message data manager
	MessageDataManager* m_pobMessageDataManager;
};


/***************************************************************************************************
*
*	CLASS			CString
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CString
{
public:
	void Render();

	float RenderWidth();
	float RenderHeight();

	void SetColour ( CVector& obColour );
	const CVector GetColour( void );

protected:

	//! Construction Destruction
	CString( const WCHAR_T* pwcStringContents, CStringDefinition& obDefinition, Transform* pobTransform, CGuiUnit::RENDER_SPACE eRenderSpace );
	~CString( void );

	//! Construction help
	void	BuildCharacters( const WCHAR_T* pwcChars, CCharacterDefinition& obCharDef );
	void	LayoutCharacters( void );

	//! Dynamic formatting
	void	DynamicallyFormat( void );
	void	InsertDynamicBreak( ntstd::List< CCharacter* >::iterator obStart );

	//! Utility methods
	float								GetLineLength( ntstd::List< CCharacter* >::iterator obCharacter );
	ntstd::List<CCharacter*>::iterator	GetNextLineStart( ntstd::List< CCharacter* >::iterator obCharacter );
	float								GetStringHeight( ntstd::List< CCharacter* >::iterator obCharacter );
	float								GetEffectiveCharacterWidth( ntstd::List< CCharacter* >::iterator obCharacter );	

	//! String properties
	CStringDefinition		m_obStringDefinition;
	
	//! The characters
	ntstd::List< CCharacter* >	m_obCharacters;

	//! Our base string position
	Transform*				m_pobTransform;

	//! Does the string fit within its given bounds
	bool					m_bExceedsBounds;

	//! Renderspace for this string
	CGuiUnit::RENDER_SPACE m_eRenderSpace;

	float m_fStringScale;

	//! He's our friend - and the only one who can make us live or die 
	friend class CStringManager;
};


/***************************************************************************************************
*
*	CLASS			CStringManager
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CStringManager : public Singleton< CStringManager >
{
public:
	//! Construction Destruction
	CStringManager( void );
	~CStringManager( void );

	//! The things what this class does
	void		Update( void );
	CString*	MakeString( const char* pcStringID, CStringDefinition& obStringDef, Transform* pobTransform, CGuiUnit::RENDER_SPACE eRenderSpace  );
	CString*	MakeString( const WCHAR_T* pwcStringContents, CStringDefinition& obStringDef, Transform* pobTransform, CGuiUnit::RENDER_SPACE eRenderSpace  );
	void		DestroyString( CString* pobString );

	const WCHAR_T* GetResourceString( const char* pcIdentifier );
	const WCHAR_T* GetResourceString( const WCHAR_T* wpcIdentifier );

	CLanguage *GetGeneralLanguage(void) const { return m_pobGeneralStrings; }
	CLanguage *GetLevelLanguage(void) const { return m_pobLevelStrings; }

	void LevelChange( const char *pcLanguagePath );

	void SetLanguage( SubtitleLanguage iLanguage);

	SubtitleLanguage GetLanguage(void) const
	{
		return m_eCurrentLangauge;
	}

	const char *GetLanguageName(void) const
	{
		return Language::GetSubtitleLanguageName( m_eCurrentLangauge );
	}

protected:

	//! The strings that we are managing
	ntstd::List< CString* >	m_obStrings;

private:	

	// Strings for frontend/general and per level strings
	CLanguage *m_pobGeneralStrings;
	CLanguage *m_pobLevelStrings;

	// Currently selected language
	SubtitleLanguage m_eCurrentLangauge;

	// Name of current level
	const char *m_pcLevelName;
	const char *m_strRegion;
};

/***************************************************************************************************
*
*	CLASS			CFontDef
*
*	DESCRIPTION		Definition of a font, will probably need to inherit from CXMLElement in
*					the long run so it can be read from the Gui style Xml
*
***************************************************************************************************/

class CFontDef : public CXMLElement
{
public:
	CFontDef( CFont::FONT_TYPE eFontType, const char* pcGlyphInfoFile, const char* pcRenderableFile );
	CFontDef();

	~CFontDef();
protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

private:
	int m_iFontType;
	const char* m_pcName;
	const char* m_pcGlyphInfoFile;
	const char* m_pcRenderableFile;

	friend class CFontManager;
	friend class CLanguage;
};

/***************************************************************************************************
*
*	CLASS			CFontManager
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CFontManager : public Singleton< CFontManager >
{
public:

	//! Construction Destruction
	CFontManager( void );
	~CFontManager( void );

	//! The things what this class does
	void			Update( void );
	//CFont*			MakeFont( void );
	CFont*			MakeFont( CFontDef* pobFontDef, const char *strRegion );
	CFont*			GetFont( const char * pcFontName );
	void			DestroyFont( CFont* pobFont );
	void			FreeFonts(void);
	void			Initialise(const char *strRegion);

protected:

	typedef ntstd::Map< u_int, CFont* > FontMap;
	typedef ntstd::Map< u_int, CFont* >::iterator FontMapIt;

	//! The fonts that we are managing
	FontMap	m_obFonts;
};

#endif // _GUITEXT_H
