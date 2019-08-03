//--------------------------------------------------
//!
//!	\file proc_vertexbuffer.h
//!	Class that wraps a vertex declaration and a
//! corresponding dynamic vertex buffer
//!
//--------------------------------------------------

#ifndef GFX_PROC_VB
#define GFX_PROC_VB

//--------------------------------------------------
//!
//!	Procedural Vertex Buffer slots
//!
//--------------------------------------------------
enum PROCVERTEX_ELEMENT
{
	PV_ELEMENT_0 = 0,
	PV_ELEMENT_POS = 1,
	PV_ELEMENT_2 = 2,
	PV_ELEMENT_3 = 3,
	PV_ELEMENT_4 = 4,
	PV_ELEMENT_5 = 5,
	PV_ELEMENT_6 = 6,
	PV_ELEMENT_7 = 7,
	PV_ELEMENT_8 = 8,
	PV_ELEMENT_9 = 9,
	PV_ELEMENT_10 = 10,
	PV_ELEMENT_11 = 11,
	PV_ELEMENT_12 = 12,
	PV_ELEMENT_13 = 13,
	PV_ELEMENT_14 = 14,
	PV_ELEMENT_15 = 15,

	PV_ELEMENT_MAX,		// at the moment this matches the maximum number of elements
						// in a d3d vertex declaration
};

//--------------------------------------------------
//!
//!	Procedural Vertex Buffer slots
//!
//--------------------------------------------------
#if defined( PLATFORM_PC )
#	include "gfx/proc_vertexbuffer_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "gfx/proc_vertexbuffer_ps3.h"
#endif

//--------------------------------------------------
//!
//!	Procedural Data Buffer
//!
//--------------------------------------------------
class ProceduralDB : public CNonCopyable
{
public:
	ProceduralDB() : m_pData(0) { Reset(); }
	~ProceduralDB() { Reset(); }

	void Reset();
	void FreeBuffers();
	void BuildMe( u_int iMaxVertices );
	void PushVertexElement( u_int iElementSize, u_int iSlot )
	{
		ntAssert_p( !IsSlotUsed(iSlot), ("Vertex Element not present in vertex") );

		m_iValidElements |= (1<<iSlot);
		m_aElementOffsets[iSlot] = (uint8_t)m_iVertexSize;
		m_aElementSizes[iSlot] = (uint8_t)iElementSize;

		m_iVertexSize += iElementSize;
	}

	bool IsValid() const { return (m_pData != 0); }
	bool IsSlotUsed( u_int iSlot ) const { return !!(m_iValidElements & (1 << iSlot)); }

	u_int GetVertexSize() const	{ return m_iVertexSize; }
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

protected:
	static const u_int MAX_VERTEX_ELEMENTS = PV_ELEMENT_MAX;

	uint8_t*	m_pData;
	u_int	m_iVertexSize;
	u_int	m_iMaxVertices;

	u_int	m_iValidElements;
	uint8_t	m_aElementOffsets[MAX_VERTEX_ELEMENTS];
	uint8_t	m_aElementSizes[MAX_VERTEX_ELEMENTS];
};

#endif // GFX_PROC_VB
