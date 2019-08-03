/***************************************************************************************************
*
*	$Header: /tools/jamhash/hashserver/hash.h 8     24/06/03 16:56 Ben $
*
*	Hashing functions
*
*	Note that this file is also used by the exporter, so please don't add anything that pulls in
*	game-specific files!
*
*	CHANGES
*
*	29 Apr 2003	Ben		Snarfed from KFC codebase
*
***************************************************************************************************/

#ifndef _JAMHASH_H_
#define _JAMHASH_H_



/***************************************************************************************************
*
*	CLASS			CJamHashedString
*
*	DESCRIPTION		Instead of storing strings everywhere in the project, we use hashes. These 32-bit
*					unsigned integers are used to allow efficient storage (and searching of) elements
*					that have to be accessed using a name.
*
***************************************************************************************************/

class	CJamHashedString
{
public:
	
	// Constructors
	CJamHashedString() { m_uiHash = 0; };
	CJamHashedString( const char* pcString );

	// Operators
	CJamHashedString&	operator =	( const CJamHashedString& obHashedString );
	bool			operator == ( const CJamHashedString& obHashedString ) const;
	bool			operator != ( const CJamHashedString& obHashedString ) const;
	bool			operator <  ( const CJamHashedString& obHashedString ) const;
	bool			operator >  ( const CJamHashedString& obHashedString ) const;
	bool			operator <= ( const CJamHashedString& obHashedString ) const;
	bool			operator >= ( const CJamHashedString& obHashedString ) const;

	// Determine whether it's valid
	bool			IsValid( void ) const;

	unsigned int	GetValue( void ) const { return m_uiHash; };
	// to be compatible with ATG FwHashedString
	unsigned int	Get( void ) const { return m_uiHash; };

	static unsigned int GenerateHash( const char* pcString );

protected:
	unsigned int	m_uiHash;
};

inline	CJamHashedString::CJamHashedString( const char* pcString )
{
	m_uiHash = CJamHashedString::GenerateHash( pcString );
}


inline	CJamHashedString&	CJamHashedString::operator = ( const CJamHashedString& obHashedString )
{
	m_uiHash = obHashedString.m_uiHash;
	return *this; 
}

inline	bool	CJamHashedString::operator == ( const CJamHashedString& obHashedString ) const
{
	return ( m_uiHash == obHashedString.m_uiHash );
}

inline	bool	CJamHashedString::operator != ( const CJamHashedString& obHashedString ) const
{
	return ( m_uiHash != obHashedString.m_uiHash );
}

inline	bool	CJamHashedString::operator < ( const CJamHashedString& obHashedString ) const
{
	return ( m_uiHash < obHashedString.m_uiHash );
}

inline	bool	CJamHashedString::operator > ( const CJamHashedString& obHashedString ) const
{
	return ( m_uiHash > obHashedString.m_uiHash );
}

inline	bool	CJamHashedString::operator <= ( const CJamHashedString& obHashedString ) const
{
	return ( m_uiHash <= obHashedString.m_uiHash );
}

inline	bool	CJamHashedString::operator >= ( const CJamHashedString& obHashedString ) const
{
	return ( m_uiHash >= obHashedString.m_uiHash );
}

inline	bool	CJamHashedString::IsValid( void ) const
{
	return ( m_uiHash != 0 );
}




#endif // end of _JAMHASH_H_
