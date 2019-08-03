
//--------------------------------------------------
//!
//!	\file water_utils.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _WATER_UTILS_H_
#define _WATER_UTILS_H_



#include <basetypes_spu.h>
#include <debug_spu.h>
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/vecmath_spu_soa_ps3.h"


typedef vector signed int vi128;
typedef vector unsigned int vu128;


const v128 g_v128_allone	= (v128){ 1.0f, 1.0f, 1.0f, 1.0f };
const v128 g_v128_allzero	= (v128){ 0.0f, 0.0f, 0.0f, 0.0f };
const v128 g_v128_epsilon	= (v128){ EPSILON, EPSILON, EPSILON, EPSILON };

const vu128 g_vu128_all_one = { 1, 1, 1, 1 };



// TODO_OZZ: there must be a smarter way of doing this
inline v128 SPU_Pow( v128 v, float k )
{
	v128 result = { 0,0,0,0 };

	result = spu_insert( pow( spu_extract(v,0), k ), result, 0 );
	result = spu_insert( pow( spu_extract(v,1), k ), result, 1 );
	result = spu_insert( pow( spu_extract(v,2), k ), result, 2 );
	result = spu_insert( pow( spu_extract(v,3), k ), result, 3 );

	return result;
}


inline v128 SPU_Clamp( v128 value, v128 min, v128 max )
{
	using namespace Intrinsics;
	return SPU_Max( SPU_Min( value, max ), min );
}


inline v128 SPU_Lerp( v128 a, v128 b, v128 t )
{
	t = SPU_Clamp( t, g_v128_allzero, g_v128_allone );
	return spu_madd( spu_sub( b, a ), t, a );
}


inline float MaxElem( v128 v )
{
	v128 result = Intrinsics::SPU_Max( spu_promote( spu_extract( v, 1 ), 0 ), v );
	result		= Intrinsics::SPU_Max( spu_promote( spu_extract( v, 2 ), 0 ), result );
	result		= Intrinsics::SPU_Max( spu_promote( spu_extract( v, 3 ), 0 ), result );
	return spu_extract( result, 0 );
}


inline float MinElem( v128 v )
{
	v128 result = Intrinsics::SPU_Min( spu_promote( spu_extract( v, 1 ), 0 ), v );
	result		= Intrinsics::SPU_Min( spu_promote( spu_extract( v, 2 ), 0 ), result );
	result		= Intrinsics::SPU_Min( spu_promote( spu_extract( v, 3 ), 0 ), result );
	return spu_extract( result, 0 );
}


inline bool ElemCmpAllGreaterThan( v128 vec, float value )
{
	vector unsigned int compResult = spu_cmpgt( vec, spu_splats(value) );
    vector unsigned int resultVec = spu_gather(compResult);
	return ( spu_extract( resultVec, 0 ) == 15 );
}


inline bool ElemCmpAnyGreaterThan( v128 vec, float value )
{
	vector unsigned int compResult = spu_cmpgt( vec, spu_splats(value) );
    vector unsigned int resultVec = spu_gather(compResult);
	return ( spu_extract( resultVec, 0 ) > 0 );
}

inline bool ElemCmpAnyAbsoluteGreaterThan( v128 vec, float value )
{
	vector unsigned int compResult = spu_cmpabsgt( vec, spu_splats(value) );
    vector unsigned int resultVec = spu_gather(compResult);
	return ( spu_extract( resultVec, 0 ) > 0 );
}


// horizontal dot product
// ignores Y component
inline static v128 HDot( CPoint_SOA const& lhs, CPoint_SOA const& rhs )
{
    v128 result;
	result = spu_mul( lhs.X(), rhs.X() );
	result = spu_madd( lhs.Z(), rhs.Z(), result );
    return result;
}





#endif // end of _WATER_UTILS_H_
