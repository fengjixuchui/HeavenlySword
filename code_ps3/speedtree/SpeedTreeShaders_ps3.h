//--------------------------------------------------
//!
//!	\file SpeedTreeShaders_ps3.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------



#ifndef _SPEEDTREE_VERTEXSHADERS_H_
#define _SPEEDTREE_VERTEXSHADERS_H_



///////////////////////////////////////////////////////////////////////  
//  Includes

#include "speedtree/SpeedTreeConfig_ps3.h"
#include "speedtree/SpeedTreeMaterial_ps3.h"
#include "speedtree/SpeedTreeUtil_ps3.h"

#include "gfx/proceduralMaterial_ps3.h"

#include "core/explicittemplate.h"
#include "gfx/texture.h"
#include "gfx/shaderID.h"

#include "../content_ps3/cg/SpeedTree_Defines.h"


class CMeshVertexElement;
class CMaterialProperty;
class GcStreamField;
class CMaterial;



////////////////////////////////////////////////
//  MISC

struct SpeedTreeTextures
{
    Texture::Ptr					  m_texBranchTexture;             // branch texture object
    Texture::Ptr					  m_texBranchNormalTexture;       // branch normalmap object
	Texture::Ptr					  m_texBranchDetailTexture;
	Texture::Ptr					  m_texCompositeTexture;
	Texture::Ptr					  m_texCompositeNormalTexture;
	Texture::Ptr					  m_texBillboardTexture;
	Texture::Ptr					  m_texBillboardNormalTexture;
};

struct SpeedTreeMaterials
{
    CSpeedTreeMaterial              m_cBranchMaterial;              // branch material
    CSpeedTreeMaterial              m_cLeafMaterial;                // leaf material
    CSpeedTreeMaterial              m_cFrondMaterial;               // frond material
};












////////////////////////////////////////////////
//  LEAF
class SpeedTreeLeafBuffers
{
public:
	uint16_t	m_usNumLods;                // the number of leaf LODs
	VBHandle    m_pVertexBuffers[SpeedTree::g_MaxNumLODs]; 

public:
	SpeedTreeLeafBuffers()
		:m_usNumLods(0)
	{
		// nothing
	}
	bool IsValid()
	{
		return m_usNumLods>0;
	}
	void Release()
	{
		for (unsigned short i = 0; i < m_usNumLods; ++i)
		{
			// todo MONSTERS\Frank 10/01/2006 17:03:22
		}
	}
	~SpeedTreeLeafBuffers()
	{
		// nothing
	}

	unsigned int	GetVertexFootprint();
};

struct SpeedTreeLeafVertex
{
	Vec3	m_vPosition;
	Vec3	m_fTexCoords;
	Vec4	m_windCoef;
	Vec4	m_vDimensions;
	Vec4	m_vAngles;
	Vec3	m_vNormal;

	SpeedTreeLeafVertex(unsigned int corner, const float* vPosition, const float* vNormal, const float* fTexCoords, float width, float height,
		const float* pivotPoint, const float* angleOffsets, float angleIndex, float dimming, float fWindIndex0, float fWindWeight0, float fWindIndex1, float fWindWeight1 ) 
		:m_vPosition(vPosition)
		,m_fTexCoords(fTexCoords[0], fTexCoords[1], (float)corner)
		,m_windCoef(fWindIndex0, fWindWeight0, fWindIndex1, fWindWeight1)
		,m_vDimensions(width, height, pivotPoint[0] - 0.5f, pivotPoint[1] - 0.5f)
		,m_vAngles(DegToRad(angleOffsets[0]), DegToRad(angleOffsets[1]), angleIndex, dimming)
		,m_vNormal(vNormal)
		
	{
		// nothing
		ntAssert((int)fWindIndex0 < NUM_WIND_MATRICES);
		ntAssert((int)fWindIndex1 < NUM_WIND_MATRICES);
	}

	SpeedTreeLeafVertex() {}

};












