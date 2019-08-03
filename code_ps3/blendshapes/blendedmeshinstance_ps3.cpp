
#include "blendshapes/BlendedMeshInstance.h"
#include "blendshapes/BlendShapes.h"
#include "blendshapes/shading/bsskin.h"
#include "blendshapes/xpushapeblending.h"
#include "blendshapes/blendshapesbatchinfo_spu_ppu.h"
#include "gfx/vertexdeclaration.h"
#include "gfx/renderer.h"
#include "gfx/clump.h"

VBHandle CreateMyVertexBuffer( const CMeshHeader* pMeshHeader, uint32_t uiStreamIndex, Gc::BufferType bufferType );
int FindPosStream( const CMeshHeader* pMeshHeader );


//static_assert_in_class( sizeof(CMatrix) == sizeof(float)*16, Sizeof_StreamReconstructionMatrix_Mismatch );


//--------------------------------------------------
//!
//!	Patched newly animated wrinkle area weights straight into
//! its mesh's material properties
//!
//--------------------------------------------------
void BlendedMeshInstance::PatchWrinkleWeights( void ) const
{
	if ( m_pobMeshHeader->m_obMaterialNameHash == CHashedString("bsskin_w") && m_pBlendShapes )
	{
		if ( m_apWrinkleWeightsPropertiesPtrs[0] )
			NT_MEMCPY( m_apWrinkleWeightsPropertiesPtrs[0]->GetFloatData().afFloats, m_pBlendShapes->GetWrinkleAreaWeights(), 4*sizeof(float) );
		
		if ( m_apWrinkleWeightsPropertiesPtrs[1] )
			NT_MEMCPY( m_apWrinkleWeightsPropertiesPtrs[1]->GetFloatData().afFloats, m_pBlendShapes->GetWrinkleAreaWeights() + 4, 4*sizeof(float) );
	}
}

//--------------------------------------------------
//!
//!	Gets the corresponding CMaterialProperties ptr
//! from the mesh header so we don't have to ask for
//! them each time. Refreshed on ForceMaterial calls
//!
//--------------------------------------------------
void BlendedMeshInstance::CacheWrinkleWeightsPropertiesPtrs( void ) const
{
	// this is lame. It should change once the MaterialIntance class hierarchy is flattened
	// note that we could also ask for the material name hash in the mesh header but this is not
	// guaranteed to be consistent with our current material instance ptr
	CMaterialInstance* pobMaterialInstance = reinterpret_cast<CMaterialInstance*>( m_pMaterial.Get() );
	if ( pobMaterialInstance && pobMaterialInstance->GetMaterial()->GetHashName() == CHashedString("bsskin_w") )
	{
		BSSkin* pBSSkin = reinterpret_cast<BSSkin*>( pobMaterialInstance );

		//
		// const_cast here. So, sue me...
		//
		CMaterialProperty* pMatProp0 = const_cast<CMaterialProperty*>( pBSSkin->GetMaterialProperty( PROPERTY_BSSKIN_WRINKLE_REGIONS0_WEIGHTS) );
		CMaterialProperty* pMatProp1 = const_cast<CMaterialProperty*>( pBSSkin->GetMaterialProperty( PROPERTY_BSSKIN_WRINKLE_REGIONS1_WEIGHTS) );

		user_warn_p( pMatProp0 && pMatProp1, ("BSSkin: Wrinkle area weight properties not found!\n") );

		m_apWrinkleWeightsPropertiesPtrs[0] = pMatProp0;
		m_apWrinkleWeightsPropertiesPtrs[1] = pMatProp1;
		
	}
	else 
	{
		m_apWrinkleWeightsPropertiesPtrs[0] = 0;
		m_apWrinkleWeightsPropertiesPtrs[1] = 0;
	}
}


void BlendedMeshInstance::CacheVertexStreamReconstructionMatrices( void )
{
	if ( GetMeshHeader()->IsPositionCompressed() )
	{
		m_obStreamMatrices[0] = CMatrix(GetMeshHeader()->m_afReconstructionMatrix);	
		m_obStreamMatrices[1] = m_obStreamMatrices[0].GetFullInverse();
	}
	else
	{
		m_obStreamMatrices[0].SetIdentity();
		m_obStreamMatrices[1].SetIdentity();
	}
}


