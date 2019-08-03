//----------------------------------------------------------------------------------------------------//
//!
//!	\file waves.h
//!	This is a dummy holding a few doxygen comments.
//!
//----------------------------------------------------------------------------------------------------//

#ifndef _WAVES_H_
#define _WAVES_H_

#ifndef __SPU__
#	error this file is for spu projects only 
#endif

#include "spu/water/water_utils.h"
#include "spu/water/water_spu_config.h"

#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/vecmath_spu_soa_ps3.h"



// Pre-conditions:
// assumes position.X = pos0.X, position.Z = pos0.Z on initialisation 
// binormal.X = 1, tangent.Z = 1
inline void Gerstner_SOA( const CPoint_SOA& pos, const CPoint_SOA& dir, 
						 float frequency, float amplitude, float phase, float k, float time, 
						 v128 attenuation, 
						  CPoint_SOA& out_position, CPoint_SOA& out_binormal, CPoint_SOA& out_tangent )
{
	using namespace Intrinsics;

	const v128 Ak = spu_splats( amplitude * k );
	const v128 Aw = spu_splats( amplitude * frequency );
	const v128 Awk = spu_splats( amplitude * frequency * k );

	const v128 XX = spu_mul( dir.X(), dir.X() );
	const v128 ZZ = spu_mul( dir.Z(), dir.Z() );
	const v128 XZ = spu_mul( dir.X(), dir.Z() );
	
	v128 xxxx, yyyy, zzzz;
	v128 sines, cosines;
	
	// get the inner horizontal dot product sin/cos values
	//Intrinsics::SPU_SinCos( spu_madd(HDot(dir,pos), spu_splats(frequency), spu_splats(phase*time) ), &sines, &cosines );
	Intrinsics::SPU_SinCos( spu_msub(HDot(dir,pos), spu_splats(frequency), spu_splats(phase*time) ), &sines, &cosines );

	//
	// Position
	//
	xxxx = SPU_Negate( spu_mul( spu_mul( Ak, dir.X() ), sines ) );
	yyyy = spu_mul( spu_splats( amplitude ), cosines );
	zzzz = SPU_Negate( spu_mul( spu_mul( Ak, dir.Z() ), sines ) );

	yyyy = spu_add( yyyy, spu_splats( amplitude * 0.5f ) );

	out_position.Y() = spu_madd( yyyy, attenuation, out_position.Y() );
#ifdef WATER_AFFECT_SURFACE_HORIZONTAL_MOVEMENT
	out_position.X() = spu_madd( xxxx, attenuation, out_position.X() );
	out_position.Z() = spu_madd( zzzz, attenuation, out_position.Z() );
#endif

	//
	// Binormal
	//
	xxxx = SPU_Negate( spu_mul( spu_mul( Awk, XX ), cosines ) );
	yyyy = SPU_Negate( spu_mul( spu_mul( Aw, dir.X() ), sines ) );
	zzzz = SPU_Negate( spu_mul( spu_mul( Awk, XZ ), cosines ) );

	out_binormal.X() = spu_madd( xxxx, attenuation, out_binormal.X() );
	out_binormal.Y() = spu_madd( yyyy, attenuation, out_binormal.Y() );
	out_binormal.Z() = spu_madd( zzzz, attenuation, out_binormal.Z() );

	//
	// Tangent
	//
	xxxx = zzzz;
	yyyy = SPU_Negate( spu_mul( spu_mul( Aw, dir.Z() ), sines ) );
	zzzz = SPU_Negate( spu_mul( spu_mul( Awk, ZZ ), cosines ) );

	out_tangent.X() = spu_madd( xxxx, attenuation, out_tangent.X() );
	out_tangent.Y() = spu_madd( yyyy, attenuation, out_tangent.Y() );
	out_tangent.Z() = spu_madd( zzzz, attenuation, out_tangent.Z() );
}




///////////////////////////////////////////////////////////////////////////////////////////////////
//
//							OLD plain waves
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//inline v128 Wi_SOA( const CPoint_SOA& pos, const CPoint_SOA& dir, float frequency, float amplitude, float phase, float k, float time )
//{
//	v128 result;
//	v128 one_half = spu_splats( 0.5f );
//
//	v128 sines = Intrinsics::SPU_Sin( spu_madd(HDot(dir,pos), spu_splats(frequency), spu_splats(phase*time) ) );
//	result = spu_madd( sines, one_half, one_half );
//	result = SPU_Pow( result, k );
//	result = spu_mul( result, spu_splats(1.0f*amplitude ) );
//
//    return  result;   	
//}
//
//inline void DxDzWi_SOA( const CPoint_SOA& pos, const CPoint_SOA& dir, float frequency, float amplitude, float phase, float k, float time, v128& dxwi, v128& dzwi )
//{
//	v128 one_half	= spu_splats( 0.5f );
//	v128 constants	= spu_splats( 0.5f * amplitude * frequency * k);
//
//	v128 sines, cosines;
//	Intrinsics::SPU_SinCos( spu_madd(HDot(dir,pos), spu_splats(frequency), spu_splats(phase*time) ), &sines, &cosines );
//	v128 power = SPU_Pow( spu_madd( sines, one_half, one_half ), k-1 );
//	v128 partial_result = spu_mul( constants, spu_mul( power, cosines ) );
//
//	dxwi = spu_mul( partial_result, dir.X() );
//	dzwi = spu_mul( partial_result, dir.Z() );
//}




#endif // end of _WAVES_H_


