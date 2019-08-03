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

	const	unsigned int			kShufX				=	0x00010203;
	const	unsigned int			kShufY				=	0x04050607;
	const	unsigned int			kShufZ				=	0x08090a0b;
	const	unsigned int			kShufW				=	0x0c0d0e0f;
	const	unsigned int			kShufA				=	0x10111213;
	const	unsigned int			kShufB				=	0x14151617;
	const	unsigned int			kShufC				=	0x18191a1b;
	const	unsigned int			kShufD				=	0x1c1d1e1f;

	const	vector unsigned char	kShufWZBA			=	( vector unsigned char )( vector unsigned int ){ kShufW, kShufZ, kShufB, kShufA };
	const	vector unsigned char	kShufCWXB			=	( vector unsigned char )( vector unsigned int ){ kShufC, kShufW, kShufX, kShufB };
	const	vector unsigned char	kShufYAWC			=	( vector unsigned char )( vector unsigned int ){ kShufY, kShufA, kShufW, kShufC };

	const	vector unsigned char	kShufXZBX			=	( vector unsigned char )( vector unsigned int ){ kShufX, kShufZ, kShufB, kShufX };
	const	vector unsigned char	kShufCXXX			=	( vector unsigned char )( vector unsigned int ){ kShufC, kShufX, kShufX, kShufX };
	const	vector unsigned char	kShufYAXX			=	( vector unsigned char )( vector unsigned int ){ kShufY, kShufA, kShufX, kShufX };
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

#define	_VEC_GET_MASK_X()					( vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x0c ) )
#define	_VEC_GET_MASK_Y()					( vec_sld( ( vector float )( 0.0f ), vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x0c ), 0x0c ) )
#define	_VEC_GET_MASK_Z()					( vec_sld( ( vector float )( 0.0f ), vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x0c ), 0x08 ) )
#define _VEC_GET_MASK_W()					( vec_sld( ( vector float )( 0.0f ), ( vector float )vec_vspltisw( -1 ), 0x04 ) )
#define	_VEC_GET_MASK_ZW()					( vec_sld( ( vector float )( 0.0f ), ( vector float )vec_vspltisw( -1 ), 0x08 ) )

#define	_VEC_GET_MASKOFF_X()				( vec_sld( ( vector float )( 0.0f ), ( vector float )vec_vspltisw( -1 ), 0x0c ) )
#define	_VEC_GET_MASKOFF_Y()				( vec_sld( vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x04 ), ( vector float )vec_vspltisw( -1 ), 0x08 ) )
#define	_VEC_GET_MASKOFF_Z()				( vec_sld( vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x04 ), ( vector float )vec_vspltisw( -1 ), 0x04 ) )
#define	_VEC_GET_MASKOFF_W()				( vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x04 ) )
#define	_VEC_GET_MASKOFF_ZW()				( vec_sld( ( vector float )vec_vspltisw( -1 ), ( vector float )( 0.0f ), 0x08 ) )

#define	_VEC_GET_UNIT_X()					( vec_ctf( ( vec_sld( vec_vspltisw( 1 ), ( vector signed int )( 0 ), 0x0c ) ), 0 ) )
#define	_VEC_GET_UNIT_Y()					( vec_ctf( vec_sld( ( vector signed int )( 0 ), ( vec_sld( vec_vspltisw( 1 ), ( vector signed int )( 0 ), 0x0c ) ), 0x0c ), 0 ) )
#define	_VEC_GET_UNIT_Z()					( vec_ctf( vec_sld( ( vector signed int )( 0 ), ( vec_sld( vec_vspltisw( 1 ), ( vector signed int )( 0 ), 0x0c ) ), 0x08 ), 0 ) )
#define	_VEC_GET_UNIT_W()					( vec_ctf( ( vec_sld( ( vector signed int )( 0 ), vec_vspltisw( 1 ), 0x04 ) ), 0 ) )

#define	_VEC_GET_INVALID_VECTOR()			( ( vector float )vec_vspltisw( -1 ) )

