//--------------------------------------------------------------------------------------------------
/**
	@file		FwHelpers.h
	
	@brief		Helper Macros, Templates, and Classes

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_HELPERS_H
#define	FW_HELPERS_H

// Common macros.. 
#undef	min
#undef	max

#define FW_UNUSED(X)	(void)X

/// These macros are useful for handling 4 character tags
/// e.g. when loading a BIFF file where you compare a u32 against a constant
/// if ( tag == FW_MAKE_TAG( 'A','T','G',' ' ) ) ...
#if FW_BIG_ENDIAN
#define FW_MAKE_TAG( a, b, c, d )	( ( ( ( a ) & 0xff ) << 24 ) | ( ( ( b ) & 0xff ) << 16 ) | ( ( ( c ) & 0xff ) <<  8 ) | ( ( ( d ) & 0xff ) <<  0 ) )
#else
#define FW_MAKE_TAG( a, b, c, d )	( ( ( ( a ) & 0xff ) <<  0 ) | ( ( ( b ) & 0xff ) <<  8 ) | ( ( ( c ) & 0xff ) << 16 ) | ( ( ( d ) & 0xff ) << 24 ) ) 
#endif


// Macro trickery to join two arguments together. This works even when the arguments themselves are macros,
// in which case, they're expanded before being concatenated together.
//
// For example,
//
//		FW_JOIN_MACRO_ARGS(HELLO_FROM_,__LINE__);
//
// expands to HELLO_FROM_34 (on line 34) rather than HELLO_FROM___LINE__

#define FW_JOIN_MACRO_ARGS(A, B) 			FW_DO_JOIN_MACRO_ARGS(A, B)
#define FW_DO_JOIN_MACRO_ARGS(A, B)  	   	FW_DO_JOIN_MACRO_ARGS_2(A, B)
#define FW_DO_JOIN_MACRO_ARGS_2(A, B) 	   	A##B
#define FW_JOIN_MACRO_ARGS3(A,B,C)			A##B##C

// Platform-independent variable alignment control
#ifdef	_MSC_VER
#define	FW_ALIGN_BEGIN( x )		__declspec( align( x ) ) 
#define	FW_ALIGN_END( x )		
#else
#define	FW_ALIGN_BEGIN( x )
#define	FW_ALIGN_END( x )		__attribute__( ( aligned( x ) ) )
#endif


// Macros to help with embedding and referencing data in elfs and exes
// To embed data, simply add the file you wish to embed to the Imogen <source> list.
//
// For Example:
//
//	<source>
// 		<file>../../Data/Textures/rockwall.dds</file>
//	</source>
//
// Then in your cpp file, add the following global scope declaration 
// (based on the specified files leafname)
//
// FW_DECLARE_EMBEDDED_DATA(rockwall_dds)
//
// Note, each . and / character in the filename is replaced by an _ in the symbol name.
//
// Then reference your data via:
//
//	u8* pDataStart	= FW_EMBEDDED_DATA_START(rockwall_dds);
//	u8* pDataEnd	= FW_EMBEDDED_DATA_END(rockwall_dds);
//	size_t DataSize = FW_EMBEDDED_DATA_SIZE(rockwall_dds);

#ifdef _MSC_VER
#define FW_DECLARE_EMBEDDED_DATA(dataname)																			\
	extern "C" unsigned char	FW_JOIN_MACRO_ARGS3(_binary_,dataname,_start)[];									\
	extern "C" unsigned char	FW_JOIN_MACRO_ARGS3(_binary_,dataname,_end)[];										\
	extern "C" unsigned long	FW_JOIN_MACRO_ARGS3(_binary_,dataname,_size);
#define FW_EMBEDDED_DATA_START(dataname)	((u8*)FW_JOIN_MACRO_ARGS3(_binary_,dataname,_start))
#define FW_EMBEDDED_DATA_END(dataname)		((u8*)FW_JOIN_MACRO_ARGS3(_binary_,dataname,_end))
#define FW_EMBEDDED_DATA_SIZE(dataname)		((size_t)(FW_EMBEDDED_DATA_END(dataname)-FW_EMBEDDED_DATA_START(dataname)))
#else
#define FW_DECLARE_EMBEDDED_DATA(dataname)																			\
	extern "C" unsigned char	FW_JOIN_MACRO_ARGS3(_binary_,dataname,_start)[];									\
	extern "C" unsigned char	FW_JOIN_MACRO_ARGS3(_binary_,dataname,_end)[];										\
	extern "C" unsigned long	FW_JOIN_MACRO_ARGS3(_binary_,dataname,_size)[];
#define FW_EMBEDDED_DATA_START(dataname)	((u8*)FW_JOIN_MACRO_ARGS3(_binary_,dataname,_start))
#define FW_EMBEDDED_DATA_END(dataname)		((u8*)FW_JOIN_MACRO_ARGS3(_binary_,dataname,_end))
#define FW_EMBEDDED_DATA_SIZE(dataname)		((size_t) &FW_JOIN_MACRO_ARGS3(_binary_,dataname,_size))
#endif



// Common template functions
template<typename T> inline T min(T const& a, T const& b)					{ return (a < b) ? a : b; }
template<typename T> inline T max(T const& a, T const& b)					{ return (a > b) ? a : b; }
template<typename T> inline T clamp( T const& v, T const& mn, T const& mx )	{ return max( min( v, mx ), mn ); }
template<typename T> inline T sqr(T const& a)								{ return a * a; }

//--------------------------------------------------------------------------------------------------
//  EXTERNAL DECLARATIONS
//--------------------------------------------------------------------------------------------------

void	FwMemsetAligned(void* ptr, u8 value, size_t length);
void	FwMemcpyAligned(void* ptr, const void* src, size_t length);
void	FwMemcpyAlignedBackwards(void* ptr, const void* src, size_t length);
void	FwMemcpy(void* ptr, const void* src, size_t length);

bool	FwStrCpy( char* pDest, const char* pSource, size_t destSize );
bool	FwStrCat( char* pDest, const char* pSource, size_t destSize );

#ifdef	ATG_PC_PLATFORM
bool	FwStrFormat( char* pBuffer, size_t bufferSize, const char* pFormat, ... );
bool	FwStrFormatArgs( char* pBuffer, size_t bufferSize, const char* pFormat, va_list argList );
#else
bool	FwStrFormat( char* pBuffer, size_t bufferSize, const char* pFormat, ... ) __attribute__( ( format( printf, 3, 4 ) ) );
bool	FwStrFormatArgs( char* pBuffer, size_t bufferSize, const char* pFormat, va_list argList ) __attribute__( ( format( printf, 3, 0 ) ) );
#endif	// ATG PC_PLATFORM

#ifdef	ATG_PC_PLATFORM
inline	int	FwStrCmp( const char* pStr1, const char* pStr2 )					{ return strcmp( pStr1, pStr2 ); }
inline	int	FwStrCaseCmp( const char* pStr1, const char* pStr2 )				{ return _stricmp( pStr1, pStr2 ); }
inline	int	FwStrCmp( const char* pStr1, const char* pStr2, size_t count )		{ return strncmp( pStr1, pStr2, count ); }
inline	int FwStrCaseCmp( const char* pStr1, const char* pStr2, size_t count )	{ return _strnicmp( pStr1, pStr2, count ); }
#else
inline	int	FwStrCmp( const char* pStr1, const char* pStr2 )					{ return std::strcmp( pStr1, pStr2 ); }
inline	int	FwStrCaseCmp( const char* pStr1, const char* pStr2 )				{ return std::strcasecmp( pStr1, pStr2 ); }
inline	int	FwStrCmp( const char* pStr1, const char* pStr2, size_t count )		{ return std::strncmp( pStr1, pStr2, count ); }
inline	int FwStrCaseCmp( const char* pStr1, const char* pStr2, size_t count )	{ return std::strncasecmp( pStr1, pStr2, count ); }
#endif	// ATG_PC_PLATFORM

//--------------------------------------------------------------------------------------------------
/**
	@class			FwNonCopyable
	
	@brief			Assists in the creation of class that should not be copyable.

	Most of the classes within the API should not be copyable. Rather than making these classes
	define copy constructors and assignment operators as private, classes should derive from 
	FwNonCopyable, making the non-copyable behaviour much more apparent.

	@note			This has no runtime or memory overhead whatsoever.
**/
//--------------------------------------------------------------------------------------------------

