//--------------------------------------------------
//!
//!	\file proc_vertexbuffer_pc.h
//!	Class that wraps a vertex declaration and a
//! corresponding dynamic vertex buffer
//!
//--------------------------------------------------

#ifndef GFX_PROC_VB_PC
#define GFX_PROC_VB_PC

#ifndef GFX_VERTEXDECLARATION_H
#include "gfx/vertexdeclaration.h"
#endif

//--------------------------------------------------
//!
//!	Procedural Vertex Buffer
//!
//--------------------------------------------------
class ProceduralVB : public CNonCopyable
{
public:
	ProceduralVB() : m_pData(0) { Reset(); }
	~ProceduralVB() { Reset(); }

	void Reset();
	void FreeBuffers();
	void BuildMe( u_int iMaxVertices );

	// add this element to the declaration, store its position within the vertex
	void PushVertexElement( VERTEX_DECL_STREAM_TYPE type, u_int iSlot, const char* pPS3Only )
	{
		UNUSED(pPS3Only);	// on PS3 this is the bind field name

		ntAssert_p( m_iCurrElement < MAX_VERTEX_ELEMENTS, ("Added too many vertex elements") );
		ntAssert_p( iSlot < MAX_VERTEX_ELEMENTS, ("invalid slot for vertex element") );
		ntAssert_p( !IsSlotUsed(iSlot), ("Added too many vertex elements") );

		D3DDECLTYPE d3dType = MapStreamType(type);
		m_aVertexElements[ m_iCurrElement ] = d3dType;
		m_iCurrElement++;

		m_iValidElements |= (1<<iSlot);
		m_aElementOffsets[iSlot] = (uint8_t)m_iVertexSize;
		m_aElementSizes[iSlot] = (uint8_t)SizeOfElement( d3dType );

		m_iVertexSize += SizeOfElement( d3dType );
	}

	bool IsValid() const { return (m_pData != 0); }
	bool IsSlotUsed( u_int iSlot ) const { return !!(m_iValidElements & (1 << iSlot)); }

	u_int GetVertexSize() const	{ ntAssert( IsValid() ); return m_iVertexSize; }
	u_int GetMaxVertices() const{ ntAssert( IsValid() ); return m_iMaxVertices; }

	void* GetVertex( u_int iIndex )
	{
		ntAssert( IsValid() );
		ntAssert( iIndex < m_iMaxVertices );

		return m_pData + (iIndex * m_iVertexSize);
	}

	void* GetVertexElement( u_int iIndex, u_int iSlot )
	{
		ntAssert( IsValid() );
		ntAssert( iIndex < m_iMaxVertices );
		ntAssert_p( IsSlotUsed(iSlot), ("Vertex Element not present in vertex") );

		return m_pData + (iIndex * m_iVertexSize) + m_aElementOffsets[iSlot];
	}

	void SetVertex( u_int iIndex, void* pValue )
	{
		ntAssert( IsValid() );
		ntAssert( iIndex < m_iMaxVertices );
		ntAssert( pValue );

		NT_MEMCPY( m_pData + (iIndex * m_iVertexSize), pValue, m_iVertexSize );
	}

	void SetVertexElement( u_int iIndex, u_int iSlot, void* pValue )
	{
		ntAssert( IsValid() );
		ntAssert( iIndex < m_iMaxVertices );
		ntAssert( pValue );
		ntAssert_p( IsSlotUsed(iSlot), ("Vertex Element not present in vertex") );

		NT_MEMCPY( m_pData + (iIndex * m_iVertexSize) + m_aElementOffsets[iSlot],
				pValue, m_aElementSizes[iSlot] );
	}

	// here for lazy compatability code
	bool CanSubmitToGPU( u_int = 0xffffffff ) { return true; }

	// copy contents of m_pData to discard buffer
	void SubmitToGPU( u_int = 0xffffffff );

	// alloc a new discard buffer so we can write explicitly
	void LockForWrite( u_int = 0xffffffff  );
	void Write( const uint8_t* pData, uint32_t offset, uint32_t size );
	void UnlockAndSubmit();

	// overide vertex declaration; change from procedural mapping of
	// vertex elements from texcoord0 -> n to an aribitrary decl
	void OverideDeclaration( D3DVERTEXELEMENT9* pDecl );

private:
	static const u_int MAX_VERTEX_ELEMENTS = PV_ELEMENT_MAX;

	// construction data
	D3DDECLTYPE	m_aVertexElements[MAX_VERTEX_ELEMENTS];
	u_int m_iCurrElement;
	static inline u_int SizeOfElement( D3DDECLTYPE type );
	static inline D3DDECLTYPE MapStreamType( VERTEX_DECL_STREAM_TYPE eType );

