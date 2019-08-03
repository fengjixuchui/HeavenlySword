/***************************************************************************************************
*
*	$Header:: /game/meshinstance.cpp 6     24/07/03 11:53 Simonb                                   $
*
*
*
*	CHANGES
*
*	9/5/2003	SimonB	Created
*
***************************************************************************************************/

#include "core/memman.h"
#include "anim/transform.h"
#include "gfx/meshinstance.h"
#include "gfx/renderersettings.h"
#include "gfx/clump.h"
#include "materialdebug.h"
#include "renderer.h"


#ifdef PLATFORM_PS3
#include "blendshapes/shading/bsskin.h"
#endif

#include "heresypushbuffers.h"


typedef ntstd::Map< uint32_t , CHeresyPushBuffers*> PBContainer;

namespace 
{
	PBContainer* theMap = NULL;
#ifndef _RELEASE
	bool		 mapValid = false;
#endif
}

PBContainer& GetPBMap()
{
	if ( !theMap )
	{
		ntError( !mapValid );
		theMap = NT_NEW_CHUNK(Mem::MC_GFX) PBContainer;
#		ifndef _RELEASE
			mapValid = true;
#		endif
	}

#	ifndef _RELEASE
		ntError( mapValid );
#	endif

	return *theMap;
}

void DestroyHeresyBuffers()
{
	NT_DELETE_CHUNK(Mem::MC_GFX, theMap);
	theMap = NULL;

#ifndef _RELEASE
	mapValid = false;
#endif
}

CHeresyPushBuffers::CHeresyPushBuffers(CMeshInstance* meshInstance, bool registerInMap) 
	:  m_registerInMap(registerInMap)
	, m_refCount(1)
	, m_pMeshHeader(meshInstance->m_pobMeshHeader)
	, m_pDepthPushBufferHeader(NULL)
	, m_pRenderPushBufferHeader(NULL)
	, m_pShadowMapPushBufferHeader(NULL)
{
	if (registerInMap)
	{
		PBContainer&	pbMap = GetPBMap();

		m_ui32PBTag = ComputePBTag( m_pMeshHeader, meshInstance->IsShadowRecieving() );
		ntAssert (pbMap.find(m_ui32PBTag) == pbMap.end());
		pbMap.insert(PBContainer::value_type( m_ui32PBTag, this));
	}

	meshInstance -> BuildDepthPB(&m_pDepthPushBufferHeader);
	meshInstance -> BuildRenderPB(&m_pRenderPushBufferHeader);
	meshInstance -> BuildShadowMapPB(&m_pShadowMapPushBufferHeader);

}

CHeresyPushBuffers::~CHeresyPushBuffers()
{
	if (m_registerInMap)
	{
		PBContainer& pbMap = GetPBMap();
		pbMap.erase(m_ui32PBTag);
	}

	CMeshInstance::DestroyPushBuffer(m_pDepthPushBufferHeader);
	CMeshInstance::DestroyPushBuffer(m_pRenderPushBufferHeader);
	CMeshInstance::DestroyPushBuffer(m_pShadowMapPushBufferHeader);
}


CMeshInstance::CMeshInstance(	Transform const* pobTransform,
								CMeshHeader const* pobMeshHeader,
								bool bRenderOpaque, bool bShadowRecieve, bool bShadowCast,
								bool bCreatePushBuffers, unsigned int iRenderableType ) :
	CRenderable( pobTransform, bRenderOpaque, bShadowRecieve, bShadowCast, iRenderableType ), 
	m_pobMeshHeader( pobMeshHeader )
{
	if (m_pobMeshHeader->m_iFlags & ClumpFlags::IS_BLENDSHAPES_CAPABLE)
	{
		SetUseHeresy( false );
	}

	// create the game material
	CMaterial const* pobMaterial = ShaderManager::Get().FindMaterial( pobMeshHeader->m_obMaterialNameHash );


	if( !pobMaterial )
		pobMaterial = ShaderManager::Get().FindMaterial( ERROR_MATERIAL );
		

	ntError_p( pobMaterial, ("No possible material (including lambert) found!") );

	// count polys
	if( CRendererSettings::OptimiseMeshesOnLoad() )
	{
		m_meshType = PT_TRIANGLELIST;
		m_iPolyCount = m_pobMeshHeader->m_iNumberOfIndices / 3;
		m_iIndexCount = m_pobMeshHeader->m_iNumberOfIndices;
	}
	else
	{
		m_meshType = PT_TRIANGLESTRIP;
		m_iPolyCount = m_pobMeshHeader->m_iNumberOfIndices - 2;
		m_iIndexCount = m_pobMeshHeader->m_iNumberOfIndices;
	}

	if (bCreatePushBuffers)
		CreatePushBuffers();
}

