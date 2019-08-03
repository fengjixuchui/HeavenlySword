/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_TEXTUREREDUCED_H
#define ICE_TEXTUREREDUCED_H

#include "icebitextract.h"

namespace Ice
{
	namespace Render
	{
		struct RenderTarget;

#ifdef __SPU__
		struct ICE_ALIGN(16) TextureReduced
#else
		struct TextureReduced
#endif
		{
			U32 m_baseOffset;        // Register 0x1A00
			U32 m_format;            // Register 0x1A04
			U32 m_control1;          // Register 0x1A08
			U32 m_control2;          // Register 0x1A0C
			U32 m_swizzle;           // Register 0x1A10
			U32 m_filter;            // Register 0x1A14
			U32 m_size1;             // Register 0x1A18
			U32 m_size2;             // Register 0x1840
			
			void Init();
			
			void SetImage1D(U32 offset, TextureContext context, U32 levelCount, U32 width, U32 format, U32 pitch = 0);
			void SetImage2D(U32 offset, TextureContext context, U32 levelCount, U32 width, U32 height, U32 format, U32 pitch = 0);
			void SetImage3D(U32 offset, TextureContext context, U32 levelCount, U32 width, U32 height, U32 depth, U32 format, U32 pitch = 0);
			void SetImageCube(U32 offset, TextureContext context, U32 levelCount, U32 size, U32 format, U32 pitch = 0);
			void SetImageRect(U32 offset, TextureContext context, U32 width, U32 height, U32 format, U32 pitch = 0);
			
			void SetWrapMode(TexcoordComponent coord, WrapMode mode);
			void SetFilterMode(FilterMode magFilter, FilterMode minFilter);
			void SetLevelRange(U32 minLevel, U32 maxLevel);
			void SetLevelBias(I32 bias);
			void SetMaxAnisotropy(AnisotropyLevel anisotropy);
			void SetDepthFunc(ComparisonFunc func);
			void EnableSignedExpand();
			void DisableSignedExpand();
			void SetGammaCorrect(U32 mask);
			void SetColorKeyMode(ColorKeyMode mode);
			void SetKernelMode(KernelMode mode);

#ifndef __SPU__
			void *GetImageAddress() const;
#endif
			U32 GetFormat() const;
			U32 GetWidth() const;
			U32 GetHeight() const;
			U32 GetLinearImagePitch() const;
			TextureContext GetContext() const;
		};
		
		//
		// TEXTURE FUNCTIONS
		//
		
		inline void TextureReduced::Init()
		{
			m_baseOffset = 0;
			m_format = 0;
			// Wrap modes = kWrapRepeat
			m_control1 = 0x00010101;		
			// Min level = 0.0, Max level = 15.0
			m_control2 = 0x80078000;		
			m_swizzle = 0;
			// Magnify = kFilterLinear
			// Minify = kFilterNearestMipmapLinear
			// Kernel = kKernelNormal
			// Level bias = 0.0
			m_filter = 0x02052000;			
			m_size1 = 0;
			m_size2 = 0;
		}

		inline void TextureReduced::SetImage1D(U32 offset, TextureContext context, U32 levelCount, U32 width, U32 format, U32 pitch)
		{
			ICE_ASSERTF((offset & 0x7F) == 0, ("Misaligned texture image."));
			ICE_ASSERT(width <= 0x1000);
			m_baseOffset = offset;
			m_format = (pitch == 0 ? 0x8018 : 0xA018) | (levelCount << 16) | ((format >> 16) & 0x1F00) | context;
			m_swizzle = format & 0x1FFFF;
#ifdef __SPU__
			m_filter = ((format << 8) & 0xF0000000) | (m_filter & 0x0FFFFFFF);
#else
			m_filter = Rlwimi(m_filter, format, 8, 0, 3);
#endif
			m_control1 = (m_control1 & 0xFFF0F0FF) | ((kWrapClampToEdge << 16) | (kWrapClampToEdge << 8));
			m_size1 = (width << 16) | 0x0001;
#ifdef __SPU__
			m_size2 = 0x00100000 | (pitch & 0x000FFFFF);
#else
			register U32 size2 = 0x00100000;
			m_size2 = Rlwimi(size2, pitch, 0, 12, 31);
#endif
		}

