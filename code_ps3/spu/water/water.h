//--------------------------------------------------
//!
//!	\file waves.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _WATER_H_
#define _WATER_H_


#ifndef __SPU__
#	error this file is for spu projects only 
#endif

#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/vecmath_spu_soa_ps3.h"
#include "water/waterdmadata.h"


#ifndef ntDMA_ID
typedef uint32_t ntDMA_ID;
#endif


class WaterDmaBuffer
{
public:
	void Init( const WaterInstanceDma* pobWaterInstance, WaveDma* pWaveArray,
				uint32_t iMaxNumOfVertices, uint32_t iMaxNumOfWaves );

	void Release( void );

	void DmaIn( uint32_t ea, uint32_t nVerts, uint32_t offset );

	void DmaOut( void );

	void Process( v128& max_height, v128& min_height );

	inline void ProcessAndDmaOut( v128& max_height, v128& min_height )
	{
		Process( max_height, min_height );
		DmaOut();
	}

private:

	void ProcessWaves( const CPoint_SOA& pos0, v128 global_att, CPoint_SOA& position, CPoint_SOA& binormal, CPoint_SOA& tangent );

private:
	//
	// Local buffers
	//
	VertexDma*					m_pVertexArray;

	//
	// shortcuts and members
	//
	const WaterInstanceDma*		m_pobWaterInstance;
	WaveDma*					m_pWaveArray;

	uint32_t					m_iMaxNumOfVerts;
	uint32_t					m_iNumOfVerts;
	uint32_t					m_iMaxNumOfWaves;

	uint32_t					m_DmaEA0;
	ntDMA_ID					m_DmaID0;

	uint32_t					m_iOffset;
};





#endif // end of _WATER_H_
