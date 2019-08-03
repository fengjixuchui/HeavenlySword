#include "SpeedTreeRenderable.h"


#include "speedtree/SpeedTreeWrapper.h"
#include "anim/transform.h"
#include "gfx/renderersettings.h"
#include "gfx/rendercontext.h"
#include "gfx/fxmaterial.h"
#include "speedtree/SpeedTreeUtil.h"
#include "gfx/renderer.h"
#include "speedtree/SpeedTreeManager.h"
#include "speedtree/SpeedTreeVertexShaders.h"
#include "speedtree/SpeedTreeForest.h"
#include "core/gatso.h"

/////////////////////////////////////
//RenderableSpeedTree

















	//! constructor
RenderableSpeedTree::RenderableSpeedTree(CSpeedTreeWrapper* pTreeInForest, FXMaterial* pMaterial)
	:CRenderable(pTreeInForest->GetTransform(),true,true,true)
	,m_pTreeWrapper(pTreeInForest)
	,m_pMaterial(pMaterial)
{
	SetBoundingBox();
}

//! render depths for z pre-pass
void RenderableSpeedTree::RenderDepth()
{
	CGatso::Start( "___RenderableSpeedTree::RenderDepth" );
	if(!PreRender()) return;
	m_triangleCount[DEPTH] = SendGeometry("depth_output");
	PostRender();
	CGatso::Stop( "___RenderableSpeedTree::RenderDepth" );
}

//! Renders the game material for this renderable.
void RenderableSpeedTree::RenderMaterial()
{
	CGatso::Start( "___RenderableSpeedTree::RenderMaterial" );
	if(!PreRender()) return;
	if(CRendererSettings::bEnableShadows)
	{
		m_triangleCount[MATERIAL] = SendGeometry("getshadowterm");
	}
	else
	{
		m_triangleCount[MATERIAL] = SendGeometry("notshadowed");
	}
	PostRender();
	CGatso::Stop( "___RenderableSpeedTree::RenderMaterial" );
}
//! Renders the shadow map depths.
void RenderableSpeedTree::RenderShadowMap()
{
	CGatso::Start( "___RenderableSpeedTree::RenderShadowMap" );
	if(!PreRender()) return;
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP0 )
	{
		CMatrix worldViewProjShadow = m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[0];
		VERIFY(m_pMaterial->GetEffect()->SetMatrix ("m_worldViewProj", reinterpret_cast<const D3DXMATRIX*>( &worldViewProjShadow ) ) );  
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[0] );
		m_triangleCount[SHADOW_MAP] = SendGeometry("depth_output");
	}
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP1 )
	{
		CMatrix worldViewProjShadow = m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[1];
		VERIFY(m_pMaterial->GetEffect()->SetMatrix ("m_worldViewProj", reinterpret_cast<const D3DXMATRIX*>( &worldViewProjShadow ) ) );  
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[1] );
		m_triangleCount[SHADOW_MAP] = SendGeometry("depth_output");
	}
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP2 )
	{
		CMatrix worldViewProjShadow = m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[2];
		VERIFY(m_pMaterial->GetEffect()->SetMatrix ("m_worldViewProj", reinterpret_cast<const D3DXMATRIX*>( &worldViewProjShadow ) ) );  
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[2] );
		m_triangleCount[SHADOW_MAP] = SendGeometry("depth_output");
	}
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP3 )
	{
		CMatrix worldViewProjShadow = m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[3];
		VERIFY(m_pMaterial->GetEffect()->SetMatrix ("m_worldViewProj", reinterpret_cast<const D3DXMATRIX*>( &worldViewProjShadow ) ) );  
		Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[3] );
		m_triangleCount[SHADOW_MAP] = SendGeometry("depth_output");
	}
	PostRender();
	CGatso::Stop( "___RenderableSpeedTree::RenderShadowMap" );
}
//! Renders with a shadow map compare only. 
void RenderableSpeedTree::RenderShadowOnly()
{
	CGatso::Start( "___RenderableSpeedTree::RenderShadowOnly" );
	if(!PreRender()) return;
	m_triangleCount[RECEIVE_SHADOW] = SendGeometry("recieve_shadow");
	PostRender();
	CGatso::Stop( "___RenderableSpeedTree::RenderShadowOnly" );
}

void RenderableSpeedTree::PostRender()
{
	GetD3DDevice()->SetIndices( 0 );
	GetD3DDevice()->SetStreamSource( 0, 0, 0, 0 );
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL);
}

