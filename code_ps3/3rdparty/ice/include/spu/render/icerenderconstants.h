/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_RENDERCONSTANTS_H
#define ICE_RENDERCONSTANTS_H

namespace Ice
{
	namespace Render
	{
		//! A 16-bit (half precision) floating point type.
		typedef U16 half;
		//! A color where hex value 0x01020304 represents 1 for alpha, 2 for red, 3 for green and 4 for blue
		typedef U32 ArgbColor; 
		//! A color where hex value 0x01020304 represents 4 for alpha, 3 for red, 2 for green and 1 for blue
		typedef U32 BgraColor;
		
		//! The Maximum number of semaphores that can be used.
		U64 static const kMaxSemaphores = 256;
		//! The number of semaphores that are reserved by the os.
		U64 static const kNumReservedOsSemaphores = 64;
		//! The number of semaphores that are reserved by IceRender.
		U64 static const kNumReservedIceRenderSemaphores = 8;
		//! The number of semaphores that are reserved by the os & icerender.
		U64 static const kNumReservedSemaphores = kNumReservedOsSemaphores + kNumReservedIceRenderSemaphores;
		//! The default depth cull feedback value. Passed to SetDepthCullFeedback().
		U32 static const kDefaultDepthCullFeedback = 0x01000100;
		//! The default texture control 3 value. Passed to SetTextureReducedControl3().
		U32 static const kDefaultTextureControl3 = 0x00002DC4;
	
		#define CMD(a, b) ((a) | ((b) << 16))

		enum
		{
			//! Setting this bit on a command will tell the GPU command parser 
			//! to not increment the register number and repeate the current 
			//! command when processing the next U32 of data.
			kCmdNoIncFlag  = 0x40000000,	
			//! Setting this bit causes the command to be interpreted as a jump 
			//! destination address.
			kCmdJumpFlag   = 0x20000000,    
			//! Setting this bit causes the command to be interpreted as a call 
			//! destination address. 
			/*! Stack depth is 1. */
			kCmdCallFlag   = 0x00000002, 	
			//! Setting this bit causes the command to be interpreted as a 
			//! return to the command following the last call.
			/*! Stack depth is 1. */
			kCmdReturnFlag = 0x00020000	    
		};

