///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper Class
//
//  (c) 2003 IDV, Inc.
//
//  This class is provided to illustrate one way to incorporate
//  SpeedTreeRT into an OpenGL application.  All of the SpeedTreeRT
//  calls that must be made on a per tree basis are done by this class.
//  Calls that apply to all trees (i.e. static SpeedTreeRT functions)
//  are made in the functions in main.cpp.
//
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com
//

///////////////////////////////////////////////////////////////////////  
//  Include Files

#include <stdlib.h>
#include <stdio.h>
#include "SpeedTreeVertexShaders.h"
#include "SpeedTreeWrapper.h"
#include "SpeedTreeUtil.h"
#include "SpeedTreeUtil.h"
#include "gfx/texturemanager.h"
#include "gfx/graphicsdevice.h"
#include "gfx/fxmaterial.h"
#include "core/util.h"
#include "speedTreeManager.h"
#include "gfx/renderer.h"
#include "speedtree/SpeedTreeRenderable.h"
#include "anim/transform.h"
#include "speedtree/SpeedTreeForest.h"
#include "gfx/sector.h"



///////////////////////////////////////////////////////////////////////  
//  Color conversion macro

#define AGBR2ARGB(dwColor) (dwColor & 0xff00ff00) + ((dwColor & 0x00ff0000) >> 16) + ((dwColor & 0x000000ff) << 16)

std::string NoExt(const std::string& filename)
{
	return std::string( Util::NoExtension(filename.c_str()) );
}

std::string TextureDirname(const std::string& filename)
{
	return filename.substr(5,filename.size()-5);
}




