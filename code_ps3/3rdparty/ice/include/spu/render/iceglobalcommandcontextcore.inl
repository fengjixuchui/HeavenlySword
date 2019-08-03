/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

static inline bool Reserve(U32 wordCount)
{
	return static_cast<ICERENDER_TYPE>(g_currentCommandContext)->Reserve(wordCount);
}

static inline void SetRenderTarget(RenderTarget const *target)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetRenderTarget(target);
}

static inline void EnableRenderState(RenderState state)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->EnableRenderState(state);
}

static inline void DisableRenderState(RenderState state)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->DisableRenderState(state);
}

static inline void ToggleRenderState(RenderState state, bool enable)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->ToggleRenderState(state, enable);
}

static inline void SetDepthMinMaxControl(bool cullNearFarEnable, bool clampEnable, bool cullIgnoreW)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetDepthMinMaxControl(cullNearFarEnable, clampEnable, cullIgnoreW);
}

static inline void SetDrawBufferMask(U32 mask)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetDrawBufferMask(mask);
}

static inline void SetAlphaFunc(ComparisonFunc func, U32 ref)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetAlphaFunc(func, ref);
}

static inline void SetMrtBlendEnable(U32 mask)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetMrtBlendEnable(mask);
}

static inline void SetBlendFunc(BlendFactor src, BlendFactor dst)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetBlendFunc(src, dst);
}

static inline void SetBlendFuncSeparate(BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcAlpha, BlendFactor dstAlpha)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetBlendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha);
}

static inline void SetBlendEquation(BlendEquation equation)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetBlendEquation(equation);
}

static inline void SetBlendEquationSeparate(BlendEquation equationRgb, BlendEquation equationAlpha)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetBlendEquationSeparate(equationRgb, equationAlpha);
}

static inline void SetBlendColor(ArgbColor color)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetBlendColor(color);
}

static inline void SetFloatBlendColor(half red, half green, half blue, half alpha)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetFloatBlendColor(red, green, blue, alpha);
}

static inline void SetColorMask(bool red, bool green, bool blue, bool alpha)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetColorMask(red, green, blue, alpha);
}

static inline void SetMrtColorMask(U16 mask)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetMrtColorMask(mask);
}

static inline void SetDepthMask(bool mask)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetDepthMask(mask);
}

static inline void SetStencilMask(U32 mask)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetStencilMask(mask);
}

static inline void SetStencilMaskSeparate(U32 maskFront, U32 maskBack)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetStencilMaskSeparate(maskFront, maskBack);
}

static inline void SetStencilFunc(ComparisonFunc func, U32 ref, U32 mask)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetStencilFunc(func, ref, mask);
}

static inline void SetStencilFuncSeparate(ComparisonFunc funcFront, U32 refFront, U32 maskFront, ComparisonFunc funcBack, U32 refBack, U32 maskBack)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetStencilFuncSeparate(funcFront, refFront, maskFront, funcBack, refBack, maskBack);
}

static inline void SetStencilOp(StencilOp sfail, StencilOp dfail, StencilOp dpass)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetStencilOp(sfail, dfail, dpass);
}

static inline void SetStencilOpSeparate(StencilOp sfailFront, StencilOp dfailFront, StencilOp dpassFront, StencilOp sfailBack, StencilOp dfailBack, StencilOp dpassBack)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetStencilOpSeparate(sfailFront, dfailFront, dpassFront, sfailBack, dfailBack, dpassBack);
}

static inline void SetShadeModel(ShadeModel model)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetShadeModel(model);
}

static inline void SetLogicOp(LogicOp op)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetLogicOp(op);
}

static inline void SetFogMode(FogMode mode)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetFogMode(mode);
}

static inline void SetFogRange(float fmin, float fmax)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetFogRange(fmin, fmax);
}

static inline void SetFogDensity(float density)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetFogDensity(density);
}

static inline void SetDepthBounds(float dmin, float dmax)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetDepthBounds(dmin, dmax);
}

static inline void SetViewport(U32 left, U32 top, U32 width, U32 height, float dmin = 0.0F, float dmax = 1.0F)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetViewport(left, top, width, height, dmin, dmax);
}

static inline void SetViewport(U32 left, U32 top, U32 width, U32 height, float dmin, float dmax, float scale[4], float bias[4])
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetViewport(left, top, width, height, dmin, dmax, scale, bias);
}

static inline void SetScissor(U32 left, U32 top, U32 width, U32 height)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetScissor(left, top, width, height);
}

static inline void SetDepthFunc(ComparisonFunc func)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetDepthFunc(func);
}

static inline void SetPolygonOffset(float factor, float units)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetPolygonOffset(factor, units);
}

static inline void SetPolygonStipplePattern(const void *pattern)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetPolygonStipplePattern(pattern);
}

