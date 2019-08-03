
#include "water.h"
#include "waves.h"
#include "water_utils.h"
#include "attenuation.h"
#include "water_spu_config.h"

#include <basetypes_spu.h>
#include <debug_spu.h>
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/vecmath_spu_soa_ps3.h"
#include "ntlib_spu/ntDma.h"


#include <float.h>
#include <stdlib.h>
#include <math.h>

#define QUADWORD_COMP_X 0
#define QUADWORD_COMP_Y 1
#define QUADWORD_COMP_Z 2
#define QUADWORD_COMP_W 3

#ifndef TWO_PI
#	define TWO_PI 6.283185307179586476925286766559f
#endif

#ifndef PI
#	define PI 3.1415926535897932384626433832795f
#endif


static const v128 DEFAULT_BINORMAL	= (v128){ 1.0f, 0.0f, 0.0f, 0.0f };
static const v128 DEFAULT_TANGENT	= (v128){ 0.0f, 0.0f, 1.0f, 0.0f };
static const CQuat QuatYRot( CDirection(VECTORMATH_UNIT_0100), 90 * 0.01745329f  );
static const CQuat QuatYRot_SOA( QuatYRot  );


//
// Debug stuff
//
#ifndef _RELEASE
void DEBUG_DrawLine( const WaterInstanceDma& header, const CPoint& a, const CPoint& b, uint32_t col );
void DEBUG_DrawTangentBasis( const WaterInstanceDma& header, const VertexDma& vertex );
#endif

void CheckNormalisedDir( const CPoint_SOA& wave_direction );


//
// Helper fwd declarations
//
CPoint_SOA GetUnperturbedGridPosition_SOA( uint32_t index0, const WaterInstanceDma& water );


//////////////////////////////////////////////////////////////////////////////////////
//																					//
//									WaterDmaBuffer									//
//																					//
//////////////////////////////////////////////////////////////////////////////////////

void WaterDmaBuffer::Init( const WaterInstanceDma* pobWaterInstance, WaveDma* pWaveArray,
							uint32_t iMaxNumOfVertices, uint32_t iMaxNumOfWaves  )
{
	// number of verts to process *MUST* be 4-aligned
	ntAssert( Util::IsAligned( iMaxNumOfVertices, 4 ) );
	ntAssert( pobWaterInstance );
	ntAssert( pWaveArray );

	m_pobWaterInstance = pobWaterInstance;
	m_pWaveArray = pWaveArray;

	m_iMaxNumOfVerts = iMaxNumOfVertices;
	m_iMaxNumOfWaves = iMaxNumOfWaves;

	m_DmaID0 = ntDMA::GetFreshID();

	m_pVertexArray = reinterpret_cast<VertexDma*>( Allocate( sizeof(VertexDma) * m_iMaxNumOfVerts ) );
	ntError_p( m_pVertexArray != NULL, ("Unable to allocate water LS vertex buffer\n") );
}


void WaterDmaBuffer::Release( void )
{
	ntDMA::StallForCompletion( m_DmaID0 );
	ntDMA::FreeID( m_DmaID0 );
}


void WaterDmaBuffer::DmaIn( uint32_t ea, uint32_t nVerts, uint32_t offset )
{
	ntAssert_p( nVerts <= m_iMaxNumOfVerts, ("nVerts: %i, MaxVerts: %i\n", nVerts, m_iMaxNumOfVerts) );
	ntAssert_p( Util::IsAligned( nVerts, 4 ), ("nVerts(%i) not aligned to 4", nVerts) );

	// wait for the previous dma out to complete before getting new data in
	ntDMA::StallForCompletion( m_DmaID0 );

	m_iNumOfVerts = nVerts;
	m_DmaEA0 = ea;
	//ntDMA::Params param;
	//param.Init32( m_pVertexArray, m_DmaEA0, m_iNumOfVerts*sizeof(VertexDma), m_DmaID0 );
	//ntDMA::DmaToSPU( param );

	m_iOffset = offset;
}



void WaterDmaBuffer::DmaOut()
{
	if ( m_iNumOfVerts > 0 )
	{
		//ntAssert( ntDMA::HasCompleted( m_DmaID0 ) );

		ntDMA::Params param;
		param.Init32( m_pVertexArray, m_DmaEA0, m_iNumOfVerts*sizeof(VertexDma), m_DmaID0 );
		ntDMA::DmaToPPU( param );	
	}

	// this means we have no more verts to process. Next DMA_In will set this again
	m_iNumOfVerts = 0;
}




