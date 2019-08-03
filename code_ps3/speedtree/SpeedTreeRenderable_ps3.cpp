#include "speedtree/SpeedTreeRenderable_ps3.h"


#include "speedtree/SpeedTreeWrapper_ps3.h"
#include "speedtree/SpeedTreeManager_ps3.h"
#include "speedtree/SpeedTreeShaders_ps3.h"
#include "speedtree/SpeedTreeForest_ps3.h"
#include "speedtree/speedtreebillboard.h"

#include "anim/transform.h"
#include "speedtree/SpeedTreeUtil_ps3.h"
#include "core/gatso.h"

#include "gfx/renderer.h"
#include "gfx/shadowsystem.h"
#include "gfx/renderersettings.h"
#include "gfx/rendercontext.h"
#include "gfx/fxmaterial.h"
#include "gfx/materialinstance.h"
#include "gfx/depthhazeconsts.h"
#include "gfx/debugshadergraph_ps3.h"

#include "../content_ps3/cg/SpeedTree_Defines.h"

//debug
#include "input/inputhardware.h"	
#include "game/keybinder.h"

#define SPEEDTREE_ENABLE_BATCHING

// Macro functions------------------
#define CHECK_IF_ENABLED if (m_bufferIndex < 0) return;


class CSpeedTreeShading;

namespace
{
	unsigned int c_maxShadowMaps = 3;

	CSpeedTreeShading*	g_speedTreeShaders;

	uint32_t g_lastSortKey = 0;

	inline float SetAlphaTestValue(float alphaTest)
	{
		const float alphaFix = 0.05f;  // so the transparent textures does not go opaque due to the lod-transition alpharef values
		return alphaTest / 255.f + alphaFix;
	}

	inline void NanCheck(float* array, int size)
	{
		for (int i = 0; i < size; ++ i)
		{
			if (IsNan(array[i]) || fabs(array[i]) < 0.00001f)
			{
				array[i] = 0;			
			}
		}
	}

	template <class VertexShader>
	void SetDepthHaze(const VertexShader& VS)
	{
		DECLARE_VS_CONSTANT(g_worldViewMatrix)
		DECLARE_VS_CONSTANT(g_eyePosObjSpace)
		DECLARE_VS_CONSTANT(g_aConsts)
		DECLARE_VS_CONSTANT(g_gConsts)
		DECLARE_VS_CONSTANT(g_sunDir)
		DECLARE_VS_CONSTANT(g_beta1PlusBeta2)
		DECLARE_VS_CONSTANT(g_betaDash1)
		DECLARE_VS_CONSTANT(g_betaDash2)
		DECLARE_VS_CONSTANT(g_oneOverBeta1PlusBeta2)
		DECLARE_VS_CONSTANT(g_sunColor)

		VS -> SetVSConstant(g_aConsts, CDepthHazeSetting::GetAConsts());
		VS -> SetVSConstant(g_gConsts, CDepthHazeSetting::GetGConsts());
		VS -> SetVSConstant(g_sunDir, CDepthHazeSetting::GetSunDir());
		VS -> SetVSConstant(g_beta1PlusBeta2, CDepthHazeSetting::GetBeta1PlusBeta2());
		VS -> SetVSConstant(g_betaDash1, CDepthHazeSetting::GetBetaDash1());
		VS -> SetVSConstant(g_betaDash2, CDepthHazeSetting::GetBetaDash2());
		VS -> SetVSConstant(g_oneOverBeta1PlusBeta2, CDepthHazeSetting::GetOneOverBeta1PlusBeta2());
		VS -> SetVSConstant(g_sunColor, CDepthHazeSetting::GetSunColour());
		VS -> SetVSConstant(g_eyePosObjSpace, RenderingContext::Get()->GetEyePos());
		VS -> SetVSConstant(g_worldViewMatrix, RenderingContext::Get()->m_worldToView);

	}
}


class CSpeedTreeShading : public CDebugShaderGraph
{
public:
	struct ShaderDescription : public CDebugShaderGraph::ShaderDescriptionBase<4>
	{
		ShaderDescription(unsigned int pass, unsigned int treePart, unsigned int shaderType, bool detailMap)
		{
			m_levels[0] = treePart;
			m_levels[1] = pass;
			m_levels[2] = (unsigned int)detailMap;
			m_levels[3] = shaderType;
		}

	};

	CSpeedTreeShading()
		: CDebugShaderGraph(SPEEDTREE_MEMORY_CHUNK)
	{
		CreateGraph("speedtree_");
	}

private:
	virtual void InitGraphDefinitions()
	{
		BEGIN_GRAPH_LEVEL "branch_", "frond_", "leaf_", "billboard_", "billboard2_" END_GRAPH_LEVEL
		BEGIN_GRAPH_LEVEL "material_", "depth_"										END_GRAPH_LEVEL
		BEGIN_GRAPH_LEVEL "nodetail_", "detail_"									END_GRAPH_LEVEL
		BEGIN_GRAPH_LEVEL "vp", "fp"												END_GRAPH_LEVEL
	}

};


// this is to support DECLARE_CONSTANT macro
// making each shader a separate type and parametrising on it will make sure that we'll correctly initialiazed statics for all the shaders
template <unsigned int Pass, unsigned int TreePart, unsigned int ShaderType, bool DetailTexture = false, bool NormalMapped = true>
class CSpeedTreeShader : public CDebugShaderBase 
{
public:
	CSpeedTreeShader()
	{
		ntAssert(g_speedTreeShaders);
		m_shader = g_speedTreeShaders -> GetShader(CSpeedTreeShading::ShaderDescription(Pass, TreePart, ShaderType, DetailTexture));
	}
};

class SpeedTreeBillboardData : public CSpeedTreeRT::SGeometry::S360Billboard
{
};


void CreateSpeedTreeShaders()
{
	g_speedTreeShaders = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK) CSpeedTreeShading();
}

void DestroySpeedTreeShaders()
{
	NT_DELETE_CHUNK(SPEEDTREE_MEMORY_CHUNK, g_speedTreeShaders);
	g_speedTreeShaders = NULL;
}

namespace
{
	inline uint32_t CreateSortKey(uint32_t renderableType, uint32_t data)
	{
		// sort key format :  31		: 1
		//					  29..30	: renderable type (00 - branch, 01 - frond, 10 - leaves, 11 - billboards)
		//					  0..28	    : lower 28 bits of the parent treewrapper class
		const uint32_t addressMask	= 0x1fffffff;
		const uint32_t typeMask		= 0x60000000;
		const uint32_t speedTreeSign = 0x80000000;

		return data & addressMask  | renderableType & typeMask | speedTreeSign;
	
	}
}


/////////////////////////////////////
//RenderableSpeedTree
void RenderableSpeedTree::ResetBatch()
{
	g_lastSortKey = 0;
	//Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL);
	EndBatch();
}

void RenderableSpeedTree::EndBatch()
{
	if (Renderer::Get().GetVertexShader())
	{
		Renderer::Get().m_Platform.ClearStreams();
	}
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL);
}