static inline void SetLineStipplePattern(U32 factor, U32 pattern)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetLineStipplePattern(factor, pattern);
}

static inline void SetLineWidth(U32 width)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetLineWidth(width);
}

static inline void SetClipFunc(ClipFunc func0, ClipFunc func1, ClipFunc func2, ClipFunc func3, ClipFunc func4, ClipFunc func5)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetClipFunc(func0, func1, func2, func3, func4, func5);
}

static inline void SetPrimitiveRestartIndex(U32 index)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetPrimitiveRestartIndex(index);
}

static inline void SetPolygonMode(PolygonMode mode)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetPolygonMode(mode);
}

static inline void SetPolygonModeSeparate(PolygonMode modeFront, PolygonMode modeBack)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetPolygonModeSeparate(modeFront, modeBack);
}

static inline void SetCullFace(CullFace cull)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetCullFace(cull);
}

static inline void SetFrontFace(FrontFace front)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetFrontFace(front);
}

static inline void SetClearColor(ArgbColor color)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetClearColor(color);
}

static inline void SetClearDepthStencil(U32 value)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetClearDepthStencil(value);
}

static inline void Clear(U32 flags)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->Clear(flags);
}

static inline void SetMultisampleParameters(bool enabled, bool alphaToCoverage, bool alphaToOne, U16 mask)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetMultisampleParameters(enabled, alphaToCoverage, alphaToOne, mask);
}

static inline void SetPointSize(float psize)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetPointSize(psize);
}

static inline void SetPointSpriteParameters(bool enabled, U32 texcoordMask, SpriteMode mode)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetPointSpriteParameters(enabled, texcoordMask, mode);
}

static inline void SetVertexAttribPointer(U32 index, U32 offset, AttribContext context)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttribPointer(index, offset, context);
}

static inline void SetVertexAttribBaseOffset(U32 baseOffset)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttribBaseOffset(baseOffset);
}

static inline void SetVertexAttribFormat(U32 index, AttribType type, AttribCount count, U32 stride, U32 divider = 0)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttribFormat(index, type, count, stride, divider);
}

static inline void SetVertexAttribFrequencyMode(U32 mode)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttribFrequencyMode(mode);
}

static inline void DisableVertexAttribArray(U32 index)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->DisableVertexAttribArray(index);
}

static inline void SetVertexAttrib1f(U32 index, float x)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib1f(index, x);
}

static inline void SetVertexAttrib1f(U32 index, const float *data)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib1f(index, data);
}

static inline void SetVertexAttrib2f(U32 index, float x, float y)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib2f(index, x, y);
}

static inline void SetVertexAttrib2f(U32 index, const float *data)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib2f(index, data);
}

static inline void SetVertexAttrib3f(U32 index, float x, float y, float z)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib3f(index, x, y, z);
}

static inline void SetVertexAttrib3f(U32 index, const float *data)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib3f(index, data);
}

static inline void SetVertexAttrib4f(U32 index, float x, float y, float z, float w)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib4f(index, x, y, z, w);
}

static inline void SetVertexAttrib4f(U32 index, const float *data)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib4f(index, data);
}

static inline void SetVertexAttrib2s(U32 index, I16 x, I16 y)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib2s(index, x, y);
}

static inline void SetVertexAttrib4s(U32 index, I16 x, I16 y, I16 z, I16 w)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib4s(index, x, y, z, w);
}

static inline void SetVertexAttrib4Ns(U32 index, I16 x, I16 y, I16 z, I16 w)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib4Ns(index, x, y, z, w);
}

static inline void SetVertexAttrib4Nub(U32 index, U8 x, U8 y, U8 z, U8 w)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib4Nub(index, x, y, z, w);
}

static inline void SetVertexAttrib4Nub(U32 index, U32 xyzwData)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexAttrib4Nub(index, xyzwData);
}

static inline void DrawArrays(DrawMode mode, U32 start, U32 count)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->DrawArrays(mode, start, count);
}

static inline void DrawElements(DrawMode mode, U32 start, U32 count, IndexType type, U32 indexOffset, IndexContext indexContext)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->DrawElements(mode, start, count, type, indexOffset, indexContext);
}

static inline void DrawInstancedElements(DrawMode mode, U32 indexStart, U32 indexCount, IndexType type, U32 indexOffset, IndexContext indexContext, U32 divider, U32 instanceStart, U32 instanceCount)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->DrawInstancedElements(mode, indexStart, indexCount, type, indexOffset, indexContext, divider, instanceStart, instanceCount);
}

static inline void Begin(DrawMode mode)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->Begin(mode);
}

