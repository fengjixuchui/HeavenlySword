/***************************************************************************************************
*
*	$Header:: /game/stringutil.h 1     15/08/03 15:50 Giles                                        $
*
*
*
*	CHANGES
*
*	15/8/2003	Giles	Created
*
***************************************************************************************************/

#ifndef _STRINGUTIL_H
#define _STRINGUTIL_H

/***************************************************************************************************
*
*	CLASS			CStringUtil
*
*	DESCRIPTION		Static class containing utility functions for dealing with strings.
*
***************************************************************************************************/

class CStringUtil
{

public:

	// For linking enumurated values to a string representation 
	struct STRING_FLAG
	{
		int			iBit;
		const char* pcFlag;
	};

	// For linking enumurated values to a wide string representation 
	struct STRING_FLAG_W
	{
		int				iBit;
		const WCHAR_T*	pwcFlag;
	};

	// The useful methods
	inline	static bool		StrCmp( const char* pcA, const char* pcB );
	inline	static bool		CharacterIsSeparator( const char cCharacter );
	inline	static bool		CharacterIsSeparatorW( const WCHAR_T wcCharacter );
			static int		GetBitsFromString( const char* pcFlagString, const STRING_FLAG* astrFlagInfo );  
			static int		GetBitsFromStringW( const WCHAR_T* pwcFlagString, const STRING_FLAG_W* astrFlagInfo );  
};


/***************************************************************************************************
*	
*	FUNCTION		CStringUtil::StrCmp
*
*	DESCRIPTION		A more efficient version of strcmp. Knowing whether a string is greater or 
*					less than another is hardly ever required by a project - so this version bails
*					out at the earliest possible opportunity. As mentioned in the note, this does
*					*not* return the same type as ANSI strcmp, and you should also be aware that 
*					it returns 'true' if both pointers are NULL.. which can be handy.
*
*	INPUTS			pcFirst, pcSecond	- Two strings to compare with each other.
*
*	RESULT			bool				- true if strings match.
*
*	NOTES			This function's return type is NOT the same as ANSI strcmp!
*
***************************************************************************************************/

bool CStringUtil::StrCmp( const char* pcFirst, const char* pcSecond )
{
	// Same addresses must mean identical string... this will pass with both strings being 
	// NULL pointers

	if ( pcFirst == pcSecond )
		return( true );
	
	// If either of the arguments is NULL and we're still here, then it's not a match - both
	// pointers must be NULL, or none at all..

	if ( ( pcFirst == NULL ) || ( pcSecond == NULL ) )
		return( false );

	// First not matching test means inequality. If the test matches but the test character was a
	// terminator, strings are identical...

	do
	{
		if ( *pcFirst++ != *pcSecond )
			return( false );
	} while ( *pcSecond++ != 0 );

	return( true );
}


/***************************************************************************************************
*	
*	FUNCTION		CStringUtil::CharacterIsSeparator
*
*	DESCRIPTION		Tells the user if the character should be regarded as one that seperates tokens
*					in a string.
*
*					The characters in question are "<SPACE>" "," ";" "<TAB>" 
*
***************************************************************************************************/

bool CStringUtil::CharacterIsSeparator( const char cCharacter )
{
	// Check for that character values...
	if ( ( cCharacter == 0x020 )
		 ||
		 ( cCharacter == 0x02C )
		 ||
		 ( cCharacter == 0x03B )
		 ||
		 ( cCharacter == 0x009 ) )
	{
		return true;
	}

	return false;
}


/***************************************************************************************************
*	
*	FUNCTION		CStringUtil::CharacterIsSeparatorW
*
*	DESCRIPTION		Tells the user if the wide character should be regarded as one that seperates 
*					tokens in a string.
*
*					The characters in question are "<SPACE>" "," ";" "<TAB>" 
*
***************************************************************************************************/

bool CStringUtil::CharacterIsSeparatorW( const WCHAR_T wcCharacter )
{
	// Check for that character values...
	if ( ( wcCharacter == 0x020 )
		 ||
		 ( wcCharacter == 0x02C )
		 ||
		 ( wcCharacter == 0x03B )
		 ||
		 ( wcCharacter == 0x009 ) )
	{
		return true;
	}

	return false;
}


#endif // _STRINGUTIL_H
