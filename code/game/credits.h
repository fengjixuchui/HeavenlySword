//-------------------------------------------------------------------------------------------------
//!
//!	\file	/game/credits.h
//!
//!	Definition of class to manage presentation of the credits screen.
//!
//-------------------------------------------------------------------------------------------------

#ifndef _CREDITS_H
#define _CREDITS_H

#include "core/nt_std.h"

//-- Forward declarations.
class CPoint;
class Transform;
class CString;
class CHierarchy;
class CCreditLine;

//-------------------------------------------------------------------------------------------------
//!
//! CXMLCredits class to manager the loading of credits xml data.
//!
//-------------------------------------------------------------------------------------------------
class CXMLCredits
{
public:
	HAS_INTERFACE(CXMLCredits);

	ntstd::List<CHashedString>	m_obXMLCredits;
};

//-------------------------------------------------------------------------------------------------
//!
//! Credits class to manage the display of game credits.
//!
//-------------------------------------------------------------------------------------------------
class CCredits : public Singleton< CCredits >
{
public:

	CCredits();

	void Initialise( void );
	void Run( void );
	void Update( void );
	void Render( void );
	void Pause( void );
	bool DisplayedAll( void );
	bool Terminate( void );

	float GetTopMargin( void );
	float GetBottomMargin( void );
	float GetScrollSpeed( void );
	bool Running( void );

private:

	void Destroy( void );

	bool m_bRunCredits;
	float m_fScrollSpeed;
	bool m_bDisplayedAll;
	float m_fTopMargin;
	float m_fBottomMargin;
	bool m_bAllowUpdate;
	float m_fInitialDelay;

	ntstd::List< CCreditLine* >	m_obCreditLines;

	CXMLCredits* m_pobXMLCredits;	// XML credit data.

	static CHashedString HashSN;	// Hash for SN, (S)ection (N)ame
};

//-------------------------------------------------------------------------------------------------
//!
//! CreditLine class, represents one line of credits information.
//!
//-------------------------------------------------------------------------------------------------
class CCreditLine
{
public:

	friend class CCredits;

	CCreditLine::CCreditLine();
	CCreditLine::~CCreditLine();

	enum eType
	{
		NONE = 0,
		SECTION_NAME,
		PERSON_NAME,
		BLANK_LINE
	};

	void SetType( const eType Type );
	void SetHashedString( const CHashedString HashString );

	const bool StringNeedsToBeInitialised( void )	{ return !(m_pStr != NULL); }

private:

	void InitialisePosition( const int iIndex );
	void InitialiseString( void );
	
	const bool Render( void );
	void EnableRender( const bool bEnable );

	void Update( const float fSpeed );
	void UpdateColour( void );

	const bool OnScreen( void );
	const bool OnScreenSoon( void );
	const bool OffTop( void );
	const bool OffBottom( void );
	
	Transform*		m_pobBaseTransform;
	CPoint			m_BasePosition;
	CString*		m_pStr;
	bool			m_bRender;
	float			m_fFontHeight;
	CHashedString	m_HashedString;
	eType			m_eType;
};

#endif
