//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Internal types, macros and so on..

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_VEC_INTERNAL_H
#define FW_VEC_INTERNAL_H

#include <math.h>

#ifdef	__PPU__
#include <altivec.h>
#endif	//__PPU__

#ifdef	__SPU__
#include <vmx2spu.h>
#endif	//__SPU__

#ifndef	assert
#define	assert( x )	
#endif

//--------------------------------------------------------------------------------------------------
/**
	@brief			This typedef defines our base vector type, and is used in public interfaces. 
**/
//--------------------------------------------------------------------------------------------------

typedef	vector float	v128;

//--------------------------------------------------------------------------------------------------
/**
	@namespace		FwVecConsts
	
	@brief			This namespace contains various constants & masks that are used during vector
					operations.
**/
//--------------------------------------------------------------------------------------------------

namespace FwVecConsts
{
	// Shuffle/Permute Masks : For shuffles, words are labeled [x,y,z,w] [a,b,c,d]

	const	vector unsigned char	kShufWZBA			=	( vector unsigned char ){ 0x0c, 0x0d, 0x0e, 0x0f, 0x08, 0x09, 0x0a, 0x0b, 0x14, 0x15, 0x16, 0x17, 0x10, 0x11, 0x12, 0x13 };
	const	vector unsigned char	kShufCWXB			=	( vector unsigned char ){ 0x18, 0x19, 0x1a, 0x1b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x01, 0x02, 0x03, 0x14, 0x15, 0x16, 0x17 };
	const	vector unsigned char	kShufYAWC			=	( vector unsigned char ){ 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12, 0x13, 0x0c, 0x0d, 0x0e, 0x0f, 0x18, 0x19, 0x1a, 0x1b };
																																											  
	const	vector unsigned char	kShufXZBX			=	( vector unsigned char ){ 0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0a, 0x0b, 0x14, 0x15, 0x16, 0x17, 0x00, 0x01, 0x02, 0x03 };
	const	vector unsigned char	kShufCXXX			=	( vector unsigned char ){ 0x18, 0x19, 0x1a, 0x1b, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03 };
	const	vector unsigned char	kShufYAXX			=	( vector unsigned char ){ 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12, 0x13, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03 };

	const	vector unsigned char	kShufYZX			=	( vector unsigned char ){ 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03 };
	const	vector unsigned char	kShufZXY			=	( vector unsigned char ){ 0x08, 0x09, 0x0a, 0x0b, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03 };

	// Can we compute these?

	const	vector unsigned char	kMaskSign			=	( vector unsigned char ){ 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00 };
	const	vector unsigned char	kMaskOffSign		=	( vector unsigned char ){ 0x7f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff };

	const	vector unsigned char	kMaskConjugate		=	( vector unsigned char ){ 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };										  

