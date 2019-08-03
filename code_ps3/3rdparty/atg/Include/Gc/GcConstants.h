//--------------------------------------------------------------------------------------------------
/**
	@file		GcConstants.h

	@brief		Common Graphics Core Constants		

	@note		All constants are defined in alphabetical order. This eases navigation within the file.
	
	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_CONSTANTS_H
#define GC_CONSTANTS_H

#include <Gc/Gc.h>

//--------------------------------------------------------------------------------------------------
//	FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------

class GcInitParams;

//--------------------------------------------------------------------------------------------------
//	NAMESPACE DEFINITION
//--------------------------------------------------------------------------------------------------

namespace Gc
{
	/// Resource data start alignment constants.

	enum Alignment
	{
		kTiledRenderBufferAlignment		= 64 * 1024,	// 64K
		kRenderBufferAlignment			= 128,			// 128 Bytes
		
		kShaderAlignment				= 64,			// 64 Bytes
		kStreamBufferAlignment			= 128,			// 128 Bytes
		kTextureAlignment				= 128,			// 128 Bytes
	};

	/// Anisotropy level.

	enum AnisotropyLevel
	{
		kAnisotropy1					= Ice::Render::kAnisotropy1,
		kAnisotropy2					= Ice::Render::kAnisotropy2,
		kAnisotropy4					= Ice::Render::kAnisotropy4,
		kAnisotropy6					= Ice::Render::kAnisotropy6,
		kAnisotropy8					= Ice::Render::kAnisotropy8,
		kAnisotropy10					= Ice::Render::kAnisotropy10,
		kAnisotropy12					= Ice::Render::kAnisotropy12,
		kAnisotropy16					= Ice::Render::kAnisotropy16,
	};

	// Aspect ratio.

	enum AspectRatio
	{
		kAspectStandard,
		kAspectWidescreen,
	};

	/// Blending equations.
	
	enum BlendEquation
	{
		kBlendEquationAdd				= Ice::Render::kBlendEquationAdd,				///< C = (Cs x S) + (Cd x D)
		kBlendEquationMin				= Ice::Render::kBlendEquationMin,				///< C = min(Cs, Cd)
		kBlendEquationMax				= Ice::Render::kBlendEquationMax,				///< C = max(Cs, Cd)
		kBlendEquationSub				= Ice::Render::kBlendEquationSubtract,			///< C = (Cs x S) - (Cd x D)
		kBlendEquationRevSub			= Ice::Render::kBlendEquationReverseSubtract,	///< C = (Cd x D) - (Cs x S)
	};
	
	
	/// Blending functions.
	
	enum BlendFunc
	{
		kBlendZero						= Ice::Render::kBlendZero,					///< (0, 0, 0, 0)
		kBlendOne						= Ice::Render::kBlendOne,					///< (1, 1, 1, 1)
		kBlendSrcColour					= Ice::Render::kBlendSrcColor,				///< (Rs, Gs, Bs, As)
		kBlendOneMinusSrcColour			= Ice::Render::kBlendOneMinusSrcColor,		///< (1 - Rs, 1 - Gs, 1 - Bs, 1 - As) 
		kBlendSrcAlpha					= Ice::Render::kBlendSrcAlpha,				///< (As, As, As, As)
		kBlendOneMinusSrcAlpha			= Ice::Render::kBlendOneMinusSrcAlpha,		///< (1 - As, 1 - As, 1 - As, 1 - As) 
		kBlendDstAlpha					= Ice::Render::kBlendDstAlpha,				///< (Ad, Ad, Ad, Ad)
		kBlendOneMinusDstAlpha			= Ice::Render::kBlendOneMinusDstAlpha,		///< (1 - Ad, 1 - Ad, 1 - Ad, 1 - Ad)
		kBlendDstColour					= Ice::Render::kBlendDstColor,				///< (Rd, Gd, Bd, Ad)
		kBlendOneMinusDstColour			= Ice::Render::kBlendOneMinusDstColor,		///< (1 - Rd, 1 - Gd, 1 - Bd, 1 - Ad)
		kBlendSrcAlphaSaturate			= Ice::Render::kBlendSrcAlphaSaturate,		///< (f, f, f, 1) where f = min(As, 1 - Ad)
		kBlendConstColour				= Ice::Render::kBlendConstantColor,			///< (Rc, Gc, Bc, Ac)
		kBlendOneMinusConstColour		= Ice::Render::kBlendOneMinusConstantColor,	///< (1 - Rc, 1 - Gc, 1 - Bc, 1 - Ac)
		kBlendConstAlpha				= Ice::Render::kBlendConstantAlpha,			///< (Ac, Ac, Ac, Ac)
		kBlendOneMinusConstAlpha		= Ice::Render::kBlendOneMinusConstantAlpha,	///< (1 - Ac, 1 - Ac, 1 - Ac, 1 - Ac)
	};
	
	
	// Buffer types.

	enum BufferFormat
	{
		kBufferFormatRGB565				= Ice::Render::kBufferRgb565,
		kBufferFormatARGB8				= Ice::Render::kBufferArgb8888,	
		kBufferFormatB8					= Ice::Render::kBufferB8, 
		kBufferFormatGB8				= Ice::Render::kBufferGb88, 
		kBufferFormatRGBA16F			= Ice::Render::kBufferRgba16f,	
		kBufferFormatRGBA32F			= Ice::Render::kBufferRgba32f, 
		kBufferFormatR32F				= Ice::Render::kBufferR32f, 	
		kBufferFormatD24S8				= Ice::Render::kBufferD24S8, 
		kBufferFormatD16S0				= Ice::Render::kBufferD16S0, 
	};


	// Buffer types.

	enum BufferType
	{
		kStaticBuffer,
		kScratchBuffer, 
		kUserBuffer,
	};


	/// Clear flag bits.
	
	enum ClearFlags
	{
		kColourBufferRedBit				= Ice::Render::kClearRed,
		kColourBufferGreenBit			= Ice::Render::kClearGreen,
		kColourBufferBlueBit			= Ice::Render::kClearBlue,
		kColourBufferAlphaBit			= Ice::Render::kClearAlpha,
		kColourBufferBit				= Ice::Render::kClearColor,
		kDepthBufferBit					= Ice::Render::kClearDepth,
		kStencilBufferBit				= Ice::Render::kClearStencil,

		kAllBufferBits					= kColourBufferBit | kDepthBufferBit | kStencilBufferBit, 
	};

	
	/// Comparison functions. Used for alpha, depth, stencil and depth texture tests.
	
	enum CmpFunc
	{
		kNever							= Ice::Render::kFuncNever,
		kLess							= Ice::Render::kFuncLess,
		kEqual							= Ice::Render::kFuncEqual,
		kLEqual							= Ice::Render::kFuncLessEqual,
		kGreater						= Ice::Render::kFuncGreater,
		kNotEqual						= Ice::Render::kFuncNotEqual,
		kGEqual							= Ice::Render::kFuncGreaterEqual,
		kAlways							= Ice::Render::kFuncAlways,
	};


	/// Rasterisatiom face culling mode.
	
	enum CullFaceMode
	{
		kCullFront						= Ice::Render::kCullFaceFront,
		kCullBack						= Ice::Render::kCullFaceBack, 
	};


	/// Display mode.
#ifndef GC_TOOLS	
	enum DisplayMode
	{
		kDisplay480						= Ice::Render::kDisplay480,				///< Resolution 720x480
		kDisplay576						= Ice::Render::kDisplay576,				///< Resolution 720x576
		kDisplay720						= Ice::Render::kDisplay720,				///< Resolution 1280x720
		kDisplay1080					= Ice::Render::kDisplay1080,			///< Resolution 1920x1080
		kDisplayAuto					= Ice::Render::kDisplayAutoTiled,		///< Resolution defined by OSD

#ifdef ATG_PC_PLATFORM
		kDisplayCustom
#endif
	};	
#endif // ndef ICEPC

	/// Display swap mode.
	
	enum DisplaySwapMode
	{
		kDisplaySwapModeHSync			= Ice::Render::kSwapModeHSync,			///< Swap at next H-Sync.
		kDisplaySwapModeVSync			= Ice::Render::kSwapModeVSync			///< Swap at next V-Sync.
	};

	
	/// Fog modes
	enum FogMode
	{
		kFogLinear						= Ice::Render::kFogLinear,
		kFogExp							= Ice::Render::kFogExp,
		kFogExp2						= Ice::Render::kFogExp2
	};
		
	/// Face orientation type.
	
	enum FaceType
	{
		kCwFace							= Ice::Render::kFrontFaceCw,
		kCcwFace						= Ice::Render::kFrontFaceCcw,
	};
		

	/// Texture gamma correction flags

	enum GammaCorrectionFlags
	{
		kGammaCorrectR					= Ice::Render::kGammaCorrectR,
		kGammaCorrectG					= Ice::Render::kGammaCorrectG,
		kGammaCorrectB					= Ice::Render::kGammaCorrectB,
		kGammaCorrectA					= Ice::Render::kGammaCorrectA,
		kGammaCorrectSrgb				= Ice::Render::kGammaCorrectSRgb, // this just combines R, G and B
	};

	
#ifndef GC_TOOLS

	/// Texture convolution kernels

	enum KernelMode
	{
		// 5-tap kernel: center tap weight 4, other taps weight 1.
		// Sample Pattern:
		//       1 1
		//        4 
		//       1 1
		kKernelQuincunx					= Ice::Render::kKernelQuincunx,
		// 9-tap kernel: ordered grid of even weighting.
		// Sample Pattern:
		//       1 1 1
		//       1 1 1
		//       1 1 1
		kKernelGaussian					= Ice::Render::kKernelGaussian,
		// 5-tap kernel: center tap weight 4, other taps weight 1.
		kKernelQuincunxAlt				= Ice::Render::kKernelQuincunxAlt,
	};
	
#endif // GC_TOOLS
	
	/// Logic operation

	enum LogicOp
	{
		kOpClear						= Ice::Render::kLogicClear,
		kOpAnd							= Ice::Render::kLogicAnd,
		kOpAndReverse					= Ice::Render::kLogicAndReverse,
		kOpCopy							= Ice::Render::kLogicCopy,
		kOpAndInverted					= Ice::Render::kLogicAndInverted,
		kOpNoOp							= Ice::Render::kLogicNop,
		kOpXor							= Ice::Render::kLogicXor,
		kOpOr							= Ice::Render::kLogicOr,
		kOpNor							= Ice::Render::kLogicNor,
		kOpEquiv						= Ice::Render::kLogicEquiv,
		kOpInvert						= Ice::Render::kLogicInvert,
		kOpOrReverse					= Ice::Render::kLogicOrReverse,
		kOpCopyInverted					= Ice::Render::kLogicCopyInverted,
		kOpOrInverted					= Ice::Render::kLogicOrInverted,
		kOpNand							= Ice::Render::kLogicNand,
		kOpSet							= Ice::Render::kLogicSet,
	};

	
	/// Maximum number of simultaneous colour render targets.
	
	static const uint kMaxRenderTargets	= 4;
	
	
	/// Maximum number of vertex attributes.
	
	static const uint kMaxAttributes	= 16;
	
	
	/// Maximum number of texture samplers.
	
	static const uint kMaxSamplers		= 16;
	
	
	/// Maximum number of vertex program texture samplers.
	
	static const uint kMaxVertexProgramSamplers	= 4;
	
	
	/// Memory Context
	
	enum MemoryContext
	{
		kVideoMemory,		///< GPU Local Memory
		kHostMemory			///< CELL XDR Memory
	};
	
	
	/// Multi-sampling modes

	enum MultisampleMode
	{
		kMultisampleNone				= Ice::Render::kMultisampleNone,		///< No multisampling
		kMultisample2x					= Ice::Render::kMultisample2X,			///< 2x diagonal pattern (2*width)
		kMultisample4xOrdered			= Ice::Render::kMultisample4XOrdered,	///< 4x ordered grid pattern (2*width, 2*height)
		kMultisample4xRotated			= Ice::Render::kMultisample4XRotated,	///< 4x rotated grid pattern (2*width, 2*height)
	};


	/// polygon modes

	enum PolygonMode
	{
		kPolygonModePoint				= Ice::Render::kPolygonModePoint,
		kPolygonModeLine				= Ice::Render::kPolygonModeLine,
		kPolygonModeFill				= Ice::Render::kPolygonModeFill,
	};


	/// Primitive types
	
	enum PrimitiveType
	{
		kPoints							= Ice::Render::kDrawPoints,
		kLines							= Ice::Render::kDrawLines,
		kLineLoop						= Ice::Render::kDrawLineLoop,
		kLineStrip						= Ice::Render::kDrawLineStrip,
		kTriangles						= Ice::Render::kDrawTriangles,
		kTriangleStrip					= Ice::Render::kDrawTriangleStrip,
		kTriangleFan					= Ice::Render::kDrawTriangleFan,
		kQuads							= Ice::Render::kDrawQuads,
		kQuadStrip						= Ice::Render::kDrawQuadStrip,	
		kPolygon						= Ice::Render::kDrawPolygon,
	};


	// Shader program types

	enum ProgramType
	{
		kVertexProgram, 
		kFragmentProgram, 
	};


	/// Render states.
	
	enum RenderState 
	{ 
		kBlend							= Ice::Render::kRenderBlend, 
		kDither							= Ice::Render::kRenderDither, 
		kAlphaTest						= Ice::Render::kRenderAlphaTest, 
		kStencilTestFront				= Ice::Render::kRenderStencilTestFront, 
		kStencilTestBack				= Ice::Render::kRenderStencilTestBack, 
		kColourLogicOp					= Ice::Render::kRenderLogicOp, 
		kDepthBounds					= Ice::Render::kRenderDepthBoundsTest, 
		kPolygonOffsetFill				= Ice::Render::kRenderPolygonOffsetFill, 
		kDepthTest						= Ice::Render::kRenderDepthTest, 
		kBackfacingColour				= Ice::Render::kRenderBackfacingColor,
		kCullFace						= Ice::Render::kRenderCullFace, 
		kPrimitiveRestart				= Ice::Render::kRenderPrimitiveRestart, 
		kVertexProgramPointSize			= Ice::Render::kRenderVertexProgramPointSize,
#ifndef GC_TOOLS	
		kGammaCorrectedWrites			= Ice::Render::kRenderGammaCorrectedWrites,
#endif
	};
	

	/// Shading model
	
	enum ShadeModel
	{
		kFlat							= Ice::Render::kShadeModelFlat,
		kSmooth							= Ice::Render::kShadeModelSmooth,
	};


	// Sprite texture R coordinate replacement mode
	
	enum SpriteMode
	{
		kSpriteZero						= Ice::Render::kSpriteZero,
		kSpriteR						= Ice::Render::kSpriteR, 
		kSpriteS						= Ice::Render::kSpriteS, 
	};


	// Stencil test operation.
	
	enum StencilOp
	{
		kStencilZero					= Ice::Render::kStencilZero,
		kStencilKeep					= Ice::Render::kStencilKeep,
		kStencilReplace					= Ice::Render::kStencilReplace,
		kStencilIncr					= Ice::Render::kStencilIncrement,
		kStencilDecr					= Ice::Render::kStencilDecrement,
		kStencilInvert					= Ice::Render::kStencilInvert,
		kStencilIncrWrap				= Ice::Render::kStencilIncrementWrap,
		kStencilDecrWrap				= Ice::Render::kStencilDecrementWrap,
	};
	
	
	// Stream index types.

	enum StreamIndexType
	{
		kIndex16						= Ice::Render::kIndex16,
		kIndex32						= Ice::Render::kIndex32, 
	};


	/// Stream Type
	
	enum StreamType
	{
		kFloat,
		kHalf,
		kShort,
		kUByte,
		kPacked,						// Signed 11-11-10 format
		kByte,
		kInt,
		kUShort,
		kUInt,
		
		kNumberOfStreamTypes,
	};


	/// Cubemap faces.

	enum TexCubeFace
	{
		kFacePositiveX,
		kFaceNegativeX,
		kFacePositiveY,
		kFaceNegativeY,
		kFacePositiveZ,
		kFaceNegativeZ,
	};


	/// Texture filter types.

	enum TexFilter
	{
		kFilterNearest					= Ice::Render::kFilterNearest,
		kFilterLinear					= Ice::Render::kFilterLinear,
		kFilterNearestMipMapNearest		= Ice::Render::kFilterNearestMipmapNearest,	///< Nearest min filter, nearest mip filter 
		kFilterLinearMipMapNearest		= Ice::Render::kFilterLinearMipmapNearest, 	///< Linear min filter, nearest mip filter
		kFilterNearestMipMapLinear		= Ice::Render::kFilterNearestMipmapLinear, 
		kFilterLinearMipMapLinear		= Ice::Render::kFilterLinearMipmapLinear,
#ifndef GC_TOOLS
		kFilterConvolutionKernelMin		= Ice::Render::kFilterConvolutionKernelMin,
		kFilterConvolutionKernelMag		= Ice::Render::kFilterConvolutionKernelMag,
#endif
	};

	
	/// Texture surface formats.

	enum TexFormat
	{
		kTexFormatL8                    = Ice::Render::kTextureLuminance8,
        kTexFormatL16                   = Ice::Render::kTextureLuminance16,
        kTexFormatA8                    = Ice::Render::kTextureAlpha8,
        kTexFormatA16                   = Ice::Render::kTextureAlpha16,
        kTexFormatL8A8                  = Ice::Render::kTextureLuminanceAlpha88, 
        kTexFormatL16A16                = Ice::Render::kTextureLuminanceAlpha16, 
		kTexFormatRGB565				= Ice::Render::kTextureRgb565,
		kTexFormatARGB4444				= Ice::Render::kTextureArgb4444,
		kTexFormatARGB1555				= Ice::Render::kTextureArgb1555,
        kTexFormatARGB8                 = Ice::Render::kTextureArgb8888, 
        kTexFormatBGRA8                 = Ice::Render::kTextureBgra8888, 
        kTexFormatRGBA8                 = Ice::Render::kTextureRgba8888, 
        kTexFormatDXT1                  = Ice::Render::kTextureDxt1, 
        kTexFormatDXT3                  = Ice::Render::kTextureDxt3, 
        kTexFormatDXT5                  = Ice::Render::kTextureDxt5, 
        kTexFormatD24X8                 = Ice::Render::kTextureDepth24X8, 
		kTexFormatD16					= Ice::Render::kTextureDepth16, 
        kTexFormatR32F                  = Ice::Render::kTextureR32f, 
        kTexFormatRGBA16F               = Ice::Render::kTextureRgba16f, 
        kTexFormatRGBA32F               = Ice::Render::kTextureRgba32f, 
		kTexFormatGR16F					= Ice::Render::kTextureGr16f,
	};


	/// Texture address mode types.

	enum TexWrapMode
	{
		kWrapModeRepeat					= Ice::Render::kWrapRepeat, 
		kWrapModeMirroredRepeat			= Ice::Render::kWrapMirrorRepeat, 
		kWrapModeClampToEdge			= Ice::Render::kWrapClampToEdge, 
		kWrapModeClampToBorder			= Ice::Render::kWrapClampToBorder, 
		kWrapModeClamp					= Ice::Render::kWrapClamp, 
		kWrapModeMirroredClampToEdge	= Ice::Render::kWrapMirrorClampToEdge, 
		kWrapModeMirroredClampToBorder	= Ice::Render::kWrapMirrorClampToBorder, 
		kWrapModeMirroredClamp			= Ice::Render::kWrapMirrorClamp,
	};
	
	/// Texture type.

	enum TexType
	{
		kTexture2D,
		kTexture3D,
		kTextureCube,
	};

#ifdef ATG_PC_PLATFORM
	void GetDisplayModeDimensions( const GcInitParams& initParams, u32& dispWidth, u32& dispHeight );
#endif


	// Safe memory aliasing helpers - so Gc adheres to the strict-aliasing rule.
	
	static inline uint FloatToUInt(float f)
	{
		union { uint m_uint; float m_float; } alias;
		alias.m_float = f;
		return alias.m_uint;
	}

	static inline int FloatToInt(float f)
	{
		union { int m_int; float m_float;} alias;
		alias.m_float = f;
		return alias.m_int;
	}

	
	// Texture utility functions.

	uint GetBitsPerPixel( TexFormat format );
	uint GetBitsPerPixel( BufferFormat format );

	uint GetBytesPerPixel( TexFormat format );
	uint GetBytesPerPixel( BufferFormat format );

	TexFormat ConvertToTextureFormat( BufferFormat format );
	BufferFormat ConvertToBufferFormat( TexFormat format, bool& success );

	void SwizzleTextureData( void* pDest, const void* pSrc, uint width, uint height, uint depth, Gc::TexFormat format );

} // namespace Gc

#endif // GC_CONSTANTS_H