void CSpeedTreeWrapper::SetRenderable(Transform* pTransform)
{
	m_pTransform = pTransform;
	
	const float* pPositionAux = m_pSpeedTree->GetTreePosition();
	CPoint position(pPositionAux[0],pPositionAux[1],pPositionAux[2]);
	m_pTransform->SetLocalMatrix(CMatrix(CQuat(CONSTRUCT_IDENTITY),position));

	m_pSpeedTree->GetBoundingBox(m_boundingBox);
	//m_boundingBox[0] += position.X();
	//m_boundingBox[1] += position.Y();
	//m_boundingBox[2] += position.Z();

	//m_boundingBox[3] += position.X();
	//m_boundingBox[4] += position.Y();
	//m_boundingBox[5] += position.Z();

	m_renderables[LEAVES] = NT_NEW  RenderableSpeedtreeLeaf(this);
	m_renderables[FROND] = NT_NEW  RenderableSpeedtreeFrond(this);
	m_renderables[BRANCH] = NT_NEW  RenderableSpeedtreeBranch(this);
	m_renderables[BILLBOARD] = NT_NEW  RenderableSpeedtreeBillboard(this);
	
	for(int iIndex = 0 ; iIndex < 4 ; ++iIndex )
	{
		CSector::Get().GetRenderables().AddRenderable(m_renderables[iIndex]);
	}
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::MakeInstance

CSpeedTreeWrapper* CSpeedTreeWrapper::MakeInstance(void)
{
    // create a new object
    CSpeedTreeWrapper* pInstance = NT_NEW  CSpeedTreeWrapper( m_pSpeedTree->MakeInstance(), m_pForest );

    // make an instance of this object's SpeedTree
    if (pInstance->m_pSpeedTree)
    {
        // use the same materials
		pInstance->m_materials = m_materials;
		pInstance->m_textures = m_textures;
		pInstance->m_pTextureInfo = m_pTextureInfo;

        // use the same geometry cache
        pInstance->m_pGeometryCache = m_pGeometryCache;
        
        // use the same buffers
        pInstance->m_branchBuffers = m_branchBuffers;
        pInstance->m_frondBuffers = m_frondBuffers;
        pInstance->m_leafBuffers = m_leafBuffers;

        // new stuff
        memcpy(pInstance->m_afBoundingBox, m_afBoundingBox, 6 * sizeof(float));
        pInstance->m_pInstanceOf = this;
        //m_vInstances.push_back(pInstance);
    }
    else
    {
        ntPrintf( "SpeedTreeRT Error: %s\n", m_pSpeedTree->GetCurrentError( ));
        NT_DELETE( pInstance );
        pInstance = NULL;
    }

    return pInstance;
}


SpeedTreeStat CSpeedTreeWrapper::GetStat()
{
	return SpeedTreeStat(
		m_renderables[LEAVES]->GetTriangleCount(),
		m_renderables[FROND]->GetTriangleCount(),
		m_renderables[BRANCH]->GetTriangleCount(),
		m_renderables[BILLBOARD]->GetTriangleCount());
}

unsigned long CSpeedTreeWrapper::GetRenderBitVector()
{
	return m_ulRenderBitVector & m_pForest->GetRenderBitVector();
}



///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::CSpeedTreeWrapper

CSpeedTreeWrapper::CSpeedTreeWrapper(CSpeedTreeRT* pTree, CSpeedTreeForest* pForest)
	:m_pSpeedTree(pTree)
    ,m_pGeometryCache(NULL)
	,m_pForest(pForest)
	,m_pTransform(0)
	,m_ulRenderBitVector(Forest_RenderAll)
	,m_pInstanceOf(0)
{
    //m_afPos[0] = m_afPos[1] = m_afPos[2] = 0.0f;
    //m_unNumWrappersActive++;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::~CSpeedTreeWrapper

CSpeedTreeWrapper::~CSpeedTreeWrapper( )
{
	for(int iIndex = 0 ; iIndex < 4 ; ++iIndex )
	{
		CSector::Get().GetRenderables().RemoveRenderable(m_renderables[iIndex]);
		NT_DELETE( m_renderables[iIndex] );
	}

	if(!m_pInstanceOf)
	{
		SAFE_DELETE(m_pTextureInfo);
		m_branchBuffers.Release();
		m_frondBuffers.Release();
		m_leafBuffers.Release();
	}

	SAFE_DELETE(m_pSpeedTree);
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::LoadTree

bool CSpeedTreeWrapper::LoadTree(const std::string& pszSptFile, unsigned int nSeed, float fSize, float fSizeVariance)
{
    bool bSuccess = false;

    // directx, so allow for flipping of the texture coordinate
    #ifdef WRAPPER_FLIP_T_TEXCOORD
        m_pSpeedTree->SetTextureFlip(true);
    #endif

    // load the tree file
		if (m_pSpeedTree->LoadTree( pszSptFile.c_str() ) )
    {
        // override the lighting method stored in the spt file
        m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
        m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
        m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);

        // set the wind method
        m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_GPU);
        m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_GPU);
        m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_GPU);

        m_pSpeedTree->SetNumLeafRockingGroups(1);

        // override the size, if necessary
        if (fSize >= 0.0f && fSizeVariance >= 0.0f)
            m_pSpeedTree->SetTreeSize(fSize, fSizeVariance);

        // generate tree geometry
        if (m_pSpeedTree->Compute(NULL, nSeed))
        {
            // get the dimensions
            m_pSpeedTree->GetBoundingBox(m_afBoundingBox);
        
            // make the leaves rock in the wind
            m_pSpeedTree->SetLeafRockingState(true);

            // billboard setup
            #ifdef WRAPPER_BILLBOARD_MODE
                CSpeedTreeRT::SetDropToBillboard(true);
            #else
                CSpeedTreeRT::SetDropToBillboard(false);
            #endif

            // query & set materials
            m_materials.m_cBranchMaterial.Set(m_pSpeedTree->GetBranchMaterial( ));
            m_materials.m_cFrondMaterial.Set(m_pSpeedTree->GetFrondMaterial( ));
            m_materials.m_cLeafMaterial.Set(m_pSpeedTree->GetLeafMaterial( ));

            // adjust lod distances
            float fHeight = m_afBoundingBox[5] - m_afBoundingBox[2];
            m_pSpeedTree->SetLodLimits(fHeight * c_fNearLodFactor, fHeight * c_fFarLodFactor);

            // query textures
            m_pTextureInfo = NT_NEW  CSpeedTreeRT::STextures;
            m_pSpeedTree->GetTextures(*m_pTextureInfo);

            // load branch textures
            std::string dirname = TextureDirname((std::string( Util::DirName(pszSptFile.c_str()) ) + "/"));
            m_textures.m_texBranchTexture = TextureManager::Get().LoadTexture_Neutral( (dirname+NoExt(m_pTextureInfo->m_pBranchTextureFilename)+".dds").c_str() );
			m_textures.m_texBranchNormalTexture = TextureManager::Get().LoadTexture_Neutral( (dirname+NoExt(m_pTextureInfo->m_pBranchTextureFilename)+"Normal.dds").c_str() );
            if (m_pTextureInfo->m_pSelfShadowFilename != NULL && m_textures.m_texShadow == NULL)
            {
            	m_textures.m_texShadow = TextureManager::Get().LoadTexture_Neutral( (dirname+NoExt(m_pTextureInfo->m_pSelfShadowFilename)+".dds").c_str() );
            }
			m_textures.m_texCompositeMap = TextureManager::Get().LoadTexture_Neutral( c_pCompositeMapFilename );

            // setup the vertex and index buffers
            SetupBuffers( );

            // everything appeared to go well
            bSuccess = true;
        }
        else // tree failed to compute
            ntPrintf( "\nFatal Error, cannot compute tree [%s]\n\n", CSpeedTreeRT::GetCurrentError( ));

    }
    else // tree failed to load
        ntPrintf( "SpeedTreeRT Error: %s\n", CSpeedTreeRT::GetCurrentError( ));

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupBuffers

void CSpeedTreeWrapper::SetupBuffers(void)
{
    // read all the geometry for highest LOD into the geometry cache
    m_pSpeedTree->SetLodLevel(1.0f);
    if (m_pGeometryCache == NULL)
        m_pGeometryCache = NT_NEW  CSpeedTreeRT::SGeometry;
    m_pSpeedTree->GetGeometry(*m_pGeometryCache);

    // setup the buffers for each tree part
    SetupBranchBuffers( );
    SetupFrondBuffers( );
    SetupLeafBuffers( );
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupBranchBuffers

void CSpeedTreeWrapper::SetupBranchBuffers(void)
{
    // reference to the branch structure
    CSpeedTreeRT::SGeometry::SIndexed* pBranches = &(m_pGeometryCache->m_sBranches);
    m_branchBuffers.m_unBranchVertexCount = pBranches->m_usVertexCount; // we asked for a contiguous strip

    // check if this tree has branches
    if (m_branchBuffers.m_unBranchVertexCount > 1)
    {
        // create the vertex buffer for storing branch vertices
        GetD3DDevice()->CreateVertexBuffer(m_branchBuffers.m_unBranchVertexCount * sizeof(SFVFBranchVertex),
			D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_DEFAULT, &m_branchBuffers.m_pBranchVertexBuffer, NULL);

        // fill the vertex buffer by interleaving SpeedTree data
        SFVFBranchVertex* pVertexBuffer = NULL;
        m_branchBuffers.m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
        for (unsigned int i = 0; i < m_branchBuffers.m_unBranchVertexCount; ++i)
        {
            // position
            memcpy(&pVertexBuffer->m_vPosition, &(pBranches->m_pCoords[i * 3]), 3 * sizeof(float));

            // normal
            memcpy(&pVertexBuffer->m_vNormal, &(pBranches->m_pNormals[i * 3]), 3 * sizeof(float));

            // texcoords for layer 0
            pVertexBuffer->m_afTexCoords[0] = pBranches->m_pTexCoords0[i * 2];
            pVertexBuffer->m_afTexCoords[1] = pBranches->m_pTexCoords0[i * 2 + 1];

            // texcoords for layer 1 (if enabled)
            pVertexBuffer->m_afShadowCoords[0] = pBranches->m_pTexCoords1[i * 2];
            pVertexBuffer->m_afShadowCoords[1] = pBranches->m_pTexCoords1[i * 2 + 1];

            // gpu wind data
            pVertexBuffer->m_fWindIndex = pBranches->m_pWindMatrixIndices[i];
            pVertexBuffer->m_fWindWeight = (float)pBranches->m_pWindWeights[i];

            // tangents and binormals
            memcpy(&pVertexBuffer->m_vBinormal, &(pBranches->m_pBinormals[i * 3]), 3 * sizeof(float));
            memcpy(&pVertexBuffer->m_vTangent, &(pBranches->m_pTangents[i * 3]), 3 * sizeof(float));

            ++pVertexBuffer;
        }
        m_branchBuffers.m_pBranchVertexBuffer->Unlock( );

        // create and fill the index counts for each LOD
        unsigned int unNumLodLevels = m_pSpeedTree->GetNumBranchLodLevels( );
        m_branchBuffers.m_pBranchIndexCounts = NT_NEW  unsigned short[unNumLodLevels];
        for (unsigned int i = 0; i < unNumLodLevels; ++i)
        {
            // force geometry update for this LOD
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, (short)i);

            // check if this LOD has branches
            if (pBranches->m_usNumStrips > 0)
                m_branchBuffers.m_pBranchIndexCounts[i] = pBranches->m_pStripLengths[0];
            else
                m_branchBuffers.m_pBranchIndexCounts[i] = 0;
        }
        // force update of geometry to highest LOD
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, 0);

        // the first LOD level contains the most indices of all the levels, so
        // we use its size to allocate the index buffer
        GetD3DDevice()->CreateIndexBuffer(m_branchBuffers.m_pBranchIndexCounts[0] * sizeof(unsigned short),
			D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_branchBuffers.m_pBranchIndexBuffer, NULL);

        // fill the index buffer
        unsigned short* pIndexBuffer = NULL;
        m_branchBuffers.m_pBranchIndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pIndexBuffer), 0);
        memcpy(pIndexBuffer, pBranches->m_pStrips[0], pBranches->m_pStripLengths[0] * sizeof(unsigned short));
        m_branchBuffers.m_pBranchIndexBuffer->Unlock( );
    }
}

