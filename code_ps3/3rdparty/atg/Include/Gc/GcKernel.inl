//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Graphics Core Kernel - Provides initialisation, shutdown and the basic building blocks
				for rendering.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_KERNEL_INL_H
#define GC_KERNEL_INL_H

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns true if the GPU is initialised.
**/
//--------------------------------------------------------------------------------------------------

inline bool GcKernel::IsInitialised()
{
	return ms_instance.m_isInitialised;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Register callback function that is executed upon the first GPU timeout.
**/
//--------------------------------------------------------------------------------------------------
	
inline void GcKernel::RegisterTimeoutCallback(TimeoutCallbackFunc* pCallback)
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	ms_instance.m_pTimeoutCallback = pCallback;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns pointer to the last registered timeout callback.
**/
//--------------------------------------------------------------------------------------------------
	
inline GcKernel::TimeoutCallbackFunc* GcKernel::GetRegisteredTimeoutCallback( void )
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	return ms_instance.m_pTimeoutCallback;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Register the GPU finished callback.

	The finish callback will be executed in GcKernel::Present()

		* After the GPU has finished processing the previous push buffer.
		* Before the next push buffer is kicked.
		
	In other words, once the GPU is known to be idle.
**/
//--------------------------------------------------------------------------------------------------
	
inline void GcKernel::RegisterFinishCallback( FinishCallbackFunc* pCallback )
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	ms_instance.m_pFinishCallback = pCallback;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns pointer to the last registered GPU finished callback.
**/
//--------------------------------------------------------------------------------------------------
	
inline GcKernel::FinishCallbackFunc* GcKernel::GetRegisteredFinishCallback( void )
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	return ms_instance.m_pFinishCallback;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Register the allocation failure callback.

	If non-NULL, this function is called when a given allocation fails.  This is recommended for 
	debug purposes only.	
**/
//--------------------------------------------------------------------------------------------------
	
inline void GcKernel::RegisterAllocFailureCallback( AllocFailureCallbackFunc* pCallback )
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	ms_instance.m_pAllocFailureCallback = pCallback;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns pointer to the last registered allocation failure callback.
**/
//--------------------------------------------------------------------------------------------------
	
inline GcKernel::AllocFailureCallbackFunc* GcKernel::GetRegisteredAllocFailureCallback( void )
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	return ms_instance.m_pAllocFailureCallback;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the display swap mode.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetDisplaySwapMode( Gc::DisplaySwapMode mode )
{
	// call the current context
	Ice::Render::SetDisplaySwapMode( ( Ice::Render::DisplaySwapMode ) mode );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Gets the current command context.
**/
//--------------------------------------------------------------------------------------------------

inline GcContext& GcKernel::GetContext()
{
	return ms_context;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the back buffer for the current frame.
**/
//--------------------------------------------------------------------------------------------------

inline const GcRenderBufferHandle& GcKernel::GetBackBuffer()
{
	return ms_instance.m_hBackBuffer;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the front buffer for the current frame.
**/
//--------------------------------------------------------------------------------------------------

inline const GcRenderBufferHandle& GcKernel::GetFrontBuffer()
{
	return ms_instance.m_hFrontBuffer;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Kicks the GPU off on the current push buffer and performs a display swap.

	@param		stallGpu			If set, the GPU will stall at the start of the frame

	@return		The IO address of the last valid GPU command.
	

	@note		:IMPORTANT: If you are using GPU references for your own purposes please use the version
				of GcKernel::Present() which takes a 'syncFrameReference' as an argument. Failure to do
				so will cause PPU <-> RSX end-of-frame syncing to fail. See the documentation of
				GcKernel::Present(uint syncFrameReference) for further details.
**/
//--------------------------------------------------------------------------------------------------

inline u32 GcKernel::Present( bool stallGpu )
{
	return Present(ms_instance.m_referenceValue + 1, stallGpu);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Wait until the previous frame's command context - kicked with GcKernel::Present() -
				has been processed by the GPU.
				
	@note		This function is BLOCKING.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SyncPreviousFrame()
{
	SyncPreviousFrameInternal("GcKernel::SyncPreviousFrame()");
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets all render target buffers.

	This function sets a new render target using the given buffers. Either depth or colour 0 (or 
	both) must be a valid buffer, and there can be no gaps in the span of valid colour buffers.

	In future there may be functions to specifically set depth-only or colour-only render targets.

	@param			hDepthBuffer	The depth buffer (can be null).
	@param			hColourBuffer0	Colour buffer 0 (can be null).
	@param			hColourBuffer1	Colour buffer 1 (can be null).
	@param			hColourBuffer2	Colour buffer 2 (can be null).
	@param			hColourBuffer3	Colour buffer 3 (can be null).
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetRenderTarget( const GcRenderBufferHandle& hDepthBuffer, 
									   const GcRenderBufferHandle& hColourBuffer0, 
									   const GcRenderBufferHandle& hColourBuffer1, 
									   const GcRenderBufferHandle& hColourBuffer2, 
									   const GcRenderBufferHandle& hColourBuffer3 )
{
	// validate
	ms_validation.SetRenderTarget(
		hDepthBuffer, 
		hColourBuffer0, 
		hColourBuffer1, 
		hColourBuffer2, 
		hColourBuffer3
	);

	// call the current context
	ms_context.SetRenderTarget(
		hDepthBuffer, 
		hColourBuffer0, 
		hColourBuffer1, 
		hColourBuffer2, 
		hColourBuffer3
	);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Clear active render target configuration.
**/
//--------------------------------------------------------------------------------------------------
	
inline void GcKernel::Clear( int clearBits )
{
	// validate
	ms_validation.Clear( clearBits );
	
	// call the current context
	ms_context.Clear( clearBits );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets an array of constants with the given row offset.

	All rows from the offset to the end of the array are assumed to be valid and are uploaded to
	the program.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetShaderConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset )
{
	// validate
	ms_validation.SetShaderConstant( hShader );

	// call the current context
	ms_context.SetShaderConstant( hShader, index, pValues, rowOffset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets an array of constants with the given row offset.

	Only the rows from rowOffset to ( rowOffset + rowCount ) are uploaded to the program.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetShaderConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset, uint rowCount )
{
	// validate
	ms_validation.SetShaderConstant( hShader );

	// call the current context
	ms_context.SetShaderConstant( hShader, index, pValues, rowOffset, rowCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a single floating-point scalar into a shader constant row.

	The other components of the constant are set to zero.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetShaderConstant( const GcShaderHandle& hShader, uint index, float value, uint rowOffset )
{
	// validate
	ms_validation.SetShaderConstant( hShader );

	// call the current context
	ms_context.SetShaderConstant( hShader, index, value, rowOffset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a vector value into a shader constant row.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetShaderConstant( const GcShaderHandle& hShader, uint index, const FwVector4& vec, uint rowOffset )
{
	// validate
	ms_validation.SetShaderConstant( hShader );

	// call the current context
	ms_context.SetShaderConstant( hShader, index, vec, rowOffset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets some or all of a matrix value into an array of shader constant rows.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetShaderConstant( const GcShaderHandle& hShader, uint index, const FwMatrix44& mat, uint rowOffset, uint rowCount )
{
	// validate
	ms_validation.SetShaderConstant( hShader );

	// call the current context
	ms_context.SetShaderConstant( hShader, index, mat, rowOffset, rowCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets an array of constants with the given row offset.

	All rows from the offset to the end of the array are assumed to be valid and are uploaded to
	the program.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexProgramConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset )
{
	// validate
	ms_validation.SetShaderConstant( hShader );

	// call the current context
	ms_context.SetVertexProgramConstant( hShader, index, pValues, rowOffset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets an array of constants with the given row offset.

	Only the rows from rowOffset to ( rowOffset + rowCount ) are uploaded to the program.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexProgramConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset, uint rowCount )
{
	// validate
	ms_validation.SetShaderConstant( hShader );

	// call the current context
	ms_context.SetVertexProgramConstant( hShader, index, pValues, rowOffset, rowCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a single floating-point scalar into a shader constant row.

	The other components of the constant are set to zero.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexProgramConstant( const GcShaderHandle& hShader, uint index, float value, uint rowOffset )
{
	// validate
	ms_validation.SetShaderConstant( hShader );

	// call the current context
	ms_context.SetVertexProgramConstant( hShader, index, value, rowOffset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a vector value into a shader constant row.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexProgramConstant( const GcShaderHandle& hShader, uint index, const FwVector4& vec, uint rowOffset )
{
	// validate
	ms_validation.SetShaderConstant( hShader );

	// call the current context
	ms_context.SetVertexProgramConstant( hShader, index, vec, rowOffset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets some or all of a matrix value into an array of shader constant rows.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexProgramConstant( const GcShaderHandle& hShader, uint index, const FwMatrix44& mat, uint rowOffset, uint rowCount )
{
	// validate
	ms_validation.SetShaderConstant( hShader );

	// call the current context
	ms_context.SetVertexProgramConstant( hShader, index, mat, rowOffset, rowCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets an array of constants with the given row offset.

	All rows from the offset to the end of the array are assumed to be valid and are uploaded to
	the program.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetFragmentProgramConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset, bool noProgramChange )
{
	// validate
	ms_validation.SetShaderConstant( hShader, noProgramChange );

	// call the current context
	ms_context.SetFragmentProgramConstant( hShader, index, pValues, rowOffset, noProgramChange );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets an array of constants with the given row offset.

	Only the rows from rowOffset to ( rowOffset + rowCount ) are uploaded to the program.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetFragmentProgramConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset, uint rowCount, bool noProgramChange )
{
	// validate
	ms_validation.SetShaderConstant( hShader, noProgramChange );

	// call the current context
	ms_context.SetFragmentProgramConstant( hShader, index, pValues, rowOffset, rowCount, noProgramChange );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a single floating-point scalar into a shader constant row.

	The other components of the constant are set to zero.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetFragmentProgramConstant( const GcShaderHandle& hShader, uint index, float value, uint rowOffset, bool noProgramChange )
{
	// validate
	ms_validation.SetShaderConstant( hShader, noProgramChange );

	// call the current context
	ms_context.SetFragmentProgramConstant( hShader, index, value, rowOffset, noProgramChange );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a vector value into a shader constant row.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetFragmentProgramConstant( const GcShaderHandle& hShader, uint index, const FwVector4& vec, uint rowOffset, bool noProgramChange )
{
	// validate
	ms_validation.SetShaderConstant( hShader, noProgramChange );

	// call the current context
	ms_context.SetFragmentProgramConstant( hShader, index, vec, rowOffset, noProgramChange );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets some or all of a matrix value into an array of shader constant rows.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetFragmentProgramConstant( const GcShaderHandle& hShader, uint index, const FwMatrix44& mat, uint rowOffset, uint rowCount, bool noProgramChange )
{
	// validate
	ms_validation.SetShaderConstant( hShader, noProgramChange );

	// call the current context
	ms_context.SetFragmentProgramConstant( hShader, index, mat, rowOffset, rowCount, noProgramChange );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets vertex program constants directly to hardware registers.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexProgramConstant( uint resourceIndex, const float* pValues, uint rowCount )
{
	// call the current context
	ms_context.SetVertexProgramConstant( resourceIndex, pValues, rowCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets vertex program constants directly to hardware registers.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexProgramConstant( uint resourceIndex, float value )
{
	// call the current context
	ms_context.SetVertexProgramConstant( resourceIndex, value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets vertex program constants directly to hardware registers.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexProgramConstant( uint resourceIndex, const FwVector4& vec )
{
	// call the current context
	ms_context.SetVertexProgramConstant( resourceIndex, vec );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets vertex program constants directly to hardware registers.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexProgramConstant( uint resourceIndex, const FwMatrix44& mat, uint rowCount )
{
	// call the current context
	ms_context.SetVertexProgramConstant( resourceIndex, mat, rowCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets attributes from the stream to the vertex shader.

	@param		hVertexShader	The vertex shader.
	@param		hStream			The stream.
	@param		offsetInBytes	The offset in bytes to start the vertices at. Usually this offset 
								should be a whole number of vertices, but this is not enforced.

	@return		Returns the number of stream elements that were bound during this call. Validating
				this value against the expected number in debug builds is encouraged to catch bugs.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcKernel::SetStream( const GcShaderHandle& hVertexShader, 
								 const GcStreamBufferHandle& hStream, 
								 uint offsetInBytes, 
								 uint divider )
{
	// validate
	ms_validation.SetStream( hVertexShader, hStream, divider );

	// call the current context
	return ms_context.SetStream( hVertexShader, hStream, offsetInBytes, divider );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a vertex attribute.

	@param		resourceIndex	The vertex attribute index to bind to.
	@param		hStream			The stream.
	@param		fieldIndex		The field within the stream that contains the element.
	@param		offsetInBytes	The offset in bytes to the start of the vertices. This offset is
								to the start of the <b>vertex</b>. The offset from the start of the 
								vertex to the field is added automatically. Usually \c offsetInBytes 
								should be a whole number of vertices, but this is not enforced.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexAttribute( uint resourceIndex, 
										  const GcStreamBufferHandle& hStream, 
										  uint fieldIndex, 
										  uint offsetInBytes,
										  uint divider )
{
	// validate
	ms_validation.SetVertexAttribute( resourceIndex, hStream->GetCount(), divider );

	// call the current context
	ms_context.SetVertexAttribute( resourceIndex, hStream, fieldIndex, offsetInBytes, divider );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the vertex frequency mode for all attributes.

	Each bit applies to vertex attribute index 0 to 15 from the least significant bit upwards. 
	A 0 bit means divide, and a 1 bit means modulo.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexAttributeFrequencyMode( uint mode )
{
	// validate
	ms_validation.SetVertexAttributeFrequencyMode( mode );

	// call the current context
	ms_context.SetVertexAttribFrequencyMode( mode );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Disables the given vertex attribute index.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::DisableVertexAttribute( uint resourceIndex )
{
	// validate
	ms_validation.DisableVertexAttribute( resourceIndex );

	// call the current context
	ms_context.DisableVertexAttribArray( resourceIndex );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Clears data from all vertex attribute indices used by the shader.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::ClearStreams( const GcShaderHandle& hVertexShader )
{
	// validate
	ms_validation.ClearStreams( hVertexShader );

	// call the current context
	ms_context.ClearStreams( hVertexShader );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets a constant value on the given vertex attribute.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexAttribute1f( uint resourceIndex, float x )
{
	// validate
	ms_validation.SetVertexAttribute( resourceIndex );

	// call the current context
	ms_context.SetVertexAttrib1f( resourceIndex, x );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets a constant value on the given vertex attribute.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexAttribute2f( uint resourceIndex, float x, float y )
{
	// validate
	ms_validation.SetVertexAttribute( resourceIndex );

	// call the current context
	ms_context.SetVertexAttrib2f( resourceIndex, x, y );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets a constant value on the given vertex attribute.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexAttribute3f( uint resourceIndex, float x, float y, float z )
{
	// validate
	ms_validation.SetVertexAttribute( resourceIndex );

	// call the current context
	ms_context.SetVertexAttrib3f( resourceIndex, x, y, z );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets a constant value on the given vertex attribute.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexAttribute4f( uint resourceIndex, float x, float y, float z, float w )
{
	// validate
	ms_validation.SetVertexAttribute( resourceIndex );

	// call the current context
	ms_context.SetVertexAttrib4f( resourceIndex, x, y, z, w );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets a constant value on the given vertex attribute.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexAttribute2s( uint resourceIndex, short x, short y )
{
	// validate
	ms_validation.SetVertexAttribute( resourceIndex );

	// call the current context
	ms_context.SetVertexAttrib2s( resourceIndex, x, y );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets a constant value on the given vertex attribute.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexAttribute4s( uint resourceIndex, short x, short y, short z, short w )
{
	// validate
	ms_validation.SetVertexAttribute( resourceIndex );

	// call the current context
	ms_context.SetVertexAttrib4s( resourceIndex, x, y, z, w );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets a constant value on the given vertex attribute.

	Normalised means the components will appear to the vertex shader in the [-1, 1] range.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexAttribute4Ns( uint resourceIndex, short x, short y, short z, short w )
{
	// validate
	ms_validation.SetVertexAttribute( resourceIndex );

	// call the current context
	ms_context.SetVertexAttrib4Ns( resourceIndex, x, y, z, w );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets a constant value on the given vertex attribute.

	Normalised means the components will appear to the vertex shader in the [-1, 1] range.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexAttribute4Nub( uint resourceIndex, u8 x, u8 y, u8 z, u8 w )
{
	// validate
	ms_validation.SetVertexAttribute( resourceIndex );

	// call the current context
	ms_context.SetVertexAttrib4Nub( resourceIndex, x, y, z, w );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Set a texture.

	This function sets textures for fragment program reads.

	@param			resourceIndex	The hardware index to set the texture at.
	@param			hTexture		The texture handle.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetTexture( uint resourceIndex, const GcTextureHandle& hTexture )
{
	// validate
	ms_validation.SetTexture( resourceIndex, hTexture );

	// call the current context
	ms_context.SetTexture( resourceIndex, hTexture );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Disable a texture.

	This function disables a texture unit for fragment program reads.

	@param			resourceIndex	The hardware index of the texture unit to disable.
	
	@note	A disabled texture unit returns float4(0.0, 0.0, 0.0, 0.0)
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::DisableTexture( uint resourceIndex )
{
	// validate
	ms_validation.DisableTexture( resourceIndex );
	
	// call the current context
	ms_context.DisableTexture( resourceIndex );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set a vertex program texture.

	This function sets textures for vertex program reads.

	@param		resourceIndex	The hardware index to set the texture at.
	@param		hTexture		The texture handle.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexProgramTexture( uint resourceIndex, const GcTextureHandle& hTexture )
{
	// validate
	ms_validation.SetVertexProgramTexture( resourceIndex, hTexture );

	// call the current context
	ms_context.SetVertexProgramTexture( resourceIndex, hTexture );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Disable a vertex program texture.

	This function disables a texture unit for vertex program reads.

	@param			resourceIndex	The hardware index of the vertex texture unit to disable.

	@note	A disabled texture unit returns float4(0.0, 0.0, 0.0, 0.0)
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::DisableVertexProgramTexture( uint resourceIndex )
{
	// validate
	ms_validation.DisableVertexProgramTexture( resourceIndex );
	
	// call the current context
	ms_context.DisableVertexProgramTexture( resourceIndex );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Loads and selects the vertex shader.

	@param		hShader		A valid vertex shader.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetVertexShader( const GcShaderHandle& hShader )
{
	// validate
	ms_validation.SetVertexShader( hShader );

	// call the current context
	ms_context.SetVertexShader( hShader );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Loads the vertex shader.

	@param		hShader		A valid vertex shader.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::LoadVertexShader( const GcShaderHandle& hShader )
{
	// call the current context
	ms_context.LoadVertexShader( hShader );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Selects the loaded vertex shader.

	@param		hShader		A valid vertex shader.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SelectVertexShader( const GcShaderHandle& hShader )
{
	// validate
	ms_validation.SetVertexShader( hShader );

	// call the current context
	ms_context.SelectVertexShader( hShader );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the fragment shader.

	@param		hShader		A valid fragment shader.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetFragmentShader( const GcShaderHandle& hShader )
{
	// validate
	ms_validation.SetFragmentShader( hShader );

	// call the current context
	ms_context.SetFragmentShader( hShader );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Refreshes the fragment shader.

	This function can only be called if the fragment shader is still bound, but the constants
	are not valid.

	@param		hShader		A valid fragment shader.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::RefreshFragmentShader( const GcShaderHandle& hShader )
{
	// validate
	ms_validation.RefreshFragmentShader( hShader );

	// call the current context
	ms_context.RefreshFragmentShader( hShader );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Render non-indexed primitives.

	@param			primType	The primitive type
	@param			start		The vertex to start the primitive with
	@param			count		The number of vertices in the primitive
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::DrawArrays( Gc::PrimitiveType primType, uint start, uint count )
{
	// validate 
	ms_validation.DrawArrays( start, count );

	// call the current context
	ms_context.DrawArrays( primType, start, count );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Render indexed primitives.

	@param			primType	The primitive type.
	@param			start		The primitive type.
	@param			count		The primitive type.
	@param			hIndices	The index buffer to render with.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::DrawElements( Gc::PrimitiveType primType, uint start, uint count, const GcStreamBufferHandle& hIndices )
{
	// validate 
	ms_validation.DrawElements();

	// call the current context
	ms_context.DrawElements( primType, start, count, hIndices );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Enable the given render state.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::Enable( Gc::RenderState state )
{
	// validate 
	ms_validation.Enable( state );

	// call the current context
	ms_context.EnableRenderState( ( Ice::Render::RenderState )state );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Disable the given render sate.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::Disable( Gc::RenderState state )
{
	// validate 
	ms_validation.Disable( state );

	// call the current context
	ms_context.DisableRenderState( ( Ice::Render::RenderState )state );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the alpha test function and reference value.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetAlphaFunc( Gc::CmpFunc func, float ref )
{
	// this is evil but we'll convert here
	int value = max( 0, max( 0, int( ref*255.0f ) ) );

	// call the current context
	ms_context.SetAlphaFunc( ( Ice::Render::ComparisonFunc )func, value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the blend equation for both rgb and alpha simultaneously.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetBlendEquation( Gc::BlendEquation mode )
{
	// call the current context
	ms_context.SetBlendEquation( ( Ice::Render::BlendEquation )mode );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the blend equation for rgb and alpha separately.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetBlendEquationSeparate( Gc::BlendEquation modeRgb, Gc::BlendEquation modeAlpha )
{
	// call the current context
	ms_context.SetBlendEquationSeparate( ( Ice::Render::BlendEquation )modeRgb, 
										 ( Ice::Render::BlendEquation )modeAlpha );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the blend function for rgb and alpha simultaneously.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetBlendFunc( Gc::BlendFunc src, Gc::BlendFunc dest )
{
	// call the current context
	ms_context.SetBlendFunc( ( Ice::Render::BlendFactor )src, ( Ice::Render::BlendFactor )dest );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the blend function for rgb and alpha separately.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetBlendFuncSeparate( Gc::BlendFunc srcRgb, Gc::BlendFunc destRgb, Gc::BlendFunc srcAlpha, Gc::BlendFunc destAlpha )
{
	// call the current context
	ms_context.SetBlendFuncSeparate( ( Ice::Render::BlendFactor )srcRgb, 
									 ( Ice::Render::BlendFactor )destRgb, 
									 ( Ice::Render::BlendFactor )srcAlpha, 
									 ( Ice::Render::BlendFactor )destAlpha );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the blend colour for 8-bit render targets.
	
	@note		Will trash the fp16 blend colour as the same hardware registers are used.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetBlendColour( float red, float green, float blue, float alpha )
{
	// this is evil, but we'll convert to argb on the fly here
	int argb =    ( min( 255, max( 0, int( alpha*255.0f ) ) ) << 24 )
				| ( min( 255, max( 0, int(   red*255.0f ) ) ) << 16 )
				| ( min( 255, max( 0, int( green*255.0f ) ) ) << 8 )
				| ( min( 255, max( 0, int(  blue*255.0f ) ) ) << 0 );

	// call the current context
	ms_context.SetBlendColor( argb );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the blend colour for fp16 render targets.
	
	@note		Will trash the argb 8-bit blend colour as the same hardware registers are used.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetFloatBlendColour( float red, float green, float blue, float alpha )
{
	// this is evil, but we'll convert to halfs on the fly here
	short hred		= FloatToHalf( Gc::FloatToInt( red ) );
	short hgreen	= FloatToHalf( Gc::FloatToInt( green ) );
	short hblue		= FloatToHalf( Gc::FloatToInt( blue ) );
	short halpha	= FloatToHalf( Gc::FloatToInt( alpha ) );

	// call the current context
	ms_context.SetFloatBlendColor( hred, hgreen, hblue, halpha );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the clear colour.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetClearColour( float red, float green, float blue, float alpha )
{
	// this is evil, but we'll convert to argb on the fly here
	int argb =    ( min( 255, max( 0, int( alpha*255.0f ) ) ) << 24 )
				| ( min( 255, max( 0, int(   red*255.0f ) ) ) << 16 )
				| ( min( 255, max( 0, int( green*255.0f ) ) ) << 8 )
				| ( min( 255, max( 0, int(  blue*255.0f ) ) ) << 0 );

	// call the current context
	ms_context.SetClearColor( argb );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the clear depth-stencil value.

	This function only works correctly for D24S8 depth buffers. Users should migrate to the
	version of this version that takes a single packed value and format this value for the 
	current depth buffer format.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetClearDepthStencil( float depth, int stencil )
{
	// this is evil, but'll convert to the single value here
	int fixed = min( 0xffffff, max( 0, int( depth*16777216.0f ) ) );
	int value = ( fixed << 8 ) | ( stencil & 0xff );

	// call the preferred version
	SetClearDepthStencil( value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the clear depth-stencil value.

	This value should match the current depth buffer format, e.g. D24S8. For example, the clear 
	value for ( 1.0f, 0 ) is \c 0xffffff00 in D24S8 and 0x0000ffff in D16S0.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetClearDepthStencil( int value )
{
	// call the current context
	ms_context.SetClearDepthStencil( value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the colour write mask.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetColourMask( bool red, bool green, bool blue, bool alpha )
{
	// validate
	ms_validation.SetColourMask( red, green, blue, alpha );

	// call the current context
	ms_context.SetColorMask( red, green, blue, alpha );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the face winding order to cull.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetCullFace( Gc::CullFaceMode mode )
{
	// call the current context
	ms_context.SetCullFace( ( Ice::Render::CullFace )mode );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the depth bounds for the depth bounds test.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetDepthBounds( float zmin, float zmax )
{
	// call the current context
	ms_context.SetDepthBounds( zmin, zmax );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the depth bounds for the depth bounds test.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetDepthMinMaxControl( bool cullNearFarEnable, bool clampEnable, bool cullIgnoreW )
{
	// call the current context
	ms_context.SetDepthMinMaxControl( cullNearFarEnable, clampEnable, cullIgnoreW );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the depth test function.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetDepthTest( Gc::CmpFunc func )
{
	// call the current context
	ms_context.SetDepthFunc( ( Ice::Render::ComparisonFunc )func );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the depth write mask.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetDepthMask( bool depth )
{
	// validate
	ms_validation.SetDepthMask( depth );

	// call the current context
	ms_context.SetDepthMask( depth );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the current fog mode.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetFogMode( Gc::FogMode mode )
{
	// call the current context
	ms_context.SetFogMode( ( Ice::Render::FogMode )mode );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets fog range for the linear fog mode.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetFogRange( float fmin, float fmax )
{
	// call the current context
	ms_context.SetFogRange( fmin, fmax );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the fog density for the exponential fog modes.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetFogDensity( float density )
{
	// call the current context
	ms_context.SetFogDensity( density );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the winding order considered to be front-facing.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetFrontFace( Gc::FaceType face )
{
	// call the current context
	ms_context.SetFrontFace( ( Ice::Render::FrontFace )face );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the line width.

	@param		width		Width must be in (0, 10].
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetLineWidth( float width )
{
	// this is evil but we'll convert here
	FW_ASSERT( 0.0f < width && width <= 10.0f );
	int value = int( width * 8.0f );

	// call the current context
	ms_context.SetLineWidth( value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the colour logic operation.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetLogicOp( Gc::LogicOp logicOp )
{
	// call the current context
	ms_context.SetLogicOp( ( Ice::Render::LogicOp )logicOp );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the blend enabled state for MRT rendering.
	
	@param		mask	Specifies which MRT buffers are blended.
	
		    Little Endian: 31                             0
		       Big Endian: 0                             31
		                  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		                  |- - - U N U S E D - - - 3 2 1 0|
		                  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetMrtBlendEnable( u32 mask )
{
	// call the current context
	ms_context.SetMrtBlendEnable( mask );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the colour write mask for multiple render targets (MRTs).
	
	@param		mask	Write enable bit for each channel of each render target. The format of 'mask' is
	
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
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetMrtColourMask( u16 mask )
{
	// call the current context
	ms_context.SetMrtColorMask( mask );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Enables/disables multisampling and sets parameters.

	@param		enabled				Enables/disables multi-sampling. This should only be enabled for 
									multi-sampled render targets.
	@param		alphaToCoverage		Lets the alpha channel control coverage. The pixel alpha value is
									dithered and distributed over the available samples.
	@param		alphaToOne			Sets the fragment alpha to 1.
	@param		coverageMask		The coverage mask.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetMultisampleParameters( bool enabled, bool alphaToCoverage, bool alphaToOne, u16 coverageMask )
{
	// validate
	ms_validation.SetMultisampleParameters( enabled );

	// call the current context
	ms_context.SetMultisampleParameters( enabled, alphaToCoverage, alphaToOne, coverageMask );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the point size when not using vertex program point size.

	To use vertex program point size make sure that you enable the kVertexProgramPointSize 
	render state.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetPointSize( float psize )
{
	// call the current context
	ms_context.SetPointSize( psize );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the point sprite parameters.

	@param	enabled			Enable point sprites.
	@param	texCoordMask	An 8-bit value specifying which texture coordinates get the sprite
							texture coordinate. TEXCOORD0 is the lsb.
	@param	mode			The sprite R coordinate mode. This functions identically to the
							NV_point_sprite extension documented at 
							http://oss.sgi.com/projects/ogl-sample/registry/NV/point_sprite.txt
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetPointSpriteParameters( bool enabled, uint texCoordMask, Gc::SpriteMode mode )
{
	// call the current context
	ms_context.SetPointSpriteParameters( enabled, texCoordMask, ( Ice::Render::SpriteMode )mode );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the polygon mode for both front- and back-facing polygons.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetPolygonMode( Gc::PolygonMode mode )
{
	// call the current context
	ms_context.SetPolygonMode( ( Ice::Render::PolygonMode )mode );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the polygon mode for front- and back-facing polygons separately.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetPolygonModeSeparate( Gc::PolygonMode modeFront, Gc::PolygonMode modeBack )
{
	// call the current context
	ms_context.SetPolygonModeSeparate( ( Ice::Render::PolygonMode )modeFront, 
									   ( Ice::Render::PolygonMode )modeBack );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the filled polygon offset.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetPolygonOffset( float factor, float units )
{
	// call the current context
	ms_context.SetPolygonOffset( factor, units );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the primitive restart index.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetPrimitiveRestartIndex( uint index )
{
	// call the current context
	ms_context.SetPrimitiveRestartIndex( index );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the scissor region.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetScissor( int x, int y, int w, int h )
{
	// call the current context
	ms_context.SetScissor( x, y, w, h );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the shading model.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetShadeModel( Gc::ShadeModel shadeModel )
{
	// call the current context
	ms_context.SetShadeModel( ( Ice::Render::ShadeModel )shadeModel );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the stencil test function, reference and mask.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetStencilTest( Gc::CmpFunc func, int ref, int mask )
{
	// call the current context
	ms_context.SetStencilFunc( ( Ice::Render::ComparisonFunc )func, ref, mask );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the stencil test function, reference and mask.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetStencilTestSeparate( Gc::CmpFunc funcFront, int refFront, int maskFront, Gc::CmpFunc funcBack, int refBack, int maskBack )
{
	// call the current context
	ms_context.SetStencilFuncSeparate( ( Ice::Render::ComparisonFunc )funcFront, refFront, maskFront, 
									   ( Ice::Render::ComparisonFunc )funcBack, refBack, maskBack );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the stencil test operation.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetStencilOp( Gc::StencilOp stencilFail, Gc::StencilOp depthFail, Gc::StencilOp depthPass )
{
	// call the current context
	ms_context.SetStencilOp( ( Ice::Render::StencilOp )stencilFail, 
							 ( Ice::Render::StencilOp )depthFail, 
							 ( Ice::Render::StencilOp )depthPass );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the stencil test operation.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetStencilOpSeparate( Gc::StencilOp stencilFailFront, Gc::StencilOp depthFailFront, Gc::StencilOp depthPassFront, 
										    Gc::StencilOp stencilFailBack, Gc::StencilOp depthFailBack, Gc::StencilOp depthPassBack )
{
	// call the current context
	ms_context.SetStencilOpSeparate( ( Ice::Render::StencilOp )stencilFailFront, 
									 ( Ice::Render::StencilOp )depthFailFront, 
									 ( Ice::Render::StencilOp )depthPassFront, 
									 ( Ice::Render::StencilOp )stencilFailBack, 
									 ( Ice::Render::StencilOp )depthFailBack, 
									 ( Ice::Render::StencilOp )depthPassBack );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the stencil write mask.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetStencilMask( int mask )
{
	// call the current context
	ms_context.SetStencilMask( mask );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the stencil write mask.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetStencilMaskSeparate( int maskFront, int maskBack )
{
	// call the current context
	ms_context.SetStencilMaskSeparate( maskFront, maskBack );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set the viewport (and scissor).
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::SetViewport( int x, int y, int w, int h, float zmin, float zmax )
{
	// call the current context
	ms_context.SetViewport( x, y, w, h, zmin, zmax );
	ms_context.SetScissor( x, y, w, h );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Jumps to the given context.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::InsertJump( void* pAddr )
{
	ms_context.InsertJump( Ice::Render::TranslateAddressToIoOffset( pAddr )) ;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Calls the given context.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::InsertCall( void* pAddr )
{
	ms_context.InsertCall( Ice::Render::TranslateAddressToIoOffset( pAddr ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns to the caller.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::InsertReturn()
{
	ms_context.InsertReturn();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the number of times present has been called since initialisation.
**/
//--------------------------------------------------------------------------------------------------

inline u64 GcKernel::GetPresentCounter()
{
	return ms_instance.m_presentCounter;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Initialise the temporary SPU DMA mode.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::EnableValidation( bool enabled )
{
	ms_validation.EnableValidation( enabled );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the base address and size of the report buffer (if any)
	
	The report buffer is automatically allocated by GcKernel at initialisation. Its size is set using
	GcInitParams::SetReportBufferSize() - zero implies no report buffer.
	
	@note: GcKernel will automatically deallocate the report buffer upon GcKernel::Shutdown().
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::GetReportBufferMemoryInfo(void** ppBase, u32* pSize)
{
#ifndef ATG_PC_PLATFORM
	FW_ASSERT_MSG(ms_instance.m_pReportBuffer, ("No Report Buffer allocated. Use GcInitParams::SetReportBufferSize() at initialisation!"));
#endif
	FW_ASSERT(ppBase && pSize);

	*ppBase = ms_instance.m_pReportBuffer;
	*pSize = ms_instance.m_reportBufferSize;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Called internally for every host memory allocation.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::HostMemoryAddRef()
{
#ifdef ATG_ASSERTS_ENABLED
	ms_instance.m_hostMemoryRefCount++;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Called internally for every host memory deallocation.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::HostMemoryRelease()
{
#ifdef ATG_ASSERTS_ENABLED
	FW_ASSERT( ms_instance.m_hostMemoryRefCount > 0 );
	ms_instance.m_hostMemoryRefCount--;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Called internally for every vram allocation.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::VramAddRef()
{
#ifdef ATG_ASSERTS_ENABLED
	ms_instance.m_vramRefCount++;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Called internally for every vram deallocation.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::VramRelease()
{
#ifdef ATG_ASSERTS_ENABLED
	FW_ASSERT( ms_instance.m_vramRefCount > 0 );
	ms_instance.m_vramRefCount--;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the amount of unallocated host memory.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcKernel::GetFreeHostMemory()
{
	return ms_instance.m_freeHostMemory;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the amount of unallocated vram.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcKernel::GetFreeVram()
{
	return ms_instance.m_freeVram;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline bool GcKernel::QueryGetNewScratchMemory( uint sizeInBytes, uint alignment )
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));

	// align the start address
	void* pStart = FwAlign( ms_instance.m_pScratchCurrent, alignment );

	// add the amount we need
	void* pEnd = ( void* )( ( size_t )pStart + sizeInBytes );
	
	// check we can allocate this 
	return ( pEnd <= ms_instance.m_pScratchEnd );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline void* GcKernel::GetNewScratchMemory( uint sizeInBytes, uint alignment )
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));

	// align the start address
	void* pStart = FwAlign( ms_instance.m_pScratchCurrent, alignment );

	// add the amount we need
	void* pEnd = ( void* )( ( size_t )pStart + sizeInBytes );
	
	// check we can allocate this 
	FW_ASSERT_MSG( pEnd <= ms_instance.m_pScratchEnd, ( "Run out of scratch memory for this frame" ) );

    // allocate it
	ms_instance.m_pScratchCurrent = pEnd;
	return pStart;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline bool GcKernel::IsValidScratchAddress( void const* pAddress )
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
    
	// check against the addresses
	return ( ms_instance.m_pScratchBegin <= pAddress && pAddress < ms_instance.m_pScratchEnd );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns the base address and size of the managed Host Memory region visible by RSX.
	
	@note	On CEB-20x0 devkits with NV47 GPUs (NULL, 0) is returned due to lack of Host Memory access.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::GetManagedHostMemoryInfo(void** ppBase, u32* pSize)
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	FW_ASSERT(ppBase && pSize);

	*ppBase = ms_instance.m_pHostMem;
	*pSize = ms_instance.m_hostMemByteSize;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns the base address, local offset and size of the managed Video Memory.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::GetManagedVideoMemoryInfo(void** ppBase, u32* pOffset, u32* pSize)
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	FW_ASSERT(ppBase && pOffset && pSize);
	
	*ppBase = Ice::Render::g_gpuHardwareConfig.m_videoMemoryBaseAddr;
	*pOffset = 0;
	*pSize = Ice::Render::g_gpuHardwareConfig.m_usableVideoMemorySize;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns start and end pointers of the last push buffer kicked by GcKernel::Present()
			NULL implies no push buffers have kicked via GcKernel.
**/
//--------------------------------------------------------------------------------------------------

inline void GcKernel::GetLastContextKickedPointers(void** ppStart, void** ppEnd)
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	FW_ASSERT(ppStart && ppEnd);

	*ppStart = ms_instance.m_pLastKickedStart;
	*ppEnd = ms_instance.m_pLastKickedEnd;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns the GPU reference value that was used to sync the previous frame.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcKernel::GetLastSyncFrameReference()
{
	return ms_instance.m_referenceValue;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns the display width
**/
//--------------------------------------------------------------------------------------------------

inline uint GcKernel::GetDisplayWidth()
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	return ms_instance.m_displayWidth;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns the display height
**/
//--------------------------------------------------------------------------------------------------

inline uint GcKernel::GetDisplayHeight()
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	return ms_instance.m_displayHeight;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns the display pitch
**/
//--------------------------------------------------------------------------------------------------

inline uint GcKernel::GetDisplayPitch()
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	return ms_instance.m_displayPitch;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns the display aspect ratio type
**/
//--------------------------------------------------------------------------------------------------

inline Gc::AspectRatio GcKernel::GetDisplayAspectRatio()
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	return ms_instance.m_displayAspectRatio;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns the display aspect ratio value
**/
//--------------------------------------------------------------------------------------------------

inline float GcKernel::GetDisplayAspectRatioValue()
{
	FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	return ( ms_instance.m_displayAspectRatio == Gc::kAspectStandard ? ( 4.0f / 3.0f ) : ( 16.0f / 9.0f ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns the global push buffer validation state
	
	Useful for manually updating the state when pre-built push buffers are memcopy'd into the global push buffer.
	
	@note	Be very careful when you update the GcValidation state manually. It must be updated in an identical
			fashion to your pre-built push buffer segments, otherwise validation asserts will fire errorneously
			or not at all when they should!
**/
//--------------------------------------------------------------------------------------------------

inline GcValidation& GcKernel::GetValidationState()
{
    FW_ASSERT_MSG(IsInitialised(), (ms_pIsUninitialisedMessage));
	return ms_validation;
}

//--------------------------------------------------------------------------------------------------

#endif // GC_KERNEL_INL_H