// when a part of the tree is not "participating",
// then we temporarily turning it off
void RenderableSpeedTree::EnableLod(bool bValue)
{
	if (IsRendering() != bValue)		DisableRendering(!bValue);
	if (IsShadowCasting() != bValue)	DisableShadowCasting(!bValue);
	if (IsShadowRecieving() != bValue)	DisableShadowRecieving(!bValue);
}

// calculate sort key so all speedtree renderables are located continiously in the list of renderables 
void RenderableSpeedTree::CalculateSortKey(const CMatrix *pTransform, uint32_t renderableType)
{
	// ATTENTION : this depends on CalculateSortKey implementation in the CRenderable!
	// it assumes that the base implementation clears the highest bit for the opaque objects
	CSpeedTreeWrapper* parent = m_pTreeWrapper -> GetParent();
	if (!parent)
	{
		parent = m_pTreeWrapper;
	}
	m_iSortKey = CreateSortKey(renderableType, (uint32_t)parent); 
}

// is rendered
bool RenderableSpeedTree::IsSpeedTreeEnable()
{
	return (1<<m_speedtreeType) & m_pTreeWrapper->GetRenderBitVector();
}

	//! constructor
RenderableSpeedTree::RenderableSpeedTree(CSpeedTreeWrapper* pTreeInForest, SpeedTreeRenderableEnum speedtreeType, bool visible)
	:CRenderable(pTreeInForest->GetTransform(),visible,visible,visible, RT_SPEED_TREE)
	,m_pTreeWrapper(pTreeInForest)
	,m_speedtreeType(speedtreeType)
	,m_lod(-1)
{
	// nothing
}

void RenderableSpeedTree::Initialise()
{
	SetBoundingBox();
	//const SpeedTreeGameLink& link = SpeedTreeManager::Get().GetLink(m_speedtreeType);
	//SetMaterial(link);
}

// set current shader
void RenderableSpeedTree::SetCurrentShaderId(ShaderId::RenderMode renderMode)
{
	m_currentShaderId = ShaderId(renderMode, ShaderId::VSTT_BASIC,
		CRendererSettings::bEnableDepthHaze, CShadowSystemController::Get().IsShadowMapActive() &&  renderMode==ShaderId::RECEIVE_SHADOW);
}

const CShaderGraph* RenderableSpeedTree::GetCurrentShader()
{
	switch(m_currentShaderId.GetRenderMode())
	{
	case ShaderId::WRITE_DEPTH:
	case ShaderId::SHADOW_MAP:
		{
			return m_pMaterialInstance->GetMaterial()->GetDepthWriteGraph(VERTEXSHADER_TRANSFORM_TYPE(m_currentShaderId.GetVertexMode()));
			break;
		}
	case ShaderId::RECEIVE_SHADOW:
		{
			return m_pMaterialInstance->GetMaterial()->GetBasicGraph(m_currentShaderId.GetTechniqueIndex(),VERTEXSHADER_TRANSFORM_TYPE(m_currentShaderId.GetVertexMode()));
			break;
		}
	case ShaderId::MATERIAL:
		{
			return m_pMaterialInstance->GetMaterial()->GetBasicGraph(m_currentShaderId.GetTechniqueIndex(),VERTEXSHADER_TRANSFORM_TYPE(m_currentShaderId.GetVertexMode()));
			break;
		}
	default:
		{
			ntError_p(false, ("RenderableSpeedTree::SetCurrentShader badness"));
			return 0;
			break;
		}
	}	

	return NULL;
}

//! render depths for z pre-pass
void RenderableSpeedTree::RenderDepth()
{
	CGatso::Start( "RenderableSpeedTree::RenderDepth" );
	if(!IsSpeedTreeEnable()) return;
	if (g_lastSortKey != m_iSortKey)
	{
		EndBatch();
	}

	RenderDepth(RenderingContext::Get()->m_worldToScreen);
	UpdateBatch();
	CGatso::Stop( "RenderableSpeedTree::RenderDepth" );
}

//! Renders the game material for this renderable.
void RenderableSpeedTree::RenderMaterial()
{
	CGatso::Start( "RenderableSpeedTree::RenderMaterial" );
	if(!IsSpeedTreeEnable()) return;

	if (g_lastSortKey != m_iSortKey)
	{
		EndBatch();
	}

	RenderMaterial(0);

	UpdateBatch();

	CGatso::Stop( "RenderableSpeedTree::RenderMaterial" );
}
const CMatrix& RenderableSpeedTree::SetShadowMap(uint32_t uIndex)
{
	//Shader* VS = CSpeedTreeShading::Get().GetShader(SPEEDTREE_PASS_DEPTH, SPEEDTREE_BRANCH, SHADERTYPE_VERTEX);
	
	//const CShaderGraph* pCurrentShader = GetCurrentShader();
	//CMatrix worldViewProjShadow = m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[uIndex];
	//uint32_t index = pCurrentShader->GetVertexShader()->GetConstantIndex( "projection" );
	//pCurrentShader->GetVertexShader()->SetVSConstant( index, m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[uIndex] );
	//VS -> SetVSConstant("g_mModelViewProj", RenderingContext::Get()->m_shadowMapProjection[uIndex]);
	Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[uIndex] );
	return RenderingContext::Get()->m_shadowMapProjection[uIndex];

	//Renderer::Get().SetVertexShader(VS);
	
}

//! Renders the shadow map depths.
void RenderableSpeedTree::RenderShadowMap()
{
	if(!IsSpeedTreeEnable()) return;
	CGatso::Start( "___RenderableSpeedTree::RenderShadowMap" );

	if (g_lastSortKey != m_iSortKey)
	{
		EndBatch();
	}


	for (unsigned int sm = 0; sm < c_maxShadowMaps; ++ sm)
	{
		if( m_FrameFlags & (CRenderable::RFF_CAST_SHADOWMAP0 << sm) )
		{
			RenderDepth(SetShadowMap(sm));
		}
	}

	PostRender();

	UpdateBatch();

	CGatso::Stop( "___RenderableSpeedTree::RenderShadowMap" );
	
}
//! Renders with a shadow map compare only. 
void RenderableSpeedTree::RenderShadowOnly()
{
	/*
	if(!IsSpeedTreeEnable()) return;
	CGatso::Start( "___RenderableSpeedTree::RenderShadowOnly" );

	SetCurrentShaderId(ShaderId::RECEIVE_SHADOW);
	if(!PreRender()) return;
	m_pMaterialInstance->PreRenderShadowRecieve( m_pobTransform );
	m_triangleCount[m_currentShaderId.GetRenderMode()] = SendGeometry();
	PostRender();
	m_pMaterialInstance->PostRenderShadowRecieve( );

	CGatso::Stop( "___RenderableSpeedTree::RenderShadowOnly" );
	*/
}

void RenderableSpeedTree::PostRender()
{
	//Renderer::Get().m_Platform.ClearStreams();
	//Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL);
}