#define	_VEC_GET_MASK_SIGN()				( vec_xor( ( vector float )vec_vspltisw( -1 ), ( vector float )vec_sr( ( vector unsigned int )vec_vspltisw( -1 ), vec_splat_u32( 1 ) ) ) )
#define	_VEC_GET_MASKOFF_SIGN()				( ( vector float )vec_sr( ( vector unsigned int )vec_vspltisw( -1 ), vec_splat_u32( 1 ) ) )
#define	_VEC_GET_MASK_CONJUGATE()			( vec_sld( vec_xor( ( vector float )vec_vspltisw( -1 ), ( vector float )vec_sr( ( vector unsigned int )vec_vspltisw( -1 ), vec_splat_u32( 1 ) ) ), ( vector float )( 0.0f ), 0x04 ) )

#define	_VEC_GET_SIGN_PNPN()				( vec_mergeh( ( vector float )( 0.0f ), _VEC_GET_MASK_SIGN() ) )
#define	_VEC_GET_SIGN_NPNP()				( vec_mergeh( _VEC_GET_MASK_SIGN(), ( vector float )( 0.0f ) ) )

#define	_VEC_GET_SHUF_YZX()					( ( vector unsigned char )vec_sld( vec_perm( vec_lvsl( 0, ( float* )0 ), vec_lvsl( 0, ( float* )0 ), vec_lvsr( 4, ( float* )0 ) ), vec_lvsl( 0, ( float* )0 ), 0x08 ) )
#define	_VEC_GET_SHUF_ZXY()					( ( vector unsigned char )vec_sld( vec_perm( vec_lvsl( 0, ( float* )0 ), vec_lvsl( 0, ( float* )0 ), vec_lvsr( 4, ( float* )0 ) ), vec_lvsl( 0, ( float* )0 ), 0x0c ) )

// -- Element replication

#define	_VEC_REPLICATE_FLOAT( f )			vec_splat( ( ( vector float ){ ( f ), 0.0f, 0.0f, 0.0f } ), 0 )
#define	_VEC_REPLICATE_X( vec )				vec_splat( ( vec ), 0 )
#define	_VEC_REPLICATE_Y( vec )				vec_splat( ( vec ), 1 )
#define	_VEC_REPLICATE_Z( vec )				vec_splat( ( vec ), 2 )
#define	_VEC_REPLICATE_W( vec )				vec_splat( ( vec ), 3 )
#define	_VEC_REPLICATE( vec, idx )			vec_splat( ( vec ), idx )

// -- Setting

#define	_VEC_SET_ZERO()						( vector float )( 0.0f )

#define	_VEC_SET_FLOATS( vec, x, y, z, w )																		\
{																												\
	FwMathsInternal::QuadwordElements	internalWork;															\
	internalWork.m_elements[0] = x; internalWork.m_elements[1] = y;												\
	internalWork.m_elements[2] = z; internalWork.m_elements[3] = w;												\
	vec = internalWork.m_quadword;																				\
}

#define	_VEC_SET_X( vec, val )																					\
{																												\
	vec = vec_sel( ( vec ), ( val ), ( vector unsigned int )_VEC_GET_MASK_X() );								\
}

#define	_VEC_SET_Y( vec, val )																					\
{																												\
	vec = vec_sel( ( vec ), ( val ), ( vector unsigned int )_VEC_GET_MASK_Y() );								\
}

#define	_VEC_SET_Z( vec, val )																					\
{																												\
	vec = vec_sel( ( vec ), ( val ), ( vector unsigned int )_VEC_GET_MASK_Z() );								\
}

#define	_VEC_SET_W( vec, val )																					\
{																												\
	vec = vec_sel( ( vec ), ( val ), ( vector unsigned int )_VEC_GET_MASK_W() );								\
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
		vec = vec_sel( ( vec ), ( value ), ( vector unsigned int )_VEC_ROR( _VEC_GET_MASK_X(), index ) );		\
	}																											\
}

#define	_VEC_GET_FLOAT_X( res, vec )																			\
{																												\
	FwMathsInternal::QuadwordElements internalResult;															\
	internalResult.m_quadword = ( vec );																		\
	res = internalResult.m_elements[ 0 ];																		\
}

// -- Basic operations

#define	_VEC_NEG( vec )						vec_sub( ( vector float )( 0.0f ), ( vec ) )
#define	_VEC_ABS( vec )						vec_abs( ( vec ) )
#define	_VEC_MIN( lhs, rhs )				vec_min( ( lhs ), ( rhs ) ) 		
#define	_VEC_MAX( lhs, rhs )				vec_max( ( lhs ), ( rhs ) ) 
#define	_VEC_CLAMP( vec, mn, mx )			vec_max( vec_min( ( vec ), ( mx ) ), ( mn ) )

