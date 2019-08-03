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
#include "gfx/graphicsdevice.h"
#include "gfx/dxerror_pc.h"

//--------------------------------------------------
//!
//!	FreeBuffers
//! Use this if you intend to build new buffers
//!
//--------------------------------------------------
void ProceduralVB::FreeBuffers()
{
	if (m_pVB)
		m_pVB.SafeRelease();

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
	if (m_pDecl)
		m_pDecl.SafeRelease();

	m_iCurrElement = 0;
	m_iVertexSize = 0;
	m_iValidElements = 0;
	m_bLocked = false;

	FreeBuffers();
}

//--------------------------------------------------
//!
//!	OverideDeclaration
//! overide vertex declaration; change from procedural mapping of
//! vertex elements from texcoord0 -> n to an aribitrary decl
//!
//--------------------------------------------------
void ProceduralVB::OverideDeclaration( D3DVERTEXELEMENT9* pDecl )
{
	ntError(pDecl);

	for (u_int i = 0; i < m_iCurrElement; i++)
	{
		ntError_p( pDecl[i].Type == (uint8_t)m_aVertexElements[i], ("Decl does not match existing vertex usage") );
	}

	m_pDecl = CVertexDeclarationManager::Get().GetDeclaration( pDecl );
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

	// build vertex declaration and vertex size

	D3DVERTEXELEMENT9 decl[MAX_VERTEX_ELEMENTS];
	for (u_int i = 0; i < m_iCurrElement; i++)
	{
		decl[i].Stream = 0;
		decl[i].Offset = (WORD)iValidator;
		decl[i].Type = (uint8_t)m_aVertexElements[i];
		decl[i].Method = 0;
		decl[i].Usage = D3DDECLUSAGE_TEXCOORD;
		decl[i].UsageIndex = (uint8_t)i;

		iValidator += SizeOfElement(m_aVertexElements[i]);
	}

	ntAssert( iValidator == m_iVertexSize );

	static const D3DVERTEXELEMENT9 stEnd = D3DDECL_END();
	NT_MEMCPY( &decl[m_iCurrElement], &stEnd, sizeof( D3DVERTEXELEMENT9 ) );

	m_pDecl = CVertexDeclarationManager::Get().GetDeclaration( decl );
		
	ntAssert_p( m_pDecl, ("Vertex declaration creation failed.") );

	// now allocate some memory for the buffers

	int iBufferSize = m_iVertexSize * m_iMaxVertices;

	m_pData = NT_NEW_CHUNK(Mem::MC_GFX) uint8_t [iBufferSize];

	if (!EffectManager::AllocsDisabled())
		m_pVB = Renderer::Get().m_Platform.CreateDynamicVertexBuffer( iBufferSize );
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

	// copy to discard buffer
	LockForWrite( iNumVerts );
	Write( m_pData, 0, m_iVertexSize*m_iMaxVertices );
	UnlockAndSubmit();
}

//--------------------------------------------------
//!
//!	LockForWrite
//! Allocate our discard buffer and get its address
//!
//--------------------------------------------------
void ProceduralVB::LockForWrite( u_int )
{
	ntAssert_p( m_bLocked == false, ("This ProceduralVB is already locked!") );
	m_bLocked = true;
	dxerror_p(	m_pVB->Lock( 0, 0, (void**)&m_pWriteAddres, D3DLOCK_DISCARD ),
				("Falided to allocate our discard buffer for ProceduralVB") );
}

//--------------------------------------------------
//!
//!	Write
//! NT_MEMCPY into our discard buffer
//!
//--------------------------------------------------
void ProceduralVB::Write( const uint8_t* pData, uint32_t offset, uint32_t size )
{
	ntAssert_p( m_bLocked == true, ("This ProceduralVB is not already locked!") );
	NT_MEMCPY( m_pWriteAddres+offset, pData, size );
}

//--------------------------------------------------
//!
//!	UnlockAndSubmit
//! signal we've finished writing, set the decl
//! and bind the stream
//!
//--------------------------------------------------
void ProceduralVB::UnlockAndSubmit()
{
	ntAssert_p( m_bLocked == true, ("This ProceduralVB is not already locked!") );
	m_bLocked = false;
	m_pWriteAddres = 0;

	m_pVB->Unlock();
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pDecl );
	GetD3DDevice()->SetStreamSource( 0, m_pVB.Get(), 0, m_iVertexSize );
}