// set the bounding box according to m_pTreeWrapper
void RenderableSpeedTree::SetBoundingBox()
{
	const float* pBB = m_pTreeWrapper->GetBoundingBox();
	m_obBounds = CAABB(CPoint(pBB[0],pBB[1],pBB[2]), CPoint(pBB[3],pBB[4],pBB[5]));
}

u32 RenderableSpeedTree::GetTriangleCount(RenderMode mode) const
{
	if(mode == NBMODE)
	{
		u32 res = 0;
		for(int iIndex = 0 ; iIndex < NBMODE ; ++iIndex )
		{
			res += m_triangleCount[iIndex];
		}
		return res;
	}
	else
	{
		ntAssert((mode<NBMODE)&&(mode>=DEPTH));
		return m_triangleCount[mode];
	}
}







//! constructor
RenderableSpeedtreeLeaf::RenderableSpeedtreeLeaf(CSpeedTreeWrapper* pTreeInForest)
	:RenderableSpeedTree(pTreeInForest,SpeedTreeManager::Get().m_pLeafMaterial)
{
	
}


// prepare shader consatnts and others
bool RenderableSpeedtreeLeaf::PreRender()
{
	CGatso::Start( "RenderableSpeedTreeLeaf::PreRender" );
	if(!( SpeedTreeManager::Get().m_bSpeedTreeRender && (m_pTreeWrapper->GetRenderBitVector() & Forest_RenderLeaves) ))
    {
		CGatso::Stop( "RenderableSpeedTreeLeaf::PreRender" );
    	return false;
    }

	// set RT camera
	m_pTreeWrapper->m_pForest->SetCamera(false);

	// might need to draw 2 LOD's
	CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_LeafGeometry);
	int iCount=0;
	for (unsigned int unLeafLevel = 0; unLeafLevel < 2; ++unLeafLevel)
	{
		const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = (unLeafLevel == 0) ?
			&(pGeometryCache->m_sLeaves0) : pLeaf = &(pGeometryCache->m_sLeaves1);

		// if this LOD is active and has leaves, draw it
		if (pLeaf->m_nDiscreteLodLevel > -1 && pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
		{
			iCount++;
		}
	}

	// return false if nothing is drawn
	if(iCount==0)
	{
		CGatso::Stop( "RenderableSpeedTreeLeaf::PreRender" );
		return false;
	}

	// update global
	FXMaterial::UploadGlobalParameters(m_pMaterial->GetEffect());
	
		// update local
	CMatrix objectToWorld = m_pTreeWrapper->GetTransform()->GetWorldMatrixFast();
	FXMaterial::UploadObjectParameters( m_pMaterial->GetEffect(), objectToWorld, objectToWorld.GetAffineInverse() );

	// diffuse
	const CSpeedTreeMaterial& cLeafMaterial = m_pTreeWrapper->GetMaterials()->m_cLeafMaterial;
	const SpeedTreeTextures* pTextures = m_pTreeWrapper->GetTextures();
	VERIFY(m_pMaterial->GetEffect()->SetVector("m_diffuseColour0",(D3DXVECTOR4*)(&cLeafMaterial.m_cMaterial.Diffuse)));  
	
	// textute (diffuse and normal)
	VERIFY(m_pMaterial->GetEffect()->SetTexture("g_texCompositeLeafMap",pTextures->m_texCompositeMap->m_Platform.Get2DTexture()));

	// wind matrix
	m_pTreeWrapper->m_pForest->LoadWindMatrix(m_pMaterial);
	// angle matrix
	m_pTreeWrapper->m_pForest->LoadLeafMatrix(m_pMaterial);

    // leaf matrix
	LoadLeafClusterS();

	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	CGatso::Stop( "RenderableSpeedTreeLeaf::PreRender" );
	return true;
}


