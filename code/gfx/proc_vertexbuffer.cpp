//--------------------------------------------------
//!
//!	\file proc_vertexbuffer.cpp
//!	Class that wraps a vertex declaration and a
//! corresponding dynamic vertex buffer
//!
//--------------------------------------------------

#include "gfx/proc_vertexbuffer.h"

//--------------------------------------------------
//!
//!	FreeBuffers
//! Use this if you intend to build new buffers
//!
//--------------------------------------------------
void ProceduralDB::FreeBuffers()
{
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
void ProceduralDB::Reset()
{
	m_iVertexSize = 0;
	m_iValidElements = 0;
	FreeBuffers();
}

//--------------------------------------------------
//!
//!	BuildMe
//!
//--------------------------------------------------
void ProceduralDB::BuildMe( u_int iMaxVertices )
{
	m_iMaxVertices = iMaxVertices;

	ntAssert_p( m_iMaxVertices > 0, ("Pointless to have a zero sized procedural vertex buffer") );
	ntAssert_p( m_iVertexSize > 0, ("Vertex has no elements added to it yet.") );

	if (IsValid())
		FreeBuffers();

	// allocate some memory for the bufferers
	int iBufferSize = m_iVertexSize * m_iMaxVertices;
	m_pData = NT_NEW_ARRAY_CHUNK( Mem::MC_GFX ) uint8_t [iBufferSize];
}