// set the bounding box according to m_pTreeWrapper
void RenderableSpeedTree::SetBoundingBox()
{
	const float* pBB = m_pTreeWrapper->GetBoundingBox();
	m_obBounds = CAABB(CPoint(pBB[0],pBB[1],pBB[2]), CPoint(pBB[3],pBB[4],pBB[5]));
}

uint32_t RenderableSpeedTree::GetTriangleCount(ShaderId::RenderMode mode) const
{
	if(mode == ShaderId::_NB_RENDER_MODE)
	{
		uint32_t res = 0;
		for(int iIndex = 0 ; iIndex < ShaderId::_NB_RENDER_MODE ; ++iIndex )
		{
			res += m_triangleCount[iIndex];
		}
		return res;
	}
	else
	{
		assert((mode>=0)&&(mode<ShaderId::_NB_RENDER_MODE));
		return m_triangleCount[mode];
	}
}

template <class VertexShader>
void RenderableSpeedTree::UploadWindMatrices(const VertexShader& VS)
{
	DECLARE_VS_CONSTANT(g_amWindMatrices);

	//
	if (m_pTreeWrapper)
	{
		const CSpeedTreeForest* forest = m_pTreeWrapper -> GetForest();

		if (forest)
		{
			const CSpeedWind* wind = forest -> GetWind();

			if (wind)
			{
				float windMatrices[16 * NUM_WIND_MATRICES];
				for (unsigned int mat = 0; mat < NUM_WIND_MATRICES; ++ mat)
				{
					NT_MEMCPY(&windMatrices[mat * 16], wind -> GetWindMatrix(mat), 64);
				}

				//NanCheck(windMatrices, 16 * NUM_WIND_MATRICES);
				VS -> SetVSConstant(g_amWindMatrices, windMatrices, 4 * NUM_WIND_MATRICES);
			}
		}

	}
}

template <class VertexShader, class PixelShader>
void RenderableSpeedTree::SetCommonConstants(const VertexShader& VS, const PixelShader& PS)
{
	DECLARE_VS_CONSTANT(g_mModelViewProj)
	DECLARE_VS_CONSTANT(g_vOSLightDir)
	DECLARE_PS_CONSTANT(g_VPOStoUV)

	VS -> SetVSConstant(g_mModelViewProj, RenderingContext::Get()->m_worldToScreen, 4 );
	VS -> SetVSConstant(g_vOSLightDir, (float*)&RenderingContext::Get()->m_toKeyLight, 1);

	CVector posToUV( CONSTRUCT_CLEAR );
	posToUV.X() = 1.f / (Renderer::Get().m_targetCache.GetWidth() - 1);
	posToUV.Y() = 1.f / (Renderer::Get().m_targetCache.GetHeight() - 1);

	PS -> SetPSConstant( g_VPOStoUV, posToUV );

	UploadWindMatrices(VS);

	SetDepthHaze(VS);

}

template <class VertexShader, class PixelShader>
void RenderableSpeedTree::SetPerInstanceConstants(const VertexShader& VS, const PixelShader& PS)
{
	DECLARE_VS_CONSTANT(g_vTreePos)
	DECLARE_PS_CONSTANT(g_AlphaTestRef)
	DECLARE_VS_CONSTANT(g_vTreeParams)

	const float*	pos = m_pTreeWrapper -> GetSpeedTree() -> GetTreePosition();
	float			rot = m_pTreeWrapper -> GetRotation();
	VS -> SetVSConstant(g_vTreePos, CVector(pos[0], pos[1], pos[2], rot));

	CVector alphaTestVec(m_alphaTest,m_alphaTest,m_alphaTest,m_alphaTest);
	PS->SetPSConstant(g_AlphaTestRef, alphaTestVec);

	float scale = m_pTreeWrapper -> GetScale();
	float treeParams[4];
	treeParams[0] = scale;
	treeParams[1] = m_pTreeWrapper -> GetWindOffset();
	VS -> SetVSConstant(g_vTreeParams, treeParams, 1);
}

template <class VertexShader, class PixelShader>
void RenderableSpeedTree::PreRenderTransparent(const VertexShader& VS, const PixelShader& PS)
{
	DECLARE_VS_CONSTANT(g_SHCoeffs)
	DECLARE_VS_CONSTANT(g_keyDirColour);

	SetCommonConstants(VS, PS);

	VS -> SetVSConstant(g_SHCoeffs, (float*)RenderingContext::Get()->m_SHMatrices.m_aChannelMats, 12);
	VS -> SetVSConstant( g_keyDirColour, RenderingContext::Get()->m_keyColour );

	Renderer::Get().SetTexture(0, m_pTreeWrapper -> GetTextures() -> m_texCompositeTexture, true);
	Renderer::Get().SetTexture(1, RenderingContext::Get()->m_pStencilTarget, true );

	Renderer::Get().SetSamplerFilterMode(0, TEXTUREFILTER_TRILINEAR);
	Renderer::Get().SetSamplerFilterMode(1, TEXTUREFILTER_BILINEAR);

	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WRAPALL );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_CLAMPALL );

}

template <class VertexShader, class PixelShader>
void RenderableSpeedTree::PreRenderOpaque(const VertexShader& VS, const PixelShader& PS)
{
	DECLARE_PS_CONSTANT(g_SHCoeffs)
	DECLARE_PS_CONSTANT(g_keyDirColour);

	SetCommonConstants(VS, PS);

	PS -> SetPSConstant(g_SHCoeffs, (float*)RenderingContext::Get()->m_SHMatrices.m_aChannelMats, 12);
	PS -> SetPSConstant( g_keyDirColour, RenderingContext::Get()->m_keyColour );

	Renderer::Get().SetCullMode( GFX_CULLMODE_EXPLICIT_CW );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WRAPALL );
	Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_WRAPALL );
	Renderer::Get().SetSamplerAddressMode( 2, TEXTUREADDRESS_CLAMPALL );

	Renderer::Get().SetSamplerFilterMode(0, TEXTUREFILTER_TRILINEAR);
	Renderer::Get().SetSamplerFilterMode(1, TEXTUREFILTER_TRILINEAR);
	Renderer::Get().SetSamplerFilterMode(2, TEXTUREFILTER_BILINEAR);

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

}

template <class VertexShader>
void RenderableSpeedTree::PreRenderDepth(const VertexShader& VS, const Texture::Ptr& texture, const CMatrix& screenTransform)
{
	//VS -> SetVSConstant("g_mModelViewProj", screenTransform, 4 );
	UNUSED(screenTransform);

	UploadWindMatrices(VS);

	Renderer::Get().SetTexture(0, texture, true);
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WRAPALL );
	Renderer::Get().SetSamplerFilterMode(0, TEXTUREFILTER_TRILINEAR);

}