void WaterDmaBuffer::Process( v128& max_height, v128& min_height )
{
	using namespace Intrinsics;

	if (  m_iNumOfVerts > 0 )
	{
		// vertical scale serves as global attenuation
		const v128 global_att = spu_splats( m_pobWaterInstance->m_fVScale );

		// process vertices in groups of four
		for ( uint32_t iVert = 0; iVert < m_iNumOfVerts; iVert+=4 )
		{
			// rebuild swizzled unperturbed surface positions in the form:  xxxx yyyy zzzz
			const CPoint_SOA pos0 = GetUnperturbedGridPosition_SOA( iVert + m_iOffset, *m_pobWaterInstance );
			
			// wave influence accumulators 
			CPoint_SOA position( pos0 );
			CPoint_SOA binormal( DEFAULT_BINORMAL );
			CPoint_SOA tangent( DEFAULT_TANGENT );

			// accumulate wave influences over these vertices
			ProcessWaves( pos0, global_att, position, binormal, tangent );

			// don't forget to normalise'em so we don't have to in the vertex shader
			binormal.Normalise();
			tangent.Normalise();

			// xxxx yyyy zzzz --> xyz0 xyz0 xyz0 xyz0
			position.DeSwizzle( m_pVertexArray[iVert].m_position.Quadword(), m_pVertexArray[iVert+1].m_position.Quadword(), m_pVertexArray[iVert+2].m_position.Quadword(), m_pVertexArray[iVert+3].m_position.Quadword() );
			binormal.DeSwizzle( m_pVertexArray[iVert].m_binormal.Quadword(), m_pVertexArray[iVert+1].m_binormal.Quadword(), m_pVertexArray[iVert+2].m_binormal.Quadword(), m_pVertexArray[iVert+3].m_binormal.Quadword() );
			tangent.DeSwizzle( m_pVertexArray[iVert].m_tangent.Quadword(), m_pVertexArray[iVert+1].m_tangent.Quadword(), m_pVertexArray[iVert+2].m_tangent.Quadword(), m_pVertexArray[iVert+3].m_tangent.Quadword() );

			// set m_position.w back to 1
			for ( int i = 0; i < 4; ++i )
			{
				m_pVertexArray[iVert+i].m_position.Quadword() = spu_shuffle( m_pVertexArray[iVert+i].m_position.QuadwordValue(), g_v128_allone, VECTORMATH_SHUF_XYZA );
			}

			// update height limits so far
			max_height = SPU_Max( position.Y(), max_height );
			min_height = SPU_Min( position.Y(), min_height );

#ifndef _RELEASE
			if ( !((iVert+m_iOffset) % 3 ) )
			{
				DEBUG_DrawTangentBasis( *m_pobWaterInstance, m_pVertexArray[iVert] );
			}	
#endif
		}
	}
}


void WaterDmaBuffer::ProcessWaves( const CPoint_SOA& pos0, v128 global_att, CPoint_SOA& position, CPoint_SOA& binormal, CPoint_SOA& tangent )
{
	float amp, freq, phase, t, k;
#ifdef WATER_SORT_WAVE_ARRAY
	for ( uint32_t iWave = 0; iWave < m_iMaxNumOfWaves && m_pWaveArray[iWave].IsValid(); ++iWave )
#else
	for ( uint32_t iWave = 0; iWave < m_iMaxNumOfWaves; ++iWave )
#endif
	{
		const WaveDma& wave = m_pWaveArray[iWave];

		// amplitude and frequency are always greater than zero (clamped when emitted)
		amp = wave.m_fAmplitude;
		freq = wave.m_fFrequency;							
		phase = wave.m_fPhase;
		t = wave.m_fAge;
		k = ntstd::Clamp( wave.m_fSharpness, EPSILON, 1.0f / (freq * amp ) );

		v128 att = global_att;

		// xyzw xyzw xyzw xyzw --> xxxx yyyy zzzz 
		CPoint_SOA wave_direction( wave.m_obDirection.QuadwordValue() );
		CPoint_SOA wave_origin( wave.m_obOrigin.QuadwordValue() );
		CPoint_SOA relative_vertex_pos( pos0 - wave_origin );


		switch ( wave.m_iFlags & kWF_Type_All )
		{
			case kWF_Type_Circular:
			{
				wave_direction = pos0 - wave_origin;
				wave_direction.Normalise();
				break;
			}
			
			case kWF_Type_Directional:
			{
				break;
			}

			case kWF_Type_Attack0:
			{
				t = 1.0f;
				phase = 0;
				// wave width attenuation
				att = spu_mul( att, WidthAttenuation( relative_vertex_pos, wave_direction, wave.m_TypeSpecific.m_Attack0.m_fWidth, wave.m_TypeSpecific.m_Attack0.m_fFalloff ) );
				// profile attenuation
				const CPoint_SOA perp_dir( QuatYRot.Rotate( wave.m_obDirection.QuadwordValue() ) );
				att = spu_mul( att, WidthAttenuation( relative_vertex_pos, perp_dir, PI / wave.m_fFrequency, 0.0f ) );
				break;
			}

			case kWF_Type_Attack1:
			{
				t = 1.0f;
				phase = 0; 
				att = spu_mul( att, TrailAttenuation( relative_vertex_pos, wave_direction, wave.m_TypeSpecific.m_Attack1.m_fBackTrail, wave.m_TypeSpecific.m_Attack1.m_fFrontTrail ) );
				att = spu_mul( att, WidthAttenuation( relative_vertex_pos, wave_direction, PI / wave.m_fFrequency, 0.0f ) );
				wave_direction = CPoint_SOA( QuatYRot.Rotate( wave.m_obDirection.QuadwordValue() ) );
				wave_direction.Normalise();
				break;
			}

			case kWF_Type_Attack2:
			{
				t = 1.0f;
				wave_direction = pos0 - wave_origin;
				wave_direction.Normalise();
				phase = wave.m_TypeSpecific.m_Attack2.m_fRadius * freq;
				// profile-ring attenuation
				att = spu_mul( att, RingAttenuation( relative_vertex_pos, wave_direction, wave.m_TypeSpecific.m_Attack2.m_fRadius, TWO_PI / wave.m_fFrequency ) );
				break;
			}
		}

		att = spu_mul( att, DistanceAttenuation( relative_vertex_pos, wave.m_fAttLinear, wave.m_fAttQuadratic ) );
		att = spu_mul( att, AgeAttenuation( wave.m_fAge, wave.m_fMaxAge, wave.m_fFadeInTime, wave.m_fFadeOutTime ) );

		Gerstner_SOA( relative_vertex_pos, wave_direction, freq, amp, phase, k, t, att, 
			position, binormal, tangent );
	}
}