////////////////////////////////////////////////
//  FROND
//class SpeedTreeFrondBuffers
//{
//public:
//	VBHandle			m_pVertexBuffer;           // frond vertex buffer
//	CSharedArray<IBHandle> m_pIndexBuffers; // number of indexes per  LOD level
//	uint16_t		m_unNumLods;				// number of frond LODs
//public:
//	SpeedTreeFrondBuffers()
//		:m_unNumLods(0)
//	{
//		// nothing
//	}
//	bool IsValid()
//	{
//		return m_unNumLods>0;
//	}
//	void Release()
//	{
//		// todo MONSTERS\Frank 10/01/2006 17:03:22
//	}
//	~SpeedTreeFrondBuffers()
//	{
//		// nothing
//	}
//};

struct SpeedTreeFrondVertex
{
	Vec3	m_vPosition;
	Vec2	m_afTexCoords;
	Vec4	m_windCoef;           // GPU Only
	Vec3	m_vNormal;

	SpeedTreeFrondVertex(const float* vPosition, const float* vNormal, const float* afTexCoords, float fWindIndex0, float fWindWeight0, float fWindIndex1, float fWindWeight1)
		:m_vPosition(vPosition)
		,m_afTexCoords(afTexCoords)
		,m_windCoef(fWindIndex0, fWindWeight0, fWindIndex1, fWindWeight1)
		,m_vNormal(vNormal)
		//,m_windCoef(fWindIndex,fWindWeight)
	{
		// nothing
		ntAssert((int)fWindIndex0 < NUM_WIND_MATRICES);
		ntAssert((int)fWindIndex1 < NUM_WIND_MATRICES);
	}
	SpeedTreeFrondVertex()
	{
		// nothing
	}
};










////////////////////////////////////////////////
//  BRANCH
class SpeedTreeIndexedBuffers
{
public:
	VBHandle			m_pVertexBuffer;          //  vertex buffer
	IBHandle			m_pIndexBuffer;           //  index buffer
	//CSharedArray<uint16_t> m_pIndexCounts; // number of indexes per  LOD level
	uint32_t			m_IndexCounts[SpeedTree::g_MaxNumLODs][SpeedTree::g_MaxNumStrips];

public:
	SpeedTreeIndexedBuffers()
	{
		memset(m_IndexCounts, 0, sizeof(m_IndexCounts));
		// nothing
	}
	bool IsValid()
	{
		//return bool(m_pIndexCounts);
		return true;
	}
	void Release()
	{
		// todo MONSTERS\Frank 10/01/2006 17:03:22
	}
	~SpeedTreeIndexedBuffers()
	{
		// nothing
	}

	unsigned int	GetVertexFootprint();
	unsigned int	GetIndexFootprint();
};

struct SpeedTreeBranchVertex
{
	Vec3 m_vPosition;            
	Vec2 m_afTexCoords;       
	Vec4 m_windCoef;           // GPU Only
	Vec3 m_vNormal;              
	Vec3 m_vBinormal;
	Vec3 m_vTangent;
	Vec2 m_afNormalCoords;
	Vec2 m_afDetailCoords;
	//Vec2 m_afShadowCoords;    // Texture coordinates for the shadows

	SpeedTreeBranchVertex() {}
	SpeedTreeBranchVertex(const float* afPosition, const float* afNormal,
		const float* texCoords, const float* normalCoords, const float* detailCoords,
		float fWindIndex0, float fWindWeight0, float fWindIndex1, float fWindWeight1,
		const float* afBinormal, const float* afTangent) : 
			m_windCoef(fWindIndex0, fWindWeight0, fWindIndex1, fWindWeight1)
			//:m_windCoef(fWindIndex,fWindWeight)
	{
		NT_MEMCPY(m_vPosition.begin(), afPosition, 3*sizeof(float));
		NT_MEMCPY(m_afTexCoords.begin(), texCoords, 2*sizeof(float));
		NT_MEMCPY(m_vNormal.begin(), afNormal, 3*sizeof(float));
		NT_MEMCPY(m_vBinormal.begin(), afBinormal, 3*sizeof(float));
		NT_MEMCPY(m_vTangent.begin(), afTangent, 3*sizeof(float));
		//NT_MEMCPY(m_vNormal.begin(), afPosition, 3*sizeof(float));
		NT_MEMCPY(m_afNormalCoords.begin(), normalCoords, 2*sizeof(float));
		NT_MEMCPY(m_afDetailCoords.begin(), detailCoords, 2*sizeof(float));

		//NT_MEMCPY(m_afShadowCoords.begin(), afPosition, 2*sizeof(float));
		//NT_MEMCPY(m_vBinormal.begin(), afBinormal, 3*sizeof(float));
		//NT_MEMCPY(m_vTangent.begin(), afTangent, 3*sizeof(float));
	}
};