		inline void TextureReduced::SetImage2D(U32 offset, TextureContext context, U32 levelCount, U32 width, U32 height, U32 format, U32 pitch)
		{
			ICE_ASSERTF((offset & 0x7F) == 0, ("Misaligned texture image."));
			ICE_ASSERT(width <= 0x1000);
			ICE_ASSERT(height <= 0x1000);
			m_baseOffset = offset;
			m_format = (pitch == 0 ? 0x8028 : 0xA028) | (levelCount << 16) | ((format >> 16) & 0x1F00) | context;
			m_swizzle = format & 0x1FFFF;
#ifdef __SPU__
			m_filter = ((format << 8) & 0xF0000000) | (m_filter & 0x0FFFFFFF);
			m_control1 = ((kWrapClampToEdge << 16) & 0x000F0000) | (m_control1 & 0xFFF0FFFF);
#else
			m_filter = Rlwimi(m_filter, format, 8, 0, 3);
			m_control1 = Rlwimi(m_control1, kWrapClampToEdge, 16, 12, 15);
#endif
			m_size1 = (width << 16) | height;
#ifdef __SPU__
			m_size2 = 0x00100000 | (pitch & 0x000FFFFF);
#else
			register U32 size2 = 0x00100000;
			m_size2 = Rlwimi(size2, pitch, 0, 12, 31);
#endif
		}

		inline void TextureReduced::SetImage3D(U32 offset, TextureContext context, U32 levelCount, U32 width, U32 height, U32 depth, U32 format, U32 pitch)
		{
			ICE_ASSERTF((offset & 0x7F) == 0, ("Misaligned texture image."));
			ICE_ASSERT(width <= 0x0200);
			ICE_ASSERT(height <= 0x0200);
			ICE_ASSERT(depth <= 0x0200);
			m_baseOffset = offset;
			m_format = (pitch == 0 ? 0x8038 : 0xA038) | (levelCount << 16) | ((format >> 16) & 0x1F00) | context;
			m_swizzle = format & 0x1FFFF;
#ifdef __SPU__
			m_filter = ((format << 8) & 0xF0000000) | (m_filter & 0x0FFFFFFF);
#else
			m_filter = Rlwimi(m_filter, format, 8, 0, 3);
#endif
			m_control1 = 0x00010101;
			m_size1 = (width << 16) | height;
#ifdef __SPU__
			m_size2 = (depth << 20) | (pitch & 0x000FFFFF);
#else
			register U32 size2 = depth << 20;
			m_size2 = Rlwimi(size2, pitch, 0, 12, 31);
#endif
		}

		inline void TextureReduced::SetImageCube(U32 offset, TextureContext context, U32 levelCount, U32 size, U32 format, U32 pitch)
		{
			ICE_ASSERTF((offset & 0x7F) == 0, ("Misaligned texture image."));
			ICE_ASSERT(size <= 0x1000);
			m_baseOffset = offset;
			m_format = (pitch == 0 ? 0x802C : 0xA02C) | (levelCount << 16) | ((format >> 16) & 0x1F00) | context;
			m_swizzle = format & 0x1FFFF;
#ifdef __SPU__
			m_filter = ((format << 8) & 0xF0000000) | (m_filter & 0x0FFFFFFF);
			m_control1 = ((kWrapClampToEdge << 16) & 0x000F0000) | (m_control1 & 0xFFF0FFFF);
#else
			m_filter = Rlwimi(m_filter, format, 8, 0, 3);
			m_control1 = Rlwimi(m_control1, kWrapClampToEdge, 16, 12, 15);
#endif
			m_size1 = (size << 16) | size;
#ifdef __SPU__
			m_size2 = 0x00100000 | (pitch & 0x000FFFFF);
#else
			register U32 size2 = 0x00100000;
			m_size2 = Rlwimi(size2, pitch, 0, 12, 31);
#endif
		}