// render the goemetry
u32 RenderableSpeedtreeLeaf::SendGeometry(const std::string& techName)
{
	CGatso::Start( "RenderableSpeedTreeLeaf::SendGeometry" );
	u32 uiNumPasses = 0;
	u32 uiNumTrig = 0;

	CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_LeafGeometry);

	VERIFY(m_pMaterial->GetEffect()->SetTechnique(techName.c_str()));
	VERIFY(m_pMaterial->GetEffect()->Begin(&uiNumPasses, 0));
	VERIFY(m_pMaterial->GetEffect()->BeginPass(0));

	GetD3DDevice()->SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
	
	// might need to draw 2 LOD's
	for (unsigned int unLeafLevel = 0; unLeafLevel < 2; ++unLeafLevel)
	{
		const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = (unLeafLevel == 0) ?
			&(pGeometryCache->m_sLeaves0) : pLeaf = &(pGeometryCache->m_sLeaves1);

		int unLod = pLeaf->m_nDiscreteLodLevel;

		// if this LOD is active and has leaves, draw it
		if (unLod > -1 && pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
		{
			//// debug
			//void* pData = 0;
			//m_pLeafVertexBuffer[unLod]->Lock(0,0,&pData,D3DLOCK_READONLY);
			//m_pLeafVertexBuffer[unLod]->Unlock();
			//// end debug

			VERIFY(m_pMaterial->GetEffect()->SetFloat("m_clipTreshold", pLeaf->m_fAlphaTestValue / 255.0f));  
			VERIFY(m_pMaterial->GetEffect()->CommitChanges( ));

			// draw
			GetD3DDevice()->SetStreamSource(0, m_pTreeWrapper->GetLeafBuffers()->m_pLeafVertexBuffer[unLod], 0, sizeof(SFVFLeafVertex));
			GetD3DDevice()->DrawPrimitive(D3DPT_TRIANGLELIST, 0, pLeaf->m_usLeafCount * 2);
			uiNumTrig += pLeaf->m_usLeafCount * 2;
		}
	}
	VERIFY(m_pMaterial->GetEffect()->EndPass( ));
    VERIFY(m_pMaterial->GetEffect()->End( ));

	CGatso::Stop( "RenderableSpeedTreeLeaf::SendGeometry" );
	return uiNumTrig;
}



void RenderableSpeedtreeLeaf::LoadLeafClusterS() const
{
    unsigned int uiEntryCount = 0;
 	CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_LeafGeometry, -1, -1, 0);
	const float* pTable = m_pTreeWrapper->GetSpeedTree()->GetLeafBillboardTable(uiEntryCount);
    VERIFY(m_pMaterial->GetEffect()->SetVectorArray("g_avLeafClusters", reinterpret_cast<const D3DXVECTOR4*>(pTable), uiEntryCount / 4));
}





	
		
		



/////////////////////////////////////
//RenderableSpeedtreeFrond


RenderableSpeedtreeFrond::RenderableSpeedtreeFrond(CSpeedTreeWrapper* pTreeInForest)
	:RenderableSpeedTree(pTreeInForest,SpeedTreeManager::Get().m_pFrondMaterial)
{
	
}

bool RenderableSpeedtreeFrond::PreRender()
{
	CGatso::Start( "RenderableSpeedTreeFrond::PreRender" );
	// return if not enable
    if(!( SpeedTreeManager::Get().m_bSpeedTreeRender && (m_pTreeWrapper->GetRenderBitVector() & Forest_RenderFronds)))
    {
		CGatso::Stop( "RenderableSpeedTreeFrond::PreRender" );
    	return false;
    }

	// set RT camera
	m_pTreeWrapper->m_pForest->SetCamera(false);

	// MONSTERS\Frank 16/12/2005 14:08:06 todo: SetSamplerState
	// no more
	
	// get geometry
	CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_FrondGeometry);
	
	// returm if completely transparent
	if(!(pGeometryCache->m_fFrondAlphaTestValue > 0.0f))
	{
		CGatso::Stop( "RenderableSpeedTreeFrond::PreRender" );
		return false;
	}

	// return if no level of detail to render
	const SpeedTreeFrondBuffers* pFrondBuffers = m_pTreeWrapper->GetFrondBuffers();
	if(!(pFrondBuffers->m_pFrondIndexCounts && pGeometryCache->m_sFronds.m_nDiscreteLodLevel > -1
		&& pFrondBuffers->m_pFrondIndexCounts[pGeometryCache->m_sFronds.m_nDiscreteLodLevel] > 0))
	{
		CGatso::Stop( "RenderableSpeedTreeFrond::PreRender" );
		return false;
	}
	

	//////////////////////////////////////////////
	// SET EFFECT PARAM

	// update global
	FXMaterial::UploadGlobalParameters(m_pMaterial->GetEffect());

	// wind matrix
	m_pTreeWrapper->m_pForest->LoadWindMatrix(m_pMaterial);
	
	// update local
	CMatrix objectToWorld = m_pTreeWrapper->GetTransform()->GetWorldMatrixFast();
	FXMaterial::UploadObjectParameters( m_pMaterial->GetEffect(), objectToWorld, objectToWorld.GetAffineInverse() );
	
	///////////////////////////////////
	// ideally this should be in the "UploadObjectParameters" function
	const CSpeedTreeMaterial& cFrondMaterial = m_pTreeWrapper->GetMaterials()->m_cFrondMaterial;
	const SpeedTreeTextures* pTextures = m_pTreeWrapper->GetTextures();

	// diffuse
	VERIFY(m_pMaterial->GetEffect()->SetVector("m_diffuseColour0", (D3DXVECTOR4*)(&cFrondMaterial.m_cMaterial.Diffuse)));  
	// textute (diffuse and normal)
    VERIFY(m_pMaterial->GetEffect()->SetTexture( "g_texCompositeShadowMap", pTextures->m_texShadow->m_Platform.Get2DTexture() ));
	VERIFY(m_pMaterial->GetEffect()->SetTexture( "g_texCompositeLeafMap", pTextures->m_texCompositeMap->m_Platform.Get2DTexture()));
	// clip treahold (bcause we dont use alpha-test
	VERIFY(m_pMaterial->GetEffect()->SetFloat("m_clipTreshold", pGeometryCache->m_fFrondAlphaTestValue / 255.0f));
	// end of ideal world
	///////////////////////////////////



	//////////////////////////////////////////////
	// SET BIG STUFF

	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	CGatso::Stop( "RenderableSpeedTreeFrond::PreRender" );
	return true;
}

