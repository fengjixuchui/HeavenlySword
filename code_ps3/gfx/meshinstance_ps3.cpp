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

#include "anim/transform.h"
#include "gfx/meshinstance.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "gfx/rendercontext.h"
#include "gfx/heresypushbuffers.h"

void CMeshInstance::RenderShadowMap()
{
	// Get world matrix
	CMatrix mWorldMatrix =  m_pobTransform->GetWorldMatrixFast();
	// if position is compressed attach a decompression/reconstruction matrix
	CMatrix mReconstuction;
	const void* pMatrix = GetReconstructionMatrix();

	if (pMatrix != NULL)
		// I can't use the overloaded assignment operator cause the source matrix from CMeshHeader is not aligned
		NT_MEMCPY (&mReconstuction, pMatrix, sizeof(CMatrix));
	else
		mReconstuction.SetIdentity();

	// this flag let me know if we have to set one or two matrices in the depth vertex shader
	bool bSetReconstMatrix = ( m_pobMeshHeader->IsPositionCompressed() && ( GetMaterialInstance()->GetBoundType() == VSTT_SKINNED) ) ;

	// if a reconstruction matrix is not needed we can just patch the default projection matrix
	if ( !bSetReconstMatrix )
		mWorldMatrix = mReconstuction * mWorldMatrix;

	if( !m_heresyPushBuffers.IsValid() || m_heresyPushBuffers -> m_pShadowMapPushBufferHeader == 0 )
	{
		m_pMaterial->PreRenderDepth( m_pobTransform, true );

		// note! pVertexShader MUST have been set by PreRenderDepth(), so this is okay, if kinda hokey
		Shader* pVertexShader = Renderer::Get().GetVertexShader();

		// set up streams
		Renderer::Get().m_Platform.SetStream( m_hVertexBuffer[0] );
		if ( m_hVertexBuffer[1] )
			Renderer::Get().m_Platform.SetStream( m_hVertexBuffer[1] );

		// setting un a projection matrix
		u32 index;
		// ****************************************************************************************************
		// we have to handle 4 different cases:
		// 1a) Static Mesh  / No Compressed Position-> vertices are transformed by the standard projection matrix
		// 1b) Static Mesh  / Compressed Position	-> vertices are transformed by a patched (premultiplied reconstruction matrix) projection matrix
		// 2a) Skinned Mesh / No Compressed Position-> vertices are skinned and then transformed by the standard projection matrix
		// 2b) Skinned Mesh / Compressed Position	-> vertices are reconstructed, skinned and then transformed by the standard projection matrix
		if ( bSetReconstMatrix )
		{
			u32 recon_index = pVertexShader->GetConstantIndex( "pos_reconstruction_mat" );
			pVertexShader->SetVSConstant( recon_index, mReconstuction );

			index = pVertexShader->GetConstantIndex( "projection_noreconst_mat" );
		}
		else index = pVertexShader->GetConstantIndex( "projection" );

		if( m_FrameFlags & RFF_CAST_SHADOWMAP0 )
		{
			pVertexShader->SetVSConstant( index, mWorldMatrix * RenderingContext::Get()->m_shadowMapProjection[0] );

			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[0] );

			// draw primitives
			Renderer::Get().m_Platform.DrawIndexedPrimitives( ConvertPRIMTYPEToGCPRIM(m_meshType), 0, m_iIndexCount, m_hIndexBuffer );
		}
		if( m_FrameFlags & RFF_CAST_SHADOWMAP1 )
		{
			pVertexShader->SetVSConstant( index, mWorldMatrix * RenderingContext::Get()->m_shadowMapProjection[1] );

			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[1] );

			// draw primitives
			Renderer::Get().m_Platform.DrawIndexedPrimitives( ConvertPRIMTYPEToGCPRIM(m_meshType), 0, m_iIndexCount, m_hIndexBuffer );
		}
		if( m_FrameFlags & RFF_CAST_SHADOWMAP2 )
		{
			pVertexShader->SetVSConstant( index, mWorldMatrix * RenderingContext::Get()->m_shadowMapProjection[2] );

			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[2] );

			// draw primitives
			Renderer::Get().m_Platform.DrawIndexedPrimitives( ConvertPRIMTYPEToGCPRIM(m_meshType), 0, m_iIndexCount, m_hIndexBuffer );
		}
		if( m_FrameFlags & RFF_CAST_SHADOWMAP3 )
		{
			pVertexShader->SetVSConstant( index, mWorldMatrix * RenderingContext::Get()->m_shadowMapProjection[3] );

			Renderer::Get().SetScissorRegion( &RenderingContext::Get()->m_shadowScissor[3] );

			// draw primitives
			Renderer::Get().m_Platform.DrawIndexedPrimitives( ConvertPRIMTYPEToGCPRIM(m_meshType), 0, m_iIndexCount, m_hIndexBuffer );
		}

		// clear streams
		Renderer::Get().m_Platform.ClearStreams();
		m_pMaterial->PostRenderDepth( true );
	} else
	{

		// warning this is really a dodgy case because the fixup lists will be invalid for a Gc context...
		// but are weren't aren't doing any fix ups... its okay as long as you wear sensible shoes...
		Heresy_PushBuffer* pMainPB = (Heresy_PushBuffer*)&GcKernel::GetContext();
		uint32_t* pStart = pMainPB->m_pCurrent;
		RenderPB( m_heresyPushBuffers -> m_pShadowMapPushBufferHeader );

		// now fix up the main push to not skip etc. i
		// change skip and set the parameters
		uint32_t flag = RFF_CAST_SHADOWMAP0;
		for( uint16_t i=0;i < 4;i++, flag<<=1 )
		{
			if( m_FrameFlags & flag )
			{
				ntAssert( m_heresyPushBuffers -> m_pShadowMapPushBufferHeader ->m_iMarker[i] != 0xFFFF );

				uint32_t* pPB = pStart + m_heresyPushBuffers -> m_pShadowMapPushBufferHeader->m_iMarker[i];
				CMatrix mat = mWorldMatrix * RenderingContext::Get()->m_shadowMapProjection[i];
				Heresy_Set32bit( pPB, Heresy_Cmd( RSX_NOOP, 0) | HPBC_NOINCREMENT );
				pPB += (1 + 2); // skip NOP + skip set vertex constant command and constant num
				Heresy_CopyN128bits( pPB, mat.operator const float *(), 4 );
				pPB += (16 + 1); // skip 4 quad words + set scissor rect command
				uint16_t iX = RenderingContext::Get()->m_shadowScissor[i].left;
				uint16_t iY = RenderingContext::Get()->m_shadowScissor[i].top;
				uint16_t iWidth = RenderingContext::Get()->m_shadowScissor[i].right - RenderingContext::Get()->m_shadowScissor[i].left;
				uint16_t iHeight = RenderingContext::Get()->m_shadowScissor[i].bottom - RenderingContext::Get()->m_shadowScissor[i].top;
				*pPB = Heresy_ScissorHorizontal( iX, iWidth		);
				*(pPB+1) = Heresy_ScissorVertical( iY, iHeight );
			}	
		}

	}
}