#define	_VEC_IS_POSITIVE( vec )				vec_all_ge( ( vec ), ( vector float )( 0.0f ) )
#define	_VEC_IS_NEGATIVE( vec )				vec_all_lt( ( vec ), ( vector float )( 0.0f ) )

#define	_VEC_MADD( lhs, rhs, add )			vec_madd( ( lhs ), ( rhs ), ( add ) )
#define	_VEC_ADD( lhs, rhs )				vec_add( ( lhs ), ( rhs ) )
#define	_VEC_SUB( lhs, rhs )				vec_sub( ( lhs ), ( rhs ) )
#define	_VEC_MUL( lhs, rhs )				vec_madd( ( lhs ), ( rhs ), ( vector float )( 0.0f ) )
#define	_VEC_DIV( res, lhs, rhs )																									\
{																																	\
	vector float	internalEstimate	= vec_re( ( rhs ) );																		\
	vector float	internalResult;																									\
	internalResult	= vec_nmsub( internalEstimate, ( rhs ), ( vector float )( 1.0f ) );												\
	internalResult	= vec_madd( internalResult, internalEstimate, internalEstimate );												\
	internalResult	= vec_madd( internalResult, ( lhs ), ( vector float )( 0.0f ) );												\
	res = internalResult;																											\
}

#define	_VEC_AND( lhs, rhs )				vec_and( ( lhs ), ( rhs ) )
#define	_VEC_XOR( lhs, rhs )				vec_xor( ( lhs ), ( rhs ) )
#define	_VEC_ROR( value, count)				vec_perm( ( value ), ( value ), vec_lvsr( ( count * 4 ), ( unsigned int* ) 0 ) )
#define	_VEC_ROL( value, count)				vec_perm( ( value ), ( value ), vec_lvsl( ( count * 4 ), ( unsigned int* ) 0 ) )


// -- Comparisons

#define	_VEC_ALL_COMPONENTS_LT( lhs, rhs )	( vec_all_lt( ( lhs ), ( rhs ) ) )
#define	_VEC_ALL_COMPONENTS_LE( lhs, rhs )	( vec_all_le( ( lhs ), ( rhs ) ) )
#define	_VEC_ALL_COMPONENTS_GT( lhs, rhs )	( vec_all_gt( ( lhs ), ( rhs ) ) )
#define	_VEC_ALL_COMPONENTS_GE( lhs, rhs )	( vec_all_ge( ( lhs ), ( rhs ) ) )
#define	_VEC_ALL_COMPONENTS_EQ( lhs, rhs )	( vec_all_eq( ( lhs ), ( rhs ) ) )


// -- More complex macros, used to generally used to implement function bodies

#define	_VEC_RCP( res, vec )																										\
{																																	\
	vector float	internalEstimate	= vec_re( ( vec ) );																		\
	vector float	internalResult;																									\
	internalResult	= vec_nmsub( internalEstimate, ( vec ), ( vector float )( 1.0f ) );												\
	internalResult	= vec_madd( internalResult, internalEstimate, internalEstimate );												\
	res = internalResult;																											\
}

#define	_VEC_SQRT( res, vec )																										\
{																																	\
	vector float	internalEstimate	= vec_rsqrte( ( vec ) );																	\
	vector float	internalEstSquared	= _VEC_MUL( internalEstimate, internalEstimate );											\
	vector float	internalResult;																									\
	internalResult	= vec_nmsub( ( vec ), internalEstSquared, ( vector float )( 1.0f ) );											\
	internalResult	= vec_madd( internalResult, _VEC_MUL( internalEstimate, ( vector float )( 0.5f ) ), internalEstimate );			\
	internalResult	= _VEC_MUL( internalResult, ( vec ) );																			\
	internalResult	= vec_sel( ( vector float )( 0.0f ), internalResult, vec_cmpgt( ( vec ), ( vector float )( 0.0f ) ) );			\
	res = internalResult;																											\
}

