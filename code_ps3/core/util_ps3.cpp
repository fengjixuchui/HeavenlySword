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
#include "core/osddisplay.h"
#include "core/fileio_ps3.h"
#include "game/shellconfig.h"

#include <ctype.h>
#include <sys/paths.h>

bool g_message_box_override = false;
bool g_using_neutral = true;
extern const char *g_GameDataPath;
extern const char *g_SysCachePath;
extern const char* g_BluRayPath;

namespace Util
{

void SetToPlatformResources()
{
	g_using_neutral = false;
}

void SetToNeutralResources()
{
	g_using_neutral = true;
}

const char* GetBluRayPath()
{
	return g_BluRayPath;
}

const char* GetGameDataPath()
{
	return g_GameDataPath;
}

const char* GetSysCachePath()
{
	return g_SysCachePath;
}

// deliberately not visible to the rest of the codebase, except file system init points
const char* GetAppHomePath()
{
	return SYS_APP_HOME"/";
}

const char* GetBluRayDevicePath()
{
	return SYS_DEV_BDVD"/PS3_GAME/USRDIR/hs/";
}

void GetContentDirectory( char* pDest )
{
	if (g_using_neutral)
		strcat( pDest, "content_neutral/" );
	else
		strcat( pDest, "content_ps3/" );
}

void StringToLowerInplace( char* pString )
{
	while( *pString != 0 )
	{
		if( *pString >= 'A' && *pString <= 'Z' )
		{
			*pString = tolower( *pString );
		}
		pString++;
	}
}

void ConvertSlashes( char* pString )
{
	char* pNext = strstr( pString, "\\" );
	while (pNext)
	{
		*pNext = '/';
		pNext = strstr( pNext, "\\" );
	};
}

/***************************************************************************************************
*
*	FUNCTION		Util::GetFiosFilePath
*
*	DESCRIPTION		convert to a lowercase FIOS path
*
***************************************************************************************************/
void	GetFiosFilePath( const char* pSrc, char* pDest )
{
	strcpy( pDest, "" );
	GetContentDirectory( pDest );		// prepend content
	strcat( pDest, pSrc );				// add file
	StringToLowerInplace( pDest );		// make lower case
	ConvertSlashes( pDest );			// convert slashes
}

/***************************************************************************************************
*
*	FUNCTION		Util::GetFullFilePath
*
*	DESCRIPTION		convert to a full starting path
*
***************************************************************************************************/
void	GetFullFilePath( const char* pSrc, const char* pRoot, char* pDest, bool add_content_dir )
{
	ntError_p( pRoot != NULL, ("Bad file root, are you using a path before its initialised in shell?") );
	strcpy( pDest, pRoot );							// start with root

	if ( add_content_dir )
	{
		GetContentDirectory( pDest );				// prepend content (if requested).
	}

	strcat( pDest, pSrc );							// add file
	StringToLowerInplace( pDest + strlen(pRoot) );	// make lower case
	ConvertSlashes( pDest );						// convert slashes
}

void	GetFullBluRayFilePath( const char* pSrc, char* pDest, bool add_content_dir/* = true*/ )
{
	GetFullFilePath(	pSrc, GetBluRayPath(), pDest, add_content_dir );
}

void	GetFullGameDataFilePath( const char* pSrc, char* pDest, bool add_content_dir/* = true*/ )
{
	GetFullFilePath(	pSrc,
						g_ShellOptions->m_bUsingHDD ? GetGameDataPath() : GetAppHomePath(),
						pDest,
						add_content_dir );
}

void	GetFullSysCacheFilePath( const char* pSrc, char* pDest, bool add_content_dir/* = true*/ )
{
	GetFullFilePath(	pSrc,
						g_ShellOptions->m_bUsingHDD ? GetSysCachePath() : GetAppHomePath(),
						pDest,
						add_content_dir );
}

/***************************************************************************************************
*
*	FUNCTION		Util::GetFiosFilePath_Platform
*
*	DESCRIPTION		Platform variants of the above
*
***************************************************************************************************/
void	GetFiosFilePath_Platform( const char* pSrc, char* pDest )
{
	bool bOld = g_using_neutral;
	g_using_neutral = false;
	GetFiosFilePath( pSrc, pDest );
	g_using_neutral = bOld;
}

/***************************************************************************************************
*
*	FUNCTION		Util::GetSpuProgramPath
*
*	DESCRIPTION		retrieve position of SPU exectuatble
*
***************************************************************************************************/
void	GetSpuProgramPath( const char* pcSrc, char* pcDest, int32_t max_length )
{

#	ifdef	_RELEASE
#		define SPU_BUILD_PATH	"/rel/"
#	elif	_DEVELOPMENT
#		define SPU_BUILD_PATH	"/dev/"
#	elif	_DEBUG_FAST
#		define SPU_BUILD_PATH	"/dbf/"
#	elif	_DEBUG
#		define SPU_BUILD_PATH	"/dbg/"
#	else
#		error Unknown build type!
#	endif

	snprintf( pcDest, max_length, "%sspu%s%s", GetBluRayPath(), SPU_BUILD_PATH, pcSrc );
}

// temporary buffer for returning filenames from utility functions
static char s_FileNameTmpBuf[ MAX_PATH ];

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

} // end namespace Util
