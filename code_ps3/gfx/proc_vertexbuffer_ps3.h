//--------------------------------------------------
//!
//!	\file proc_vertexbuffer_ps3.h
//!	Class that wraps a vertex declaration and a
//! corresponding dynamic vertex buffer
//!
//--------------------------------------------------

#ifndef GFX_PROC_VB_PS3
#define GFX_PROC_VB_PS3

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
	void PushVertexElement( VERTEX_DECL_STREAM_TYPE type, u_int iSlot, const char* pBindName )
	{
		ntAssert_p( m_iCurrElement < MAX_VERTEX_ELEMENTS, ("Added too many vertex elements") );
		ntAssert_p( iSlot < MAX_VERTEX_ELEMENTS, ("invalid slot for vertex element") );
		ntAssert_p( !IsSlotUsed(iSlot), ("Added too many vertex elements") );
		ntAssert_p( pBindName, ("MUST provide a bind name for this element on PS3") );

		m_aVertexElements[ m_iCurrElement ].m_type = type;
		m_aVertexElements[ m_iCurrElement ].m_bindNameHash = FwHashedString( pBindName ).Get();

		m_iCurrElement++;

		m_iValidElements |= (1<<iSlot);
		m_aElementOffsets[iSlot] = (uint8_t)m_iVertexSize;
		m_aElementSizes[iSlot] = (uint8_t)GetSizeOfElement( type );

		m_iVertexSize += GetSizeOfElement( type );
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

	bool CanSubmitToGPU( u_int iNumVerts = 0xffffffff );
	void SubmitToGPU( u_int iNumVerts = 0xffffffff );

	// alloc a new discard buffer so we can write explicitly
	void LockForWrite( u_int iNumVerts = 0xffffffff );
	void Write( const uint8_t* pData, uint32_t offset, uint32_t size );
	void UnlockAndSubmit();

private:
	static const u_int MAX_VERTEX_ELEMENTS = PV_ELEMENT_MAX;

	// construction data
	struct StreamElement
	{
		VERTEX_DECL_STREAM_TYPE m_type;
		u_int					m_bindNameHash;
	};

	StreamElement m_aVertexElements[MAX_VERTEX_ELEMENTS];
	u_int m_iCurrElement;

	// runtime data
	VBHandle			m_pVB;
	uint8_t*			m_pData;

	u_int				m_iVertexSize;
	u_int				m_iMaxVertices;

	u_int				m_iValidElements;
	uint8_t				m_aElementOffsets[MAX_VERTEX_ELEMENTS];
	uint8_t				m_aElementSizes[MAX_VERTEX_ELEMENTS];

	bool				m_scratchAllocated;
};

#endif // GFX_PROC_VB_PS3