CMeshInstance::~CMeshInstance()
{
	//if (m_heresyPushBuffers.IsValid())
	//	DeletePushBuffers();
	ReleaseAreaResources();
}

void CMeshInstance::CreatePushBuffers()
{
	//ntAssert_p( m_bPushBuffersValid == false, ("Already have valid push buffers") );
	ForceMaterial(0);
}


void CMeshInstance::CreateAreaResources( void )
{
	CreatePushBuffers();
}

void CMeshInstance::GetVertexAndIndexBufferHandles( void )
	{
		const CVertexStreamsContainter* pVBHandles = CClumpLoader::Get().RetrieveVBHandles( m_pobMeshHeader );
		if ( pVBHandles != NULL )
		{
			m_hVertexBuffer[0] = pVBHandles->GetVBHandle(0);
			m_hVertexBuffer[1] = pVBHandles->GetVBHandle(1);
		}
		else
		{
			ntAssert_p( m_hVertexBuffer, ("Can't retrieve valid VBs from the VBCache"));
			m_hVertexBuffer[0] = VBHandle();
			m_hVertexBuffer[1] = VBHandle();
		}

	m_hIndexBuffer = CClumpLoader::Get().RetrieveIBHandle( m_pobMeshHeader );
}

void CMeshInstance::ForceMaterial( CMaterial const* pobMaterial )
{
	if (m_heresyPushBuffers.IsValid())
		DeletePushBuffers();

	GetVertexAndIndexBufferHandles();

#ifdef PLATFORM_PS3
#ifdef _DEBUG
	{
		if (m_pobMeshHeader->m_iNumberOfVertexStreams == 1)
		{
			ntAssert_p( m_hVertexBuffer[0], ("Must have a valid vertex buffer at this point"));
		}
		else if (m_pobMeshHeader->m_iNumberOfVertexStreams == 2)
		{
			ntAssert_p( m_hVertexBuffer[0] && m_hVertexBuffer[1], ("Must have a valid vertex buffer at this point"));
		}
		ntAssert_p( m_hIndexBuffer, ("Must have a valid index buffer at this point"));
	}
#endif
#endif

	bool customMaterial = true;
	// force material has been called with 0, find our exported material
	if( !pobMaterial )
	{
		customMaterial = false;

		pobMaterial = ShaderManager::Get().FindMaterial( m_pobMeshHeader->m_obMaterialNameHash );

		if( !pobMaterial )
			pobMaterial = ShaderManager::Get().FindMaterial( ERROR_MATERIAL );
	}
	ntAssert( pobMaterial );

	MaterialDebug::RecordMaterialUsage( pobMaterial->GetName() );

	// set up the bounds (only here because of the material related bounds fudge)
	m_obBounds.Min() = CPoint(&m_pobMeshHeader->m_afAxisAlignedMin[0]);
	m_obBounds.Max() = CPoint(&m_pobMeshHeader->m_afAxisAlignedMax[0]);

#if defined( PLATFORM_PC )
	// see if we can use shader model 3 based effect materials instead of our old 
	// shader dictionary versions. This means we can have the additional lighting 
	// if they're present.
	FXMaterial* pLitMaterial = FXMaterialManager::Get().FindMaterial( m_pobMeshHeader->m_obMaterialNameHash );
	
	if (pLitMaterial)
	{
		m_pMaterial.Reset( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterialInstance( 
			pLitMaterial, 
			m_pobMeshHeader->m_pobVertexElements, 
			m_pobMeshHeader->m_iNumberOfVertexElements, 
			m_pobMeshHeader->m_pobMaterialProperties, 
			m_pobMeshHeader->m_iNumberOfProperties
		) );

		m_pMaterial->SetBoneIndices( m_pobMeshHeader->m_pucBoneIndices, m_pobMeshHeader->m_iNumberOfBonesUsed );

		if (m_pMaterial->IsAlphaBlended())
			m_bIsAlphaBlended = true;

		// FIXME: make the bounding volumes bigger for skinned meshes
		if( (m_pMaterial->GetBoundType() == VSTT_SKINNED) ||
			(m_pMaterial->GetBoundType() == VSTT_SKINNED) )
		{
			static CPoint const obFudgeFactor( 1.0f, 1.0f, 1.0f );
			m_obBounds.Min() -= obFudgeFactor;
			m_obBounds.Max() += obFudgeFactor;
		}
	}
	else
#endif // end PLATFORM_PC
	{
#ifdef PLATFORM_PS3
		CHashedString materialNameHash = pobMaterial->GetHashName();
		if ( materialNameHash == CHashedString("bsskin") ||
			materialNameHash == CHashedString("bsskin_w") )
		{
			char skinName[32];
			sprintf( skinName, "bsskin_%u", m_pobMeshHeader->m_obNameHash.Get() );
			m_pMaterial.Reset( NT_NEW_CHUNK(Mem::MC_GFX) BSSkin( 
				pobMaterial, 
				m_pobMeshHeader->m_pobVertexStreams->m_pobVertexElements, 
				m_pobMeshHeader->m_pobVertexStreams->m_iNumberOfVertexElements, 
				m_pobMeshHeader->m_pobMaterialProperties, 
				m_pobMeshHeader->m_iNumberOfProperties,
				skinName
			) );
		}
		else
		{
			m_pMaterial.Reset( NT_NEW_CHUNK(Mem::MC_GFX) CGameMaterialInstance( 
				pobMaterial, 
				m_pobMeshHeader->m_pobVertexStreams->m_pobVertexElements, 
				m_pobMeshHeader->m_pobVertexStreams->m_iNumberOfVertexElements, 
				m_pobMeshHeader->m_pobMaterialProperties, 
				m_pobMeshHeader->m_iNumberOfProperties
			) );
		}
#else // PLATFORM_PS3
		{
			m_pMaterial.Reset( NT_NEW_CHUNK(Mem::MC_GFX) CGameMaterialInstance( 
				pobMaterial, 
				m_pobMeshHeader->m_pobVertexElements, 
				m_pobMeshHeader->m_iNumberOfVertexElements, 
				m_pobMeshHeader->m_pobMaterialProperties, 
				m_pobMeshHeader->m_iNumberOfProperties
			) );
		}
#endif

		m_pMaterial->SetBoneIndices( m_pobMeshHeader->m_pucBoneIndices, m_pobMeshHeader->m_iNumberOfBonesUsed );

		// look to see if the material wants alpha blending
		if( m_pMaterial->IsAlphaBlended() )
			m_bIsAlphaBlended = true;

		// FIXME: make the bounding volumes bigger for skinned meshes
		if( (m_pMaterial->GetBoundType() == VSTT_SKINNED) ||
			(m_pMaterial->GetBoundType() == VSTT_SKINNED) )
		{
			static CPoint const obFudgeFactor( 1.0f, 1.0f, 1.0f );
			m_obBounds.Min() -= obFudgeFactor;
			m_obBounds.Max() += obFudgeFactor;
		}
	}

#if defined( PLATFORM_PS3 )
	if( CRendererSettings::bUseHeresy  && UseHeresy() )
	{
		if(customMaterial)
		{
			m_heresyPushBuffers = HeresyPushBuffersPtr(NT_NEW_CHUNK( Mem::MC_GFX ) CHeresyPushBuffers(this, false) );
		}
		else
		{
			m_heresyPushBuffers = GetPushBuffers();
		}
	}																   
#endif
	
}