#define	_VEC_RSQRT( res, vec )																										\
{																																	\
	vector float	internalEstimate	= vec_rsqrte( ( vec ) );																	\
	vector float	internalEstSquared	= _VEC_MUL( internalEstimate, internalEstimate );											\
	vector float	internalResult;																									\
	internalResult	= vec_nmsub( ( vec ), internalEstSquared, ( vector float )( 1.0f ) );											\
	internalResult	= vec_madd( internalResult, _VEC_MUL( internalEstimate, ( vector float )( 0.5f ) ), internalEstimate );			\
	res = internalResult;																											\
}

#define	_VEC_DOT3( res, lhs, rhs )																									\
{																																	\
	vector float	internalResult;																									\
	vector float	internalVector;																									\
	internalVector	= _VEC_MUL( ( lhs ), ( rhs ) );																					\
	internalResult	= vec_add( internalVector, vec_sld( internalVector, internalVector, 0x04 ) );									\
	internalResult	= vec_add( internalResult, vec_sld( internalVector, internalVector, 0x08 ) );									\
	res = internalResult;																											\
}

#define	_VEC_DOT4( res, lhs, rhs )																									\
{																																	\
	vector float	internalResult;																									\
	vector float	internalVector;																									\
	internalVector	= _VEC_MUL( ( lhs ), ( rhs ) );																					\
	internalResult	= vec_add( internalVector, vec_sld( internalVector, internalVector, 0x04 ) );									\
	internalResult	= vec_add( internalResult, vec_sld( internalVector, internalVector, 0x08 ) );									\
	internalResult	= vec_add( internalResult, vec_sld( internalVector, internalVector, 0x0c ) );									\
	res = internalResult;																											\
}

#define	_VEC_CROSS( res, lhs, rhs )																									\
{																																	\
	vector float			internalShuffled0;																						\
	vector float			internalShuffled1;																						\
	vector float			internalResult;																							\
	vector unsigned char	internalShufYZX = _VEC_GET_SHUF_YZX();																	\
	internalShuffled0	= vec_perm( ( lhs ), ( lhs ), internalShufYZX );															\
	internalShuffled1	= vec_perm( ( rhs ), ( rhs ), internalShufYZX );															\
	internalResult = _VEC_MUL( ( lhs ), internalShuffled1 );																		\
	internalResult = vec_nmsub( internalShuffled0, ( rhs ), internalResult );														\
	internalResult = vec_perm( internalResult, internalResult, internalShufYZX );													\
	res = internalResult;																											\
}

#define	_VEC_SIGN( res, vec )																										\
{																																	\
	vector float internalPosMask = ( vector float )vec_cmpge( ( vec ), ( vector float )( 0.0f ) );									\
	vector float internalNegMask = ( vector float )vec_cmplt( ( vec ), ( vector float )( 0.0f ) );									\
	internalPosMask = vec_and( internalPosMask, ( vector float )( 1.0f ) );															\
	internalNegMask = vec_and( internalNegMask, ( vector float )( -1.0f ) );														\
	res = vec_or( internalPosMask, internalNegMask );																				\
}

#define	_VEC_SIGNUM( res, vec )																										\
{																																	\
	vector float internalPosMask = ( vector float )vec_cmpgt( ( vec ), ( vector float )( 0.0f ) );									\
	vector float internalNegMask = ( vector float )vec_cmplt( ( vec ), ( vector float )( 0.0f ) );									\
	internalPosMask = vec_and( internalPosMask, ( vector float )( 1.0f ) );															\
	internalNegMask = vec_and( internalNegMask, ( vector float )( -1.0f ) );														\
	res = vec_or( internalPosMask, internalNegMask );																				\
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
	vector float	internalTolMask = ( vector float )vec_cmpgt( internalDotVec, ( vector float )( 1.0e-30f ) );							\
	res = _VEC_AND( _VEC_MUL( ( vec ), internalDotRSqrt ), internalTolMask );																\
	res	= vec_sel( ( def ), res, ( vector unsigned int )internalTolMask );																	\
}

#define	_VEC_SAFE_NORMALIZE4( res, vec, def )																								\
{																																			\
	vector float	internalDotVec;		_VEC_DOT4( internalDotVec, ( vec ), ( vec ) ); internalDotVec = _VEC_REPLICATE_X( internalDotVec );	\
	vector float	internalDotRSqrt;	_VEC_RSQRT( internalDotRSqrt, internalDotVec );														\
	vector float	internalTolMask = ( vector float )vec_cmpgt( internalDotVec, ( vector float )( 1.0e-30f ) );							\
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
