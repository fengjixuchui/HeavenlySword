//--------------------------------------------------
//!
//!	\file proc_vertexbuffer.cpp
//!	Class that wraps a vertex declaration and a
//! corresponding dynamic vertex buffer
//!
//--------------------------------------------------

#include "gfx/proc_vertexbuffer.h"
#include "gfx/renderer.h"
#include "effect/effect_manager.h"

//--------------------------------------------------
//!
//!	FreeBuffers
//! Use this if you intend to build new buffers
//!
//--------------------------------------------------
void ProceduralVB::FreeBuffers()
{
	if (m_pVB)
		m_pVB = VBHandle();

	if (m_pData)
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, m_pData );
		m_pData = 0;
	}
}

//--------------------------------------------------
//!
//!	Reset
//! Use this if you intend to build a new vertex
//! declaration type as well as new buffers
//!
//--------------------------------------------------
void ProceduralVB::Reset()
{
	m_iCurrElement = 0;
	m_iVertexSize = 0;
	m_iValidElements = 0;

	FreeBuffers();
	m_scratchAllocated = false;
}

//--------------------------------------------------
//!
//!	BuildMe
//! Allocate our buffers and create our declaration.
//! Must be called before ProceduralVB is used.
//!
//--------------------------------------------------
void ProceduralVB::BuildMe( u_int iMaxVertices )
{
	m_iMaxVertices = iMaxVertices;

	ntAssert_p( m_iMaxVertices > 0, ("Pointless to have a zero sized procedural vertex buffer") );
	ntAssert_p( m_iCurrElement > 0, ("Vertex has no elements added to it yet.") );
	ntAssert_p( m_iCurrElement < MAX_VERTEX_ELEMENTS, ("Too many elements added to Vertex type") );

	if (IsValid())
		FreeBuffers();

	u_int iValidator = 0;

	// build stream field array and vertex size

	GcStreamField	geometryDesc[MAX_VERTEX_ELEMENTS];
	for (u_int i = 0; i < m_iCurrElement; i++)
	{
		VERTEX_DECL_STREAM_TYPE type = m_aVertexElements[i].m_type;

		geometryDesc[i] = GcStreamField(	FwHashedString( m_aVertexElements[i].m_bindNameHash ),
											iValidator,
											GetStreamType( type ),
											GetStreamElements( type ),
											IsTypeNormalised( type )
										);

		iValidator += GetSizeOfElement( type );
	}

	ntAssert( iValidator == m_iVertexSize );

	// now allocate some memory for the buffers

	int iBufferSize = m_iVertexSize * m_iMaxVertices;

	m_pData = NT_NEW_CHUNK( Mem::MC_GFX ) uint8_t [iBufferSize];

	if (!EffectManager::AllocsDisabled())
	{
		m_pVB = RendererPlatform::CreateVertexStream(	m_iMaxVertices,
														m_iVertexSize,
														m_iCurrElement,
														geometryDesc,
														Gc::kScratchBuffer );
	}
}

//--------------------------------------------------
//!
//!	CanSubmitToGPU
//! check to see if we have enough dynamic memory
//! to draw us
//!
//--------------------------------------------------
bool ProceduralVB::CanSubmitToGPU( u_int iNumVerts )
{
	if ( iNumVerts == 0xffffffff )	
	{
		return m_pVB->QueryGetNewScratchMemory();
	}
	else
	{
		ntError_p( iNumVerts <= m_pVB->GetCount(), ("Too many verts requested") );
		return GcKernel::QueryGetNewScratchMemory( m_pVB->GetStride() * iNumVerts, Gc::kStreamBufferAlignment );
	}
}

//--------------------------------------------------
//!
//!	SubmitToGPU
//! copy contents of m_pData to discard buffer
//!
//--------------------------------------------------
void ProceduralVB::SubmitToGPU( u_int iNumVerts )
{
	ntError( !EffectManager::AllocsDisabled() );
	ntError( CanSubmitToGPU( iNumVerts ) );

	// copy to discard buffer
	if (iNumVerts == 0xffffffff)
	{
		m_pVB->GetNewScratchMemory();
		m_pVB->Write( m_pData );
	}
	else
	{
		ntError_p( iNumVerts <= m_iMaxVertices, ("Too many verts requested") );
		m_pVB->GetNewScratchMemory( iNumVerts );
		m_pVB->Write( m_pData, 0, iNumVerts * m_pVB->GetStride() );
	}

	// set vertex buffer as stream source
	Renderer::Get().m_Platform.SetStream( m_pVB );
}

//--------------------------------------------------
//!
//!	LockForWrite
//! Allocate our discard buffer and get its address
//!
//--------------------------------------------------
void ProceduralVB::LockForWrite( u_int iNumVerts )
{
	ntError( !EffectManager::AllocsDisabled() );
	ntError( CanSubmitToGPU( iNumVerts ) );

	ntAssert_p( m_scratchAllocated == false, ("This ProceduralVB already has scratch VRAM allocated!") );
	m_scratchAllocated = true;

	if (iNumVerts == 0xffffffff)
	{
		m_pVB->GetNewScratchMemory();
	}
	else
	{
		ntError_p( iNumVerts <= m_iMaxVertices, ("Too many verts requested") );
		m_pVB->GetNewScratchMemory( iNumVerts );
	}
}

//--------------------------------------------------
//!
//!	Write
//! NT_MEMCPY into our discard buffer
//!
//--------------------------------------------------
void ProceduralVB::Write( const uint8_t* pData, uint32_t offset, uint32_t size )
{
	ntError( !EffectManager::AllocsDisabled() );

	ntAssert_p( m_scratchAllocated == true, ("This ProceduralVB has no scratch VRAM allocated!") );
	m_pVB->Write( pData, offset, size );
}

//--------------------------------------------------
//!
//!	UnlockAndSubmit
//! signal we've finished writing and bind the stream
//!
//--------------------------------------------------
void ProceduralVB::UnlockAndSubmit()
{
	ntError( !EffectManager::AllocsDisabled() );

	ntAssert_p( m_scratchAllocated == true, ("This ProceduralVB has no scratch VRAM allocated!") );
	m_scratchAllocated = false;

	Renderer::Get().m_Platform.SetStream( m_pVB );
}