u32 RenderableSpeedtreeFrond::SendGeometry(const std::string& techName)
{
	CGatso::Start( "RenderableSpeedTreeFrond::SendGeometry" );
	u32 uiNumPasses = 0;
	u32 uiNumTrig = 0;

	GetD3DDevice()->SetFVF(D3DFVF_SPEEDTREE_FROND_VERTEX);
    VERIFY(m_pMaterial->GetEffect()->SetTechnique(techName.c_str()));
    VERIFY(m_pMaterial->GetEffect()->Begin(&uiNumPasses, 0));
    VERIFY(m_pMaterial->GetEffect()->BeginPass(0));

	const SpeedTreeFrondBuffers* pBuffer = m_pTreeWrapper->GetFrondBuffers();
	CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_FrondGeometry);

	// activate the branch vertex buffer
	GetD3DDevice()->SetStreamSource(0, pBuffer->m_pFrondVertexBuffer, 0, sizeof(SFVFFrondVertex));
	// set the index buffer
	GetD3DDevice()->SetIndices(pBuffer->m_pFrondIndexBuffers[pGeometryCache->m_sFronds.m_nDiscreteLodLevel]);
	// draw
	int iNbPrimitive = pBuffer->m_pFrondIndexCounts[pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2;
	GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, pGeometryCache->m_sFronds.m_usVertexCount, 0, iNbPrimitive);
	uiNumTrig += iNbPrimitive;

	VERIFY(m_pMaterial->GetEffect()->EndPass( ));
	VERIFY(m_pMaterial->GetEffect()->End( ));

	CGatso::Stop( "RenderableSpeedTreeFrond::SendGeometry" );
	return uiNumTrig;
}






















/////////////////////////////////////
//RenderableSpeedtreeBranch


RenderableSpeedtreeBranch::RenderableSpeedtreeBranch(CSpeedTreeWrapper* pTreeInForest)
	:RenderableSpeedTree(pTreeInForest,SpeedTreeManager::Get().m_pBranchMaterial)
{
	
}