		inline void TextureReduced::SetImageRect(U32 offset, TextureContext context, U32 width, U32 height, U32 format, U32 pitch)
		{
			ICE_ASSERTF((offset & 0x7F) == 0, ("Misaligned texture image."));
			ICE_ASSERT(width <= 0x1000);
			ICE_ASSERT(height <= 0x1000);
			m_baseOffset = offset;
			m_format = (pitch == 0 ? 0x1C028 : 0x1E028) | ((format >> 16) & 0x1F00) | context;
			m_swizzle = format & 0x1FFFF;
#ifdef __SPU__
			m_filter = ((format << 8) & 0xF0000000) | (m_filter & 0x0FFFFFFF);
			m_control1 = ((kWrapClampToEdge << 16) & 0x000F0000) | (m_control1 & 0xFFF0FFFF);
#else
			m_filter = Rlwimi(m_filter, format, 8, 0, 3);
			m_control1 = Rlwimi(m_control1, kWrapClampToEdge, 16, 12, 15);
#endif
			m_size1 = (width << 16) | height;
#ifdef __SPU__
			m_size2 = 0x00100000 | (pitch & 0x000FFFFF);
#else
			register U32 size2 = 0x00100000;
			m_size2 = Rlwimi(size2, pitch, 0, 12, 31);
#endif
		}

		inline void TextureReduced::SetWrapMode(TexcoordComponent coord, WrapMode mode)
		{
			// To use border wrap modes, use a regular non-reduced texture structure.
			ICE_ASSERT(mode != kWrapClampToBorder);
			ICE_ASSERT(mode != kWrapMirrorClampToBorder);
			U32 shift = coord * 8;
			U32 mask = 0x0F << shift;
			m_control1 = (m_control1 & ~mask) | (mode << shift);
		}

		inline void TextureReduced::SetFilterMode(FilterMode magFilter, FilterMode minFilter)
		{
#ifdef __SPU__
			m_filter = (magFilter << 24) | (minFilter << 16) | (m_filter & 0xF0F0FFFF);
#else
			U64 filter = m_filter;
			filter = Rlwimi(filter, magFilter, 24, 4, 7);
			m_filter = (U32) Rlwimi(filter, minFilter, 16, 12, 15);
#endif
		}

		inline void TextureReduced::SetLevelRange(U32 minLevel, U32 maxLevel)
		{
#ifdef __SPU__
			m_control2 = (minLevel << 19) | (maxLevel << 7) | (m_control2 & 0x8000007F);
#else
			U64 control2 = m_control2;
			control2 = Rlwimi(control2, minLevel, 19, 1, 12);
			m_control2 = (U32) Rlwimi(control2, maxLevel, 7, 13, 24);
#endif
		}

		inline void TextureReduced::SetLevelBias(I32 bias)
		{
#ifdef __SPU__
			m_filter = bias | (m_filter & 0xFFFFE000);
#else
			m_filter = Rlwimi(m_filter, bias, 0, 19, 31);
#endif
		}

		inline void TextureReduced::SetMaxAnisotropy(AnisotropyLevel anisotropy)
		{
			U32 enable = (anisotropy != kAnisotropy1);
#ifdef __SPU__
			m_control1 = (enable << 5) | (m_control1 & 0xFFFFFFDF);
			m_control2 = (anisotropy << 4) | (m_control2 & 0xFFFFFF8F);
#else
			m_control1 = Rlwimi(m_control1, enable, 5, 26, 26);
			m_control2 = Rlwimi(m_control2, anisotropy, 4, 25, 27);
#endif
		}

