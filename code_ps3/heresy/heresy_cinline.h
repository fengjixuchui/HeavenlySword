#if !defined( HERESY_CINLINE_H )
#define HERESY_CINLINE_H

union Heresy_HalfSwap128
{
#if defined( __SPU__ )
	v128		v128Data;
#endif
	uint128_t	qwordData;
	float		floatData[4];
	uint16_t	shortData[8];
};
inline void Heresy_Set128bitPixelShaderConstant( uint16_t* pDestAddr, const Heresy_HalfSwap128* pSrcData )
{
	pDestAddr[0] = pSrcData->shortData[1];
	pDestAddr[1] = pSrcData->shortData[0];
	pDestAddr[2] = pSrcData->shortData[3];
	pDestAddr[3] = pSrcData->shortData[2];
	pDestAddr[4] = pSrcData->shortData[5];
	pDestAddr[5] = pSrcData->shortData[4];
	pDestAddr[6] = pSrcData->shortData[7];
	pDestAddr[7] = pSrcData->shortData[6];
}

//---
// for now we use ICE to do the memory space conversion, but not for long;
inline uint32_t Heresy_MainRamAddressToRSX( const Heresy_GlobalData* pGlobal, void* pPtr )
{
	return( ((uint32_t)pPtr) - pGlobal->m_RSXMainBaseAdjust );
}

inline uint32_t Heresy_VramRamAddressToRSX( const Heresy_GlobalData* pGlobal, void* pPtr )
{
	return( ((uint32_t)pPtr) - pGlobal->m_IceVramOffset );
}

inline void* Heresy_AllocatePixelShaderSpaceInXDDR( Heresy_GlobalData* pGlobal, uint32_t space )
{
	return (void*) DoubleEnderFrameAllocator_MemAlign( &pGlobal->m_PushBufferAllocator, space, 64 );
}

// gets the vertex shader constants that are embedded in the shader itself
inline const float* Heresy_GetVertexShaderConstantTable( restrict const Heresy_VertexShader* pProgram )
{
	return (const float*)( (sizeof(Heresy_VertexShader) + pProgram->m_constantCount * 4 + 0xF) & ~0xF);
}

// if we use ice we need to fill in dummy to the expand command buffer callback (which we always fail for heresy api)
inline void Heresy_InitPushBuffer( restrict Heresy_PushBuffer* pb, uint32_t* s, uint32_t* e, uint16_t iMaxPatches, uint16_t iMaxScratch )
{											
	pb->m_pStart = s;						
	pb->m_pEnd = e;							
	pb->m_pCurrent = s;
	pb->m_iNumPatches = 0;
	pb->m_iMaxPatches = iMaxPatches;
	pb->m_iCurScratch = 0;
	pb->m_iMaxScratch = iMaxScratch;

	for( int i=0;i < HERETIC_NUM_PB_MARKERS;i++)
	{
		pb->m_iMarker[i] = 0xFFFF;
	}
#if defined(USE_ICE_AS_BACKEND_TO_HERESY)
	pb->m_pDummy = (void*)&Heresy_DummyCommandFillCallback;
#endif
}

inline void Heresy_AlignPushBuffer( restrict Heresy_PushBuffer* pb, const uint32_t alignment )
{
	uint32_t* pAligned = (uint32_t*)( (((uintptr_t)pb->m_pCurrent) + (alignment-1)) & ~(alignment-1));
	uint16_t skips = (pAligned - pb->m_pCurrent);

	// check to see if we need aligning or if we already are...
	if( skips > 0 )
	{
		// as we need alignment insert a skipping noop to align us all nicely
		Heresy_CheckPBSpace( pb, skips );
		Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( RSX_NOOP, (skips-1)*4) | HPBC_NOINCREMENT );
		Heresy_IncPB( pb, skips);
	}

	Heresy_Assert( pAligned == pb->m_pCurrent );
}

// NOTE VERY VERY IMPORTANT This only increments the push buffer by 4 bytes (the size of the nop command NOT the skip)
// it also places a marker to the START of the nop, the idea is you can then conditionally turn on the skip space 
// (the skip which can be whatever the hell you want) by simple setting a zero skip NOP at the marker position
inline void Heresy_SkippingMarker( restrict Heresy_PushBuffer* pb, const uint16_t iBytesToSkip )
{
	Heresy_CheckPBSpace( pb, iBytesToSkip/4 + 1 );

	// find free marker
	uint16_t* pMarker = pb->m_iMarker;
	for( uint16_t i=0;i < HERETIC_NUM_PB_MARKERS;i++, pMarker++)
	{
		if( *pMarker == 0xFFFF )
			break;
	}
	// see if we have run out
	Heresy_Assert( *pMarker == 0xFFFF );
	Heresy_Assert( (pb->m_pCurrent - pb->m_pStart) < 0xFFFF );

	*pMarker = (uint16_t) (pb->m_pCurrent - pb->m_pStart);
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( RSX_NOOP, iBytesToSkip+4) | HPBC_NOINCREMENT );
	Heresy_IncPB( pb, 1);

}