///////////////////////////////////////////////////////////////////////////////////////
//
//										Helpers
//
///////////////////////////////////////////////////////////////////////////////////////

CPoint_SOA GetUnperturbedGridPosition_SOA( uint32_t index0, const WaterInstanceDma& water )
{
	typedef vector signed int vi128;

	//ntAssert_p( index0 < water.m_iGridSize[0] * water.m_iGridSize[1] - 4, ("Water: index0 is out of bounds\n") );

	// map vertex indices from 1D --> 2D
	vi128 iiii = (vi128){0,0,0,0};
	vi128 jjjj = (vi128){0,0,0,0};
	for ( int e = 0; e < 4; ++e )
	{
		const int index = index0 + e;
		iiii = spu_insert( index / water.m_iGridSize[1], iiii, e );
		jjjj = spu_insert( index % water.m_iGridSize[1], jjjj, e );
	}
	const v128 resolution = spu_splats(water.m_fResolution);
	const v128 one_half = spu_splats(0.5f);

	// now compute the actual positions from the 2D coords
	return CPoint_SOA
		(
		spu_sub( spu_mul( spu_convtf( jjjj, 0 ), resolution ), spu_mul( one_half, spu_splats(water.m_fWidth) ) ),
		g_v128_allzero,
		spu_sub( spu_mul( spu_convtf( iiii, 0 ), resolution ), spu_mul( one_half, spu_splats(water.m_fLength) ) )
		);
}


void CheckNormalisedDir( const CPoint_SOA& wave_direction )
{
	v128 lengths = wave_direction.Length();
	UNUSED( lengths );
	ntAssert_p( !ElemCmpAnyGreaterThan( lengths, 1.0f + EPSILON ), ("Wave direction is not normalised\n") );
}


#if !defined( _RELEASE )

#	include "syncprims_spu.h"

	inline void DEBUG_DrawLine( const WaterInstanceDma& header, const CPoint& a, const CPoint& b, uint32_t col )
	{
		uint32_t iIndex = AtomicIncrementU( (uint32_t) header.m_iNumDebugLines );

		if (  iIndex < MAX_DEBUG_LINES )
		{
			//ntPrintf( "Spu single line  %i \n", iIndex);
			uint32_t eaAddr = ((uint32_t)header.m_pDebugLineBuffer) + (iIndex * sizeof(SPU_DebugLine));
			SPU_DebugLine tmp;
			tmp.a = a;
			tmp.b = b;
			tmp.col = col;

			ntDMA::Params lineDmaParams;
			ntDMA_ID lineId = ntDMA::GetFreshID();
			lineDmaParams.Init32( &tmp, eaAddr, sizeof(SPU_DebugLine) , lineId );
			ntDMA::DmaToPPU( lineDmaParams );
 			ntDMA::StallForCompletion( lineId );
			ntDMA::FreeID( lineId );
		}
	}

	void DEBUG_DrawTangentBasis( const WaterInstanceDma& header, const VertexDma& vertex )
	{
		const CPoint& P = vertex.m_position;
		const CDirection& B = vertex.m_binormal;
		const CDirection& T = vertex.m_tangent;
		
		CDirection N( T.Cross(B) );
		N.Normalise();

		const float scale = 0.2f;
		DEBUG_DrawLine( header, P, P + scale * N, 0x00ff00ff );
		//DEBUG_DrawLine( header, P, P + scale * B, 0x0000ffff );
		//DEBUG_DrawLine( header, P, P + scale * T, 0xff0000ff );
	}

#endif // _RELEASE



