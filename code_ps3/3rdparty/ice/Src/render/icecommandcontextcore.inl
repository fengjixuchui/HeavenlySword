/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#if ICERENDER_INLINE
#define COMMANDCONTEXT_DECL FORCE_INLINE
#else
#define COMMANDCONTEXT_DECL
#endif

#if ICERENDER_UNSAFE && ICEDEBUG
#define CHECK_RESERVE_BYTES(a) ICE_ASSERT(WillReserveBytesFail(a) == false)
#define CHECK_RESERVE(a) ICE_ASSERT(WillReserveFail(a) == false)
#define CHECK_RESERVE_RET(a, b) ICE_ASSERT(WillReserveFail(a) == false)
#elif (ICERENDER_UNSAFE==0)
#define CHECK_RESERVE_BYTES(a) if(!ReserveBytes(a)) return
#define CHECK_RESERVE(a) if(!Reserve(a)) return
#define CHECK_RESERVE_RET(a, b) if(!Reserve(a)) return (b)
#else
#define CHECK_RESERVE_BYTES(a)
#define CHECK_RESERVE(a)
#define CHECK_RESERVE_RET(a, b)
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetRenderTarget(const RenderTarget *__restrict target)
{
	ICE_ASSERT(target != NULL);

	CHECK_RESERVE(30);

	U32 targetLeftWidth = target->m_targetLeftWidth;
	U32 targetTopHeight = target->m_targetTopHeight;
	m_cmdptr[0] = CMD(kCmdRenderTargetLeftWidth, 32);
	m_cmdptr[1] = targetLeftWidth;
	m_cmdptr[2] = targetTopHeight;
	U32 targetFormat = target->m_targetFormat;
	U32 colorPitch0 = target->m_colorPitch[0];
	m_cmdptr[3] = targetFormat;
	m_cmdptr[4] = colorPitch0;
	m_cmdptr[20] = CMD(kCmdMultisampleControl, 4);
#ifdef __SPU__
	m_cmdptr[21] = (((targetFormat >> 12) & 0x0F) == 0) ? 0xFFFF0000 : 0xFFFF0001;
#else
#if 0 // @@@ This should work?! popcntb is an invalid op-code?
	U64 msEnable = (targetFormat >> 12) & 6;
	Popcntb(msEnable, msEnable);
	m_cmdptr[21] = 0xFFFF0000 | msEnable;
#else // The alternative...
	m_cmdptr[21] = 0xFFFF0000 | ((0 - ((targetFormat >> 12) & 0x0F)) >> 31);
#endif
#endif
	U32 colorOffset0 = target->m_colorOffset[0];
	U32 depthOffset = target->m_depthOffset;
	m_cmdptr[5] = colorOffset0;
	m_cmdptr[6] = depthOffset;
	U32 colorOffset1 = target->m_colorOffset[1];
	U32 colorPitch1 = target->m_colorPitch[1];
	m_cmdptr[7] = colorOffset1;
	m_cmdptr[8] = colorPitch1;
	U32 depthPitch = target->m_depthPitch;
	m_cmdptr[9] = CMD(kCmdDepthBufferPitch, 4);
	m_cmdptr[10] = depthPitch;
	U32 colorPitch2 = target->m_colorPitch[2];
	U32 colorPitch3 = target->m_colorPitch[3];
	m_cmdptr[11] = CMD(kCmdColorBufferPitch2, 16);
	m_cmdptr[12] = colorPitch2;
	m_cmdptr[13] = colorPitch3;
	U32 colorOffset2 = target->m_colorOffset[2];
	U32 colorOffset3 = target->m_colorOffset[3];
	m_cmdptr[14] = colorOffset2;
	m_cmdptr[15] = colorOffset3;
	U32 targetOrigin = target->m_targetOrigin;
	m_cmdptr[16] = CMD(kCmdRenderTargetOrigin, 4);
	m_cmdptr[17] = targetOrigin;
	U32 targetAuxHeight = target->m_targetAuxHeight;
	m_cmdptr[18] = CMD(kCmdRenderTargetAuxHeight, 4);
	m_cmdptr[19] = targetAuxHeight;
	U32 colorContext0 = target->m_colorContext[0];
	U32 depthContext = target->m_depthContext;
	m_cmdptr[22] = CMD(kCmdColorBufferContext0, 8);
	m_cmdptr[23] = colorContext0;
	m_cmdptr[24] = depthContext;
	U32 colorContext1 = target->m_colorContext[1];
	m_cmdptr[25] = CMD(kCmdColorBufferContext1, 4);
	m_cmdptr[26] = colorContext1;
	U32 colorContext2 = target->m_colorContext[2];
	U32 colorContext3 = target->m_colorContext[3];
	m_cmdptr[27] = CMD(kCmdColorBufferContext2, 8);
	m_cmdptr[28] = colorContext2;
	m_cmdptr[29] = colorContext3;
	m_cmdptr += 30;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::ToggleRenderState(RenderState state, bool enable)
{
	CHECK_RESERVE(2);
	
	ICE_ASSERTF((state == kRenderDither)
		|| (state == kRenderAlphaTest)
		|| (state == kRenderStencilTestFront)
		|| (state == kRenderStencilTestBack)
		|| (state == kRenderLogicOp)
		|| (state == kRenderDepthBoundsTest)
		|| (state == kRenderLineSmooth)
		|| (state == kRenderPolygonOffsetPoint)
		|| (state == kRenderPolygonOffsetLine)
		|| (state == kRenderPolygonOffsetFill)
		|| (state == kRenderDepthTest)
		|| (state == kRenderSecondaryColor)
		|| (state == kRenderBackfacingColor)
		|| (state == kRenderPolygonStipple)
		|| (state == kRenderPixelCounting)
		|| (state == kRenderDepthCullReports)
		|| (state == kRenderPolygonSmooth)
		|| (state == kRenderCullFace)
		|| (state == kRenderPrimitiveRestart)
		|| (state == kRenderLineStipple)
		|| (state == kRenderVertexProgramPointSize)
		|| (state == kRenderGammaCorrectedWrites)
		|| (state == kRenderBlend)
		, ("%i", state));

	m_cmdptr[0] = CMD(state, 4);
	m_cmdptr[1] = enable;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetDepthMinMaxControl(bool cullNearFarEnable, bool clampEnable, bool cullIgnoreW)
{
	ICE_ASSERT((cullNearFarEnable == 1) || (cullNearFarEnable == 0));
	ICE_ASSERT((clampEnable == 1) || (clampEnable == 0));
	ICE_ASSERT((cullIgnoreW == 1) || (cullIgnoreW == 0));
	
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdDepthClamp, 4);
	m_cmdptr[1] = cullNearFarEnable | (clampEnable << 4) | (cullIgnoreW<<8);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetDrawBufferMask(U32 mask)
{
	ICE_ASSERTF((mask & ~0xF) == 0, ("%x", mask));

	CHECK_RESERVE(2);
	
	if (mask > 1) mask |= 0x10;
	
	m_cmdptr[0] = CMD(kCmdDrawBufferMask, 4);
	m_cmdptr[1] = mask;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetAlphaFunc(ComparisonFunc func, U32 ref)
{
	ICE_ASSERTF((func == kFuncNever)
		|| (func == kFuncLess)
		|| (func == kFuncEqual)
		|| (func == kFuncLessEqual)
		|| (func == kFuncGreater)
		|| (func == kFuncNotEqual)
		|| (func == kFuncGreaterEqual)
		|| (func == kFuncAlways)
		, ("%i", func));
		
	ICE_ASSERTF((ref & ~0xFF) == 0, ("%x", ref));
	
	CHECK_RESERVE(3);

	m_cmdptr[0] = CMD(kCmdAlphaFunction, 8);
	m_cmdptr[1] = func;
	m_cmdptr[2] = ref;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetMrtBlendEnable(U32 mask)
{
	CHECK_RESERVE(4);

	m_cmdptr[0] = CMD(kCmdBlendEnable, 4);
	m_cmdptr[1] = mask & 0x1;
	m_cmdptr[2] = CMD(kCmdMRTBlendEnable, 4);
	m_cmdptr[3] = mask & 0xE;
	m_cmdptr += 4;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetBlendFunc(BlendFactor src, BlendFactor dst)
{
	ICE_ASSERTF((src == kBlendZero)
		|| (src == kBlendOne)
		|| (src == kBlendSrcColor)
		|| (src == kBlendOneMinusSrcColor)
		|| (src == kBlendSrcAlpha)
		|| (src == kBlendOneMinusSrcAlpha)
		|| (src == kBlendDstAlpha)
		|| (src == kBlendOneMinusDstAlpha)
		|| (src == kBlendDstColor)
		|| (src == kBlendOneMinusDstColor)
		|| (src == kBlendSrcAlphaSaturate)
		|| (src == kBlendConstantColor)
		|| (src == kBlendOneMinusConstantColor)
		|| (src == kBlendConstantAlpha)
		|| (src == kBlendOneMinusConstantAlpha)
		, ("%i", src));
	
	ICE_ASSERTF((dst == kBlendZero)
		|| (dst == kBlendOne)
		|| (dst == kBlendSrcColor)
		|| (dst == kBlendOneMinusSrcColor)
		|| (dst == kBlendSrcAlpha)
		|| (dst == kBlendOneMinusSrcAlpha)
		|| (dst == kBlendDstAlpha)
		|| (dst == kBlendOneMinusDstAlpha)
		|| (dst == kBlendDstColor)
		|| (dst == kBlendOneMinusDstColor)
		|| (dst == kBlendConstantColor)
		|| (dst == kBlendOneMinusConstantColor)
		|| (dst == kBlendConstantAlpha)
		|| (dst == kBlendOneMinusConstantAlpha)
		, ("%i", dst));

	CHECK_RESERVE(3);

	m_cmdptr[0] = CMD(kCmdBlendSrcFactor, 8);
	m_cmdptr[1] = src | (src << 16);
	m_cmdptr[2] = dst | (dst << 16);
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha)
{
	ICE_ASSERTF((srcRGB == kBlendZero)
		|| (srcRGB == kBlendOne)
		|| (srcRGB == kBlendSrcColor)
		|| (srcRGB == kBlendOneMinusSrcColor)
		|| (srcRGB == kBlendSrcAlpha)
		|| (srcRGB == kBlendOneMinusSrcAlpha)
		|| (srcRGB == kBlendDstAlpha)
		|| (srcRGB == kBlendOneMinusDstAlpha)
		|| (srcRGB == kBlendDstColor)
		|| (srcRGB == kBlendOneMinusDstColor)
		|| (srcRGB == kBlendSrcAlphaSaturate)
		|| (srcRGB == kBlendConstantColor)
		|| (srcRGB == kBlendOneMinusConstantColor)
		|| (srcRGB == kBlendConstantAlpha)
		|| (srcRGB == kBlendOneMinusConstantAlpha)
		, ("%i", srcRGB));
	
	ICE_ASSERTF((dstRGB == kBlendZero)
		|| (dstRGB == kBlendOne)
		|| (dstRGB == kBlendSrcColor)
		|| (dstRGB == kBlendOneMinusSrcColor)
		|| (dstRGB == kBlendSrcAlpha)
		|| (dstRGB == kBlendOneMinusSrcAlpha)
		|| (dstRGB == kBlendDstAlpha)
		|| (dstRGB == kBlendOneMinusDstAlpha)
		|| (dstRGB == kBlendDstColor)
		|| (dstRGB == kBlendOneMinusDstColor)
		|| (dstRGB == kBlendConstantColor)
		|| (dstRGB == kBlendOneMinusConstantColor)
		|| (dstRGB == kBlendConstantAlpha)
		|| (dstRGB == kBlendOneMinusConstantAlpha)
		, ("%i", dstRGB));

	ICE_ASSERTF((srcAlpha == kBlendZero)
		|| (srcAlpha == kBlendOne)
		|| (srcAlpha == kBlendSrcColor)
		|| (srcAlpha == kBlendOneMinusSrcColor)
		|| (srcAlpha == kBlendSrcAlpha)
		|| (srcAlpha == kBlendOneMinusSrcAlpha)
		|| (srcAlpha == kBlendDstAlpha)
		|| (srcAlpha == kBlendOneMinusDstAlpha)
		|| (srcAlpha == kBlendDstColor)
		|| (srcAlpha == kBlendOneMinusDstColor)
		|| (srcAlpha == kBlendSrcAlphaSaturate)
		|| (srcAlpha == kBlendConstantColor)
		|| (srcAlpha == kBlendOneMinusConstantColor)
		|| (srcAlpha == kBlendConstantAlpha)
		|| (srcAlpha == kBlendOneMinusConstantAlpha)
		, ("%i", srcAlpha));
	
	ICE_ASSERTF((dstAlpha == kBlendZero)
		|| (dstAlpha == kBlendOne)
		|| (dstAlpha == kBlendSrcColor)
		|| (dstAlpha == kBlendOneMinusSrcColor)
		|| (dstAlpha == kBlendSrcAlpha)
		|| (dstAlpha == kBlendOneMinusSrcAlpha)
		|| (dstAlpha == kBlendDstAlpha)
		|| (dstAlpha == kBlendOneMinusDstAlpha)
		|| (dstAlpha == kBlendDstColor)
		|| (dstAlpha == kBlendOneMinusDstColor)
		|| (dstAlpha == kBlendConstantColor)
		|| (dstAlpha == kBlendOneMinusConstantColor)
		|| (dstAlpha == kBlendConstantAlpha)
		|| (dstAlpha == kBlendOneMinusConstantAlpha)
		, ("%i", dstAlpha));

	CHECK_RESERVE(3);
		
	m_cmdptr[0] = CMD(kCmdBlendSrcFactor, 8);
	m_cmdptr[1] = srcRGB | (srcAlpha << 16);
	m_cmdptr[2] = dstRGB | (dstAlpha << 16);
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetBlendEquation(BlendEquation equation)
{
	ICE_ASSERTF((equation == kBlendEquationAdd)
		|| (equation == kBlendEquationMin)
		|| (equation == kBlendEquationMax)
		|| (equation == kBlendEquationSubtract)
		|| (equation == kBlendEquationReverseSubtract)
		|| (equation == kBlendEquationSignedReverseSubtract)
		|| (equation == kBlendEquationSignedAdd)
		|| (equation == kBlendEquationSignedSubtract)
		, ("%i", equation));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdBlendEquation, 4);
	m_cmdptr[1] = equation | (equation << 16);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetBlendEquationSeparate(BlendEquation equationRGB, BlendEquation equationAlpha)
{
	ICE_ASSERTF((equationRGB == kBlendEquationAdd)
		|| (equationRGB == kBlendEquationMin)
		|| (equationRGB == kBlendEquationMax)
		|| (equationRGB == kBlendEquationSubtract)
		|| (equationRGB == kBlendEquationReverseSubtract)
		|| (equationRGB == kBlendEquationSignedReverseSubtract)
		|| (equationRGB == kBlendEquationSignedAdd)
		|| (equationRGB == kBlendEquationSignedSubtract)
		, ("%i", equationRGB));

	ICE_ASSERTF((equationAlpha == kBlendEquationAdd)
		|| (equationAlpha == kBlendEquationMin)
		|| (equationAlpha == kBlendEquationMax)
		|| (equationAlpha == kBlendEquationSubtract)
		|| (equationAlpha == kBlendEquationReverseSubtract)
		|| (equationAlpha == kBlendEquationSignedReverseSubtract)
		|| (equationAlpha == kBlendEquationSignedAdd)
		|| (equationAlpha == kBlendEquationSignedSubtract)
		, ("%i", equationAlpha));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdBlendEquation, 4);
	m_cmdptr[1] = equationRGB | (equationAlpha << 16);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetBlendColor(ArgbColor color)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdBlendColor, 4);
	m_cmdptr[1] = color;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetFloatBlendColor(half red, half green, half blue, half alpha)
{
	CHECK_RESERVE(4);
	
	m_cmdptr[0] = CMD(kCmdBlendColor, 4);
	m_cmdptr[1] = (green << 16) | red;
	m_cmdptr[2] = CMD(kCmdFloatingPointBlendColor, 4);
	m_cmdptr[3] = (alpha << 16) | blue;
	m_cmdptr += 4;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetColorMask(bool red, bool green, bool blue, bool alpha)
{
	ICE_ASSERT((red == 0) || (red == 1));
	ICE_ASSERT((green == 0) || (green == 1));
	ICE_ASSERT((blue == 0) || (blue == 1));
	ICE_ASSERT((alpha == 0) || (alpha == 1));

	CHECK_RESERVE(4);
	
	m_cmdptr[0] = CMD(kCmdColorMask, 4);
	m_cmdptr[1] = ((U32) alpha << 24) | ((U32) red << 16) | ((U32) green << 8) | ((U32) blue);
	
	m_cmdptr[2] = CMD(kCmdMRTColorMask, 4);
	U32 mask = ((U32) blue << 7) | ((U32) green << 6) | ((U32) red << 5) | ((U32) alpha << 4);
	m_cmdptr[3] = (mask << 8) | (mask << 4) | mask;
	
	m_cmdptr += 4;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetMrtColorMask(U16 mask)
{
	CHECK_RESERVE(4);
	
	m_cmdptr[0] = CMD(kCmdColorMask, 4);
	m_cmdptr[1] = ((U32)(mask&1) << 24) | ((U32)(mask&2) << 15) | ((U32)(mask&4) << 6) | ((U32)(mask&8) >> 3);
	m_cmdptr[2] = CMD(kCmdMRTColorMask, 4);
	m_cmdptr[3] = mask & 0xFFF0;
	m_cmdptr += 4;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetDepthMask(bool mask)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdDepthMask, 4);
	m_cmdptr[1] = mask;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetStencilMask(U32 mask)
{
	ICE_ASSERTF((mask & ~0xFF) == 0, ("%x", mask));

	CHECK_RESERVE(4);
	
	m_cmdptr[0] = CMD(kCmdFrontStencilMask, 4);
	m_cmdptr[1] = mask;
	m_cmdptr[2] = CMD(kCmdBackStencilMask, 4);
	m_cmdptr[3] = mask;
	m_cmdptr += 4;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetStencilMaskSeparate(U32 maskFront, U32 maskBack)
{
	ICE_ASSERTF((maskFront & ~0xFF) == 0, ("%x", maskFront));
	ICE_ASSERTF((maskBack & ~0xFF) == 0, ("%x", maskBack));

	CHECK_RESERVE(4);
	
	m_cmdptr[0] = CMD(kCmdFrontStencilMask, 4);
	m_cmdptr[1] = maskFront;
	m_cmdptr[2] = CMD(kCmdBackStencilMask, 4);
	m_cmdptr[3] = maskBack;
	m_cmdptr += 4;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetStencilFunc(ComparisonFunc func, U32 ref, U32 mask)
{
	ICE_ASSERTF((func == kFuncNever)
		|| (func == kFuncLess)
		|| (func == kFuncEqual)
		|| (func == kFuncLessEqual)
		|| (func == kFuncGreater)
		|| (func == kFuncNotEqual)
		|| (func == kFuncGreaterEqual)
		|| (func == kFuncAlways)
		, ("%i", func));
		
	ICE_ASSERTF((ref & ~0xFF) == 0, ("%x", ref));
	ICE_ASSERTF((mask & ~0xFF) == 0, ("%x", mask));

	CHECK_RESERVE(8);

	m_cmdptr[0] = CMD(kCmdFrontStencilFunction, 12);
	m_cmdptr[1] = func;
	m_cmdptr[2] = ref;
	m_cmdptr[3] = mask;
	m_cmdptr[4] = CMD(kCmdBackStencilFunction, 12);
	m_cmdptr[5] = func;
	m_cmdptr[6] = ref;
	m_cmdptr[7] = mask;
	m_cmdptr += 8;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetStencilFuncSeparate(ComparisonFunc funcFront, U32 refFront, U32 maskFront, ComparisonFunc funcBack, U32 refBack, U32 maskBack)
{
	ICE_ASSERTF((funcFront == kFuncNever)
		|| (funcFront == kFuncLess)
		|| (funcFront == kFuncEqual)
		|| (funcFront == kFuncLessEqual)
		|| (funcFront == kFuncGreater)
		|| (funcFront == kFuncNotEqual)
		|| (funcFront == kFuncGreaterEqual)
		|| (funcFront == kFuncAlways)
		, ("%i", funcFront));
		
	ICE_ASSERTF((refFront & ~0xFF) == 0, ("%x", refFront));
	ICE_ASSERTF((maskFront & ~0xFF) == 0, ("%x", maskFront));

	ICE_ASSERTF((funcBack == kFuncNever)
		|| (funcBack == kFuncLess)
		|| (funcBack == kFuncEqual)
		|| (funcBack == kFuncLessEqual)
		|| (funcBack == kFuncGreater)
		|| (funcBack == kFuncNotEqual)
		|| (funcBack == kFuncGreaterEqual)
		|| (funcBack == kFuncAlways)
		, ("%i", funcBack));
		
	ICE_ASSERTF((refBack & ~0xFF) == 0, ("%x", refBack));
	ICE_ASSERTF((maskBack & ~0xFF) == 0, ("%x", maskBack));

	CHECK_RESERVE(8);

	m_cmdptr[0] = CMD(kCmdFrontStencilFunction, 12);
	m_cmdptr[1] = funcFront;
	m_cmdptr[2] = refFront;
	m_cmdptr[3] = maskFront;
	m_cmdptr[4] = CMD(kCmdBackStencilFunction, 12);
	m_cmdptr[5] = funcBack;
	m_cmdptr[6] = refBack;
	m_cmdptr[7] = maskBack;
	m_cmdptr += 8;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetStencilOp(StencilOp sfail, StencilOp dfail, StencilOp dpass)
{
	ICE_ASSERTF((sfail == kStencilZero)
		|| (sfail == kStencilKeep)
		|| (sfail == kStencilReplace)
		|| (sfail == kStencilIncrement)
		|| (sfail == kStencilDecrement)
		|| (sfail == kStencilInvert)
		|| (sfail == kStencilIncrementWrap)
		|| (sfail == kStencilDecrementWrap)
		, ("%i", sfail));

	ICE_ASSERTF((dfail == kStencilZero)
		|| (dfail == kStencilKeep)
		|| (dfail == kStencilReplace)
		|| (dfail == kStencilIncrement)
		|| (dfail == kStencilDecrement)
		|| (dfail == kStencilInvert)
		|| (dfail == kStencilIncrementWrap)
		|| (dfail == kStencilDecrementWrap)
		, ("%i", dfail));

	ICE_ASSERTF((dpass == kStencilZero)
		|| (dpass == kStencilKeep)
		|| (dpass == kStencilReplace)
		|| (dpass == kStencilIncrement)
		|| (dpass == kStencilDecrement)
		|| (dpass == kStencilInvert)
		|| (dpass == kStencilIncrementWrap)
		|| (dpass == kStencilDecrementWrap)
		, ("%i", dpass));

	CHECK_RESERVE(8);

	m_cmdptr[0] = CMD(kCmdFrontStencilFailOp, 12);
	m_cmdptr[1] = sfail;
	m_cmdptr[2] = dfail;
	m_cmdptr[3] = dpass;
	m_cmdptr[4] = CMD(kCmdBackStencilFailOp, 12);
	m_cmdptr[5] = sfail;
	m_cmdptr[6] = dfail;
	m_cmdptr[7] = dpass;
	m_cmdptr += 8;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetStencilOpSeparate(StencilOp sfailFront, StencilOp dfailFront, StencilOp dpassFront, StencilOp sfailBack, StencilOp dfailBack, StencilOp dpassBack)
{
	ICE_ASSERTF((sfailFront == kStencilZero)
		|| (sfailFront == kStencilKeep)
		|| (sfailFront == kStencilReplace)
		|| (sfailFront == kStencilIncrement)
		|| (sfailFront == kStencilDecrement)
		|| (sfailFront == kStencilInvert)
		|| (sfailFront == kStencilIncrementWrap)
		|| (sfailFront == kStencilDecrementWrap)
		, ("%i", sfailFront));

	ICE_ASSERTF((dfailFront == kStencilZero)
		|| (dfailFront == kStencilKeep)
		|| (dfailFront == kStencilReplace)
		|| (dfailFront == kStencilIncrement)
		|| (dfailFront == kStencilDecrement)
		|| (dfailFront == kStencilInvert)
		|| (dfailFront == kStencilIncrementWrap)
		|| (dfailFront == kStencilDecrementWrap)
		, ("%i", dfailFront));

	ICE_ASSERTF((dpassFront == kStencilZero)
		|| (dpassFront == kStencilKeep)
		|| (dpassFront == kStencilReplace)
		|| (dpassFront == kStencilIncrement)
		|| (dpassFront == kStencilDecrement)
		|| (dpassFront == kStencilInvert)
		|| (dpassFront == kStencilIncrementWrap)
		|| (dpassFront == kStencilDecrementWrap)
		, ("%i", dpassFront));
	
	ICE_ASSERTF((sfailBack == kStencilZero)
		|| (sfailBack == kStencilKeep)
		|| (sfailBack == kStencilReplace)
		|| (sfailBack == kStencilIncrement)
		|| (sfailBack == kStencilDecrement)
		|| (sfailBack == kStencilInvert)
		|| (sfailBack == kStencilIncrementWrap)
		|| (sfailBack == kStencilDecrementWrap)
		, ("%i", sfailBack));

	ICE_ASSERTF((dfailBack == kStencilZero)
		|| (dfailBack == kStencilKeep)
		|| (dfailBack == kStencilReplace)
		|| (dfailBack == kStencilIncrement)
		|| (dfailBack == kStencilDecrement)
		|| (dfailBack == kStencilInvert)
		|| (dfailBack == kStencilIncrementWrap)
		|| (dfailBack == kStencilDecrementWrap)
		, ("%i", dfailBack));

	ICE_ASSERTF((dpassBack == kStencilZero)
		|| (dpassBack == kStencilKeep)
		|| (dpassBack == kStencilReplace)
		|| (dpassBack == kStencilIncrement)
		|| (dpassBack == kStencilDecrement)
		|| (dpassBack == kStencilInvert)
		|| (dpassBack == kStencilIncrementWrap)
		|| (dpassBack == kStencilDecrementWrap)
		, ("%i", dpassBack));

	CHECK_RESERVE(8);

	m_cmdptr[0] = CMD(kCmdFrontStencilFailOp, 12);
	m_cmdptr[1] = sfailFront;
	m_cmdptr[2] = dfailFront;
	m_cmdptr[3] = dpassFront;
	m_cmdptr[4] = CMD(kCmdBackStencilFailOp, 12);
	m_cmdptr[5] = sfailBack;
	m_cmdptr[6] = dfailBack;
	m_cmdptr[7] = dpassBack;
	m_cmdptr += 8;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetShadeModel(ShadeModel model)
{
	ICE_ASSERTF((model == kShadeModelFlat)
		|| (model == kShadeModelSmooth)
		, ("%i", model));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdShadeModel, 4);
	m_cmdptr[1] = model;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetLogicOp(LogicOp op)
{
	ICE_ASSERTF((op == kLogicClear)
		|| (op == kLogicAnd)
		|| (op == kLogicAndReverse)
		|| (op == kLogicCopy)
		|| (op == kLogicAndInverted)
		|| (op == kLogicNop)
		|| (op == kLogicXor)
		|| (op == kLogicOr)
		|| (op == kLogicNor)
		|| (op == kLogicEquiv)
		|| (op == kLogicInvert)
		|| (op == kLogicOrReverse)
		|| (op == kLogicCopyInverted)
		|| (op == kLogicOrInverted)
		|| (op == kLogicNand)
		|| (op == kLogicSet)
		, ("%i", op));
	
	CHECK_RESERVE(2);

	m_cmdptr[0] = CMD(kCmdLogicOp, 4);
	m_cmdptr[1] = op;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetFogMode(FogMode mode)
{
	ICE_ASSERTF((mode == kFogLinear)
		|| (mode == kFogExp)
		|| (mode == kFogExp2)
		, ("%i", mode));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdFogMode, 4);
	m_cmdptr[1] = mode;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetFogRange(float fmin, float fmax)
{
	CHECK_RESERVE(3);
	
	float scale = 1.0F / (fmax - fmin);
	
	m_cmdptr[0] = CMD(kCmdFogBias, 8);
	*(float *) &m_cmdptr[1] = fmax * scale + 1.0F;
	*(float *) &m_cmdptr[2] = -scale;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetFogDensity(float density)
{
	CHECK_RESERVE(3);
	
	m_cmdptr[0] = CMD(kCmdFogBias, 8);
	*(float *) &m_cmdptr[1] = 1.5F;
	*(float *) &m_cmdptr[2] = -0.0901681F * density;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetDepthBounds(float dmin, float dmax)
{
	CHECK_RESERVE(3);
	
	m_cmdptr[0] = CMD(kCmdDepthBoundsMin, 8);
	*(float *) &m_cmdptr[1] = dmin;
	*(float *) &m_cmdptr[2] = dmax;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetViewport(U32 left, U32 top, U32 width, U32 height, float dmin, float dmax)
{
	CHECK_RESERVE(15);
	
	m_cmdptr[0] = CMD(kCmdViewportX, 8);
	m_cmdptr[1] = (width << 16) | left;
	m_cmdptr[2] = (height << 16) | top;
	
	m_cmdptr[3] = CMD(kCmdDepthRangeMin, 8);
	*(float *) &m_cmdptr[4] = dmin;
	*(float *) &m_cmdptr[5] = dmax;
	
	float w = (float) width * 0.5f;
	float h = (float) height * 0.5f;
	float d = (dmax - dmin) * 0.5f;
	
	float x = (float) left + w;
	float y = (float) top + h;
	float z = (dmax + dmin) * 0.5f;
	
	m_cmdptr[6] = CMD(kCmdViewportTranslate, 32);
	*(float *) &m_cmdptr[7] = x;
	*(float *) &m_cmdptr[8] = y;
	*(float *) &m_cmdptr[9] = z;
	m_cmdptr[10] = 0;
	*(float *) &m_cmdptr[11] = w;
	*(float *) &m_cmdptr[12] = -h;
	*(float *) &m_cmdptr[13] = d;
	m_cmdptr[14] = 0;
	
	m_cmdptr += 15;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetViewport(U32 left, U32 top, U32 width, U32 height, float dmin, float dmax, float scale[4], float bias[4])
{
	CHECK_RESERVE(15);
	
	m_cmdptr[0] = CMD(kCmdViewportX, 8);
	m_cmdptr[1] = (width << 16) | left;
	m_cmdptr[2] = (height << 16) | top;
	
	m_cmdptr[3] = CMD(kCmdDepthRangeMin, 8);
	*(float *) &m_cmdptr[4] = dmin;
	*(float *) &m_cmdptr[5] = dmax;
	
	U32 b0 = F32ToU32(bias[0]);
	m_cmdptr[6] = CMD(kCmdViewportTranslate, 32);
	U32 b1 = F32ToU32(bias[1]);
	m_cmdptr[7] = b0;
	U32 b2 = F32ToU32(bias[2]);
	m_cmdptr[8] = b1;
	U32 b3 = F32ToU32(bias[3]);
	m_cmdptr[9] = b2;
	U32 s0 = F32ToU32(scale[0]);
	m_cmdptr[10] = b3;
	U32 s1 = F32ToU32(scale[1]);
	m_cmdptr[11] = s0;
	U32 s2 = F32ToU32(scale[2]);
	m_cmdptr[12] = s1;
	U32 s3 = F32ToU32(scale[3]);
	m_cmdptr[13] = s2;
	m_cmdptr[14] = s3;
	
	m_cmdptr += 15;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetScissor(U32 left, U32 top, U32 width, U32 height)
{
	CHECK_RESERVE(3);
	
	m_cmdptr[0] = CMD(kCmdScissorX, 8);
	m_cmdptr[1] = (width << 16) | left;
	m_cmdptr[2] = (height << 16) | top;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetDepthFunc(ComparisonFunc func)
{
	CHECK_RESERVE(2);
	
	ICE_ASSERTF((func == kFuncNever)
		|| (func == kFuncLess)
		|| (func == kFuncEqual)
		|| (func == kFuncLessEqual)
		|| (func == kFuncGreater)
		|| (func == kFuncNotEqual)
		|| (func == kFuncGreaterEqual)
		|| (func == kFuncAlways)
		, ("%i", func));

	m_cmdptr[0] = CMD(kCmdDepthFunction, 4);
	m_cmdptr[1] = func;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetPolygonOffset(float factor, float units)
{
	CHECK_RESERVE(3);
	
	m_cmdptr[0] = CMD(kCmdPolygonOffsetFactor, 8);
	*(float *) &m_cmdptr[1] = factor;
	*(float *) &m_cmdptr[2] = units + units;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetPolygonStipplePattern(const void *pattern)
{
	CHECK_RESERVE(33);
	
	m_cmdptr[0] = CMD(kCmdPolygonStipplePattern, 128);
	
	const U32 *data = static_cast<const U32 *>(pattern) - 1;
	for (U32F i = 1; i <= 32; ++i)
		m_cmdptr[i] = data[i];
	
	m_cmdptr += 33;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetLineStipplePattern(U32 factor, U32 pattern)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdLineStipplePattern, 4);
	factor = 31 - Cntlzw(factor);
	m_cmdptr[1] = Rlwimi(factor, pattern, 16, 0, 15);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetLineWidth(U32 width)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdLineWidth, 4);
	m_cmdptr[1] = width;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetClipFunc(ClipFunc func0, ClipFunc func1, ClipFunc func2, ClipFunc func3, ClipFunc func4, ClipFunc func5)
{
	ICE_ASSERTF((func0 == kClipNone)
		|| (func0 == kClipLessZero)
		|| (func0 == kClipGreaterEqualZero)
		, ("%i", func0));
	
	ICE_ASSERTF((func1 == kClipNone)
		|| (func1 == kClipLessZero)
		|| (func1 == kClipGreaterEqualZero)
		, ("%i", func1));

	ICE_ASSERTF((func2 == kClipNone)
		|| (func2 == kClipLessZero)
		|| (func2 == kClipGreaterEqualZero)
		, ("%i", func2));

	ICE_ASSERTF((func3 == kClipNone)
		|| (func3 == kClipLessZero)
		|| (func3 == kClipGreaterEqualZero)
		, ("%i", func3));

	ICE_ASSERTF((func4 == kClipNone)
		|| (func4 == kClipLessZero)
		|| (func4 == kClipGreaterEqualZero)
		, ("%i", func4));

	ICE_ASSERTF((func5 == kClipNone)
		|| (func5 == kClipLessZero)
		|| (func5 == kClipGreaterEqualZero)
		, ("%i", func5));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdClipPlaneControl, 4);
	m_cmdptr[1] = func0 | (func1 << 4) | (func2 << 8) | (func3 << 12) | (func4 << 16) | (func5 << 20);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetPrimitiveRestartIndex(U32 index)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdPrimitiveRestartIndex, 4);
	m_cmdptr[1] = index;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetPolygonMode(PolygonMode mode)
{
	ICE_ASSERTF((mode == kPolygonModePoint)
		|| (mode == kPolygonModeLine)
		|| (mode == kPolygonModeFill)
		, ("%i", mode));
	
	CHECK_RESERVE(3);
	
	m_cmdptr[0] = CMD(kCmdFrontPolygonMode, 8);
	m_cmdptr[1] = mode;
	m_cmdptr[2] = mode;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetPolygonModeSeparate(PolygonMode modeFront, PolygonMode modeBack)
{
	ICE_ASSERTF((modeFront == kPolygonModePoint)
		|| (modeFront == kPolygonModeLine)
		|| (modeFront == kPolygonModeFill)
		, ("%i", modeFront));

	ICE_ASSERTF((modeBack == kPolygonModePoint)
		|| (modeBack == kPolygonModeLine)
		|| (modeBack == kPolygonModeFill)
		, ("%i", modeBack));

	CHECK_RESERVE(3);
	
	m_cmdptr[0] = CMD(kCmdFrontPolygonMode, 8);
	m_cmdptr[1] = modeFront;
	m_cmdptr[2] = modeBack;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetCullFace(CullFace cull)
{
	ICE_ASSERTF((cull == kCullFaceFront)
		|| (cull == kCullFaceBack)
		|| (cull == kCullFaceFrontAndBack)
		, ("%i", cull));
	
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdCullFace, 4);
	m_cmdptr[1] = cull;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetFrontFace(FrontFace front)
{
	ICE_ASSERTF((front == kFrontFaceCw)
		|| (front == kFrontFaceCcw)
		, ("%i", front));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdFrontFace, 4);
	m_cmdptr[1] = front;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetClearColor(BgraColor color)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdClearColor, 4);
	m_cmdptr[1] = color;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetClearDepthStencil(U32 value)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdClearDepthStencil, 4);
	m_cmdptr[1] = value;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::Clear(U32 flags)
{
	CHECK_RESERVE(2);

	m_cmdptr[0] = CMD(kCmdClear, 4);
	m_cmdptr[1] = flags;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetMultisampleParameters(bool enabled, bool alphaToCoverage, bool alphaToOne, U16 mask)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdMultisampleControl, 4);
	m_cmdptr[1] = (mask << 16) | (U32) enabled | ((U32) alphaToCoverage << 4) | ((U32) alphaToOne << 8);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetPointSize(float psize)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdPointSize, 4);
	*(float *) &m_cmdptr[1] = psize;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetPointSpriteParameters(bool enabled, U32 texcoordMask, SpriteMode mode)
{
	ICE_ASSERTF((mode == kSpriteZero)
		|| (mode == kSpriteR)
		|| (mode == kSpriteS)
		, ("%i", mode));
		
	ICE_ASSERTF((texcoordMask & ~0xFF) == 0, ("%x", texcoordMask));
	
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdPointSpriteControl, 4);
	m_cmdptr[1] = (U32) enabled | (texcoordMask << 8) | (mode << 1);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttribPointer(U32 index, U32 offset, AttribContext context)
{
	ICE_ASSERTF(index <= 15, ("%i", index));
	ICE_ASSERTF((context == kAttribVideoMemory)
		|| (context == kAttribMainMemory)
		, ("%i", context));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdVertexAttribAddress + index * 4, 4);
	m_cmdptr[1] = offset | context;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttribBaseOffset(U32 baseOffset)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdVertexAttribAddressOffset, 4);
	m_cmdptr[1] = baseOffset;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttribFormat(U32 index, AttribType type, AttribCount count, U32 stride, U32 divider)
{
	ICE_ASSERTF(index <= 15, ("%i", index));
	ICE_ASSERTF((type == kAttribSignedShortNormalized)
		|| (type == kAttribFloat)
		|| (type == kAttribHalfFloat)
		|| (type == kAttribUnsignedByteNormalized)
		|| (type == kAttribSignedShort)
		|| (type == kAttribX11Y11Z10Normalized)
		|| (type == kAttribUnsignedByte)
		, ("%i", type));
	ICE_ASSERTF((count == kAttribDisable)
		|| (count == kAttribCount1)
		|| (count == kAttribCount2)
		|| (count == kAttribCount3)
		|| (count == kAttribCount4)
		, ("%i", count));
	ICE_ASSERTF(!((type == kAttribX11Y11Z10Normalized) && (count != kAttribCount1)), ("111110 format requires a count of 1, even though it has 3 components."));
	ICE_ASSERTF(!((type == kAttribUnsignedByte) && (count != kAttribCount4)), ("Unsigned byte format requires a count of 4."));
	
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdVertexAttribFormat + index * 4, 4);
	m_cmdptr[1] = (divider << 16) | (stride << 8) | (count << 4) | type;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttribFrequencyMode(U32 mode)
{
	ICE_ASSERTF((mode & ~0xFFFF) == 0, ("%x", mode));
	
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdVertexFrequencyControl, 4);
	m_cmdptr[1] = mode;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::DisableVertexAttribArray(U32 index)
{
	ICE_ASSERTF(index <= 15, ("%i", index));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdVertexAttribFormat + index * 4, 4);
	m_cmdptr[1] = 0x0002;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib1f(U32 index, float x)
{
	ICE_ASSERTF(index <= 15, ("%i", index));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdVertexAttrib1f + index * 4, 4);
	*(float *) &m_cmdptr[1] = x;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib1f(U32 index, const float *data)
{
	ICE_ASSERTF(index <= 15, ("%i", index));
	
	CHECK_RESERVE(2);

	U32 d0 = F32ToU32(data[0]);
	m_cmdptr[0] = CMD(kCmdVertexAttrib1f + index * 4, 4);
	m_cmdptr[1] = d0;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib2f(U32 index, float x, float y)
{
	ICE_ASSERTF(index <= 15, ("%i", index));

	CHECK_RESERVE(3);
	
	m_cmdptr[0] = CMD(kCmdVertexAttrib2f + index * 8, 8);
	*(float *) &m_cmdptr[1] = x;
	*(float *) &m_cmdptr[2] = y;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib2f(U32 index, const float *data)
{
	ICE_ASSERTF(index <= 15, ("%i", index));

	CHECK_RESERVE(3);

	U32 d0 = F32ToU32(data[0]);
	U32 d1 = F32ToU32(data[1]);
	m_cmdptr[0] = CMD(kCmdVertexAttrib2f + index * 8, 8);
	m_cmdptr[1] = d0;
	m_cmdptr[2] = d1;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib3f(U32 index, float x, float y, float z)
{
	ICE_ASSERTF(index <= 15, ("%i", index));
	
	CHECK_RESERVE(4);
	
	m_cmdptr[0] = CMD(kCmdVertexAttrib3f + index * 16, 12);
	*(float *) &m_cmdptr[1] = x;
	*(float *) &m_cmdptr[2] = y;
	*(float *) &m_cmdptr[3] = z;
	m_cmdptr += 4;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib3f(U32 index, const float *data)
{
	ICE_ASSERTF(index <= 15, ("%i", index));
	
	CHECK_RESERVE(4);

	U32 d0 = F32ToU32(data[0]);
	U32 d1 = F32ToU32(data[1]);
	m_cmdptr[0] = CMD(kCmdVertexAttrib3f + index * 16, 12);
	U32 d2 = F32ToU32(data[2]);
	m_cmdptr[1] = d0;
	m_cmdptr[2] = d1;
	m_cmdptr[3] = d2;
	m_cmdptr += 4;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib4f(U32 index, float x, float y, float z, float w)
{
	ICE_ASSERTF(index <= 15, ("%i", index));
	
	CHECK_RESERVE(5);
	
	m_cmdptr[0] = CMD(kCmdVertexAttrib4f + index * 16, 16);
	*(float *) &m_cmdptr[1] = x;
	*(float *) &m_cmdptr[2] = y;
	*(float *) &m_cmdptr[3] = z;
	*(float *) &m_cmdptr[4] = w;
	m_cmdptr += 5;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib4f(U32 index, const float *data)
{
	ICE_ASSERTF(index <= 15, ("%i", index));
	
	CHECK_RESERVE(5);
	
	U32 d0 = F32ToU32(data[0]);
	U32 d1 = F32ToU32(data[1]);
	m_cmdptr[0] = CMD(kCmdVertexAttrib4f + index * 16, 16);
	U32 d2 = F32ToU32(data[2]);
	m_cmdptr[1] = d0;
	U32 d3 = F32ToU32(data[3]);
	m_cmdptr[2] = d1;
	m_cmdptr[3] = d2;
	m_cmdptr[4] = d3;
	m_cmdptr += 5;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib2s(U32 index, I16 x, I16 y)
{
	ICE_ASSERTF(index <= 15, ("%i", index));
	
	CHECK_RESERVE(2);
	
	I32 x0 = x;		// Prevents sign extension after Rlwimi
	
	m_cmdptr[0] = CMD(kCmdVertexAttrib2s + index * 4, 4);
	m_cmdptr[1] = Rlwimi(x0, y, 16, 0, 15);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib4s(U32 index, I16 x, I16 y, I16 z, I16 w)
{
	ICE_ASSERTF(index <= 15, ("%i", index));
	
	CHECK_RESERVE(3);

	I32 x0 = x;		// Prevents sign extension after Rlwimi
	I32 z0 = z;
	
	m_cmdptr[0] = CMD(kCmdVertexAttrib4s + index * 8, 8);
	m_cmdptr[1] = Rlwimi(x0, y, 16, 0, 15);
	m_cmdptr[2] = Rlwimi(z0, w, 16, 0, 15);
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib4Ns(U32 index, I16 x, I16 y, I16 z, I16 w)
{
	ICE_ASSERTF(index <= 15, ("%i", index));
	
	CHECK_RESERVE(3);

	I32 x0 = x;		// Prevents sign extension after Rlwimi
	I32 z0 = z;
	
	m_cmdptr[0] = CMD(kCmdVertexAttrib4Ns + index * 8, 8);
	m_cmdptr[1] = Rlwimi(x0, y, 16, 0, 15);
	m_cmdptr[2] = Rlwimi(z0, w, 16, 0, 15);
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib4Nub(U32 index, U8 x, U8 y, U8 z, U8 w)
{
	ICE_ASSERTF(index <= 15, ("%i", index));

	CHECK_RESERVE(2);

	U32F attrib = x;
	attrib = Rlwimi(attrib, y, 8, 16, 23);
	attrib = Rlwimi(attrib, z, 16, 8, 15);
	attrib = Rlwimi(attrib, w, 24, 0, 7);
	
	m_cmdptr[0] = CMD(kCmdVertexAttrib4Nub + index * 4, 4);
	m_cmdptr[1] = (U32) attrib;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexAttrib4Nub(U32 index, U32 xyzwData)
{
	CHECK_RESERVE(2);

	m_cmdptr[0] = CMD(kCmdVertexAttrib4Nub + index * 4, 4);
	m_cmdptr++;
#ifdef __SPU__
	m_cmdptr[0] = (xyzwData >> 24) | ((xyzwData >> 8) & 0xFF00) | ((xyzwData << 8) & 0xFF0000) | (xyzwData << 24);
#else
	__asm__("stwbrx %0,0,%1\n" :: "r"(xyzwData), "r"(m_cmdptr) : "memory");
#endif
	m_cmdptr++;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::DrawArrays(DrawMode mode, U32 start, U32 count)
{
	ICE_ASSERTF((mode == kDrawPoints)
		|| (mode == kDrawLines)
		|| (mode == kDrawLineLoop)
		|| (mode == kDrawLineStrip)
		|| (mode == kDrawTriangles)
		|| (mode == kDrawTriangleStrip)
		|| (mode == kDrawTriangleFan)
		|| (mode == kDrawQuads)
		|| (mode == kDrawQuadStrip)
		|| (mode == kDrawPolygon)
		, ("%i", mode));

	// If we are trying to render 0 vertexes, return
	if (count == 0) 
		return;
	
	// Issue begin command
	
	U32F blockCount = count >> 8;
	I32F remainCount = count & 0x000000FF;
	U32F hasRemainder = (remainCount != 0);
	U32F commandCount = (count + 0xFF) >> 8;
	ICE_ASSERT(commandCount < 2048);
	CHECK_RESERVE(5+commandCount);
	
	m_cmdptr[0] = CMD(kCmdDrawMode, 4);
	m_cmdptr[1] = mode;
	m_cmdptr[2] = CMD(kCmdDrawArrays, commandCount * 4) | kCmdNoIncFlag;
	m_cmdptr[commandCount+3] = CMD(kCmdDrawMode, 4);
	m_cmdptr[commandCount+4] = 0;
	U32 *__restrict cmd = m_cmdptr+3;
	m_cmdptr += commandCount+5;
	--remainCount;
	start |= 0xFF000000;

	if(blockCount != 0)
	{
		do 
		{
			*cmd++ = start;
			start += 0x100;
		} while(--blockCount);
	}
	
	if(hasRemainder)
	{
#ifdef __SPU__
		*cmd = (remainCount << 24) | (start & 0x00FFFFFF);
#else
		*cmd = Rlwimi(start, remainCount, 24, 0, 7);
#endif
	}
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::DrawElements(DrawMode mode, U32 start, U32 count, IndexType type, U32 indexOffset, IndexContext context)
{
	ICE_ASSERTF((mode == kDrawPoints)
		|| (mode == kDrawLines)
		|| (mode == kDrawLineLoop)
		|| (mode == kDrawLineStrip)
		|| (mode == kDrawTriangles)
		|| (mode == kDrawTriangleStrip)
		|| (mode == kDrawTriangleFan)
		|| (mode == kDrawQuads)
		|| (mode == kDrawQuadStrip)
		|| (mode == kDrawPolygon)
		, ("%i", mode));

	// Index Buffers must be 128 byte aligned
	ICE_ASSERTF((indexOffset & 0x7F) == 0, ("0x%08x", indexOffset));

	// If we are trying to render 0 indexes, return
	if (count == 0) 
		return;
	
	U32F blockCount = count >> 8;
	U32F remainCount = count & 0x000000FF;
	U32F hasRemainder = (remainCount != 0);
	U32F commandCount = (count + 0xFF) >> 8;
	ICE_ASSERT(commandCount < 2048);
	CHECK_RESERVE(8 + commandCount);
	
	m_cmdptr[0] = CMD(kCmdIndexBufferAddress, 8);
	m_cmdptr[1] = indexOffset;
	m_cmdptr[2] = context | type;
	m_cmdptr[3] = CMD(kCmdDrawMode, 4);
	m_cmdptr[4] = mode;
	m_cmdptr[5] = CMD(kCmdDrawElements, commandCount * 4) | kCmdNoIncFlag;
	m_cmdptr[commandCount+6] = CMD(kCmdDrawMode, 4);
	m_cmdptr[commandCount+7] = 0;
	U32 *__restrict cmd = m_cmdptr+6;
	m_cmdptr += commandCount+8;
	--remainCount;
	start |= 0xFF000000;

	if(blockCount != 0)
	{
		do 
		{
			*cmd++ = start;
			start += 0x100;
		} while(--blockCount);
	}
	
	if(hasRemainder)
	{
#ifdef __SPU__
		*cmd = (remainCount << 24) | (start & 0x00FFFFFF);
#else
		*cmd = Rlwimi(start, remainCount, 24, 0, 7);
#endif
	}
}
#endif

COMMANDCONTEXT_DECL void CommandContext::DrawInstancedElements(DrawMode mode, U32 start, U32 count, IndexType type, U32 indexOffset, IndexContext context, U32 divider, U32 instanceStart, U32 instanceCount)
{
	ICE_ASSERTF((mode == kDrawPoints)
		|| (mode == kDrawLines)
		|| (mode == kDrawLineLoop)
		|| (mode == kDrawLineStrip)
		|| (mode == kDrawTriangles)
		|| (mode == kDrawTriangleStrip)
		|| (mode == kDrawTriangleFan)
		|| (mode == kDrawQuads)
		|| (mode == kDrawQuadStrip)
		|| (mode == kDrawPolygon)
		, ("%i", mode));

	// Index Buffers must be 128 byte aligned
	ICE_ASSERTF((indexOffset & 0x7F) == 0, ("0x%08x", indexOffset));

	// If we are trying to render 0 indexes, return
	if ((count == 0) || (instanceCount == 0))
		return;
	
	U32F blockCount = count >> 8;
	U32F const remainCount = count & 0x000000FF;
	U32F const commandCount = blockCount + (remainCount != 0);
	ICE_ASSERT(commandCount < 2048);
	CHECK_RESERVE(9 + (commandCount+3)*instanceCount);
	
	// Set index buffer address and format
	
	m_cmdptr[0] = CMD(kCmdIndexBufferAddress, 8);
	m_cmdptr[1] = indexOffset;
	m_cmdptr[2] = context | type;
	
	// Issue begin command
	
	m_cmdptr[3] = CMD(kCmdDrawMode, 4);
	m_cmdptr[4] = mode;
	m_cmdptr += 5;
	
	// Issue draw commands
	U32F endInst = instanceStart + instanceCount;
	for(U32F i = instanceStart; i != endInst; ++i)
	{
		m_cmdptr[0] = CMD(kCmdVertexAttribBaseIndex, 4);
		m_cmdptr[1] = i*divider;
		m_cmdptr[2] = U32(CMD(kCmdDrawElements, commandCount * 4) | kCmdNoIncFlag);
		m_cmdptr += 3;
			
		U32 indexStart = start;
		if(blockCount != 0)
		{
			U32F instBlockCount = blockCount;
			U32F value = 0xFF000000 | indexStart;
			do 
			{
				*m_cmdptr++ = (U32)value;
				value += 0x100;
			} while(--instBlockCount);
			indexStart = value & 0x00FFFFFF;
		}
		
		if (remainCount != 0)
		{
			*m_cmdptr++ = ((remainCount - 1) << 24) | indexStart;
		}
	}
	
	// Issue end command
	
	m_cmdptr[0] = CMD(kCmdVertexAttribBaseIndex, 4);
	m_cmdptr[1] = 0;
	m_cmdptr[2] = CMD(kCmdDrawMode, 4);
	m_cmdptr[3] = 0;
	m_cmdptr += 4;
}

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::Begin(DrawMode mode)
{
	ICE_ASSERTF((mode == kDrawPoints)
		|| (mode == kDrawLines)
		|| (mode == kDrawLineLoop)
		|| (mode == kDrawLineStrip)
		|| (mode == kDrawTriangles)
		|| (mode == kDrawTriangleStrip)
		|| (mode == kDrawTriangleFan)
		|| (mode == kDrawQuads)
		|| (mode == kDrawQuadStrip)
		|| (mode == kDrawPolygon)
		, ("%i", mode));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdDrawMode, 4);
	m_cmdptr[1] = mode;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InsertIndexArrayOffsetAndFormat(IndexType type, U32 indexOffset, IndexContext context)
{
	// Index Buffers must be 128 byte aligned
	ICE_ASSERTF((indexOffset & 0x7F) == 0, ("0x%08x", indexOffset));

	CHECK_RESERVE(3);

	m_cmdptr[0] = CMD(kCmdIndexBufferAddress, 8);
	m_cmdptr[1] = indexOffset;
	m_cmdptr[2] = context | type;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InsertDraw(DrawType drawType, U32 start, U32 count)
{
	U32 blockCount = count >> 8;
	U32 remainCount = count & 0x000000FF;
	U32 hasRemainder = (remainCount != 0);
	U32 commandCount = (count + 0xFF) >> 8;
	ICE_ASSERT(commandCount < 2048);
	CHECK_RESERVE(1 + commandCount);

	m_cmdptr[0] = CMD(drawType, commandCount * 4) | kCmdNoIncFlag;
	U32 *__restrict cmd = m_cmdptr+1;
	m_cmdptr += commandCount+1;
	--remainCount;
	start |= 0xFF000000;

	if(blockCount != 0)
	{
		do 
		{
			*cmd++ = start;
			start += 0x100;
		} while(--blockCount);
	}
	
	if(hasRemainder)
	{
#ifdef __SPU__
		*cmd = (remainCount << 24) | (start & 0x00FFFFFF);
#else
		*cmd = Rlwimi(start, remainCount, 24, 0, 7);
#endif
	}
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::End()
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdDrawMode, 4);
	m_cmdptr[1] = 0;
	m_cmdptr += 2;
}
#endif

COMMANDCONTEXT_DECL void CommandContext::DrawImmediateArrays(DrawMode mode, const void *array, U32 size)
{
	ICE_ASSERTF((mode == kDrawPoints)
		|| (mode == kDrawLines)
		|| (mode == kDrawLineLoop)
		|| (mode == kDrawLineStrip)
		|| (mode == kDrawTriangles)
		|| (mode == kDrawTriangleStrip)
		|| (mode == kDrawTriangleFan)
		|| (mode == kDrawQuads)
		|| (mode == kDrawQuadStrip)
		|| (mode == kDrawPolygon)
		, ("%i", mode));

	// If we are trying to render 0 vertexes, return
	if (size == 0) 
		return;

	size >>= 2;	
	U32F blockCount = size / 2047;
	U32F remainCount = size - blockCount * 2047;
	CHECK_RESERVE(4 + blockCount + (blockCount * 2047) + remainCount + (remainCount != 0));
	
	// Issue begin command
	m_cmdptr[0] = CMD(kCmdDrawMode, 4);
	m_cmdptr[1] = mode;
	m_cmdptr += 2;
	
	// Issue draw commands
	U32 *data = (U32*)array;
	if(blockCount != 0)
	{
		do 
		{
			*m_cmdptr++ = CMD(kCmdDrawArraysInline, 0x1FFC) | kCmdNoIncFlag;
#ifdef __SPU__
			for(U32F i = 0; i < 0x7FF; ++i)
				m_cmdptr[i] = data[i];
#else
			memcpy(m_cmdptr, data, 0x1FFC);
#endif
			m_cmdptr += 0x7FF;
			data += 0x7FF;
		} while(--blockCount);
	}
	
	if(remainCount != 0)
	{
		U32F remainSize = remainCount << 2;
		*m_cmdptr++ = CMD(kCmdDrawArraysInline, remainSize) | kCmdNoIncFlag;
#ifdef __SPU__
		for(U32F i = 0; i < remainCount; ++i)
			m_cmdptr[i] = data[i];
#else
		memcpy(m_cmdptr, data, remainSize);
#endif
		m_cmdptr += remainCount;
	}

	// Issue end command
	m_cmdptr[0] = CMD(kCmdDrawMode, 4);
	m_cmdptr[1] = 0;
	m_cmdptr += 2;
}

COMMANDCONTEXT_DECL void CommandContext::DrawImmediateElements(DrawMode mode, U32 count, IndexType type, const void *index)
{
	ICE_ASSERTF((mode == kDrawPoints)
		|| (mode == kDrawLines)
		|| (mode == kDrawLineLoop)
		|| (mode == kDrawLineStrip)
		|| (mode == kDrawTriangles)
		|| (mode == kDrawTriangleStrip)
		|| (mode == kDrawTriangleFan)
		|| (mode == kDrawQuads)
		|| (mode == kDrawQuadStrip)
		|| (mode == kDrawPolygon)
		, ("%i", mode));

	// If we are trying to render 0 vertexes, return
	if (count == 0) 
		return;

	// Convert into bytes, rounding up.
	U32F size = (count << ((type != kIndex32) ? 1 : 2)) + 3;
	size >>= 2;	
	U32F blockCount = size / 2047;
	U32F const remainCount = size - blockCount * 2047;
	CHECK_RESERVE(4 + blockCount + (blockCount * 2047) + remainCount + (remainCount != 0));
	
	// Issue begin command
	m_cmdptr[0] = CMD(kCmdDrawMode, 4);
	m_cmdptr[1] = mode;
	m_cmdptr += 2;
	
	if(type != kIndex32)
	{
		// Issue draw commands
		U32 *pData = (U32*)index;
		if(blockCount != 0)
		{
			do 
			{
				*m_cmdptr++ = CMD(kCmdDraw16bitElementsInline, 0x1FFC) | kCmdNoIncFlag;
				for (U32F i = 0; i < 0x7FF; ++i)
				{
					U32 value = *pData++;
					*m_cmdptr++ = (value >> 16) | (value << 16);
				}
			} while(--blockCount);
		}
		
		if(remainCount != 0)
		{
			*m_cmdptr++ = CMD(kCmdDraw16bitElementsInline, remainCount << 2) | kCmdNoIncFlag;
			for (U32F i = 0; i < remainCount; ++i)
			{
				U32 value = *pData++;
				*m_cmdptr++ = (value >> 16) | (value << 16);
			}
		}
	}
	else
	{
		// Issue draw commands
		U32 *pData = (U32*)index;
		if(blockCount != 0)
		{
			do 
			{
				*m_cmdptr++ = CMD(kCmdDraw32bitElementsInline, 0x1FFC) | kCmdNoIncFlag;
#ifdef __SPU__
				for(U32F i = 0; i < 0x7FF; ++i)
					m_cmdptr[i] = pData[i];
#else
				memcpy(m_cmdptr, pData, 0x1FFC);
#endif
				m_cmdptr += 0x7FF;
				pData += 0x7FF;
			} while(--blockCount);
		}
		
		if(remainCount != 0)
		{
			U32 remainSize = remainCount << 2;
			*m_cmdptr++ = CMD(kCmdDraw32bitElementsInline, remainSize) | kCmdNoIncFlag;
#ifdef __SPU__
			for(U32F i = 0; i < remainCount; ++i)
				m_cmdptr[i] = pData[i];
#else
			memcpy(m_cmdptr, pData, remainSize);
#endif
			m_cmdptr += remainCount;
		}
	}

	// Issue end command
	m_cmdptr[0] = CMD(kCmdDrawMode, 4);
	m_cmdptr[1] = 0;
	m_cmdptr += 2;
}

// Refreshes the internal pre-transform vertex cache.
#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InvalidatePreTransformCache()
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdInvalidatePreTransformCache, 4);
	m_cmdptr[1] = 0;
	m_cmdptr += 2;
}
#endif

// Refreshes the internal post-transform vertex cache.
#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InvalidatePostTransformCache()
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdInvalidatePostTransformCache, 4);
	m_cmdptr[1] = 0;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexProgramStaticBranchBits(U32 branchBits)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdVertexProgramBranchBits, 4);
	m_cmdptr[1] = branchBits;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SelectVertexProgram(const VertexProgram *__restrict program)
{
	ICE_ASSERTF(program != NULL, ("%p", program));
	ICE_ASSERT(program->m_patchCount || (program->m_instructionSlot + program->m_instructionCount <= 544));
	ICE_ASSERTF(!program->m_patchCount || (program->m_instructionSlot + program->m_instructionCount <= 512), ("Vertex programs with branching must exist below 512 instruction slots."));
	
	U32F const constCount = program->m_constantCount;
	CHECK_RESERVE_BYTES(28 + constCount * 24);
	
	U32 *__restrict cmd = m_cmdptr;
	cmd[0] = CMD(kCmdVertexProgramExecuteSlot, 4);
	cmd[1] = program->m_instructionSlot;
	cmd[2] = CMD(kCmdVertexAttributeMask, 8);
	cmd[3] = program->m_vertexAttributeMask;
	cmd[4] = program->m_vertexResultMask;
	cmd[5] = CMD(kCmdVertexProgramLimits, 4);
	cmd[6] = program->m_vertexLimits;
	m_cmdptr += 7;
	
	// Set any literal constant values
	
	const float *value = program->GetConstantTable();
	for (U32F i = 0; i < constCount; ++i)
	{
		U32 d0 = F32ToU32(value[0]);
		m_cmdptr[0] = CMD(kCmdVertexProgramConstLoadSlot, 0x14);
		U32 d1 = F32ToU32(value[1]);
		m_cmdptr[1] = program->m_index[i];
		U32 d2 = F32ToU32(value[2]);
		m_cmdptr[2] = d0;
		U32 d3 = F32ToU32(value[3]);
		m_cmdptr[3] = d1;
		m_cmdptr[4] = d2;
		m_cmdptr[5] = d3;
		value += 4;
		m_cmdptr += 6;
	}
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::LoadVertexProgram(const VertexProgram *__restrict program)
{
	ICE_ASSERTF(program != NULL, ("%p", program));
	ICE_ASSERT(program->m_patchCount || (program->m_instructionSlot + program->m_instructionCount <= 544));
	ICE_ASSERTF(!program->m_patchCount || (program->m_instructionSlot + program->m_instructionCount <= 512), ("Vertex programs with branching must exist below 512 instruction slots."));
	
	U32 instrCount = program->m_instructionCount;
	U32 blockCount = instrCount >> 3;
	U32 remainCount = instrCount & 7;
	U32 reserveBytes = 12 + ((instrCount << 4) & ~0xF) + ((instrCount >> 1) & ~0x3);
	ICE_ASSERT(reserveBytes == 12 + blockCount * 132 + remainCount * 16); // optimization check
	CHECK_RESERVE_BYTES(reserveBytes);
	
	// Set the load and execute slot indexes
	
	m_cmdptr[0] = CMD(kCmdVertexProgramInstrLoadSlot, 4);
	m_cmdptr[1] = program->m_instructionSlot;
	m_cmdptr += 2;
	
	// Dump the vertex program instructions into the command buffer
	U32 const *__restrict code = program->GetMicrocode();
	for (U32F i = 0; i < blockCount; ++i)
	{
		U32 d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15,d16,d17,d18,d19,d20,d21,d22,d23,d24,d25,d26,d27,d28,d29,d30,d31;
		d0 = code[0];
		d1 = code[1];
		m_cmdptr[0] = CMD(kCmdVertexProgramInstruction, 0x80);
		d2 = code[2];
		m_cmdptr[1] = d0;
		d3 = code[3];
		m_cmdptr[2] = d1;
		d4 = code[4];
		m_cmdptr[3] = d2;
		d5 = code[5];
		m_cmdptr[4] = d3;
		d6 = code[6];
		m_cmdptr[5] = d4;
		d7 = code[7];
		m_cmdptr[6] = d5;
		d8 = code[8];
		m_cmdptr[7] = d6;
		d9 = code[9];
		m_cmdptr[8] = d7;
		d10 = code[10];
		m_cmdptr[9] = d8;
		d11 = code[11];
		m_cmdptr[10] = d9;
		d12 = code[12];
		m_cmdptr[11] = d10;
		d13 = code[13];
		m_cmdptr[12] = d11;
		d14 = code[14];
		m_cmdptr[13] = d12;
		d15 = code[15];
		m_cmdptr[14] = d13;
		d16 = code[16];
		m_cmdptr[15] = d14;
		d17 = code[17];
		m_cmdptr[16] = d15;
		d18 = code[18];
		m_cmdptr[17] = d16;
		d19 = code[19];
		m_cmdptr[18] = d17;
		d20 = code[20];
		m_cmdptr[19] = d18;
		d21 = code[21];
		m_cmdptr[20] = d19;
		d22 = code[22];
		m_cmdptr[21] = d20;
		d23 = code[23];
		m_cmdptr[22] = d21;
		d24 = code[24];
		m_cmdptr[23] = d22;
		d25 = code[25];
		m_cmdptr[24] = d23;
		d26 = code[26];
		m_cmdptr[25] = d24;
		d27 = code[27];
		m_cmdptr[26] = d25;
		d28 = code[28];
		m_cmdptr[27] = d26;
		d29 = code[29];
		m_cmdptr[28] = d27;
		d30 = code[30];
		m_cmdptr[29] = d28;
		d31 = code[31];
		m_cmdptr[30] = d29;
		m_cmdptr[31] = d30;
		m_cmdptr[32] = d31;
		m_cmdptr += 33;
		code += 32;
	}

	if(remainCount != 0)
	{
		m_cmdptr[0] = CMD(kCmdVertexProgramInstruction, remainCount * 16);
		m_cmdptr += 1;

		for(U32F i = 0; i < remainCount; ++i)
		{
			U32 d0,d1,d2,d3;
			d0 = code[0];
			d1 = code[1];
			m_cmdptr[0] = d0;
			d2 = code[2];
			m_cmdptr[1] = d1;
			d3 = code[3];
			m_cmdptr[2] = d2;
			m_cmdptr[3] = d3;
			m_cmdptr += 4;
			code += 4;
		}
	}
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexProgramConstant(U32 index, float x, float y, float z, float w)
{
	ICE_ASSERTF(index <= 467, ("%i", index));

	CHECK_RESERVE(6);
	
	m_cmdptr[0] = CMD(kCmdVertexProgramConstLoadSlot, 0x14);
	m_cmdptr[1] = index;
	*(float *) &m_cmdptr[2] = x;
	*(float *) &m_cmdptr[3] = y;
	*(float *) &m_cmdptr[4] = z;
	*(float *) &m_cmdptr[5] = w;
	m_cmdptr += 6;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexProgramConstant(U32 index, const float *data)
{
	ICE_ASSERTF(index <= 467, ("%i", index));
	
	CHECK_RESERVE(6);

	U32 d0 = F32ToU32(data[0]);
	m_cmdptr[0] = CMD(kCmdVertexProgramConstLoadSlot, 0x14);
	U32 d1 = F32ToU32(data[1]);
	m_cmdptr[1] = index;
	U32 d2 = F32ToU32(data[2]);
	m_cmdptr[2] = d0;
	U32 d3 = F32ToU32(data[3]);
	m_cmdptr[3] = d1;
	m_cmdptr[4] = d2;
	m_cmdptr[5] = d3;
	m_cmdptr += 6;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexProgram2Constants(U32 index, const float *data)
{
	ICE_ASSERTF(index <= 466, ("%i", index));
	
	CHECK_RESERVE(10);

	U32 d0 = F32ToU32(data[0]);
	m_cmdptr[0] = CMD(kCmdVertexProgramConstLoadSlot, 0x24);
	U32 d1 = F32ToU32(data[1]);
	m_cmdptr[1] = index;
	U32 d2 = F32ToU32(data[2]);
	m_cmdptr[2] = d0;
	U32 d3 = F32ToU32(data[3]);
	m_cmdptr[3] = d1;
	U32 d4 = F32ToU32(data[4]);
	m_cmdptr[4] = d2;
	U32 d5 = F32ToU32(data[5]);
	m_cmdptr[5] = d3;
	U32 d6 = F32ToU32(data[6]);
	m_cmdptr[6] = d4;
	U32 d7 = F32ToU32(data[7]);
	m_cmdptr[7] = d5;
	m_cmdptr[8] = d6;
	m_cmdptr[9] = d7;
	m_cmdptr += 10;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexProgram3Constants(U32 index, const float *data)
{
	ICE_ASSERTF(index <= 465, ("%i", index));
	
	CHECK_RESERVE(14);
	
	U32 d0 = F32ToU32(data[0]);
	m_cmdptr[0] = CMD(kCmdVertexProgramConstLoadSlot, 0x34);
	U32 d1 = F32ToU32(data[1]);
	m_cmdptr[1] = index;
	U32 d2 = F32ToU32(data[2]);
	m_cmdptr[2] = d0;
	U32 d3 = F32ToU32(data[3]);
	m_cmdptr[3] = d1;
	U32 d4 = F32ToU32(data[4]);
	m_cmdptr[4] = d2;
	U32 d5 = F32ToU32(data[5]);
	m_cmdptr[5] = d3;
	U32 d6 = F32ToU32(data[6]);
	m_cmdptr[6] = d4;
	U32 d7 = F32ToU32(data[7]);
	m_cmdptr[7] = d5;
	U32 d8 = F32ToU32(data[8]);
	m_cmdptr[8] = d6;
	U32 d9 = F32ToU32(data[9]);
	m_cmdptr[9] = d7;
	U32 d10 = F32ToU32(data[10]);
	m_cmdptr[10] = d8;
	U32 d11 = F32ToU32(data[11]);
	m_cmdptr[11] = d9;
	m_cmdptr[12] = d10;
	m_cmdptr[13] = d11;
	m_cmdptr += 14;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexProgram4Constants(U32 index, const float *data)
{
	ICE_ASSERTF(index <= 464, ("%i", index));
	
	CHECK_RESERVE(18);
	
	U32 d0 = F32ToU32(data[0]);
	m_cmdptr[0] = CMD(kCmdVertexProgramConstLoadSlot, 0x44);
	U32 d1 = F32ToU32(data[1]);
	m_cmdptr[1] = index;
	U32 d2 = F32ToU32(data[2]);
	m_cmdptr[2] = d0;
	U32 d3 = F32ToU32(data[3]);
	m_cmdptr[3] = d1;
	U32 d4 = F32ToU32(data[4]);
	m_cmdptr[4] = d2;
	U32 d5 = F32ToU32(data[5]);
	m_cmdptr[5] = d3;
	U32 d6 = F32ToU32(data[6]);
	m_cmdptr[6] = d4;
	U32 d7 = F32ToU32(data[7]);
	m_cmdptr[7] = d5;
	U32 d8 = F32ToU32(data[8]);
	m_cmdptr[8] = d6;
	U32 d9 = F32ToU32(data[9]);
	m_cmdptr[9] = d7;
	U32 d10 = F32ToU32(data[10]);
	m_cmdptr[10] = d8;
	U32 d11 = F32ToU32(data[11]);
	m_cmdptr[11] = d9;
	U32 d12 = F32ToU32(data[12]);
	m_cmdptr[12] = d10;
	U32 d13 = F32ToU32(data[13]);
	m_cmdptr[13] = d11;
	U32 d14 = F32ToU32(data[14]);
	m_cmdptr[14] = d12;
	U32 d15 = F32ToU32(data[15]);
	m_cmdptr[15] = d13;
	m_cmdptr[16] = d14;
	m_cmdptr[17] = d15;
	m_cmdptr += 18;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexProgramConstants(U32 start, U32 count, const float *__restrict data)
{
	ICE_ASSERTF(start <= 467, ("%i", start));
	ICE_ASSERTF(start+count <= 468, ("%i + %i", start, count));
	ICE_ASSERT(data != NULL);

	U32 blockCount = count >> 3;
	U32 remainCount = count & 7;
	U32 reserveBytes = 8 + ((count << 4) & ~0xF) + (count & ~0x7);
	ICE_ASSERT(reserveBytes == 8 + blockCount * 136 + remainCount * 16); // optimization check
	CHECK_RESERVE_BYTES(reserveBytes);
	
	// Set as many constants as possible in groups of 8
	
	for (U32F i = 0; i < blockCount; ++i)
	{
		U32 d0 = F32ToU32(data[0]);
		m_cmdptr[0] = CMD(kCmdVertexProgramConstLoadSlot, 0x84);
		U32 d1 = F32ToU32(data[1]);
		m_cmdptr[1] = start;
		U32 d2 = F32ToU32(data[2]);
		m_cmdptr[2] = d0;
		U32 d3 = F32ToU32(data[3]);
		m_cmdptr[3] = d1;
		U32 d4 = F32ToU32(data[4]);
		m_cmdptr[4] = d2;
		U32 d5 = F32ToU32(data[5]);
		m_cmdptr[5] = d3;
		U32 d6 = F32ToU32(data[6]);
		m_cmdptr[6] = d4;
		U32 d7 = F32ToU32(data[7]);
		m_cmdptr[7] = d5;
		U32 d8 = F32ToU32(data[8]);
		m_cmdptr[8] = d6;
		U32 d9 = F32ToU32(data[9]);
		m_cmdptr[9] = d7;
		U32 d10 = F32ToU32(data[10]);
		m_cmdptr[10] = d8;
		U32 d11 = F32ToU32(data[11]);
		m_cmdptr[11] = d9;
		U32 d12 = F32ToU32(data[12]);
		m_cmdptr[12] = d10;
		U32 d13 = F32ToU32(data[13]);
		m_cmdptr[13] = d11;
		U32 d14 = F32ToU32(data[14]);
		m_cmdptr[14] = d12;
		U32 d15 = F32ToU32(data[15]);
		m_cmdptr[15] = d13;
		U32 d16 = F32ToU32(data[16]);
		m_cmdptr[16] = d14;
		U32 d17 = F32ToU32(data[17]);
		m_cmdptr[17] = d15;
		U32 d18 = F32ToU32(data[18]);
		m_cmdptr[18] = d16;
		U32 d19 = F32ToU32(data[19]);
		m_cmdptr[19] = d17;
		U32 d20 = F32ToU32(data[20]);
		m_cmdptr[20] = d18;
		U32 d21 = F32ToU32(data[21]);
		m_cmdptr[21] = d19;
		U32 d22 = F32ToU32(data[22]);
		m_cmdptr[22] = d20;
		U32 d23 = F32ToU32(data[23]);
		m_cmdptr[23] = d21;
		U32 d24 = F32ToU32(data[24]);
		m_cmdptr[24] = d22;
		U32 d25 = F32ToU32(data[25]);
		m_cmdptr[25] = d23;
		U32 d26 = F32ToU32(data[26]);
		m_cmdptr[26] = d24;
		U32 d27 = F32ToU32(data[27]);
		m_cmdptr[27] = d25;
		U32 d28 = F32ToU32(data[28]);
		m_cmdptr[28] = d26;
		U32 d29 = F32ToU32(data[29]);
		m_cmdptr[29] = d27;
		U32 d30 = F32ToU32(data[30]);
		m_cmdptr[30] = d28;
		U32 d31 = F32ToU32(data[31]);
		m_cmdptr[31] = d29;
		m_cmdptr[32] = d30;
		m_cmdptr[33] = d31;

		m_cmdptr += 34;
		data += 32;
		start += 8;
	}
	
	// Set the remainder of the constants using one more command
	
	if (remainCount != 0)
	{
		m_cmdptr[0] = CMD(kCmdVertexProgramConstLoadSlot, remainCount * 16 + 4);
		m_cmdptr[1] = start;
		m_cmdptr += 2;
		
		for(U32F i = 0; i < remainCount; ++i)
		{
			U32 d0 = F32ToU32(data[0]);
			U32 d1 = F32ToU32(data[1]);
			m_cmdptr[0] = d0;
			U32 d2 = F32ToU32(data[2]);
			m_cmdptr[1] = d1;
			U32 d3 = F32ToU32(data[3]);
			m_cmdptr[2] = d2;
			m_cmdptr[3] = d3;
			m_cmdptr += 4;
			data += 4;
		}
	}
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetFragmentProgram(const FragmentProgram *__restrict program)
{
	CHECK_RESERVE(15);
	
	// Fragment programs have a 64byte alignment requirement, assert this
	ICE_ASSERTF((program->m_offsetAndContext & 0x3C) == 0, ("0x%08x", program->m_offsetAndContext));
	
	// Set the base address and control register
	U32 *__restrict cmd = m_cmdptr;	
	cmd[0] = CMD(kCmdFragmentProgramAddress, 0x4);
	cmd[1] = program->m_offsetAndContext;
	cmd[2] = CMD(kCmdFragmentProgramControl, 0x4);
	cmd[3] = program->m_control;

	// Set the 2D flag for the appropriate texture coordinates
	
	cmd[4] = CMD(kCmdTexcoordControl, 0x28);
	U32 mask0 = program->m_texcoordMask;
	U32 mask1 = U32(program->m_centroidMask) << 4;
	for (U32F i = 0; i < 10; ++i, mask0>>=1, mask1>>=1)
		cmd[i + 5] = (mask0 & 0x01) | (mask1 & 0x10);
	m_cmdptr += 15;
}
#endif

#if ICERENDER_ASM==0
#ifndef __SPU__
COMMANDCONTEXT_DECL void CommandContext::SetNullFragmentProgram()
{
	CHECK_RESERVE(15);
		
	ICE_ASSERT(g_nullFragmentProgramOffset != nullptr);
	
	// Set the base address and control register
	
	U32 *__restrict cmd = m_cmdptr;	

	cmd[0] = CMD(kCmdFragmentProgramAddress, 0x4);
	cmd[1] = g_nullFragmentProgramOffset | kFragmentProgramMainMemory;
	cmd[2] = CMD(kCmdFragmentProgramControl, 0x4);
	cmd[3] = 0x02008400;
	
	// Set the 2D flag for all texture coordinates
	
	cmd[4] = CMD(kCmdTexcoordControl, 0x28);
	cmd[5] = 1;
	cmd[6] = 1;
	cmd[7] = 1;
	cmd[8] = 1;
	cmd[9] = 1;
	cmd[10] = 1;
	cmd[11] = 1;
	cmd[12] = 1;
	cmd[13] = 1;
	cmd[14] = 1;
	m_cmdptr += 15;
}
#endif
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::RefreshFragmentProgram(const FragmentProgram *__restrict program)
{
	ICE_ASSERT(program != NULL);

	CHECK_RESERVE(2);
	
	U32 *__restrict cmd = m_cmdptr;	
	
	cmd[0] = CMD(kCmdFragmentProgramAddress, 4);
	cmd[1] = program->m_offsetAndContext;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetFragmentProgramConstant(FragmentProgram *__restrict program, U32 index, const float *__restrict data, bool noProgramChange)
{
	ICE_ASSERT(program != NULL);
	ICE_ASSERT(data != NULL);
	ICE_ASSERT(index < program->m_patchCount);
	
	// only set constants that actually exist
	if (index >= program->m_patchCount)
		return;
		
	PatchData const *patchData = program->GetPatchData(index);
	U32 patchCount = patchData->m_count;
	if(patchCount == 0)
		return;

	CHECK_RESERVE_BYTES(0x20 + 0x24*patchCount);

	U32 *__restrict cmd = m_cmdptr;

	if(!noProgramChange)
	{
		U32 dst = program->m_offsetAndContext & ~0x3;
		U32 context = 0xFEED0000 | ((program->m_offsetAndContext ^ 1) & 1);

		cmd[0] = CMD(kCmdSurfaceSourceContext, 0x8);
		cmd[1] = context;
		cmd[2] = context;
		cmd[3] = CMD(kCmdSurfaceDestinationAddress, 0x4);
		cmd[4] = dst;
		cmd[5] = CMD(kCmdSurfaceFormat, 0x8);
		cmd[6] = 0x000B;
		cmd[7] = 0x10001000;
		cmd += 8;
	}

	U32 x = F32ToU32(data[0]);
	U32 y = F32ToU32(data[1]);
	U32 z = F32ToU32(data[2]);
	U32 w = F32ToU32(data[3]);

	x = (x << 16) | (x >> 16);
	y = (y << 16) | (y >> 16);
	z = (z << 16) | (z >> 16);
	w = (w << 16) | (w >> 16);

	U16 const *patchIndexes = patchData->m_index;
	do
	{
		U32 patchIdx = *patchIndexes;
		U32 val = ((patchIdx << 8) & 0xFFFF0000) | ((patchIdx << 2) & 0x03FF);
		ICE_ASSERT(val == ((((patchIdx << 4) >> 12) << 16) | (((patchIdx << 4) & 0x0FFF) >> 2))); // optimization check

		cmd[0] = CMD(kCmdBlitDestinationPoint, 0xC);
		cmd[1] = val;
		cmd[2] = 0x00010004;
		cmd[3] = 0x00010004;
		cmd[4] = CMD(kCmdBlitData, 0x10);
		cmd[5] = x;
		cmd[6] = y;
		cmd[7] = z;
		cmd[8] = w;
		cmd += 9;
		patchIndexes += 1;
	} while(--patchCount != 0);
	
	m_cmdptr = cmd;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetFragmentProgramConstants(FragmentProgram *__restrict program, U32 start, U32 count, const float *__restrict data, bool noProgramChange)
{
	ICE_ASSERT(program != NULL);
	ICE_ASSERT(data != NULL);
	ICE_ASSERT(start < program->m_patchCount);
	ICE_ASSERT(start+count <= program->m_patchCount);

	CHECK_RESERVE(8);

	if(count == 0)
		return;

	ICE_ASSERTF(start < program->m_patchCount, ("SetFragmentProgramConstants: start index exceeds size of patch table"));
	U32 end = start + count;
	ICE_ASSERTF(end <= program->m_patchCount, ("SetFragmentProgramConstants: end index exceeds size of patch table"));

	U32 *__restrict cmd = m_cmdptr;

	if(!noProgramChange)
	{
		U32 dst = program->m_offsetAndContext & ~0x3;
		U32 context = 0xFEED0000 | ((program->m_offsetAndContext ^ 1) & 1);

		cmd[0] = CMD(kCmdSurfaceSourceContext, 0x8);
		cmd[1] = context;
		cmd[2] = context;
		cmd[3] = CMD(kCmdSurfaceDestinationAddress, 0x4);
		cmd[4] = dst;
		cmd[5] = CMD(kCmdSurfaceFormat, 0x8);
		cmd[6] = 0x000B;
		cmd[7] = 0x10001000;
		cmd += 8;
	}

	do
	{
		PatchData const *__restrict patchData = program->GetPatchData(start);
		U32 patchCount = patchData->m_count;
		
		U32 x = F32ToU32(data[0]);
		U32 y = F32ToU32(data[1]);
		U32 z = F32ToU32(data[2]);
		U32 w = F32ToU32(data[3]);
		data += 4;
		start += 1;

		if(!patchCount)
			continue;

		m_cmdptr = cmd;
		CHECK_RESERVE(9 * patchCount);
		cmd = m_cmdptr;

		x = (x << 16) | (x >> 16);
		y = (y << 16) | (y >> 16);
		z = (z << 16) | (z >> 16);
		w = (w << 16) | (w >> 16);

		U16 const *__restrict patchIndexes = patchData->m_index;
		do
		{
			U32 patchIdx = *patchIndexes;
			U32 val = ((patchIdx << 8) & 0xFFFF0000) | ((patchIdx << 2) & 0x03FF);
			ICE_ASSERT(val == ((((patchIdx << 4) >> 12) << 16) | (((patchIdx << 4) & 0x0FFF) >> 2))); // optimization check

			cmd[0] = CMD(kCmdBlitDestinationPoint, 0xC);
			cmd[1] = val;
			cmd[2] = 0x00010004;
			cmd[3] = 0x00010004;
			cmd[4] = CMD(kCmdBlitData, 0x10);
			cmd[5] = x;
			cmd[6] = y;
			cmd[7] = z;
			cmd[8] = w;
			cmd += 9;
			patchIndexes += 1;
		} while(--patchCount != 0);
	} while(--count);
	m_cmdptr = cmd;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetTextureCylindricalWrapEnable(U32 enable0to7, U32 enable8to9)
{
	CHECK_RESERVE(3);
	
	m_cmdptr[0] = CMD(kCmdCylindricalWrapEnable0, 8);
	m_cmdptr[1] = enable0to7;
	m_cmdptr[2] = enable8to9;
	m_cmdptr += 3;
}
#endif


#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetTexture(U32 unit, const Texture *__restrict texture)
{
	ICE_ASSERT(unit < 16);
	ICE_ASSERT(texture != NULL);
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x10)       // 1D
		&& ((texture->m_format&0x8)==0)           // Border texels are on
		&& ((texture->m_format&0x2000)==0)        // Swizzled
		&& ((texture->m_size1>>16) > 0x800)       // width > 2048
		), ("Max 1D texture size with border texels on is 2048."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x10)       // 1D
		&& ((texture->m_format&0x8)==0x8)         // Border constant color is on
		&& ((texture->m_size1>>16) > 0x1000)      // width > 4096
		), ("Max 1D texture size with border texels off is 4096."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x20)       // 2D
		&& ((texture->m_format&0x8)==0)           // Border texels are on
		&& ((texture->m_format&0x2000)==0)        // Swizzled
		&& ((texture->m_size1>>16) > 0x800)       // width > 2048
		&& ((texture->m_size1&0xFFFF) > 0x800)    // height > 2048
		), ("Max 2D texture size with border texels on is 2048."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x20)       // 2D
		&& ((texture->m_format&0x8)==0x8)         // Border constant color is on
		&& ((texture->m_size1>>16) > 0x1000)      // width > 4096
		&& ((texture->m_size1&0xFFFF) > 0x1000)   // height > 4096
		), ("Max 2D texture size with border texels off is 4096."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x30)       // 3D
		&& ((texture->m_format&0x8)==0)           // Border texels are on
		&& ((texture->m_format&0x2000)==0)        // Swizzled
		&& ((texture->m_size1>>16) > 0x100)       // width > 256
		&& ((texture->m_size1&0xFFFF) > 0x100)    // height > 256
		), ("Max 3D texture size with border texels on is 256."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x30)        // 3D
		&& ((texture->m_format&0x8)==0x8)          // Border constant color is on
		&& ((texture->m_size1>>16) > 0x200)        // width > 512
		&& ((texture->m_size1&0xFFFF) > 0x200)     // height > 512
		), ("Max 3D texture size with border texels off is 512."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x30)        // 3D
		&& ((texture->m_control2&0x70)!=0)         // Anisotropic filtering enabled
		), ("3D textures do not support anisotropic filtering."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x30)        // 3D
		&& ((texture->m_filter&0xE000)==0xE000)    // Convolution filter
		), ("3D textures do not support convolution filters."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x20)        // 2D
		&& ((texture->m_filter&0xE000)==0xE000)    // Convolution filter
		&& ((texture->m_control2&0x70)!=0)         // Anisotropic filtering enabled
		), ("2D convolution filters do not support anisotropic filtering."));
	ICE_ASSERTF(
		   ((texture->m_control1&0x00F00000)==0)   // Gamma conversion disabled.
		|| ((texture->m_format&0x1F00)==0x0100)    // B8
		|| ((texture->m_format&0x1F00)==0x0500)    // Rgba8888
		|| ((texture->m_format&0x1F00)==0x0600)    // Dxt1
		|| ((texture->m_format&0x1F00)==0x0700)    // Dxt3
		|| ((texture->m_format&0x1F00)==0x0800)    // Dxt5
		|| ((texture->m_format&0x1F00)==0x0900)    // SB8
		|| ((texture->m_format&0x1F00)==0x0B00)    // Gb88
		|| ((texture->m_format&0x1F00)==0x0C00)    // SG8SB8
		|| ((texture->m_format&0x1F00)==0x0D00)    // B8R8,G8R8
		|| ((texture->m_format&0x1F00)==0x0E00)    // R8B8,R8G8
		|| ((texture->m_format&0x1F00)==0x1800)    // Unsigned HiLo8
		|| ((texture->m_format&0x1F00)==0x1900)    // Signed HiLo8
		|| ((texture->m_format&0x1F00)==0x1E00)    // Xrgb8888
		, ("Gamma correction must use a texture format which has 8 bits per component."));
	ICE_ASSERTF(!(
		   ((texture->m_control2&0x70)!=0)          // Anisotropic filtering enabled
		&& (   ((texture->m_format&0x1F00)==0x1A00) // Rgba16f
		    || ((texture->m_format&0x1F00)==0x1B00) // Rgba32f
		    || ((texture->m_format&0x1F00)==0x1C00) // R32f
		    || ((texture->m_format&0x1F00)==0x1F00) // Rg16f
		   )
		), ("Float textures do not support anisotropic filtering."));
	ICE_ASSERTF(!(
		   ((texture->m_swizzle&0xFF)!=0xE4)        // No remapping
		&& (   ((texture->m_format&0x1F00)==0x1A00) // Rgba16f
		    || ((texture->m_format&0x1F00)==0x1B00) // Rgba32f
		    || ((texture->m_format&0x1F00)==0x1C00) // R32f
		    || ((texture->m_format&0x1F00)==0x1F00) // Rg16f
		   )
		), ("Float textures do not support swizzles."));
	ICE_ASSERTF(!(
		   (   ((texture->m_format&0x1F00)==0x1400) // R16
		    || ((texture->m_format&0x1F00)==0x1500) // Rg16
		   )
		&& ((texture->m_swizzle&0xFF)!=0xE4)        // Supported 16-bit remaps
		&& ((texture->m_swizzle&0xFF)!=0x4E)
		&& ((texture->m_swizzle&0xFF)!=0xEE)
		&& ((texture->m_swizzle&0xFF)!=0x44)
		), ("Texture swizzle for 16-bit per component textures must move 16-bits."));
	ICE_ASSERTF(!(
		   ((texture->m_control2&0x4)!=0)           // Alpha test
		&& (   ((texture->m_format&0x1F00)==0x1A00) // Rgba16f
		    || ((texture->m_format&0x1F00)==0x1B00) // Rgba32f
		    || ((texture->m_format&0x1F00)==0x1C00) // R32f
		    || ((texture->m_format&0x1F00)==0x1F00) // Rg16f
		   )
		), ("Float textures do not support per-texture alpha test."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0x4)!=0)             // Cube-mapping enabled.
		&& ((texture->m_size1&0xFFFF)!=(texture->m_size1>>16))
		), ("Cube-mapping requires textures with equal width and height."));

	CHECK_RESERVE(15);
	
	U32 baseOffset = texture->m_baseOffset;
	m_cmdptr[0] = CMD(kCmdTextureAddress + unit * 32, 32);
	U32 format = texture->m_format;
	m_cmdptr[1] = baseOffset;
	U32 control1 = texture->m_control1;
	m_cmdptr[2] = format;
	U32 control2 = texture->m_control2;
	m_cmdptr[3] = control1;
	U32 swizzle = texture->m_swizzle;
	m_cmdptr[4] = control2;
	U32 filter = texture->m_filter;
	m_cmdptr[5] = swizzle;
	U32 size1 = texture->m_size1;
	m_cmdptr[6] = filter;
	U32 borderColor = texture->m_borderColor;
	m_cmdptr[7] = size1;
	m_cmdptr[8] = borderColor;
	U32 size2 = texture->m_size2;
	m_cmdptr[9] = CMD(kCmdTextureSize2 + unit * 4, 4);
	m_cmdptr[10] = size2;
	U32 control3 = texture->m_control3;
	m_cmdptr[11] = CMD(kCmdTextureControl3 + unit * 4, 4);
	m_cmdptr[12] = control3;
	U32 colorKeyColor = texture->m_colorKeyColor;
	m_cmdptr[13] = CMD(kCmdTextureColorKeyColor + unit * 4, 4);
	m_cmdptr[14] = colorKeyColor;
	m_cmdptr += 15;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetTextureReduced(U32 unit, const TextureReduced *__restrict texture)
{
	ICE_ASSERT(unit < 16);
	ICE_ASSERT(texture != NULL);
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x10)       // 1D
		&& ((texture->m_format&0x8)==0)           // Border texels are on
		&& ((texture->m_format&0x2000)==0)        // Swizzled
		&& ((texture->m_size1>>16) > 0x800)       // width > 2048
		), ("Max 1D texture size with border texels on is 2048."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x10)       // 1D
		&& ((texture->m_format&0x8)==0x8)         // Border constant color is on
		&& ((texture->m_size1>>16) > 0x1000)      // width > 4096
		), ("Max 1D texture size with border texels off is 4096."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x20)       // 2D
		&& ((texture->m_format&0x8)==0)           // Border texels are on
		&& ((texture->m_format&0x2000)==0)        // Swizzled
		&& ((texture->m_size1>>16) > 0x800)       // width > 2048
		&& ((texture->m_size1&0xFFFF) > 0x800)    // height > 2048
		), ("Max 2D texture size with border texels on is 2048."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x20)       // 2D
		&& ((texture->m_format&0x8)==0x8)         // Border constant color is on
		&& ((texture->m_size1>>16) > 0x1000)      // width > 4096
		&& ((texture->m_size1&0xFFFF) > 0x1000)   // height > 4096
		), ("Max 2D texture size with border texels off is 4096."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x30)       // 3D
		&& ((texture->m_format&0x8)==0)           // Border texels are on
		&& ((texture->m_format&0x2000)==0)        // Swizzled
		&& ((texture->m_size1>>16) > 0x100)       // width > 256
		&& ((texture->m_size1&0xFFFF) > 0x100)    // height > 256
		), ("Max 3D texture size with border texels on is 256."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x30)        // 3D
		&& ((texture->m_format&0x8)==0x8)          // Border constant color is on
		&& ((texture->m_size1>>16) > 0x200)        // width > 512
		&& ((texture->m_size1&0xFFFF) > 0x200)     // height > 512
		), ("Max 3D texture size with border texels off is 512."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x30)        // 3D
		&& ((texture->m_control2&0x70)!=0)         // Anisotropic filtering enabled
		), ("3D textures do not support anisotropic filtering."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x30)        // 3D
		&& ((texture->m_filter&0xE000)==0xE000)    // Convolution filter
		), ("3D textures do not support convolution filters."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0xF0)==0x20)        // 2D
		&& ((texture->m_filter&0xE000)==0xE000)    // Convolution filter
		&& ((texture->m_control2&0x70)!=0)         // Anisotropic filtering enabled
		), ("2D convolution filters do not support anisotropic filtering."));
	ICE_ASSERTF(
		   ((texture->m_control1&0x00F00000)==0)   // Gamma conversion disabled.
		|| ((texture->m_format&0x1F00)==0x0100)    // B8
		|| ((texture->m_format&0x1F00)==0x0500)    // Rgba8888
		|| ((texture->m_format&0x1F00)==0x0600)    // Dxt1
		|| ((texture->m_format&0x1F00)==0x0700)    // Dxt3
		|| ((texture->m_format&0x1F00)==0x0800)    // Dxt5
		|| ((texture->m_format&0x1F00)==0x0900)    // SB8
		|| ((texture->m_format&0x1F00)==0x0B00)    // Gb88
		|| ((texture->m_format&0x1F00)==0x0C00)    // SG8SB8
		|| ((texture->m_format&0x1F00)==0x0D00)    // B8R8,G8R8
		|| ((texture->m_format&0x1F00)==0x0E00)    // R8B8,R8G8
		|| ((texture->m_format&0x1F00)==0x1800)    // Unsigned HiLo8
		|| ((texture->m_format&0x1F00)==0x1900)    // Signed HiLo8
		|| ((texture->m_format&0x1F00)==0x1E00)    // Xrgb8888
		, ("Gamma correction must use a texture format which has 8 bits per component."));
	ICE_ASSERTF(!(
		   ((texture->m_control2&0x70)!=0)          // Anisotropic filtering enabled
		&& (   ((texture->m_format&0x1F00)==0x1A00) // Rgba16f
		    || ((texture->m_format&0x1F00)==0x1B00) // Rgba32f
		    || ((texture->m_format&0x1F00)==0x1C00) // R32f
		    || ((texture->m_format&0x1F00)==0x1F00) // Rg16f
		   )
		), ("Float textures do not support anisotropic filtering."));
	ICE_ASSERTF(!(
		   ((texture->m_swizzle&0xFF)!=0xE4)        // No remapping
		&& (   ((texture->m_format&0x1F00)==0x1A00) // Rgba16f
		    || ((texture->m_format&0x1F00)==0x1B00) // Rgba32f
		    || ((texture->m_format&0x1F00)==0x1C00) // R32f
		    || ((texture->m_format&0x1F00)==0x1F00) // Rg16f
		   )
		), ("Float textures do not support swizzles."));
	ICE_ASSERTF(!(
		   (   ((texture->m_format&0x1F00)==0x1400) // R16
		    || ((texture->m_format&0x1F00)==0x1500) // Rg16
		   )
		&& ((texture->m_swizzle&0xFF)!=0xE4)        // Supported 16-bit remaps
		&& ((texture->m_swizzle&0xFF)!=0x4E)
		&& ((texture->m_swizzle&0xFF)!=0xEE)
		&& ((texture->m_swizzle&0xFF)!=0x44)
		), ("Texture swizzle for 16-bit per component textures must move 16-bits."));
	ICE_ASSERTF(!(
		   ((texture->m_control2&0x4)!=0)           // Alpha test
		&& (   ((texture->m_format&0x1F00)==0x1A00) // Rgba16f
		    || ((texture->m_format&0x1F00)==0x1B00) // Rgba32f
		    || ((texture->m_format&0x1F00)==0x1C00) // R32f
		    || ((texture->m_format&0x1F00)==0x1F00) // Rg16f
		   )
		), ("Float textures do not support per-texture alpha test."));
	ICE_ASSERTF(!(
		   ((texture->m_format&0x4)!=0)             // Cube-mapping enabled.
		&& ((texture->m_size1&0xFFFF)!=(texture->m_size1>>16))
		), ("Cube-mapping requires textures with equal width and height."));

	CHECK_RESERVE(10);

	U32 baseOffset = texture->m_baseOffset;
	m_cmdptr[0] = CMD(kCmdTextureAddress + unit * 32, 0x1C);
	U32 format = texture->m_format;
	m_cmdptr[1] = baseOffset;
	U32 control1 = texture->m_control1;
	m_cmdptr[2] = format;
	U32 control2 = texture->m_control2;
	m_cmdptr[3] = control1;
	U32 swizzle = texture->m_swizzle;
	m_cmdptr[4] = control2;
	U32 filter = texture->m_filter;
	m_cmdptr[5] = swizzle;
	U32 size1 = texture->m_size1;
	m_cmdptr[6] = filter;
	m_cmdptr[7] = size1;
	U32 size2 = texture->m_size2;
	m_cmdptr[8] = CMD(kCmdTextureSize2 + unit * 4, 4);
	m_cmdptr[9] = size2;
	m_cmdptr += 10;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetTextureReducedBorderColor(U32 unit, U32 color)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdTextureBorderColor + unit * 32, 0x4);
	m_cmdptr[1] = color;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetTextureReducedBorderDepth(U32 unit, float depth)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdTextureBorderColor + unit * 32, 0x4);
	m_cmdptr[1] = U32(16777215.f * depth);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetTextureReducedColorKey(U32 unit, U32 color)
{
	CHECK_RESERVE(2);

	m_cmdptr[0] = CMD(kCmdTextureColorKeyColor + unit * 4, 4);
	m_cmdptr[1] = color;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetTextureReducedControl3(U32 unit, U32 control3Value)
{
	CHECK_RESERVE(2);

	m_cmdptr[0] = CMD(kCmdTextureControl3 + unit * 4, 4);
	m_cmdptr[1] = control3Value;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::DisableTexture(U32 unit)
{
	ICE_ASSERT(unit < 16);
	CHECK_RESERVE(2);

	m_cmdptr[0] = CMD(kCmdTextureControl2 + unit * 32, 4);
	m_cmdptr[1] = 0;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetVertexProgramTexture(U32 unit, const Texture *__restrict texture)
{
	ICE_ASSERT(unit < 4);
	ICE_ASSERT(texture != NULL);
	ICE_ASSERTF(((texture->m_format & 0x1F00) == 0x1B00) || ((texture->m_format & 0x1F00) == 0x1C00), ("Format must be Rgba32f or R32f"));
	ICE_ASSERTF((texture->m_format&0x4000)==0, ("Vertex textures must be normalized."));
	ICE_ASSERTF((texture->m_format&0x2000)!=0, ("Vertex textures must be linear."));
	ICE_ASSERTF((texture->m_format&0x30)!=0x30, ("Vertex textures cannot be 3D."));

	CHECK_RESERVE(9);

	U32 baseOffset = texture->m_baseOffset;
	U32 format = texture->m_format & 0xFFFFFFF3;
	m_cmdptr[1] = baseOffset;
	U32 control1 = texture->m_control1 & 0x00000F0F;
	m_cmdptr[2] = format;
	U32 control2 = texture->m_control2 & 0xFFFFFF80;
	m_cmdptr[3] = control1;
	U32 size2 = texture->m_size2 & 0x0000FFFF;
	m_cmdptr[4] = control2;
	U32 filter = texture->m_filter & 0x00001FFF;
	m_cmdptr[5] = size2;
	U32 size1 = texture->m_size1;
	m_cmdptr[6] = filter;
	U32 borderColor = texture->m_borderColor;
	m_cmdptr[7] = size1;
	m_cmdptr[8] = borderColor;
	m_cmdptr[0] = CMD(kCmdVertexProgramTextureAddress + unit * 32, 32);
	m_cmdptr += 9;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::DisableVertexProgramTexture(U32 unit)
{
	ICE_ASSERT(unit < 4);
	CHECK_RESERVE(2);

	m_cmdptr[0] = CMD(kCmdVertexProgramTextureControl2 + unit * 32, 4);
	m_cmdptr[1] = 0;
	m_cmdptr += 2;
}
#endif

// Refreshes the internal texture cache if a texture was modified after being set.
#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InvalidateTextureCache()
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdTextureCacheInvalidate, 4);
	m_cmdptr[1] = 1;
	m_cmdptr += 2;
}
#endif

// Refreshes the internal vertex texture cache if a vertex texture was modified after being set.
#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InvalidateVertexTextureCache()
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdTextureCacheInvalidate, 4);
	m_cmdptr[1] = 2;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InvalidateDepthCull()
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdInvalidateDepthCull, 4);
	m_cmdptr[1] = 0;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetDepthCullControl(DepthCullDirection dir, DepthCullFormat format)
{
	ICE_ASSERTF((dir == kCullLess) || (dir == kCullGreater), ("%i", dir));
	ICE_ASSERTF((format == kCullMsb) || (format == kCullLones), ("%i", format));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdDepthCullDepthControl, 4);
	m_cmdptr[1] = dir | (format << 4);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetStencilCullHint(ComparisonFunc func, U32 ref, U32 mask)
{
	ICE_ASSERTF((func == kFuncNever)
		|| (func == kFuncLess)
		|| (func == kFuncEqual)
		|| (func == kFuncLessEqual)
		|| (func == kFuncGreater)
		|| (func == kFuncNotEqual)
		|| (func == kFuncGreaterEqual)
		|| (func == kFuncAlways)
		, ("%i", func));
	ICE_ASSERTF((ref & ~0xFF) == 0, ("%x", ref));
	ICE_ASSERTF((mask & ~0xFF) == 0, ("%x", mask));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdDepthCullStencilControl, 4);
	m_cmdptr[1] = (mask << 24) | (ref << 16) | (func & 0x00FF);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetDepthCullFeedback(U32 feedbackValue)
{
	CHECK_RESERVE(2);

	m_cmdptr[0] = CMD(kCmdDepthCullFeedbackControl, 4);
	m_cmdptr[1] = feedbackValue;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InsertReference(I32 value)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdReference, 4);
	m_cmdptr[1] = value;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetWaitSemaphoreIndex(U32 index)
{
	ICE_ASSERTF(index < kMaxSemaphores, ("%i", index));
	ICE_ASSERTF(index >= kNumReservedSemaphores, ("The first %i semaphores are reserved by the OS.", (U32)kNumReservedSemaphores));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdSemaphoreOffset, 4);
	m_cmdptr[1] = index * 16;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::WaitOnSemaphore(U32 value)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdWaitOnSemaphore, 4);
	m_cmdptr[1] = value;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetFlushPipeSemaphoreIndex(U32 index)
{
	ICE_ASSERTF(index < kMaxSemaphores, ("%i", index));
	ICE_ASSERTF(index >= kNumReservedSemaphores, ("The first %i semaphores are reserved by the OS.", (U32)kNumReservedSemaphores));

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdSyncSemaphoreOffset, 4);
	m_cmdptr[1] = index * 16;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::FlushTexturePipeAndWriteSemaphore(U32 value)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdSignalTextureReadSemaphore, 4);
	m_cmdptr[1] = value;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::FlushBackendPipeAndWriteSemaphore(U32 value)
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdSignalBackendWriteSemaphore, 4);
	m_cmdptr[1] = (value & 0xff00ff00) | ((value >> 16) & 0xff) | ((value & 0xff) << 16);
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InsertJump(U32 offset)
{
	CHECK_RESERVE(1);
	
	m_cmdptr[0] = offset | kCmdJumpFlag;
	m_cmdptr += 1;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InsertCall(U32 offset)
{
	CHECK_RESERVE(1);
	
	m_cmdptr[0] = offset | kCmdCallFlag;
	m_cmdptr += 1;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InsertReturn()
{
	CHECK_RESERVE(1);
	
	m_cmdptr[0] = kCmdReturnFlag;
	m_cmdptr += 1;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InsertNop()
{
	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdNoOperation, 4);
	m_cmdptr[1] = 0;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL U32* CommandContext::InsertHole(U32 size)
{
	// If asked for a size of 0, don't do anything. 
	if (size == 0) 
		return m_cmdptr;

	size >>= 2;

	// Ask for 3 more U32s for alignment purposes.
	CHECK_RESERVE_RET(size + 3, nullptr);

	// 0 -> 4 -> 0
	// 1 -> 3 -> 3
	// 2 -> 2 -> 2
	// 3 -> 1 -> 1
	U32F align = (4 - ((U32(m_cmdptr) >> 2) & 3)) & 3;

	switch (align) 
	{
  		case 3: *m_cmdptr++ = 0; /* fall through! */
 		case 2: *m_cmdptr++ = 0; /* fall through! */
		case 1: *m_cmdptr++ = 0; /* fall through! */
	}

	U32 *ptr = m_cmdptr;
	m_cmdptr += size;
	return ptr;
}
#endif

#if ICERENDER_SPU_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InsertData(const void *data, U32 size)
{
	CHECK_RESERVE(size/4);

#ifdef __SPU__
	const U32 *src = static_cast<const U32 *>(data);
	for (U32F i = 0; i < size/4; ++i)
		m_cmdptr[i] = src[i];
#else
	__builtin_memcpy(m_cmdptr, data, size);
#endif
	m_cmdptr += size/4;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL U32 *CommandContext::InsertSkipHole(U32 numBytes)
{
	ICE_ASSERT((numBytes & 0x3) == 0); // Alignment restriction
	ICE_ASSERT(numBytes <= 0x1FFC); // Hardware limit

	U32 numWords = numBytes >> 2;
	CHECK_RESERVE_RET(1+numWords, nullptr);
	
	m_cmdptr[0] = CMD(kCmdNoOperation, numBytes) | kCmdNoIncFlag;
	U32 *retVal = m_cmdptr+1;
	m_cmdptr += 1+numWords;
	return retVal;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::ResetReport(ReportType type)
{
	ICE_ASSERTF(type != kReportDepthCullFeedbackB, ("This report cannot be reset"));
	ICE_ASSERTF(type != kReportDepthCullFeedbackC, ("This report cannot be reset"));
	ICE_ASSERTF(type != kReportDepthCullFeedbackD, ("This report cannot be reset"));

	CHECK_RESERVE(2);

	m_cmdptr[0] = CMD(kCmdResetReport, 4);
	m_cmdptr[1] = type;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::WriteReport(ReportType type, U32 ioOffset)
{
	ICE_ASSERTF((ioOffset & 0x00000003) == 0, ("Offset must be 4-byte aligned."));
	ICE_ASSERTF((ioOffset & 0xFF000000) == 0, ("Only the first 16 megs are accessable by reports."));

	CHECK_RESERVE(2);

	m_cmdptr[0] = CMD(kCmdWriteReport, 4);
	m_cmdptr[1] = (U32(type) << 24) | ioOffset;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetRenderControl(RenderControlMode mode, U32 ioOffset)
{
	ICE_ASSERTF((ioOffset & 0x00000003) == 0, ("Offset must be 4-byte aligned."));
	ICE_ASSERTF((ioOffset & 0xFF000000) == 0, ("Only the first 16 megs are accessable by reports."));

	CHECK_RESERVE(4);
	
	m_cmdptr[0] = CMD(kCmdNoOperation, 4);
	m_cmdptr[1] = 0;
	m_cmdptr[2] = CMD(kCmdRenderControl, 4);
	m_cmdptr[3] = mode | ioOffset;
	m_cmdptr += 4;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetWindowClipMode(WindowClipMode mode)
{
	CHECK_RESERVE(2);

	m_cmdptr[0] = CMD(kCmdWindowClipControl, 4);
	m_cmdptr[1] = mode;
	m_cmdptr += 2;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::SetWindow(U32 index, U32 left, U32 top, U32 right, U32 bottom)
{
	ICE_ASSERT(index < 8);
	ICE_ASSERT(left < 0x1000);
	ICE_ASSERT(top < 0x1000);
	ICE_ASSERT(right < 0x1000);
	ICE_ASSERT(bottom < 0x1000);
	ICE_ASSERT(right >= left);
	ICE_ASSERT(bottom >= top);

	CHECK_RESERVE(3);

	m_cmdptr[0] = CMD(kCmdWindowClipLeftRight+index*8, 8);
	m_cmdptr[1] = (right << 16) | left;
	m_cmdptr[2] = (bottom << 16) | top;
	m_cmdptr += 3;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::InsertPpuInterrupt(U32 parameterValue)
{
	CHECK_RESERVE(4);

	m_cmdptr[0] = CMD(kCmdSetSwDriverSubchannel, 4);
	m_cmdptr[1] = 0xCAFEBABE;
	m_cmdptr[2] = CMD(kCmdSwDriverPpuInterrupt, 4);
	m_cmdptr[3] = parameterValue;
	m_cmdptr += 4;
}
#endif

#if ICERENDER_ASM==0
COMMANDCONTEXT_DECL void CommandContext::WritePerformanceReport()
{
	CHECK_RESERVE(4);
	
	m_cmdptr[0] = CMD(kCmdWaitForIdle, 4);
	m_cmdptr[1] = 0;
	m_cmdptr[2] = CMD(kCmdWritePerformanceReport, 4);
	m_cmdptr[3] = 1;
	m_cmdptr += 4;
}
#endif

#if ICERENDER_SPU_ASM==0
COMMANDCONTEXT_DECL void CommandContext::CopyMemoryImmediate(U32 dstOffset, CopyContext dstContext, void const *__restrict srcAdr, U32 sizeInDWords)
{
	if (sizeInDWords == 0)
		return;

	ICE_ASSERT((dstOffset & 3) == 0);
	ICE_ASSERT(sizeInDWords < 896);

	U32 sizeInWords = sizeInDWords*2;
	CHECK_RESERVE(13 + sizeInWords);
	
	m_cmdptr[0] = CMD(kCmdSurfaceSourceContext, 8);
	m_cmdptr[1] = dstContext;
	m_cmdptr[2] = dstContext;
	m_cmdptr[3] = CMD(kCmdSurfaceDestinationAddress, 4);
	m_cmdptr[4] = dstOffset & ~63;
	m_cmdptr[5] = CMD(kCmdSurfaceFormat, 8);
	m_cmdptr[6] = 0x0000000B;
	m_cmdptr[7] = 0x10001000;
	m_cmdptr[8] = CMD(kCmdBlitDestinationPoint, 12);
	m_cmdptr[9] = (dstOffset & 63) >> 2;
	m_cmdptr[10] = 0x00010000 | sizeInWords;
	m_cmdptr[11] = 0x00010000 | sizeInWords;
	m_cmdptr[12] = CMD(kCmdBlitData, sizeInDWords*8);
	m_cmdptr += 13;
	
#ifdef __SPU__
	// copy data into the command fifo
	U32 *__restrict src = (U32*)srcAdr;
	U32 *__restrict srcEnd = src + sizeInDWords*2;
	while(src < srcEnd)
		*m_cmdptr++ = *src++;
#else
	memcpy(m_cmdptr, srcAdr, sizeInDWords*8);
	m_cmdptr += sizeInWords;
#endif
	
}
#endif

COMMANDCONTEXT_DECL void CommandContext::CopyMemory(U32 dstOffset, CopyContext dstContext, I32 dstPitch, U32 srcOffset, CopyContext srcContext, I32 srcPitch, I32 bytesPerRow, I32 rowCount)
{
	CHECK_RESERVE(3);
	
	m_cmdptr[0] = CMD(kCmdHostToVidSourceContext, 8);
	m_cmdptr[1] = srcContext;
	m_cmdptr[2] = dstContext;
	m_cmdptr += 3;

	// can we turn this into a contigous blit ?
	if ((srcPitch == bytesPerRow) && (dstPitch == bytesPerRow))
	{
		bytesPerRow *= rowCount;
		rowCount = 1;
		srcPitch = 0;
		dstPitch = 0;
	}

	// unusual pitch values
	if ((srcPitch < -32768) || (srcPitch > 32767) || (dstPitch < -32768) || (dstPitch > 32767))
	{
		// Blit one line at a time
		while(--rowCount >= 0)
		{
			U32 cols;
			for(U32 colCount = bytesPerRow; colCount>0; colCount -= cols)
			{
				CHECK_RESERVE(9);
				
				cols = (colCount > 0x3fffff) ? 0x3fffff : colCount;
				m_cmdptr[0] = CMD(kCmdHostToVidSourceOffset, 0x20);
				m_cmdptr[1] = srcOffset + (bytesPerRow - colCount);
				m_cmdptr[2] = dstOffset + (bytesPerRow - colCount);
				m_cmdptr[3] = 0;
				m_cmdptr[4] = 0;
				m_cmdptr[5] = cols;
				m_cmdptr[6] = 1;
				m_cmdptr[7] = 0x101;
				m_cmdptr[8] = 0;
				m_cmdptr += 9;
			}

			dstOffset += dstPitch;
			srcOffset += srcPitch;
		}
	}
	else
	{
		// for each batch of rows
		U32 rows;
		for(;rowCount>0; rowCount -= rows)
		{
			// clamp to limit ?
			rows = (rowCount > 0x7ff) ? 0x7ff : rowCount;

			// for each batch of cols
			U32 cols;
			for(U32 colCount = bytesPerRow; colCount>0; colCount -= cols)
			{
				CHECK_RESERVE(9);
				
				cols = (colCount > 0x3fffff) ? 0x3fffff : colCount;
				m_cmdptr[0] = CMD(kCmdHostToVidSourceOffset, 0x20);
				m_cmdptr[1] = srcOffset + (bytesPerRow - colCount);
				m_cmdptr[2] = dstOffset + (bytesPerRow - colCount);
				m_cmdptr[3] = srcPitch;
				m_cmdptr[4] = dstPitch;
				m_cmdptr[5] = cols;
				m_cmdptr[6] = rows;
				m_cmdptr[7] = 0x101;
				m_cmdptr[8] = 0;
				m_cmdptr += 9;
			}

			// Advance to next set of rows
			srcOffset += rows * srcPitch;
			dstOffset += rows * dstPitch;
		}
	}

	CHECK_RESERVE(2);
	
	m_cmdptr[0] = CMD(kCmdHostToVidDestinationOffset, 4);
	m_cmdptr[1] = 0;
	m_cmdptr += 2;
}

COMMANDCONTEXT_DECL void CommandContext::CopyImage(U32 dstOffset, CopyContext dstContext, U32 dstPitch, U32 dstX, U32 dstY, U32 srcOffset, U32 srcContext, U32 srcPitch, U32 srcX, U32 srcY, U32 width, U32 height, U32 bytesPerPixel)
{
	ICE_ASSERT((dstOffset & 63) == 0);
	ICE_ASSERT((dstPitch & 63) == 0);
	ICE_ASSERT(srcPitch < 0xffff);
	ICE_ASSERT(dstPitch < 0xffff);

	// determine color formats
	U32 srcFormat, dstFormat;
	switch(bytesPerPixel)
	{
	case 2:
		srcFormat = 0x7;
		dstFormat = 0x4;
		break;
	case 4:
		srcFormat = 0x3;
		dstFormat = 0xA;
		break;
	default:
		ICE_ASSERT(!"Invalid bytes per pixel. Can only be 2 or 4.");
		return;
	}

	CHECK_RESERVE(6);
	
	m_cmdptr[0] = CMD(kCmdSurfaceDestinationContext, 4);
	m_cmdptr[1] = dstContext;
	m_cmdptr[2] = CMD(kCmdStretchBlitSourceContext, 4);
	m_cmdptr[3] = srcContext;
	m_cmdptr[4] = CMD(kCmdStretchBlitDestinationContext, 4);
	m_cmdptr[5] = 0x313371C3;
	m_cmdptr += 6;

	U32 finalDstX = dstX + width;
	U32 finalDstY = dstY + height;
	for(U32 y = dstY; y < finalDstY;)
	{
		U32 dstTop = y & ~0x3FF;
		U32 dstBot = dstTop + 0x400;
		U32 dstBltHeight = ((dstBot < finalDstY) ? dstBot : finalDstY) - y;
		for(U32 x = dstX; x < finalDstX;)
		{
			CHECK_RESERVE(20);
			
			U32 dstLeft = x & ~0x3FF;
			U32 dstRight = dstLeft + 0x400;
			U32 dstBltWidth = ((dstRight < finalDstX) ? dstRight : finalDstX) - x;

			// align the surface/destination surface properly
			U32 dstBlockOffset = bytesPerPixel * (dstLeft & ~0x3FF) + dstPitch * dstTop;
			U32 srcBlockOffset = bytesPerPixel * (srcX + x-dstX) + srcPitch * (srcY + y-dstY);
			U32 safeDstBltWidth = (dstBltWidth < 16) ? 16 : (dstBltWidth + 1) & ~1;

			m_cmdptr[0] = CMD(kCmdSurfaceFormat, 8);
			m_cmdptr[1] = dstFormat;
			m_cmdptr[2] = (dstPitch << 16) | dstPitch;
			m_cmdptr[3] = CMD(kCmdSurfaceDestinationAddress, 4);
			m_cmdptr[4] = dstOffset + dstBlockOffset;
			m_cmdptr[5] = CMD(kCmdStretchBlitConversion, 0x24);
			m_cmdptr[6] = 0x00000001;
			m_cmdptr[7] = srcFormat;
			m_cmdptr[8] = 0x00000003;
			m_cmdptr[9] = ((y - dstTop) << 16) | (x - dstLeft);
			m_cmdptr[10] = (dstBltHeight << 16) | dstBltWidth;
			m_cmdptr[11] = ((y - dstTop) << 16) | (x - dstLeft);
			m_cmdptr[12] = (dstBltHeight << 16) | dstBltWidth;
			m_cmdptr[13] = 0x00100000;
			m_cmdptr[14] = 0x00100000;
			m_cmdptr[15] = CMD(kCmdStretchBlitInSize, 0x10);
			m_cmdptr[16] = (dstBltHeight << 16) | safeDstBltWidth;
			m_cmdptr[17] = srcPitch | 0x00020000;
			m_cmdptr[18] = srcOffset + srcBlockOffset;
			m_cmdptr[19] = 0x00000000;
			m_cmdptr += 20;

			x += dstBltWidth;
		}
		y += dstBltHeight;
	}
}

COMMANDCONTEXT_DECL void CommandContext::CopyScaledImage(U32 dstOffset, CopyContext dstContext, U32 dstPitch, U32 dstWidth, U32 dstHeight, U32 srcOffset, CopyContext srcContext, U32 srcPitch, U32 bytesPerPixel, I32 log2XScale, I32 log2YScale, CopyFilterMode filter)
{
	ICE_ASSERT((dstOffset & 63) == 0);
	ICE_ASSERT((dstPitch & 63) == 0);
	ICE_ASSERT(srcPitch < 0xffff);
	ICE_ASSERT(dstPitch < 0xffff);

	// determine color format
	U32 srcFormat, dstFormat;
	switch(bytesPerPixel)
	{
	case 2:
		srcFormat = 0x7;
		dstFormat = 0x4;
		break;
	case 4:
		srcFormat = 0x3;
		dstFormat = 0xA;
		break;
	default:
		ICE_ASSERT(!"Invalid bytes per pixel. Can only be 2 or 4.");
		return;
	}

	CHECK_RESERVE(6);

	m_cmdptr[0] = CMD(kCmdSurfaceDestinationContext, 4);
	m_cmdptr[1] = dstContext;
	m_cmdptr[2] = CMD(kCmdStretchBlitSourceContext, 4);
	m_cmdptr[3] = srcContext;
	m_cmdptr[4] = CMD(kCmdStretchBlitDestinationContext, 4);
	m_cmdptr[5] = 0x313371C3;
	m_cmdptr += 6;

	U32 blockSizeX = 1 << (10 + log2XScale);
	U32 blockSizeY = 1 << (10 + log2YScale);
	U32 xoffset = filter ? (8<<(16-log2XScale)>>16) : 0;
	U32 yoffset = filter ? (8<<(16-log2YScale)>>16) : 0;

	for(U32 dsty = 0; dsty < dstHeight; dsty+=blockSizeY)
	{
		U32 srcy = (dsty << (16 - log2YScale)) >> 16;
		U32 dstBltHeight = dstHeight - dsty;
		if(dstBltHeight > blockSizeY) 
			dstBltHeight = blockSizeY;

		for(U32 dstx = 0; dstx < dstWidth;dstx+=blockSizeX)
		{
			CHECK_RESERVE(20);

			U32 srcx = (dstx << (16 - log2XScale)) >> 16;

			// determine this blits width
			U32 dstBltWidth = dstWidth - dstx;
			if(dstBltWidth > blockSizeX)
				dstBltWidth = blockSizeX;

			// align the surface/destination surface properly
			U32 dstBlockOffset = bytesPerPixel * dstx + dstPitch * dsty;
			U32 srcBlockOffset = bytesPerPixel * srcx + srcPitch * srcy;

			m_cmdptr[0] = CMD(kCmdSurfaceFormat, 8);
			m_cmdptr[1] = dstFormat;
			m_cmdptr[2] = (dstPitch << 16) | dstPitch;
			m_cmdptr[3] = CMD(kCmdSurfaceDestinationAddress, 4);
			m_cmdptr[4] = dstOffset + dstBlockOffset;
			m_cmdptr[5] = CMD(kCmdStretchBlitConversion, 0x24);
			m_cmdptr[6] = 0x00000001;
			m_cmdptr[7] = srcFormat;
			m_cmdptr[8] = 0x00000003;
			m_cmdptr[9] = 0x00000000;
			m_cmdptr[10] = (dstBltHeight << 16) | dstBltWidth;
			m_cmdptr[11] = 0x00000000;
			m_cmdptr[12] = (dstBltHeight << 16) | dstBltWidth;
			m_cmdptr[13] = 1 << (20 - log2XScale);
			m_cmdptr[14] = 1 << (20 - log2YScale);
			m_cmdptr[15] = CMD(kCmdStretchBlitInSize, 0x10);
			m_cmdptr[16] = 0x04000400;
			m_cmdptr[17] = srcPitch | 0x00020000 | filter;
			m_cmdptr[18] = srcOffset + srcBlockOffset;
			m_cmdptr[19] = (yoffset << 16) | xoffset;
			m_cmdptr += 20;
		}
	}
}

COMMANDCONTEXT_DECL void CommandContext::CopyAndSwizzleImage(U32 dstOffset, U32 dstWidth, U32 dstHeight, U32 dstX, U32 dstY, U32 srcOffset, U32 srcPitch, U32 srcX, U32 srcY, U32 width, U32 height, U32 bytesPerPixel)
{
	ICE_ASSERT(height && width);
	ICE_ASSERT((width<4096) && (height<4096));
	ICE_ASSERT((height <= dstHeight) && (width <= dstWidth));
	ICE_ASSERT(((dstY + height) <= dstHeight) && ((dstX + width) <= dstWidth));
	ICE_ASSERT(((dstWidth & (dstWidth - 1)) == 0) && (((dstHeight & (dstHeight - 1)) == 0)));
	ICE_ASSERT(srcPitch < 0xffff);
	ICE_ASSERT((bytesPerPixel == 2) || (bytesPerPixel == 4));

#ifdef __SPU__
	U32 dstwlog2 = 31 - ({__asm__("clz %0,%1" : "=r" (dstwlog2) : "r" (dstWidth)); dstwlog2;});
	U32 dsthlog2 = 31 - ({__asm__("clz %0,%1" : "=r" (dsthlog2) : "r" (dstHeight)); dsthlog2;});
#else
	U32 dstwlog2 = 31 - ({__asm__("cntlzw %0,%1" : "=r" (dstwlog2) : "r" (dstWidth)); dstwlog2;});
	U32 dsthlog2 = 31 - ({__asm__("cntlzw %0,%1" : "=r" (dsthlog2) : "r" (dstHeight)); dsthlog2;});
#endif

	if ((dstwlog2 <= 1) || (dsthlog2 == 0))
	{
		U32 dstPitch = bytesPerPixel << dstwlog2;
		srcOffset += srcX * bytesPerPixel + srcY * srcPitch;
		dstOffset += dstX * bytesPerPixel + dstY * dstPitch;

		for(U32 linesLeft = height; linesLeft; )
		{
			U32 actualHeight = (linesLeft > 2047) ? 2047 : linesLeft;

			CopyMemory(dstOffset, kCopyVideoMemory, dstPitch, srcOffset, kCopyMainMemory, srcPitch, width*bytesPerPixel, actualHeight);
			srcOffset += actualHeight * srcPitch;
			dstOffset += actualHeight * dstPitch;
			linesLeft -= actualHeight;
		}
		return;
	}

	ICE_ASSERT((dstWidth >= 4) && (dstHeight >= 2));

	U32 srcFormat, dstFormat;
	switch(bytesPerPixel)
	{
	case 2:
		srcFormat = 0x7;
		dstFormat = 0x4;
		break;
	case 4:
		srcFormat = 0x3;
		dstFormat = 0xA;
		break;
	default:
		ICE_ASSERT(!"Invalid bytes per pixel. Can only be 2 or 4.");
		return;
	}

	CHECK_RESERVE(4);

	m_cmdptr[0] = CMD(kCmdStretchBlitSourceContext, 4);
	m_cmdptr[1] = 0xFEED0001;
	m_cmdptr[2] = CMD(kCmdStretchBlitDestinationContext, 4);
	m_cmdptr[3] = 0x31337A73;
	m_cmdptr += 4;

	U32 logWidthLimit = (dstwlog2 > 10 ) ? 10 : dstwlog2;
	U32 logHeightLimit = (dsthlog2 > 10 ) ? 10 : dsthlog2;

	// align the Src Blt to the Dst, that way we can forget about srcX and srcY.
	U32 origSrcOffset = srcOffset;
	srcOffset += (srcX - dstX) * bytesPerPixel + (srcY - dstY) * srcPitch;

	U32 xEnd = dstX + width;
	U32 yEnd = dstY + height;
	U32 yTop = dstY & ~0x3FF;
	for(U32 y = dstY; y < yEnd;)
	{
		U32 yBottom = yTop + 0x400;
		if(yBottom > (1ul << dsthlog2))
			yBottom = (1 << dsthlog2);
		U32 bltHeight = (yBottom > yEnd) ? yEnd - y : yBottom - y;

		U32 xLeft = dstX & ~0x3FF;
		for(U32 x = dstX; x < xEnd;)
		{
			U32 xRight = xLeft + 0x400;
			U32 bltWidth = (xRight > xEnd ) ? xEnd - x : xRight - x;

			U32 blockDstOffset;
			if (!dstwlog2)
			{
				blockDstOffset = dstOffset + yTop * bytesPerPixel;
			}
			else if (!dsthlog2)
			{
				blockDstOffset = dstOffset + xLeft * bytesPerPixel;
			}
			else
			{
				// #'common' bits
				U32 log = (dstwlog2 < dsthlog2) ? dstwlog2 : dsthlog2;  
				// # of bits to interleave
				U32 doubleLog = log << 1;                     
				// bits to preserve
				U32 upperMask = ~((1 << doubleLog) - 1);      
				// bits to interleave
				U32 lowerMask = ~upperMask;                   

				U32 upperU = (xLeft << log) & upperMask;
				U32 upperV = (yTop << log) & upperMask;
				U32 lower = ((xLeft & 0x400) << 10) | ((yTop & 0x400) << 11) | ((xLeft & 0x800) << 11) | ((yTop & 0x800) << 12);
				ICE_ASSERT((xLeft < 4096) && (yTop < 4096));
				blockDstOffset = dstOffset + ((lower & lowerMask) | upperU | upperV) * bytesPerPixel;
			}

			ICE_ASSERT((blockDstOffset & 0x3f) == 0); // ** SERIOUS (RENDERING) ERROR **

			// clip - blockX and blockY are the X and Y offsets within this block
			U32 blockX = x & 0x3FF;
			U32 blockY = y & 0x3FF;

			// compute blt location in src
			U32 blockSrcOffset = srcOffset + x * bytesPerPixel + y * srcPitch;
			ICE_ASSERT(blockSrcOffset >= origSrcOffset);

			// handle bizarre class behavior
			U32 srcWidth = (bltWidth < 16) ? 16 : (bltWidth + 1) & ~1;

			// set dst format/offset
			CHECK_RESERVE(18);

			m_cmdptr[0] = CMD(kCmdSwizzleSurfaceFormat, 8);
			m_cmdptr[1] = dstFormat | (logWidthLimit << 16) | (logHeightLimit << 24);
			m_cmdptr[2] = blockDstOffset;
			m_cmdptr[3] = CMD(kCmdStretchBlitConversion, 0x24);
			m_cmdptr[4] = 0x00000001;
			m_cmdptr[5] = srcFormat;
			m_cmdptr[6] = 0x00000003;
			m_cmdptr[7] = (blockY << 16) | blockX;
			m_cmdptr[8] = (bltHeight << 16) | bltWidth;
			m_cmdptr[9] = (blockY << 16) | blockX;
			m_cmdptr[10] = (bltHeight << 16) | bltWidth;
			m_cmdptr[11] = 0x00100000;
			m_cmdptr[12] = 0x00100000;
			m_cmdptr[13] = CMD(kCmdStretchBlitInSize, 0x10);
			m_cmdptr[14] = (bltHeight << 16) | srcWidth;
			m_cmdptr[15] = srcPitch | 0x00020000;
			m_cmdptr[16] = blockSrcOffset;
			m_cmdptr[17] = 0x00000000;
			m_cmdptr += 18;

			x = xLeft = xRight;
		}
		y = yTop = yBottom;
	}
}


// Inserts a Nop with a special token and a string insertion, 
// this is then	interpreted by the command dissassembler and output
// appropriately.
COMMANDCONTEXT_DECL void CommandContext::InsertDebugString(const char *debugString)
{
	// Get the length of the debug string we wish to insert into the push buffer
	U32 len = (U32)strlen(debugString)+1;
	// Allocate space for the length of the string + Nop + special Code
	U32 cmdCount = 2+((len+3)/4);
	// Make sure we have enough room to insert it
	CHECK_RESERVE(cmdCount);
	// Insert the Nop
	m_cmdptr[0] = CMD(kCmdNoOperation, (cmdCount-1)*4) | kCmdNoIncFlag;
	// Insert the special code for the push buffer disassembler.
	m_cmdptr[1] = 0xDB65DB65;
#ifdef __SPU__
	// Insert the string itself
	for(U32F i = 0; i < (len+3)/4; ++i)
		m_cmdptr[2+i] = ((U32*)debugString)[i];
#else
	memcpy(&m_cmdptr[2], debugString, len);
#endif
	// Increment the command pointer
	m_cmdptr += cmdCount;
}

#undef CHECK_RESERVE_BYTES
#undef CHECK_RESERVE
#undef CHECK_RESERVE_RET
#undef COMMANDCONTEXT_DECL