///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupFrondBuffers

void CSpeedTreeWrapper::SetupFrondBuffers(void)
{
    // reference to frond structure 
    CSpeedTreeRT::SGeometry::SIndexed* pFronds = &(m_pGeometryCache->m_sFronds);
    m_frondBuffers.m_unFrondVertexCount = pFronds->m_usVertexCount; // we asked for a contiguous strip

    // check if this tree has fronds
    if (m_frondBuffers.m_unFrondVertexCount > 1)
    {
        // create the vertex buffer for storing frond vertices
        GetD3DDevice()->CreateVertexBuffer(m_frondBuffers.m_unFrondVertexCount * sizeof(SFVFFrondVertex),
			D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_FROND_VERTEX, D3DPOOL_DEFAULT, &m_frondBuffers.m_pFrondVertexBuffer, NULL);
        
        // fill the vertex buffer by interleaving SpeedTree data
        SFVFFrondVertex* pVertexBuffer = NULL;
        m_frondBuffers.m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
        for (unsigned short i = 0; i < m_frondBuffers.m_unFrondVertexCount; ++i)
        {
            // position
            memcpy(&pVertexBuffer->m_vPosition, &(pFronds->m_pCoords[i * 3]), 3 * sizeof(float));

            // normal or color
            memcpy(&pVertexBuffer->m_vNormal, &(pFronds->m_pNormals[i * 3]), 3 * sizeof(float));

            // texcoords for layer 0
            pVertexBuffer->m_afTexCoords[0] = pFronds->m_pTexCoords0[i * 2];
            pVertexBuffer->m_afTexCoords[1] = pFronds->m_pTexCoords0[i * 2 + 1];

            // texcoords for layer 1 (if enabled)
            pVertexBuffer->m_afShadowCoords[0] = pFronds->m_pTexCoords1[i * 2];
            pVertexBuffer->m_afShadowCoords[1] = pFronds->m_pTexCoords1[i * 2 + 1];
        
            // gpu wind data
            pVertexBuffer->m_fWindIndex = pFronds->m_pWindMatrixIndices[i];
            pVertexBuffer->m_fWindWeight = pFronds->m_pWindWeights[i];

            ++pVertexBuffer;
        }
        m_frondBuffers.m_pFrondVertexBuffer->Unlock( );

        // create and fill the index counts for each LOD
        m_frondBuffers.m_unNumFrondLods = m_pSpeedTree->GetNumFrondLodLevels( );
        m_frondBuffers.m_pFrondIndexCounts = NT_NEW  unsigned short[m_frondBuffers.m_unNumFrondLods];
		m_frondBuffers.m_pFrondIndexBuffers = NT_NEW  LPDIRECT3DINDEXBUFFER9[m_frondBuffers.m_unNumFrondLods];

        for (unsigned int i = 0; i < m_frondBuffers.m_unNumFrondLods; ++i)
        {
            // force update of geometry for this LOD
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, static_cast<short>(i));

            // check if this LOD has fronds
            if (pFronds->m_usNumStrips > 0)
                m_frondBuffers.m_pFrondIndexCounts[i] = pFronds->m_pStripLengths[0];
            else
                m_frondBuffers.m_pFrondIndexCounts[i] = 0;

			if (m_frondBuffers.m_pFrondIndexCounts[i] > 0)
			{
				GetD3DDevice()->CreateIndexBuffer(m_frondBuffers.m_pFrondIndexCounts[i] * sizeof(unsigned short),
					D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_frondBuffers.m_pFrondIndexBuffers[i], NULL);
				
				// fill the index buffer
				unsigned short* pIndexBuffer = NULL;
				m_frondBuffers.m_pFrondIndexBuffers[i]->Lock(0, 0, reinterpret_cast<void**>(&pIndexBuffer), 0);
				memcpy(pIndexBuffer, pFronds->m_pStrips[0], m_frondBuffers.m_pFrondIndexCounts[i] * sizeof(unsigned short));
				m_frondBuffers.m_pFrondIndexBuffers[i]->Unlock( );
			}
        }
        // force update of geometry to highest LOD
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, 0);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupLeafBuffers

void CSpeedTreeWrapper::SetupLeafBuffers(void)
{
    // set up constants
    const short anVertexIndices[6] = { 0, 1, 2, 0, 2, 3 };

    // set up the leaf counts for each LOD
    m_leafBuffers.m_usNumLeafLods = m_pSpeedTree->GetNumLeafLodLevels( );

    // create arrays for the number of LOD levels
    m_leafBuffers.m_pLeafVertexBuffer = NT_NEW LPDIRECT3DVERTEXBUFFER9[m_leafBuffers.m_usNumLeafLods];

    for (unsigned int unLod = 0; unLod < m_leafBuffers.m_usNumLeafLods; ++unLod)
    {
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, static_cast<short>(unLod));

        // if this level has no leaves, then skip it
        unsigned short usLeafCount = m_pGeometryCache->m_sLeaves0.m_usLeafCount;
        if (usLeafCount < 1)
            continue;

        // create the vertex buffer for storing leaf vertices
        GetD3DDevice()->CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex),
			D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_LEAF_VERTEX, D3DPOOL_DEFAULT, &m_leafBuffers.m_pLeafVertexBuffer[unLod], NULL);
    
        // fill the vertex buffer by interleaving SpeedTree data
        SFVFLeafVertex* pVertexBuffer = NULL;
        m_leafBuffers.m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
        SFVFLeafVertex* pVertex = pVertexBuffer;
        for (unsigned int unLeaf = 0; unLeaf < usLeafCount; ++unLeaf)
        {
            const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = &(m_pGeometryCache->m_sLeaves0);
            for (unsigned int unVert = 0; unVert < 6; ++unVert)  // 6 verts == 2 triangles
            {
                // position
                memcpy(pVertex->m_vPosition, &(pLeaf->m_pCenterCoords[unLeaf * 3]), 3 * sizeof(float));

                // normal
                memcpy(&pVertex->m_vNormal, &(pLeaf->m_pNormals[unLeaf * 3]), 3 * sizeof(float));

                // tex coord
                memcpy(pVertex->m_fTexCoords, &(pLeaf->m_pLeafMapTexCoords[unLeaf][anVertexIndices[unVert] * 2]), 2 * sizeof(float));

                // wind weights
                pVertex->m_fWindIndex = pLeaf->m_pWindMatrixIndices[unLeaf];
                pVertex->m_fWindWeight = pLeaf->m_pWindWeights[unLeaf];

                // gpu leaf placement data
                pVertex->m_fLeafPlacementIndex = pLeaf->m_pLeafClusterIndices[unLeaf] * 4.0f + anVertexIndices[unVert];
                pVertex->m_fLeafScalarValue = m_pSpeedTree->GetLeafLodSizeAdjustments( )[unLod];
                pVertex->m_fLeafRockIndex = (float)(pLeaf->m_pLeafClusterIndices[unLeaf] % c_nNumWindMatrices);

                ++pVertex;
            }
            
        }
        m_leafBuffers.m_pLeafVertexBuffer[unLod]->Unlock( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::Advance

void CSpeedTreeWrapper::Advance(void)
{
    // compute LOD level (based on distance from camera)
    m_pSpeedTree->ComputeLodLevel( );
}

CSpeedTreeRT::SGeometry* CSpeedTreeWrapper::UpdateGeometryCache(
	unsigned long ulBitVector,
	short sOverrideBranchLodValue,
	short sOverrideFrondLodValue,
	short sOverrideLeafLodValue)
{
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, ulBitVector,sOverrideBranchLodValue,sOverrideFrondLodValue,sOverrideLeafLodValue);
	return m_pGeometryCache;
}





///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::CleanUpMemory

void CSpeedTreeWrapper::CleanUpMemory(void)
{
    //if (!m_bIsInstance)
    //    m_pSpeedTree->DeleteTransientData( );
	m_pSpeedTree->DeleteTransientData( );
}

