/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_RENDERPROCS_H
#define ICE_RENDERPROCS_H

#include "icerender.h"

namespace Ice 
{
	namespace Render 
	{
#ifndef __SPU__
		//! Sets the display swap mode. 
		/*! \param mode  The type of swapping you wish to have. (Horizontal Blank Sync, or Vertical Blank Sync)
		*/
		void SetDisplaySwapMode(DisplaySwapMode mode);

		//! Registers a render target for display.
		/*! \param target  A pointer to a render target object.
		    \param index   The display registry index; must be 0-7.
		*/
		void RegisterRenderTargetForDisplay(RenderTarget *target, U32 index);

		//! Sets up a gpu memory view for the specified region with the specified parameters.
		/*! \param index        The memory view to set; must be 0-7.
		    \param offset       An offset to the start of the surface. Must be 64k aligned.
		    \param context      The location of the surface. (System or Video Memory)
		    \param size         The size in bytes of the surface. Must be 64k Aligned.
		    \param pitch        The pitch of the surface in bytes.
		    \param compType     Tile compression type. For Color/Z/Stencil bandwidth optimization.
		    \param base         The base tag offset. Typically the offset in video memory of the surface divided by 512.
		    \param bank         The access pattern to use. 0 for color surfaces, 1 - 3 for depth surfaces.
		*/
		void SetTileAndCompression(U32 index, U32 offset, TileContext context, U32 size, U32 pitch, CompressionFormat format, U32 base, U32 bank);

		//! Sets up an area of video memory for early depth rejection. 
		/*! \param index     The depth cull area to set; must be 0-7.
		    \param offset    The offset of the surface to enable early depth rejection. 
		    \param width     The width of the surface.
		    \param height    The height of the surface.
		    \param start     The start of the surface to enable early depth rejection. Typically 0.
		    \param depthFmt  The format of the surface to enable early depth rejection on.
		    \param ms        The multisample mode of the surface.
		    \param dir       The direction to optimize.
		    \param cullFmt   The culling format to use.
		    \param func      The stencil function to optimize.
		    \param ref       The stencil reference value to optimize.
		    \param mask      The stencil mask to optimize.
		*/
		void SetDepthCullArea(U32 index, U32 offset, U32 width, U32 height, U32 start, DepthStencilBufferFormat depthFmt, MultisampleMode ms, DepthCullDirection dir, DepthCullFormat cullFmt, ComparisonFunc func, U32 ref, U32 mask);
#endif // __SPU__

		//! Gives the closest representable tileable pitch for the input pitch.
		U32 GetTiledPitch(U32 pitch);

		//! Gives a texture control3 value for use with SetTextureReducedControl3().
		static inline U32 CalculateTextureControl3(U32 brilinearFactor)
		{
			return 0x00002DC0 | brilinearFactor;
		}
	
		//! Converts a color buffer format enum to a matching texture format word.
		/*! \param inFormat  A color buffer format enum value.
		    \return          The matching texture format word.
		*/
		static inline U32 ColorBufferFormatToTextureFormat(ColorBufferFormat inFormat)
		{
			switch (inFormat)
			{
			case kBufferRgb565:   return kTextureRgb565;
			case kBufferArgb8888: return kTextureArgb8888;
			case kBufferB8:       return kTextureLuminance8;
			case kBufferGb88:     return kTextureLuminanceAlpha88;
			case kBufferRgba16f:  return kTextureRgba16f;
			case kBufferRgba32f:  return kTextureRgba32f;
			case kBufferR32f:     return kTextureR32f;
			default:
				ICE_ASSERT(0);
				return 0;
			}
		}

		//! Converts a depth/stencil buffer format enum to a matching texture format word.
		/*! \param inFormat  A depth/stencil buffer format enum value.
		    \return          The matching texture format word.
		*/
		static inline U32 DepthStencilBufferFormatToTextureFormat(DepthStencilBufferFormat inFormat)
		{
			switch (inFormat)
			{
			case kBufferD16S0: return kTextureDepth16;
			case kBufferD24S8: return kTextureDepth24X8;
			default:
				ICE_ASSERT(0);
				return 0;
			}
		}