void CMeshInstance::RenderMesh() const
{
	Renderer::Get().m_Platform.SetStream( m_hVertexBuffer[0] );
	if ( m_hVertexBuffer[1] )
		Renderer::Get().m_Platform.SetStream( m_hVertexBuffer[1] );

	// draw primitives
	Renderer::Get().m_Platform.DrawIndexedPrimitives( ConvertPRIMTYPEToGCPRIM(m_meshType), 0, m_iIndexCount, m_hIndexBuffer );

	Renderer::Get().m_Platform.ClearStreams();
}


void CMeshInstance::RenderPB( restrict Heresy_PushBuffer* pPB )
{
	ntAssert_p( m_heresyPushBuffers.IsValid(), ("MUST have valid push buffers by now...") );

	CGameMaterialInstance::MATERIAL_DATA_CACHE stCache;
	stCache.pobTransform = m_pobTransform;
	stCache.obObjectToWorld = m_pobTransform->GetWorldMatrixFast();
	stCache.obWorldToObject = stCache.obObjectToWorld.GetAffineInverse();

	// if position is compressed attach a decompression/reconstruction matrix
	const void* pReconstuction = GetReconstructionMatrix();
	if (pReconstuction)
		// I can't use the overloaded assignment operator cause the source matrix from CMeshHeader is not aligned
		NT_MEMCPY (&stCache.obReconstructionMatrix, pReconstuction, sizeof(CMatrix));
	else
		stCache.obReconstructionMatrix.SetIdentity();

	if ( !m_pobMeshHeader->IsPositionCompressed() || ( GetMaterialInstance()->GetBoundType() == VSTT_SKINNED) ) 
		stCache.bApplyPosReconstMatrix = false;
	else
		stCache.bApplyPosReconstMatrix = true;
	
	CGameMaterialInstance* pMaterial = (CGameMaterialInstance*)m_pMaterial.Get();
	pMaterial->PatchProperties( pPB, stCache );

	// warning this is really a dodgy case because the fixup lists will be invalid for a Gc context...
	// but are weren't aren't doing any fix ups... its okay as long as you wear sensible shoes...
	Heresy_PushBuffer* pMainPB = (Heresy_PushBuffer*)&GcKernel::GetContext();
	uint32_t iSize = (pPB->m_pCurrent - pPB->m_pStart);
	NT_MEMCPY( pMainPB->m_pCurrent, pPB->m_pStart, iSize * 4 );

	// do the pushbuffer fixup (if path properties was done inline this could be done there...)
	restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (pPB+1);
	for( uint16_t i = 0; i < pPB->m_iNumPatches;i++, pPatch++ )
	{
		if( (pPatch->m_Semantic & Heresy_PushBufferInlineFixupToken) == Heresy_PushBufferInlineFixupToken )
		{
			Heresy_FixupPixelShaderInline( RenderingContext::ms_pHeresyGlobal, pMainPB->m_pCurrent, pPatch );
		}
	}

//		Renderer::Get().m_Platform.DumpPushBuffer( pMainPB->m_pCurrent, pMainPB->m_pCurrent + iSize, "/app_home/content_ps3/pbtest.bin" );

	Heresy_IncPB( pMainPB, iSize );

#if defined( _PROFILING )
	Renderer::Get().m_renderCount.AddTriStrip( m_iIndexCount );
#endif

}

