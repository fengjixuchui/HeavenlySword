//--------------------------------------------------
//!
//!	\file quadlist.h
//!	Class that wraps all nessecary things for a dynamic
//! quad list.
//!
//--------------------------------------------------

#ifndef _QUAD_LIST_H
#define _QUAD_LIST_H

#include "gfx/proc_vertexbuffer.h"

//--------------------------------------------------
//!
//!	QuadList
//! Base class for quad list objects
//!
//--------------------------------------------------
class QuadList
{
public:
	QuadList() : m_bValid(false) {};
	virtual ~QuadList() {};

	virtual void	ReleaseResources();
	virtual void	Initialise( u_int iNumQuads, bool bUsePointSprites, bool bUsingTriLists = true );
	virtual void	Render( u_int iNumToRender = 0xffffffff );

	ProceduralVB&	GetGPUData() { return m_VB; }
	ProceduralDB&	GetCPUData() { return m_DB; }

	bool	UsingPointSprites() const	{ return m_bUsingPointSprites; }
	u_int	GetNumQuads() const			{ return m_iNumQuads; }
	u_int	GetVertsPerQuad() const		{ return m_iVertsPerQuad; }

	inline void	SetGPUQuadInfo( u_int iIndex, void* pData, u_int iSizeOfData );

protected:
	bool			m_bValid;
	bool			m_bUsingPointSprites;

	ProceduralVB	m_VB; // GPU per-vertex data
	ProceduralDB	m_DB; // CPU per-particle data

#ifdef PLATFORM_PC
	void			DrawQuads( u_int iNumToRender );

	bool			m_bUsingTriLists;
	IBHandle		m_IB; // GPU index buffer
	u_int			m_iNumIndices;
#endif

	u_int			m_iNumQuads;
	u_int			m_iVertsPerQuad;
};

//--------------------------------------------------
//!
//!	QuadList::SetGPUQuadInfo()
//!
//--------------------------------------------------
inline void QuadList::SetGPUQuadInfo( u_int iIndex, void* pData, u_int iSizeOfData )
{
	ntAssert( iSizeOfData <= m_VB.GetVertexSize() );
	ntAssert( iIndex < m_iNumQuads );
	
	u_int iVertIndex = iIndex * m_iVertsPerQuad;

	for ( u_int i = 0; i < m_iVertsPerQuad; i++ )
		NT_MEMCPY( m_VB.GetVertex( iVertIndex + i ), pData, iSizeOfData );
}




//--------------------------------------------------
//!
//! QuadListSorted
//! Convient wrapper class for drawing rects.
//!
//--------------------------------------------------
class QuadListSorted : public QuadList
{
public:
	virtual ~QuadListSorted() { ReleaseResources(); }

	virtual void	ReleaseResources();
	virtual void	Initialise( u_int iNumQuads, bool bUsePointSprites, bool bUsingTriLists = true );
	virtual void	Render( u_int iNumToRender = 0xffffffff );

private:
	struct SortNode
	{
		float m_fDistanceSq;
		int m_iParticleIndex;
	};

	class comparatorFurtherThan
	{
	public:
		bool operator()( const SortNode& first, const SortNode& second ) const
		{
			return (first.m_fDistanceSq > second.m_fDistanceSq);
		}
	};

	SortNode* m_pSortList;
};

#endif