		//! Converts a texture format word to a matching color buffer format enum.
		/*! NOTE - the match is not exact - channels may be swizzled differently!
		    \param inFormat  A texture format word.
		    \param success   A reference to a boolean used to return whether the conversion succeeeded.
		    \return          The matching color buffer format enum value.
		*/
		static inline ColorBufferFormat TextureFormatToColorBufferFormat(U32 inFormat, bool &success)
		{
			// Extract the 5-bit 'base format';
			U32 baseFormat = (inFormat >> 24) & 0x1F;
			success = true;
			switch (baseFormat)
			{
			case kBtfRgb565:   return kBufferRgb565;
			case kBtfArgb8888: return kBufferArgb8888;
			case kBtfB8:       return kBufferB8;
			case kBtfGb88:     return kBufferGb88;
			case kBtfAbgr16f:  return kBufferRgba16f;
			case kBtfAbgr32f:  return kBufferRgba32f;
			case kBtfR32f:     return kBufferR32f;
			default:
				success = false;
				return kBufferArgb8888;
			}

		}

		//! Converts a texture format word to a matching depth/stencil buffer format enum.
		/*! NOTE - the match is not exact - channels may be swizzled differently!
		    \param inFormat  A texture format word.
		    \param success   A reference to a boolean used to return whether the conversion succeeeded.
		    \return          The matching depth/stencil buffer format enum value.
		*/
		static inline DepthStencilBufferFormat TextureFormatToDepthStencilBufferFormat(U32 inFormat, bool &success)
		{
			// Extract the 5-bit 'base format';
			U32 baseFormat = (inFormat >> 24) & 0x1F;
			success = true;
			switch (baseFormat)
			{
			case kBtfDepth16:   return kBufferD16S0;
			case kBtfDepth24X8: return kBufferD24S8;
			default:
				success = false;
				return kBufferD24S8;
			}
		}

		// SAMPLE FUNCTIONS FOR SETTING TEXTURES FROM RENDER TARGETS AND VICE VERSA
		// To be used either as is, or as a reference for writing your own versions.

		//! Setup a texture from color buffer 0 of a render target.
		/*! \param texture  A pointer to a texture object.
		    \param target   The render target.
		*/
		static inline void SetTextureFromRenderTargetColorBuffer(Texture *texture, const RenderTarget *target)
		{
			U32 width = target->GetWidth();
			U32 height = target->GetHeight();
			U32 pitch = target->IsSwizzled() ? 0 : target->GetColorBufferPitch(0);
			TextureContext context = (target->GetColorBufferContext(0) == kRenderTargetVideoMemory) ? kTextureVideoMemory : kTextureMainMemory;
			MultisampleMode msMode = target->GetMultisampleMode();
			if (msMode != kMultisampleNone) 
			{
				width *= 2;
				if (msMode != kMultisample2X) height *= 2;
			}
			texture->Init();
			texture->SetImage2D(target->m_colorOffset[0], context, 1, width, height,
								ColorBufferFormatToTextureFormat(target->GetColorBufferFormat()),
								pitch);
			texture->SetWrapMode(kTexcoordS, kWrapClampToEdge);
			texture->SetWrapMode(kTexcoordT, kWrapClampToEdge);
			texture->SetLevelRange(0x000, 0x000);
			texture->SetFilterMode(kFilterLinear, kFilterLinear);
		}
		