		// GPU register addresses
		enum
		{
			kCmdSet3DSubchannel                 = 0x0000,
			kCmdReference						= 0x0050,
			kCmdSemaphoreOffset					= 0x0064,
			kCmdWaitOnSemaphore					= 0x0068,
			kCmdSignalSemaphore					= 0x006C,
			kCmdNoOperation						= 0x0100,
			kCmdWaitForIdle                     = 0x0110,
			kCmdWritePerformanceReport          = 0x0140,
			kCmdSetNotifierContext              = 0x0180,
			kCmdSetTextureContext0				= 0x0184,
			kCmdSetTextureContext1				= 0x0188,
			kCmdColorBufferContext1				= 0x018C,
			kCmdStateContext                    = 0x0190,
			kCmdColorBufferContext0				= 0x0194,
			kCmdDepthBufferContext              = 0x0198,
			kCmdVertexContext0                  = 0x019C,
			kCmdVertexContext1                  = 0x01A0,
			kCmdSemaphoreContext                = 0x01A4,
			kCmdReportContext                   = 0x01A8,
			kCmdColorBufferContext2				= 0x01B4,
			kCmdColorBufferContext3				= 0x01B8,
			kCmdRenderTargetLeftWidth			= 0x0200,
			kCmdRenderTargetTopHeight			= 0x0204,
			kCmdRenderTargetFormat				= 0x0208,
			kCmdColorBufferPitch0				= 0x020C,
			kCmdColorBufferBaseAddress0			= 0x0210,
			kCmdDepthBufferBaseAddress			= 0x0214,
			kCmdColorBufferBaseAddress1			= 0x0218,
			kCmdColorBufferPitch1				= 0x021C,
			kCmdDrawBufferMask					= 0x0220,
			kCmdDepthBufferPitch				= 0x022C,
			kCmdInvalidateDepthCull				= 0x0234,
			kCmdCylindricalWrapEnable0          = 0x0238,
			kCmdCylindricalWrapEnable1          = 0x023C,
			kCmdColorBufferPitch2				= 0x0280,
			kCmdColorBufferPitch3				= 0x0284,
			kCmdColorBufferBaseAddress2			= 0x0288,
			kCmdColorBufferBaseAddress3			= 0x028C,
			kCmdRenderTargetOrigin				= 0x02B8,
			kCmdWindowClipControl				= 0x02BC,
			kCmdWindowClipLeftRight				= 0x02C0,
			kCmdWindowClipTopBottom				= 0x02C4,
			kCmdDitherEnable					= 0x0300,
			kCmdAlphaTestEnable					= 0x0304,
			kCmdAlphaFunction					= 0x0308,
			kCmdAlphaRefValue					= 0x030C,
			kCmdBlendEnable						= 0x0310,
			kCmdBlendSrcFactor					= 0x0314,
			kCmdBlendDstFactor					= 0x0318,
			kCmdBlendColor						= 0x031C,
			kCmdBlendEquation					= 0x0320,
			kCmdColorMask						= 0x0324,
			kCmdFrontStencilTestEnable			= 0x0328,
			kCmdFrontStencilMask				= 0x032C,
			kCmdFrontStencilFunction			= 0x0330,
			kCmdFrontStencilRefValue			= 0x0334,
			kCmdFrontStencilCompMask			= 0x0338,
			kCmdFrontStencilFailOp				= 0x033C,
			kCmdFrontStencilDepthFailOp			= 0x0340,
			kCmdFrontStencilDepthPassOp			= 0x0344,
			kCmdBackStencilTestEnable			= 0x0348,
			kCmdBackStencilMask					= 0x034C,
			kCmdBackStencilFunction				= 0x0350,
			kCmdBackStencilRefValue				= 0x0354,
			kCmdBackStencilCompMask				= 0x0358,
			kCmdBackStencilFailOp				= 0x035C,
			kCmdBackStencilDepthFailOp			= 0x0360,
			kCmdBackStencilDepthPassOp			= 0x0364,
			kCmdShadeModel						= 0x0368,
			kCmdMRTBlendEnable					= 0x036C,
			kCmdMRTColorMask					= 0x0370,
			kCmdLogicOpEnable					= 0x0374,
			kCmdLogicOp							= 0x0378,
			kCmdFloatingPointBlendColor			= 0x037C,
			kCmdDepthBoundsTestEnable			= 0x0380,
			kCmdDepthBoundsMin					= 0x0384,
			kCmdDepthBoundsMax					= 0x0388,
			kCmdDepthRangeMin					= 0x0394,
			kCmdDepthRangeMax					= 0x0398,
			kCmdColorRemapControl				= 0x03B0,
			kCmdLineWidth						= 0x03B8,
			kCmdLineSmoothEnable				= 0x03BC,
			kCmdScissorX						= 0x08C0,
			kCmdScissorY						= 0x08C4,
			kCmdFogMode							= 0x08CC,
			kCmdFogBias							= 0x08D0,
			kCmdFogScale						= 0x08D4,
			kCmdFragmentProgramAddress			= 0x08E4,
			kCmdVertexProgramTextureAddress		= 0x0900,
			kCmdVertexProgramTextureFormat		= 0x0904,
			kCmdVertexProgramTextureControl1	= 0x0908,
			kCmdVertexProgramTextureControl2	= 0x090C,
			kCmdVertexProgramTextureSwizzle		= 0x0910,
			kCmdVertexProgramTextureFilter		= 0x0914,
			kCmdVertexProgramTextureSize		= 0x0918,
			kCmdVertexProgramTextureBorderColor	= 0x091C,
			kCmdViewportX						= 0x0A00,
			kCmdViewportY						= 0x0A04,
			kCmdViewportTranslateX				= 0x0A20,
			kCmdViewportTranslate				= kCmdViewportTranslateX,
			kCmdViewportTranslateY				= 0x0A24,
			kCmdViewportTranslateZ				= 0x0A28,
			kCmdViewportTranslateW				= 0x0A2C,
			kCmdViewportScaleX					= 0x0A30,
			kCmdViewportScaleY					= 0x0A34,
			kCmdViewportScaleZ					= 0x0A38,
			kCmdViewportScaleW					= 0x0A3C,
			kCmdPolygonOffsetPointEnable		= 0x0A60,
			kCmdPolygonOffsetLineEnable			= 0x0A64,
			kCmdPolygonOffsetPolygonEnable		= 0x0A68,
			kCmdDepthFunction					= 0x0A6C,
			kCmdDepthMask						= 0x0A70,
			kCmdDepthTestEnable					= 0x0A74,
			kCmdPolygonOffsetFactor				= 0x0A78,
			kCmdPolygonOffsetUnits				= 0x0A7C,
			kCmdVertexAttrib4Ns					= 0x0A80,
			kCmdTextureControl3					= 0x0B00,
			kCmdTexcoordControl					= 0x0B40,
			kCmdVertexProgramInstruction		= 0x0B80,
			kCmdSecondaryColorEnable			= 0x1428,
			kCmdVertexProgramTwoSideEnable		= 0x142C,
			kCmdCullingControl					= 0x1438,
			kCmdPerformanceControl              = 0x1450,
			kCmdClipPlaneControl				= 0x1478,
			kCmdPolygonStippleEnable			= 0x147C,
			kCmdPolygonStipplePattern			= 0x1480,
			kCmdVertexAttrib3f					= 0x1500,
			kCmdVertexAttribAddress				= 0x1680,
			kCmdInvalidatePreTransformCache		= 0x1710,
			kCmdInvalidatePostTransformCache	= 0x1714,
			kCmdPipeNop							= 0x1718,
			kCmdVertexAttribAddressOffset       = 0x1738,
			kCmdVertexAttribBaseIndex           = 0x173C,
			kCmdVertexAttribFormat				= 0x1740,
			kCmdResetReport     				= 0x17C8,
			kCmdPixelCountMode   				= 0x17CC,
			kCmdWriteReport						= 0x1800,
			kCmdDepthCullStatsEnable			= 0x1804,
			kCmdDrawMode						= 0x1808,
			kCmdDraw16bitElementsInline         = 0x180C,
			kCmdDraw32bitElementsInline         = 0x1810,
			kCmdDrawArrays						= 0x1814,
			kCmdDrawArraysInline				= 0x1818,
			kCmdIndexBufferAddress				= 0x181C,
			kCmdIndexBufferFormat				= 0x1820,
			kCmdDrawElements					= 0x1824,
			kCmdFrontPolygonMode				= 0x1828,
			kCmdBackPolygonMode					= 0x182C,
			kCmdCullFace						= 0x1830,
			kCmdFrontFace						= 0x1834,
			kCmdPolygonSmoothEnable				= 0x1838,
			kCmdCullFaceEnable					= 0x183C,
			kCmdTextureSize2					= 0x1840,
			kCmdVertexAttrib2f					= 0x1880,
			kCmdVertexAttrib2s					= 0x1900,
			kCmdVertexAttrib4Nub				= 0x1940,
			kCmdVertexAttrib4s					= 0x1980,
			kCmdTextureAddress					= 0x1A00,
			kCmdTextureFormat					= 0x1A04,
			kCmdTextureControl1					= 0x1A08,
			kCmdTextureControl2					= 0x1A0C,
			kCmdTextureSwizzle					= 0x1A10,
			kCmdTextureFilter					= 0x1A14,
			kCmdTextureSize1					= 0x1A18,
			kCmdTextureBorderColor				= 0x1A1C,
			kCmdVertexAttrib4f					= 0x1C00,
			kCmdTextureColorKeyColor			= 0x1D00,
			kCmdFragmentProgramControl			= 0x1D60,
			kCmdVertexProgramConstantRange		= 0x1D64,
			kCmdSyncSemaphoreOffset				= 0x1D6C,
			kCmdSignalBackendWriteSemaphore		= 0x1D70,
			kCmdSignalTextureReadSemaphore		= 0x1D74,
			kCmdDepthClamp						= 0x1D78,
			kCmdMultisampleControl				= 0x1D7C,
			kCmdCompressionEnable               = 0x1D80,
			kCmdCullingEnable					= 0x1D84,
			kCmdRenderTargetAuxHeight			= 0x1D88,
			kCmdClearDepthStencil				= 0x1D8C,
			kCmdClearColor						= 0x1D90,
			kCmdClear							= 0x1D94,
			kCmdPrimitiveRestartEnable			= 0x1DAC,
			kCmdPrimitiveRestartIndex			= 0x1DB0,
			kCmdLineStippleEnable				= 0x1DB4,
			kCmdLineStipplePattern				= 0x1DB8,
			kCmdVertexAttrib1f					= 0x1E40,
			kCmdRenderControl					= 0x1E98,
			kCmdVertexProgramInstrLoadSlot		= 0x1E9C,
			kCmdVertexProgramExecuteSlot		= 0x1EA0,
			kCmdDepthCullDepthControl			= 0x1EA4,
			kCmdDepthCullFeedbackControl		= 0x1EA8,
			kCmdDepthCullStencilControl			= 0x1EAC,
			kCmdPointSize						= 0x1EE0,
			kCmdVertexProgramPointSizeEnable	= 0x1EE4,
			kCmdPointSpriteControl				= 0x1EE8,
			kCmdVertexProgramLimits				= 0x1EF8,
			kCmdVertexProgramConstLoadSlot		= 0x1EFC,
			kCmdVertexProgramConst1				= 0x1F00,
			kCmdVertexProgramConst2				= 0x1F04,
			kCmdVertexProgramConst3				= 0x1F08,
			kCmdVertexProgramConst4				= 0x1F0C,
			kCmdVertexFrequencyControl			= 0x1FC0,
			kCmdVertexResultMapping1			= 0x1FC4,
			kCmdVertexResultMapping2			= 0x1FC8,
			kCmdVertexResultMapping3			= 0x1FCC,
			kCmdVertexResultMapping4			= 0x1FD0,
			kCmdVertexResultMapping5			= 0x1FD4,
			kCmdTextureCacheInvalidate			= 0x1FD8,
			kCmdFragmentProgramControl2         = 0x1FEC,
			kCmdVertexAttributeMask				= 0x1FF0,
			kCmdVertexProgramResultMask			= 0x1FF4,
			kCmdVertexProgramBranchBits         = 0x1FF8,