HeresyPushBuffersPtr	CMeshInstance::GetPushBuffers()
{
	PBContainer::iterator	iter = GetPBMap().find( CHeresyPushBuffers::ComputePBTag( m_pobMeshHeader, IsShadowRecieving()) );

	if (iter != GetPBMap().end())
	{
		IntrusivePtrAddRef(iter -> second); 
		return HeresyPushBuffersPtr(iter -> second);

	}
	else
	{
		return HeresyPushBuffersPtr( NT_NEW_CHUNK( Mem::MC_GFX ) CHeresyPushBuffers(this, true) );
	}
}


void CMeshInstance::MaterialRollBack()
{
	if(m_pMaterialLast)
	{
		m_pMaterial.Swap(m_pMaterialLast);
		ForceFXMaterialFinalise();
		m_pMaterialLast.Reset(0);
	}
}

void CMeshInstance::ResetMaterial(MaterialInstanceBase* pMaterial)
{
	m_pMaterialLast.Reset(pMaterial);
	m_pMaterial.Swap(m_pMaterialLast);
}

void CMeshInstance::ForceFXMaterialFinalise()
{
#if defined( PLATFORM_PC )
	m_pMaterial->SetBoneIndices( m_pobMeshHeader->m_pucBoneIndices, m_pobMeshHeader->m_iNumberOfBonesUsed );

	if (m_pMaterial->IsAlphaBlended())
		m_bIsAlphaBlended = true;

	// FIXME: make the bounding volumes bigger for skinned meshes
	if( (m_pMaterial->GetBoundType() == VSTT_SKINNED) ||
		(m_pMaterial->GetBoundType() == VSTT_SKINNED) )
	{
		static CPoint const obFudgeFactor( 1.0f, 1.0f, 1.0f );
		m_obBounds.Min() -= obFudgeFactor;
		m_obBounds.Max() += obFudgeFactor;
	}
#endif
}