		//! Setup a depth texture from the depth/stencil buffer of a render target.
		/*! \param texture  A pointer to a texture object.
		    \param target   The render target.
		*/
		static inline void SetTextureFromRenderTargetDepthStencilBuffer(Texture *texture, const RenderTarget *target)
		{
			U32 width = target->GetWidth();
			U32 height = target->GetHeight();
			U32 pitch = target->IsSwizzled() ? 0 : target->GetDepthStencilBufferPitch();
			TextureContext context = (target->GetColorBufferContext(0) == kRenderTargetVideoMemory) ? kTextureVideoMemory : kTextureMainMemory;
			MultisampleMode msMode = target->GetMultisampleMode();
			if (msMode != kMultisampleNone) 
			{
				width *= 2;
				if (msMode != kMultisample2X) height *= 2;
			}
			texture->Init();
			texture->SetImage2D(target->m_depthOffset, context, 1, width, height,
								DepthStencilBufferFormatToTextureFormat(target->GetDepthStencilBufferFormat()),
								pitch);
			texture->SetWrapMode(kTexcoordS, kWrapClampToEdge);
			texture->SetWrapMode(kTexcoordT, kWrapClampToEdge);
			texture->SetLevelRange(0x000, 0x000);
			texture->SetFilterMode(kFilterLinear, kFilterLinear);
			texture->SetDepthFunc(kFuncGreaterEqual);
		}
		
		//! Setup a color-only render target from a texture image.
		/*! NOTE - this render target must only be used with depth and stencil tests and writes disabled!
		    NOTE - the texture must be a 2D texture!
		    NOTE - the format match is not exact - channels may be swizzled differently!
		    \param target   A pointer to a render target.
		    \param texture  The texture.
		    \return         If succeeded, true. If failed, false (render target not initialized in this case).
		*/
		static inline bool SetColorOnlyRenderTargetFromTexture(RenderTarget *target, const Texture *texture)
		{
			bool convertOk;
			ColorBufferFormat colorFormat = TextureFormatToColorBufferFormat(texture->GetFormat(), convertOk);
			if (convertOk) 
			{
				U32 linearImagePitch = texture->GetLinearImagePitch();
				RenderTargetContext context = (texture->GetContext() == kTextureVideoMemory) ? kRenderTargetVideoMemory : kRenderTargetMainMemory;
				U32 flags = (linearImagePitch != 0) ? kTargetLinear : kTargetSwizzled;
				U32 pitch = (linearImagePitch != 0) ? linearImagePitch : 64;
				target->Init(texture->GetWidth(), texture->GetHeight(), colorFormat, kBufferD24S8, kMultisampleNone, flags);
				target->SetColorNoDepthBuffers(texture->m_baseOffset, context, pitch);
			}
			return convertOk;
		}
		
		//! Setup a depth/stencil-only render target from a texture image.
		/*! NOTE - this render target must only be used with color writes disabled!
		    NOTE - the texture must be a 2D texture!
		    NOTE - the format match is not exact - channels may be swizzled differently!
		    \param target   A pointer to a render target.
		    \param texture  The texture.
		    \return         If succeeded, true. If failed, false (render target not initialized in this case).
		*/
		static inline bool SetDepthStencilOnlyRenderTargetFromTexture(RenderTarget *target, const Texture *texture)
		{
			bool convertOk;
			DepthStencilBufferFormat depthStencilFormat = TextureFormatToDepthStencilBufferFormat(texture->GetFormat(), convertOk);
			if (convertOk) 
			{
				U32 linearImagePitch = texture->GetLinearImagePitch();
				RenderTargetContext context = (texture->GetContext() == kTextureVideoMemory) ? kRenderTargetVideoMemory : kRenderTargetMainMemory;
				U32 flags = (linearImagePitch != 0) ? kTargetLinear : kTargetSwizzled;
				U32 pitch = (linearImagePitch != 0) ? linearImagePitch : 64;
				target->Init(texture->GetWidth(), texture->GetHeight(), kBufferArgb8888, depthStencilFormat, kMultisampleNone, flags);
				target->SetDepthNoColorBuffers(texture->m_baseOffset, context, pitch);
			}
			return convertOk;
		}

