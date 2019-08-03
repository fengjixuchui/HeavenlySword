//----------------------------------------------------------------------------------------
//! 
//! \filename core\util_ps3.h
//! 
//----------------------------------------------------------------------------------------
#if !defined( CORE_UTIL_PC_H_ )
#define CORE_UTIL_PC_H_

// alignment macros
#define ALIGNOF( type )	__alignof( type )
#define ALIGNTO_PREFIX( size ) 	__declspec ( align( size ) )	
#define ALIGNTO_POSTFIX( size )

// make the normal one map to windows one
#define vsnprintf _vsnprintf
#define snprintf _snprintf

/***************************************************************************************************
*
*	CLASS			Util
*
*	DESCRIPTION		Static class containing several utility member functions.
*
***************************************************************************************************/

namespace Util
{

//! Is given number a power of 2?
template< typename T>
inline bool	IsPow2( T iNum )
{
	return ( (iNum & ( iNum - 1 )) == 0 );
}

// Return the next highest power of two. See http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
unsigned inline int NextPow2(unsigned int iValue)
{
	iValue--;
	iValue |= iValue >> 1;
	iValue |= iValue >> 2;
	iValue |= iValue >> 4;
	iValue |= iValue >> 8;
	iValue |= iValue >> 16;
	iValue++;
	return iValue;
}

//! returns an aligned version (to alignment) of value
template< typename T>
inline T Align( const T value, const size_t alignment )
{
	ntAssert( IsPow2(alignment) );
	return( (value + (alignment - 1)) & ~(alignment - 1) );
}

//! returns true if value is aligned to alignment
template< typename T>
inline bool IsAligned( const T value, const size_t alignment )
{
	ntAssert( IsPow2(alignment) );
	return !(size_t(value) & (alignment - 1));
}


// do funky file name conversion if required
void		GetFiosFilePath( const char* pcSrc, char* pcDest );

// same as above on PC
void		GetFiosFilePath_Platform( const char* pcSrc, char* pcDest );

// switch directory of loading
void		SetPlatformDir( const char* pPlatformDir );
void		SetNeutralDir( const char* pNeutralDir );
void		SetToPlatformResources();
void		SetToNeutralResources();


// ---------------------------------------------------------------------------------------------
// Floating point & SIMD exception behaviour

#ifndef	_ENABLE_FP_EXCEPTIONS
inline void	EnableFPUExceptions( void )			{};

inline void	EnableFPUInvalidException( void )	{};
inline void	DisableFPUInvalidException( void )	{};
	
inline void	EnableFPUZeroDivException( void )	{};
inline void	DisableFPUZeroDivException( void )	{};
	
inline void	EnableFPUOverflowException( void )	{};
inline void	DisableFPUOverflowException( void )	{};
#else
void	EnableFPUExceptions( void );

void	EnableFPUInvalidException( void );
void	DisableFPUInvalidException( void );
	
void	EnableFPUZeroDivException( void );
void	DisableFPUZeroDivException( void );
	
void	EnableFPUOverflowException( void );
void	DisableFPUOverflowException( void );
#endif	//_ENABLE_FP_EXCEPTIONS	


	
//--------------------------------------------------------------------------------------------------

// remove the extension: "bar.pibble" returns "bar", "C:/foo/bar.pibble" returns "C:/foo/bar"
const char* NoExtension( const char* pcFullPath );

// remove the last extension: "bar.pibble.pibble" returns "bar.pibble", "C:/foo/bar.pibble.pibble" returns "C:/foo/bar.pibble"
const char* NoLastExtension( const char* pcPath );

// extract base filename from a full path eg "C:/foo/bar.pibble" returns "bar.pibble"
const char* BaseName( const char* pcFullPath );

// extract base filename from a full path eg "C:/foo/bar.pibble" returns "C:/foo"
const char* DirName( const char* pcFullPath );

// join together a dir path and a filename (a trailing path separator on pcDir is optional)
const char* JoinPath( const char* pcDir, const char* pcFile );

// make the given string uppercase
const char* Upppercase( const char* pcPath);

#if defined(PLATFORM_PC)
// Safe mechanism to get ntError from a failed DX call
bool DXSafeCheckError( HRESULT hr, const char* pMessage, ... );
#endif


}; // end namespace util

#endif // _UTIL_H