			kCmdHostToVidSetSubchannel          = 0x2000,
			kCmdHostToVidNotifiesContext        = 0x2180,
			kCmdHostToVidSourceContext          = 0x2184,
			kCmdHostToVidDestinationContext     = 0x2188,
			kCmdHostToVidSourceOffset           = 0x230C,
			kCmdHostToVidDestinationOffset      = 0x2310,
			kCmdHostToVidSourcePitch            = 0x2314,
			kCmdHostToVidDestinationPitch       = 0x2318,
			kCmdHostToVidSourceLineLength       = 0x231C,
			kCmdHostToVidLineCount              = 0x2320,
			kCmdHostToVidFormat                 = 0x2324,
			kCmdHostToVidBufferNotify           = 0x2328,

			kCmdSetSurfaceSubchannel            = 0x6000,
			kCmdSurfaceSourceContext            = 0x6184,
			kCmdSurfaceDestinationContext       = 0x6188,
			kCmdSurfaceFormat					= 0x6300,
			kCmdSurfacePitch					= 0x6304,
			kCmdSurfaceSourceAddress			= 0x6308,
			kCmdSurfaceDestinationAddress		= 0x630C,

			kCmdSetSwizzledSurfaceSubchannel    = 0x8000,
			kCmdSwizzleSurfaceNotifiesContext   = 0x8180,
			kCmdSwizzleSurfaceContext           = 0x8184,
			kCmdSwizzleSurfaceFormat            = 0x8300,
			kCmdSwizzleSurfaceOffset            = 0x8304,
			
			kCmdSetBlitSubchannel               = 0xA000,
			kCmdBlitSourcePoint					= 0xA300,
			kCmdBlitDestinationPoint			= 0xA304,
			kCmdBlitSize						= 0xA308,
			kCmdBlitData						= 0xA400,

			kCmdSetStretchBlitSubchannel        = 0xC000,
			kCmdStretchBlitNotifiesContext      = 0xC180,
			kCmdStretchBlitSourceContext        = 0xC184,
			kCmdStretchBlitDestinationContext   = 0xC198,
			kCmdStretchBlitConversion           = 0xC2FC,
			kCmdStretchBlitFormat               = 0xC300,
			kCmdStretchBlitOperation            = 0xC304,
			kCmdStretchBlitSourcePoint          = 0xC308,
			kCmdStretchBlitSourceSize           = 0xC30C,
			kCmdStretchBlitDestinationPoint     = 0xC310,
			kCmdStretchBlitDestinationSize      = 0xC314,
			kCmdStretchBlitDsDx                 = 0xC318,
			kCmdStretchBlitDtDy                 = 0xC31C,
			kCmdStretchBlitInSize               = 0xC400,
			kCmdStretchBlitInFormat             = 0xC404,
			kCmdStretchBlitInOffset             = 0xC408,
			kCmdStretchBlitInPoint              = 0xC40C,

			kCmdSetSwDriverSubchannel           = 0xE000,
			kCmdSwDriverPpuInterrupt            = 0xEB00,

		};

		//! Display Mode.
		/*! These values are passed to the Initialize() function.
		*/
		enum DisplayMode
		{
			//! Resolution 720x480.
			kDisplay480        = 1,
			//! Resolution 720x576
			kDisplay576        = 3,
			//! Resolution 720x576
			kDisplay720        = 5,
			//! Resolution 1920x1080.
			kDisplay1080       = 7,
			//! Resolution defined by setmonitor.self, pitch is defined as displayWidth*pixelSize
			kDisplayAutoLinear = 8,
			//! Resolution defined by setmonitor.self, pitch is defined as GetTiledPitch(displayWidth*pixelSize)
			kDisplayAutoTiled  = 9,
			//! Resolution defined by setmonitor.self, pitch is overridden with passed in value
			kDisplayAutoCustom = 10,
		};
		
		//! Display Aspect Ratio
		enum DisplayAspectRatio
		{
			//! 4:3 aspect ratio
			kAspectStandard   = 0,
			//! 16:9 aspect ratio
			kAspectWidescreen = 1
		};
		
		//! The display swap mode
		enum DisplaySwapMode
		{
			kSwapModeHSync     = 0,
			kSwapModeVSync     = 1,
			kSwapModeImmediate = 2
		};

		//! The color format of the output buffer.
		enum DisplayOutputMode
		{
			kOutModeArgb8888 = 1,
			kOutModeAbgr8888 = 2,
			kOutModeRgba16f  = 3
		};

		//! Enable codes for on/off rendering state.
		/*! These values are passed to the EnableRenderState() and DisableRenderState() functions.
		*/
		enum RenderState
		{
			kRenderDither                 = 0x0300,
			kRenderAlphaTest              = 0x0304,
			//! Only toggles blending for render target color buffer 0
			kRenderBlend                  = 0x0310,
			kRenderStencilTestFront       = 0x0328,
			kRenderStencilTestBack        = 0x0348,
			kRenderLogicOp                = 0x0374,
			kRenderDepthBoundsTest        = 0x0380,
			kRenderLineSmooth             = 0x03BC,
			kRenderPolygonOffsetPoint     = 0x0A60,
			kRenderPolygonOffsetLine      = 0x0A64,
			kRenderPolygonOffsetFill      = 0x0A68,
			kRenderDepthTest              = 0x0A74,
			kRenderSecondaryColor         = 0x1428,
			kRenderBackfacingColor        = 0x142C,
			kRenderPolygonStipple         = 0x147C,
			kRenderPixelCounting          = 0x17CC,
			//! Depth Cull Reports need to be disabled when writing their reports via CommandContext::WriteReport(kDepthCull*).
			kRenderDepthCullReports       = 0x1804,
			kRenderPolygonSmooth          = 0x1838,
			kRenderCullFace               = 0x183C,
			kRenderPrimitiveRestart       = 0x1DAC,
			kRenderLineStipple            = 0x1DB4,
			kRenderVertexProgramPointSize = 0x1EE4,
			kRenderGammaCorrectedWrites   = 0x1FEC
		};
		
		
		//! Draw buffer mask flags
		/*! These are combined and passed to the SetDrawBufferMask() function.
		*/
		enum
		{
			kDrawBuffer0 = 0x01,
			kDrawBuffer1 = 0x02,
			kDrawBuffer2 = 0x04,
			kDrawBuffer3 = 0x08
		};
		
		
		//! Attribute array type codes
		/*! These are passed to the SetVertexAttribArray() function.
		*/
		enum AttribType
		{
			kAttribSignedShortNormalized  = 0x01, // -1..1
			kAttribFloat                  = 0x02, // 32-bit float
			kAttribHalfFloat              = 0x03, // 16-bit float, format is s10e5
			kAttribUnsignedByteNormalized = 0x04, // 0..1
			kAttribSignedShort            = 0x05, // -32K..32K
			kAttribX11Y11Z10Normalized    = 0x06, // 32bit 10:11:11 bits (z,y,x), each -1..1, must be used with kAttribCount1
			kAttribUnsignedByte           = 0x07  // 0..255, must be used with kAttribCount4
		};
		
		
		//! Attribute array count codes
		/*! These are passed to the SetVertexAttribArray() function.
		*/
		enum AttribCount
		{
			kAttribDisable = 0x00,
			kAttribCount1  = 0x01, // Also used with kAttribX11Y11Z10Normalized although actual count is 3.
			kAttribCount2  = 0x02,
			kAttribCount3  = 0x03,
			kAttribCount4  = 0x04
		};
		
		
		//! Draw types
		/*! These are passed to the InsertDraw() function.
		*/
		enum DrawType
		{
			kDrawArrays         = 0x1814,
			kDrawElements       = 0x1824
		};
		