void CMeshInstance::BuildDepthPB(Heresy_PushBuffer** pushBuffer)
{
	// one day this will be done offline... so the structure is a bit weird, I do it all to a stack fake push buffer...
	static const uint32_t iMaxSize = 240*1024/4;
	static const uint32_t iMaxPatchSize = 100;
	static const uint32_t iMaxScratchSize = 32*1024;
	uint32_t* pFakePBBuffer = (uint32_t*) NT_ALLOC_CHUNK( Mem::MC_MISC, iMaxSize );
	Heresy_PushBuffer *pFakePB = (Heresy_PushBuffer *)NT_MEMALIGN_CHUNK( Mem::MC_MISC, sizeof(Heresy_PushBuffer) + (sizeof(Heresy_PushBufferPatch) * iMaxPatchSize) + iMaxScratchSize, 0x80 );

	Heresy_InitPushBuffer( pFakePB, pFakePBBuffer, pFakePBBuffer + iMaxSize, iMaxPatchSize, iMaxScratchSize );
	bool pbCreationOk;

//	Heresy_InsertDebugMarker( pFakePB, "Deano001" );
	// on PS3 I think all materials are CGameMaterialInstance, if so... I vote we flatten the hierachy...
	ntAssert( m_pMaterial->isFX() == false );
	CGameMaterialInstance* pMaterial = (CGameMaterialInstance*)m_pMaterial.Get();

	pbCreationOk = pMaterial->BuildPreRenderDepthPB( pFakePB, m_hVertexBuffer, false );
	if( pbCreationOk == false )
	{
		goto FreeDebugMem;
	}

	// actually do the render
	Heresy_SetIndexArrayOffsetAndType( pFakePB, m_hIndexBuffer->GetDataOffset(), Heresy_IndexArrayType( HIT_16_BIT, HRL_LOCAL_GDDR ) );
	Heresy_DrawIndexPrims( pFakePB, HPT_TRIANGLE_STRIP, 0, m_iIndexCount );
	pbCreationOk = pMaterial->BuildPostRenderDepthPB( pFakePB, m_hVertexBuffer, false );
	if( pbCreationOk == false )
	{
		goto FreeDebugMem;
	}
	// okay we now have a valid depth push buffer, so we allocate some RAM and copy our fake push buffer into it
	// where it will be ready to go

	// NOTE currently always have max patches space... due to me being stoopid
	// calc the size of the actual fix up list and the header
	uint32_t iPBHeaderSize =	sizeof(Heresy_PushBuffer) + 
								pFakePB->m_iMaxPatches * sizeof( Heresy_PushBufferPatch ) + 
								pFakePB->m_iCurScratch * sizeof(uint32_t);
	*pushBuffer = (Heresy_PushBuffer*) NT_MEMALIGN_CHUNK( Mem::MC_GFX, iPBHeaderSize, 0x80 );
	NT_MEMCPY( *pushBuffer, pFakePB, iPBHeaderSize );
	// now the actual pushbuffer
	uint32_t iPBSize = (pFakePB->m_pCurrent - pFakePB->m_pStart);
	(*pushBuffer)->m_pStart = (uint32_t*) NT_MEMALIGN_CHUNK( Mem::MC_GFX, iPBSize*4, 0x80 );
	NT_MEMCPY((*pushBuffer)->m_pStart, pFakePB->m_pStart, iPBSize*4 );
	//ntPrintf( Debug::DCU_RESOURCES, "Total Push Buffer Size : %i\n", (iPBHeaderSize + iPBSize*4) );
	// just to ensure nobody trys to fiddle with it and so we can calculate the size.
	(*pushBuffer)->m_pCurrent = (*pushBuffer)->m_pEnd = (*pushBuffer)->m_pStart + iPBSize;
	(*pushBuffer)->m_iMaxScratch = (*pushBuffer)->m_iCurScratch;
//	m_pDepthPushBufferHeader->m_iMaxPatches = m_pDepthPushBufferHeader->m_iNumPatches;

FreeDebugMem:
	NT_FREE_CHUNK( Mem::MC_MISC, (uintptr_t)pFakePBBuffer );
	NT_FREE_CHUNK( Mem::MC_MISC, (uintptr_t)pFakePB );
}