void CMeshInstance::ForceFXMaterial( const FXMaterial* pLitMaterial )
{
#if defined( PLATFORM_PC )
	if (pLitMaterial)
	{
		this->ResetMaterial( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterialInstance( 
			pLitMaterial, 
			m_pobMeshHeader->m_pobVertexElements, 
			m_pobMeshHeader->m_iNumberOfVertexElements, 
			m_pobMeshHeader->m_pobMaterialProperties, 
			m_pobMeshHeader->m_iNumberOfProperties
		) );
		
		ForceFXMaterialFinalise();
		
		//m_pMaterial->SetBoneIndices( m_pobMeshHeader->m_pucBoneIndices, m_pobMeshHeader->m_iNumberOfBonesUsed );

		//if (m_pMaterial->IsAlphaBlended())
		//	m_bIsAlphaBlended = true;

		//// FIXME: make the bounding volumes bigger for skinned meshes
		//if( (m_pMaterial->GetBoundType() == SHADERSLOT_SKINNEDVERTEX_SM2) ||
		//	(m_pMaterial->GetBoundType() == SHADERSLOT_SKINNEDVERTEX_SM3) )
		//{
		//	static CPoint const obFudgeFactor( 1.0f, 1.0f, 1.0f );
		//	m_obBounds.Min() -= obFudgeFactor;
		//	m_obBounds.Max() += obFudgeFactor;
		//}
	}
	else
	{
		this->ResetMaterial(0);
	}
#endif
}


const void* CMeshInstance::GetReconstructionMatrix() const
{
	const void* pReconstructionMatrix = NULL;
#ifdef PLATFORM_PS3
	// if position is compressed attach a decompression/reconstruction matrix
	if ( m_pobMeshHeader->IsPositionCompressed() )
		pReconstructionMatrix = static_cast<const void*>(m_pobMeshHeader->m_afReconstructionMatrix);
#endif
	return pReconstructionMatrix;
}


void CMeshInstance::RenderDepth()
{
	if( !m_heresyPushBuffers.IsValid() || m_heresyPushBuffers -> m_pDepthPushBufferHeader == 0 )
	{
#ifdef PLATFORM_PS3
		m_pMaterial->PreRenderDepth( m_pobTransform, IsShadowRecieving(), GetReconstructionMatrix() );
#else
		m_pMaterial->PreRenderDepth( m_pobTransform, IsShadowRecieving() );
#endif
		RenderMesh();
		m_pMaterial->PostRenderDepth( false );
	} else
	{
		RenderPB( m_heresyPushBuffers -> m_pDepthPushBufferHeader );
	}
}


void CMeshInstance::RenderMaterial()
{
	if( !m_heresyPushBuffers.IsValid() || m_heresyPushBuffers -> m_pRenderPushBufferHeader == 0 )
	{
#ifdef PLATFORM_PS3
        m_pMaterial->PreRender( m_pobTransform, IsShadowRecieving(), GetReconstructionMatrix() );
#else
		m_pMaterial->PreRender( m_pobTransform, IsShadowRecieving() );
#endif
		RenderMesh();
		m_pMaterial->PostRender();
	} else
	{
			RenderPB( m_heresyPushBuffers -> m_pRenderPushBufferHeader );
	}	
}

void CMeshInstance::RenderShadowOnly()
{
	m_pMaterial->PreRenderShadowRecieve( m_pobTransform );
	RenderMesh();
	m_pMaterial->PostRenderShadowRecieve();
}

const Heresy_PushBuffer* CMeshInstance::GetDepthPushBuffer(void)
{
	if (m_heresyPushBuffers.IsValid())
	{
		return m_heresyPushBuffers -> m_pDepthPushBufferHeader;
	}
	else
	{
		return NULL;
	}
}
const Heresy_PushBuffer* CMeshInstance::GetRenderPushBuffer(void)
{
	if (m_heresyPushBuffers.IsValid())
	{
		return m_heresyPushBuffers -> m_pRenderPushBufferHeader;
	}
	else
	{
		return NULL;
	}
}
const Heresy_PushBuffer* CMeshInstance::GetShadowMapPushBuffer(void)
{
	if (m_heresyPushBuffers.IsValid())
	{
		return m_heresyPushBuffers -> m_pShadowMapPushBufferHeader;
	}
	else
	{
		return NULL;
	}
}
 
///--------------------------------