class FwNonCopyable
{
public:
	//! Public constructor.
	FwNonCopyable() {}

private:
	//! Private copy-constructor declaration.
	FwNonCopyable( FwNonCopyable const& rOther);

	//! Private assignment operator declaration.
	FwNonCopyable& operator = ( FwNonCopyable const& rOther);
};


//--------------------------------------------------------------------------------------------------
/**
	@brief		Helpers to allow us to determine whether a template parameter is a pointer at
				compile time.

	In some templated classes/functions, it may be necessary to tell whether a templated parameter
	is a pointer or not. For example, you might want to prohibit template instantiation for
	non-pointer types through the use of static asserts.

	The following example shows how this can be achieved. If MySampleClass is instantiated with a
	non-pointer template parameter, compilation will fail.

	@code
		template<typename T>
		class	MySampleClass
		{
			FW_STATIC_ASSERT( FwIsPointer<T>::value );
			...
		};
	@endcode
**/
//--------------------------------------------------------------------------------------------------

template<typename T> struct FwIsPointer						{ enum { value = false }; };
template<typename T> struct FwIsPointer<T*>					{ enum { value = true }; };
template<typename T> struct FwIsPointer<T* const>			{ enum { value = true }; };
template<typename T> struct FwIsPointer<T* volatile>		{ enum { value = true }; };
template<typename T> struct FwIsPointer<T* const volatile>	{ enum { value = true }; };