void CMeshInstance::BuildRenderPB(Heresy_PushBuffer** pushBuffer)
{
	// one day this will be done offline... so the structure is a bit weird, I do it all to a stack fake push buffer...
	static const uint32_t iMaxSize = 240*1024/4;
	static const uint32_t iMaxPatchSize = 100;
	static const uint32_t iMaxScratchSize = 32*1024;
	uint32_t* pFakePBBuffer = (uint32_t*) NT_ALLOC_CHUNK( Mem::MC_MISC, iMaxSize );
	Heresy_PushBuffer *pFakePB = (Heresy_PushBuffer *)NT_MEMALIGN_CHUNK( Mem::MC_MISC, sizeof(Heresy_PushBuffer) + (sizeof(Heresy_PushBufferPatch) * iMaxPatchSize) + iMaxScratchSize, 0x80 );

	Heresy_InitPushBuffer( pFakePB, pFakePBBuffer, pFakePBBuffer + iMaxSize, iMaxPatchSize, iMaxScratchSize );
	bool pbCreationOk;

//	Heresy_InsertDebugMarker( pFakePB, "Deano001" );
	// on PS3 I think all materials are CGameMaterialInstance, if so... I vote we flatten the hierachy...
	ntAssert( m_pMaterial->isFX() == false );
	CGameMaterialInstance* pMaterial = (CGameMaterialInstance*)m_pMaterial.Get();

	pbCreationOk = pMaterial->BuildPreRenderPB( pFakePB, m_hVertexBuffer, 2, IsShadowRecieving() );
	if( pbCreationOk == false )
	{
		goto FreeDebugMem;
	}

	// actually do the render
	Heresy_SetIndexArrayOffsetAndType( pFakePB, m_hIndexBuffer->GetDataOffset(), Heresy_IndexArrayType( HIT_16_BIT, HRL_LOCAL_GDDR ) );
	
	if( CRendererSettings::bShowWireframe )
	{
		Heresy_SetFrontPolygonMode( pFakePB, HPM_LINE );
		Heresy_SetBackPolygonMode( pFakePB, HPM_LINE );
	}

	Heresy_DrawIndexPrims( pFakePB, HPT_TRIANGLE_STRIP, 0, m_iIndexCount );
	if( CRendererSettings::bShowWireframe )
	{
		Heresy_SetBackPolygonMode( pFakePB, HPM_FILL );
		Heresy_SetFrontPolygonMode( pFakePB, HPM_FILL );
	}

	pbCreationOk = pMaterial->BuildPostRenderPB( pFakePB, m_hVertexBuffer, 2 );
	if( pbCreationOk == false )
	{
		goto FreeDebugMem;
	}

	// okay we now have a valid depth push buffer, so we allocate some RAM and copy our fake push buffer into it
	// where it will be ready to go

	// NOTE currently always have max patches space... due to me being stoopid
	// calc the size of the actual fix up list and the header
	uint32_t iPBHeaderSize =	sizeof(Heresy_PushBuffer) + 
								pFakePB->m_iMaxPatches * sizeof( Heresy_PushBufferPatch ) + 
								pFakePB->m_iCurScratch * sizeof(uint32_t);
	*pushBuffer = (Heresy_PushBuffer*) NT_MEMALIGN_CHUNK( Mem::MC_GFX, iPBHeaderSize, 0x80 );
	NT_MEMCPY( *pushBuffer, pFakePB, iPBHeaderSize );
	// now the actual pushbuffer
	uint32_t iPBSize = (pFakePB->m_pCurrent - pFakePB->m_pStart);
	(*pushBuffer)->m_pStart = (uint32_t*) NT_MEMALIGN_CHUNK( Mem::MC_GFX, iPBSize*4, 0x80 );
	NT_MEMCPY((*pushBuffer)->m_pStart, pFakePB->m_pStart, iPBSize*4 );
//	ntPrintf( Debug::DCU_RESOURCES, "Total Push Buffer Size : %i\n", (iPBHeaderSize + iPBSize*4) );

	// just to ensure nobody trys to fiddle with it and so we can calculate the size.
	(*pushBuffer)->m_pCurrent = (*pushBuffer)->m_pEnd = (*pushBuffer)->m_pStart + iPBSize;
	(*pushBuffer)->m_iMaxScratch = (*pushBuffer)->m_iCurScratch;
//	m_pRenderPushBufferHeader->m_iMaxPatches = m_pRenderPushBufferHeader->m_iNumPatches;

FreeDebugMem:
	NT_FREE_CHUNK( Mem::MC_MISC, (uintptr_t)pFakePBBuffer );
	NT_FREE_CHUNK( Mem::MC_MISC, (uintptr_t)pFakePB );
}