void BlendedMeshInstance::ForceMaterial( CMaterial const* pobMaterial )
{
	CMeshInstance::ForceMaterial( pobMaterial );
	CacheWrinkleWeightsPropertiesPtrs();
}

BlendedMeshInstance::BlendedMeshInstance(	Transform const* pobTransform, CMeshHeader const* pobMeshHeader,
											bool bRenderOpaque, bool bShadowRecieve, bool bShadowCast ) :
	CMeshInstance( pobTransform, pobMeshHeader, bRenderOpaque, bShadowRecieve, bShadowCast, false, RT_BS_MESH_INSTANCE ), 
	m_pBlendShapes( 0 ),
	m_bHasAreaResources( false ),
	m_pSpuAdditionalInfo( 0 )
{
	// remember to set blendshape flag
	SetIsShapeBlended( true );
	ntAssert_p( !UseHeresy(), ("BlendedMeshInstances cannot use Heresy for the time being. Stay tuned") );

	memset( m_aVramPtrs, 0, m_cNumOfBuffers * sizeof(void*) );
	memset( m_apWrinkleWeightsPropertiesPtrs, 0, 2 * sizeof(void*) );
	memset( m_obStreamMatrices, 0, 2 * sizeof(CMatrix) );

	//if ( pobMeshHeader->IsPositionCompressed() )
	//{
	//	m_obStreamMatrices[0] = CMatrix(pobMeshHeader->m_afReconstructionMatrix);	
	//	m_obStreamMatrices[1] = m_obStreamMatrices[0].GetAffineInverse();
	//}
	//else
	//{
	//	m_obStreamMatrices[0].SetIdentity();
	//	m_obStreamMatrices[1].SetIdentity();
	//}

	//// call this for the 1st time. Later material changes are handled by ForceMaterial()
	//CacheWrinkleWeightsPropertiesPtrs();

	//InitUserVram( GetMeshHeader()->m_pobVertexStreams->m_iNumberOfVertices * GetMeshHeader()->m_pobVertexStreams->m_iVertexStride );

	//BuildVertexBuffers();

	// register with blendshapes manager
	//XPUShapeBlending::Get().Register( this );


}

BlendedMeshInstance::~BlendedMeshInstance( void )
{
	ReleaseAreaResources();
}

void BlendedMeshInstance::CreateAreaResources( void )
{
	if ( !m_bHasAreaResources )
	{
		CMeshInstance::CreateAreaResources();

		// these should be available by now
		CacheWrinkleWeightsPropertiesPtrs();
		CacheVertexStreamReconstructionMatrices();

		m_pSpuAdditionalInfo = (BSSpuAdditionalInfo*) NT_MEMALIGN_CHUNK(Mem::MC_GFX, sizeof(BSSpuAdditionalInfo), 128 );

		XPUShapeBlending::Get().Register( this );

		m_bHasAreaResources = true;
	}
}

void BlendedMeshInstance::ReleaseAreaResources( void )
{
	if ( m_bHasAreaResources )
	{
		CMeshInstance::ReleaseAreaResources();

		memset( m_apWrinkleWeightsPropertiesPtrs, 0, 2 * sizeof(void*) );
		memset( m_obStreamMatrices, 0, 2 * sizeof(CMatrix) );

		if ( m_pSpuAdditionalInfo )
		{
			NT_FREE_CHUNK(Mem::MC_GFX, (uintptr_t)m_pSpuAdditionalInfo);
			m_pSpuAdditionalInfo = 0;
		}

		XPUShapeBlending::Get().Unregister( this );
		FreeUserVram();

		m_bHasAreaResources = false;
	}
}


BSSpuAdditionalInfo* BlendedMeshInstance::GetSpuAdditionalInfo( void )
{
	ntError_p( HasBlendShapes(), ("BS - this shouldn't happen\n") );

	m_pSpuAdditionalInfo->m_iNumOfTargets = GetBlendShapes()->GetNumOfBSTargets();
	m_pSpuAdditionalInfo->m_iVertexStride = m_hVertexBuffer[0]->GetStride();
	m_pSpuAdditionalInfo->m_iNumOfVertices = m_hVertexBuffer[0]->GetCount();

	return m_pSpuAdditionalInfo;
}