		//! Draw mode codes
		/*! These are passed to the DrawArrays() and DrawElements() functions.
		*/
		enum DrawMode
		{
			kDrawPoints        = 0x01,
			kDrawLines         = 0x02,
			kDrawLineLoop      = 0x03,
			kDrawLineStrip     = 0x04,
			kDrawTriangles     = 0x05,
			kDrawTriangleStrip = 0x06,
			kDrawTriangleFan   = 0x07,
			kDrawQuads         = 0x08,
			kDrawQuadStrip     = 0x09,
			kDrawPolygon       = 0x0A
		};
		
		
		//! Clear flags
		/*! These are combined and passed to the Clear() function.
		*/
		enum
		{
			kClearColor   = 0x00F0,
			kClearRed     = 0x0010,
			kClearGreen   = 0x0020,
			kClearBlue    = 0x0040,
			kClearAlpha   = 0x0080,
			kClearDepth   = 0x0001,
			kClearStencil = 0x0002
		};
		
		
		//! Point sprite modes
		/*! These are passed to the SetPointSpriteParameters() function.
		*/
		enum SpriteMode
		{
			kSpriteZero = 0x00,
			kSpriteR    = 0x01,
			kSpriteS    = 0x02
		};
		
		
		//! Texture coordinate indexes
		/*! These are passed to the SetTextureWrapMode() function.
		*/
		enum TexcoordComponent
		{
			kTexcoordS = 0,
			kTexcoordT = 1,
			kTexcoordR = 2
		};
		
		
		//! Texture wrap mode codes
		/*! These are passed to the SetTextureWrapMode() function.
		*/
		enum WrapMode
		{
			kWrapRepeat              = 0x01,
			kWrapMirrorRepeat        = 0x02,
			kWrapClampToEdge         = 0x03,
			kWrapClampToBorder       = 0x04,
			kWrapClamp               = 0x05,
			kWrapMirrorClampToEdge   = 0x06,
			kWrapMirrorClampToBorder = 0x07,
			kWrapMirrorClamp         = 0x08
		};
		
		
		//! Texture filter mode codes
		/*! These are passed to the SetTextureFilterMode() function.
		*/
		enum FilterMode
		{
			kFilterNearest              = 0x01,
			kFilterLinear               = 0x02,
			kFilterNearestMipmapNearest = 0x03,
			kFilterLinearMipmapNearest  = 0x04,
			kFilterNearestMipmapLinear  = 0x05,
			kFilterLinearMipmapLinear   = 0x06,
			kFilterConvolutionKernelMin = 0x07,
			kFilterConvolutionKernelMag = 0x04
		};

		//! Texture Convolution Kernel mode codes
		/*! These are passed to the SetTextureKernelMode() function.
		*/
		enum KernelMode
		{
			// 5-tap kernel: center tap weight 4, other taps weight 1.
			// Sample Pattern:
			//       1 1
			//        4 
			//       1 1
			kKernelQuincunx    = 0x00002000,
			// 9-tap kernel: ordered grid of even weighting.
			// Sample Pattern:
			//       1 1 1
			//       1 1 1
			//       1 1 1
			kKernelGaussian    = 0x00004000,
			// 5-tap kernel: center tap weight 4, other taps weight 1.
			kKernelQuincunxAlt = 0x00006000
		};
		
		
		//! Texture anisotropy codes
		/*! These are passed to the SetTextureMaxAnisotropy() function.
		*/
		enum AnisotropyLevel
		{
			kAnisotropy1  = 0x00,
			kAnisotropy2  = 0x01,
			kAnisotropy4  = 0x02,
			kAnisotropy6  = 0x03,
			kAnisotropy8  = 0x04,
			kAnisotropy10 = 0x05,
			kAnisotropy12 = 0x06,
			kAnisotropy16 = 0x07
		};


		//! OpenGL comparison function codes
		/*! These are passed to the following functions.
			- SetAlphaFunc()
			- SetDepthFunc()
			- SetStencilFunc()
			- SetStencilFuncSeparate()
			- SetStencilCullHint()
			- SetTextureDepthFunc()
		*/
		enum ComparisonFunc
		{
			kFuncNever        = 0x0200,
			kFuncLess         = 0x0201,
			kFuncEqual        = 0x0202,
			kFuncLessEqual    = 0x0203,
			kFuncGreater      = 0x0204,
			kFuncNotEqual     = 0x0205,
			kFuncGreaterEqual = 0x0206,
			kFuncAlways       = 0x0207
		};


		//! OpenGL blend factor codes
		/*! These are passed to the SetBlendFunc() and SetBlendFuncSeparate() functions.
		*/
		enum BlendFactor
		{
			kBlendZero                  = 0x0000,
			kBlendOne                   = 0x0001,
			kBlendSrcColor              = 0x0300,
			kBlendOneMinusSrcColor      = 0x0301,
			kBlendSrcAlpha              = 0x0302,
			kBlendOneMinusSrcAlpha      = 0x0303,
			kBlendDstAlpha              = 0x0304,
			kBlendOneMinusDstAlpha      = 0x0305,
			kBlendDstColor              = 0x0306,
			kBlendOneMinusDstColor      = 0x0307,
			kBlendSrcAlphaSaturate      = 0x0308,
			kBlendConstantColor         = 0x8001,
			kBlendOneMinusConstantColor = 0x8002,
			kBlendConstantAlpha         = 0x8003,
			kBlendOneMinusConstantAlpha = 0x8004
		};


		//! OpenGL blend equation codes
		/*! These are passed to the SetBlendEquation() and SetBlendEquationSeparate() functions.
		*/
		enum BlendEquation
		{
			kBlendEquationAdd                   = 0x8006,
			kBlendEquationMin                   = 0x8007,
			kBlendEquationMax                   = 0x8008,
			kBlendEquationSubtract              = 0x800A,
			kBlendEquationReverseSubtract       = 0x800B,
			kBlendEquationSignedReverseSubtract = 0xF005,
			kBlendEquationSignedAdd             = 0xF006,
			kBlendEquationSignedSubtract        = 0xF007,
		};


		//! OpenGL stencil operation codes
		/*! These are passed to the SetStencilOp() and SetStencilOpSeparate() functions.
		*/
		enum StencilOp
		{
			kStencilZero          = 0x0000,
			kStencilKeep          = 0x1E00,
			kStencilReplace       = 0x1E01,
			kStencilIncrement     = 0x1E02,
			kStencilDecrement     = 0x1E03,
			kStencilInvert        = 0x150A,
			kStencilIncrementWrap = 0x8507,
			kStencilDecrementWrap = 0x8508
		};