static inline void InsertIndexArrayOffsetAndFormat(IndexType type, U32 offset, IndexContext context)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertIndexArrayOffsetAndFormat(type, offset, context);
}

static inline void InsertDraw(DrawType drawType, U32 start, U32 count)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertDraw(drawType, start, count);
}

static inline void End()
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->End();
}

static inline void DrawImmediateArrays(DrawMode mode, const void *array, U32 size)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->DrawImmediateArrays(mode, array, size);
}

static inline void DrawImmediateElements(DrawMode mode, U32 count, IndexType type, const void *index)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->DrawImmediateElements(mode, count, type, index);
}

static inline void InvalidatePreTransformCache()
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InvalidatePreTransformCache();
}

static inline void InvalidatePostTransformCache()
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InvalidatePostTransformCache();
}

static inline void SetVertexProgramStaticBranchBits(U32 branchBits)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexProgramStaticBranchBits(branchBits);
}

static inline void SelectVertexProgram(VertexProgram const *program)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SelectVertexProgram(program);
}

static inline void LoadVertexProgram(VertexProgram const *program)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->LoadVertexProgram(program);
}

static inline void SetVertexProgram(VertexProgram const *program)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexProgram(program);
}

static inline void SetVertexProgramConstant(U32 index, float x, float y, float z, float w)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexProgramConstant(index, x, y, z, w);
}

static inline void SetVertexProgramConstant(U32 index, const float *data)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexProgramConstant(index, data);
}

static inline void SetVertexProgram2Constants(U32 index, const float *data)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexProgram2Constants(index, data);
}

static inline void SetVertexProgram3Constants(U32 index, const float *data)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexProgram3Constants(index, data);
}

static inline void SetVertexProgram4Constants(U32 index, const float *data)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexProgram4Constants(index, data);
}

static inline void SetVertexProgramConstants(U32 start, U32 count, const float *data)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexProgramConstants(start, count, data);
}


static inline void SetFragmentProgram(const FragmentProgram *program)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetFragmentProgram(program);
}

#ifndef __SPU__
static inline void SetNullFragmentProgram()
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetNullFragmentProgram();
}
#endif

static inline void RefreshFragmentProgram(const FragmentProgram *program)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->RefreshFragmentProgram(program);
}

static inline void SetFragmentProgramConstant(FragmentProgram *program, U32 index, float x, float y, float z, float w, bool noProgramChange=false)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetFragmentProgramConstant(program, index, x, y, z, w, noProgramChange);
}

static inline void SetFragmentProgramConstant(FragmentProgram *program, U32 index, const float *data, bool noProgramChange=false)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetFragmentProgramConstant(program, index, data, noProgramChange);
}

static inline void SetFragmentProgramConstants(FragmentProgram *program, U32 start, U32 count, const float *data, bool noProgramChange=false)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetFragmentProgramConstants(program, start, count, data, noProgramChange);
}

static inline void SetTextureCylindricalWrapEnable(U32 enable0to7, U32 enable8to9)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetTextureCylindricalWrapEnable(enable0to7, enable8to9);
}

static inline void SetTexture(U32 unit, const Texture *texture)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetTexture(unit, texture);
}

static inline void SetTextureReduced(U32 unit, const TextureReduced *texture)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetTextureReduced(unit, texture);
}

static inline void SetTextureReducedBorderColor(U32 unit, U32 color)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetTextureReducedBorderColor(unit, color);
}

static inline void SetTextureReducedBorderDepth(U32 unit, float depth)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetTextureReducedBorderDepth(unit, depth);
}

static inline void SetTextureReducedColorKey(U32 unit, U32 color)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetTextureReducedColorKey(unit, color);
}

static inline void SetTextureReducedControl3(U32 unit, U32 control3Value)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetTextureReducedControl3(unit, control3Value);
}

static inline void DisableTexture(U32 unit)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->DisableTexture(unit);
}

static inline void SetVertexProgramTexture(U32 unit, const Texture *texture)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetVertexProgramTexture(unit, texture);
}

static inline void DisableVertexProgramTexture(U32 unit)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->DisableVertexProgramTexture(unit);
}

static inline void InvalidateTextureCache()
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InvalidateTextureCache();
}

static inline void InvalidateVertexTextureCache()
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InvalidateVertexTextureCache();
}

static inline void InvalidateDepthCull()
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InvalidateDepthCull();
}

static inline void SetDepthCullControl(DepthCullDirection dir, DepthCullFormat format)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetDepthCullControl(dir, format);
}

static inline void SetStencilCullHint(ComparisonFunc func, U32 ref, U32 mask)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetStencilCullHint(func, ref, mask);
}

static inline void SetDepthCullFeedback(U32 feedbackValue)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetDepthCullFeedback(feedbackValue);
}