	// runtime data
	CVertexDeclaration	m_pDecl;
	VBHandle			m_pVB;
	uint8_t*			m_pData;
	u_int				m_iVertexSize;
	u_int				m_iMaxVertices;

	u_int				m_iValidElements;
	uint8_t				m_aElementOffsets[MAX_VERTEX_ELEMENTS];
	uint8_t				m_aElementSizes[MAX_VERTEX_ELEMENTS];

	bool				m_bLocked;
	uint8_t*			m_pWriteAddres;
};

//--------------------------------------------------
//!
//!	SizeOfElement
//!
//--------------------------------------------------
inline u_int ProceduralVB::SizeOfElement( D3DDECLTYPE type )
{
	switch(type)
	{
	case D3DDECLTYPE_FLOAT1:	return (sizeof(float) * 1);
	case D3DDECLTYPE_FLOAT2:	return (sizeof(float) * 2);
	case D3DDECLTYPE_FLOAT3:	return (sizeof(float) * 3);
	case D3DDECLTYPE_FLOAT4:	return (sizeof(float) * 4);
	case D3DDECLTYPE_D3DCOLOR:	return (sizeof(uint32_t) * 1);
	case D3DDECLTYPE_UBYTE4:	return (sizeof(uint32_t) * 1);
	case D3DDECLTYPE_SHORT2:	return (sizeof(short) * 2);
	case D3DDECLTYPE_SHORT4:	return (sizeof(short) * 4);
	case D3DDECLTYPE_UBYTE4N:	return (sizeof(uint32_t) * 1);
	case D3DDECLTYPE_SHORT2N:	return (sizeof(short) * 2);
	case D3DDECLTYPE_SHORT4N:	return (sizeof(short) * 4);
	case D3DDECLTYPE_USHORT2N:	return (sizeof(short) * 2);
	case D3DDECLTYPE_USHORT4N:	return (sizeof(short) * 4);
	case D3DDECLTYPE_UDEC3:		return (sizeof(uint32_t) * 1);
	case D3DDECLTYPE_DEC3N:		return (sizeof(uint32_t) * 1);
	case D3DDECLTYPE_FLOAT16_2:	return (sizeof(short) * 2);
	case D3DDECLTYPE_FLOAT16_4:	return (sizeof(short) * 4);

	case D3DDECLTYPE_UNUSED:
	default:
		ntAssert(0);
		return 0;
	}
}

//--------------------------------------------------
//!
//!	MapStreamType
//! convert VERTEX_DECL_STREAM_TYPE to D3DDECLTYPE
//!
//--------------------------------------------------
inline D3DDECLTYPE ProceduralVB::MapStreamType( VERTEX_DECL_STREAM_TYPE eType )
{
	switch( eType )
	{
	case VD_STREAM_TYPE_FLOAT1: return D3DDECLTYPE_FLOAT1;
	case VD_STREAM_TYPE_FLOAT2: return D3DDECLTYPE_FLOAT2;
	case VD_STREAM_TYPE_FLOAT3: return D3DDECLTYPE_FLOAT3;
	case VD_STREAM_TYPE_FLOAT4: return D3DDECLTYPE_FLOAT4;
	case VD_STREAM_TYPE_PACKED: return D3DDECLTYPE_D3DCOLOR;

	case VD_STREAM_TYPE_UBYTE4: return D3DDECLTYPE_UBYTE4;
	case VD_STREAM_TYPE_SHORT2: return D3DDECLTYPE_SHORT2;
	case VD_STREAM_TYPE_SHORT4: return D3DDECLTYPE_SHORT4;

	case VD_STREAM_TYPE_UBYTE4N: return D3DDECLTYPE_UBYTE4N;
	case VD_STREAM_TYPE_SHORT2N: return D3DDECLTYPE_SHORT2N;
	case VD_STREAM_TYPE_SHORT4N: return D3DDECLTYPE_SHORT4N;

	case VD_STREAM_TYPE_USHORT2N: return D3DDECLTYPE_USHORT2N;
	case VD_STREAM_TYPE_USHORT4N: return D3DDECLTYPE_USHORT4N;

	case VD_STREAM_TYPE_HALF2: return D3DDECLTYPE_FLOAT16_2;
	case VD_STREAM_TYPE_HALF4: return D3DDECLTYPE_FLOAT16_4;
	}
	ntAssert_p(0, ("Unrecognised stream type"));
	return D3DDECLTYPE_UNUSED;
}

#endif _PROC_VB
