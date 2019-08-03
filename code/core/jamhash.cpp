/***************************************************************************************************
*
*	$Header:: /game/hash.cpp 6     6/06/03 16:18 Dean                                              $
*
*	Hashing Functions
*
*	CHANGES
*
*	6/6/2003	Dean	Created
*
***************************************************************************************************/

#include "jamhash.h"

/***************************************************************************************************
*
*	FUNCTION		CJamHashedString::GenerateHash
*
*	DESCRIPTION		Generates a 32-bit hash from the input string.
*
*	INPUTS			pcString		-	Pointer to a NULL terminated string
*
*	RESULT			A 32-bit hash. 
*
*	NOTES			This function is for internal use by CJamHashedString *only*. Do not attempt to
*					use it directly in your own code.
*
***************************************************************************************************/

unsigned int	CJamHashedString::GenerateHash( const char* pcString )
{
	unsigned int uiTag = 0;

	if ( pcString )
	{
		while( *pcString )
		{
			uiTag = ( uiTag << 7 ) + uiTag;
			uiTag = uiTag + *pcString++;
		}
	}
	return( uiTag );
}





