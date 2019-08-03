#ifndef _SPEEDTREEBUFFER_H_
#define _SPEEDTREEBUFFER_H_

//--------------------------------------------------
//!
//!	\file SpeedTreeBuffer.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------



#include "SpeedTreeConfig.h"
#include "SpeedTreeMaterial.h"
#include "SpeedTreeUtil.h"

// branch geometry
class SpeedTreeBranchBuffers
{
public:
	LPDIRECT3DVERTEXBUFFER9         m_pBranchVertexBuffer;          // branch vertex buffer
	unsigned int                    m_unBranchVertexCount;          // number of vertices in branches
	LPDIRECT3DINDEXBUFFER9          m_pBranchIndexBuffer;           // branch index buffer
	unsigned short*                 m_pBranchIndexCounts;           // number of indexes per branch LOD level
public:
	SpeedTreeBranchBuffers()
		:m_pBranchVertexBuffer(0)
		,m_unBranchVertexCount(0)
		,m_pBranchIndexBuffer(0)
		,m_pBranchIndexCounts(0)
	{
		// nothing
	}
	void Release()
	{
		SAFE_RELEASE(m_pBranchVertexBuffer);
		SAFE_RELEASE(m_pBranchIndexBuffer);
		SAFE_DELETE_ARRAY(m_pBranchIndexCounts);
	}
	~SpeedTreeBranchBuffers()
	{
		// nothing
	}
};

// frond geometry
class SpeedTreeFrondBuffers
{
public:
	LPDIRECT3DVERTEXBUFFER9         m_pFrondVertexBuffer;           // frond vertex buffer
	unsigned int                    m_unFrondVertexCount;           // number of vertices in frond
	LPDIRECT3DINDEXBUFFER9*         m_pFrondIndexBuffers;           // frond index buffer
	unsigned short*                 m_pFrondIndexCounts;            // number of indexes per frond LOD level
	unsigned int					m_unNumFrondLods;				// number of frond LODs
public:
	SpeedTreeFrondBuffers()
		:m_pFrondVertexBuffer(0)
		,m_unFrondVertexCount(0)
		,m_pFrondIndexBuffers(0)
		,m_pFrondIndexCounts(0)
		,m_unNumFrondLods(0)
	{
		// nothing
	}
	void Release()
	{
		SAFE_RELEASE(m_pFrondVertexBuffer);
		for (unsigned int i = 0; i < m_unNumFrondLods; ++i)
			if (m_pFrondIndexCounts[i] > 0)
				SAFE_RELEASE(m_pFrondIndexBuffers[i]);
		SAFE_DELETE_ARRAY(m_pFrondIndexBuffers);
		SAFE_DELETE_ARRAY(m_pFrondIndexCounts);
	}
	~SpeedTreeFrondBuffers()
	{
		// nothing
	}
};

// leaf geometry
class SpeedTreeLeafBuffers
{
public:
	unsigned short                  m_usNumLeafLods;                // the number of leaf LODs
	LPDIRECT3DVERTEXBUFFER9*        m_pLeafVertexBuffer;            // leaf vertex buffer
public:
	SpeedTreeLeafBuffers()
		:m_usNumLeafLods(0)
		,m_pLeafVertexBuffer(0)
	{
		// nothing
	}
	void Release()
	{
		for (unsigned short i = 0; i < m_usNumLeafLods; ++i)
		{
			// suppose to check if there's any allocation in thid lod before
			SAFE_RELEASE(m_pLeafVertexBuffer[i]);
		}
	}
	~SpeedTreeLeafBuffers()
	{
		// nothing
	}
};

struct SpeedTreeTextures
{
    Texture::Ptr					  m_texBranchTexture;             // branch texture object
    Texture::Ptr					  m_texBranchNormalTexture;       // branch normalmap object
	Texture::Ptr					  m_texShadow;                    // shadow texture object
	Texture::Ptr					  m_texCompositeMap;                    // shadow texture object
};

struct SpeedTreeMaterials
{
    CSpeedTreeMaterial              m_cBranchMaterial;              // branch material
    CSpeedTreeMaterial              m_cLeafMaterial;                // leaf material
    CSpeedTreeMaterial              m_cFrondMaterial;               // frond material
};


#endif // end of _SPEEDTREEBUFFER_H_