unsigned int RenderableSpeedTree::SendGeometry(const SpeedTreeIndexedBuffers* pBuffer)
{
	uint32_t uiNumTrig = 0;

	//Renderer::Get().m_Platform.SetStream( pBuffer->m_pVertexBuffer );

	// draw
	int lod = m_lod;

	unsigned int offset = 0;
	for (int strip = 0; strip < SpeedTree::g_MaxNumStrips; ++ strip)
	{
		if (pBuffer -> m_IndexCounts[lod][strip] > 0)
		{
			int iNbIndex = pBuffer->m_IndexCounts[lod][strip];
			Renderer::Get().m_Platform.DrawIndexedPrimitives( Gc::kTriangleStrip, offset, iNbIndex, pBuffer->m_pIndexBuffer );
			uiNumTrig += iNbIndex-2;
			offset += iNbIndex;
		}
	}

	return uiNumTrig;

}

void RenderableSpeedTree::UpdateBatch()
{
#ifdef SPEEDTREE_ENABLE_BATCHING
	g_lastSortKey = m_iSortKey;
#endif

}



// LEAF //////////////////////////////////////////////////


//! constructor
RenderableSpeedtreeLeaf::RenderableSpeedtreeLeaf(CSpeedTreeWrapper* pTreeInForest, bool visible)
	:RenderableSpeedTree(pTreeInForest,SPEEDTREE_LEAF, visible)
	,m_lod2(-1)
{
	// nothing
}


//! Set lod enable flag
void RenderableSpeedtreeLeaf::SetLodCulling()
{
	
	// set to true and see if we override it
	EnableLod(true);
	
	if( !(m_pTreeWrapper->GetRenderBitVector() & Speedtree_RenderLeaves))
    {
    	EnableLod(false);
		return;
    }

	// might need to draw 2 LOD's
	//CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_LeafGeometry);
	CSpeedTreeRT* theTree = m_pTreeWrapper -> GetSpeedTree();
	ntAssert(theTree);
	CSpeedTreeRT::SLodValues Lod;
	theTree -> GetLodValues(Lod);

	int iCount=0;
	if (Lod.m_anLeafActiveLods[0] > -1)
	{
		m_alphaTest = SetAlphaTestValue(Lod.m_afLeafAlphaTestValues[0]);
		m_lod		= Lod.m_anLeafActiveLods[0];
		iCount++;
	}
	else
	{
		m_alphaTest = 0;
		m_lod = -1;
	}

	if (Lod.m_anLeafActiveLods[1] > -1)
	{
		m_alphaTest2 = SetAlphaTestValue(Lod.m_afLeafAlphaTestValues[1]);
		m_lod2		= Lod.m_anLeafActiveLods[1];
		iCount++;
	}
	else
	{
		m_alphaTest2 = 0;
		m_lod2 = -1;
	}


	// return false if nothing is drawn
	if(iCount==0)
	{
		EnableLod(false);
		return;
	}  
}


template <class PixelShader>
unsigned int RenderableSpeedtreeLeaf::SendGeometry(int lod, float alphaTest, const PixelShader& PS)
{
	DECLARE_PS_CONSTANT(g_AlphaTestRef)

	uint32_t uiNumTrig = 0;

	VBHandle vb = m_pTreeWrapper->GetLeafBuffers()->m_pVertexBuffers[lod];

	if (!(bool)vb)
	{
		return 0;
	}

	CVector alphaTestVec(alphaTest,alphaTest,alphaTest,alphaTest);
	PS->SetPSConstant(g_AlphaTestRef, alphaTestVec);

	Renderer::Get().m_Platform.ForcePSRefresh();
	Renderer::Get().SetPixelShader(PS.GetShader());

	// if this LOD is active and has leaves, draw it
	if (lod > -1 && vb -> GetCount() > 0)
	{
		unsigned int numVertices = vb -> GetCount();
		// draw
		Renderer::Get().m_Platform.SetStream( vb );
		Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, numVertices );
		uiNumTrig += vb -> GetCount() / 4;
	}

	return uiNumTrig;
}

template <class VertexShader, class PixelShader>
void RenderableSpeedtreeLeaf::RenderLeaves(const VertexShader& VS, const PixelShader& PS)
{
	static CMatrix	offsets(
		0, 0.5f, 0.5f, 0, 
		0, -0.5f, 0.5f, 0,
		0, -0.5f, -0.5f, 0,
		0, 0.5f, -0.5f, 0);

	DECLARE_VS_CONSTANT(g_mLeafUnitSquare)
	DECLARE_VS_CONSTANT(g_vCameraAngles)
	DECLARE_VS_CONSTANT(g_avLeafAngles)
	DECLARE_VS_CONSTANT(g_vLeafAngleScalars)

	if (m_iSortKey != g_lastSortKey)
	{
		VS -> SetVSConstant(g_mLeafUnitSquare, offsets);

		float azimuth;
		float pitch;
		float camAngles[4] = {0};
		CSpeedTreeRT::GetCameraAngles(azimuth, pitch);
		camAngles[0] = DegToRad(azimuth);
		camAngles[1] = DegToRad(pitch);
		VS -> SetVSConstant(g_vCameraAngles, camAngles, 1);


		// Set leaf angles
		if (m_pTreeWrapper)
		{
			const CSpeedTreeForest* forest = m_pTreeWrapper -> GetForest();

			if (forest)
			{
				const CSpeedTreeForest::WindVector& rockAngles = forest -> GetRockAngles();
				const CSpeedTreeForest::WindVector& rustleAngles = forest -> GetRustleAngles();


				float windAngles[4 * MAX_NUM_LEAF_ANGLES];
				for (unsigned int elem = 0; elem < MAX_NUM_LEAF_ANGLES; ++ elem)
				{
					windAngles[elem * 4] = DegToRad(rockAngles[elem]);
					windAngles[elem * 4 + 1] = DegToRad(rustleAngles[elem]);
				}

				VS -> SetVSConstant(g_avLeafAngles, windAngles, MAX_NUM_LEAF_ANGLES);
			}

			float leafScalars[4] = {0};
			m_pTreeWrapper -> GetLeafScalars(leafScalars[0], leafScalars[1]);
			VS -> SetVSConstant(g_vLeafAngleScalars, leafScalars, 1);
		}

		Renderer::Get().SetVertexShader(VS.GetShader());
	}
			

	if (m_lod != -1)
	{
		SendGeometry(m_lod, m_alphaTest, PS);
	}
	if (m_lod2 != -1)
	{
		SendGeometry(m_lod2, m_alphaTest2, PS);
	}


	PostRender();

}

// prepare shader consatnts and others
void RenderableSpeedtreeLeaf::RenderMaterial(int)
{
	
	CGatso::Start( "RenderableSpeedTreeLeaf::RenderMaterial" );

	static CSpeedTreeShader<SPEEDTREE_PASS_MATERIAL, SPEEDTREE_LEAF, SHADERTYPE_VERTEX>	VS;
	static CSpeedTreeShader<SPEEDTREE_PASS_MATERIAL, SPEEDTREE_FROND, SHADERTYPE_PIXEL> PS;

	SetPerInstanceConstants(VS, PS);
	if (m_iSortKey != g_lastSortKey)
	{
		PreRenderTransparent(VS, PS);
	}

	RenderLeaves(VS, PS);

	CGatso::Stop( "RenderableSpeedTreeLeaf::RenderMaterial" );
	
}

