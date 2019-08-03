/***************************************************************************************************
*
*	DESCRIPTION		Useful snippets for GUI functionality
*
*	NOTES
*
***************************************************************************************************/

// Includes
#include "guiutil.h"
#include "core\timer.h"
#include "gui\guitext.h"
#include "anim/hierarchy.h"
#include "gui/guimanager.h"
#include "gui/guisettings.h"

/***************************************************************************************************
*
*	FUNCTION		CGuiTimer::Set
*
*	DESCRIPTION
*
***************************************************************************************************/

void CGuiTimer::Set( float fCountdownTime )
{
	m_fCountdownTime = fCountdownTime;
	m_fInitialTime = fCountdownTime;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiTimer::Reset
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CGuiTimer::Reset( void )
{
	m_fCountdownTime = m_fInitialTime;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiTimer::Update
*
*	DESCRIPTION
*
***************************************************************************************************/

void CGuiTimer::Update( void )
{
	if ( m_fCountdownTime > 0.0f )
		m_fCountdownTime -= CTimer::Get().GetSystemTimeChange();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiTimer::Passed
*
*	DESCRIPTION		True if the time has passed
*
***************************************************************************************************/

bool CGuiTimer::Passed( void )
{
	return ( m_fCountdownTime <= 0.0f );
}

/***************************************************************************************************
*
*	FUNCTION		CGuiTimer::Time
*
*	DESCRIPTION		Give us the current time of the timer
*
***************************************************************************************************/

float CGuiTimer::Time( void )
{
	return m_fCountdownTime;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiTimer::NormalisedTime
*
*	DESCRIPTION		Give us the amount of the timer passed. 
*
***************************************************************************************************/

float CGuiTimer::NormalisedTime( void )
{
	float fValue = m_fCountdownTime/m_fInitialTime;
	if (fValue > 1.0f)
		fValue = 1.0f;
	if (fValue < 0.0f)
		fValue = 0.0f;

	return fValue;
}

/***************************************************************************************************
*
*	FUNCTION		GuiUtil::SetFloat
*
*	DESCRIPTION		process a float
*
***************************************************************************************************/

bool GuiUtil::SetFloat(const char* pcValue, float* const pFloat)
{
	// We'll get a float from the string
	int iResult = sscanf( pcValue, "%f", pFloat); 

	// Make sure we extracted the value
	ntAssert( iResult == 1 );
	UNUSED( iResult );

	return true;
}

bool GuiUtil::SetFloats(const char* pcValue, float* const pFloatA, float* const pFloatB)
{
	// We'll get two floats from the string
	int iResult = sscanf( pcValue, "%f,%f", pFloatA, pFloatB); 

	// Make sure we extracted two values
	ntAssert( iResult == 2 );
	UNUSED( iResult );

	return true;
}

bool GuiUtil::SetFloats(const char* pcValue, float* const pFloatA, float* const pFloatB, float* const pFloatC)
{
	// We'll get 3 floats from the string
	int iResult = sscanf( pcValue, "%f,%f,%f", pFloatA, pFloatB, pFloatC); 

	// Make sure we extracted 3 values
	ntAssert( iResult == 3 );
	UNUSED( iResult );

	return true;
}

bool GuiUtil::SetFloats(const char* pcValue, float* const pFloatA, float* const pFloatB, float* const pFloatC, float* const pFloatD)
{
	// We'll get 4 floats from the string
	int iResult = sscanf( pcValue, "%f,%f,%f,%f", pFloatA, pFloatB, pFloatC, pFloatD); 

	// Make sure we extracted 4 values
	ntAssert( iResult == 4 );
	UNUSED( iResult );

	return true;
}

bool GuiUtil::SetString( const char* pcValue, const char** const ppcString )
{
	char* pcTempString = NT_NEW char[ strlen( pcValue) + 1 ];
	strcpy( pcTempString, pcValue );
	*ppcString = pcTempString;
	return true;
}

bool GuiUtil::SetInt(const char* pcValue, int* const pInt)
{
	// We'll get an int from the string
	int iResult = sscanf( pcValue, "%i", pInt); 

	// Make sure we extracted the value
	ntAssert( iResult == 1 );
	UNUSED( iResult );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		GuiUtil::SetFlags
*
*	DESCRIPTION		Sets up property flags from a string. Multiple flags in the string
*					should be seperated by spaces.
*
*					We start from the end of the string, working backwards to find tokens, comparing
*					each with a string representation of the enumeration value.
*
***************************************************************************************************/

bool GuiUtil::SetFlags( const char* pcValue, const CStringUtil::STRING_FLAG* const pastrFlags, int* const piFlags )
{
	// Get the bits represented in the string
	*piFlags = CStringUtil::GetBitsFromString( pcValue, pastrFlags );

	// If we are here, yet no flags have been recognised then there is something wrong
	ntAssert_p( *piFlags != 0, ("Unknown flag %s\n", pcValue) );

	return true;
}

bool GuiUtil::SetFlags( const WCHAR_T* pwcValue, const CStringUtil::STRING_FLAG_W* const pastrFlags, int* const piFlags )
{
	// Get the bits represented in the string
	*piFlags = CStringUtil::GetBitsFromStringW( pwcValue, pastrFlags );

	// If we are here, yet no flags have been recognised then there is something wrong
	ntAssert_p( *piFlags != 0, ("Unknown flag %s\n", pwcValue) );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		GuiUtil::SetEnum
*
*	DESCRIPTION		Seperated from SetFlags as a zero value is allowed
*
***************************************************************************************************/

bool GuiUtil::SetEnum( const char* pcValue, const CStringUtil::STRING_FLAG* const pastrFlags, int* const piFlags )
{
	// Get the bits represented in the string
	*piFlags = CStringUtil::GetBitsFromString( pcValue, pastrFlags );

	return true;
}

bool GuiUtil::SetEnum( const WCHAR_T* pwcValue, const CStringUtil::STRING_FLAG_W* const pastrFlags, int* const piFlags )
{
	// Get the bits represented in the string
	*piFlags = CStringUtil::GetBitsFromStringW( pwcValue, pastrFlags );

	return true;
}

bool GuiUtil::SetBool( const char* pcValue, bool* const pBool )
{
	if (( strcmp( pcValue, "true" ) == 0 ) || ( strcmp( pcValue, "TRUE" ) == 0 ))
	{
		*pBool = true;
	}
	
	if (( strcmp( pcValue, "false" ) == 0 ) || ( strcmp( pcValue, "FALSE" ) == 0 ))
	{
		*pBool = false;
	}

	// If we are here we are successful
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CStringTable::CalculateHash
*
*	DESCRIPTION		Calculates a hash based on the ascii codes of characters in a string.
*
***************************************************************************************************/

u_int GuiUtil::CalculateHash( const char* pcIdentifier )
{
	u_int	uiHashValue	= 0;
	int		iIdentifierLength = strlen( pcIdentifier );

	for ( int iCount = iIdentifierLength; iCount > 0; iCount-- )
	{
		uiHashValue = ( uiHashValue << 7 ) + uiHashValue;
		uiHashValue = uiHashValue + *pcIdentifier++;
	}

	return( uiHashValue );
}


/***************************************************************************************************
*
*	FUNCTION		CStringTable::CalculateHash
*
*	DESCRIPTION		We need to calculate hashes from the unicode identifiers from the string.  The
*					strings are wchar_ts but should only use ascii characters.  In this function we
*					check that that is the case as we convert the identifier to a char string and
*					then use that string to obtain the hash value.
*
*					...err, that is what we used to do.  I have now replicated the hash calculater
*					from above to work directly on the unicode string.  Otherwise this function
*					would perform countless new/deletes during game startup.  I didn't really want
*					to duplicate the algorythm but i can't see any other quick way of doing things.
*					I still check that the hash values translate between unicode and ascii in 
*					debug builds.
*
***************************************************************************************************/

u_int GuiUtil::CalculateHash( const WCHAR_T* pwcIdentifier )
{
	u_int	uiHashValue	= 0;
	int		iIdentifierLength = wcslen( (const wchar_t*)pwcIdentifier );

	for ( int iCount = iIdentifierLength; iCount > 0; iCount-- )
	{
		// Make sure that our character does not contain something non ascii
		ntAssert( *pwcIdentifier <= 255 );

		uiHashValue = ( uiHashValue << 7 ) + uiHashValue;
		uiHashValue = uiHashValue + *pwcIdentifier++;
	}

	return( uiHashValue );
}


void GuiUtil::RenderString(float fX, float fY, CStringDefinition* pobDef, const char* str)
{
	// Create a point with the data
	CPoint obBasePoint( fX, fY, 0.0f );

	// Create a matrix with the point
//	CMatrix obBaseMatrix( CONSTRUCT_IDENTITY );
//	obBaseMatrix.SetTranslation( obBasePoint );

	Transform t;
//	t.SetLocalMatrix(obBaseMatrix);
	t.SetLocalTranslation(obBasePoint);

	CHierarchy::GetWorld()->GetRootTransform()->AddChild( &t );
    
	WCHAR_T buf[128] = {0};
	mbstowcs(buf,str, strlen(str));
	CString* pStr = CStringManager::Get().MakeString(buf, *pobDef, &t, CGuiUnit::RENDER_SCREENSPACE);

	pStr->Render( );

	CStringManager::Get().DestroyString(pStr);

	t.RemoveFromParent();
}

/**
	Get a pointer to the appropriate font given a description of it.

	@note Descriptions are either "BodyFont" or "TitleFont".
	@note For japanese the body font is always chosen.
*/
CFont *GuiUtil::GetFontFromDescription(const char *strFontDescription)
{
	CFont *pobFont;


	if (CStringManager::Get().GetLanguage() != NT_SUBTITLE_JAPANESE &&
		CStringManager::Get().GetLanguage() != NT_SUBTITLE_CHINESE &&
		CStringManager::Get().GetLanguage() != NT_SUBTITLE_KOREAN)
	{
		if (stricmp(strFontDescription, "BodyFont") == 0)
		{
			pobFont = CFontManager::Get().GetFont( CGuiManager::Get().GuiSettings()->BodyFont() );
		}
		else
		{
			pobFont = CFontManager::Get().GetFont( CGuiManager::Get().GuiSettings()->TitleFont() );
		}
	}
	else
	{
		pobFont = CFontManager::Get().GetFont( CGuiManager::Get().GuiSettings()->BodyFont() );
	}

	return pobFont;
}

/**
*/
CFont *GuiUtil::GetFontAbstracted(const char *pcFontName)
{
	if (CStringManager::Get().GetLanguage() == NT_SUBTITLE_JAPANESE ||
		CStringManager::Get().GetLanguage() == NT_SUBTITLE_CHINESE ||
		CStringManager::Get().GetLanguage() == NT_SUBTITLE_KOREAN)
	{
		if (strcmp(pcFontName, "Title") == 0)
		{
			// Only allow body in these languages
			pcFontName = "Body";
		}
	}
	
	return CFontManager::Get().GetFont( pcFontName );
}
