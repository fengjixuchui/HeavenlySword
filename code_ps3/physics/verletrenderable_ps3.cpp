#include "verletrenderable_ps3.h"

#include "gfx/shadowsystem_ps3.h"
#include "core/gatso.h"
#include "anim/transform.h"
#include "gfx/rendercontext.h"
#include "physics/verlet.h"

#include "verletinstance_ps3.h"
#include "verletdef_ps3.h"
#include "verletManager_ps3.h"

namespace Physics
{
//! constructor
VerletRenderableInstance::VerletRenderableInstance(const VerletInstance* pInstance, const VerletInstanceDef& def, const VerletMaterialInstanceDef& materialDef, const VerletGameLink& link)
	:CRenderable(pInstance->m_pTransform,true,true,true, RT_VERLET)
	,m_material(materialDef,link)
	,m_pInstance(pInstance)
{
	InitVertexStream(def,link);
}
//! dtor
VerletRenderableInstance::~VerletRenderableInstance()
{
//	GcKernel::FreeHostMemory(m_pDynamicVertexBuffer[0]);
	NT_FREE_CHUNK( Mem::MC_RSX_MAIN_USER, (uintptr_t)m_pDynamicVertexBuffer[0] );
}

// enable ?
bool VerletRenderableInstance::IsVerletRenderEnable()
{
	return VerletManager::Get().GetFlags(VerletManager::RENDERING);
}

void VerletRenderableInstance::InitVertexStream(const VerletInstanceDef& def, const VerletGameLink& gameLink)
{
	m_uiNbVertices=def.GetNbParticles();
	m_uiNbIndices=def.m_connectivity.size();
	
	//dynamic stream
	m_pVertexHandleDynamic = RendererPlatform::CreateVertexStream(
		m_uiNbVertices,
		sizeof(FlagBinding::VertexDynamic),
		gameLink.m_streamDynamic.m_iNbStreamElem,
		gameLink.m_streamDynamic.m_gcVertexElem.Get(),
		Gc::kUserBuffer );
	//mem
	uint32_t uiOneSize = m_uiNbVertices*sizeof(FlagBinding::VertexDynamic);
//	m_pDynamicVertexBuffer[0]=static_cast<uint8_t*>( GcKernel::AllocateHostMemory(uiOneSize*g_uiNbBuffers) );
	m_pDynamicVertexBuffer[0]=(uint8_t*)( NT_MEMALIGN_CHUNK( Mem::MC_RSX_MAIN_USER, uiOneSize*g_uiNbBuffers, 128 ) );
	for(uint32_t iBuffer = 1 ; iBuffer < g_uiNbBuffers ; ++iBuffer )
	{
		m_pDynamicVertexBuffer[iBuffer] = m_pDynamicVertexBuffer[0] + uiOneSize*iBuffer;
	}

	// allocate static stream
	m_pVertexHandleStatic = RendererPlatform::CreateVertexStream(
		m_uiNbVertices,
		sizeof(FlagBinding::VertexStatic),
		gameLink.m_streamStatic.m_iNbStreamElem,
		gameLink.m_streamStatic.m_gcVertexElem.Get(),
		Gc::kStaticBuffer );
	// init static stream
	m_pVertexHandleStatic->Write(&(def.m_staticBuffer.front()));
	// mem
	//m_pStaticVertexBuffer =  NT_NEW uint8_t[m_uiNbVertices*sizeof(FlagBinding::VertexStatic)];
	//NT_MEMCPY(m_pStaticVertexBuffer,&(def.m_staticBuffer.front()),m_uiNbVertices*sizeof(FlagBinding::VertexStatic));

	// index
	//m_pIndexBuffer =  NT_NEW uint16_t[m_uiNbIndices];
	//NT_MEMCPY(m_pIndexBuffer,&(def.m_connectivity.front()),m_uiNbIndices*sizeof(uint16_t));
	m_pIndexHandle = RendererPlatform::CreateIndexStream(Gc::kIndex16,m_uiNbIndices,Gc::kStaticBuffer);
	m_pIndexHandle->Write(&(def.m_connectivity.front()));
}


// set current shader
void VerletRenderableInstance::SetCurrentShaderId(ShaderId::RenderMode renderMode)
{
	m_currentShaderId = ShaderId(renderMode, ShaderId::VSTT_BASIC,
		CRendererSettings::bEnableDepthHaze, CShadowSystemController::Get().IsShadowMapActive() &&  renderMode==ShaderId::RECEIVE_SHADOW);
}

const CShaderGraph* VerletRenderableInstance::GetCurrentShader()
{
	switch(m_currentShaderId.GetRenderMode())
	{
	case ShaderId::WRITE_DEPTH:
	case ShaderId::SHADOW_MAP:
		{
			return GetMaterialInstance()->GetMaterial()->GetDepthWriteGraph(VERTEXSHADER_TRANSFORM_TYPE(m_currentShaderId.GetVertexMode()));
			break;
		}
	case ShaderId::RECEIVE_SHADOW:
		{
			return GetMaterialInstance()->GetMaterial()->GetBasicGraph(m_currentShaderId.GetTechniqueIndex(),VERTEXSHADER_TRANSFORM_TYPE(m_currentShaderId.GetVertexMode()));
			break;
		}
	case ShaderId::MATERIAL:
		{
			return GetMaterialInstance()->GetMaterial()->GetBasicGraph(m_currentShaderId.GetTechniqueIndex(),VERTEXSHADER_TRANSFORM_TYPE(m_currentShaderId.GetVertexMode()));
			break;
		}
	default:
		{
			ntError_p(false, ("VerletRenderableInstance::SetCurrentShader badness"));
			return 0;
			break;
		}
	}	

	return NULL;
}

// prepare shader consatnts and others
bool VerletRenderableInstance::PreRender()
{
	// this will prevent any state caching from occouring,
	// something invalid went within a heresy block.
	if (CRendererSettings::bUseHeresy)
		Renderer::Get().m_Platform.FlushCaches();

	CGatso::Start( "VerletRenderableInstance::PreRender" );
	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );
	CGatso::Stop( "VerletRenderableInstance::PreRender" );