void RenderableSpeedtreeLeaf::RenderDepth(const CMatrix& screenTransform)
{
	static CSpeedTreeShader<SPEEDTREE_PASS_DEPTH, SPEEDTREE_LEAF, SHADERTYPE_VERTEX>	VS;
	static CSpeedTreeShader<SPEEDTREE_PASS_DEPTH, SPEEDTREE_BRANCH, SHADERTYPE_PIXEL>	PS;

	DECLARE_VS_CONSTANT(g_mModelViewProj)

	VS -> SetVSConstant(g_mModelViewProj, screenTransform, 4 );
	SetPerInstanceConstants(VS, PS);
	if (m_iSortKey != g_lastSortKey)
	{
		//UploadWindMatrices(VS);
		PreRenderDepth(VS, m_pTreeWrapper -> GetTextures() -> m_texCompositeTexture, screenTransform );
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );
	}

	RenderLeaves(VS, PS);

}

void RenderableSpeedtreeLeaf::CalculateSortKey(const CMatrix *pTransform)
{
	const uint32_t leafType = 0x40000000;
	RenderableSpeedTree::CalculateSortKey(pTransform, leafType);
}



/////////////////////////////////////
//RenderableSpeedtreeFrond


RenderableSpeedtreeFrond::RenderableSpeedtreeFrond(CSpeedTreeWrapper* pTreeInForest, bool visible)
	:RenderableSpeedTree(pTreeInForest,SPEEDTREE_FROND, visible)
{
	// nothing
}

// set material
void RenderableSpeedtreeFrond::SetMaterial(const SpeedTreeGameLink& link)
{
}

//! Set lod enable flag
void RenderableSpeedtreeFrond::SetLodCulling()
{
	
	// set to true and see if we override it
	EnableLod(true);

	// return if not enable
   if( !(m_pTreeWrapper->GetRenderBitVector() & Speedtree_RenderFronds))
    {
    	EnableLod(false);
		return;
    }

	// get geometry
	//CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_FrondGeometry);
	CSpeedTreeRT* theTree = m_pTreeWrapper -> GetSpeedTree();
	ntAssert(theTree);
	CSpeedTreeRT::SLodValues Lod;
	theTree -> GetLodValues(Lod);
	
	// returm if completely transparent
	if(!(Lod.m_fFrondAlphaTestValue > 0.0f))
	{
		EnableLod(false);
		return;
	}

	// return if no level of detail to render
	// todo MONSTERS\Frank 12/01/2006 17:19:08 is this check ok ?? (can I get rid of it)
	const SpeedTreeIndexedBuffers* pFrondBuffers = m_pTreeWrapper->GetFrondBuffers();
	if(!(Lod.m_nFrondActiveLod > -1 
		&& pFrondBuffers->m_IndexCounts[Lod.m_nFrondActiveLod][0] > 0))
	{
		EnableLod(false);
		return;
	}

	m_alphaTest = SetAlphaTestValue(Lod.m_fFrondAlphaTestValue);
	m_lod		= Lod.m_nFrondActiveLod;
	
}

void RenderableSpeedtreeFrond::RenderMaterial(int)
{

   CGatso::Start( "RenderableSpeedTreeFrond::RenderMaterial" );

	static CSpeedTreeShader<SPEEDTREE_PASS_MATERIAL, SPEEDTREE_FROND, SHADERTYPE_VERTEX>	VS;
	static CSpeedTreeShader<SPEEDTREE_PASS_MATERIAL, SPEEDTREE_FROND, SHADERTYPE_PIXEL>		PS;

	const SpeedTreeIndexedBuffers* pBuffer = m_pTreeWrapper->GetFrondBuffers();

	SetPerInstanceConstants(VS, PS);
	if (m_iSortKey != g_lastSortKey)
	{
		PreRenderTransparent(VS, PS);
		Renderer::Get().SetVertexShader(VS.GetShader());

		Renderer::Get().m_Platform.SetStream( pBuffer->m_pVertexBuffer );
	}
	else
	{
		Renderer::Get().m_Platform.ForcePSRefresh();
	}
	Renderer::Get().SetPixelShader(PS.GetShader());

	SendGeometry(pBuffer);

	PostRender();

	CGatso::Stop( "RenderableSpeedTreeFrond::RenderMaterial" );

}


void RenderableSpeedtreeFrond::RenderDepth(const CMatrix& screenTransform)
{
	static CSpeedTreeShader<SPEEDTREE_PASS_DEPTH, SPEEDTREE_BRANCH, SHADERTYPE_VERTEX>	VS;
	static CSpeedTreeShader<SPEEDTREE_PASS_DEPTH, SPEEDTREE_BRANCH, SHADERTYPE_PIXEL>	PS;

	DECLARE_VS_CONSTANT(g_mModelViewProj)

	const SpeedTreeIndexedBuffers* pBuffer = m_pTreeWrapper->GetFrondBuffers();

	VS -> SetVSConstant(g_mModelViewProj, screenTransform, 4 );
	SetPerInstanceConstants(VS, PS);
	if (m_iSortKey != g_lastSortKey)
	{
		PreRenderDepth(VS, m_pTreeWrapper -> GetTextures() -> m_texCompositeTexture, screenTransform);
		Renderer::Get().SetVertexShader(VS.GetShader());
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

		Renderer::Get().m_Platform.SetStream( pBuffer->m_pVertexBuffer );
	}
	else
	{
		Renderer::Get().m_Platform.ForcePSRefresh();
	}
	Renderer::Get().SetPixelShader(PS.GetShader());

	SendGeometry(pBuffer);

	PostRender();
}

void RenderableSpeedtreeFrond::CalculateSortKey(const CMatrix *pTransform)
{
	const uint32_t frondType = 0x20000000;
	RenderableSpeedTree::CalculateSortKey(pTransform, frondType);
}





/////////////////////////////////////
//RenderableSpeedtreeBranch


RenderableSpeedtreeBranch::RenderableSpeedtreeBranch(CSpeedTreeWrapper* pTreeInForest, bool visible)
	:RenderableSpeedTree(pTreeInForest,SPEEDTREE_BRANCH, visible)
{
	// nothing
}


