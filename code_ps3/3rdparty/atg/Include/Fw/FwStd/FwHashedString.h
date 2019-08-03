//--------------------------------------------------------------------------------------------------
/**
	@file	
	
	@brief		Simple Hashed String

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_HASHED_STRING_H
#define FW_HASHED_STRING_H

#include <Fw/FwCrc.h>

//--------------------------------------------------------------------------------------------------
/**
	@class			FwHashedString

	@brief			String

	Instead of storing strings everywhere in the project, we use hashes. These 32-bit unsigned
	integers are used to allow for reasonably efficient storage (and searching of) elements that have
	to be accessed using a name. By encapsulating this hash into an object, it makes client code 
	cleaner, especially when caching hashes locally. 

	For example, if a number of functions need to use the same hash, you'd do this:

	@code
		// Construct a hash for 'left_arm'
		FwHashedString	leftArm( "left_arm" );
		
		// By using the hash string object rather than passing in the string twice, we avoid
		// computing the hash value multiple times. Saving on CPU time.
		myExampleObject->GetChannel( leftArm, ROTATION );
		myExampleObject->GetChannel( leftArm, TRANSLATION );
	@endcode
**/
//--------------------------------------------------------------------------------------------------

class	FwHashedString
{
public:
	
	// Constructors
	FwHashedString() { m_hashValue = kNullHash; };
	
	explicit FwHashedString( u32 hash );
	explicit FwHashedString( const char* pString );
	explicit FwHashedString( const char* pString, size_t length );

	// Copy constructors
	// we rely on default copy construction and = operator since memberwise copy is OK

	// Operators
	bool			operator == ( const FwHashedString& hashedString ) const;
	bool			operator != ( const FwHashedString& hashedString ) const;
	bool			operator <  ( const FwHashedString& hashedString ) const;
	bool			operator >  ( const FwHashedString& hashedString ) const;
	bool			operator <= ( const FwHashedString& hashedString ) const;
	bool			operator >= ( const FwHashedString& hashedString ) const;

	// Retrieve actual u32 data
	u32				Get( void ) const;

	// State query
	bool			IsValid( void ) const;
	bool			IsNull( void ) const;

private:
	enum
	{
		kNullHash = 0
	};

	// note: The hash is designed so hash("") is not zero; this allows us to differentiate "" and a "null pointer to a string"
	static u32				GenerateHash( const char* pString, size_t length)	{ return ~FwCrc32(pString, length); }
	static u32				GenerateHash( const char* pString )					{ return ~FwCrc32String(pString); }

	u32						m_hashValue;
};


inline	FwHashedString::FwHashedString( u32 hash )
{
	m_hashValue = hash;
}

inline	FwHashedString::FwHashedString( const char* pString )
{
	m_hashValue = FwHashedString::GenerateHash( pString );
}

inline	FwHashedString::FwHashedString( const char* pString, size_t length )
{
	m_hashValue = FwHashedString::GenerateHash( pString, length );
}

inline bool FwHashedString::operator==(const FwHashedString& other ) const
{
	return m_hashValue == other.m_hashValue;
}

inline bool FwHashedString::operator!=( const FwHashedString& other ) const
{
	return m_hashValue != other.m_hashValue;
}

inline bool FwHashedString::operator<( const FwHashedString& other ) const
{
	return m_hashValue < other.m_hashValue;
}

inline bool FwHashedString::operator>( const FwHashedString& other ) const
{
	return m_hashValue > other.m_hashValue;
}

inline bool FwHashedString::operator<=( const FwHashedString& other ) const
{
	return m_hashValue <= other.m_hashValue;
}

inline bool FwHashedString::operator>=( const FwHashedString& other ) const
{
	return m_hashValue >= other.m_hashValue;
}

inline u32	FwHashedString::Get( void ) const
{ 
	return m_hashValue;
}

inline bool FwHashedString::IsValid( void ) const
{
	return ( m_hashValue != kNullHash );
}

inline bool	FwHashedString::IsNull( void ) const
{
	return ( m_hashValue == kNullHash );
}

#endif // FW_HASHED_STRING_H