void* BlendedMeshInstance::InitUserVram( size_t bufferSize )
{
	//! align buffer size. This should probably be checked when exporting
	size_t alignedBufferSize = Util::Align( bufferSize, 128 );
	size_t totalSize = alignedBufferSize * m_cNumOfBuffers;

	//! allocate enough memory for all the work buffers in one big block
	uint8_t* pVram = reinterpret_cast<uint8_t*>( NT_MEMALIGN_CHUNK( Mem::MC_RSX_MAIN_USER, totalSize, 128 ) );
	ntError_p( pVram, ("Vram allocation failed!\n") );

	memset( pVram, 0, totalSize );

	//! init vram ptrs array for multi-buffering
	for( int iBuffer = 0 ; iBuffer < m_cNumOfBuffers ; ++iBuffer )
	{
		m_aVramPtrs[ iBuffer ] = pVram + iBuffer * alignedBufferSize;
	}

	return pVram;
}


//--------------------------------------------------
//!
//!	Free user vram
//! Note that, since it was allocated as one big 
//! block, we only release the start address
//!
//--------------------------------------------------
void BlendedMeshInstance::FreeUserVram( void )
{
	if ( m_aVramPtrs[0] )
	{
		NT_FREE_CHUNK( Mem::MC_RSX_MAIN_USER, (uintptr_t)m_aVramPtrs[0] );
		memset( m_aVramPtrs, 0,  m_cNumOfBuffers * sizeof(void*) );
	}
}




MeshBSSetPtr_t BlendedMeshInstance::SetBlendShapes( MeshBSSetPtr_t pMeshBlendShapes )
{
	ntAssert( pMeshBlendShapes );
	ntError_p( IsCompatible( pMeshBlendShapes ), ("Blendshape set(%u) is not incompatible with mesh(%u)", pMeshBlendShapes->GetMeshNameHash(), \
																									GetMeshHeader()->m_obNameHash.Get()  ) );
	
	MeshBSSetPtr_t pPrevBlendShapes = m_pBlendShapes;
	m_pBlendShapes = pMeshBlendShapes;
	return pPrevBlendShapes;
}


MeshBSSetPtr_t BlendedMeshInstance::RemoveBlendShapes( void )
{
	MeshBSSetPtr_t pPrevBlendShapes = m_pBlendShapes;
	m_pBlendShapes = 0;
	return pPrevBlendShapes;
}

//void BlendedMeshInstance::RenderMesh() const
//{
//	PatchWrinkleWeights();
//	CMeshInstance::RenderMesh();
//}


void BlendedMeshInstance::RenderMesh() const
{
	ntAssert( m_bHasAreaResources );

	// write the new weight properties into the header
	PatchWrinkleWeights();

	Renderer::Get().m_Platform.SetStream( m_hVertexBuffer[0] );

	if ( m_hVertexBuffer[1] )
		Renderer::Get().m_Platform.SetStream( m_hVertexBuffer[1] );

	// draw primitives
	Renderer::Get().m_Platform.DrawIndexedPrimitives( ConvertPRIMTYPEToGCPRIM(m_meshType), 0, m_iIndexCount, m_hIndexBuffer );
	Renderer::Get().m_Platform.ClearStreams();
}

bool BlendedMeshInstance::IsCompatible( MeshBSSetPtr_t pBlendShapes )
{
	return ( pBlendShapes->GetMeshNameHash() == GetMeshHeader()->m_obNameHash.Get() );
}




//--------------------------------------------------
//!
//!	Resets the contents of the vbuffer with the original
//! data in the mesh header. This *MUST* be called before
//! rendering or writing to vram
//!
//--------------------------------------------------
void BlendedMeshInstance::ResetVertexBuffer( void )
{	
	if ( m_bHasAreaResources && IsRendering() )
	{
	// get the next one
		m_bufferIndex++;
		m_hVertexBuffer[0]->SetDataAddress( m_aVramPtrs[ m_bufferIndex ] );

		//! fill with original data
		m_hVertexBuffer[0]->Write( m_pobMeshHeader->m_pobVertexStreams->m_pvVertexBufferData );
	}
}


//--------------------------------------------------
//!
//!	returns the write-to address of the vbuffer for this 
//! frame. Note that this requires scratch vram to be set
//! beforehand if using that memory mode
//!
//--------------------------------------------------
void* BlendedMeshInstance::GetVertexBufferWriteAddress( void ) 
{
	return m_hVertexBuffer[0]->GetDataAddress();
}