//! Set lod enable flag
void RenderableSpeedtreeBranch::SetLodCulling()
{
	// set to true and see if we override it
	EnableLod(true);
	//return;

	// return if not enable
    if( !(m_pTreeWrapper->GetRenderBitVector() & Speedtree_RenderBranches))
    {
		EnableLod(false);
		return;
    }

	// get geometry
	//CSpeedTreeRT::SGeometry* pGeometryCache = m_pTreeWrapper->UpdateGeometryCache(SpeedTree_BranchGeometry);
	CSpeedTreeRT* theTree = m_pTreeWrapper -> GetSpeedTree();
	ntAssert(theTree);
	CSpeedTreeRT::SLodValues Lod;
	theTree -> GetLodValues(Lod);
	
	// returm if completely transparent
	if(!(Lod.m_fBranchAlphaTestValue > 0.0f))
	{
		EnableLod(false);
		return;
	}

	// return if no level of detail to render
	const SpeedTreeIndexedBuffers* pBranchBuffers = m_pTreeWrapper->GetBranchBuffers();
	if(!(Lod.m_nBranchActiveLod > -1
		&& pBranchBuffers->m_IndexCounts[Lod.m_nBranchActiveLod][0] > 0))
	{
		EnableLod(false);
		return;
	}

	m_lod		= Lod.m_nBranchActiveLod;
	m_alphaTest	= SetAlphaTestValue(Lod.m_fBranchAlphaTestValue);
}


void RenderableSpeedtreeBranch::RenderMaterial(int)
{
	CGatso::Start( "RenderableSpeedTreeBranch::RenderMaterial" );

	static CSpeedTreeShader<SPEEDTREE_PASS_MATERIAL, SPEEDTREE_BRANCH, SHADERTYPE_VERTEX>	VS;
	static CSpeedTreeShader<SPEEDTREE_PASS_MATERIAL, SPEEDTREE_BRANCH, SHADERTYPE_PIXEL>	PS;
	static CSpeedTreeShader<SPEEDTREE_PASS_MATERIAL, SPEEDTREE_BRANCH, SHADERTYPE_PIXEL, true> PS_detail;

	bool hasDetailMap = !!(m_pTreeWrapper -> GetTextures() -> m_texBranchDetailTexture);

	const SpeedTreeIndexedBuffers* pBuffer = m_pTreeWrapper->GetBranchBuffers();

	if (hasDetailMap)
	{
		SetPerInstanceConstants(VS, PS_detail);
	}
	else
	{
		SetPerInstanceConstants(VS, PS);
	}

	if (m_iSortKey != g_lastSortKey)
	{
		Renderer::Get().SetTexture(0, m_pTreeWrapper -> GetTextures() -> m_texBranchTexture, true);
		Renderer::Get().SetTexture(1, m_pTreeWrapper -> GetTextures() -> m_texBranchNormalTexture, true);
		Renderer::Get().SetTexture(2, RenderingContext::Get()->m_pStencilTarget, true );

		if (hasDetailMap)
		{
			Renderer::Get().SetTexture(3, m_pTreeWrapper -> GetTextures() -> m_texBranchDetailTexture, true );
			Renderer::Get().SetSamplerFilterMode( 3, TEXTUREFILTER_TRILINEAR );
			Renderer::Get().SetSamplerAddressMode( 3, TEXTUREADDRESS_WRAPALL );

			PreRenderOpaque(VS, PS_detail);
		}
		else
		{
			PreRenderOpaque(VS, PS);
		}

		Renderer::Get().SetVertexShader(VS.GetShader());

		Renderer::Get().m_Platform.SetStream( pBuffer->m_pVertexBuffer );
	}
	else
	{
		Renderer::Get().m_Platform.ForcePSRefresh();
	}
	if (hasDetailMap)
	{
		Renderer::Get().SetPixelShader(PS_detail.GetShader());
	}
	else
	{
		Renderer::Get().SetPixelShader(PS.GetShader());
	}

	SendGeometry(pBuffer);

	PostRender();

    CGatso::Stop( "RenderableSpeedTreeBranch::RenderMaterial" );
}


void RenderableSpeedtreeBranch::RenderDepth(const CMatrix& screenTransform)
{
	static CSpeedTreeShader<SPEEDTREE_PASS_DEPTH, SPEEDTREE_BRANCH, SHADERTYPE_VERTEX>	VS;
	static CSpeedTreeShader<SPEEDTREE_PASS_DEPTH, SPEEDTREE_BRANCH, SHADERTYPE_PIXEL>	PS;

	DECLARE_VS_CONSTANT(g_mModelViewProj)

	const SpeedTreeIndexedBuffers* pBuffer = m_pTreeWrapper->GetBranchBuffers();

	VS -> SetVSConstant(g_mModelViewProj, screenTransform, 4 );

	SetPerInstanceConstants(VS, PS);
	if (m_iSortKey != g_lastSortKey)
	{
		PreRenderDepth(VS, m_pTreeWrapper -> GetTextures() -> m_texBranchTexture, screenTransform);
		Renderer::Get().SetVertexShader(VS.GetShader());
		Renderer::Get().m_Platform.SetStream( pBuffer->m_pVertexBuffer );

		Renderer::Get().SetCullMode( GFX_CULLMODE_EXPLICIT_CW );
	}
	else
	{
		Renderer::Get().m_Platform.ForcePSRefresh();
	}
	Renderer::Get().SetPixelShader(PS.GetShader());


	SendGeometry(pBuffer);

	PostRender();
}

void RenderableSpeedtreeBranch::CalculateSortKey(const CMatrix *pTransform)
{
	const uint32_t branchType = 0x0;
	RenderableSpeedTree::CalculateSortKey(pTransform, branchType);
}


// CELL ///////////////////


CRenderableSpeedTreeCell::CRenderableSpeedTreeCell(const Transform* transform)
	: CRenderable(transform, true, false, false, RT_SPEED_TREE_CELL)
	, m_pTreeWrappers(NULL)
	, m_vertexBuffers(NULL)
	, m_bufferIndex(-1)
{
}

void CRenderableSpeedTreeCell::SetBounds(const CAABB& aabb)
{
	m_obBounds = aabb;
}

void CRenderableSpeedTreeCell::SetTreeData(CSpeedTreeBillboardBuffers* vb, CSpeedTreeWrapper** trees, unsigned int size)
{
	m_vertexBuffers = vb;
	m_pTreeWrappers	= trees;
	m_numBaseTrees	= size;
	PreComputeConstants();
}

void CRenderableSpeedTreeCell::PreComputeConstants()
{
	//if (!m_pTreeWrapper) return;

//

}

