//--------------------------------------------------------------------------------------------------
/**
	@file		GcColour.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_COLOUR_H
#define GC_COLOUR_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fw/FwMaths/FwVector.h>

//--------------------------------------------------------------------------------------------------
//  SWITCHES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

class GcColour;

#ifdef	_MSC_VER
typedef	const GcColour&	GcColour_arg;
#else
typedef	const GcColour	GcColour_arg;
#endif	//_MSC_VER

namespace Gc
{
	enum COLOUR_BLACK_TAG		{ kColourBlack	 };
	enum COLOUR_BLUE_TAG		{ kColourBlue	 };
	enum COLOUR_GREEN_TAG		{ kColourGreen	 };
	enum COLOUR_CYAN_TAG		{ kColourCyan	 };
	enum COLOUR_RED_TAG			{ kColourRed	 };
	enum COLOUR_MAGENTA_TAG		{ kColourMagenta };
	enum COLOUR_YELLOW_TAG		{ kColourYellow	 };
	enum COLOUR_WHITE_TAG		{ kColourWhite	 };
	enum COLOUR_ORANGE_TAG		{ kColourOrange	 };
};

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class		GcColour

	@brief		handy debug colours
**/
//--------------------------------------------------------------------------------------------------

class GcColour : public FwVector4
{
public:
	inline	GcColour() {}
	inline	GcColour( float r, float g, float b, float a=1.0f ) : FwVector4( r, g, b, a ) {}
	inline	GcColour( FwVector4_arg v ) : FwVector4( v ) {}
	inline	GcColour( Gc::COLOUR_BLACK_TAG	 );
	inline	GcColour( Gc::COLOUR_BLUE_TAG	 );
	inline	GcColour( Gc::COLOUR_GREEN_TAG	 );
	inline	GcColour( Gc::COLOUR_CYAN_TAG	 );
	inline	GcColour( Gc::COLOUR_RED_TAG	 );
	inline	GcColour( Gc::COLOUR_MAGENTA_TAG );
	inline	GcColour( Gc::COLOUR_YELLOW_TAG	 );
	inline	GcColour( Gc::COLOUR_WHITE_TAG	 );
	inline	GcColour( Gc::COLOUR_ORANGE_TAG	 );

	u32 GetPackedColour() const;
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

GcColour::GcColour( Gc::COLOUR_BLACK_TAG   ) : FwVector4( 0.0f, 0.0f, 0.0f, 1.0f ) {}
GcColour::GcColour( Gc::COLOUR_BLUE_TAG	   ) : FwVector4( 0.0f, 0.0f, 1.0f, 1.0f ) {}
GcColour::GcColour( Gc::COLOUR_GREEN_TAG   ) : FwVector4( 0.0f, 1.0f, 0.0f, 1.0f ) {}
GcColour::GcColour( Gc::COLOUR_CYAN_TAG	   ) : FwVector4( 0.0f, 1.0f, 1.0f, 1.0f ) {}
GcColour::GcColour( Gc::COLOUR_RED_TAG	   ) : FwVector4( 1.0f, 0.0f, 0.0f, 1.0f ) {}
GcColour::GcColour( Gc::COLOUR_MAGENTA_TAG ) : FwVector4( 1.0f, 0.0f, 1.0f, 1.0f ) {}
GcColour::GcColour( Gc::COLOUR_YELLOW_TAG  ) : FwVector4( 1.0f, 1.0f, 0.0f, 1.0f ) {}
GcColour::GcColour( Gc::COLOUR_WHITE_TAG   ) : FwVector4( 1.0f, 1.0f, 1.0f, 1.0f ) {}
GcColour::GcColour( Gc::COLOUR_ORANGE_TAG  ) : FwVector4( 1.0f, 0.5f, 0.0f, 1.0f ) {}

inline u32 GcColour::GetPackedColour() const
{
	// clamp to between 0 and 255
	FwScalar maxColour( 255.0f );
	FwVector4 work = Clamp( FwVector4( *this )*maxColour, FwMaths::kZero, FwVector4( maxColour ) );

	// pack into 8-bit

#if defined(ATG_PC_PLATFORM)

	u32 packed = ((u32)work.W() << 24)
		| ( (u32)work.Z() << 16 )
		| ( (u32)work.Y() << 8 )
		| ( (u32)work.X() << 0 );

#elif defined(ATG_PS3_PLATFORM)

		u32 packed = (u32)work.W()
		| ( (u32)work.Z() << 8 )
		| ( (u32)work.Y() << 16 )
		| ( (u32)work.X() << 24 );
#else

#error Unknown platform!

#endif

	return packed;
}

#endif // GC_COLOUR_H