void BlendedMeshInstance::BuildVertexBuffers( void )
{
//	ntAssert_p( !m_bVertexBuffersReady, ("I already have valid vertex buffers!\n") );
	ntAssert( GetMeshHeader() );
	ntAssert( GetMeshHeader()->m_iNumberOfVertexStreams > 0 &&  GetMeshHeader()->m_pobVertexStreams );
	
//	if ( !m_bVertexBuffersReady )
	{
		int iPosStreamIndex = FindPosStream( GetMeshHeader() );
		ntAssert( iPosStreamIndex == 0 );

		FreeUserVram();
		InitUserVram( GetMeshHeader()->m_pobVertexStreams[(uint32_t)iPosStreamIndex].m_iNumberOfVertices * GetMeshHeader()->m_pobVertexStreams[(uint32_t)iPosStreamIndex].m_iVertexStride );
		
		m_hVertexBuffer[iPosStreamIndex] = CreateMyVertexBuffer( GetMeshHeader(), 0, Gc::kUserBuffer );
		m_hVertexBuffer[iPosStreamIndex]->SetDataAddress( m_aVramPtrs[ m_bufferIndex ] );
		m_hVertexBuffer[iPosStreamIndex]->Write( GetMeshHeader()->m_pobVertexStreams[(uint32_t)iPosStreamIndex].m_pvVertexBufferData );
			
		if ( GetMeshHeader()->m_iNumberOfVertexStreams > 1 )
		{
			m_hVertexBuffer[1] = CreateMyVertexBuffer( GetMeshHeader(), 1, Gc::kStaticBuffer );
			m_hVertexBuffer[1]->Write( m_pobMeshHeader->m_pobVertexStreams[(uint32_t)1].m_pvVertexBufferData );
		}
	}
}





VBHandle CreateMyVertexBuffer( const CMeshHeader* pMeshHeader, uint32_t uiStreamIndex, Gc::BufferType bufferType )
{

	static const int MAX_STREAM_SEMANTICS = 32;
	GcStreamField	aStreamElements[MAX_STREAM_SEMANTICS];

	ntAssert_p( pMeshHeader->m_pobVertexStreams[uiStreamIndex].m_iNumberOfVertexElements < MAX_STREAM_SEMANTICS, ("too many vertex elements, increase MAX_STREAM_SEMANTICS\n") );

	// pointer to the first vertex stream
	CVertexStream* pVertexStream = pMeshHeader->m_pobVertexStreams + uiStreamIndex;

	for( int i = 0;i < pVertexStream->m_iNumberOfVertexElements;i++)
	{
		uint32_t idx = i;
		aStreamElements[i] = GcStreamField( GetSemanticName(	pVertexStream->m_pobVertexElements[idx].m_eStreamSemanticTag ),
																pVertexStream->m_pobVertexElements[idx].m_iOffset,
											GetStreamType(		pVertexStream->m_pobVertexElements[idx].m_eType ),
											GetStreamElements(	pVertexStream->m_pobVertexElements[idx].m_eType ),
											IsTypeNormalised(	pVertexStream->m_pobVertexElements[idx].m_eType ) );
	}
	
	VBHandle pobBuffer = RendererPlatform::CreateVertexStream(	pVertexStream->m_iNumberOfVertices, 
																pVertexStream->m_iVertexStride,
																pVertexStream->m_iNumberOfVertexElements,
																aStreamElements,
																bufferType );

	// return the buffer
	return pobBuffer;
}


void BlendedMeshInstance::GetVertexAndIndexBufferHandles( void )
{
	BuildVertexBuffers();
	m_hIndexBuffer = CClumpLoader::Get().RetrieveIBHandle( m_pobMeshHeader );
}

int FindPosStream( const CMeshHeader* pMeshHeader )
{
	for ( u_int iStream = 0; iStream < (u_int)pMeshHeader->m_iNumberOfVertexStreams; ++iStream )
	{
		for ( u_int iElem = 0; iElem < (u_int)pMeshHeader->m_pobVertexStreams[iStream].m_iNumberOfVertexElements; ++iElem )
		{
			if ( pMeshHeader->m_pobVertexStreams[iStream].m_pobVertexElements[iElem].m_eStreamSemanticTag == STREAM_POSITION )
			{
				return iStream;
			}
		}
	}
	return -1;
}



// eof


