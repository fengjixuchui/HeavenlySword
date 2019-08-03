//--------------------------------------------------
//!
//!	\file WaterInstance.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "water/WaterInstance_pc.h"
#include "water/WaterInstanceDef.h"
#include "water/watermanager.h"
#include "water/waterwaveemitter.h"
#include "water/waterdmadata.h"
#include "water/waterbuoyproxy.h"

#include "area/arearesourcedb.h"

#include "anim/hierarchy.h"

static const unsigned int MAX_GRID_SIZE = 18 * 1000;

// TODO_OZZ: bump MC_PROCEDURAL limits and change default chunk to that
#define WATER_DMA_ALIGNMENT  128
#define WATER_DMA_CHUNK Mem::MC_MISC



//
// Some helpful macros
//
#ifndef _RELEASE
#	define CHECK_MATRIX( m ) user_warn_p( m != CMatrix(CONSTRUCT_CLEAR), ("World matrix is zero!!!\n") );
#else 
#	define CHECK_MATRIX( m )
#endif

#ifndef _RELEASE
#	define CLEAN_MEMORY( ptr, size ) memset( ptr, 0, size );
#else
#	define CLEAN_MEMORY( ptr, size ) memset( ptr, 0, size );
#endif




///////////////////////////////////////////////////////////////////////////////////////////
//
//										Helpers
//
///////////////////////////////////////////////////////////////////////////////////////////
template<class T>
T* FindFirstElemWithFlags( T* pArray, uint32_t elemCount, int flags )
{
	for ( uint32_t iElem = 0; iElem < elemCount; ++iElem )
		if( pArray[iElem].m_iFlags & flags )
			return pArray + iElem;

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//										WaterInstance
//
///////////////////////////////////////////////////////////////////////////////////////////

WaterInstance::WaterInstance( WaterInstanceDef* pobDef )
: CRenderable( &pobDef->m_obTransform, true, false, false )
, m_pobWaterDma( 0 )
, m_pVertexDmaArray( 0 )
, m_pWaveDmaArray( 0 )
, m_pBuoyDmaArray( 0 )
, m_pobDef( pobDef )
{
	ntError( pobDef );
#ifdef WATER_SECTORED_DMA_RESOURCES
	const char* name = ntStr::GetString( ObjectDatabase::Get().GetNameFromPointer(m_pobDef) );
	AreaResourceDB::Get().AddAreaResource( name, AreaResource::WATER, pobDef->m_iSectorBits );
#else
	CreateDmaResources();
#endif
}


WaterInstance::~WaterInstance()
{
	DestroyDmaResources();
}


bool WaterInstance::HasDmaResources( void ) const 
{
	return m_pobWaterDma && m_pVertexDmaArray && m_pWaveDmaArray && m_pBuoyDmaArray;
}	


// NOTE: these thing recurses if resulting grid has too many cells. Just so you know...
void WaterInstance::CreateDmaResources( void )
{	
	u_int subDivsWidth  = Util::Align( static_cast<int>(floor(m_pobDef->m_fWidth/m_pobDef->m_fResolution)), 4 );
	u_int subDivsLength = Util::Align( static_cast<int>(floor(m_pobDef->m_fLength/m_pobDef->m_fResolution)), 4 );
	u_int totalGridCells = subDivsWidth * subDivsLength;

	ntAssert( Util::IsAligned( totalGridCells, 4 ) );
	
	if ( totalGridCells <= MAX_GRID_SIZE )
	{
		m_pobWaterDma = reinterpret_cast<WaterInstanceDma*>( NT_MEMALIGN_CHUNK( WATER_DMA_CHUNK, sizeof(WaterInstanceDma), WATER_DMA_ALIGNMENT ) );
		CLEAN_MEMORY( m_pobWaterDma, sizeof(WaterInstanceDma) );
		m_pobWaterDma->m_iGridSize[0] = subDivsLength;
		m_pobWaterDma->m_iGridSize[1]	= subDivsWidth;
		m_pobWaterDma->m_fWidth = m_pobDef->m_fWidth;
		m_pobWaterDma->m_fLength = m_pobDef->m_fLength;
		m_pobWaterDma->m_fMaxHeight = EPSILON;
		m_pobWaterDma->m_fMinHeight = -EPSILON;
		m_pobWaterDma->m_fResolution = m_pobDef->m_fResolution;
		m_pobWaterDma->m_fTime = 0;

		m_pobWaterDma->m_obWorldMatrix		= m_pobTransform->GetWorldMatrix();
		m_pobWaterDma->m_obWorldMatrixInv	= m_pobWaterDma->m_obWorldMatrix.GetAffineInverse();

		// init AABB to grid limits. MinY/MaxY will be updated by the spu
		m_obBounds.Min().X() = -m_pobWaterDma->m_fWidth/2;
		m_obBounds.Min().Z() = -m_pobWaterDma->m_fLength/2;
		m_obBounds.Max().X() = m_pobWaterDma->m_fWidth/2;
		m_obBounds.Max().Z() = m_pobWaterDma->m_fLength/2;
		m_obBounds.Min().Y() = m_pobWaterDma->m_fMinHeight;
		m_obBounds.Max().Y() = m_pobWaterDma->m_fMaxHeight;
		
		m_pWaveDmaArray = reinterpret_cast<WaveDma*>( NT_MEMALIGN_CHUNK( WATER_DMA_CHUNK, MAX_NUM_WAVES * sizeof(WaveDma), WATER_DMA_ALIGNMENT ) );
		CLEAN_MEMORY( m_pWaveDmaArray, MAX_NUM_WAVES * sizeof(WaveDma) );
		for ( uint32_t i = 0; i < MAX_NUM_WAVES; ++i )
		{
			m_pWaveDmaArray[i].m_iFlags = kWF_Control_Invalid;
		}

		// init buoys dma array
		m_pBuoyDmaArray = reinterpret_cast<BuoyDma*>( NT_MEMALIGN_CHUNK( WATER_DMA_CHUNK, MAX_NUM_BUOYS * sizeof(BuoyDma), WATER_DMA_ALIGNMENT ) );
		CLEAN_MEMORY( m_pBuoyDmaArray, MAX_NUM_WAVES * sizeof(BuoyDma) );
		for ( uint32_t i = 0; i < MAX_NUM_BUOYS; ++i )
		{
			m_pBuoyDmaArray[i].m_iFlags = kBF_Control_Invalid;
		}
	}
}


void WaterInstance::Update( float fTimeStep )
{
	UNUSED( fTimeStep );
}

void WaterInstance::DestroyDmaResources( void )
{
#ifndef _RELEASE
	if ( m_pobWaterDma && m_pobWaterDma->m_pDebugLineBuffer )
	{
		NT_FREE_CHUNK( Mem::MC_DEBUG, (uintptr_t)m_pobWaterDma->m_pDebugLineBuffer );
		m_pobWaterDma->m_pDebugLineBuffer = 0;
	}

	if ( m_pobWaterDma && m_pobWaterDma->m_iNumDebugLines )
	{
		NT_FREE_CHUNK( Mem::MC_DEBUG, (uintptr_t)m_pobWaterDma->m_iNumDebugLines );
		m_pobWaterDma->m_iNumDebugLines = 0;
	}
#endif

	if ( m_pWaveDmaArray )
	{
		NT_FREE_CHUNK( WATER_DMA_CHUNK, (uintptr_t)m_pWaveDmaArray );
		m_pWaveDmaArray = 0;
	}

	if ( m_pBuoyDmaArray )
	{
		NT_FREE_CHUNK( WATER_DMA_CHUNK, (uintptr_t)m_pBuoyDmaArray );
		m_pBuoyDmaArray = 0;
	}

	if ( m_pobWaterDma )
	{
		NT_FREE_CHUNK( WATER_DMA_CHUNK, (uintptr_t)m_pobWaterDma );
		m_pobWaterDma = 0;
	}
}


uint32_t	WaterInstance::GetNumOfWaveSlots( void ) const
{
	return MAX_NUM_WAVES;
}


WaveDma*	WaterInstance::GetWaveSlot( uint32_t index )
{
	ntAssert( index < GetNumOfWaveSlots() );
	return m_pWaveDmaArray + index;
}


WaveDma*	WaterInstance::GetFirstAvailableWaveSlot( void )
{
	return FindFirstElemWithFlags( m_pWaveDmaArray, GetNumOfWaveSlots(), kWF_Control_Invalid );
}

uint32_t WaterInstance::GetNumOfBuoySlots( void ) const
{
	return MAX_NUM_BUOYS;
}

BuoyDma*	WaterInstance::GetBuoySlot( uint32_t index )
{
	ntAssert( index < GetNumOfBuoySlots() );
	return m_pBuoyDmaArray + index;	
}

BuoyDma*	WaterInstance::GetFirstAvailableBuoySlot( void )
{
	return FindFirstElemWithFlags( m_pBuoyDmaArray, GetNumOfBuoySlots(), kBF_Control_Invalid );
}



const VertexDma& WaterInstance::GetVertexAtCoords( uint32_t i, uint32_t j ) const
{
	ntAssert( i < m_pobWaterDma->m_iGridSize[0] && j < m_pobWaterDma->m_iGridSize[1] );
	return m_pVertexDmaArray[ i * m_pobWaterDma->m_iGridSize[1] + j ]; 
}


const VertexDma& WaterInstance::GetVertexAtAproxPos( float x, float z ) const
{
	float nx = ntstd::Clamp( (x + 0.5f * m_pobWaterDma->m_fWidth)/m_pobWaterDma->m_fWidth, 0.0f, 1.0f );
	float nz = ntstd::Clamp( (z + 0.5f * m_pobWaterDma->m_fLength)/ m_pobWaterDma->m_fLength, 0.0f, 1.0f );
	uint32_t i = static_cast<uint32_t>( floor( nz * m_pobWaterDma->m_iGridSize[0] ) );
	uint32_t j = static_cast<uint32_t>( floor( nx * m_pobWaterDma->m_iGridSize[1] ) );

	return GetVertexAtCoords( i, j );
}

const CMatrix&	WaterInstance::GetWaterToWorldMatrix( void ) const
{
	return m_pobWaterDma->m_obWorldMatrix;
}

const CMatrix&	WaterInstance::GetWorldToWaterMatrix( void ) const
{
	return m_pobWaterDma->m_obWorldMatrixInv;
}



//eof


