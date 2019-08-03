//----------------------------------------------------------------------------------------
//! 
//! \filename core\util_ps3.h
//! 
//----------------------------------------------------------------------------------------
#if !defined( CORE_UTIL_PS3_H_ )
#define CORE_UTIL_PS3_H_


// cross compiler alignment macros
#define ALIGNOF( type )			__alignof__( type )

// GCC want attributes after the decleration, VC wants them before <sigh>
#define ALIGNTO_PREFIX( size )	
#define ALIGNTO_POSTFIX( size )		__attribute__ ((aligned( size )))

namespace Util
{
//! Is given number a power of 2?
template< typename T>
inline bool	IsPow2( const T iNum )
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

inline uint32_t GetRightMostBit( uint32_t val )
{
	return val & ( -val );
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

// returns us a lower case fios path without any device paths
void GetFiosFilePath( const char* pSrc, char* pDest );
void GetFiosFilePath_Platform( const char* pSrc, char* pDest );

// returns us a lower case full path for the relevant devices, for use with cellFS
void GetFullBluRayFilePath( const char* pSrc, char* pDest, bool add_content_dir = true );
void GetFullGameDataFilePath( const char* pSrc, char* pDest, bool add_content_dir = true );
void GetFullSysCacheFilePath( const char* pSrc, char* pDest, bool add_content_dir = true );

// retrieve the absolute path to our SPU programs
void GetSpuProgramPath( const char* pSrc, char* pDest, int32_t max_length );

void SetToPlatformResources();
void SetToNeutralResources();

// for fios config
const char* GetBluRayPath();

// note, these can be null early on in game startup
const char* GetGameDataPath();
const char* GetSysCachePath();

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
 
}; // end namespace util
	

//--------------------------------------------------------------------------------------------------



#endif //CORE_UTIL_PS3_H_