void CMeshInstance::BuildShadowMapPB(Heresy_PushBuffer** pushBuffer)
{
	// one day this will be done offline... so the structure is a bit weird, I do it all to a stack fake push buffer...
	static const uint32_t iMaxSize = 240*1024/4;
	static const uint32_t iMaxPatchSize = 100;
	static const uint32_t iMaxScratchSize = 32*1024;
	uint32_t* pFakePBBuffer = (uint32_t*) NT_ALLOC_CHUNK( Mem::MC_MISC, iMaxSize );
	Heresy_PushBuffer *pFakePB = (Heresy_PushBuffer *)NT_MEMALIGN_CHUNK( Mem::MC_MISC, sizeof(Heresy_PushBuffer) + (sizeof(Heresy_PushBufferPatch) * iMaxPatchSize) + iMaxScratchSize, 0x80 );
	Heresy_InitPushBuffer( pFakePB, pFakePBBuffer, pFakePBBuffer + iMaxSize, iMaxPatchSize, iMaxScratchSize );
	bool pbCreationOk;

//	Heresy_InsertDebugMarker( pFakePB, "Deano001" );
	// on PS3 I think all materials are CGameMaterialInstance, if so... I vote we flatten the hierachy...
	ntAssert( m_pMaterial->isFX() == false );
	CGameMaterialInstance* pMaterial = (CGameMaterialInstance*)m_pMaterial.Get();

	pbCreationOk = pMaterial->BuildPreRenderDepthPB( pFakePB, m_hVertexBuffer, true );
	if( pbCreationOk == false )
	{
		goto FreeDebugMem;
	}

	// actually do the render
	Heresy_SetIndexArrayOffsetAndType( pFakePB, m_hIndexBuffer->GetDataOffset(), Heresy_IndexArrayType( HIT_16_BIT, HRL_LOCAL_GDDR ) );

	// here the special shader map rendering bit, we put four a skipping nops to 'jump' the render and markers
	// then per frame we reduce the nop to 0 if this bit and put in the new projection matrix

	uint128_t dummy[4];
	const CShaderGraph* graph = pMaterial->GetMaterial()->GetDepthWriteGraph( pMaterial->GetBoundType() );
	u32 index;
	
	if ( m_pobMeshHeader->IsPositionCompressed() && ( GetMaterialInstance()->GetBoundType() == VSTT_SKINNED) ) 
		index = graph->GetVertexShader()->GetConstantIndex( "projection_noreconst_mat" );
	else	
		index = graph->GetVertexShader()->GetConstantIndex( "projection" );

	const GcShaderResource::Constant* pConstant = graph->GetVertexShader()->m_hShaderHandle->GetResource()->GetConstants() + index;
	for( int i=0;i<4;i++)
	{
		// come back and fill in the skipping amount in a minute
		Heresy_SkippingMarker( pFakePB, 0 );
		uint32_t* pCurPB = pFakePB->m_pCurrent;

		Heresy_SetVertexShaderConstantsSmall( pFakePB, pConstant->m_resourceStart, 4, dummy );
		Heresy_SetScissorRegion( pFakePB, Heresy_ScissorHorizontal( 0, 0 ), Heresy_ScissorVertical( 0, 0 ) );
		Heresy_DrawIndexPrims( pFakePB, HPT_TRIANGLE_STRIP, 0, m_iIndexCount );
		Heresy_Set32bit( (pFakePB->m_pStart + pFakePB->m_iMarker[i]), Heresy_Cmd( RSX_NOOP, (pFakePB->m_pCurrent - pCurPB)*4) | HPBC_NOINCREMENT );
	}
	
	pbCreationOk = pMaterial->BuildPostRenderDepthPB( pFakePB, m_hVertexBuffer, true );
	if( pbCreationOk == false )
	{
		goto FreeDebugMem;
	}
	// okay we now have a valid depth push buffer, so we allocate some RAM and copy our fake push buffer into it
	// where it will be ready to go

	// NOTE currently always have max patches space... due to me being stoopid
	// calc the size of the actual fix up list and the header
	uint32_t iPBHeaderSize =	sizeof(Heresy_PushBuffer) +		
								pFakePB->m_iMaxPatches * sizeof( Heresy_PushBufferPatch ) + 
								pFakePB->m_iCurScratch * sizeof(uint32_t);
	*pushBuffer = (Heresy_PushBuffer*) NT_MEMALIGN_CHUNK( Mem::MC_GFX, iPBHeaderSize, 0x80 );
	NT_MEMCPY( *pushBuffer, pFakePB, iPBHeaderSize );
	// now the actual pushbuffer
	uint32_t iPBSize = (pFakePB->m_pCurrent - pFakePB->m_pStart);
	(*pushBuffer)->m_pStart = (uint32_t*) NT_MEMALIGN_CHUNK( Mem::MC_GFX, iPBSize*4, 0x80 );
	NT_MEMCPY((*pushBuffer)->m_pStart, pFakePB->m_pStart, iPBSize*4 );
//	ntPrintf( Debug::DCU_RESOURCES, "Total Push Buffer Size : %i\n", (iPBHeaderSize + iPBSize*4) );


	// just to ensure nobody trys to fiddle with it and so we can calculate the size.
	(*pushBuffer)->m_pCurrent = (*pushBuffer)->m_pEnd = (*pushBuffer)->m_pStart + iPBSize;
	(*pushBuffer)->m_iMaxScratch = (*pushBuffer)->m_iCurScratch;
//	m_pShadowMapPushBufferHeader->m_iMaxPatches = m_pShadowMapPushBufferHeader->m_iNumPatches;

FreeDebugMem:
	NT_FREE_CHUNK( Mem::MC_MISC, (uintptr_t)pFakePBBuffer );
	NT_FREE_CHUNK( Mem::MC_MISC, (uintptr_t)pFakePB );

}