inline void Heresy_InsertDebugMarker( restrict Heresy_PushBuffer* pb, char text[8] )
{
	Heresy_CheckPBSpace( pb, 4 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( RSX_NOOP, 12) | HPBC_NOINCREMENT );
	Heresy_Set32bit( pb->m_pCurrent+1, 0xDEEDEE00 );
	Heresy_CopyN32bits( pb->m_pCurrent+2, (uint32_t*)text, 2 );
	Heresy_IncPB( pb, 4);
}

inline void Heresy_InsertJump( const Heresy_GlobalData* pGlobalData, restrict Heresy_PushBuffer* pb, void* pEAAddr )
{
	Heresy_CheckPBSpace( pb, 1 );
	uint32_t iCmd = Heresy_MainRamAddressToRSX( pGlobalData, pEAAddr ) | HPBC_JUMP;
	Heresy_Set32bit( pb->m_pCurrent, iCmd );
	Heresy_IncPB( pb, 1);
}

inline void Heresy_InsertCall( const Heresy_GlobalData* pGlobalData, restrict Heresy_PushBuffer* pb, void* pEAAddr )
{
	Heresy_CheckPBSpace( pb, 1 );
	uint32_t iCmd = Heresy_MainRamAddressToRSX( pGlobalData, pEAAddr ) | HPBC_CALL;
	Heresy_Set32bit( pb->m_pCurrent, iCmd );
	Heresy_IncPB( pb, 1);
}

inline void Heresy_InsertReturn( restrict Heresy_PushBuffer* pb )
{
	Heresy_CheckPBSpace( pb, 1 );
	Heresy_Set32bit( pb->m_pCurrent, HPBC_RET );
	Heresy_IncPB( pb, 1);
}

// use to insert the relevant space and fix up so that a push buffer reference can be added later must be less than 8
// use Heresy_FixupVertexShaderConstant to handle small and normal safely
inline void Heresy_SetVertexShaderConstantSmallFixupSpace( restrict Heresy_PushBuffer* pb, const uint16_t constantnum, const uint16_t semantic, uint16_t sizeIn128bits )	
{
	Heresy_Assert( sizeIn128bits <= 8 ); // ICE reckon 8 qwords per set max...
	Heresy_CheckPBSpace( pb, 2 + (sizeIn128bits*4) );

	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( SELECT_VERTEX_CONSTANT_WINDOW, 4 + (sizeIn128bits*16) ) );
	Heresy_Set32bit( pb->m_pCurrent+1, constantnum );

	restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (pb+1);
	Heresy_Assert( pb->m_iNumPatches+1 < pb->m_iMaxPatches );
	Heresy_Assert( ((pb->m_pCurrent+2) - pb->m_pStart) < 0xFFFF );

	pPatch[ pb->m_iNumPatches ].m_iData = HPBP_VERTEX_CONSTANT_FIXUP;
	pPatch[ pb->m_iNumPatches ].m_offset = (uint16_t) ((pb->m_pCurrent+2) - pb->m_pStart);
	pPatch[ pb->m_iNumPatches ].m_Semantic = semantic;

	pb->m_iNumPatches++;
	Heresy_IncPB( pb, 2 + (sizeIn128bits*4) );
}

// use to insert the relevant space and fix up so that a push buffer reference can be added later
// use Heresy_FixupVertexShaderConstant to handle safely
inline void Heresy_SetVertexShaderConstantFixupSpace( restrict Heresy_PushBuffer* pb, uint16_t constantnum, const uint16_t semantic, uint16_t sizeIn128bits )	
{
	restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (pb+1);
	Heresy_Assert( pb->m_iNumPatches+1 < pb->m_iMaxPatches );
	Heresy_Assert( ((pb->m_pCurrent+2) - pb->m_pStart) < 0xFFFF );

	pPatch[ pb->m_iNumPatches ].m_iData = HPBP_VERTEX_CONSTANT_FIXUP;
	pPatch[ pb->m_iNumPatches ].m_offset = (uint16_t) ((pb->m_pCurrent+2) - pb->m_pStart);
	pPatch[ pb->m_iNumPatches ].m_Semantic = semantic;
	pb->m_iNumPatches++;

	const uint16_t blocks = (sizeIn128bits/8); 
	const uint16_t remain = sizeIn128bits - blocks*8;
	for( uint16_t i=0;i < blocks;++i )
	{
		Heresy_CheckPBSpace( pb, 2 + (8*4) );
		Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( SELECT_VERTEX_CONSTANT_WINDOW, 4 + (8*16) ) );
		Heresy_Set32bit( pb->m_pCurrent+1, constantnum );
		Heresy_IncPB( pb, 2 + (8*4) );
		constantnum += 8;
	}
	Heresy_CheckPBSpace( pb, 2 + (remain*4) );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( SELECT_VERTEX_CONSTANT_WINDOW, 4 + (remain*16) ) );
	Heresy_Set32bit( pb->m_pCurrent+1, constantnum );
	Heresy_IncPB( pb, 2 + (remain*4) );

}