//--------------------------------------------------------------------------------------------------
/**
	@class			FwOffset
	
	@brief			Templated helper class to convert from 32-bit offset to a specific pointer type.

	@note			Some offsets may need to deliberately return a NULL pointer, as the data it 
					refers to may be optional. Our offset is stored relative to the address of
					the FwOffset objects, so we can use zero to indicate that there is no
					data referred to by the offset.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
class	FwOffset
{
public:
	// Accessors
	inline	T*	Get( void ) const;

private:
	s32	m_offset;
};


//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns a resolved pointer whose type is the template parameter.

	@result		The resolved pointer.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
inline	T*	FwOffset<T>::Get( void ) const
{
	if ( m_offset )
		return	( T* )( ( const char* )( &m_offset ) + ( intptr_t )( m_offset ) );	
	else
		return	NULL;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether the specified parameter is a power of 2

	@param			value		-	Value we want to check.

	@return			'true' if it is a power of 2, else 'false'.
**/
//--------------------------------------------------------------------------------------------------

inline bool FwIsPow2(u32 value)
{
	return (((value & (value - 1)) == 0) && value);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether the specified parameter is a power of 2

	@param			value		-	Value we want to check.

	@return			'true' if it is a power of 2, else 'false'.
**/
//--------------------------------------------------------------------------------------------------

inline bool FwIsPow2(u64 value)
{
	return (((value & (value - 1)) == 0) && value);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Aligns an address to a specific alignment

	@param			pAddress			-	Pointer to some memory
	@param			alignment			-	Alignment required (must be power of 2)

	@return			Aligned address
**/
//--------------------------------------------------------------------------------------------------

inline void* FwAlign(const void* pAddress, size_t alignment)
{
	FW_ASSERT(FwIsPow2(alignment));
	
	return (void*)(((size_t(pAddress)) + (alignment - 1)) & ~(alignment - 1));
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Aligns a 32-bit unsigned value to a specific alignment

	@param			value				-	Value we wish to align
	@param			alignment			-	Alignment required (must be power of 2)

	@return			Aligned value
**/
//--------------------------------------------------------------------------------------------------

inline u32 FwAlign(u32 value, u32 alignment)
{
	FW_ASSERT(FwIsPow2(alignment));

	return (value + (alignment - 1)) & ~(alignment - 1);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Aligns a 64-bit unsigned value to a specific alignment

	@param			value				-	Value we wish to align
	@param			alignment			-	Alignment required (must be power of 2)

	@return			Aligned value
**/
//--------------------------------------------------------------------------------------------------

inline u64 FwAlign(u64 value, u64 alignment)
{
	FW_ASSERT(FwIsPow2(alignment));

	return (value + (alignment - 1)) & ~(alignment - 1);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether an address has the specified alignment

	@param			pAddress			-	Pointer to some memory
	@param			alignment			-	Alignment required (must be power of 2)

	@return			'true' if the address is aligned, else 'false'.
**/
//--------------------------------------------------------------------------------------------------

inline bool FwIsAligned(const void* pAddress, size_t alignment)
{
	FW_ASSERT(FwIsPow2(alignment));
	
	return !(size_t(pAddress) & (alignment - 1));
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether an unsigned 32-bit value has the specified alignment

	@param			value				-	Value we wish to align
	@param			alignment			-	Alignment required (must be power of 2)

	@return			'true' if the value is aligned, else 'false'.
**/
//--------------------------------------------------------------------------------------------------

inline bool FwIsAligned(u32 value, u32 alignment)
{
	FW_ASSERT(FwIsPow2(alignment));
	
	return !(value & (alignment - 1));
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns whether an unsigned 64-bit value has the specified alignment

	@param			value				-	Value we wish to align
	@param			alignment			-	Alignment required (must be power of 2)

	@return			'true' if the value is aligned, else 'false'.
**/
//--------------------------------------------------------------------------------------------------

inline bool FwIsAligned(u64 value, u64 alignment)
{
	FW_ASSERT(FwIsPow2(alignment));
	
	return !(value & (alignment - 1));
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Adds a byte offset to a pointer

	@param			ptr				-	Pointer to offset
	@param			offset			-	Offset required (in bytes)

	@return			the offset pointer value as a void*
**/
//--------------------------------------------------------------------------------------------------

inline void* FwAddPtr(void* ptr, ptrdiff_t offset)
{
	return ((u8*)ptr) + offset;
}

inline const void* FwAddPtr(const void* ptr, ptrdiff_t offset)
{
	return ((u8*)ptr) + offset;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Finds the byte difference between two pointers

	@param			lptr			-	left pointer
	@param			rptr			-	right pointer

	@return			the difference
**/
//--------------------------------------------------------------------------------------------------

inline ptrdiff_t FwSubPtr(const void* lptr, const void* rptr)
{
	return ((u8*)lptr) - ((u8*)rptr);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			converts a float to u32 preserving the bit pattern
					I thought reinterpret_cast<u32>(float) would do this ... but MSVC barfs

	@param			value			-	the value to convert

	@return			the IEEE bit pattern for the floating point number provided
**/
//--------------------------------------------------------------------------------------------------

inline u32 FwFloatBits(float value)
{
	union
	{
		float	m_float;
		u32		m_bits;
	} floatBits;

	floatBits.m_float = value;
	return floatBits.m_bits;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			converts a double to u64 preserving the bit pattern

	@param			value			-	the value to convert

	@return			the IEEE bit pattern for the floating point number provided
**/
//--------------------------------------------------------------------------------------------------

inline u64 FwDoubleBits(double value)
{
	union
	{
		double	m_double;
		u64		m_bits;
	} doubleBits;

	doubleBits.m_double = value;
	return doubleBits.m_bits;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			converts a u32 to a float preserving the bit pattern
					I thought reinterpret_cast<float>(u32) would do this ...

	@param			value			-	the IEEE bit pattern to create a float from

	@return			the floating point number represented by value
**/
//--------------------------------------------------------------------------------------------------

inline float FwFloatFromBits(u32 value)
{
	union
	{
		float	m_float;
		u32		m_bits;
	} floatBits;

	floatBits.m_bits = value;
	return floatBits.m_float;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			converts a u64 to a double preserving the bit pattern

	@param			value			-	the IEEE bit pattern to create a double from

	@return			the floating point number represented by value
**/
//--------------------------------------------------------------------------------------------------

inline double FwDoubleFromBits(u64 value)
{
	union
	{
		double	m_double;
		u64		m_bits;
	} doubleBits;

	doubleBits.m_bits = value;
	return doubleBits.m_double;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			create 2^n in float format

	@param			pow				-	power of 2 from -127 to 127

	@return			a float of 2^n

	@note			pow of -127 returns 0, pow of >127 returns +INF
**/
//--------------------------------------------------------------------------------------------------

inline float FwFloatPow2(s32 pow)
{
	if (pow <= -127)	return 0.0f;
	else if (pow > 127)	return FwFloatFromBits(0x7F800000);
	else				return FwFloatFromBits(u32((pow + 127) << 23));
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			count the number of leading zeroes on a number - this is equivalent to the index
					of the highest bit (where MSB = 0 and LSB = 31) or 32 if there is no highest bit

	@param			value			-	the number to scan

	@return			the number of leading zeroes in value
**/
//--------------------------------------------------------------------------------------------------

inline u32 FwCountLeadingZeroes(u32 value)
{
	if (!value)	return 32;

	u32	highest = 0;

	while (!(value & 0x80000000))
	{
		highest++;
		value <<= 1;
	}

	return highest;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			count the number of trailing zeroes on a number, which is its alignment
					0 returns 32

	@param			value			-	the number to scan

	@return			the number of trailing zeroes in value, which is its alignment
**/
//--------------------------------------------------------------------------------------------------

inline u8 FwGetAlignment(u32 value)
{
	if (!value)	return 32;

	u8	lowest = 0;

	while (!(value & 1))
	{
		lowest++;
		value >>= 1;
	}

	return lowest;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			make a u128 (in a big-endian order)

	@param			v1				-	high word
	@param			v2				-	2nd word
	@param			v3				-	3rd word
	@param			v4				-	low word

	@return			the u128 v1:v2:v3:v4
**/
//--------------------------------------------------------------------------------------------------

inline u128 FwMakeU128(u32 v1, u32 v2, u32 v3, u32 v4)
{
	union
	{
		u128	x;
		u32		v[4];
	} u;

#if FW_BIG_ENDIAN
	u.v[0] = v1;
	u.v[1] = v2;
	u.v[2] = v3;
	u.v[3] = v4;
#else
	u.v[3] = v1;
	u.v[2] = v2;
	u.v[1] = v3;
	u.v[0] = v4;
#endif

	return u.x;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			shift left that handles n = 32 properly (or n = 8, etc.)
					this will hopefully be a basic shift on the target, instead of this if crap

	@param			a				-	number to shift
	@param			shift			-	amount to shift

	@return			shifted value
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
T FwShl(T a, size_t shift)
{
	return (shift == 8 * sizeof(T)) ? 0 : (a << shift);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			shift right that handles n = 32 properly (or n = 8, etc.)
					this will hopefully be a basic shift on the target, instead of this if crap

	@param			a				-	number to shift
	@param			shift			-	amount to shift

	@return			shifted value
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
T FwShr(T a, size_t shift)
{
	return (shift == 8 * sizeof(T)) ? 0 : (a >> shift);
}

//--------------------------------------------------------------------------------------------------
/**
	@class			FwNearPtr

	@brief			32-bit "near pointer" stored as an offset, so it can be serialized assuming
					it points within the same contiguous block of memory.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
class FwNearPtr
{
public:
	operator T*() const			{ return GetPointer(); }
	T* operator->() const		{ return GetPointer(); }

	T* operator=(T* object)		{ SetPointer(object); return object; }
	
	s32	GetOffset( void ) const	{ return m_offset; }

private:
	void SetPointer(T* object)	{ ptrdiff_t p = ((u8*)object) - ((u8*)this); FW_ASSERT((p >= -0x80000000ULL) && (p <= 0x7FFFFFFFULL)); m_offset = s32(p); }
	T* GetPointer() const		{ return (T*)(((u8*)this) + ptrdiff_t(m_offset)); }

	s32	m_offset;
};

//--------------------------------------------------------------------------------------------------
/**
	@class			FwNearPtrC

	@brief			32-bit "near pointer-to-constant" stored as an offset, so it can be serialized
					assuming it points within the same contiguous block of memory.
**/
//--------------------------------------------------------------------------------------------------

template<typename T>
class FwNearPtrC
{
public:
	operator const T*() const			{ return GetPointer(); }
	const T* operator->() const			{ return GetPointer(); }

	const T* operator=(const T* object)	{ SetPointer(object); return object; }
	
	s32	GetOffset( void ) const			{ return m_offset; }

private:
	void SetPointer(const T* object)
	{
		ptrdiff_t p = ((u8*)object) - ((u8*)this);
#if FW_64_BIT_POINTERS
		FW_ASSERT((p >= -0x80000000LL) && (p <= 0x7FFFFFFFLL));
#endif
		m_offset = s32(p);
	}
	const T* GetPointer() const			{ return (const T*)(((u8*)this) + ptrdiff_t(m_offset)); }

	s32	m_offset;
};

//--------------------------------------------------------------------------------------------------
/**
	@brief			Extract the tag characters from a u32 tag created with FW_MAKE_TAG()
	
	Useful for printing the tag.

	@param			tag				-	Input tag.
	
	@param			pC0				-	Ptr to character 0 storage.
	@param			pC1				-	Ptr to character 1 storage.
	@param			pC2				-	Ptr to character 2 storage.
	@param			pC3				-	Ptr to character 3 storage.

	@note			The tag is always stored in big-endian format.
**/
//--------------------------------------------------------------------------------------------------

inline void FwExtractTag( u32 tag, char* pC0, char* pC1, char* pC2, char* pC3)
{
	FW_ASSERT(pC0 && pC1 && pC2 && pC3);

	*pC0 = char(tag >> 0);
	*pC1 = char(tag >> 8);
	*pC2 = char(tag >> 16);
	*pC3 = char(tag >> 24);
}
//--------------------------------------------------------------------------------------------------
/**
	@brief			Temporary wrapper for C99 restrict
**/
//--------------------------------------------------------------------------------------------------

#ifdef _MSC_VER
#	define FW_RESTRICT
#elif defined(__GNUC__)
#	define FW_RESTRICT __restrict__
#endif 

//--------------------------------------------------------------------------------------------------

#endif	// FW_HELPERS_H