		//! OpenGL logic operation codes
		/*! These are passed to the SetLogicOp() function.
		*/
		enum LogicOp
		{
			kLogicClear        = 0x1500,
			kLogicAnd          = 0x1501,
			kLogicAndReverse   = 0x1502,
			kLogicCopy         = 0x1503,
			kLogicAndInverted  = 0x1504,
			kLogicNop          = 0x1505,
			kLogicXor          = 0x1506,
			kLogicOr           = 0x1507,
			kLogicNor          = 0x1508,
			kLogicEquiv        = 0x1509,
			kLogicInvert       = 0x150A,
			kLogicOrReverse    = 0x150B,
			kLogicCopyInverted = 0x150C,
			kLogicOrInverted   = 0x150D,
			kLogicNand         = 0x150E,
			kLogicSet          = 0x150F
		};


		//! OpenGL shading model codes
		/*! These are passed to the SetShadeModel() function.
		*/
		enum ShadeModel
		{
			kShadeModelFlat   = 0x1D00,
			kShadeModelSmooth = 0x1D01
		};


		//! OpenGL fog mode codes
		/*! These are passed to the SetFogMode() function.
		*/
		enum FogMode
		{
			kFogLinear = 0x2601,
			kFogExp    = 0x0800,
			kFogExp2   = 0x0801
		};
		
		
		//! OpenGL polygon mode codes
		/*! These are passed to the SetPolygonMode() and SetPolygonModeSeparate() functions.
		*/
		enum PolygonMode
		{
			kPolygonModePoint = 0x1B00,
			kPolygonModeLine  = 0x1B01,
			kPolygonModeFill  = 0x1B02
		};


		//! OpenGL cull face codes
		/*! These are passed to the SetCullFace() function.
		*/
		enum CullFace
		{
			kCullFaceFront        = 0x0404,
			kCullFaceBack         = 0x0405,
			kCullFaceFrontAndBack = 0x0408
		};


		//! OpenGL front face codes
		/*! These are passed to the SetFrontFace() function.
		*/
		enum FrontFace
		{
			kFrontFaceCw  = 0x0900,
			kFrontFaceCcw = 0x0901
		};
		
		
		//! Clip plane function codes
		/*! These are passed to the SetClipFunc() function.
		*/
		enum ClipFunc
		{
			kClipNone             = 0x0000,
			kClipLessZero         = 0x0001,
			kClipGreaterEqualZero = 0x0002
		};
		
		
		//! Depth-cull depth test direction codes
		/*! These are passed to the SetDepthCullControl() function.
		*/
		enum DepthCullDirection
		{
			kCullLess    = 0x0000,
			kCullGreater = 0x0001
		};


		//! Index type codes
		/*! These are passed to the DrawElements() function.
		*/
		enum IndexType
		{
			kIndex16 = 0x0010,
			kIndex32 = 0x0000
		};
		
		
		//! Texture formats
		/*! These are passed to the various SetTextureImage*() functions.
		*/
		enum
		{
			kTextureLuminance8       = 0x0100A9FF,
			kTextureIntensity8       = 0x0100AAFF,
			kTextureAlpha8           = 0x010002FF,
			kTextureArgb1555         = 0x0200AAE4,
			kTextureArgb4444         = 0x0300AAE4,
			kTextureRgb565           = 0x0400A9E4,
			kTextureRgba8888         = 0x0500AA93,
			kTextureArgb8888         = 0x0500AAE4,
			kTextureBgra8888         = 0x0500AA1B,
			kTextureDxt1             = 0x0600AAE4,
			kTextureDxt3             = 0x0700AAE4,
			kTextureDxt5             = 0x0800AAE4,
			kTextureLuminanceAlpha88 = 0x0B00AAAB,
			kTextureDepth24X8        = 0x1000AAE4,
			kTextureDepth16          = 0x1200A900,
			kTextureLuminance16      = 0x1400A9EE,
			kTextureIntensity16      = 0x1400AAEE,
			kTextureAlpha16          = 0x140002EE,
			kTextureLuminanceAlpha16 = 0x1501AAE4,
			kTextureRgba16f          = 0x1A00AAE4,
			kTextureRgba32f          = 0x1B00AAE4,
			kTextureR32f             = 0x1C0009E4,
			kTextureLuminance32f     = 0x1C00A9E4,
			kTextureGr16f            = 0x1F0029E4
		};

		
		//! Render target color buffer formats
		/*! These are stored in the RenderTarget struct.
		    NOTE - Swizzled buffers cannot be multisampled.
		    NOTE - Swizzled buffers cannot be in a tiled memory region.
		*/
		enum ColorBufferFormat
		{
			//! If swizzled, must be used with a D16S0 Depth buffer.
			kBufferXrgbZrgb1555 = 0x01,
			//! If swizzled, must be used with a D16S0 Depth buffer.
			kBufferXrgbOrgb1555 = 0x02,
			//! If swizzled, must be used with a D16S0 Depth buffer.
			kBufferRgb565       = 0x03,
			//! If swizzled, must be used with a D24S8 Depth buffer.
			kBufferXrgbZrgb8888 = 0x04,
			//! If swizzled, must be used with a D24S8 Depth buffer.
			kBufferXrgbOrgb8888 = 0x05,
			//! If swizzled, must be used with a D24S8 Depth buffer.
			kBufferArgb8888     = 0x08,
			//! WARNING: Rendertargets with the B8 color buffer format cannot 
			//! have depth or stencil testing enabled and the depth or stencil
			//! buffer cannot be cleared!
			//! Swizzling must also be disabled.
			//! Mrt is must also be disabled.
			kBufferB8           = 0x09,
			//! WARNING: Rendertargets with the Gb88 color buffer format cannot 
			//! have depth or stencil testing enabled and the depth or stencil
			//! buffer cannot be cleared!
			//! Swizzling must also be disabled.
			//! Mrt is must also be disabled.
			kBufferGb88         = 0x0A,
			/*! Fp buffers cannot be swizzled or multisampled and must use a D24S8
			    depth buffer. 
			*/
			kBufferRgba16f      = 0x0B,
			/*! Fp buffers cannot be swizzled or multisampled and must use a D24S8
			    depth buffer. 
			*/
			kBufferRgba32f      = 0x0C,
			/*! Fp buffers cannot be swizzled or multisampled and must use a D24S8
			    depth buffer. 
			*/
			kBufferR32f         = 0x0D,
			//! If swizzled, must be used with a D24S8 Depth buffer.
			kBufferXbgrZbgr8888 = 0x0E,
			//! If swizzled, must be used with a D24S8 Depth buffer.
			kBufferXbgrObgr8888 = 0x0F,
			//! If swizzled, must be used with a D24S8 Depth buffer.
			kBufferAbgr8888     = 0x10,
		};
		
		
		//! Render target depth/stencil buffer formats
		/*! These are stored in the RenderTarget struct.
		*/
		enum DepthStencilBufferFormat
		{
			//! Stencil testing/writing must be disabled when using this format.
			kBufferD16S0 = 0x20,
			kBufferD24S8 = 0x40
		};


		//! Multisample modes
		/*! These are stored in the RenderTarget struct.
		*/
		enum MultisampleMode
		{
			kMultisampleNone      = 0x00,
			kMultisample2X        = 0x03,
			kMultisample4XOrdered = 0x04,
			kMultisample4XRotated = 0x05
		};
		
		
		//! Render target flags
		/*! These are stored in the m_targetFlags field of the RenderTarget struct.
		*/
		enum
		{
			kTargetLinear   = 0,
			kTargetSwizzled = 1
		};

		
	