bool RenderableSpeedtreeBranch::PreRender()
{
	CGatso::Start( "RenderableSpeedTreeBranch::PreRender" );
	// return if not enable
    if(!(SpeedTreeManager::Get().m_bSpeedTreeRender && (m_pTreeWrapper->GetRenderBitVector() & Forest_RenderBranches)))
    {
    	CGatso::Stop( "RenderableSpeedTreeBranch::PreRender" );
		return false;
    }

	// set RT camera
	m_pTreeWrapper->m_pForest->SetCamera(false);
	
	// get geometry
	CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_BranchGeometry);
	
	// returm if completely transparent
	if(!(pGeometryCache->m_fBranchAlphaTestValue > 0.0f))
	{
    	CGatso::Stop( "RenderableSpeedTreeBranch::PreRender" );
		return false;
	}

	// return if no level of detail to render
	const SpeedTreeBranchBuffers* pBranchBuffers = m_pTreeWrapper->GetBranchBuffers();
	if(!(pBranchBuffers->m_pBranchIndexCounts && pGeometryCache->m_sBranches.m_nDiscreteLodLevel > -1
		&& pBranchBuffers->m_pBranchIndexCounts[pGeometryCache->m_sBranches.m_nDiscreteLodLevel] > 0))
	{
    	CGatso::Stop( "RenderableSpeedTreeBranch::PreRender" );
		return false;
	}
	

	//////////////////////////////////////////////
	// SET EFFECT PARAM

	// update global
	FXMaterial::UploadGlobalParameters(m_pMaterial->GetEffect());

	// wind matrix
	m_pTreeWrapper->m_pForest->LoadWindMatrix(m_pMaterial);
	
	// update local
	CMatrix objectToWorld = m_pTreeWrapper->GetTransform()->GetWorldMatrixFast();
	FXMaterial::UploadObjectParameters( m_pMaterial->GetEffect(), objectToWorld, objectToWorld.GetAffineInverse() );
	
	///////////////////////////////////
	// ideally this should be in the "UploadObjectParameters" function
	const CSpeedTreeMaterial& cBranchMaterial = m_pTreeWrapper->GetMaterials()->m_cBranchMaterial;
	const SpeedTreeTextures* pTextures = m_pTreeWrapper->GetTextures();
	// diffuse
	VERIFY(m_pMaterial->GetEffect()->SetVector("m_diffuseColour0", (D3DXVECTOR4*)(&cBranchMaterial.m_cMaterial.Diffuse)));  
	// textute (diffuse and normal)
    VERIFY(m_pMaterial->GetEffect()->SetTexture( "m_diffuse0", pTextures->m_texBranchTexture->m_Platform.Get2DTexture() ));
    VERIFY(m_pMaterial->GetEffect()->SetTexture( "m_normalMap", pTextures->m_texBranchNormalTexture->m_Platform.Get2DTexture() ));
    VERIFY(m_pMaterial->GetEffect()->SetTexture( "g_texCompositeShadowMap", pTextures->m_texShadow->m_Platform.Get2DTexture() ));
	// clip treahold (bcause we dont use alpha-test
	VERIFY(m_pMaterial->GetEffect()->SetFloat("m_clipTreshold", pGeometryCache->m_fBranchAlphaTestValue / 255.0f));
	// end of ideal world
	///////////////////////////////////




	//////////////////////////////////////////////
	// SET BIG STUFF

	Renderer::Get().SetCullMode( GFX_CULLMODE_REVERSED );

    CGatso::Stop( "RenderableSpeedTreeBranch::PreRender" );
	return true;
}



u32 RenderableSpeedtreeBranch::SendGeometry(const std::string& techName)
{
	CGatso::Start( "RenderableSpeedTreeBranch::SendGeometry" );
	u32 uiNumPasses = 0;
	u32 uiNumTrig = 0;

	GetD3DDevice()->SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
	VERIFY(m_pMaterial->GetEffect()->SetTechnique(techName.c_str()));
	VERIFY(m_pMaterial->GetEffect()->Begin(&uiNumPasses, 0));
	VERIFY(m_pMaterial->GetEffect()->BeginPass(0));

	const SpeedTreeBranchBuffers* pBuffer = m_pTreeWrapper->GetBranchBuffers();
	CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_BranchGeometry);

	// activate the branch vertex buffer
	GetD3DDevice()->SetStreamSource(0, pBuffer->m_pBranchVertexBuffer, 0, sizeof(SFVFBranchVertex));
	// set the index buffer
	GetD3DDevice()->SetIndices(pBuffer->m_pBranchIndexBuffer);
	// draw
	int iNbPrimitive = pBuffer->m_pBranchIndexCounts[pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2;
	GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, pGeometryCache->m_sBranches.m_usVertexCount, 0, iNbPrimitive);
	uiNumTrig += iNbPrimitive;

	VERIFY(m_pMaterial->GetEffect()->EndPass( ));
	VERIFY(m_pMaterial->GetEffect()->End( ));

	CGatso::Start( "RenderableSpeedTreeBranch::SendGeometry" );
	return uiNumTrig;
}