// inline pixel shader need to point into the pushbuffer itself, this does this
inline void Heresy_FixupCopyN128bitsVertexShaderConstant( restrict Heresy_PushBuffer* pb, const restrict Heresy_PushBufferPatch* pPatch,  const void* restrict pSource, uint16_t sizeIn128bits )
{
	Heresy_Assert( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) == HPBP_VERTEX_CONSTANT_FIXUP );

	uint32_t* pDst = pb->m_pStart + pPatch->m_offset;
	const uint32_t* pSrc = (const uint32_t*) pSource;
	const uint16_t blocks = (sizeIn128bits/8); 
	const uint16_t remain = sizeIn128bits - blocks*8;
	for( uint16_t i=0;i < blocks;++i )
	{
		Heresy_CopyN128bits( pDst, pSrc, 8 );
		pDst += 2 + 8*4;
		pSrc += 8*4;
	}
	Heresy_CopyN128bits( pDst, pSrc, remain );

}



inline void Heresy_SetVertexShaderConstant4F( restrict Heresy_PushBuffer* pb, const uint16_t constantnum, const float* restrict constant )	
{
	Heresy_CheckPBSpace( pb, 6 );
	Heresy_Assert( constantnum < 492 );

	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( SELECT_VERTEX_CONSTANT_WINDOW, 20) );
	Heresy_Set32bit( pb->m_pCurrent+1, constantnum );
	Heresy_Set128bit( pb->m_pCurrent+2, *((const uint128_t*)constant) );
	Heresy_IncPB( pb, 6);
}

inline void Heresy_SetVertexShaderConstant16B( restrict Heresy_PushBuffer* pb, const uint16_t constantnum, const uint128_t* restrict constant )
{
	Heresy_CheckPBSpace( pb, 6 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( SELECT_VERTEX_CONSTANT_WINDOW, 20) );
	Heresy_Set32bit( pb->m_pCurrent+1, constantnum );
	Heresy_Set128bit( pb->m_pCurrent+2, *constant );
	Heresy_IncPB( pb, 6);
}
inline void Heresy_SetVertexShaderConstantsSmall( restrict Heresy_PushBuffer* pb, const uint16_t constantnum, const uint16_t sizeIn128bits, const uint128_t* restrict constant )	
{
	Heresy_Assert( sizeIn128bits <= 8 ); // ICE reckon 8 qwords per set max...
	Heresy_CheckPBSpace( pb, 2 + (sizeIn128bits*4) );

	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( SELECT_VERTEX_CONSTANT_WINDOW, 4 + (sizeIn128bits*16) ) );
	Heresy_Set32bit( pb->m_pCurrent+1, constantnum );

	Heresy_CopyN128bits( pb->m_pCurrent+2, constant, sizeIn128bits  );

	Heresy_IncPB( pb, 2 + (sizeIn128bits*4) );
}