		//! Tile compression format.
		/*! These are passed to the SetTileAndCompression() function.
		*/
		enum CompressionFormat
		{
			kCompDisabled              = 0x0,
			kCompColor2x1              = 0x7,
			kCompColor2x2              = 0x8,
			kCompDepthStencil          = 0x9,
			kCompDepthStencil2X        = 0xB,
			kCompDepthStencil4XOrdered = 0xA,
			kCompDepthStencil4XRotated = 0xC
		};
		
		//! Early depth cull rejection format.
		/*! These are passed to the SetDepthCullArea() function.
		*/
		enum DepthCullFormat
		{
			//! Uses the most significant 12 bits of the depth value as the compressed format. 
			kCullMsb   = 0x0,
			//! Counts the number of bits in the most significant 16 bits of the depth value
			//! and uses this data in a format similar to floating point as the compressed format.
			kCullLones = 0x1
		};
		
		//! Specifies the location of the index buffer.
		/*! These are passed to the DrawElements() function.
		*/
		enum IndexContext
		{
			kIndexVideoMemory = 0x0,
			kIndexMainMemory  = 0x1
		};

		//! Specifies the location of the texture.
		/*! These are passed to the various SetTextureImage*() functions.
		*/
		enum TextureContext
		{
			kTextureVideoMemory = 0x1,
			kTextureMainMemory  = 0x2
		};

		//! Specifies the location of the fragment program.
		/*! These are passed to the SetFragmentProgram() function.
		*/
		enum FragmentProgramContext
		{
			kFragmentProgramVideoMemory = 0x1,
			kFragmentProgramMainMemory  = 0x2
		};

		//! Specifies the location of the vertex array.
		/*! These are passed to the SetVertexAttribPointer() function.
		*/
		enum AttribContext
		{
			kAttribVideoMemory = 0x00000000,
			kAttribMainMemory  = 0x80000000
		};

		//! Specifies the location of a Render Target Color and Depth Buffers.
		/*! These are passed to the SetColorAndDepthBuffers() functions.
		*/
		enum RenderTargetContext
		{
			kRenderTargetVideoMemory = 0xFEED0000,
			kRenderTargetMainMemory  = 0xFEED0001
		};
		
		//! Specifies the location of a Tiled Region.
		/*! These are passed to the SetTileAndCompression() function.
		*/
		enum TileContext
		{
			kTileVideoMemory = 0,
			kTileMainMemory  = 1
		};
		
		//! Specifies the location of a source or destination offset when used
		//! in a Copy*() operation.
		enum CopyContext
		{
			kCopyVideoMemory = 0xFEED0000,
			kCopyMainMemory  = 0xFEED0001
		};

		//! Specifies the degree of validation that you would like to perform with ValidatePushBuffer().
		enum ValidationLevel
		{
			kValidateNormal,
			kValidateHeavy
		};

