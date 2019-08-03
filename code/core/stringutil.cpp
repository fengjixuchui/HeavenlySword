/***************************************************************************************************
*
*	$Header:: /game/stringutil.cpp 1     15/08/03 15:50 Giles                                      $
*
*
*
*	CHANGES
*
*	15/8/2003	Giles	Created
*
***************************************************************************************************/

// Includes
#include "stringutil.h"


/***************************************************************************************************
*
*	FUNCTION		CStringUtil::GetBitsFromString
*
*	DESCRIPTION		This function is generally useful for converting string representations of
*					bitwise enumerated values from a string to their integer values. 
*
*					The enum values are linked to their string representations using the STRING_FLAG
*					structure.  The function requires an array of these structures, terminated with
*					a structure containing a NULL pointer to the string component.  Eg:
*
*					enum
*					{	
*						SCREEN_NO_BACK	= ( 1 << 0 ),
*						SCREEN_HUD		= ( 1 << 1 ),
*					};
*
*					const STRING_FLAG astrFlags[] = 
*					{	{ SCREEN_NO_BACK,	"SCREEN_NO_BACK"	},
*						{ SCREEN_HUD,		"SCREEN_HUD"		},
*						{ 0,				0					} };
*
*					The function will deal with multiple flags separated by those characters
*					recognised as separators by CharacterIsSeparator().  If multiple flags are found
*					then the enumerated values are added together for the return value.
*
***************************************************************************************************/

int CStringUtil::GetBitsFromString( const char* pcFlagString, const STRING_FLAG* astrFlagInfo )
{
	// Set up a return value
	int iReturn = 0;

	// Get the length of our input string
	int iLastCharPosition = ( strlen( pcFlagString ) - 1 );

	// Initialise the string 'coordinates' of out token
	int iTokenEnd = iLastCharPosition;
	int iTokenStart = iLastCharPosition;

	while ( iTokenStart > 0 )
	{
		// Find the end point of this token
		while ( ( CharacterIsSeparator( pcFlagString[iTokenEnd] ) ) && ( iTokenEnd > 0 ) )
		{
			iTokenEnd--;
		}

		// Set the start character to the end character
		iTokenStart = iTokenEnd;

		// Scan back along the string until we hit another space or the start of pcFlagString
		while ( ( !CharacterIsSeparator( pcFlagString[iTokenStart] ) ) && ( iTokenStart >= 0 ) )
		{
			iTokenStart--;
		}

		// Move back so we are pointing at the first character
		iTokenStart++;

		// Now run through our array of flags to see if our token equates to one of them
		for ( int iFlag = 0; astrFlagInfo[iFlag].pcFlag != 0; iFlag++ )
		{
			// Only compare the string if the flag has not already been added to the return value
			if ( !( iReturn & astrFlagInfo[iFlag].iBit ) && 
				( strncmp( &pcFlagString[iTokenStart], astrFlagInfo[iFlag].pcFlag, ( iTokenEnd - iTokenStart ) + 1 ) == 0 ) && 
				( strlen( astrFlagInfo[iFlag].pcFlag ) == (size_t)( iTokenEnd - iTokenStart  + 1) ) )
			{
				iReturn |= astrFlagInfo[iFlag].iBit;
				break;
			}
		}

		// Move back so we are pointing at separator
		iTokenStart--;

		// Set the end so we can look for any other token
		iTokenEnd = iTokenStart;
	}

	// Give it to them
	return iReturn;
}


/***************************************************************************************************
*
*	FUNCTION		CStringUtil::GetBitsFromStringW
*
*	DESCRIPTION		As for GetBitsFromString but for wide strings.  The flag string representations
*					must be in the wide format too.
*
*					const STRING_FLAG_W astrFlags[] = 
*					{	{ SCREEN_NO_BACK,	L"SCREEN_NO_BACK"	},
*						{ SCREEN_HUD,		L"SCREEN_HUD"		},
*						{ 0,				0					} };
*
***************************************************************************************************/

int CStringUtil::GetBitsFromStringW( const WCHAR_T* pwcFlagString, const STRING_FLAG_W* astrFlagInfo )
{
	// Set up a return value
	int iReturn = 0;

	// Get the length of our input string
	int iLastCharPosition = ( wcslen( (const wchar_t*)pwcFlagString ) - 1 );

	// Initialise the string 'coordinates' of out token
	int iTokenEnd = iLastCharPosition;
	int iTokenStart = iLastCharPosition;

	while ( iTokenStart > 0 )
	{
		// Find the end point of this token
		while ( ( CharacterIsSeparatorW( pwcFlagString[iTokenEnd] ) ) && ( iTokenEnd > 0 ) )
		{
			iTokenEnd--;
		}

		// Set the start character to the end character
		iTokenStart = iTokenEnd;

		// Scan back along the string until we hit another space or the start of pcFlagString
		while ( ( !CharacterIsSeparatorW( pwcFlagString[iTokenStart] ) ) && ( iTokenStart >= 0 ) )
		{
			iTokenStart--;
		}

		// Move back so we are pointing at the first character
		iTokenStart++;

		// Now run through our array of flags to see if our token equates to one of them
		for ( int iFlag = 0; astrFlagInfo[iFlag].pwcFlag != 0; iFlag++ )
		{
			// Only compare the string if the flag has not already been added to the return value
			if ( !( iReturn & astrFlagInfo[iFlag].iBit ) && 
				( wcsncmp( (const wchar_t*)&pwcFlagString[iTokenStart], (const wchar_t*)astrFlagInfo[iFlag].pwcFlag, ( iTokenEnd - iTokenStart ) + 1 ) == 0 ) &&
				( wcslen( astrFlagInfo[iFlag].pwcFlag ) == (size_t)( iTokenEnd - iTokenStart  + 1) ) )
			{
				iReturn |= astrFlagInfo[iFlag].iBit;
				break;
			}
		}
		
		// Move back so we are pointing at separator
		iTokenStart--;

		// Set the end so we can look for any other token
		iTokenEnd = iTokenStart;
	}

	// Give it to them
	return iReturn;
}

