//! use to insert the relevant space and fix up so that a push buffer reference can be added later
//! the returned pointer is to be passed to SetPixelShaderConstantScratch or Heresy_SetPixelShaderConstantScratchFixupSpace
inline uint32_t* Heresy_SetPixelShaderScratch( restrict Heresy_PushBuffer* pb, Heresy_PixelShader* pShader )	
{
	Heresy_Assert( pb->m_iCurScratch + (pShader->m_microcodeSize / 4) < pb->m_iMaxScratch );

	// get the start of the patch table
	restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (pb+1);
	// get the beginning of scratch ram for this push buffer
	restrict uint32_t* pUnAlignedScratch = ((uint32_t*) (pPatch+pb->m_iMaxPatches)) + pb->m_iCurScratch;
	// align the scratch ram for SPU DMA'ablility
	uint32_t* pScratch = (uint32_t*)( (((uintptr_t)pUnAlignedScratch) + (16-1)) & ~(16-1));
	pb->m_iCurScratch += (pScratch - pUnAlignedScratch); // account for alignment

	// copy shader into the space that we made
	uint32_t* pSrcShader = (uint32_t*)((char*)pShader + pShader->m_microcodeOffset);
	const uint16_t pShaderSize = pShader->m_microcodeSize / 4;

	uint32_t* pDestShader = pScratch;
	Heresy_CopyN32bits( pDestShader, pSrcShader, pShaderSize );
	pb->m_iCurScratch += pShaderSize;

	Heresy_CheckPBSpace( pb, 15 );
	// create a special fixup that will succesfully point the GPU back to the memory above
	Heresy_Assert( pb->m_iNumPatches+1 < pb->m_iMaxPatches );
	Heresy_Assert( (pb->m_pCurrent+1 - pb->m_pStart) < 0xFFFF );
	// we pack the size in words in the bottom 14 bits of semantic
	Heresy_Assert( (pShader->m_microcodeSize / 4) < 0x3FFF );

	restrict Heresy_PushBufferPatch* pCurPatch = &pPatch[ pb->m_iNumPatches ];
	// where the original pixel shader lives
	pCurPatch->m_iData = ((uint32_t)(pDestShader-(uint32_t*)pCurPatch)<<HPBP_DATA_MASK_LENGTH) | HPBP_PIXEL_SHADER_FIXUP; 
	// the offset in the push buffer to put actual address (we is updated each frame) of the pixel shader
	pCurPatch->m_offset = (pb->m_pCurrent+1 - pb->m_pStart);
	// we pack the size in words in the bottom 14 bits of semantic
	pCurPatch->m_Semantic = Heresy_PushBufferInlineFixupToken | pShaderSize;
	pb->m_iNumPatches++;

	Heresy_Set32bit( pb->m_pCurrent,   Heresy_Cmd( FRAGMENT_SHADER_ADDRESS, 4 ) );
#if _DEBUG
	Heresy_Set32bit( pb->m_pCurrent+1, 0xFEFEFEFE ); // will be filled in in the fixup pass
#endif
	Heresy_Set32bit( pb->m_pCurrent+2, Heresy_Cmd( FRAGMENT_SHADER_CONTROL, 4 ) );
	
	Heresy_Set32bit( pb->m_pCurrent+3, pShader->m_control );

	Heresy_Set32bit( pb->m_pCurrent+4, Heresy_Cmd( FRAGMENT_CONTROL_0, 0x28 ) );
	uint32_t mask0 = pShader->m_texcoordMask;
	uint32_t mask1 = (uint32_t)(pShader->m_centroidMask) << 4;
	for( uint32_t i=0;i < 10;++i, mask0>>=1, mask1>>=1 )
	{
		Heresy_Set32bit( pb->m_pCurrent+5+i, (mask0 & 0x1) | (mask1 & 0x10) );
	}
	Heresy_IncPB( pb, 15 );

	return pDestShader;
}

// use to insert the relevant space and fix up so that a push buffer reference can be added later
inline void Heresy_SetPixelShaderScratchConstant4F( restrict Heresy_PushBuffer* pb, Heresy_PixelShader* pShader, uint32_t* pCode, const uint16_t index, const float* restrict pData )	
{
	Heresy_Assert(  index < pShader->m_patchCount );
	Heresy_IcePatchData* pIcePatch = (Heresy_IcePatchData*)((char*)pShader + (((uint32_t*)(pShader+1))[index]));
	uint16_t count = pIcePatch->m_count;
	uint16_t* pOffset = (uint16_t*)(pIcePatch+1);

	for( uint16_t i = 0; i < count;i++)
	{
		// *pOffset is in an offset in 16 byte chunks
		uint16_t* pAddrInPS = (uint16_t*)(pCode + ((*pOffset)*(16/4)));
		Heresy_Set128bitPixelShaderConstant( pAddrInPS, (const Heresy_HalfSwap128*)pData );
		pOffset++;
	}
}
// use to insert the relevant space and fix up so that a push buffer reference can be added later
inline void Heresy_SetPixelShaderScratchConstantInlineFixupSpace( restrict Heresy_PushBuffer* pb, Heresy_PixelShader* pShader, uint32_t* pCode, uint16_t* pIndex, const uint16_t semantic, uint16_t sizeIn128bits )	
{
	restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (pb+1);

	// hate this generates a lot of push buffer fixup traffic... Must do better by thinking about the problem more...
	for( uint16_t j=0;j < sizeIn128bits;++j )
	{
		
		uint16_t index = pIndex[j];
		if( index == 0xFFFF )
			continue;

		uint32_t pPatchOffset = ((uint32_t*)(pShader+1))[index];
		Heresy_IcePatchData* pIcePatch = (Heresy_IcePatchData*)(((char*)pShader) + pPatchOffset);
		uint16_t* pOffset = (uint16_t*)(pIcePatch+1);
		uint16_t count = pIcePatch->m_count;
		restrict Heresy_PushBufferPatch* pCurPatch = pPatch + pb->m_iNumPatches;

		Heresy_Assert( pb->m_iNumPatches+count < pb->m_iMaxPatches );
		for( uint16_t i = 0; i < count;i++)
		{
			// *pOffset is in an offset in 16 byte chunks
			uint32_t* pAddrInPS = pCode + ((*pOffset)*(16/4));
#if _DEBUG
			memset( pAddrInPS, 0xFD, 4*4 );
#endif

			// our offset is relative to the patch we are putting this in Note we are 32 bit aligned so the bottom bits are free
			// this works cos currently our scratch ram allows follows our patch ram in a constant amount
			uint32_t iPatchRelOffset = pAddrInPS - ((uint32_t*)pCurPatch);

			pCurPatch->m_offset = j;
			pCurPatch->m_Semantic = semantic;
			pCurPatch->m_iData = (iPatchRelOffset<<HPBP_DATA_MASK_LENGTH) | HPBP_PIXEL_CONSTANT_FIXUP;
			pCurPatch++;
			pb->m_iNumPatches++;
			pOffset++;
		}
	}
}