	return true;
}

void VerletRenderableInstance::RendererUpdate()
{
	// we should sync on spu completion here
	UpdateBoundingBox();
}

//! render depths for z pre-pass
void VerletRenderableInstance::RenderDepth()
{
	if(!IsVerletRenderEnable()) return;

	SetCurrentShaderId(ShaderId::WRITE_DEPTH);
	if(!PreRender()) return;
	GetMaterialInstance()->PreRenderDepth( m_pobTransform, false );
	m_triangleCount[m_currentShaderId.GetRenderMode()] = SendGeometry();
	PostRender();
	GetMaterialInstance()->PostRenderDepth(false);
}

//! Renders the game material for this renderable.
void VerletRenderableInstance::RenderMaterial()
{
	if(!IsVerletRenderEnable()) return;

	SetCurrentShaderId(ShaderId::MATERIAL);
	if(!PreRender()) return;
	GetMaterialInstance()->PreRender( m_pobTransform, IsShadowRecieving() );
	m_triangleCount[m_currentShaderId.GetRenderMode()] = SendGeometry();
	PostRender();
	GetMaterialInstance()->PostRender();
}
void VerletRenderableInstance::SetShadowMap(uint32_t uIndex)
{
	const CShaderGraph* pCurrentShader = GetCurrentShader();
	CMatrix worldViewProjShadow = m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[uIndex];
	uint32_t index = pCurrentShader->GetVertexShader()->GetConstantIndex( "projection" );
	pCurrentShader->GetVertexShader()->SetVSConstant( index, m_pobTransform->GetWorldMatrixFast() * RenderingContext::Get()->m_shadowMapProjection[uIndex] );
	Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[uIndex] );
}

//! Renders the shadow map depths.
void VerletRenderableInstance::RenderShadowMap()
{
	if(!IsVerletRenderEnable()) return;

	SetCurrentShaderId(ShaderId::SHADOW_MAP);
	m_triangleCount[m_currentShaderId.GetRenderMode()]=0;
	if(!PreRender()) return;
	GetMaterialInstance()->PreRenderDepth( m_pobTransform, true );
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP0 )
	{
		SetShadowMap(0);
		m_triangleCount[m_currentShaderId.GetRenderMode()] += SendGeometry();
	}
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP1 )
	{
		SetShadowMap(1);
		m_triangleCount[m_currentShaderId.GetRenderMode()] += SendGeometry();
	}
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP2 )
	{
		SetShadowMap(2);
		m_triangleCount[m_currentShaderId.GetRenderMode()] += SendGeometry();
	}
	if( m_FrameFlags & CRenderable::RFF_CAST_SHADOWMAP3 )
	{
		SetShadowMap(3);
		m_triangleCount[m_currentShaderId.GetRenderMode()] += SendGeometry();
	}
	PostRender();
	GetMaterialInstance()->PostRenderDepth( true );
}
//! Renders with a shadow map compare only. 
void VerletRenderableInstance::RenderShadowOnly()
{
	if(!IsVerletRenderEnable()) return;

	SetCurrentShaderId(ShaderId::RECEIVE_SHADOW);
	if(!PreRender()) return;
	GetMaterialInstance()->PreRenderShadowRecieve( m_pobTransform );
	m_triangleCount[m_currentShaderId.GetRenderMode()] = SendGeometry();
	PostRender();
	GetMaterialInstance()->PostRenderShadowRecieve( );
}

void VerletRenderableInstance::PostRender()
{
	Renderer::Get().m_Platform.ClearStreams();
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL);
}

// set the bounding box according to m_pTreeWrapper
void VerletRenderableInstance::UpdateBoundingBox()
{
	m_obBounds = CAABB(m_pInstance->m_pFlagOut->m_positionMin,m_pInstance->m_pFlagOut->m_positionMax);
}



// render the goemetry
uint32_t VerletRenderableInstance::SendGeometry()
{
	CGatso::Start( "RenderableSpeedTreeLeaf::SendGeometry" );
	uint32_t uiNumTrig = 0;

	m_pVertexHandleDynamic->SetDataAddress(m_pDynamicVertexBuffer[m_multipleBufferIndex]);

	Renderer::Get().m_Platform.SetStream( m_pVertexHandleDynamic );
	Renderer::Get().m_Platform.SetStream( m_pVertexHandleStatic );
	Renderer::Get().m_Platform.DrawIndexedPrimitives( Gc::kTriangles, 0, m_uiNbIndices, m_pIndexHandle );
	uiNumTrig += m_uiNbIndices/3;

	CGatso::Stop( "RenderableSpeedTreeLeaf::SendGeometry" );
	return uiNumTrig;
}


VerletRenderableInstance::VertexBuffer VerletRenderableInstance::GetNextBufferPointer()
{
	m_multipleBufferIndex++;
	return m_pDynamicVertexBuffer[m_multipleBufferIndex];
}



} //Physics