////////////////////////////////////////////////
//  BILLBOARD
//class SpeedTreeBillboardBuffers
//{
//public:
//	VBHandle		m_pVertexBufferForCamera;
//	//VBHandle		m_pVertexBufferForLight;
//public:
//	static SpeedTreeBillboardBuffers Create();
//	bool IsValid()
//	{
//		return bool(m_pVertexBufferForCamera);
//	}
//};

struct SpeedTreeBillboardVertex
{
	Vec4	m_vPosition;
	Vec4	m_vGeom;
	Vec3	m_vMiscParams;
	Vec3	m_vLightAdjust;
	//Vec3	m_vNormal;
	//uint32_t	m_vDiffuse;
	//Vec2	m_fTexCoords;
	//float	m_fClipTreshold;

	SpeedTreeBillboardVertex()
	{
		// nothing
	}
	//SpeedTreeBillboardVertex(const float* pPos,const float* pNormal, uint32_t vDiffuse, const float* pTexCoord, float fClip)
	//	:m_vPosition(pPos[0],pPos[1],pPos[2])
	//	,m_vNormal(pNormal[0],pNormal[1],pNormal[2])
	//	,m_vDiffuse(vDiffuse)
	//	,m_fTexCoords(pTexCoord[0],pTexCoord[1])
	//	,m_fClipTreshold(fClip)
	SpeedTreeBillboardVertex(const float* pos, float cornerIndex, float width, float height, float azimuth, float fade, unsigned int numImages)
		: m_vPosition(pos[0], pos[1], pos[2], cornerIndex)
		, m_vGeom(width, height, azimuth, fade)
		, m_vMiscParams(0, 0, (float)numImages)
	{
	}
};

namespace SpeedTreeBillboard
{
	class CCell;
}

class CSpeedTreeBillboardBuffers
{
	struct CBillboardBuffer
	{
		CBillboardBuffer()
			: m_cell(NULL)
		{
		}

		VBHandle					m_vertexBuffer;
		SpeedTreeBillboard::CCell*	m_cell;
	};


public:
	static const unsigned int c_numBuffers = 10;

	//void CreateBuffers(unsigned int numElements)
	CSpeedTreeBillboardBuffers(unsigned int numElements);

	void SetAvailable(int bufferIndex)
	{
		ntAssert((unsigned int)bufferIndex < c_numBuffers);
		ntAssert(m_bufferPool[bufferIndex].m_cell);
		m_bufferPool[bufferIndex].m_cell = NULL;
	}

	bool IsAvailable(int bufferIndex) const
	{
		ntAssert((unsigned int)bufferIndex < c_numBuffers);
		ntAssert(m_bufferPool[bufferIndex].m_cell);
		return NULL == m_bufferPool[bufferIndex].m_cell;
	}

	void SetToCell(int bufferIndex, SpeedTreeBillboard::CCell* cell)
	{
		ntAssert((unsigned int)bufferIndex < c_numBuffers);
		ntAssert(!m_bufferPool[bufferIndex].m_cell);

		m_bufferPool[bufferIndex].m_cell = cell;

		// Now fill the buffer with data
		// May be better postpone it until the rendering actually takes place from this buffer???
		

	}

	static void Destroy();

	unsigned int SubmitVertexData(int bufferIndex, unsigned int treeIndex);
	VBHandle	 GetBuffer(int bufferIndex);


	CBillboardBuffer	m_bufferPool[c_numBuffers];

	static SpeedTreeBillboardVertex*	m_scratchPad;
	static unsigned int					m_scratchSize;

};












#endif // end of _SPEEDTREE_VERTEXSHADERS_H_