/////////////////////////////////////
//RenderableSpeedtreeBillboard


RenderableSpeedtreeBillboard::RenderableSpeedtreeBillboard(CSpeedTreeWrapper* pTreeInForest)
	:RenderableSpeedTree(pTreeInForest,SpeedTreeManager::Get().m_pBillboardMaterial)
{
	
}

bool RenderableSpeedtreeBillboard::PreRender()
{
	CGatso::Start( "SpeedTreeBillboard::PreRender" );

	// return if not enable
    if(!(SpeedTreeManager::Get().m_bSpeedTreeRender && (m_pTreeWrapper->GetRenderBitVector() & Forest_RenderBillboards)))
    {
		CGatso::Stop( "SpeedTreeBillboard::PreRender" );
    	return false;
    }

	// set RT camera
	m_pTreeWrapper->m_pForest->SetCamera(true);
	
	// get geometry
	CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_BillboardGeometry);
	
	// alpha test ??

	// check if active
#ifdef WRAPPER_RENDER_HORIZONTAL_BILLBOARD
    if(!(pGeometryCache->m_sBillboard0.m_bIsActive || pGeometryCache->m_sBillboard1.m_bIsActive))
    {
		CGatso::Stop( "SpeedTreeBillboard::PreRender" );
    	return false;
    }
#else
    if(!(pGeometryCache->m_sBillboard0.m_bIsActive || pGeometryCache->m_sBillboard1.m_bIsActive || pGeometryCache->m_sHorizontalBillboard.m_bIsActive))
    {
		CGatso::Stop( "SpeedTreeBillboard::PreRender" );
    	return false;
    }