		inline void TextureReduced::SetDepthFunc(ComparisonFunc func)
		{
#ifdef __SPU__
			m_control1 = (func << 28) | (m_control1 & 0x0FFFFFFF);
#else
			m_control1 = Rlwimi(m_control1, func, 28, 0, 3);
#endif
		}

		inline void TextureReduced::EnableSignedExpand()
		{
#ifdef __SPU__
			m_control1 |= 0x00001000;
#else
			m_control1 = Rlwimi(m_control1, 0x00001000, 0, 16, 19);
#endif
		}

		inline void TextureReduced::DisableSignedExpand()
		{
			m_control1 = m_control1 & 0xFFFF0FFF;
		}

		inline void TextureReduced::SetGammaCorrect(U32 mask)
		{
			m_control1 = (m_control1 & 0xFF0FFFFF) | mask;
		}

		inline void TextureReduced::SetColorKeyMode(ColorKeyMode mode)
		{
			m_control2 = (m_control2 & 0xFFFFFFFC) | mode;
		}

		inline void TextureReduced::SetKernelMode(KernelMode mode)
		{
			ICE_ASSERT((mode == kKernelQuincunx) 
				|| (mode == kKernelGaussian)
				|| (mode == kKernelQuincunxAlt));
			m_filter = (m_filter & 0xFFFF1FFF) | mode;
		}

#ifndef __SPU__
		inline void *TextureReduced::GetImageAddress() const
		{
			if((m_format & 0x3) == kTextureVideoMemory)
				return TranslateOffsetToAddress(m_baseOffset);
			else
				return TranslateIoOffsetToAddress(m_baseOffset);
		}
#endif

		inline U32 TextureReduced::GetFormat() const
		{
			U32 format = m_swizzle & 0x1FFFF;
#ifdef __SPU__
			format |= (m_format << 16) & 0x1F000000;
			format |= (m_filter << 24) & 0x00F00000;
#else
			format = Rlwimi(format, m_format, 16, 3, 7);
			format = Rlwimi(format, m_filter, 24, 8, 11);
#endif
			return format;
		}

		inline U32 TextureReduced::GetWidth() const
		{
			return m_size1 >> 16;
		}

		inline U32 TextureReduced::GetHeight() const
		{
			return m_size1 & 0xFFFF;
		}
		
		inline U32 TextureReduced::GetLinearImagePitch() const
		{
			return m_size2 & 0xFFFFF;
		}

		inline TextureContext TextureReduced::GetContext() const
		{
			return TextureContext(m_format & 0x3);
		}

		
		//! Initializes a texture reduced object.
		/*! \param texture  A pointer to an uninitialized texture object.
		*/
		static inline void InitTextureReduced(TextureReduced *texture)
		{
			texture->Init();
		}
		
		//! Specifies a 1D image for a texture reduced object.
		/*! \param texture     A pointer to a texture object.
		    \param offset      An offset to the texture image data in the specified context.
		    \param context     The context where the offset lies. (System or Video Memory)
		    \param levelCount  The number of mipmap levels contained in the texture image; must be 1-15.
		    \param width       The width of the 1D texture image.
		    \param format      The format of the texture image.
		    \param pitch       Row pitch in bytes for linear (nonswizzled) images; 0 (default) for swizzled images.
		*/
		static inline void SetTextureReducedImage1D(TextureReduced *texture, U32 offset, TextureContext context, U32 levelCount, U32 width, U32 format, U32 pitch = 0)
		{
			texture->SetImage1D(offset, context, levelCount, width, format, pitch);
		}
		
		//! Specifies a 2D image for a texture reduced object.
		/*! \param texture     A pointer to a texture object.
		    \param offset      An offset to the texture image data in the specified context.
		    \param context     The context where the offset lies. (System or Video Memory)
		    \param levelCount  The number of mipmap levels contained in the texture image; must be 1-15.
		    \param width       The width of the texture image.
		    \param height      The height of the texture image.
		    \param format      The format of the texture image.
		    \param pitch       Row pitch in bytes for linear (nonswizzled) images; 0 (default) for swizzled images.
		*/
		static inline void SetTextureReducedImage2D(TextureReduced *texture, U32 offset, TextureContext context, U32 levelCount, U32 width, U32 height, U32 format, U32 pitch = 0)
		{
			texture->SetImage2D(offset, context, levelCount, width, height, format, pitch);
		}
		
