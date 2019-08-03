/***************************************************************************************************
*
*   $Header:: /game/util.cpp 1     17/02/03 14:50 Dean                                             $
*
*	Static utility class. Note this is separate from CRWUtil, which contains Renderware-specific
*	utility functions.
*
*	CHANGES		
*
*	11/05/2001	dean	Created
*
***************************************************************************************************/

#include "core/util.h"
#include "core/OSDDisplay.h"
#include <dxerr9.h>
#include <direct.h>	// for chdir()

bool g_message_box_override = false;
bool g_using_neutral = true;
static const int g_iMAX_CHAR = 32;
char g_pPlatformDir[g_iMAX_CHAR];
char g_pNeutralDir[g_iMAX_CHAR];

bool SetResourceDirectory( const char* pDir )
{
	int iResult = chdir( pDir );
	if( iResult < 0 )
	{
		MessageBox( 0, "chdir() failed - check your config and make sure your content path is OK.", 
			"Hold It!", MB_ICONEXCLAMATION | MB_SYSTEMMODAL );
		return false;
	}

	ntError_p( iResult>=0, ( "chdir( '%s' ) failed!", pDir ) );
	return true;
}

namespace Util
{

void SetPlatformDir( const char* pPlatformDir )
{
	int len = strlen( pPlatformDir );
	ntError( len < g_iMAX_CHAR );
	strncpy( g_pPlatformDir, pPlatformDir, len );
}

void SetNeutralDir( const char* pNeutralDir )
{
	int len = strlen( pNeutralDir );
	ntError( len < g_iMAX_CHAR );
	strncpy( g_pNeutralDir, pNeutralDir, len );
}

void SetToPlatformResources()
{
	SetResourceDirectory( g_pPlatformDir );
}

void SetToNeutralResources()
{
	SetResourceDirectory( g_pNeutralDir );
}

static void StringToLowerInplace( char* pString )
{
	while( *pString != 0 )
	{
		if( *pString >= 'A' && *pString <= 'Z' )
		{
			*pString = (char)tolower( *pString );
		}
		pString++;
	}
}

/***************************************************************************************************
*
*	FUNCTION		Util::GetFiosFilePath
*
*	DESCRIPTION		do funky file name conversion if required
*
***************************************************************************************************/
void	Util::GetFiosFilePath( const char* pcSrc, char* pcDest )
{
	strcpy( pcDest, pcSrc );
	StringToLowerInplace( pcDest );

	char* pcNext = strstr( pcDest, "\\" );
	
	while (pcNext)
	{
		*pcNext = '/';
		pcNext = strstr( pcNext, "\\" );
	};
}

void	Util::GetFiosFilePath_Platform( const char* pcSrc, char* pcDest )
{
	GetFiosFilePath(pcSrc,pcDest);
}

/***************************************************************************************************
*
*	FUNCTION		Util Exception Support
*
*	DESCRIPTION		Functions to control floating point exceptions.
*
***************************************************************************************************/

#ifdef	_ENABLE_FP_EXCEPTIONS

void	Util::EnableFPUExceptions( void )
{
	_controlfp( 0, _EM_ZERODIVIDE );
	_controlfp( 0, _EM_INVALID );
	_controlfp( 0, _EM_OVERFLOW );

#ifdef	_USE_SSE
	_MM_SET_EXCEPTION_MASK( _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
#endif	//_USE_SSE
}

void	Util::DisableFPUInvalidException( void )
{
	_controlfp( _EM_INVALID, _EM_INVALID );
}
	
void	Util::EnableFPUInvalidException( void )
{
	_clearfp();
	_controlfp( 0, _EM_INVALID );
}

void	Util::DisableFPUZeroDivException( void )
{
	_controlfp( _EM_ZERODIVIDE, _EM_ZERODIVIDE );
}
	
void	Util::EnableFPUZeroDivException( void )
{
	_clearfp();
	_controlfp( 0, _EM_ZERODIVIDE );
}

void	Util::DisableFPUOverflowException( void )
{
	_controlfp( _EM_OVERFLOW, _EM_OVERFLOW );
}
	
void	Util::EnableFPUOverflowException( void )
{
	_clearfp();
	_controlfp( 0, _EM_OVERFLOW );
}

#endif	//_ENABLE_FP_EXCEPTIONS





// temporary buffer for returning filenames from utility functions
static char s_FileNameTmpBuf[ MAX_PATH ];

// make the given string uppercase
const char* Upppercase( const char* pcPath)
{
	int i;
	for (i=0; pcPath[i]; i++)
	{
		s_FileNameTmpBuf[i] = (char)toupper(pcPath[i]);
	}
	s_FileNameTmpBuf[i] = 0;
	return s_FileNameTmpBuf;
}

// remove the extension: "bar.pibble" returns "bar", "C:/foo/bar.pibble" returns "C:/foo/bar"
const char* NoExtension( const char* pcPath )
{
	int iSize = 0;
	while(pcPath[iSize]!='\0' && pcPath[iSize]!='.')
	{
		++iSize;
	}
	strncpy( s_FileNameTmpBuf, pcPath, iSize );
	s_FileNameTmpBuf[iSize]='\0';
	return s_FileNameTmpBuf;	
}

// remove the last extension: "bar.pibble.pibble" returns "bar.pibble", "C:/foo/bar.pibble.pibble" returns "C:/foo/bar.pibble"
const char* NoLastExtension( const char* pcPath )
{
	int iSize = strlen(pcPath) - 1;
	while(iSize >= 0 && pcPath[iSize]!='/' && pcPath[iSize]!='\\' && pcPath[iSize]!=':' && pcPath[iSize]!='.')
	{
		--iSize;
	}

	if (iSize < 0 || pcPath[iSize] != '.')
		iSize =  strlen(pcPath);

	strncpy( s_FileNameTmpBuf, pcPath, iSize );
	s_FileNameTmpBuf[iSize]='\0';
	return s_FileNameTmpBuf;
}

// extract base filename from a full path eg "C:/foo/bar.pibble" returns "bar.pibble"
const char* BaseName( const char* pcFullPath )
{
	const char* p = pcFullPath;
	const char* pcBase = 0;
	while( *p )
	{
		if( *p == '/' || *p == '\\' || *p ==':' )
			pcBase = p+1;
		++p;
	}

	if( pcBase )
		return pcBase;
	else
		return pcFullPath;
}


// extract base filename from a full path eg "C:/foo/bar.pibble" returns "C:/foo"
const char* DirName( const char* pcFullPath )
{
	const char* pcSrc = pcFullPath;
	char* pcDest = s_FileNameTmpBuf;
	char* pcLastSep = pcDest;
	while( *pcSrc )
	{
		char c = *pcSrc++;
		if( c == '/' || c == '\\' || c ==':' )
			pcLastSep = pcDest;
		*pcDest++ = c;
	}
	*pcLastSep = '\0';
	return s_FileNameTmpBuf;
}


// join together a dir path and a filename (a trailing path separator on pcDir is optional)
const char* JoinPath( const char* pcDir, const char* pcFile )
{
	strcpy( s_FileNameTmpBuf, pcDir );
	int i = strlen( s_FileNameTmpBuf );
	if( i>0	&& s_FileNameTmpBuf[i-1] != '/' && s_FileNameTmpBuf[i-1] != '\\' )
	{
		s_FileNameTmpBuf[i] = '/';
		++i;
	}
	strcpy( &s_FileNameTmpBuf[i], pcFile );
	return s_FileNameTmpBuf;
}

// Safe mechanism to get ntError from a failed DX call
bool DXSafeCheckError( HRESULT hr, const char* pMessage, ... )
{
	// return true if we failed
	if (!SUCCEEDED(hr))
	{
		// static buffer to _vsnprintf the text into
		static char	aBuffer[MAX_PATH*2];
		aBuffer[sizeof(aBuffer) - 1] = 0;

		// get the final text buffer
		va_list	argList;
		va_start(argList, pMessage);
		_vsnprintf(aBuffer, sizeof(aBuffer) - 1, pMessage, argList);
		va_end(argList);

		ntPrintf( "D3DError: %s: %s\n", aBuffer, DXGetErrorString9(hr) );
		ntAssert(0);
		return true;
	}
	return false;
}

} // end namespace Util