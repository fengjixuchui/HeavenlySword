/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_RENDERTARGET_H
#define ICE_RENDERTARGET_H

namespace Ice
{
	namespace Render
	{
		struct RenderTarget
		{
			U32 m_displayIndex;                    // Libgcm registry
			U32 m_targetLeftWidth;                 // Register 0x0200
			U32 m_targetTopHeight;                 // Register 0x0204
			U32 m_targetFormat;	                   // Register 0x0208
			U32 m_colorPitch[4];                   // Registers 0x020C, 0x021C, 0x0280, 0x0284
			U32 m_colorOffset[4];                  // Registers 0x0210, 0x0218, 0x0288, 0x028C
			RenderTargetContext m_colorContext[4]; // Registers 0x0194, 0x018C, 0x01B4, 0x01B8  
			U32 m_depthOffset;                     // Register 0x0214
			U32 m_depthPitch;                      // Register 0x022C
			RenderTargetContext m_depthContext;    // Register 0x0198	   
			U32 m_targetOrigin;                    // Register 0x02B8
			U32 m_targetAuxHeight;                 // Register 0x1D88
			
			void Init(U32 width, U32 height, ColorBufferFormat colorFormat, DepthStencilBufferFormat depthFormat, MultisampleMode multisample, U32 flags);
			
			void SetColorAndDepthBuffers(U32 colorBufferCount, U32 const *colorBufferOffsets, RenderTargetContext const *colorBufferContexts, U32 const *colorBufferPitches,
				U32 depthBufferOffset, RenderTargetContext depthContext, U32 depthBufferPitch);
			void SetColorAndDepthBuffers(U32 colorBufferOffset, RenderTargetContext colorBufferContext, U32 colorBufferPitch,
				U32 depthBufferOffset, RenderTargetContext depthContext, U32 depthBufferPitch);

			void SetColorNoDepthBuffers(U32 colorBufferCount, U32 const *colorBufferOffsets, RenderTargetContext const *colorContexts, U32 const *colorBufferPitches);
			void SetColorNoDepthBuffers(U32 colorBufferCount, RenderTargetContext colorContext, U32 colorBufferPitch);

			void SetDepthNoColorBuffers(U32 depthBufferOffset, RenderTargetContext depthContext, U32 depthBufferPitch);
			
			ColorBufferFormat GetColorBufferFormat() const;
			DepthStencilBufferFormat GetDepthStencilBufferFormat() const;
			MultisampleMode GetMultisampleMode() const;
#ifndef __SPU__
			void *GetColorBufferAddress(U32 bufferIndex) const;
			void *GetDepthStencilBufferAddress() const;
#endif
			bool IsSwizzled() const;
			U32 GetWidth() const;
			U32 GetHeight() const;
			U32 GetColorBufferPitch(U32 bufferIndex = 0) const;
			U32 GetDepthStencilBufferPitch() const;
			RenderTargetContext GetColorBufferContext(U32 bufferIndex = 0) const;
			RenderTargetContext GetDepthStencilBufferContext() const;
		};
		
		//
		// RENDER TARGET FUNCTIONS
		//
		
		inline ColorBufferFormat RenderTarget::GetColorBufferFormat() const
		{
			union
			{
				U32 from;
				ColorBufferFormat to;
			} convert;
			
			convert.from = m_targetFormat & 0x1F;
			return convert.to;
		}

		inline DepthStencilBufferFormat RenderTarget::GetDepthStencilBufferFormat() const
		{
			union
			{
				U32 from;
				DepthStencilBufferFormat to;
			} convert;
			
			convert.from = m_targetFormat & 0xE0;
			return convert.to;
		}

		inline MultisampleMode RenderTarget::GetMultisampleMode() const
		{
			union
			{
				U32 from;
				MultisampleMode to;
			} convert;
			
			convert.from = (m_targetFormat >> 12) & 0xF;
			return convert.to;
		}

#ifndef __SPU__
		inline void *RenderTarget::GetColorBufferAddress(U32 bufferIndex) const
		{
			if(GetColorBufferContext(bufferIndex) == kRenderTargetVideoMemory)
				return TranslateOffsetToAddress(m_colorOffset[bufferIndex]);
			else
				return TranslateIoOffsetToAddress(m_colorOffset[bufferIndex]);
		}

		inline void *RenderTarget::GetDepthStencilBufferAddress() const
		{
			if(GetDepthStencilBufferContext() == kRenderTargetVideoMemory)
				return TranslateOffsetToAddress(m_depthOffset);
			else
				return TranslateIoOffsetToAddress(m_depthOffset);
		}
#endif

		inline bool RenderTarget::IsSwizzled() const
		{
			return ((m_targetFormat >> 8) & 0x3) == 0x2;
		}

		inline U32 RenderTarget::GetWidth() const
		{
			return (m_targetLeftWidth >> 16);
		}
		
		inline U32 RenderTarget::GetHeight() const
		{
			return (m_targetTopHeight >> 16);
		}

		inline U32 RenderTarget::GetColorBufferPitch(U32 bufferIndex) const
		{
			return m_colorPitch[bufferIndex];
		}

		inline U32 RenderTarget::GetDepthStencilBufferPitch() const
		{
			return m_depthPitch;
		}

