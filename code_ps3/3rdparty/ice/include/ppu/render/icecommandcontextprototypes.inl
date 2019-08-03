/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

struct CommandContext : public CommandContextData
{
	//! Called when commands can't be added because the buffer is full.
	bool ReserveFailed(U32 minimumSize);

	//! Returns true if there is not enough space available for writing.
	bool WillReserveFail(U32 wordCount)
	{
		return m_cmdptr + wordCount > m_endptr;
	}

	//! Returns true if there is not enough space available for writing.
	bool WillReserveBytesFail(U32 byteCount)
	{
		return (U32)m_cmdptr + byteCount > (U32)m_endptr;
	}

	//! Ensures the specified number of words are available for writing.
	/*! \param wordCount  The number of words to ensure for availability.
	*/
	bool Reserve(U32 wordCount)
	{
		if (m_cmdptr + wordCount > m_endptr) 
			return ReserveFailed(wordCount);
		else
			return true;
	}

	//! Ensures the specified number of bytes are available for writing.
	/*! \param byteCount  The number of bytes to ensure for availability.
	*/
	bool ReserveBytes(U32 byteCount)
	{
		if ((U32)m_cmdptr + byteCount > (U32)m_endptr) 
			return ReserveFailed(byteCount/4);
		else
			return true;
	}

	//! Initializes the command context.
	/*! \param buffer    The base address of the buffer.
	\param size      The total size of the buffer in bytes.
	\param callback  The callback to use when we run out of space.
	*/
	void Init(void *buffer, U32 size, CommandCallback callback = nullptr)
	{
		U32 *ptr = (U32*)buffer;
		m_beginptr = m_cmdptr = ptr;

		// Leave 4 bytes of space for a jump at the end of the buffer
		m_endptr = ptr + size / 4 - 1;

		m_callback = callback;
	}

	//! Resets the buffer insertion point to the beginning.
	void ResetBuffer()
	{
		m_cmdptr = m_beginptr;
	}

	//! Makes a render target object the current render target.
	/*! This takes a RenderTarget object, and puts its state into the command context.
		This is distinct from functions such as SetRenderTargetColorBuffers which
		affect the state of the render target object but do not affect the command
		context at all.
		\param target  A pointer to a render target object.
	*/
	void SetRenderTarget(const RenderTarget *target);

	//! Enables or Disables a render state.
	/*! \param state   The state to enable or disable.
	    \param enable  This is true if you want to enable the state, or false if you want to disable the state.
	*/
	void ToggleRenderState(RenderState state, bool enable);

	//! Enables a render state.
	/*! \param state  The state to enable.
	*/
	inline void EnableRenderState(RenderState state)
	{
		ToggleRenderState(state, true);
	}

	//! Disables a render state.
	/*! \param state  The state to disable.
	*/
	inline void DisableRenderState(RenderState state)
	{
		ToggleRenderState(state, false);
	}

	//! Controls the Rsx's treatment and/or modification of the depth values 
	//! output by a vertex program.
	/*! \param cullNearFarEnable  Enable/disable culling for the viewport near/far 
	                              clipping plane.
	    \param clampEnable        Enable/disable clamping of a primitive's depth 
	                              values such that they are inside the viewport.
	    \param cullIgnoreW        Disables the culling of primitives where all 
	                              vertexes have negative W values.
	*/
	void SetDepthMinMaxControl(bool cullNearFarEnable, bool clampEnable, bool cullIgnoreW);

	//! Sets the draw buffer mask for MRT rendering.
	/*! \param mask  Specifies to which buffers fragment program outputs are written.
	*/
	void SetDrawBufferMask(U32 mask);

	//! Sets the alpha function.
	/*! \param func  The comparison function.
		\param ref   The reference value; must be 0-255 for integer render targets or a 16-bit floating-point value for FP render targets.
	*/
	void SetAlphaFunc(ComparisonFunc func, U32 ref);

	//! Sets the blending enabled state for MRT rendering.
	/*! \param mask  Specifies to which buffers fragment program outputs are blended. Buffer 0 is the LSB.
	*/
	void SetMrtBlendEnable(U32 mask);

	//! Sets the blend factors.
	/*! \param src  The source blend factor.
		\param dst  The destination blend factor.
	*/
	void SetBlendFunc(BlendFactor src, BlendFactor dst);

