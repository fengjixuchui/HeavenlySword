/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_COMMANDCONTEXT_H
#define ICE_COMMANDCONTEXT_H

namespace Ice 
{
	namespace Render 
	{
		struct CommandContextData;

		//! The CommandCallback function is called when an attempt is made
		//! to add commands to a command buffer, but there is not enough space.
		/*! This function should return true if it successfully extended the command buffer. 
		*/
		typedef bool (*CommandCallback)(CommandContextData *, U32);

#ifdef __SPU__
		struct ICE_ALIGN(16) CommandContextData
#else
		struct CommandContextData
#endif
		{
			//! The beginning of the command buffer.
			U32 *m_beginptr;
			//! The end of the space allocated to the command buffer.
			U32 *m_endptr;
			//! The current location where new commands are added.
			U32 *m_cmdptr;
			//! Called when commands can't be added because the buffer is full.
			CommandCallback m_callback;
		};

		#include "icecommandcontextprototypes.inl"

		namespace Unsafe
		{
			#include "icecommandcontextprototypes.inl"
		}

		namespace Inline
		{
			#include "icecommandcontextprototypes.inl"

			#define ICERENDER_UNSAFE 0
			#define ICERENDER_INLINE 1
			#include "icecommandcontextcore.inl"
			#undef ICERENDER_INLINE
			#undef ICERENDER_UNSAFE
		}

		namespace InlineUnsafe
		{
			#include "icecommandcontextprototypes.inl"

			#define ICERENDER_UNSAFE 1
			#define ICERENDER_INLINE 1
			#include "icecommandcontextcore.inl"
			#undef ICERENDER_INLINE
			#undef ICERENDER_UNSAFE
		}

		//! Creates a jump command to vramAddress at the location specified.
		/* \param location  The location in memory to create the command.
		   \param offset    The offset that the hardware should jump to when hiting this command.
		*/
		inline void SetJumpAt(U32 *location, U32 offset)
		{
			*location = offset | kCmdJumpFlag;
		}

		//! Creates a call command to vramAddress at the location specified.
		/*! \param location  The location in memory to create the command.
		    \param offset    The offset that the hardware should jump to when hiting this command.
		*/
		inline void SetCallAt(U32 *location, U32 offset)
		{
			*location = offset | kCmdCallFlag;
		}

		//! Creates a return command at the location specified.
		/*! \param location The location in memory to create the command.
		*/
		inline void SetReturnAt(U32 *location)
		{
			*location = kCmdReturnFlag;
		}

		//! Creates a no operation command at the location specified.
		/*! \param location The location in memory to create the command.
		*/
		inline void SetNopAt(U32 *location)
		{
			*location = 0;
		}

		//
		// Get*Reserve() functionality.
		// Gives the number of commands that would be needed to execute the corresponding IceRender command.
		//