template<class VertexShader>
void CRenderableSpeedTreeCell::SetBillboardLightingConstants(const VertexShader& VS, SpeedTreeBillboardData& sBillboard)
{
	CGatso::Start( "CRenderableSpeedTreeCell::SetBillboardLightingConstants" );
	DECLARE_CONSTANT(g_vOSLightDir, VS);
	DECLARE_CONSTANT(g_amBBNormals, VS);
	DECLARE_CONSTANT(g_amBBBinormals, VS);
	DECLARE_CONSTANT(g_amBBTangents, VS);
	DECLARE_VS_CONSTANT(g_SHCoeffs);
	DECLARE_VS_CONSTANT(g_keyDirColour);

	// Depth haze
	SetDepthHaze(VS);

	VS -> SetVSConstant(g_vOSLightDir, (float*)&RenderingContext::Get()->m_toKeyLight, 1);

	// normals
	CMatrix fourNormals(
		sBillboard.m_pNormals[0], sBillboard.m_pNormals[1], sBillboard.m_pNormals[2], 0.0f,
		sBillboard.m_pNormals[3], sBillboard.m_pNormals[4], sBillboard.m_pNormals[5], 0.0f,
		sBillboard.m_pNormals[6], sBillboard.m_pNormals[7], sBillboard.m_pNormals[8], 0.0f,
		sBillboard.m_pNormals[9], sBillboard.m_pNormals[10], sBillboard.m_pNormals[11], 0.0f
		);
	//VS -> SetVSConstant(g_amBBNormals, fourNormals.GetTranspose());
	VS -> SetVSConstant(g_amBBNormals, fourNormals);



	// binormals
	CMatrix	fourBinormals(
		sBillboard.m_pBinormals[0], sBillboard.m_pBinormals[1], sBillboard.m_pBinormals[2], 0.0f,
		sBillboard.m_pBinormals[3], sBillboard.m_pBinormals[4], sBillboard.m_pBinormals[5], 0.0f,
		sBillboard.m_pBinormals[6], sBillboard.m_pBinormals[7], sBillboard.m_pBinormals[8], 0.0f,
		sBillboard.m_pBinormals[9], sBillboard.m_pBinormals[10], sBillboard.m_pBinormals[11], 0.0f
	);
	//VS -> SetVSConstant(g_amBBBinormals, fourBinormals.GetTranspose());
	VS -> SetVSConstant(g_amBBBinormals, fourBinormals);



	// tangents
	CMatrix	fourTangents(
		sBillboard.m_pTangents[0], sBillboard.m_pTangents[1], sBillboard.m_pTangents[2], 0.0f,
		sBillboard.m_pTangents[3], sBillboard.m_pTangents[4], sBillboard.m_pTangents[5], 0.0f,
		sBillboard.m_pTangents[6], sBillboard.m_pTangents[7], sBillboard.m_pTangents[8], 0.0f,
		sBillboard.m_pTangents[9], sBillboard.m_pTangents[10], sBillboard.m_pTangents[11], 0.0f
		);
	//VS -> SetVSConstant(g_amBBTangents, fourTangents.GetTranspose());
	VS -> SetVSConstant(g_amBBTangents, fourTangents);

	//CDepthHazeSetting::GetAConsts

	VS -> SetVSConstant( g_SHCoeffs, (float*)RenderingContext::Get()->m_SHMatrices.m_aChannelMats, 12);
	VS -> SetVSConstant( g_keyDirColour, RenderingContext::Get()->m_keyColour );


	CGatso::Stop( "CRenderableSpeedTreeCell::SetBillboardLightingConstants" );

}

template <class VertexShader>
void CRenderableSpeedTreeCell::SetBillboardConstants(const VertexShader& VS, unsigned int speedTreeNum)
{
	DECLARE_CONSTANT(g_mBBUnitSquare, VS)
	DECLARE_CONSTANT(g_v360TexCoords, VS)
	DECLARE_CONSTANT(g_vCameraAngles, VS)
	DECLARE_CONSTANT(g_mModelViewProj, VS)
	DECLARE_VS_CONSTANT(g_BillboardConsts)
	DECLARE_VS_CONSTANT(g_BillboardConsts2)

	ntAssert(speedTreeNum < m_numBaseTrees);

	static const CMatrix	offsets(
		0, 0.5f, 1.0f, 0, 
		0, -0.5f, 1.0f, 0,
		0, -0.5f, 0.0f, 0,
		0, 0.5f, 0.0f, 0);

	VS -> SetVSConstant(g_mBBUnitSquare, offsets);

	float fCameraAzimuth, fCameraPitch;
	CSpeedTreeRT::GetCameraAngles(fCameraAzimuth, fCameraPitch);

	// adjust azimuth from range [-180,180] to [0,360]
	if (fCameraAzimuth < 0.0f)
		fCameraAzimuth += 360.0f;

	float afCameraPos[3], afCameraDir[3];
	CSpeedTreeRT::GetCamera(afCameraPos, afCameraDir);

	// prep azimuth for use in shader
	afCameraDir[0] = -afCameraDir[0];
	afCameraDir[1] = -afCameraDir[1];
	afCameraDir[2] = -afCameraDir[2];

	float fBillboardAzimuth = atan2f(afCameraDir[2], -afCameraDir[0]);
	//fBillboardAzimuth += PI;

	if (fBillboardAzimuth < 0.0f)
		fBillboardAzimuth += TWO_PI;
	else if (fBillboardAzimuth >= TWO_PI)
		fBillboardAzimuth = fmod(fBillboardAzimuth, TWO_PI);


    CSpeedTreeRT::SGeometry sGeometry;
    CSpeedTreeRT* speedTree = m_pTreeWrappers[speedTreeNum] -> GetSpeedTree();
	ntAssert(speedTree);

	speedTree -> GetGeometry(sGeometry, SpeedTree_BillboardGeometry);

	unsigned int numImages = sGeometry.m_s360Billboard.m_nNumImages;

	ntAssert(numImages < NUM_360_IMAGES)
	
	//float*	billboardTexCoords = (float*)alloca(numImages * 16);
	static float billboardTexCoords [NUM_360_IMAGES * 16];
	for (unsigned int bb = 0; bb < numImages; ++ bb)
	{
        // min_s, min_t (third corner)
		billboardTexCoords[4 * bb + 0] = sGeometry.m_s360Billboard.m_pTexCoordTable[bb * 8 + 4];
		billboardTexCoords[4 * bb + 1] = sGeometry.m_s360Billboard.m_pTexCoordTable[bb * 8 + 5];

        // max_s, max_t (first corner)
        billboardTexCoords[4 * bb + 2] = sGeometry.m_s360Billboard.m_pTexCoordTable[bb * 8];
        billboardTexCoords[4 * bb + 3] = sGeometry.m_s360Billboard.m_pTexCoordTable[bb * 8 + 1];
	}

	VS -> SetVSConstant(g_v360TexCoords, billboardTexCoords, numImages);

	float cameraAngles[4] = {DegToRad(fCameraAzimuth), DegToRad(fCameraPitch), fBillboardAzimuth, (float)numImages};
	VS -> SetVSConstant(g_vCameraAngles, cameraAngles, 1);

	VS -> SetVSConstant(g_mModelViewProj, RenderingContext::Get()->m_worldToScreen, 4 );

	//float billboardConsts[4] = { 6.28318530f, 255.0f, 84.0f, 171.0f };
	float billboardConsts[4] = { 6.28318530f, 1.f, 84.0f / 255.f, 171.0f / 255.f};
	VS -> SetVSConstant(g_BillboardConsts, billboardConsts, 1);

	const float alphaFadePower = 1.7f;
	float billboardConsts2[4] = { alphaFadePower };
	VS -> SetVSConstant(g_BillboardConsts2, billboardConsts2, 1);

}