		//! Calculates a depth cull feedback value for use with CommandContext::SetDepthCullFeedback().
		//! Used to optimize Depth Cull processor performance.
		/*! \param feedbackAReportValue  The value from a kReportDepthCullFeedbackA report.
		    \param feedbackBReportValue  The value from a kReportDepthCullFeedbackB report.
		*/
		static inline U32 CalculateDepthCullFeedback(U32 feedbackAReportValue, U32 feedbackBReportValue)
		{
			U32 maxSlope = feedbackAReportValue >> 16;
			U32 rangeRatio = (maxSlope == 0) ? 0 : (feedbackBReportValue/maxSlope + maxSlope);
			U32 maxComp = Max<U32>(rangeRatio >> 1, 50);
			U32 minComp = Max<U32>(maxComp >> 1, 50);
			return (maxComp << 16) | (minComp & 0xFFFF);
		}

#ifndef __SPU__
		//! This function controls the amount of debugging information output to the console.
		/*! Debugging information is output when the graphics pipeline hangs or 
		    when an exception occurs, and is used by the application to diagnose
			the cause of the problem.
			\param level  The debugging level to perform on error.
		*/
		void SetDebugOutputLevel(DebugOutputLevel level);

		//! The specified function is called when an exception has occurred on the Gpu.
		/*! This allows the program to validate the last push buffer sent to the gpu.
		    \param callback  The function to register.
		*/
		void SetGpuExceptionCallback(void (*callback)(U32 const head));

		//! The specified function is called when a swap command has started execution by the Gpu.
		/*! \param callback  The function to register.
		*/
		void SetSwapStartCallback(void (*callback)(U32 const head));

		//! The specified function is called when a swap command has finished execution by the Gpu.
		/*! \param callback  The function to register.
		*/
		void SetSwapCompleteCallback(void (*callback)(U32 const head));

		//! The specified function is called when a vblank has started.
		/*! \param callback  The function to register.
		*/
		void SetVBlankStartCallback(void (*callback)(U32 const head));

		//! The specified function is called when a ppu interrupt command is executed by the Gpu.
		/*! \param callback  The function to register. See CommandContext::InsertPpuInterrupt().
		*/
		void SetPpuInterruptCallback(void (*callback)(U32 const parameterValue));

		//! Sets the events that will be output when a WritePerformanceReport 
		//! command is executed by the gpu.
		/*! \param domain  The domain of events to set.
		    \param events  An array of four events that you want to measure in 
		                   the specified domain.
		*/
		void SetPerformanceReportEvents(DomainType domain, U32 events[4]);
		
		//! Gets the events that were output when a WritePerformanceReport 
		//! command is executed by the gpu.
		/*! \param domain    The domain of events to get.
		    \param counters  An array of four elements which will be written
		                     with the results of the report.
		    \param cycles    An array of two elements which will be written 
			                 with the number of cycles between performance 
							 timings.
		*/
		void GetPerformanceReportResults(DomainType domain, U32 counters[4], U32 cycles[2]);

		//! Single steps the gpu using the Gpu Control MMRs to help detect gpu crashes.
		/*! \param putEnd  The put offset that the gpu needs to be at to stop single stepping.
		*/
		void SingleStepPusher(unsigned int putEnd);
		
		//! Validates a push buffer for correctness. 
		/*! Does simple checking on common push buffer errors.
		    \param start        A pointer to the first command to validate.
		    \param end          A pointer to the last command to validate.
		    \param outAddr      A pointer that gets written the offset of the last validated command.
		    \param outReg       A pointer that gets written the register of the last validated command.
		    \param outValue     A pointer that gets written the value that was written to the last register of the last validated command.
			\param level        The level of validation you wish to perform. 
			\param outWarnings  A pointer that gets written the warning flags as specified by the ValidationWarning enum.
		    \return             The error enum which gives a specific error type. 
		*/
		ValidationError ValidatePushBuffer(void *start, void *end, U32* outAddr=NULL, U32 *outReg=NULL, U32 *outValue=NULL, ValidationLevel level=kValidateNormal, U64 *outWarnings=NULL);

#endif // __SPU__
	}
}

#endif // ICE_RENDERPROCS_H