inline void Heresy_FixupPixelShaderConstantScratch( restrict Heresy_PushBuffer* pb,  const restrict Heresy_PushBufferPatch* pPatch, const uint128_t* restrict pData, uint16_t sizeIn128bits )
{
	Heresy_Assert( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) == HPBP_PIXEL_CONSTANT_FIXUP );
	// patch relative offset, shifted up by 2
	uint32_t iOffset = (pPatch->m_iData & HPBP_DATA_MASK )>>HPBP_DATA_MASK_LENGTH;
	uint32_t* pDestAddr = ((uint32_t*)pPatch) + iOffset;
	const Heresy_HalfSwap128* pSrcAddr = ((const Heresy_HalfSwap128* restrict)pData) + pPatch->m_offset;
	Heresy_Set128bitPixelShaderConstant( (uint16_t*)pDestAddr, pSrcAddr );
}

inline void Heresy_FixupPixelShaderConstantScratch16B( restrict Heresy_PushBuffer* pb,  const restrict Heresy_PushBufferPatch* pPatch, const uint128_t* restrict pData )
{
	Heresy_Assert( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) == HPBP_PIXEL_CONSTANT_FIXUP );
//	Heresy_Assert( pPatch->m_offset == 0 );
	// patch relative offset, shifted up by 2
	uint32_t iOffset = (pPatch->m_iData & HPBP_DATA_MASK )>>HPBP_DATA_MASK_LENGTH;

	uint32_t* pDestAddr = ((uint32_t*)pPatch) + iOffset;

	Heresy_Set128bitPixelShaderConstant( (uint16_t*)pDestAddr, (const Heresy_HalfSwap128* restrict)pData );
}


// inline pixel shader need to point into the pushbuffer itself, this does this
inline void Heresy_FixupPixelShaderInline( Heresy_GlobalData* pGlobalData, restrict uint32_t* pStart, restrict Heresy_PushBufferPatch* pPatch )
{
	Heresy_Assert( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) == HPBP_PIXEL_SHADER_FIXUP );

	uint16_t iShaderSize = pPatch->m_Semantic & HPBP_PIXEL_SHADER_SIZE_MASK;
	uint32_t* pDestShader = (uint32_t*)Heresy_AllocatePixelShaderSpaceInXDDR( pGlobalData, iShaderSize*4 );
	uint32_t* pSrcShader = ((uint32_t*)pPatch) + ((pPatch->m_iData & HPBP_DATA_MASK)>>HPBP_DATA_MASK_LENGTH);
	Heresy_CopyN32bits( pDestShader, pSrcShader, iShaderSize );

	uint32_t* pAddrReg = pStart + pPatch->m_offset;
#if _DEBUG
	Heresy_Assert( *pAddrReg == 0xFEFEFEFE );
#endif
	*pAddrReg = Heresy_MainRamAddressToRSX( pGlobalData, pDestShader ) | HFRL_MAIN_XDDR;
}

// returns a push buffer pointer that lets you fiddle with things like wrap modes properly
inline uint32_t* Heresy_SetTexture( restrict Heresy_PushBuffer* pb, const uint8_t unit, restrict const Heresy_Texture* texture )
{
	Heresy_CheckPBSpace( pb, 15 );

	uint32_t* pAddr = pb->m_pCurrent;

	Heresy_Set32bit( pAddr, Heresy_Cmd( TEXTURE0_OFFSET + unit*32, 32) );
	Heresy_CopyN32bits( pAddr+1, &texture->m_baseOffset, 8 );
	Heresy_Set32bit( pAddr+9, Heresy_Cmd( TEXTURE0_SIZE_DP + unit*4, 4) );
	Heresy_Set32bit( pAddr+10, texture->m_size2 );
	Heresy_Set32bit( pAddr+11, Heresy_Cmd( TEXTURE0_BRILINEAR + unit*4, 4) );
	Heresy_Set32bit( pAddr+12, texture->m_control3 );
	Heresy_Set32bit( pAddr+13, Heresy_Cmd( TEXTURE0_COLOURKEYCONTROL + unit*4, 4) );
	Heresy_Set32bit( pAddr+14, texture->m_colorKeyColor );
	Heresy_IncPB( pb, 15 );

	return pAddr;
}