	const	vector unsigned char	kMaskPNPN			=	( vector unsigned char ){ 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00 };
	const	vector unsigned char	kMaskNPNP			=	( vector unsigned char ){ 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// Vector Masks & Vector Constants

	const	vector float			kUnit1000			=	{ 1.0f, 0.0f, 0.0f, 0.0f };
	const	vector float			kUnit0100			=	{ 0.0f, 1.0f, 0.0f, 0.0f };
	const	vector float			kUnit0010			=	{ 0.0f, 0.0f, 1.0f, 0.0f };
	const	vector float			kUnit0001			=	{ 0.0f, 0.0f, 0.0f, 1.0f };
};

//--------------------------------------------------------------------------------------------------
/**
	@namespace		FwMaths
	
	This addition to the FwMaths namespace contains a number of enumerations that are used to allow
	assignment of vector based objects to known values in a highly readable manner. See construction 
	and assignment operators for vectors & matrices for more information on this.
**/
//--------------------------------------------------------------------------------------------------

namespace	FwMaths
{
	static	const	enum	ZeroTag				{	valZeroTag		}	kZero		= valZeroTag;
	static	const	enum	IdentityTag			{	valIdentityTag	}	kIdentity	= valIdentityTag;
	static	const	enum	UnitXAxisTag		{	valUnitXAxisTag	}	kUnitXAxis	= valUnitXAxisTag;
	static	const	enum	UnitYAxisTag		{	valUnitYAxisTag	}	kUnitYAxis	= valUnitYAxisTag;
	static	const	enum	UnitZAxisTag		{	valUnitZAxisTag	}	kUnitZAxis	= valUnitZAxisTag;
	static	const	enum	UnitWAxisTag		{	valUnitWAxisTag	}	kUnitWAxis	= valUnitWAxisTag;
	static	const	enum	QuatITag			{	valQuatITag		}	kQuatI		= valQuatITag;
	static	const	enum	QuatJTag			{	valQuatJTag		}	kQuatJ		= valQuatJTag;
	static	const	enum	QuatKTag			{	valQuatKTag		}	kQuatK		= valQuatKTag;
	static	const	enum	OriginTag			{	valOriginTag	}	kOrigin		= valOriginTag;
};

// Quadword validation 
#ifdef	ENABLE_VECTOR_QUADWORD_VALIDATION
#define	_VEC_INITIALISE_QUADWORD( a )		( a ) = _VEC_GET_INVALID_VECTOR()
#define	_VEC_VALIDATE_QUADWORD( a )			FwMathsInternal::ValidateQuadword( ( a ) )
#define	_VEC_VALIDATE_QUADWORD_XYZ( a )		FwMathsInternal::ValidateQuadwordXYZ( ( a ) )
#else
#define	_VEC_INITIALISE_QUADWORD( a )	
#define	_VEC_VALIDATE_QUADWORD( a )			
#define	_VEC_VALIDATE_QUADWORD_XYZ( a )		
#endif	// ENABLE_VECTOR_QUADWORD_VALIDATION


// -------------------------------------------------------------------------------------------------
// Helpers : For internal use only... 

#define	_VEC_GET_CONSTANT( name )			FwVecConsts::name

#define	_VEC_GET_MASK_X()					( ( vector float )spu_maskw(  8 /*%1000*/ ) )
#define	_VEC_GET_MASK_Y()					( ( vector float )spu_maskw(  4 /*%0100*/ ) )
#define	_VEC_GET_MASK_Z()					( ( vector float )spu_maskw(  2 /*%0010*/ ) )
#define _VEC_GET_MASK_W()					( ( vector float )spu_maskw(  1 /*%0001*/ ) )
#define	_VEC_GET_MASK_ZW()					( ( vector float )spu_maskw(  3 /*%0011*/ ) )

#define	_VEC_GET_MASKOFF_X()				( ( vector float )spu_maskw(  7 /*%0111*/ ) )
#define	_VEC_GET_MASKOFF_Y()				( ( vector float )spu_maskw( 11 /*%1011*/ ) )
#define	_VEC_GET_MASKOFF_Z()				( ( vector float )spu_maskw( 13 /*%1101*/ ) )
#define	_VEC_GET_MASKOFF_W()				( ( vector float )spu_maskw( 14 /*%1110*/ ) )
#define	_VEC_GET_MASKOFF_ZW()				( ( vector float )spu_maskw( 12 /*%1100*/ ) )

#define	_VEC_GET_UNIT_X()					( _VEC_GET_CONSTANT( kUnit1000 ) )
#define	_VEC_GET_UNIT_Y()					( _VEC_GET_CONSTANT( kUnit0100 ) )
#define	_VEC_GET_UNIT_Z()					( _VEC_GET_CONSTANT( kUnit0010 ) )
#define	_VEC_GET_UNIT_W()					( _VEC_GET_CONSTANT( kUnit0001 ) )

#define	_VEC_GET_INVALID_VECTOR()			( ( vector float )spu_maskw( 0x0f ) )

#define	_VEC_GET_MASK_SIGN()				( ( vector float )spu_splats( 0x80000000 ) )
#define	_VEC_GET_MASKOFF_SIGN()				( ( vector float )_VEC_GET_CONSTANT( kMaskOffSign ) )
#define	_VEC_GET_MASK_CONJUGATE()			( ( vector float )_VEC_GET_CONSTANT( kMaskConjugate ) )

#define	_VEC_GET_SIGN_PNPN()				( ( vector float )_VEC_GET_CONSTANT( kMaskPNPN ) )
#define	_VEC_GET_SIGN_NPNP()				( ( vector float )_VEC_GET_CONSTANT( kMaskNPNP ) )

#define	_VEC_GET_SHUF_YZX()					( _VEC_GET_CONSTANT( kShufYZX ) )
#define	_VEC_GET_SHUF_ZXY()					( _VEC_GET_CONSTANT( kShufZXY ) )

// -- Element replication

#define	_VEC_REPLICATE_FLOAT( f )			spu_splats( f )
#define	_VEC_REPLICATE_X( vec )				spu_splats( si_to_float ( (vector signed char ) vec ) )
#define	_VEC_REPLICATE_Y( vec )				vec_splat( ( vec ), 1 )
#define	_VEC_REPLICATE_Z( vec )				vec_splat( ( vec ), 2 )
#define	_VEC_REPLICATE_W( vec )				vec_splat( ( vec ), 3 )
#define	_VEC_REPLICATE( vec, idx )			vec_splat( ( vec ), ( idx ) )

// -- Setting

#define	_VEC_SET_ZERO()						spu_splats( 0.0f )
#define	_VEC_SET_FLOATS( vec, x, y, z, w )																		\
{																												\
	( vec ) = (vector float){ x, y, z, w };																		\
}

#define	_VEC_SET_X( vec, val )																					\
{																												\
	vec = spu_sel( ( vec ), ( val ), ( vector unsigned int )_VEC_GET_MASK_X() );								\
}

#define	_VEC_SET_Y( vec, val )																					\
{																												\
	vec = spu_sel( ( vec ), ( val ), ( vector unsigned int )_VEC_GET_MASK_Y() );								\
}

#define	_VEC_SET_Z( vec, val )																					\
{																												\
	vec = spu_sel( ( vec ), ( val ), ( vector unsigned int )_VEC_GET_MASK_Z() );								\
}

#define	_VEC_SET_W( vec, val )																					\
{																												\
	vec = spu_sel( ( vec ), ( val ), ( vector unsigned int )_VEC_GET_MASK_W() );								\
}

// -- Index based return/setting

#define	_VEC_RETURN_ELEMENT( vec, index )																		\
{																												\
	if ( __builtin_constant_p( ( index ) ) )																	\
	{																											\
		switch ( ( index ) )																					\
		{																										\
			case	0:	return FwScalar( _VEC_REPLICATE_X( ( vec ) ) );											\
			case	1:	return FwScalar( _VEC_REPLICATE_Y( ( vec ) ) );											\
			case	2:	return FwScalar( _VEC_REPLICATE_Z( ( vec ) ) );											\
			default:	return FwScalar( _VEC_REPLICATE_W( ( vec ) ) );											\
		};																										\
	}																											\
	else																										\
	{																											\
		return FwScalar( _VEC_REPLICATE_X( _VEC_ROL( m_value, index ) ) );										\
	}																											\
}

#define	_VEC_SET_ELEMENT( vec, index, value )																	\
{																												\
	if ( __builtin_constant_p( ( index ) ) )																	\
	{																											\
		switch ( ( index ) )																					\
		{																										\
			case	0:	_VEC_SET_X( ( vec ), ( value ) ); break;												\
			case	1:	_VEC_SET_Y( ( vec ), ( value ) ); break;												\
			case	2:	_VEC_SET_Z( ( vec ), ( value ) ); break;												\
			default:	_VEC_SET_W( ( vec ), ( value ) ); break;												\
		};																										\
	}																											\
	else																										\
	{																											\
		vec = spu_sel( ( vec ), ( value ), ( vector unsigned int )_VEC_ROR( _VEC_GET_MASK_X(), index ) );		\
	}																											\
}

#define	_VEC_GET_FLOAT_X( res, vec )																			\
{																												\
	res = si_to_float( ( qword ) ( vec ) );																		\
}

// -- Basic operations

#define	_VEC_NEG( vec )						( ( vector float ) spu_xor( ( vector unsigned int ) ( vec ), spu_splats( 0x80000000 ) ) )
#define	_VEC_ABS( vec )						( ( vector float ) spu_andc( ( vector unsigned int ) ( vec ), spu_splats( 0x80000000 ) ) )  
#define	_VEC_MIN( lhs, rhs )				spu_sel( ( lhs ), ( rhs ), spu_cmpgt( ( lhs ), ( rhs ) ) )
#define	_VEC_MAX( lhs, rhs )				spu_sel( ( rhs ), ( lhs ), spu_cmpgt( ( lhs ), ( rhs ) ) )
#define	_VEC_CLAMP( vec, mn, mx )			vec_max( vec_min( ( vec ), ( mx ) ), ( mn ) )

#define	_VEC_IS_POSITIVE( vec )				vec_all_ge( ( vec ), spu_splats( 0.0f ) )
#define	_VEC_IS_NEGATIVE( vec )				vec_all_lt( ( vec ), spu_splats( 0.0f ) )

#define	_VEC_MADD( lhs, rhs, add )			spu_madd( ( lhs ), ( rhs ), ( add ) )
#define	_VEC_ADD( lhs, rhs )				spu_add( ( lhs ), ( rhs ) )
#define	_VEC_SUB( lhs, rhs )				spu_sub( ( lhs ), ( rhs ) )
#define	_VEC_MUL( lhs, rhs )				spu_mul( ( lhs ), ( rhs ) )
#define	_VEC_DIV( res, lhs, rhs )																									\
{																																	\
	vector float	internalEstimate	= spu_re( ( rhs ) );																		\
	vector float	internalResult;																									\
	internalResult	= spu_nmsub( internalEstimate, ( rhs ), ( vector float )( 1.0f ) );												\
	internalResult	= spu_madd( internalResult, internalEstimate, internalEstimate );												\
	internalResult	= spu_madd( internalResult, ( lhs ), ( vector float )( 0.0f ) );												\
	res = internalResult;																											\
}

#define	_VEC_AND( lhs, rhs )				spu_and( ( lhs ), ( rhs ) )
#define	_VEC_XOR( lhs, rhs )				spu_xor( ( lhs ), ( rhs ) )
#define	_VEC_ROR( value, count)				spu_rlqwbyte( ( value ), -count * 4 )
#define	_VEC_ROL( value, count)				spu_rlqwbyte( ( value ),  count * 4 )


// -- Comparisons

#define	_VEC_ALL_COMPONENTS_LT( lhs, rhs )	( vec_all_lt( ( lhs ), ( rhs ) ) )
#define	_VEC_ALL_COMPONENTS_LE( lhs, rhs )	( vec_all_le( ( lhs ), ( rhs ) ) )
#define	_VEC_ALL_COMPONENTS_GT( lhs, rhs )	( vec_all_gt( ( lhs ), ( rhs ) ) )
#define	_VEC_ALL_COMPONENTS_GE( lhs, rhs )	( vec_all_ge( ( lhs ), ( rhs ) ) )
#define	_VEC_ALL_COMPONENTS_EQ( lhs, rhs )	( vec_all_eq( ( lhs ), ( rhs ) ) )


// -- More complex macros, used to generally used to implement function bodies

#define	_VEC_RCP( res, vec )																										\
{																																	\
	vector float	internalEstimate	= spu_re( ( vec ) );																		\
	vector float	internalResult;																									\
	internalResult	= spu_nmsub( internalEstimate, ( vec ), spu_splats( 1.0f ) );													\
	internalResult	= spu_madd( internalResult, internalEstimate, internalEstimate );												\
	res = internalResult;																											\
}

#define	_VEC_SQRT( res, vec )																										\
{																																	\
	vector float	internalEstimate	= spu_rsqrte( ( vec ) );																	\
	vector float	internalEstSquared	= _VEC_MUL( internalEstimate, internalEstimate );											\
	vector float	internalResult;																									\
	internalResult	= spu_nmsub( ( vec ), internalEstSquared, spu_splats( 1.0f ) );													\
	internalResult	= spu_madd( internalResult, _VEC_MUL( internalEstimate, spu_splats( 0.5f ) ), internalEstimate );				\
	internalResult	= spu_mul( internalResult, ( vec ) );																			\
	internalResult	= spu_sel( spu_splats( 0.0f ), internalResult, spu_cmpgt( ( vec ), spu_splats( 0.0f ) ) );						\
	res = internalResult;																											\
}

#define	_VEC_RSQRT( res, vec )																										\
{																																	\
	vector float	internalEstimate	= spu_rsqrte( ( vec ) );																	\
	vector float	internalEstSquared	= _VEC_MUL( internalEstimate, internalEstimate );											\
	vector float	internalResult;																									\
	internalResult	= spu_nmsub( ( vec ), internalEstSquared, spu_splats( 1.0f ) );													\
	internalResult	= spu_madd( internalResult, _VEC_MUL( internalEstimate, spu_splats( 0.5f ) ), internalEstimate );				\
	res = internalResult;																											\
}

#define	_VEC_DOT3( res, lhs, rhs )																									\
{																																	\
	vector float	internalResult;																									\
	vector float	internalVector;																									\
	internalVector	= spu_mul( ( lhs ), ( rhs ) );																					\
	internalResult	= spu_add( internalVector, spu_rlqwbyte( internalVector, 0x04 ) );												\
	internalResult	= spu_add( internalResult, spu_rlqwbyte( internalVector, 0x08 ) );												\
	res = internalResult;																											\
}

#define	_VEC_DOT4( res, lhs, rhs )																									\
{																																	\
	vector float	internalResult;																									\
	vector float	internalVector;																									\
	internalVector	= spu_mul( ( lhs ), ( rhs ) );																					\
	internalResult	= spu_add( internalVector, spu_rlqwbyte( internalVector, 0x04 ) );												\
	internalResult	= spu_add( internalResult, spu_rlqwbyte( internalVector, 0x08 ) );												\
	internalResult	= spu_add( internalResult, spu_rlqwbyte( internalVector, 0x0c ) );												\
	res = internalResult;																											\
}
  
#define	_VEC_CROSS( res, lhs, rhs )																									\
{																																	\
	vector float			internalShuffled0;																						\
	vector float			internalShuffled1;																						\
	vector float			internalResult0;																						\
	vector float			internalResult1;																						\
	vector float			internalResult2;																						\
	vector unsigned char	internalShufYZX = _VEC_GET_SHUF_YZX();																	\
	internalShuffled0	= spu_shuffle( ( lhs ), ( lhs ), internalShufYZX );															\
	internalShuffled1	= spu_shuffle( ( rhs ), ( rhs ), internalShufYZX );															\
	internalResult0 = spu_mul( ( lhs ), internalShuffled1 );																		\
	internalResult1 = spu_nmsub( internalShuffled0, ( rhs ), internalResult0 );														\
	internalResult2 = spu_shuffle( internalResult1, internalResult1, internalShufYZX );												\
	res = internalResult2;																											\
}

#define	_VEC_SIGN( res, vec )																										\
{																																	\
	vector unsigned int internalNegMask = spu_cmpgt( spu_splats( 0.0f ), ( vec ) );													\
	res = spu_sel( spu_splats( 1.0f ), spu_splats( -1.0f ), internalNegMask );														\
}

#define	_VEC_SIGNUM( res, vec )																										\
{																																	\
	vector unsigned int internalNegMask = spu_cmpgt( spu_splats( 0.0f ), ( vec ) );													\
	vector unsigned int internalPosMask = spu_cmpgt( ( vec ), spu_splats( 0.0f ) );													\
	res = spu_sel( spu_sel( spu_splats( 0.0f ), spu_splats( -1.0f ), internalNegMask ), spu_splats( 1.0f ), internalPosMask);		\
}

#define	_VEC_NORMALIZE3( res, vec )																											\
{																																			\
	vector float	internalDotVec;		_VEC_DOT3( internalDotVec, ( vec ), ( vec ) ); internalDotVec = _VEC_REPLICATE_X( internalDotVec );	\
	vector float	internalDotRSqrt;	_VEC_RSQRT( internalDotRSqrt, internalDotVec );														\
	res = _VEC_MUL( ( vec ), internalDotRSqrt );																							\
}

#define	_VEC_NORMALIZE4( res, vec )																											\
{																																			\
	vector float	internalDotVec;		_VEC_DOT4( internalDotVec, ( vec ), ( vec ) ); internalDotVec = _VEC_REPLICATE_X( internalDotVec );	\
	vector float	internalDotRSqrt;	_VEC_RSQRT( internalDotRSqrt, internalDotVec );														\
	res = _VEC_MUL( ( vec ), internalDotRSqrt );																							\
}

#define	_VEC_SAFE_NORMALIZE3( res, vec, def )																								\
{																																			\
	vector float	internalDotVec;		_VEC_DOT3( internalDotVec, ( vec ), ( vec ) ); internalDotVec = _VEC_REPLICATE_X( internalDotVec );	\
	vector float	internalDotRSqrt;	_VEC_RSQRT( internalDotRSqrt, internalDotVec );														\
	vector float	internalTolMask = ( vector float )spu_cmpgt( internalDotVec, spu_splats( 1.0e-30f ) );									\
	res = _VEC_AND( _VEC_MUL( ( vec ), internalDotRSqrt ), internalTolMask );																\
	res	= vec_sel( ( def ), res, ( vector unsigned int )internalTolMask );																	\
}

#define	_VEC_SAFE_NORMALIZE4( res, vec, def )																								\
{																																			\
	vector float	internalDotVec;		_VEC_DOT4( internalDotVec, ( vec ), ( vec ) ); internalDotVec = _VEC_REPLICATE_X( internalDotVec );	\
	vector float	internalDotRSqrt;	_VEC_RSQRT( internalDotRSqrt, internalDotVec );														\
	vector float	internalTolMask = ( vector float )spu_cmpgt( internalDotVec, spu_splats( 1.0e-30f ) );									\
	res = _VEC_AND( _VEC_MUL( ( vec ), internalDotRSqrt ), internalTolMask );																\
	res	= vec_sel( ( def ), res, ( vector unsigned int )internalTolMask );																	\
}

// -- Internal structures & inlined functions

namespace	FwMathsInternal
{
	union	QuadwordElements
	{
		float	m_elements[ 4 ];
		v128	m_quadword;
	};

#ifdef	ENABLE_VECTOR_QUADWORD_VALIDATION
	inline void	ValidateQuadword( v128 quadword )
	{
		if ( !vec_all_numeric( quadword ) )
			assert( false );
	}

	inline void	ValidateQuadwordXYZ( v128 quadword )
	{
		v128 workVec = quadword;
		_VEC_SET_W( workVec, _VEC_REPLICATE_Z( quadword ) );
		if ( !vec_all_numeric( workVec ) )
			assert( false );
	}
#endif	// ENABLE_VECTOR_QUADWORD_VALIDATION
};

#endif	// FW_VEC_INTERNAL_H