		//! Specifies the error type upon validating a push buffer.
		enum ValidationError
		{
			kErrorNone                                    = 0,
			kErrorBeginWithoutEnd                         = 1,
			kErrorEndWithoutBegin                         = 2,
			kErrorCallWithoutReturn                       = 3,
			kErrorReturnWithoutCall                       = 4,
			kErrorShortJumpToInvalidLocation              = 5,
			kErrorLongJumpToInvalidLocation               = 6,
			kErrorCallToInvalidLocation                   = 7,
			kErrorStencilTestEnabledWithoutStencilBuffer  = 8,
			kErrorSemaphoreAddressAlignment               = 9,
			kErrorSemaphoreAddressOutOfRange              = 10,
			kErrorRenderTargetColorBuffer0Alignment       = 11,
			kErrorRenderTargetColorBuffer1Alignment       = 12,
			kErrorRenderTargetColorBuffer2Alignment       = 13,
			kErrorRenderTargetColorBuffer3Alignment       = 14,
			kErrorFragmentProgramAlignment                = 15,
			kErrorVertexTexture0Alignment                 = 16,
			kErrorVertexTexture1Alignment                 = 17,
			kErrorVertexTexture2Alignment                 = 18,
			kErrorVertexTexture3Alignment                 = 19,
			kErrorClearDepthBufferOnB8RenderTarget        = 20,
			kErrorClearDepthBufferOnGb88RenderTarget      = 21,
			kErrorAlphaTestingWithMultipleRenderTargets   = 22,
			kErrorAlphaToCoverageWithMultipleRenderTargets = 23,
			kErrorMultisampledFpRenderTarget              = 24,
			kErrorFpRenderTargetLineSmooth                = 25,
			kErrorFpRenderTargetPolygonSmooth             = 26,
			kErrorInvalidRegister                         = 27,
			kErrorSteppedToInvalidLocation                = 28,
			kErrorDepthTestEnabledOnB8RenderTarget        = 29,
			kErrorDepthTestEnabledOnGb88RenderTarget      = 30,
			kErrorDepthBufferMustBeD24S8WithFpRenderTarget= 31,
			kErrorInvalidRegisterValue                    = 32,
			kErrorHeightFor1DTextureIsNotOne			  = 33,
			kErrorWidthFor1DTextureWithBorderTexelsIsGreaterThan2048 = 34,
			kErrorWidthFor1DTextureIsGreaterThan4096      = 35,
			kErrorWriteReportAlignment                    = 36,
			kErrorIndexBufferAlignment                    = 37,
			kErrorSemaphoreSignalAddressAlignment         = 38,
			kErrorSemaphoreSignalAddressOutOfRange        = 39,
			kErrorConditionalRenderAlignment              = 40,
			kErrorMultisampledSwizzledRenderTarget        = 41,
			kErrorClearFloatingPointRenderTarget          = 42,
			kErrorInvalidViewport                         = 43,
			kErrorInvalidScissor                          = 44,
			kErrorSwizzledB8RenderTarget                  = 45,
			kErrorSwizzledGb88RenderTarget                = 46,
			kErrorTexture0Alignment                       = 47,
			kErrorTexture1Alignment                       = 48,
			kErrorTexture2Alignment                       = 49,
			kErrorTexture3Alignment                       = 50,
			kErrorTexture4Alignment                       = 51,
			kErrorTexture5Alignment                       = 52,
			kErrorTexture6Alignment                       = 53,
			kErrorTexture7Alignment                       = 54,
			kErrorTexture8Alignment                       = 55,
			kErrorTexture9Alignment                       = 56,
			kErrorRenderTargetDepthBuffer0Alignment       = 57,
			kErrorStartAtInvalidLocation                  = 58,
			kErrorEndAtInvalidLocation                    = 59,
			kErrorHeightFor2DTextureWithBorderTexelsIsGreaterThan2048 = 60,
			kErrorWidthFor2DTextureWithBorderTexelsIsGreaterThan2048 = 61,
			kErrorHeightFor2DTextureIsGreaterThan4096     = 62,
			kErrorWidthFor2DTextureIsGreaterThan4096      = 63,
			kErrorHeightFor3DTextureWithBorderTexelsIsGreaterThan256 = 64,
			kErrorWidthFor3DTextureWithBorderTexelsIsGreaterThan256 = 65,
			kErrorHeightFor3DTextureIsGreaterThan512      = 66,
			kErrorWidthFor3DTextureIsGreaterThan512       = 67,
			kErrorAnisotropicFilteringWith3DTexture       = 68,
			kErrorConvolutionFilterWith3DTexture          = 69,
			kErrorConvolutionFilterAndAnisotropicFilteringWith2DTexture = 70,
			kErrorGammaCorrectionOnTextureWithout8bitComponents = 71,
			kErrorAnisotropicFilteringWithFpTexture       = 72,
			kErrorComponentSwizzlingWithFpTexture         = 73,
			kErrorPerTextureAlphaTestWithFpTexture        = 74,
			kErrorInvalidSwizzleFor16bitPerComponentTexture = 75,
			kErrorCubeMappingWith1DTexture                = 76,
			kErrorCubeMappingWith3DTexture                = 77,
			kErrorCubeMappingWithoutEqualWidthHeight      = 78,
			kErrorWidthNotPowerOfTwoWithSwizzledTexture   = 79,
			kErrorHeightNotPowerOfTwoWithSwizzledTexture  = 80,
			kErrorSwizzledTextureWithNonZeroPitch         = 81,
			kErrorDepthFor3DTextureIsGreaterThan512       = 82,
			kErrorDepthFor3DTextureWithBorderTexelsIsGreaterThan256 = 83,
			kErrorDepthFor1DTextureIsNotOne               = 84,
			kErrorDepthFor2DTextureIsNotOne               = 85,
			kErrorInvalidSwizzledTextureFormat            = 86,
			kErrorDepthTexturesMustBe1DOr2D               = 87,
			kErrorWidthFor3DTexturesMustBePowerOfTwo      = 88,
			kErrorHeightFor3DTexturesMustBePowerOfTwo     = 89,
			kErrorDepthFor3DTexturesMustBePowerOfTwo      = 90,
			kErrorCubeMapTexturesMustBePowerOfTwo         = 91,
			kErrorTexturesWithBordersMustBePowerOfTwo     = 92,
			kErrorWidthOnUnormalizedTextureWithWrapOrMirrorMustBePowerOfTwo = 93,
			kErrorHeightOnUnormalizedTextureWithWrapOrMirrorMustBePowerOfTwo = 94,
			kErrorDepthOnUnormalizedTextureWithWrapOrMirrorMustBePowerOfTwo = 95,
			kErrorUnnormalizedTexturesCannotHaveMips      = 96,
			kErrorVertexTexturesCannotBe3D                = 97,
			kErrorVertexTexturesMustBeNormalized          = 98,
			kErrorVertexTexturesMustBeLinear              = 99,
			kErrorVertexTexturesMustBeRgba32fOrR32f       = 100,
			kErrorAttributeFormat111110RequiresCountOfOne = 101,
			kErrorAttributeFormatUByteRequiresCountOfFour = 102,
			kErrorBlendingWithFp32RenderTarget            = 103,
			kErrorAlphaTestingWithFp32RenderTarget        = 104,
			kErrorFp32TextureWithoutNearestMagFilter      = 105,
			kErrorFp32TextureWithoutNearestMipmapNearestMinFilter = 106,
			kErrorSwizzledFpRenderTarget                  = 107,
			kErrorSwizzledRenderTargetColorBuffer0PitchMustBePowerOfTwo = 108,
			kErrorSwizzledRenderTargetColorBuffer1PitchMustBePowerOfTwo = 109,
			kErrorSwizzledRenderTargetColorBuffer2PitchMustBePowerOfTwo = 110,
			kErrorSwizzledRenderTargetColorBuffer3PitchMustBePowerOfTwo = 111,
			kErrorSwizzledRenderTargetDepthBufferPitchMustBePowerOfTwo = 112,
			kErrorSwizzledRenderTargetColorBuffer0PitchMustBeGreaterThanOrEqualTo64 = 113,
			kErrorSwizzledRenderTargetColorBuffer1PitchMustBeGreaterThanOrEqualTo64 = 114,
			kErrorSwizzledRenderTargetColorBuffer2PitchMustBeGreaterThanOrEqualTo64 = 115,
			kErrorSwizzledRenderTargetColorBuffer3PitchMustBeGreaterThanOrEqualTo64 = 116,
			kErrorSwizzledRenderTargetDepthBufferPitchMustBeGreaterThanOrEqualTo64 = 117,
			kErrorColorBuffer0EnabledButNotWrittenByFragmentProgram = 118,
			kErrorColorBuffer1EnabledButNotWrittenByFragmentProgram = 119,
			kErrorColorBuffer2EnabledButNotWrittenByFragmentProgram = 120,
			kErrorColorBuffer3EnabledButNotWrittenByFragmentProgram = 121,
			kErrorMultisamplingEnabledWithNonMultisampledRenderTarget = 122,
			kErrorRenderTargetColorBuffer0InvalidOffset   = 123,
			kErrorRenderTargetColorBuffer1InvalidOffset   = 124,
			kErrorRenderTargetColorBuffer2InvalidOffset   = 125,
			kErrorRenderTargetColorBuffer3InvalidOffset   = 126,
			kErrorFragmentProgramInvalidOffset            = 127,
			kErrorVertexTexture0InvalidOffset             = 128,
			kErrorVertexTexture1InvalidOffset             = 129,
			kErrorVertexTexture2InvalidOffset             = 130,
			kErrorVertexTexture3InvalidOffset             = 131,
			kErrorTexture0InvalidOffset                   = 132,
			kErrorTexture1InvalidOffset                   = 133,
			kErrorTexture2InvalidOffset                   = 134,
			kErrorTexture3InvalidOffset                   = 135,
			kErrorTexture4InvalidOffset                   = 136,
			kErrorTexture5InvalidOffset                   = 137,
			kErrorTexture6InvalidOffset                   = 138,
			kErrorTexture7InvalidOffset                   = 139,
			kErrorTexture8InvalidOffset                   = 140,
			kErrorTexture9InvalidOffset                   = 141,
			kErrorTexture10InvalidOffset                  = 142,
			kErrorTexture11InvalidOffset                  = 143,
			kErrorTexture12InvalidOffset                  = 144,
			kErrorTexture13InvalidOffset                  = 145,
			kErrorTexture14InvalidOffset                  = 146,
			kErrorTexture15InvalidOffset                  = 147,
			kErrorRenderTargetDepthBufferInvalidOffset    = 148,
			kErrorIndexBufferInvalidOffset                = 149,
			kErrorVertexAttribArray0InvalidOffset         = 150,
			kErrorVertexAttribArray1InvalidOffset         = 151,
			kErrorVertexAttribArray2InvalidOffset         = 152,
			kErrorVertexAttribArray3InvalidOffset         = 153,
			kErrorVertexAttribArray4InvalidOffset         = 154,
			kErrorVertexAttribArray5InvalidOffset         = 155,
			kErrorVertexAttribArray6InvalidOffset         = 156,
			kErrorVertexAttribArray7InvalidOffset         = 157,
			kErrorVertexAttribArray8InvalidOffset         = 158,
			kErrorVertexAttribArray9InvalidOffset         = 159,
			kErrorVertexAttribArray10InvalidOffset        = 160,
			kErrorVertexAttribArray11InvalidOffset        = 161,
			kErrorVertexAttribArray12InvalidOffset        = 162,
			kErrorVertexAttribArray13InvalidOffset        = 163,
			kErrorVertexAttribArray14InvalidOffset        = 164,
			kErrorVertexAttribArray15InvalidOffset        = 165,
			kErrorTexture10Alignment                      = 166,
			kErrorTexture11Alignment                      = 167,
			kErrorTexture12Alignment                      = 168,
			kErrorTexture13Alignment                      = 169,
			kErrorTexture14Alignment                      = 170,
			kErrorTexture15Alignment                      = 171,
			kErrorDrawElementsIndexBufferMemoryOverrun    = 172,
			kErrorDrawElementsVertexBuffer0MemoryOverrun  = 173,
			kErrorDrawElementsVertexBuffer1MemoryOverrun  = 174,
			kErrorDrawElementsVertexBuffer2MemoryOverrun  = 175,
			kErrorDrawElementsVertexBuffer3MemoryOverrun  = 176,
			kErrorDrawElementsVertexBuffer4MemoryOverrun  = 177,
			kErrorDrawElementsVertexBuffer5MemoryOverrun  = 178,
			kErrorDrawElementsVertexBuffer6MemoryOverrun  = 179,
			kErrorDrawElementsVertexBuffer7MemoryOverrun  = 180,
			kErrorDrawElementsVertexBuffer8MemoryOverrun  = 181,
			kErrorDrawElementsVertexBuffer9MemoryOverrun  = 182,
			kErrorDrawElementsVertexBuffer10MemoryOverrun = 183,
			kErrorDrawElementsVertexBuffer11MemoryOverrun = 184,
			kErrorDrawElementsVertexBuffer12MemoryOverrun = 185,
			kErrorDrawElementsVertexBuffer13MemoryOverrun = 186,
			kErrorDrawElementsVertexBuffer14MemoryOverrun = 187,
			kErrorDrawElementsVertexBuffer15MemoryOverrun = 188,
			kErrorDrawArraysVertexBuffer0MemoryOverrun    = 189,
			kErrorDrawArraysVertexBuffer1MemoryOverrun    = 190,
			kErrorDrawArraysVertexBuffer2MemoryOverrun    = 191,
			kErrorDrawArraysVertexBuffer3MemoryOverrun    = 192,
			kErrorDrawArraysVertexBuffer4MemoryOverrun    = 193,
			kErrorDrawArraysVertexBuffer5MemoryOverrun    = 194,
			kErrorDrawArraysVertexBuffer6MemoryOverrun    = 195,
			kErrorDrawArraysVertexBuffer7MemoryOverrun    = 196,
			kErrorDrawArraysVertexBuffer8MemoryOverrun    = 197,
			kErrorDrawArraysVertexBuffer9MemoryOverrun    = 198,
			kErrorDrawArraysVertexBuffer10MemoryOverrun   = 199,
			kErrorDrawArraysVertexBuffer11MemoryOverrun   = 200,
			kErrorDrawArraysVertexBuffer12MemoryOverrun   = 201,
			kErrorDrawArraysVertexBuffer13MemoryOverrun   = 202,
			kErrorDrawArraysVertexBuffer14MemoryOverrun   = 203,
			kErrorDrawArraysVertexBuffer15MemoryOverrun   = 204,
			kErrorScissorMustBeInsideViewport             = 205,
			kErrorSignedBlendEquationWithFpRenderTarget   = 206,
			kErrorSignedBlendEquationWith16bitRenderTarget = 207,
			kErrorMrtWithB8RenderTarget                   = 208,
			kErrorMrtWithGb88RenderTarget                 = 209,
			kErrorSwizzledXrgbZrgb1555WithoutD16S0        = 210,
			kErrorSwizzledXrgbOrgb1555WithoutD16S0        = 211,
			kErrorSwizzledRgb565WithoutD16S0              = 212,
			kErrorSwizzledXrgbZrgb8888WithoutD24S8        = 213,
			kErrorSwizzledXrgbOrgb8888WithoutD24S8        = 214,
			kErrorSwizzledArgb8888WithoutD24S8            = 215,
			kErrorSwizzledXbgrZbgr8888WithoutD24S8        = 216,
			kErrorSwizzledXbgrObgr8888WithoutD24S8        = 217,
			kErrorSwizzledAbgr8888WithoutD24S8            = 218,
			kErrorDitherEnabledWithMrt                    = 219,
			kErrorMrtAndColorBuffersInBothVideoAndMainMemory = 220,
		};