// returns a push buffer pointer that lets you fiddle with things like wrap modes properly
inline uint32_t* Heresy_SetTextureFixup( restrict Heresy_PushBuffer* pb, const uint8_t unit, const uint16_t semantic )
{
	Heresy_CheckPBSpace( pb, 15 );

	uint32_t* pAddr = pb->m_pCurrent;

	Heresy_Set32bit( pAddr, Heresy_Cmd( TEXTURE0_OFFSET + unit*32, 32) );
	Heresy_Set32bit( pAddr+9, Heresy_Cmd( TEXTURE0_SIZE_DP + unit*4, 4) );
	Heresy_Set32bit( pAddr+11, Heresy_Cmd( TEXTURE0_BRILINEAR + unit*4, 4) );
	Heresy_Set32bit( pAddr+13, Heresy_Cmd( TEXTURE0_COLOURKEYCONTROL + unit*4, 4) );

	restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (pb+1);
	Heresy_Assert( pb->m_iNumPatches+1 < pb->m_iMaxPatches );
	Heresy_Assert( (pAddr - pb->m_pStart) < 0xFFFF );

	pPatch[ pb->m_iNumPatches ].m_offset = (uint16_t) (pAddr - pb->m_pStart);
	pPatch[ pb->m_iNumPatches ].m_Semantic = semantic;
	pPatch[ pb->m_iNumPatches ].m_iData = HPBP_TEXTURE_FIXUP;
	pb->m_iNumPatches++;

	Heresy_IncPB( pb, 15 );

	return pAddr;
}

//! these let you change the texture address mode of a texture thats previously been set into a pushbuffer, the preserved the other bits
inline void Heresy_OverrideTextureAddressOnly( uint32_t* pTexPBStart, uint32_t data )
{
	uint32_t reg = pTexPBStart[3];
	reg = (reg & 0xFFF00000) | (data & 0x000FFFFF);
	pTexPBStart[3] = reg;
}

//! these let you change the texture filter mode of a texture thats previously been set into a pushbuffer
inline void Heresy_OverrideTextureFilterOnly( uint32_t* pTexPBStart, uint32_t data )
{
	uint32_t reg = pTexPBStart[6];
	reg = (reg & 0x0000FFFF) | (data & 0xFFFF0000);
	pTexPBStart[6] = reg;
}

inline void Heresy_FixupTexture( restrict Heresy_PushBuffer* pb, restrict const Heresy_Texture* pTexture, const restrict Heresy_PushBufferPatch* pPatch )
{
	Heresy_Assert( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) == HPBP_TEXTURE_FIXUP );
	uint32_t* pAddr = pb->m_pStart + pPatch->m_offset;

	Heresy_CopyN32bits( pAddr+1, &pTexture->m_baseOffset, 8 );
	Heresy_Set32bit( pAddr+10, pTexture->m_size2 );
	Heresy_Set32bit( pAddr+12, pTexture->m_control3 );
	Heresy_Set32bit( pAddr+14, pTexture->m_colorKeyColor );
}

//! Textures are weird as we want to override some thing currently (wrap + filter modes) this handles this pain..
inline void Heresy_FixupTextureLeaveOverrides( restrict Heresy_PushBuffer* pb, restrict const Heresy_Texture* pTexture, const restrict Heresy_PushBufferPatch* pPatch )
{
	Heresy_Assert( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) == HPBP_TEXTURE_FIXUP );
	uint32_t* pAddr = pb->m_pStart + pPatch->m_offset;

	uint32_t reg3 = pAddr[3];
	uint32_t reg6 = pAddr[6];

	Heresy_CopyN32bits( pAddr+1, &pTexture->m_baseOffset, 8 );
	Heresy_Set32bit( pAddr+10, pTexture->m_size2 );
	Heresy_Set32bit( pAddr+12, pTexture->m_control3 );
	Heresy_Set32bit( pAddr+14, pTexture->m_colorKeyColor );

	Heresy_OverrideTextureAddressOnly( pAddr, reg3 );
	Heresy_OverrideTextureFilterOnly( pAddr, reg6 );
}

//! note a SetTexture will reset this
//! it has the z depth compare and remap bias bit which strictly isn't an address op, so you should merge this with the texture if you want use those features...
inline void Heresy_SetTextureAddress( restrict Heresy_PushBuffer* pb, const uint8_t unit, uint32_t data )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( TEXTURE0_ADDRESS + unit*32, 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, data );
	Heresy_IncPB( pb, 2);
}

//! note a SetTexture will reset this
inline void Heresy_SetTextureFilter( restrict Heresy_PushBuffer* pb, const uint8_t unit, uint32_t data )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( TEXTURE0_FILTER + unit*32, 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, data );
	Heresy_IncPB( pb, 2);
}

inline void Heresy_DisableTexture( restrict Heresy_PushBuffer* pb, const uint8_t unit )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( TEXTURE0_CONTROL + unit*32, 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, 0 );
	Heresy_IncPB( pb, 2);

}

//! set a the format of a vertex data element
inline void Heresy_SetVertexStreamFormat( restrict Heresy_PushBuffer* pb, const uint8_t unit, uint32_t data )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( VERTEXSTREAM0_FORMAT + unit*4, 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, data );
	Heresy_IncPB( pb, 2);
}