static inline void InsertReference(I32 value)
{
	return static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertReference(value);
}

static inline void SetWaitSemaphoreIndex(U32 index)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetWaitSemaphoreIndex(index);
}

static inline void WaitOnSemaphore(U32 value)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->WaitOnSemaphore(value);
}

static inline void SetFlushPipeSemaphoreIndex(U32 index)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetFlushPipeSemaphoreIndex(index);
}

static inline void FlushTexturePipeAndWriteSemaphore(U32 value)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->FlushTexturePipeAndWriteSemaphore(value);
}

static inline void FlushBackendPipeAndWriteSemaphore(U32 value)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->FlushBackendPipeAndWriteSemaphore(value);
}

static inline void InsertJump(U32 offset)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertJump(offset);
}

static inline void InsertCall(U32 offset)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertCall(offset);
}

static inline void InsertReturn()
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertReturn();
}

static inline void InsertNop()
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertNop();
}

static inline U32 *InsertHole(U32 size)
{
	return static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertHole(size);
}

static inline void InsertData(const void *data, U32 size)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertData(data, size);
}

static inline U32 *InsertSkipHole(U32 numBytes)
{
	return static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertSkipHole(numBytes);
}

static inline void ResetReport(ReportType type)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->ResetReport(type);
}

static inline void WriteReport(ReportType type, U32 ioOffset)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->WriteReport(type, ioOffset);
}

static inline void SetRenderControl(RenderControlMode mode, U32 ioOffset)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetRenderControl(mode, ioOffset);
}

static inline void SetWindowClipMode(WindowClipMode mode)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetWindowClipMode(mode);
}

static inline void SetWindow(U32 index, U32 left, U32 top, U32 right, U32 bottom)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->SetWindow(index, left, top, right, bottom);
}

static inline void InsertPpuInterrupt(U32 parameterValue)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertPpuInterrupt(parameterValue);
}

static inline void WritePerformanceReport()
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->WritePerformanceReport();
}

static inline void CopyMemoryImmediate(U32 dstOffset, CopyContext dstContext, void const *__restrict src, U32 sizeInDWords)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->CopyMemoryImmediate(dstOffset, dstContext, src, sizeInDWords);
}

static inline void CopyMemory(U32 dstOffset, CopyContext dstContext, I32 dstPitch, U32 srcOffset, CopyContext srcContext, I32 srcPitch, I32 bytesPerRow, I32 rowCount)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->CopyMemory(dstOffset, dstContext, dstPitch, srcOffset, srcContext, srcPitch, bytesPerRow, rowCount);
}

static inline void CopyImage(U32 dstOffset, CopyContext dstContext, U32 dstPitch, U32 dstX, U32 dstY, U32 srcOffset, U32 srcContext, U32 srcPitch, U32 srcX, U32 srcY, U32 width, U32 height, U32 bytesPerPixel)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->CopyImage(dstOffset, dstContext, dstPitch, dstX, dstY, srcOffset, srcContext, srcPitch, srcX, srcY, width, height, bytesPerPixel);
}

static inline void CopyScaledImage(U32 dstOffset, CopyContext dstContext, U32 dstPitch, U32 dstWidth, U32 dstHeight, U32 srcOffset, CopyContext srcContext, U32 srcPitch, U32 bytesPerPixel, I32 log2XScale, I32 log2YScale, CopyFilterMode filter)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->CopyScaledImage(dstOffset, dstContext, dstPitch, dstWidth, dstHeight, srcOffset, srcContext, srcPitch, bytesPerPixel, log2XScale, log2YScale, filter);
}

static inline void CopyAndSwizzleImage(U32 dstOffset, U32 dstWidth, U32 dstHeight, U32 dstX, U32 dstY, U32 srcOffset, U32 srcPitch, U32 srcX, U32 srcY, U32 width, U32 height, U32 bytesPerPixel)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->CopyAndSwizzleImage(dstOffset, dstWidth, dstHeight, dstX, dstY, srcOffset, srcPitch, srcX, srcY, width, height, bytesPerPixel);
}


static inline void InsertDebugString(const char *debugString)
{
	static_cast<ICERENDER_TYPE>(g_currentCommandContext)->InsertDebugString(debugString);
}

//! A helper function to write the depth cull report data needed to 
//! optimize depth cull processor performance.
/*! \param feedbackAIoOffset  The io offset of where you want to write depth cull feedback A.
	\param feedbackBIoOffset  The io offset of where you want to write depth cull feedback B.
*/
static inline void WriteDepthCullReports(U32 feedbackAIoOffset, U32 feedbackBIoOffset)
{
	WriteDepthCullReports(static_cast<ICERENDER_TYPE>(g_currentCommandContext), feedbackAIoOffset, feedbackBIoOffset);
}