#endif // WRAPPER_RENDER_HORIZONTAL_BILLBOARD
	

	//////////////////////////////////////////////
	// SET EFFECT PARAM

	// update global
	FXMaterial::UploadGlobalParameters(m_pMaterial->GetEffect());

	// wind matrix
	m_pTreeWrapper->m_pForest->LoadWindMatrix(m_pMaterial);
	
	// update local
	CMatrix objectToWorld = m_pTreeWrapper->GetTransform()->GetWorldMatrixFast();
	FXMaterial::UploadObjectParameters( m_pMaterial->GetEffect(), objectToWorld, objectToWorld.GetAffineInverse() );
	
	///////////////////////////////////
	// ideally this should be in the "UploadObjectParameters" function
	const SpeedTreeTextures* pTextures = m_pTreeWrapper->GetTextures();
	const CSpeedTreeMaterial& cBranchMaterial = m_pTreeWrapper->GetMaterials()->m_cBranchMaterial;
	// diffuse
	VERIFY(m_pMaterial->GetEffect()->SetVector("m_diffuseColour0", (D3DXVECTOR4*)(&cBranchMaterial.m_cMaterial.Diffuse)));  
	// textute (diffuse and normal)
    VERIFY(m_pMaterial->GetEffect()->SetTexture( "g_texCompositeLeafMap", pTextures->m_texCompositeMap->m_Platform.Get2DTexture() ));
	// end of ideal world
	///////////////////////////////////

	for(int iTex = 0 ; iTex < 8 ; ++iTex )
	{
		GetD3DDevice()->SetSamplerState(iTex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		GetD3DDevice()->SetSamplerState(iTex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		GetD3DDevice()->SetSamplerState(iTex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		//GetD3DDevice()->SetSamplerState(iTex, D3DSAMP_SRGBTEXTURE, FALSE);
	}
	

	//////////////////////////////////////////////
	// SET BIG STUFF

	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	CGatso::Stop( "SpeedTreeBillboard::PreRender" );

	return true;
}

u32 RenderableSpeedtreeBillboard::SendGeometry(const std::string& techName)
{
	CGatso::Start( "SpeedTreeBillboard::SendGeometry" );

	// draw billboards in immediate mode (as close as directx gets to immediate mode)
	u32 uiNumPasses = 0;
	u32 uiNumTrig = 0;
	
	// normals
	CPoint cameraPosition = RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldTranslation();
	CDirection cam = CDirection(cameraPosition-m_pobTransform->GetWorldTranslation());
	cam.Normalise();
	CDirection up(0.0f,1.0f,0.0f);
	CDirection right = cam.Cross(up);
	right.Normalise();
	CDirection normals[4];
	normals[0] = -0.5f * right - 0.2f * cam + 0.5f*up;
	normals[1] = 0.5f * right - 0.2f * cam + 0.5f*up;
	normals[2] = 0.5f * right - 0.2f * cam;
	normals[3] = -0.5f * right - 0.2f * cam;

	GetD3DDevice()->SetFVF(D3DFVF_SPEEDTREE_BILLBOARD_VERTEX);
    VERIFY(m_pMaterial->GetEffect()->SetTechnique(techName.c_str()));
    VERIFY(m_pMaterial->GetEffect()->Begin(&uiNumPasses, 0));
    VERIFY(m_pMaterial->GetEffect()->BeginPass(0));

	CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_BillboardGeometry);


  //  struct SBillboardVertex 
  //  {
  //      float fX, fY, fZ;
  //      float nX, nY, nZ;
  //      DWORD dColor;
  //      float fU, fV;
		//float fClipTreshold;
  //  };
	
    if (pGeometryCache->m_sBillboard0.m_bIsActive)
    {
        const float* pCoords = pGeometryCache->m_sBillboard0.m_pCoords;
        const float* pTexCoords = pGeometryCache->m_sBillboard0.m_pTexCoords;
		float fClipTreshold = pGeometryCache->m_sBillboard0.m_fAlphaTestValue / 255.0f;
        SFVFBillboardVertex sVertex[4];
		for(int iIndex = 0 ; iIndex < 4 ; ++iIndex )
		{
			sVertex[iIndex] = SFVFBillboardVertex(&pCoords[iIndex*3],&normals[iIndex][0],0xffffff,&pTexCoords[iIndex*2],fClipTreshold);
		}
        GetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SFVFBillboardVertex));
		uiNumTrig+=2;
    }

   // if there is a 360 degree billboard, then we need to draw the second one
    if (pGeometryCache->m_sBillboard1.m_bIsActive)
    {
        const float* pCoords = pGeometryCache->m_sBillboard1.m_pCoords;
        const float* pTexCoords = pGeometryCache->m_sBillboard1.m_pTexCoords;
		float fClipTreshold = pGeometryCache->m_sBillboard1.m_fAlphaTestValue / 255.0f;
        SFVFBillboardVertex sVertex[4];
		for(int iIndex = 0 ; iIndex < 4 ; ++iIndex )
		{
			sVertex[iIndex] = SFVFBillboardVertex(&pCoords[iIndex*3],&normals[iIndex][0],0xffffff,&pTexCoords[iIndex*2],fClipTreshold);
		}
 /*       SBillboardVertex sVertex[4] = 
        {
            { pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1], fClipTreshold },
            { pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3], fClipTreshold },
            { pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5], fClipTreshold },
            { pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7], fClipTreshold },
       };*/
		uiNumTrig+=2;
        GetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SFVFBillboardVertex));
    }

    // if we have a horizontal bilboard and it is enabled, draw it too 
#ifdef WRAPPER_RENDER_HORIZONTAL_BILLBOARD
    if (pGeometryCache->m_sHorizontalBillboard.m_bIsActive)
    {   
        const float* pCoords = pGeometryCache->m_sHorizontalBillboard.m_pCoords;
        const float* pTexCoords = pGeometryCache->m_sHorizontalBillboard.m_pTexCoords;
		float fClipTreshold = pGeometryCache->m_sHorizontalBillboard.m_fAlphaTestValue / 255.0f);
        SFVFBillboardVertex sVertex[4];
		for(int iIndex = 0 ; iIndex < 4 ; ++iIndex )
		{
			sVertex[iIndex] = SFVFBillboardVertex(&pCoords[iIndex*3],&normals[iIndex][0],0xffffff,&pTexCoords[iIndex*2],fClipTreshold);
		}
 /*       SBillboardVertex sVertex[4] = 
        {
            { pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1], fClipTreshold },
            { pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3], fClipTreshold},
            { pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5], fClipTreshold},
            { pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7], fClipTreshold},
        };*/
        GetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SFVFBillboardVertex));
		uiNumTrig+=2;
    }

#endif //WRAPPER_RENDER_HORIZONTAL_BILLBOARD


	VERIFY(m_pMaterial->GetEffect()->EndPass( ));
	VERIFY(m_pMaterial->GetEffect()->End( ));


	CGatso::Stop( "SpeedTreeBillboard::SendGeometry" );
	return uiNumTrig;
}