//! set a vertex stream to off
inline void Heresy_DisableVertexStream( restrict Heresy_PushBuffer* pb, const uint8_t unit )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( VERTEXSTREAM0_FORMAT + unit*4, 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, 0x0002 );
	Heresy_IncPB( pb, 2);
}

//! set the GPU offset (as this is the vertex unit, top bit is ram pool)
inline void Heresy_SetVertexStreamOffset( restrict Heresy_PushBuffer* pb, const uint8_t unit, uint32_t data )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( VERTEXSTREAM0_OFFSET + unit*4, 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, data );
	Heresy_IncPB( pb, 2);
}

//! set just the index arrays offset (cos we rarely change type...)
inline void Heresy_SetIndexArrayOffset( restrict Heresy_PushBuffer* pb, uint32_t offset )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( INDEX_ARRAY_OFFSET , 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, offset );
	Heresy_IncPB( pb, 2);
}

// set just the index arrays type on its own(cos we rarely change type...)
inline void Heresy_SetIndexArrayType( restrict Heresy_PushBuffer* pb, uint32_t format )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( INDEX_ARRAY_TYPE , 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, format );
	Heresy_IncPB( pb, 2);
}


inline void Heresy_SetJumpFixUp( restrict Heresy_PushBuffer* pb, uint32_t* jumpStart, uint32_t* jumpDestination, uint32_t jumpType )
{
	restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (pb+1);
	Heresy_Assert( pb->m_iNumPatches+1 < pb->m_iMaxPatches );
	Heresy_Assert( ((pb->m_pCurrent+2) - pb->m_pStart) < 0xFFFF );

	pPatch[ pb->m_iNumPatches ].m_iData = ((jumpDestination -  jumpStart) << HPBP_DATA_MASK_LENGTH) | jumpType;
	pPatch[ pb->m_iNumPatches ].m_offset = jumpStart - pb->m_pStart;
	pPatch[ pb->m_iNumPatches ].m_Semantic = 0;
	pb->m_iNumPatches++;
}

//! sets the format and offset of the index buffer
inline void Heresy_SetIndexArrayOffsetAndType( restrict Heresy_PushBuffer* pb, uint32_t offset, uint32_t format )
{
	Heresy_CheckPBSpace( pb, 3 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( INDEX_ARRAY_OFFSET , 8) );
	Heresy_Set32bit( pb->m_pCurrent+1, offset );
	Heresy_Set32bit( pb->m_pCurrent+2, format );
	Heresy_IncPB( pb, 3);
}

//! indicates we are going to start drawing stuff of 'type' ends with DrawKick
inline void Heresy_DrawMode( restrict Heresy_PushBuffer* pb, uint32_t type )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( DRAW , 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, type );
	Heresy_IncPB( pb, 2);
}

//! all draws are finished with a 'kick' which is just a draw of type 0
inline void Heresy_DrawKick( restrict Heresy_PushBuffer* pb )
{

	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( DRAW , 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, 0x0 );
	Heresy_IncPB( pb, 2);
}

//! register at the next DrawKick a single upto 256 non-index vertex draw call
inline void Heresy_SetVertexArrayParams( restrict Heresy_PushBuffer* pb, uint32_t data )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( VERTEX_ARRAY_PARAMS , 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, data );
	Heresy_IncPB( pb, 2);
}

//! register at the next DrawKick a single upto 256 indexed draw call
inline void Heresy_SetIndexArrayParams( restrict Heresy_PushBuffer* pb, uint32_t data )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( INDEX_ARRAY_PARAMS , 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, data );
	Heresy_IncPB( pb, 2);
}

//! set and draws indexed primitives, takes index buffer etc. so it can reduce push buffer traffic
//! note: ICE aligns things to 512 byte boundarys, so should this probably should in future (or simple pre-align the buffers...) 
inline void Heresy_DrawIndexPrims( restrict Heresy_PushBuffer* pb, uint32_t type, uint32_t start, uint32_t count )
{
	// the start offset is only 24 bits...
	Heresy_Assert( (start & 0xFF000000) == 0 );

	Heresy_DrawMode( pb, type );

	// how many 256 blocks we have and how many tailing primitives
	uint32_t blockcount = count >> 8;
	uint32_t lastcount = count & 0x000000FF;
	uint32_t cmdcount = blockcount + (lastcount!=0);

	Heresy_CheckPBSpace( pb, cmdcount+1 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( INDEX_ARRAY_PARAMS , cmdcount*4) | HPBC_NOINCREMENT );
	Heresy_IncPB( pb, 1);

	for( uint32_t i=0;i < blockcount;i++)
	{
		Heresy_Assert( (start & 0xFF000000) == 0 );
		Heresy_Set32bit( pb->m_pCurrent, (0xFF<<24)| start );
		Heresy_IncPB( pb, 1);
		start += 0x100;
	}
	if( lastcount != 0 )
	{
		Heresy_Assert( (start & 0xFF000000) == 0 );
		Heresy_Set32bit( pb->m_pCurrent, Heresy_IndexArrayParamRegs( lastcount, start ) );
		Heresy_IncPB( pb, 1);
	}

	Heresy_DrawKick( pb );
}

