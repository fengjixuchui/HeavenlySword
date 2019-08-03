#include "SpeedTreeShaders_ps3.h"

#include "speedtree/SpeedTreeManager_ps3.h"
#include "speedtree/speedtreebillboard.h"

#include "gfx/renderer.h"
#include "gfx/shader.h"
#include "core/explicittemplate.h"
#include "gfx/graphicsdevice.h"
#include "gfx/rendercontext.h"
#include "camera/camman_public.h"
#include "anim/transform.h"
#include "core/exportstruct_clump.h"
#include "core/debug.h"


#define SPEEDTREE_USE_SCRATCH_BUFFERS

unsigned int SpeedTreeLeafBuffers::GetVertexFootprint()
{
	unsigned int footprint = 0;
	for (int i = 0; i < m_usNumLods; ++ i)
	{
		if (m_pVertexBuffers[i].Get())
		{
			footprint += (unsigned int)GcStreamBuffer::QueryResourceSizeInBytes( m_pVertexBuffers[i] -> GetCount(), m_pVertexBuffers[i] -> GetStride() );
		}
	}

	return footprint;
}

unsigned int SpeedTreeIndexedBuffers::GetVertexFootprint()
{
	unsigned int footprint = 0;
	if (m_pVertexBuffer.Get())
	{
		footprint += (unsigned int)GcStreamBuffer::QueryResourceSizeInBytes( m_pVertexBuffer -> GetCount(), m_pVertexBuffer -> GetStride() );
	}
	return footprint;
}

unsigned int SpeedTreeIndexedBuffers::GetIndexFootprint()
{
	unsigned int footprint = 0;
	if (m_pIndexBuffer.Get())
	{
		footprint += (unsigned int)GcStreamBuffer::QueryResourceSizeInBytes( m_pIndexBuffer -> GetIndexType(), m_pIndexBuffer -> GetCount() );
	}
	return footprint;
}



CSpeedTreeBillboardBuffers::CSpeedTreeBillboardBuffers(unsigned int numElements)
{
	const unsigned int numFields = 4;
	GcStreamField	fields[numFields];
	NT_PLACEMENT_NEW (&fields[0]) GcStreamField( FwHashedString("input.vPosition"), 0, Gc::kFloat, 4 );
	NT_PLACEMENT_NEW (&fields[1]) GcStreamField( FwHashedString("input.vGeom"), 16, Gc::kFloat, 4 );
	NT_PLACEMENT_NEW (&fields[2]) GcStreamField( FwHashedString("input.vMiscParams"), 32, Gc::kFloat, 3 );
	NT_PLACEMENT_NEW (&fields[3]) GcStreamField( FwHashedString("input.vLightAdjusts"), 48, Gc::kFloat, 3 );

	for (unsigned int buffer = 0; buffer < c_numBuffers; ++ buffer)
	{
		m_bufferPool[buffer].m_vertexBuffer = RendererPlatform::CreateVertexStream(
			numElements, 
			sizeof(SpeedTreeBillboardVertex),
			numFields,
			fields,
#ifdef SPEEDTREE_USE_SCRATCH_BUFFERS
			Gc::kScratchBuffer );
#else
			Gc::kStaticBuffer );
#endif

		m_bufferPool[buffer].m_cell = NULL;
	}

	if (numElements > m_scratchSize)
	{
		m_scratchSize = numElements;
	}
}


unsigned int CSpeedTreeBillboardBuffers::SubmitVertexData(int bufferIndex, unsigned int treeIndex)
{
	ntAssert((unsigned int)bufferIndex < c_numBuffers);
	if (!m_bufferPool[bufferIndex].m_cell)
	{
		return 0;
	}

	VBHandle	buffer = m_bufferPool[bufferIndex].m_vertexBuffer;

#ifdef SPEEDTREE_USE_SCRATCH_BUFFERS
	if ( buffer -> QueryGetNewScratchMemory() == false )
		return 0;

	buffer -> GetNewScratchMemory();
#endif

	if (!m_scratchPad)
	{
		m_scratchPad = NT_NEW_ARRAY_CHUNK(SPEEDTREE_MEMORY_CHUNK) SpeedTreeBillboardVertex[m_scratchSize];
	}


	unsigned int numVerts = m_bufferPool[bufferIndex].m_cell -> SubmitGeometry(m_scratchPad, treeIndex);

	if (numVerts)
	{
		buffer -> Write(m_scratchPad, 0, numVerts * sizeof(SpeedTreeBillboardVertex));
	}
	//Renderer::Get().m_Platform.SetStream( buffer );


	return numVerts;
}

VBHandle	CSpeedTreeBillboardBuffers::GetBuffer(int bufferIndex)
{
	ntAssert((unsigned int)bufferIndex < c_numBuffers);
	ntAssert(m_bufferPool[bufferIndex].m_cell);

	return m_bufferPool[bufferIndex].m_vertexBuffer;
}

void CSpeedTreeBillboardBuffers::Destroy()
{
	NT_DELETE_ARRAY_CHUNK(SPEEDTREE_MEMORY_CHUNK, m_scratchPad);
	m_scratchPad = NULL;
	m_scratchSize = 0;
}

SpeedTreeBillboardVertex*	CSpeedTreeBillboardBuffers::m_scratchPad = NULL;
unsigned int				CSpeedTreeBillboardBuffers::m_scratchSize = 0;
