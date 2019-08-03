//------------------------------------------------------
//!
//!	\file keywords.h
//!
//------------------------------------------------------

#ifndef _KEYWORDS_H
#define _KEYWORDS_H

static const u_int MAX_KEYWORDS = 6;
static const u_int MAX_KEYWORD_LENGTH = 64;

#ifndef _RELEASE
#define _USE_STRINGS
#endif // _RELEASE

//#define _USE_STRINGS

//--------------------------------------------------
//!
//! CKeywords
//!
//--------------------------------------------------
class CKeywords
{
public:

	CKeywords ();

	CKeywords (const char* pcString);

	void Set (const char* pcString);

	void AddKeyword (const char* pcString);

	void RemoveKeyword (const char* pcString);

	bool operator== (const CKeywords& obOther) const;

	bool operator& (const CKeywords& obOther) const;

	bool Contains( const CHashedString& robHash ) const;
	bool Contains( const char* pcKeyword ) const { return Contains( CHashedString(pcKeyword) ); }

	// Finds the number of matching keywords between two keywords
	u_int ContainsAny( const CKeywords& obOther ) const;

	// Contains all?
	bool ContainsAll( const CKeywords& obOther ) const { return ContainsAny(obOther) == GetKeywordCount(); }

	// Return the number of keywords. 
	u_int GetKeywordCount () const { return m_uiKeywordCount; }

	// Get a keyword
	u_int GetKeyword(u_int uiIndex) const { ntAssert( uiIndex < m_uiKeywordCount ); return m_uiKeywordList[uiIndex]; }

	const char* GetKeywordString () const
	{
#ifdef _USE_STRINGS
	return m_acKeywords;
#else
		return "Non-release builds only";
#endif // _USE_STRINGS
	}

	void Dump (); // Debugging

protected:

	u_int	m_uiKeywordCount;
	u_int	m_uiKeywordList [MAX_KEYWORDS];

#ifdef _USE_STRINGS
	char	m_acKeywords [MAX_KEYWORD_LENGTH];
#endif // _USE_STRINGS

};


#endif // _KEYWORDS_H