	//! Sets the blend factors separately for RGB and alpha channels.
	/*! \param srcRgb    The source blend factor for the RGB channels.
		\param dstRgb    The destination blend factor for the RGB channels.
		\param srcAlpha  The source blend factor for the alpha channel.
		\param dstAlpha  The destination blend factor for the alpha channel.
	*/
	void SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha);

	//! Sets the blend equation.
	/*! \param equation  The blend equation.
	*/
	void SetBlendEquation(BlendEquation equation);

	//! Sets the blend equation separately for RGB and alpha channels.
	/*! \param equationRgb    The blend equation for the RGB channels.
		\param equationAlpha  The blend equation for the alpha channel.
	*/
	void SetBlendEquationSeparate(BlendEquation equationRGB, BlendEquation equationAlpha);

	//! Sets the blend constant color for integer render targets.
	/*! \param color  The constant blending color.
	*/
	void SetBlendColor(ArgbColor color);

	//! Sets the blend constant color for floating-point render targets.
	/*! \param red    The constant blending color's red component.
		\param green  The constant blending color's green component.
		\param blue   The constant blending color's blue component.
		\param alpha  The constant blending color's alpha component.
	*/
	void SetFloatBlendColor(half red, half green, half blue, half alpha);

	//! Sets the color write mask.
	/*! \param red    Specifies whether the red channel is written.
		\param green  Specifies whether the green channel is written.
		\param blue   Specifies whether the blue channel is written.
		\param alpha  Specifies whether the alpha channel is written.
	*/
	void SetColorMask(bool red, bool green, bool blue, bool alpha);

	//! Sets the color write mask for multiple render targets.
	/*! \param mask   Each bit of the mask corresponds to a different color of a different render target.
						It is of the following format:
			            
		Little Endian: 15            8 7             0
			Big Endian: 0             7 8             15
						+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
						|B G R A B G R A B G R A B G R A|
						+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			            
						where each bit corresponds to the following render target:

		Little Endian: 15            8 7             0
			Big Endian: 0             7 8             15
						+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
						|3 3 3 3 2 2 2 2 1 1 1 1 0 0 0 0|
						+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	*/
	void SetMrtColorMask(U16 mask);

	//! Sets the depth write mask.
	/*! \param mask  Specifies whether the depth channel is written.
	*/
	void SetDepthMask(bool mask);

	//! Sets the stencil write mask.
	/*! \param mask  Specifies which bits of the stencil channel are written; must be 0x00-0xFF.
	*/
	void SetStencilMask(U32 mask);

	//! Sets the stencil write mask separately for front- and back-facing primitives.
	/*! \param maskFront  Specifies which bits of the stencil channel are written for front-facing primitives; must be 0x00-0xFF.
		\param maskBack   Specifies which bits of the stencil channel are written for back-facing primitives; must be 0x00-0xFF.
	*/
	void SetStencilMaskSeparate(U32 maskFront, U32 maskBack);

	//! Sets the stencil function.
	/*! \param func  The comparison function.
		\param ref   The reference value; must be 0x00-0xFF.
		\param mask  The comparison mask; must be 0x00-0xFF.
	*/
	void SetStencilFunc(ComparisonFunc func, U32 ref, U32 mask);

	//! Sets the stencil function separately for front- and back-facing primitives.
	/*! \param funcFront  The comparison function for front-facing primitives.
		\param refFront   The reference value for front-facing primitives; must be 0x00-0xFF.
		\param maskFront  The comparison mask for front-facing primitives; must be 0x00-0xFF.
		\param funcBack   The comparison function for back-facing primitives.
		\param refBack    The reference value for back-facing primitives; must be 0x00-0xFF.
		\param maskBack   The comparison mask for back-facing primitives; must be 0x00-0xFF.
	*/
	void SetStencilFuncSeparate(ComparisonFunc funcFront, U32 refFront, U32 maskFront, ComparisonFunc funcBack, U32 refBack, U32 maskBack);

	//! Sets the stencil operation.
	/*! \param sfail  The operation to perform when the stencil test fails.
		\param dfail  The operation to perform when the stencil test passes and the depth test fails.
		\param dpass  The operation to perform when the stencil test passes and the depth test passes.
	*/
	void SetStencilOp(StencilOp sfail, StencilOp dfail, StencilOp dpass);

	//! Sets the stencil operation separately for front- and back-facing primitives.
	/*! \param sfailFront  The operation to perform when the stencil test fails for front-facing primitives.
		\param dfailFront  The operation to perform when the stencil test passes and the depth test fails for front-facing primitives.
		\param dpassFront  The operation to perform when the stencil test passes and the depth test passes for front-facing primitives.
		\param sfailBack   The operation to perform when the stencil test fails for back-facing primitives.
		\param dfailBack   The operation to perform when the stencil test passes and the depth test fails for back-facing primitives.
		\param dpassBack   The operation to perform when the stencil test passes and the depth test passes for back-facing primitives.
	*/
	void SetStencilOpSeparate(StencilOp sfailFront, StencilOp dfailFront, StencilOp dpassFront, StencilOp sfailBack, StencilOp dfailBack, StencilOp dpassBack);

	//! Sets the shading model.
	/*! \param model  The shading model; must be kShadeModelFlat or kShadeModelSmooth.
	*/
	void SetShadeModel(ShadeModel model);

	//! Sets the color logic operation.
	/*! \param model  The logic operation.
	*/
	void SetLogicOp(LogicOp op);

	//! Sets the fog mode.
	/*! \param mode  The fog mode.
	*/
	void SetFogMode(FogMode mode);

	//! Sets the fog range for linear fog.
	/*! \param fmin  The minimum fog distance.
		\param fmax  The maximum fog distance.
	*/
	void SetFogRange(float fmin, float fmax);

	//! Sets the fog density for exponential fog.
	/*! \param density  The fog density.
	*/
	void SetFogDensity(float density);

	//! Sets the depth bounds used by the depth bounds test.
	/*! \param dmin  The minimum depth value.
		\param dmax  The maximum depth value.
	*/
	void SetDepthBounds(float dmin, float dmax);

	//! Sets the viewport rectangle.
	/*! \param left    The left edge.
		\param top     The top edge.
		\param width   The width.
		\param height  The height.
		\param dmin    The minimum depth range value.
		\param dmax    The maximum depth range value.
	*/
	void SetViewport(U32 left, U32 top, U32 width, U32 height, float dmin = 0.0F, float dmax = 1.0F);

	//! Sets the viewport rectangle.
	/*! \param left    The left edge.
		\param top     The top edge.
		\param width   The width.
		\param height  The height.
		\param dmin    The minimum depth range value.
		\param dmax    The maximum depth range value.
		\param scale   An array of size 4 which contains the viewport scale calculation values. See below.
		\param bias    An array of size 4 which contains the viewport bias calculation values. See below.
		
		The viewport scale and bias values are components of 4x4 matrices.
		
		Scale matrix:
		scale[0] 0        0        0
		0        scale[1] 0        0
		0        0        scale[2] 0
		0        0        0        scale[3]

		Bias matrix:
		0  0  0  bias[0]
		0  0  0  bias[1]
		0  0  0  bias[2]
		0  0  0  bias[3]
		
		These values are typically computed as:
		scale[0] = width * 0.5
		scale[1] = height * -0.5
		scale[2] = (dmax - dmin) * 0.5
		scale[3] = 0
		
		bias[0] = left + width * 0.5
		bias[1] = top + height * 0.5
		bias[2] = (dmax - dmin) * 0.5
		bias[3] = 0
	*/
	void SetViewport(U32 left, U32 top, U32 width, U32 height, float dmin, float dmax, float scale[4], float bias[4]);

	//! Sets the scissor rectangle.
	/*! \param left    The left edge.
		\param top     The top edge.
		\param width   The width.
		\param height  The height.
	*/
	void SetScissor(U32 left, U32 top, U32 width, U32 height);

	//! Sets the depth function.
	/*! \param func  The depth comparison function.
	*/
	void SetDepthFunc(ComparisonFunc func);

	//! Sets the polygon offset parameters.
	/*! \param factor  The slope factor.
		\param units   The offset units.
	*/
	void SetPolygonOffset(float factor, float units);

	//! Sets the polygon stipple pattern.
	/*! \param pattern  A pointer to a 32x32 bit pattern (128 bytes).
	*/
	void SetPolygonStipplePattern(const void *pattern);

	//! Sets the line stipple pattern.
	/*! \param factor   The scale factor.
		\param pattern  The 16-bit pattern.
	*/
	void SetLineStipplePattern(U32 factor, U32 pattern);

	//! Sets the line width.
	/*! \param width  The line width. The lowest 3 bits are fraction (i.e., a value of 0x0008 specifies a width of 1).
	*/
	void SetLineWidth(U32 width);

	//! Sets all user clipping plane functions. Fragments are clipped when any one test fails.
	/*! \param func0  The clip test function for plane 0. The value kClipNone disables the clipping plane.
		\param func1  The clip test function for plane 1. The value kClipNone disables the clipping plane.
		\param func2  The clip test function for plane 2. The value kClipNone disables the clipping plane.
		\param func3  The clip test function for plane 3. The value kClipNone disables the clipping plane.
		\param func4  The clip test function for plane 4. The value kClipNone disables the clipping plane.
		\param func5  The clip test function for plane 5. The value kClipNone disables the clipping plane.
	*/
	void SetClipFunc(ClipFunc func0, ClipFunc func1, ClipFunc func2, ClipFunc func3, ClipFunc func4, ClipFunc func5);

	//! Sets the primitive restart index.
	/*! \param index  The index that triggers a primitive restart; can be any 32-bit value.
	*/
	void SetPrimitiveRestartIndex(U32 index);

	//! Sets the polygon mode.
	/*! \param mode  The polygon rendering mode.
	*/
	void SetPolygonMode(PolygonMode mode);

	//! Sets the polygon mode separately for front- and back-facing primitives.
	/*! \param modeFront  The polygon rendering mode for front-facing primitives.
		\param modeBack   The polygon rendering mode for back-facing primitives.
	*/
	void SetPolygonModeSeparate(PolygonMode modeFront, PolygonMode modeBack);

	//! Sets the cull face mode.
	/*! \param cull  Specifies which faces to cull; must be kCullFaceBack or kCullFaceFront.
	*/
	void SetCullFace(CullFace cull);

	//! Sets the front face mode.
	/*! \param front  Specifies which winding is front-facing; must be kFrontFaceCw or kFrontFaceCcw.
	*/
	void SetFrontFace(FrontFace front);

	//! Sets the clear color value.
	/*! \param color  The clear color value. This value is of the same format as the currently bound render target.
						Example: ARGB8888 for kBufferArgb8888, Rgb565 for kBufferRgb565.
	*/
	void SetClearColor(ArgbColor color);

	//! Sets the clear depth value.
	/*! \param value  The clear depth/stencil value. This must match the render target
						format. For D24S8, the high 24 bits are depth and the low 8 bits are stencil.
	*/
	void SetClearDepthStencil(U32 value);

	//! Clears the render target. (Does not work for FP render targets.)
	/*! \param flags  Specifies what channels to clear via kClear*
	*/
	void Clear(U32 flags);

	//! Sets the multisample coverage parameters.
	/*! \param enabled          Specifies whether multisampling is enabled.
		\param alphaToCoverage  Specifies whether the fragment alpha is used as the coverage.
		\param alphaToOne       Specifies whether the fragment alpha to replaced with 1.
		\param mask             The coverage mask.
	*/
	void SetMultisampleParameters(bool enabled, bool alphaToCoverage, bool alphaToOne, U16 mask);

	//! Sets the global point size.
	/*! \param psize  The point size; must be in the range [0.0, 63.375].
	*/
	void SetPointSize(float psize);

	//! Sets the point sprite parameters.
	/*! \param enabled       Specifies whether point sprites are enabled.
		\param texcoordMask  An 8-bit mask indicating which texture coordinates are replaced for point sprites.
								The lsb represents texcoord 0.
		\param mode          The R-coordinate replacement mode.
	*/
	void SetPointSpriteParameters(bool enabled, U32 texcoordMask, SpriteMode mode);


	//! Sets a vertex attribute array address without setting its format.
	/*! \param index    The index of the vertex attribute array; must be 0-15.
		\param offset   An offset into either video or main memory of the attribute array data.
		\param context  The location of the offset. (Main or Video Memory).
	*/
	void SetVertexAttribPointer(U32 index, const U32 offset, AttribContext context);
	
	//! Sets a vertex attribute array address offset where the specified offset
	//! is added to all the offsets of all enabled vertex attribute arrays.
	/*! \param baseOffset  The offset which is added to all enabled vertex 
	                       attribute arrays.
	*/
	void SetVertexAttribBaseOffset(U32 baseOffset);

	//! Sets a vertex attribute array format without setting its address.
	/*! \param index    The index of the vertex attribute array; must be 0-15.
		\param type     The type of the vertex attribute array.
		\param count    The number of components (0-4) per vertex; 11-11-10 format requires 1 here although it has 3 components.
		\param stride   The stride, in bytes, from one vertex to the next; must be 4-252 and a multiple of 4.
		\param divider  The 16-bit frequency divider. A value of 0 means the divider is disabled.
	*/
	void SetVertexAttribFormat(U32 index, AttribType type, AttribCount count, U32 stride, U32 divider = 0);

	//! Sets all vertex attribute array frequency divider modes.
	/*! \param mode  The 16-bit divider mode mask. The lsb corresponds to array 0.
						A 0 bit means divide, and a 1 bit means modulo.
	*/
	void SetVertexAttribFrequencyMode(U32 mode);

	//! Disables a vertex attribute array.
	/*! \param index  The index of the vertex attribute array; must be 0-15.
	*/
	void DisableVertexAttribArray(U32 index);

	//! Sets a vertex attribute using 1 floating-point value. The (y, z, w) components are set to (0, 0, 1).
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param x      The value of the x component.
	*/
	void SetVertexAttrib1f(U32 index, float x);

	//! Sets a vertex attribute using 1 floating-point value. The (y, z, w) components are set to (0, 0, 1).
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param v      A pointer to the array from which to read the value.
	*/
	void SetVertexAttrib1f(U32 index, const float *data);

	//! Sets a vertex attribute using 2 floating-point values. The (z, w) components are set to (0, 1).
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param x      The value of the x component.
		\param y      The value of the y component.
	*/
	void SetVertexAttrib2f(U32 index, float x, float y);

	//! Sets a vertex attribute using 2 floating-point values. The (z, w) components are set to (0, 1).
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param v      A pointer to the array from which to read the values.
	*/
	void SetVertexAttrib2f(U32 index, const float *data);

	//! Sets a vertex attribute using 3 floating-point values. The w component is set to 1.
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param x      The value of the x component.
		\param y      The value of the y component.
		\param z      The value of the z component.
	*/
	void SetVertexAttrib3f(U32 index, float x, float y, float z);

	//! Sets a vertex attribute using 3 floating-point values. The w component is set to 1.
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param v      A pointer to the array from which to read the values.
	*/
	void SetVertexAttrib3f(U32 index, const float *data);

	//! Sets a vertex attribute using 4 floating-point values.
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param x      The value of the x component.
		\param y      The value of the y component.
		\param z      The value of the z component.
		\param w      The value of the w component.
	*/
	void SetVertexAttrib4f(U32 index, float x, float y, float z, float w);

	//! Sets a vertex attribute using 4 floating-point values.
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param v      A pointer to the array from which to read the values.
	*/
	void SetVertexAttrib4f(U32 index, const float *data);

	//! Sets a vertex attribute using 2 signed 16-bit values. The (z, w) components are set to (0, 1).
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param x      The value of the x component.
		\param y      The value of the y component.
	*/
	void SetVertexAttrib2s(U32 index, I16 x, I16 y);

	//! Sets a vertex attribute using 4 signed 16-bit values.
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param x      The value of the x component.
		\param y      The value of the y component.
		\param z      The value of the z component.
		\param w      The value of the w component.
	*/
	void SetVertexAttrib4s(U32 index, I16 x, I16 y, I16 z, I16 w);

	//! Sets a vertex attribute using 4 signed 16-bit values, normalized to [-1, 1].
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param x      The value of the x component.
		\param y      The value of the y component.
		\param z      The value of the z component.
		\param w      The value of the w component.
	*/
	void SetVertexAttrib4Ns(U32 index, I16 x, I16 y, I16 z, I16 w);

	//! Sets a vertex attribute using 4 unsigned 8-bit values, normalized to [0, 1].
	/*! \param index  The index of the vertex attribute; must be 0-15.
		\param x      The value of the x component.
		\param y      The value of the y component.
		\param z      The value of the z component.
		\param w      The value of the w component.
	*/
	void SetVertexAttrib4Nub(U32 index, U8 x, U8 y, U8 z, U8 w);
	
	//! Sets a vertex attribute using 4 unsigned 8-bit values, normalized to [0, 1].
	/*! \param index     The index of the vertex attribute; must be 0-15.
		\param xyzwData	 The value of the components in xyzw form (8-bits per component).
	*/
	void SetVertexAttrib4Nub(U32 index, U32 xyzwData);

	//! Draws non-indexed primitives using the currently enabled vertex arrays.
	/*! \param mode   The primitive type.
		\param start  The starting position in the vertex arrays.
		\param count  The number of vertices to render.
	*/
	void DrawArrays(DrawMode mode, U32 start, U32 count);

	//! Draws indexed primitives using the currently enabled vertex arrays.
	/*! \param mode          The primitive type.
		\param start         The starting position in the index array.
		\param count         The number of vertices to render.
		\param type          The index type; must be kIndex16 or kIndex32.
		\param indexOffset   An offset into the specified context.
		\param indexContext  Specifies whether the index buffer lies in system or video memory.
	*/
	void DrawElements(DrawMode mode, U32 start, U32 count, IndexType type, U32 offset, IndexContext context);

	//! Draws indexed primitives using the currently enabled vertex arrays.
	/*! \param mode           The primitive type.
		\param indexStart     The starting position in the index array.
		\param indexCount     The number of vertices to render.
		\param type           The index type; must be kIndex16 or kIndex32.
		\param indexOffset    An offset into the specified context.
		\param indexContext   Specifies whether the index buffer lies in system or video memory.
		\param divider        The base index multiplier. Usually the same value as the frequency divider.
		\param instanceStart  The starting instance to render.
		\param instanceCount  The number of instances from the instanceStart to render.
	*/
	void DrawInstancedElements(DrawMode mode, U32 indexStart, U32 indexCount, IndexType type, U32 offset, IndexContext context, U32 divider, U32 instanceStart, U32 instanceCount);

	//! Begins drawing inlined non-indexed primitives using SetVertexAttrib* or
	//! InsertDraw().
	/*! \param mode  The primitive type.
	*/
	void Begin(DrawMode mode);

	//! Sets the index array offset and format for later use by InsertDraw().
	/*! \param type     The index type; must be kIndex16 or kIndex32.
		\param offset   An offset into the specified context.
		\param context  Specifies whether the index buffer lies in system or video memory.
	*/
	void InsertIndexArrayOffsetAndFormat(IndexType type, U32 offset, IndexContext context);

	//! Draws non-indexed or indexed primitives using the currently enabled vertex 
	//! arrays.
	/*! This must be used inside a Begin() and End().
	    \param drawType  The draw type; must be kDrawArrays or kDrawElements.
		\param start     The starting position in the vertex arrays.
		\param count     The number of vertices to render.
	*/
	void InsertDraw(DrawType drawType, U32 start, U32 count);

	//! Ends drawing inlined non-indexed primitives using SetVertexAttrib* or
	//! InsertDraw().
	void End();

	//! Draws inlined non-indexed primitives using the passed in interleaved vertex arrays.
	/*! \param mode   The primitive type.
		\param array  Interleaved vertex data.
		\param size   The size in bytes of the array to render.
	*/
	void DrawImmediateArrays(DrawMode mode, const void *array, U32 size);

	//! Draws indexed primitives using the currently enabled vertex arrays.
	/*! \param mode   The primitive type.
		\param count  The number of vertices to render.
		\param type   The index type; must be kIndex16 or kIndex32.
		\param index  A pointer to the index array to copy into the push buffer.
	*/
	void DrawImmediateElements(DrawMode mode, U32 count, IndexType type, const void *index);

	//! Refreshes the internal pre-transform vertex cache. 
	/*! This function is to be called if vertex data is modified and must be called *before*
		the data's first use. Typically in a double buffering scheme, this will only be called
		once per frame. 
	*/
	void InvalidatePreTransformCache();

	//! Refreshes the internal post-transform vertex cache. 
	void InvalidatePostTransformCache();

	//! Sets the static branching bits specified in a Cg vertex program as b0..b31. 
	/*! \param branchBits  The on/off flags for the vertex program to use. 
	*/
	void SetVertexProgramStaticBranchBits(U32 branchBits);

	//! Selects the vertex program to be used for subsequent rendering but does
	//! *not* load the vertex program into instruction slots. 
	//! WARNING!!! Assumes vertex program has previously been loaded with
	//! LoadVertexProgram() or SetVertexProgram().
	/*! \param program  A pointer to a vertex program object.
	*/
	void SelectVertexProgram(const VertexProgram *__restrict program);

	//! Loads a vertex program but does not set it as the active program
	//! to execute.
	/*! \param program  A pointer to a vertex program object.
	*/
	void LoadVertexProgram(const VertexProgram *__restrict program);
	
	//! Sets the vertex program to be used for subsequent rendering.
	/*! This takes a VertexProgram object, and puts its state into the command 
	    context.
		\param program  A pointer to a vertex program object.
	*/
	inline void SetVertexProgram(const VertexProgram *program)
	{
		LoadVertexProgram(program);
		SelectVertexProgram(program);
	}

	//! Sets a vertex program constant.
	/*! \param index  The register index of the constant being set; must be 0-467.
		\param x      The x component of the constant value.
		\param y      The y component of the constant value.
		\param z      The z component of the constant value.
		\param w      The w component of the constant value.
	*/
	void SetVertexProgramConstant(U32 index, float x, float y, float z, float w);

	//! Sets a vertex program constant.
	/*! \param index  The register index of the constant being set; must be 0-467.
		\param data   The constant value; must point to 4 floating-point values.
	*/
	void SetVertexProgramConstant(U32 index, const float *data);

	//! Sets two vertex program constants.
	/*! \param index  The register index of the constant being set; must be 0-467.
		\param data   The constant value; must point to 8 floating-point values.
	*/
	void SetVertexProgram2Constants(U32 index, const float *data);

	//! Sets three vertex program constants.
	/*! \param index  The register index of the constant being set; must be 0-467.
		\param data   The constant value; must point to 12 floating-point values.
	*/
	void SetVertexProgram3Constants(U32 index, const float *data);

	//! Sets four vertex program constants.
	/*! \param index  The register index of the constant being set; must be 0-467.
		\param data   The constant value; must point to 16 floating-point values.
	*/
	void SetVertexProgram4Constants(U32 index, const float *data);

	//! Sets multiple vertex program constants.
	/*! \param start  The register index of the first constant being set; must be 0-467.
		\param count  The number of constant registers being set; start + count cannot be greater than 468.
		\param data   The constant values; must point to count * 4 floating-point values.
	*/
	void SetVertexProgramConstants(U32 start, U32 count, const float *data);



	//! Sets the fragment program to be used for subsequent rendering.
	/*! This takes a FragmentProgram object, and puts its state into the command context.
		\param program  A pointer to a fragment program object.
	*/
	void SetFragmentProgram(const FragmentProgram *program);