		inline RenderTargetContext RenderTarget::GetColorBufferContext(U32 bufferIndex) const
		{
			return m_colorContext[bufferIndex];
		}

		inline RenderTargetContext RenderTarget::GetDepthStencilBufferContext() const
		{
			return m_depthContext;
		}
		
		//! Initializes a render target object.
		/*! \param target       A pointer to the new render target object.
		    \param width        The width of the render target.
		    \param height       The height of the render target.
		    \param colorFormat  The pixel format of the render target's color buffers.
		    \param depthFormat  The pixel format of the render target's depth/stencil buffer.
		    \param multisample  The multisampling mode used by the render target.
		    \param flags        Render target flags. Currently, must be either 0 or kTargetSwizzled.
		*/
		static inline void InitRenderTarget(RenderTarget *target, U32 width, U32 height, ColorBufferFormat colorFormat, DepthStencilBufferFormat depthFormat, MultisampleMode multisample, U32 flags)
		{
			target->Init(width, height, colorFormat, depthFormat, multisample, flags);
		}

		//! Assigns one to four color buffers and one depth buffer to a render target.
		/*! \param target         A pointer to a render target object.
		    \param colorCount     The number of color buffers to assign to the target; must be 1-4.
		    \param colorOffsets   A pointer to an array of color buffer offsets. The array must hold colorCount elements.
			\param colorContexts  A pointer to an array of color buffer contexts. The array must hold colorCount elements.
		    \param colorPitches   A pointer to an array of color buffer row pitches. The array must hold colorCount elements.
		    \param depthAddress   The address of the depth buffer.
		    \param depthPitch     The row pitch of the depth buffer.
		*/
		static inline void SetRenderTargetColorAndDepthBuffers(RenderTarget *target, 
			U32 colorCount, U32 const *colorOffsets, RenderTargetContext const *colorContexts, U32 const *colorPitches,
			U32 depthOffset, RenderTargetContext depthContext, U32 depthPitch)
		{
			target->SetColorAndDepthBuffers(colorCount, colorOffsets, colorContexts, colorPitches, depthOffset, depthContext, depthPitch);
		}

		//! Assigns one depth buffer and one color buffer to a render target.
		/*! \param target        A pointer to a render target object.
		    \param colorOffset   The offset from the base address of the context specified for the color buffer. 
			\param colorContext  The context of where the color buffer is located.
		    \param colorPitch    The row pitch of the color buffer. 
		    \param depthOffset   The offset from the base address of the context specifed for the depth buffer.
			\param depthContext  The context of where the depth buffer is located.
		    \param depthPitch    The row pitch of the depth buffer.
		*/
		static inline void SetRenderTargetColorAndDepthBuffers(RenderTarget *target, 
			U32 colorOffset, RenderTargetContext colorContext, U32 colorPitch, 
			U32 depthOffset, RenderTargetContext depthContext, U32 depthPitch)
		{
			target->SetColorAndDepthBuffers(colorOffset, colorContext, colorPitch, depthOffset, depthContext, depthPitch);
		}

		//! Assigns one or more color buffers to a render target. The depth buffer is equal to the first color buffer.
		/*! \param target         A pointer to a render target object.
		    \param colorCount     The number of color buffers to assign to the target; must be 1-4.
		    \param colorOffsets   A pointer to an array of color buffer offsets. The array must hold colorCount elements.
			\param colorContexts  A pointer to an array of color buffer contexts. The array must hold colorCount elements.
		    \param colorPitches   A pointer to an array of color buffer row pitches. The array must hold colorCount elements.
		*/
		static inline void SetRenderTargetColorNoDepthBuffers(RenderTarget *target, 
			U32 colorCount, U32 const *colorOffsets, RenderTargetContext const *colorContexts, U32 const *colorPitches)
		{
			target->SetColorNoDepthBuffers(colorCount, colorOffsets, colorContexts, colorPitches);
		}
		
		//! Assigns one color buffer to a render target. The depth buffer is equal to the color buffer.
		/*! \param target        A pointer to a render target object.
		    \param colorOffset   The offset from the base address of the context specified for the color buffer.
			\param colorContext  The context of where the color buffer is located.
		    \param colorPitch    The row pitch of the color buffer. 
		*/
		static inline void SetRenderTargetColorNoDepthBuffers(RenderTarget *target, U32 colorOffset, RenderTargetContext colorContext, U32 colorPitch)
		{
			target->SetColorNoDepthBuffers(colorOffset, colorContext, colorPitch);
		}

		//! Assigns one depth buffer to a render target. The color buffers are equal to the depth buffer.
		/*! \param target        A pointer to a render target object.
		    \param depthOffset   The offset from the base address of the context specifed for the depth buffer.
			\param depthContext  The context of where the depth buffer is located.
		    \param depthPitch    The row pitch of the depth buffer.
		*/
		static inline void SetRenderTargetDepthNoColorBuffers(RenderTarget *target, U32 depthOffset, RenderTargetContext depthContext, U32 depthPitch)
		{
			target->SetDepthNoColorBuffers(depthOffset, depthContext, depthPitch);
		}
	}
}

#endif // ICE_RENDERTARGET_H