inline void Heresy_SetCullMode( restrict Heresy_PushBuffer* pb, const uint32_t cullmode )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( CULL_FACE, 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, cullmode );
	Heresy_IncPB( pb, 2);
}

inline void Heresy_SetScissorRegion( restrict Heresy_PushBuffer* pb, uint32_t horiz, uint32_t vert )
{
	Heresy_CheckPBSpace( pb, 3 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( SCISSOR_RECT_HORIZ, 8) );
	Heresy_Set32bit( pb->m_pCurrent+1, horiz );
	Heresy_Set32bit( pb->m_pCurrent+2, vert );
	Heresy_IncPB( pb, 3);
}

inline void Heresy_SetFrontPolygonMode( restrict Heresy_PushBuffer* pb, uint32_t reg )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( FRONT_POLYGON_MODE, 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, reg );
	Heresy_IncPB( pb, 2);
}

inline void Heresy_SetBackPolygonMode( restrict Heresy_PushBuffer* pb, uint32_t reg )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( BACK_POLYGON_MODE, 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, reg );
	Heresy_IncPB( pb, 2);
}

inline void Heresy_SetTimestamp( restrict Heresy_PushBuffer* pb, uint32_t timestamp )
{
	Heresy_CheckPBSpace( pb, 2 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( TIME_STAMP, 4) );
	timestamp = (timestamp << 4) + 0x1060;
	Heresy_Set32bit( pb->m_pCurrent+1, timestamp );
	Heresy_IncPB( pb, 2);
}

inline uint64_t Heresy_GetTimestamp( uint32_t timestamp )
{
	timestamp = (timestamp << 4) + 0x1060;
	// evil hard codedness for now...
	uint64_t result = *((uint64_t*)(0x30201400+timestamp));
	return result >> 1;
}

inline void Heresy_TriggerPerfCounter( restrict Heresy_PushBuffer* pb )
{
	Heresy_CheckPBSpace( pb, 4 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( PERF_TRIG0, 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, 0x0 );
	Heresy_Set32bit( pb->m_pCurrent+2, Heresy_Cmd( PERF_TRIG1, 4) );
	Heresy_Set32bit( pb->m_pCurrent+3, 0x1 );
	Heresy_IncPB( pb, 4);
}

inline void Heresy_ResetPerfCounter( restrict Heresy_PushBuffer* pb )
{
	Heresy_CheckPBSpace( pb, 4 );
	Heresy_Set32bit( pb->m_pCurrent, Heresy_Cmd( PERF_TRIG0, 4) );
	Heresy_Set32bit( pb->m_pCurrent+1, 0x0 );
	Heresy_Set32bit( pb->m_pCurrent+2, Heresy_Cmd( PERF_TRIG1, 4) );
	Heresy_Set32bit( pb->m_pCurrent+3, 0x0 );
	Heresy_IncPB( pb, 4);
}

// specific SPU version of key functions
// these uses lot less pointers, hopefully promoted more register usage, of course it may this is right on PPU as well but...
#if defined( __SPU__ )
inline void Heresy_Set128bitPixelShaderConstant_reg( uint16_t restrict* pDestAddr, const Heresy_HalfSwap128 pSrcData )
{
	pDestAddr[0] = pSrcData.shortData[1];
	pDestAddr[1] = pSrcData.shortData[0];
	pDestAddr[2] = pSrcData.shortData[3];
	pDestAddr[3] = pSrcData.shortData[2];
	pDestAddr[4] = pSrcData.shortData[5];
	pDestAddr[5] = pSrcData.shortData[4];
	pDestAddr[6] = pSrcData.shortData[7];
	pDestAddr[7] = pSrcData.shortData[6];
}
inline void Heresy_FixupPixelShaderConstantScratch16B_reg( restrict Heresy_PushBuffer* pb,  const restrict Heresy_PushBufferPatch* pPatch, const v128 vData )
{
	Heresy_Assert( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) == HPBP_PIXEL_CONSTANT_FIXUP );
	// patch relative offset, shifted up by 2
	uint32_t iOffset = (pPatch->m_iData & HPBP_DATA_MASK )>>HPBP_DATA_MASK_LENGTH;

	uint32_t* pDestAddr = ((uint32_t*)pPatch) + iOffset;

	Heresy_HalfSwap128 swapData;
	swapData.v128Data = vData;
	Heresy_Set128bitPixelShaderConstant_reg( (uint16_t* restrict)pDestAddr, swapData );
}
// WARNING! THIS FUNCTION IS DANGEROUS! Use it only when copy aligned (to 16 bytes) data!
#define Heresy_Set128bit_reg( dest, src ) *((v128*)dest) = src
#endif

#endif // end HERESY_CINLINE_H