		//! Specifies the error type upon validating a push buffer.
		enum ValidationWarning
		{
			kWarningGammaCorrectedWritesWithoutFp16FragmentProgram  = 0x0000000000000001ULL,
			kWarningClearColorDoesNotMatchRenderTargetFormat        = 0x0000000000000002ULL,
		};

		//! Specifies the type of report to use.
		enum ReportType
		{
			//! The number of pixels written since the last Report Reset.
			kReportPixelCount         = 1,
			//! A Depth Cull Processor feedback value used to optimize Depth Cull performance. 
			/*! Note - Resetting this report resets all Depth Cull reports.
			    Use with SetDepthCullFeedback(). 
			*/
			kReportDepthCullFeedbackA = 2,
			//! A Depth Cull Processor feedback value used to optimize Depth Cull performance. 
			/*! Note - This report cannot be reset!
			    Use with SetDepthCullFeedback(). 
			*/
			kReportDepthCullFeedbackB = 3,
			//! A Depth Cull Processor feedback value used to optimize Depth Cull performance. 
			/*! Note - This report cannot be reset!
			    Use with SetDepthCullFeedback(). 
			*/
			kReportDepthCullFeedbackC = 4,
			//! A Depth Cull Processor feedback value used to optimize Depth Cull performance. 
			/*! Note - This report cannot be reset!
			    Use with SetDepthCullFeedback(). 
			*/
			kReportDepthCullFeedbackD = 5
		};

		//! Specifies the type of operation to perform on all windows
		enum WindowClipMode
		{
			//! Render only on the inside of the windows
			kClipOutside = 0,
			//! Render only on the outside of the windows
			kClipInside  = 1
		};

		//! Specified the amount of debugging information to report on error.
		enum DebugOutputLevel
		{
			//! Do not display debugging output.
			kDebugLevelNone,
			//! Display only simple debugging information.
			kDebugLevelSimple,
			//! Display the all of the debugging information available.
			kDebugLevelComplete
		};

		//! Gamma correction (sRGB) mask flags
		/*! These are combined and passed to the Texture::SetGammaCorrection() function.
		*/
		enum
		{
			kGammaCorrectR    = 0x00100000,
			kGammaCorrectG    = 0x00200000,
			kGammaCorrectB    = 0x00400000,
			kGammaCorrectA    = 0x00800000,
			kGammaCorrectSRgb = 0x00700000
		};

		//! The mode of rendering to perform.
		/*! These are passed to the SetRenderControl() function.
		*/
		enum RenderControlMode
		{
			kRenderDisabled    = 0x00000000,
			kRenderEnabled     = 0x01000000,
			kRenderConditional = 0x02000000
		};

		//! The mode of color keying to perform.
		/*! These are passed to the SetTextureColorKeyMode() function.
		*/
		enum ColorKeyMode
		{
			kColorKeyDisabled          = 0x0,
			kColorKeyZeroColorAndAlpha = 0x2,
			kColorKeyDiscard           = 0x3
		};
		
		//! The filtering mode to use during with a scaled copy.
		enum CopyFilterMode
		{
			kCopyFilterNearest = 0x00000000,
			kCopyFilterLinear  = 0x01000000
		};
	}
}
		
#endif // ICE_RENDERCONSTANTS_H