		inline U32F GetSetRenderTargetReserve() { return 30; }
		inline U32F GetEnableRenderStateReserve() { return 2; }
		inline U32F GetDisableRenderStateReserve() { return 2; }
		inline U32F GetToggleRenderStateReserve() { return 2; }
		inline U32F GetSetDepthMinMaxControlReserve() { return 2; }
		inline U32F GetSetDrawBufferMaskReserve() { return 2; }
		inline U32F GetSetAlphaFuncReserve() { return 3; }
		inline U32F GetSetMrtBlendEnableReserve() { return 4; }
		inline U32F GetSetBlendFuncReserve() { return 3; }
		inline U32F GetSetBlendFuncSeparateReserve() { return 3; }
		inline U32F GetSetBlendEquationReserve() { return 2; }
		inline U32F GetSetBlendEquationSeparateReserve() { return 2; }
		inline U32F GetSetBlendColorReserve() { return 2; }
		inline U32F GetSetFloatBlendColorReserve() { return 4; }
		inline U32F GetSetColorMaskReserve() { return 4; }
		inline U32F GetSetMrtColorMaskReserve() { return 4; }
		inline U32F GetSetDepthMaskReserve() { return 2; }
		inline U32F GetSetStencilMaskReserve() { return 4; }
		inline U32F GetSetStencilMaskSeparateReserve() { return 4; }
		inline U32F GetSetStencilFuncReserve() { return 8; }
		inline U32F GetSetStencilFuncSeparateReserve() { return 8; }
		inline U32F GetSetStencilOpReserve() { return 8; }
		inline U32F GetSetStencilOpSeparateReserve() { return 8; }
		inline U32F GetSetShadeModelReserve() { return 2; }
		inline U32F GetSetLogicOpReserve() { return 2; }
		inline U32F GetSetFogModeReserve() { return 2; }
		inline U32F GetSetFogRangeReserve() { return 3; }
		inline U32F GetSetFogDensityReserve() { return 3; }
		inline U32F GetSetDepthBoundsReserve() { return 3; }
		inline U32F GetSetViewportReserve() { return 15; }
		inline U32F GetSetScissorReserve() { return 3; }
		inline U32F GetSetDepthFuncReserve() { return 2; }
		inline U32F GetSetPolygonOffsetReserve() { return 3; }
		inline U32F GetSetPolygonStipplePatternReserve() { return 33; }
		inline U32F GetSetLineStipplePatternReserve() { return 2; }
		inline U32F GetSetLineWidthReserve() { return 2; }
		inline U32F GetSetClipFuncReserve() { return 2; }
		inline U32F GetSetPrimitiveRestartIndexReserve() { return 2; }
		inline U32F GetSetPolygonModeReserve() { return 3; }
		inline U32F GetSetPolygonModeSeparateReserve() { return 3; }
		inline U32F GetSetCullFaceReserve() { return 2; }
		inline U32F GetSetFrontFaceReserve() { return 2; }
		inline U32F GetSetClearColorReserve() { return 2; }
		inline U32F GetSetClearDepthStencilReserve() { return 2; }
		inline U32F GetClearReserve() { return 2; }
		inline U32F GetSetMultisampleParametersReserve() { return 2; }
		inline U32F GetSetPointSizeReserve() { return 2; }
		inline U32F GetSetPointSpriteParametersReserve() { return 2; }
		inline U32F GetSetVertexAttribPointerReserve() { return 2; }
		inline U32F GetSetVertexAttribBaseOffsetReserve() { return 2; }
		inline U32F GetSetVertexAttribFormatReserve() { return 2; }
		inline U32F GetSetVertexAttribFrequencyModeReserve() { return 2; }
		inline U32F GetDisableVertexAttribArrayReserve() { return 2; }
		inline U32F GetSetVertexAttrib1fReserve() { return 2; }
		inline U32F GetSetVertexAttrib2fReserve() { return 3; }
		inline U32F GetSetVertexAttrib3fReserve() { return 4; }
		inline U32F GetSetVertexAttrib4fReserve() { return 5; }
		inline U32F GetSetVertexAttrib2sReserve() { return 2; }
		inline U32F GetSetVertexAttrib4sReserve() { return 3; }
		inline U32F GetSetVertexAttrib4NsReserve() { return 3; }
		inline U32F GetSetVertexAttrib4NubReserve() { return 2; }
		inline U32F GetDrawArraysReserve(U32 count) { return count ? ((count+0x5FF)>>8) : 0; }
		inline U32F GetDrawArraysReserveMax() { return 516; }
		inline U32F GetDrawElementsReserve(U32 count) { return count ? ((count+0x8FF)>>8) : 0; }
		inline U32F GetDrawElementsReserveMax() { return 516; }
		inline U32F GetDrawInstancedElementsReserve(U32 count, U32 instanceCount) // No reserve maximum
		{
			if ((count == 0) || (instanceCount == 0))
				return 0;
			else
				return 9 + ((count+0x3FF)>>8)*instanceCount; 
		}
		inline U32F GetBeginReserve() { return 2; }
		inline U32F GetInsertIndexArrayOffsetAndFormatReserve() { return 3; }
		inline U32F GetInsertDrawReserve(U32 count) { return (count + 0x1FF) >> 8; }
		inline U32F GetEndReserve() { return 2; }
		inline U32F GetDrawImmediateArraysReserve(U32 size) // No maximum reservation.
		{ 
			if (size == 0) 
				return 0;

			size >>= 2;	
			U32F blockCount = size / 511;
			U32F remainCount = size - blockCount * 511;
			return 4 + blockCount + (blockCount * 511) + remainCount + (remainCount != 0); 
		}
		inline U32F GetDrawImmediateElementsReserve(U32 count, IndexType type)  // No maximum reservation.
		{ 
			U32F size = (count << ((type == kIndex16) ? 1 : 2)) + 3;
			size >>= 2;	
			U32F blockCount = size / 511;
			U32F const remainCount = size - blockCount * 511;
			return 4 + blockCount + (blockCount * 511) + remainCount + (remainCount != 0); 
		}
		inline U32F GetInvalidatePreTransformCacheReserve() { return 2; }
		inline U32F GetInvalidatePostTransformCacheReserve() { return 2; }
		inline U32F GetSetVertexProgramStaticBranchBitsReserve() { return 2; }
		inline U32F GetSelectVertexProgramReserve(VertexProgram const *program)	{ return 7 + program->m_constantCount * 6; }
		inline U32F GetLoadVertexProgramReserve(VertexProgram const *program) { return 3 + (program->m_instructionCount >> 3) * 33 + (program->m_instructionCount & 7) * 4; }
		inline U32F GetSetVertexProgramReserve(VertexProgram const *program) 
		{ 
			return GetLoadVertexProgramReserve(program) + GetSelectVertexProgramReserve(program);
		}
		inline U32F GetSetVertexProgramReserveMax() { return 5640; }
		inline U32F GetSetVertexProgramConstantReserve() { return 6; }
		inline U32F GetSetVertexProgramConstantsReserve(U32 count) 
		{ 
			U32F const blockCount = count / 8;
			U32F remainCount = count - blockCount * 8;
			return blockCount * 34 + remainCount*4 + (remainCount != 0) * 2; 
		}
		inline U32F GetSetVertexProgramConstantsReserveMax() { return 2304; }
		inline U32F GetSetFragmentProgramReserve() { return 15; }
		inline U32F GetSetNullFragmentProgramReserve() { return 15; }
		inline U32F GetRefreshFragmentProgramReserve() { return 2; }
		inline U32F GetSetFragmentProgramConstantReserve(FragmentProgram *program, U32 index)  // No maximum reserve
		{ 
			if (index >= program->m_patchCount)
				return 0;
			
			PatchData const *patchData = program->GetPatchData(index);
			U32F patchCount = patchData->m_count;
			if(patchCount == 0)
				return 0;

			return 8 + 9*patchCount; 
		}
		inline U32F GetSetFragmentProgramConstantsReserve(FragmentProgram *program, U32 start, U32 count)  // No maximum reserve
		{ 
			if(count == 0)
				return 0;

			U32F reserveCount = 8;
			U32F end = start + count;
			for (U32F index = start; index < end; ++index)
			{
				PatchData const *patchData = program->GetPatchData(index);
				reserveCount += 9*patchData->m_count;
			}

			return reserveCount; 
		}
		inline U32F GetSetTextureCylindricalWrapEnableReserve() { return 3; }
		inline U32F GetSetTextureReserve() { return 15; }
		inline U32F GetSetTextureReducedReserve() { return 10; }
		inline U32F GetSetTextureReducedBorderColorReserve() { return 2; }
		inline U32F GetSetTextureReducedBorderDepthReserve() { return 2; }
		inline U32F GetSetTextureReducedColorKeyReserve() { return 2; }
		inline U32F GetSetTextureReducedControl3Reserve() { return 2; }
		inline U32F GetDisableTextureReserve() { return 2; }
		inline U32F GetSetVertexProgramTextureReserve() { return 9; }
		inline U32F GetDisableVertexProgramTextureReserve() { return 2; }
		inline U32F GetInvalidateTextureCacheReserve() { return 2; }
		inline U32F GetInvalidateVertexTextureCacheReserve() { return 2; }
		inline U32F GetInvalidateDepthCullReserve() { return 2; }
		inline U32F GetSetDepthCullControlReserve() { return 2; }
		inline U32F GetSetStencilCullHintReserve() { return 2; }
		inline U32F GetSetDepthCullFeedbackReserve() { return 2; }
		inline U32F GetInsertReferenceReserve() { return 2; }
		inline U32F GetSetWaitSemaphoreIndexReserve() { return 2; }
		inline U32F GetWaitOnSemaphoreReserve() { return 2; }
		inline U32F GetSetFlushPipeSemaphoreIndexReserve() { return 2; }
		inline U32F GetFlushTexturePipeAndWriteSemaphoreReserve() { return 2; }
		inline U32F GetFlushBackendPipeAndWriteSemaphoreReserve() { return 2; }
		inline U32F GetInsertJumpReserve() { return 1; }
		inline U32F GetInsertCallReserve() { return 1; }
		inline U32F GetInsertReturnReserve() { return 1; }
		inline U32F GetInsertNopReserve() { return 2; }
		inline U32F GetInsertHoleReserve(U32 size) { return size/4+3; }
		inline U32F GetInsertDataReserve(U32 size) { return size/4; }
		inline U32F GetResetReportReserve() { return 2; }
		inline U32F GetWriteReportReserve() { return 2; }
		inline U32F GetSetRenderControlReserve() { return 4; }
		inline U32F GetSetWindowClipModeReserve() { return 2; }
		inline U32F GetSetWindowReserve() { return 3; }
		inline U32F GetInsertDebugStringReserve(const char *debugString) { return 3+strlen(debugString)/4; }
		inline U32F GetInsertDebugStringReserveMax() { return 512; }

	}
}

#endif // ICE_COMMANDCONTEXT_H