#ifndef __SPU__
	//! Sets the null fragment program (which actually just copies color0).
	void SetNullFragmentProgram();
#endif

	//! Refreshes a fragment program whose constants have been changed.
	/*! \param program  A pointer to the fragment program object whose constant have been changed.
						This must be the same fragment program that was most recently set using the
						SetFragmentProgram() function.
	*/
	void RefreshFragmentProgram(const FragmentProgram *program);

	//! Sets a fragment program constant.
	/*! \param program          A pointer to the fragment program object for which the constant is being set.
		\param index            The patch index of the constant being set; must not exceed size of program's internal patch table.
		\param x                The x component of the constant value.
		\param y                The y component of the constant value.
		\param z                The z component of the constant value.
		\param w                The w component of the constant value.
		\param noProgramChange  If this is set, the gpu's view of which fragment program to write to is not changed. 
		                        IE, make sure that if noProgramChange is set that the program used is the same as the last program used!
	*/
	inline void SetFragmentProgramConstant(FragmentProgram *program, U32 index, float x, float y, float z, float w, bool noProgramChange=false)
	{
		float data[4] = {x,y,z,w};
		SetFragmentProgramConstant(program, index, data, noProgramChange);
	}

	//! Sets a fragment program constant.
	/*! \param program          A pointer to the fragment program object for which the constant is being set.
		\param index            The patch index of the constant being set; must not exceed size of program's internal patch table.
		\param data             The constant value; must point to 4 floating-point values.
		\param noProgramChange  If this is set, the gpu's view of which fragment program to write to is not changed. 
		                        IE, make sure that if noProgramChange is set that the program used is the same as the last program used!
	*/
	void SetFragmentProgramConstant(FragmentProgram *program, U32 index, const float *data, bool noProgramChange=false);

	//! Sets multiple fragment program constants.
	/*! \param program          A pointer to the fragment program object for which the constants are being set.
		\param start            The patch index of the first constant being set; must not exceed size of program's internal patch table.
		\param count            The number of constant values being set; start + count must not exceed size of program's internal patch table.
		\param data             The constant values; must point to count * 4 floating-point values.
		\param noProgramChange  If this is set, the gpu's view of which fragment program to write to is not changed. 
		                        IE, make sure that if noProgramChange is set that the program used is the same as the last program used!
	*/
	void SetFragmentProgramConstants(FragmentProgram *program, U32 start, U32 count, const float *data, bool noProgramChange=false);

	//! Enables cylindrical wrapping instead of normal wrapping on the specified channels and textures.
	/*! \param enable0to7  A bit per component per texture with the LSB as the x component of texture 0 and
	                       the MSB as the w component of texture 7. 
	                       Format is wzyxwzyxwzyxwzyxwzyxwzyxwzyxwzyx and 77776666555544443333222211110000.
	    \param enable8to9  A bit per component per texture. Format is wzyxwzyx and 99998888.
	*/
	void SetTextureCylindricalWrapEnable(U32 enable0to7, U32 enable8to9);

	//! Sets the texture for a specific texture unit.
	/*! This takes a Texture object, and puts its state into the command context.
		This is distinct from functions such as SetTextureImage* which affect the state
		of the texture object but do not affect the command context at all.
		\param unit     The index of the texture unit; must be 0-15.
		\param texture  A pointer to a texture object.
	*/
	void SetTexture(U32 unit, const Texture *texture);

	//! Sets a reduced texture for a specific texture unit.
	/*! This takes a TextureReduced object, and puts its state into the command context.
		This is distinct from functions such as SetTextureImage* which affect the state
		of the texture object but do not affect the command context at all.
		\param unit     The index of the texture unit; must be 0-15.
		\param texture  A pointer to a texture reduced object.
	*/
	void SetTextureReduced(U32 unit, const TextureReduced *texture);
	
	//! Sets a reduced texture border color for a specific texture unit.
	/*! \param unit   The index of the texture unit; must be 0-15.
	    \param color  A border color which is used with the kWrap*Border wrap modes.
	*/
	void SetTextureReducedBorderColor(U32 unit, U32 color);

	//! Sets a reduced texture border color as a depth value for a specific texture unit.
	/*! \param unit   The index of the texture unit; must be 0-15.
	    \param depth  A border color as a depth value which is used with the kWrap*Border wrap modes.
	*/
	void SetTextureReducedBorderDepth(U32 unit, float depth);

	//! Sets a reduced texture color key color for a specific texture unit.
	/*! This is of the same format as the texture.
		NOTE - This feature does not work with DXT or FP textures.
		NOTE - Color Keying occurs pre-filtering.
	    \param unit   The index of the texture unit; must be 0-15.
	    \param color  The color to use as the color key. The format of the 
	                  color is of the same format as the texture.
	*/
	void SetTextureReducedColorKey(U32 unit, U32 color);
	
	//! Sets a reduced texture control 3 value for a specific texture unit.
	/*! To generate this value use the CalculateTextureControl3() function.
	    \param unit           The index of the texture unit; must be 0-15.
	    \param control3Value  The control3Value to set. Use the 
	                          CalculateTextureControl3() function to calculate 
	                          this value.
	*/
	void SetTextureReducedControl3(U32 unit, U32 control3Value);

	//! Disables the texture for a specific texture unit.
	/*! This is distinct from functions such as SetTextureImage* which affect the state
		of the texture object but do not affect the command context at all.
		\param unit  The index of the texture unit; must be 0-15.
	*/
	void DisableTexture(U32 unit);

	//! Sets the texture for a specific vertex program texture unit.
	/*! This takes a Texture object, and puts its state into the command context.
		This is distinct from functions such as SetTextureImage* which affect the state
		of the texture object but do not affect the command context at all.
		\param unit	    The index of the texture unit; must be 0-3.
		\param texture  A pointer to a texture object.
	*/
	void SetVertexProgramTexture(U32 unit, const Texture *texture);

	//! Disables the texture for a specific vertex program texture unit.
	/*! \param unit  The index of the texture unit; must be 0-3.
	*/
	void DisableVertexProgramTexture(U32 unit);

	//! Refreshes the internal texture cache if a texture was modified after being set.
	void InvalidateTextureCache();

	//! Refreshes the internal vertex texture cache if a vertex texture was modified after being set.
	void InvalidateVertexTextureCache();



	//! Invalidates the depth-cull processor cache. 
	/*! Usually a depth clear initializes depth-cull into a valid state and this 
		method is not needed at all (unless you manually modify the depth buffer, 
		for example by using the PPU or SPU). 
	*/
	void InvalidateDepthCull();

	//! Sets the Depth-cull processor depth test direction and format.
	/*! \param dir     The depth test direction.
	    \param format  The format of the depth cull data. Different formats are 
	                   useful for different types of scenes.
	*/
	void SetDepthCullControl(DepthCullDirection dir, DepthCullFormat format);

	//! Sets the Z-cull processor stencil cull hint.
	/*! \param func  The stencil comparison function.
		\param ref   The reference value; must be 0x00-0xFF.
		\param mask  The comparison mask; must be 0x00-0xFF.
	*/
	void SetStencilCullHint(ComparisonFunc func, U32 ref, U32 mask);

	//! Used to optimize Depth Cull processor performance.
	/*! \param feedbackValue  A value calculated from CalculateDepthCullFeedback(). The default value is kDefaultDepthCullFeedback.
	*/
	void SetDepthCullFeedback(U32 feedbackValue);


	//! Inserts a reference into the command stream. 
	/*! This is the mechanism used by OpenGL to create fences.
		\param value  The reference value. These need to be strictly increasing.
	*/
	void InsertReference(I32 value);

	//! Sets the WaitSemaphore index to use.
	/*! \param index  The index to use with WaitOnSemaphore; must be 0-255
	*/
	void SetWaitSemaphoreIndex(U32 index);

	//! Waits on a semaphore until it is the specified value.
	/*! \param value  The value the semaphore has to be when the push buffer will continue execution.
	*/
	void WaitOnSemaphore(U32 value);

	//! Sets the semaphore index to use in FlushPipeAndWriteSemaphore.
	/*! \param index  The index to use; must be 0-255
	*/
	void SetFlushPipeSemaphoreIndex(U32 index);

	//! Writes a semaphore when the texture pipes are done with execution up to this point.
	/*! \param value  The value to write to the semaphore once it is idle.
	*/
	void FlushTexturePipeAndWriteSemaphore(U32 value);

	//! Writes a semaphore when all backend writes are done with execution up to this point.
	/*! \param value  The value to write to the semaphore once it is idle.
	*/
	void FlushBackendPipeAndWriteSemaphore(U32 value);



	//! Inserts a command into the push buffer which will move execution to the specified address.
	/*! \param offset  The offset to jump to.
	*/
	void InsertJump(U32 offset);

	//! Inserts a call command into the push buffer which will move execution to the specified address.
	/*! The call stack is only a single level deep! 
		\param offset  The offset to call.
	*/
	void InsertCall(U32 offset);

	//! Inserts a command which returns push buffer execution to the command after the last call command.
	/*! The call stack is only a single level deep! 
	*/
	void InsertReturn();



	//! Inserts a No Operation command into the push buffer. 
	void InsertNop();

	//! Creates an uninitialized 16-byte aligned hole in the push buffer.
	/*! \param size  The size of the hole to create in bytes.
	*/
	U32 *InsertHole(U32 size);

	//! Inserts the specified data into the push buffer
	/*! \param data  A pointer to the data to insert.
		\param size  The size of the data to insert in bytes.
	*/
	void InsertData(const void *data, U32 size);

	//! Inserts a hole of numBytes size with no guaranteed alignment. 
	/*! This type hole is typically faster than doing a jump if 
	    numBytes is less than 1000.
	    \param numBytes  The number of bytes to skip over. Must be a multiple
	                     of 4, must be less than or equal to 8188 (0x1FFC).
	    \return          A pointer to the start of the hole that was
	                     skipped over.
	*/
	U32 *InsertSkipHole(U32 numBytes);


	//! Resets the gpu's report value for the specified type. 
	/*! \param type  The type of report you wish to reset.
	*/
	void ResetReport(ReportType type);

	//! Writes out the specified report type to the specified offset from the 
	//! base report address.
	/*! NOTE - Only the first 16 megs of io memory space is available for reports.
		\param type      The type of report you wish to write.
		\param ioOffset  An io offset into main memory of the report data to 
		                 write. must be 4 byte aligned.
	*/
	void WriteReport(ReportType type, U32 ioOffset);

	//! Sets the rendering control mode. 
	/*! If the mode is conditional, and the report value is zero no rendering will 
	    occur.
		NOTE - Only the first 16 megs of io memory space is available for reports.
		\param mode      The rendering mode to set. see RenderControlMode.
		\param ioOffset  An io offset into main memory of the report data to 
		                 test with. must be 4 byte aligned. This is only 
		                 applicable with kRenderConditional mode.
	*/
	void SetRenderControl(RenderControlMode mode, U32 ioOffset);



	//! Sets the type of clipping operation to perform on all windows.
	/*! \param mode  The type of operation to perform on all windows.
	*/
	void SetWindowClipMode(WindowClipMode mode);

	//! Sets the specified window to the specified screen rectangle.
	/*! \param index   The window to set; must be 0-7.
		\param left    The left edge in screen coordinates.
		\param top     The top edge in screen coordinates.
		\param right   The right edge in screen coordinates.
		\param bottom  The bottom edge in screen coordinates.
	*/
	void SetWindow(U32 index, U32 left, U32 top, U32 right, U32 bottom);


	//! Inserts a command into the push buffer which causes an interrupt on the 
	//! ppu upon execution by the gpu. Use SetPpuInterruptCallback() to set the 
	//! function to call. 
	/*! This interrupt does NOT stop the execution of the Gpu.
	    \param parameterValue  This value is passed to the function called durring 
	                           the interrupt. See SetPpuInterruptCallback().
	*/
	void InsertPpuInterrupt(U32 parameterValue);

	//! Inserts a command to wait for the gpu to be idle, and then another 
	//! command	to dump performance stats to a reserved video memory space.
	/*! To retrieve the stats written use GetPerformanceReportAddress().
	*/
	void WritePerformanceReport();

	//! Copy memory from the passed in source address into the push buffer 
	//! such that upon execution of the command by the gpu, the data will be 
	//! copied from the push buffer to the specified destination offset and 
	//! context.
	/*! \param dstOffset     The destination offset to copy the src data to.
	    \param dstContext    The destination context to copy the src data to.
	    \param src           A pointer to data that will be copied into the 
	                         command buffer and upon execution copied to the 
	                         destination offset and context.
	    \param sizeInDWords  The size of the data to copy in 8 byte double 
	                         words.
	*/
	void CopyMemoryImmediate(U32 dstOffset, CopyContext dstContext, void const *__restrict src, U32 sizeInDWords);

	//! Copy memory from the passed in source offset and context to the passed
	//! in destination offset and context.
	/*! \param dstOffset     The destination offset to copy the data to.
	    \param dstContext    The destination context to copy the data to.
	    \param dstPitch      The pitch of the destination surface to copy to.
	    \param srcOffset     The source offset to copy the data from.
	    \param srcContext    The source context to copy the data from.
	    \param srcPitch      The pitch of the source surface to copy from.
	    \param bytesPerRow   The number of bytes per row to copy.
	    \param rowCount      The number of rows to copy.
	*/
	void CopyMemory(U32 dstOffset, CopyContext dstContext, I32 dstPitch, U32 srcOffset, CopyContext srcContext, I32 srcPitch, I32 bytesPerRow, I32 rowCount);

	//! Copy an image from the passed in source offset and context to the passed
	//! in destination offset and context.
	/*! \param dstOffset      The destination offset to copy the data to.
	    \param dstContext     The destination context to copy the data to.
	    \param dstPitch       The pitch of the destination surface to copy to.
	    \param dstX           The destination horizontal address to copy to.
	    \param dstY           The destination vertical address to copy to.
	    \param srcOffset      The source offset to copy the data from.
	    \param srcContext     The source context to copy the data from.
	    \param srcPitch       The pitch of the source surface to copy from.
	    \param srcX           The source horizontal address to copy to.
	    \param srcY           The source vertical address to copy to.
	    \param width          The width of the surface to copy.
	    \param height         The height of the surface to copy.
	    \param bytesPerPixel  The number of bytes per pixel. 2 or 4
	*/
	void CopyImage(U32 dstOffset, CopyContext dstContext, U32 dstPitch, U32 dstX, U32 dstY, U32 srcOffset, U32 srcContext, U32 srcPitch, U32 srcX, U32 srcY, U32 width, U32 height, U32 bytesPerPixel);

	//! Copy an image from the passed in source offset and context to the passed
	//! in destination offset and context and scale it.
	/*! \param dstOffset      The destination offset to copy the data to.
	    \param dstContext     The destination context to copy the data to.
	    \param dstPitch       The pitch of the destination surface to copy to.
	    \param dstWidth       The destination surface width to copy to.
	    \param dstHeight      The destination surface height to copy to.
	    \param srcOffset      The source offset to copy the data from.
	    \param srcContext     The source context to copy the data from.
	    \param srcPitch       The pitch of the source surface to copy from.
	    \param bytesPerPixel  The number of bytes per pixel. 2 or 4
	    \param log2XScale     The logarithm of the amount to scale the x axis of
	                          the surface to copy.
	    \param log2YScale     The logarithm of the amount to scale the y axis of
	                          the surface to copy.
	    \param filter         The filtering mode to use when copying.
	*/
	void CopyScaledImage(U32 dstOffset, CopyContext dstContext, U32 dstPitch, U32 dstWidth, U32 dstHeight, U32 srcOffset, CopyContext srcContext, U32 srcPitch, U32 bytesPerPixel, I32 log2XScale, I32 log2YScale, CopyFilterMode filter);

	//! Copy an image from the passed in source offset in main memory to the passed
	//! in destination offset in video memory and swizzle it.
	/*! \param dstOffset      The destination offset to copy the data to.
	    \param dstWidth       The destination surface width to copy to.
	    \param dstHeight      The destination surface height to copy to.
	    \param dstX           The destination horizontal address to copy to.
	    \param dstY           The destination vertical address to copy to.
	    \param srcOffset      The source offset to copy the data from.
	    \param srcPitch       The pitch of the source surface to copy from.
	    \param srcX           The source horizontal address to copy to.
	    \param srcY           The source vertical address to copy to.
	    \param width          The width of the surface to copy.
	    \param height         The height of the surface to copy.
	    \param bytesPerPixel  The number of bytes per pixel. 2 or 4
	*/
	void CopyAndSwizzleImage(U32 dstOffset, U32 dstWidth, U32 dstHeight, U32 dstX, U32 dstY, U32 srcOffset, U32 srcPitch, U32 srcX, U32 srcY, U32 width, U32 height, U32 bytesPerPixel);

	//
	// DEBUG FUNCTIONS
	//

	//! Inserts a Nop with a special token and a string insertion, 
	//! this is then interpreted by the command dissassembler and output
	//! appropriately.
	/*! \param debugString  The debug string.
	*/
	void InsertDebugString(const char *debugString);
};

//! A helper function to write the depth cull report data needed to 
//! optimize depth cull processor performance.
/*! \param context            The context to write the commands into.
	\param feedbackAIoOffset  The io offset of where you want to write depth 
	                          cull feedback A.
	\param feedbackBIoOffset  The io offset of where you want to write depth 
	                          cull feedback B.
*/
inline void WriteDepthCullReports(CommandContext *context, U32 feedbackAIoOffset, U32 feedbackBIoOffset)
{
	context->DisableRenderState(kRenderDepthCullReports);
	context->WriteReport(kReportDepthCullFeedbackA, feedbackAIoOffset);
	context->WriteReport(kReportDepthCullFeedbackB, feedbackBIoOffset);
	context->ResetReport(kReportDepthCullFeedbackA);
	context->EnableRenderState(kRenderDepthCullReports);
}