		//! Specifies a 3D image for a texture reduced object.
		/*! \param texture     A pointer to a texture object.
		    \param offset      An offset to the texture image data in the specified context.
		    \param context     The context where the offset lies. (System or Video Memory)
		    \param levelCount  The number of mipmap levels contained in the texture image; must be 1-15.
		    \param width       The width of the texture image.
		    \param height      The height of the texture image.
		    \param depth       The depth of the texture image.
		    \param format      The format of the texture image.
		    \param pitch       Row pitch in bytes for linear (nonswizzled) images; 0 (default) for swizzled images.
		*/
		static inline void SetTextureReducedImage3D(TextureReduced *texture, U32 offset, TextureContext context, U32 levelCount, U32 width, U32 height, U32 depth, U32 format, U32 pitch = 0)
		{
			texture->SetImage3D(offset, context, levelCount, width, height, depth, format, pitch);
		}
		
		//! Specifies a cube map image for a texture reduced object.
		/*! \param texture    A pointer to a texture object.
		    \param offset     An offset to the texture image data in the specified context.
		    \param context    The context where the offset lies. (System or Video Memory)
		    \param levelCount The number of mipmap levels contained in the texture image; must be 1-15.
		    \param size       The width and height of the texture image (which must be square).
		    \param format     The format of the texture image.
		    \param pitch      Row pitch in bytes for linear (nonswizzled) images; 0 (default) for swizzled images.
		*/
		static inline void SetTextureReducedImageCube(TextureReduced *texture, U32 offset, TextureContext context, U32 levelCount, U32 size, U32 format, U32 pitch = 0)
		{
			texture->SetImageCube(offset, context, levelCount, size, format, pitch);
		}
		
		//! Specifies a rect image for a texture reduced object.
		/*! This differs from 2D by using unnormalized coordinates, and in that no mipmaps can be used. 
		    \param texture  A pointer to a texture object.
		    \param offset   An offset to the texture image data in the specified context.
		    \param context  The context where the offset lies. (System or Video Memory)
		    \param width    The width of the texture image.
		    \param height   The height of the texture image.
		    \param format   The format of the texture image.
		    \param pitch    Row pitch in bytes for linear (nonswizzled) images; 0 (default) for swizzled images.
		*/
		static inline void SetTextureReducedImageRect(TextureReduced *texture, U32 offset, TextureContext context, U32 width, U32 height, U32 format, U32 pitch = 0)
		{
			texture->SetImageRect(offset, context, width, height, format, pitch);
		}
		
		//! Sets the wrap mode for a texture reduced object.
		/*! \param texture  A pointer to a texture object.
		    \param coord    The coordinate for which to specify a wrap mode; must be kTexcoordS, kTexcoordT, or kTexcoordR.
		    \param mode     The wrap mode.
		*/
		static inline void SetTextureReducedWrapMode(TextureReduced *texture, TexcoordComponent coord, WrapMode mode)
		{
			texture->SetWrapMode(coord, mode);
		}
		
		//! Sets the filtering modes for a texture reduced object.
		/** \param texture    A pointer to a texture object.
		    \param magFilter  The magnification filtering mode.
		    \param minFilter  The minification filtering mode.
		*/
		static inline void SetTextureReducedFilterMode(TextureReduced *texture, FilterMode magFilter, FilterMode minFilter)
		{
			texture->SetFilterMode(magFilter, minFilter);
		}
		