template <class VertexShader>
void CRenderableSpeedTreeCell::RenderBillboards(const VertexShader& VS, unsigned int treeIndex, unsigned int numVerts)
{
	if (numVerts > 0)
	{
		SetBillboardConstants(VS, treeIndex);

		Renderer::Get().SetVertexShader(VS.GetShader(), true);

		unsigned int numVerts = m_vertexBuffers -> SubmitVertexData(m_bufferIndex, treeIndex);
		Renderer::Get().m_Platform.SetStream( m_vertexBuffers -> GetBuffer( m_bufferIndex ) ); 

		Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, numVerts );
	}

}

void CRenderableSpeedTreeCell::RenderMaterial()
{
	//CHECK_IF_ENABLED

	CGatso::Start( "CRenderableSpeedTreeCell::RenderMaterial" );

	static 	SpeedTreeBillboardData sBillboard;

	if (m_bufferIndex >= 0) 
	{
		if (g_lastSortKey != m_iSortKey)
		{
			RenderableSpeedTree::EndBatch();
		}

		if ((SpeedTreeManager::Get().GetRenderBitVector() & Speedtree_RenderBillboards))
		{
			CSpeedTreeShader<SPEEDTREE_PASS_MATERIAL, SPEEDTREE_BILLBOARD, SHADERTYPE_PIXEL>	PS;
			CSpeedTreeShader<SPEEDTREE_PASS_MATERIAL, SPEEDTREE_BILLBOARD, SHADERTYPE_VERTEX>	VS1;
			CSpeedTreeShader<SPEEDTREE_PASS_MATERIAL, SPEEDTREE_BILLBOARD2, SHADERTYPE_VERTEX>  VS2;

			DECLARE_PS_CONSTANT(g_SHCoeffs);
			//DECLARE_PS_CONSTANT(g_keyDirColour);
			DECLARE_PS_CONSTANT(g_VPOStoUV);

			//if (m_iSortKey != g_lastSortKey)
			{
				PS -> SetPSConstant(g_SHCoeffs, (float*)RenderingContext::Get()->m_SHMatrices.m_aChannelMats, 12);
				//PS -> SetPSConstant( g_keyDirColour, RenderingContext::Get()->m_keyColour );

				CVector posToUV( CONSTRUCT_CLEAR );
				posToUV.X() = 1.f / (Renderer::Get().m_targetCache.GetWidth() - 1);
				posToUV.Y() = 1.f / (Renderer::Get().m_targetCache.GetHeight() - 1);

				PS -> SetPSConstant( g_VPOStoUV, posToUV );

				Renderer::Get().SetPixelShader(PS.GetShader(), true);
				
				CSpeedTreeRT::UpdateBillboardLighting(sBillboard);
			}

			Renderer::Get().SetTexture(2, RenderingContext::Get()->m_pStencilTarget, true );
			Renderer::Get().SetSamplerAddressMode( 2, TEXTUREADDRESS_CLAMPALL );

			unsigned int hasDrawn = 0;
			for (unsigned int treeIndex = 0; treeIndex < m_numBaseTrees; ++ treeIndex)
			{
				const SpeedTreeTextures* textures = m_pTreeWrappers[treeIndex] -> GetTextures();

				Texture::Ptr	diffuseTex = textures -> m_texBillboardTexture;
				Texture::Ptr    normalTex  = textures -> m_texBillboardNormalTexture;

				if (!diffuseTex.IsValid())
				{
					diffuseTex = textures -> m_texCompositeTexture;
				}
				if (!normalTex.IsValid())
				{
					normalTex = textures -> m_texCompositeNormalTexture;
				}

				Renderer::Get().SetTexture(0, diffuseTex, true);
				Renderer::Get().SetTexture(1, normalTex, true);

				Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WRAPALL );
				Renderer::Get().SetSamplerAddressMode( 1, TEXTUREADDRESS_WRAPALL );


				unsigned int numVerts = m_vertexBuffers -> SubmitVertexData(m_bufferIndex, treeIndex);

				SetBillboardLightingConstants(VS1, sBillboard);
				RenderBillboards(VS1, treeIndex, numVerts);

				SetBillboardLightingConstants(VS2, sBillboard);
				RenderBillboards(VS2, treeIndex, numVerts);

				hasDrawn |= numVerts;
			}

		//	PostRender();
			if (hasDrawn)
			{
				Renderer::Get().m_Platform.ClearStreams();
			}
			Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL);
		}

		g_lastSortKey = m_iSortKey;
	}

	CGatso::Stop( "CRenderableSpeedTreeCell::RenderMaterial" );
}


void CRenderableSpeedTreeCell::RenderDepth()
{
	//CHECK_IF_ENABLED

	CGatso::Start( "CRenderableSpeedTreeCell::RenderDepth" );

	if (m_bufferIndex >= 0)
	{
		if (g_lastSortKey != m_iSortKey)
		{
			RenderableSpeedTree::EndBatch();
		}
		if ((SpeedTreeManager::Get().GetRenderBitVector() & Speedtree_RenderBillboards))
		{
			CSpeedTreeShader<SPEEDTREE_PASS_DEPTH, SPEEDTREE_BILLBOARD, SHADERTYPE_PIXEL>	PS;
			CSpeedTreeShader<SPEEDTREE_PASS_DEPTH, SPEEDTREE_BILLBOARD, SHADERTYPE_VERTEX>	VS1;
			CSpeedTreeShader<SPEEDTREE_PASS_DEPTH, SPEEDTREE_BILLBOARD2, SHADERTYPE_VERTEX> VS2;

			//Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

			//Renderer::Get().SetPixelShader(PS.GetShader(), true);
			//if (m_iSortKey != g_lastSortKey)
			{
				Renderer::Get().SetPixelShader(PS.GetShader(), true);
			}
			g_lastSortKey = m_iSortKey;


			unsigned int hasDrawn = 0;
			for (unsigned int treeIndex = 0; treeIndex < m_numBaseTrees; ++ treeIndex)
			{
				const SpeedTreeTextures* textures = m_pTreeWrappers[treeIndex] -> GetTextures();

				Texture::Ptr	diffuseTex = textures -> m_texBillboardTexture;

				if (!diffuseTex.IsValid())
				{
					diffuseTex = textures -> m_texCompositeTexture;
				}
				Renderer::Get().SetTexture(0, diffuseTex, true);

				Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_WRAPALL );

				unsigned int numVerts = m_vertexBuffers -> SubmitVertexData(m_bufferIndex, treeIndex);
				RenderBillboards(VS1, treeIndex, numVerts);
				RenderBillboards(VS2, treeIndex, numVerts);

				hasDrawn |= numVerts;

			}
			if (hasDrawn)
			{
				Renderer::Get().m_Platform.ClearStreams();
			}
			Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL);
		}
		else
		{
			Renderer::Get().SetPixelShader(NULL, true);
		}

	}

	CGatso::Stop( "CRenderableSpeedTreeCell::RenderDepth" );

}


void CRenderableSpeedTreeCell::CalculateSortKey(const CMatrix *pTransform)
{
	//const uint32_t billboardType = 0x60000000;
	//m_iSortKey	= CreateSortKey(billboardType, 0);

	CRenderable::CalculateSortKey(pTransform);

}



