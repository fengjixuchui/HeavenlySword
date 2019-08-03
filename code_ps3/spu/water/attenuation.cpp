//--------------------------------------------------
//!
//!	\file stam_solver.cpp
//!	Jos Stam's stable fluids solver. 
//!
//--------------------------------------------------
#include "attenuation.h"
#include "water_utils.h"

#include <basetypes_spu.h>
#include <debug_spu.h>
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/vecmath_spu_soa_ps3.h"



using namespace Intrinsics;


static const v128 one_half = { 0.5f, 0.5f, 0.5f, 0.5f };
static const vu128 mask_all = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
static const vu128 mask_none = { 0, 0, 0, 0 };



v128 DistanceAttenuation( const CPoint_SOA& relative_position, float att_linear, float att_quadratic )
{
	v128 h_distance_squared = relative_position.LengthSqr();
	v128 h_distance = Intrinsics::SPU_Sqrt( h_distance_squared );

	// get  distance attenuation coef
	v128 att_factor = spu_madd( spu_splats(att_linear), h_distance, g_v128_allone );
	att_factor = spu_madd( spu_splats(att_quadratic), h_distance_squared, att_factor );

	// return inverse
	return recipf4( att_factor );
}



v128 AgeAttenuation( float age, float lifespan, float fade_in, float fade_out )
{
	float in	= ntstd::Clamp( age / ( fade_in+EPSILON), 0.0f, 1.0f );
	float out	= ntstd::Clamp( ( lifespan-age ) / (fade_out+EPSILON), 0.0f, 1.0f );
	v128 result = spu_splats( in * out );
	return result;
}



v128 WidthAttenuation( const CPoint_SOA& relative_position, const CPoint_SOA& direction, float width, float falloff )
{
	v128 hdistance_squared = relative_position.LengthSqr();

	// perpendicular horizontal distance
	v128 hdot = HDot( direction, relative_position );
	v128 hdot_squared = spu_mul( hdot, hdot );
	v128 pdistance_squared = spu_sub( hdistance_squared, hdot_squared );
	v128 pdistance = SPU_Sqrt( pdistance_squared );

	v128 interp_factor = SPU_Div( spu_sub( pdistance, spu_splats(width * 0.5f) ),  spu_splats(1.0f+falloff) );
	v128 result = SPU_Lerp( g_v128_allone, g_v128_allzero, interp_factor );
	
	return result;
}


v128 TrailAttenuation( const CPoint_SOA& relative_position, const CPoint_SOA& direction, float back_trail, float front_trail )
{
	v128 distance =  relative_position.Length();
	v128 hdot = HDot( direction, relative_position );

	vu128 frontfacing_mask = spu_cmpgt( g_v128_allzero, hdot  );
	vu128 backfacing_mask = spu_xor( frontfacing_mask, mask_all );

	v128 frontfacing_att = spu_convtf( spu_and( frontfacing_mask, g_vu128_all_one ), 0 );
	

	v128 back_interp_factor = SPU_Div( distance, spu_splats(back_trail) );
	v128 back_att = SPU_Lerp( g_v128_allone, g_v128_allzero, back_interp_factor );

	v128 front_interp_factor = SPU_Div( distance, spu_splats(front_trail) );
	v128 front_att = SPU_Lerp( g_v128_allone, g_v128_allzero, front_interp_factor );

	v128 result = SPU_Max( frontfacing_att, back_att );

	result = spu_add( spu_and( front_att, (v128)frontfacing_mask ), spu_and( back_att, (v128)backfacing_mask ) );
	
	return result;
}


v128 RingAttenuation( const CPoint_SOA& relative_position, const CPoint_SOA& direction, float radius, float width )
{
	v128 distance = relative_position.Length();
	v128 inner_radius = spu_splats(radius - 0.5f * width);
	v128 outter_radius = spu_splats(radius + 0.5f * width);

	vu128 inner_mask = spu_cmpgt( distance, inner_radius );
	vu128 outter_mask = spu_cmpgt( outter_radius, distance );
	vu128 combined_mask = spu_and( inner_mask, outter_mask );

	v128 result = spu_convtf( spu_and( combined_mask, g_vu128_all_one ), 0 );

	//v128 interp_factor = SPU_Div( spu_sub( distance, spu_splats(width * 0.5f + radius) ),  spu_splats(1.1f) );
	//result = SPU_Lerp( g_v128_allone, g_v128_allzero, interp_factor );
	
	return result;
}