		//! Sets the mipmap level range for a texture reduced object.
		/*! \param texture   A pointer to a texture object.
		    \param minLevel  The minimum level as an unsigned 4.8-bit fixed-point number (e.g., 0x0100 is 1.0, 0x0280 is 2.5).
		    \param maxLevel  The maximum level as an unsigned 4.8-bit fixed-point number.
		*/
		static inline void SetTextureReducedLevelRange(TextureReduced *texture, U32 minLevel, U32 maxLevel)
		{
			texture->SetLevelRange(minLevel, maxLevel);
		}
		
		//! Sets the mipmap level bias for a texture reduced object.
		/** \param texture  A pointer to a texture object.
		    \param bias     The bias as a signed 5.8-bit fixed-point number (e.g., 0x0100 is +1.0, 0x1F00 is -1.0).
		*/
		static inline void SetTextureReducedLevelBias(TextureReduced *texture, I32 bias)
		{
			texture->SetLevelBias(bias);
		}
		
		//! Sets the maximum anisotropic sample count for a texture reduced object.
		/*! \param texture     A pointer to a texture object.
		    \param anisotropy  The anisotropic filtering mode; must be one of the defined constants.
		*/
		static inline void SetTextureReducedMaxAnisotropy(TextureReduced *texture, AnisotropyLevel anisotropy)
		{
			texture->SetMaxAnisotropy(anisotropy);
		}
		
		//! Sets the depth comparison function for a texture reduced object.
		/*! \param texture  A pointer to a texture object.
		    \param func     The comparison function used by textures having a depth format.
		*/
		static inline void SetTextureReducedDepthFunc(TextureReduced *texture, ComparisonFunc func)
		{
			texture->SetDepthFunc(func);
		}
		
		//! Enables 'signed expansion' for a texture reduced object.
		/*! This feature causes all unsigned channels to be 'expanded' to signed range: X'=2X-1
		    NOTE - Never enable 'signed expand' for textures with floating-point formats!
		    \param texture  A pointer to a texture object.
		*/
		static inline void EnableTextureReducedSignedExpand(TextureReduced *texture)
		{
			texture->EnableSignedExpand();
		}
		
		//! Disables 'signed expansion' for a texture reduced object.
		/*! This feature causes all unsigned channels to be 'expanded' to signed range: X'=2X-1
		    NOTE - Never enable 'signed expand' for textures with floating-point formats!
		    \param texture  A pointer to a texture object.
		*/
		static inline void DisableTextureReducedSignedExpand(TextureReduced *texture)
		{
			texture->DisableSignedExpand();
		}

		//! Enables or Disables gamma correction (sRGB) based on the supplied mask.
		/*! NOTE - This only works on 8-bit per-component textures.
		    NOTE - This is separate functionality from the fragment program unpack_4ubytegamma cg intrinsic. 
		    \param texture  A pointer to a texture object.
			\param mask     A mask constructed with kGammaCorrect* constants.
		*/
		static inline void SetTextureReducedGammaCorrect(TextureReduced *texture, U32 mask)
		{
			texture->SetGammaCorrect(mask);
		}
		
		//! Sets the 'color key' mode for a texture object.
		/*! This feature causes the alpha of the texture to be set to zero when 
		    the color of the texture is equal to color key color. Optionally, 
			instead of setting the alpha to zero, the fragment can be discarded.
			NOTE - This feature does not work with DXT or FP textures.
			NOTE - Color Keying occurs pre-filtering.
			\param texture  A pointer to a texture object.
			\param mode     The color key mode to use. 
		*/
		static inline void SetTextureReducedColorKeyMode(TextureReduced *texture, ColorKeyMode mode)
		{
			texture->SetColorKeyMode(mode);
		}

		//! Sets the convolution kernel when the magnification mode for a texture reduced object is set to convolution.
		/*! \param texture  A pointer to a texture object.
		    \param mode     The convolution kernel mode to use
		*/
		static inline void SetTextureReducedKernelMode(TextureReduced *texture, KernelMode mode)
		{
			texture->SetKernelMode(mode);
		}
	}
}


#endif // ICE_TEXTUREREDUCED_H