void CMeshInstance::DeletePushBuffers()
{
	//ntAssert_p( !UseHeresy() || m_bPushBuffersValid, ("Cannot delete Push buffers as theyre not valid") );

	//if( m_pShadowMapPushBufferHeader )
	//{
	//	NT_FREE_CHUNK( Mem::MC_GFX, (uintptr_t) m_pShadowMapPushBufferHeader->m_pStart );
	//	NT_FREE_CHUNK( Mem::MC_GFX, (uintptr_t) m_pShadowMapPushBufferHeader );
	//	m_pShadowMapPushBufferHeader = 0;
	//}
	//if( m_pDepthPushBufferHeader )
	//{
	//	NT_FREE_CHUNK( Mem::MC_GFX, (uintptr_t) m_pDepthPushBufferHeader->m_pStart );
	//	NT_FREE_CHUNK( Mem::MC_GFX, (uintptr_t) m_pDepthPushBufferHeader );
	//	m_pDepthPushBufferHeader = 0;
	//}
	//if( m_pRenderPushBufferHeader )
	//{
	//	NT_FREE_CHUNK( Mem::MC_GFX, (uintptr_t) m_pRenderPushBufferHeader->m_pStart);
	//	NT_FREE_CHUNK( Mem::MC_GFX, (uintptr_t) m_pRenderPushBufferHeader);
	//	m_pRenderPushBufferHeader = 0;
	//}

	m_hIndexBuffer = IBHandle();
	m_hVertexBuffer[0] = VBHandle();
	m_hVertexBuffer[1] = VBHandle();

	m_heresyPushBuffers.Reset();

	//m_bPushBuffersValid = false;
}


void CMeshInstance::ReleaseAreaResources( void )
{
	DeletePushBuffers();
}

void CMeshInstance::DestroyPushBuffer(Heresy_PushBuffer* pushBuffer)
{
	ntAssert(pushBuffer);

	NT_FREE_CHUNK( Mem::MC_GFX, (uintptr_t) pushBuffer->m_pStart );
	NT_FREE_CHUNK( Mem::MC_GFX, (uintptr_t) pushBuffer );
}
