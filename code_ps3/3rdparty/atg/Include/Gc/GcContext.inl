//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_CONTEXT_INL_H
#define GC_CONTEXT_INL_H

#include <Gc/GcMetrics.h>

//--------------------------------------------------------------------------------------------------
/**
	@brief		Helper function for address access.
**/
//--------------------------------------------------------------------------------------------------

inline void* GcContext::GetBeginAddress() const
{
	return m_beginptr;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Helper function for address access.
**/
//--------------------------------------------------------------------------------------------------

inline void* GcContext::GetCurrentAddress() const
{
	return m_cmdptr;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets an array of constants with the given row offset.

	All rows from the offset to the end of the array are assumed to be valid and are uploaded to
	the program.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetShaderConstant( const GcShaderHandle& hShader, 
										  uint index, 
										  const float* pValues, 
										  uint rowOffset )
{
	FW_ASSERT( hShader.IsValid() );
	if( hShader->GetType() == Gc::kVertexProgram )
		SetVertexProgramConstant( hShader, index, pValues, rowOffset );
	else
		SetFragmentProgramConstant( hShader, index, pValues, rowOffset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets an array of constants with the given row offset.

	Only the rows from rowOffset to ( rowOffset + rowCount ) are uploaded to the program.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetShaderConstant( const GcShaderHandle& hShader, 
										  uint index, 
										  const float* pValues, 
										  uint rowOffset,
										  uint rowCount )
{
	FW_ASSERT( hShader.IsValid() );
	if( hShader->GetType() == Gc::kVertexProgram )
		SetVertexProgramConstant( hShader, index, pValues, rowOffset, rowCount );
	else
		SetFragmentProgramConstant( hShader, index, pValues, rowOffset, rowCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a single floating-point scalar into a shader constant row.

	The other components of the constant are set to zero.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetShaderConstant( const GcShaderHandle& hShader, 											    
										  uint index, 
										  float value, 
										  uint rowOffset )
{
	FW_ASSERT( hShader.IsValid() );
	if( hShader->GetType() == Gc::kVertexProgram )
		SetVertexProgramConstant( hShader, index, value, rowOffset );
	else
		SetFragmentProgramConstant( hShader, index, value, rowOffset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a vector value into a shader constant row.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetShaderConstant( const GcShaderHandle& hShader, 
										  uint index, 
										  const FwVector4& vec, 
										  uint rowOffset )
{
	FW_ASSERT( hShader.IsValid() );
	if( hShader->GetType() == Gc::kVertexProgram )
		SetVertexProgramConstant( hShader, index, vec, rowOffset );
	else
		SetFragmentProgramConstant( hShader, index, vec, rowOffset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets some or all of a matrix value into an array of shader constant rows.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetShaderConstant( const GcShaderHandle& hShader, 
										  uint index, 
										  const FwMatrix44& mat, 
										  uint rowOffset, 
										  uint rowCount )
{
	FW_ASSERT( hShader.IsValid() );
	if( hShader->GetType() == Gc::kVertexProgram )
		SetVertexProgramConstant( hShader, index, mat, rowOffset, rowCount );
	else
		SetFragmentProgramConstant( hShader, index, mat, rowOffset, rowCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets an array of constants with the given row offset.

	All rows from the offset to the end of the array are assumed to be valid and are uploaded to
	the program.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetVertexProgramConstant( const GcShaderHandle& hShader, 
												 uint index, 
												 const float* pValues, 
												 uint rowOffset )
{
	const GcShaderResource::Constant* pConstant = hShader->GetResource()->GetConstants() + index;
	FW_ASSERT( rowOffset < pConstant->m_rowCount );
	SetVertexProgramConstant( hShader, index, pValues, rowOffset, pConstant->m_rowCount - rowOffset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a single floating-point scalar into a shader constant row.

	The other components of the constant are set to zero.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetVertexProgramConstant( const GcShaderHandle& hShader, 											    
												 uint index, 
												 float value, 
												 uint rowOffset )
{
	float pValues[] = { value, 0.0f, 0.0f, 0.0f };
	SetVertexProgramConstant( hShader, index, pValues, rowOffset, 1 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a vector value into a shader constant row.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetVertexProgramConstant( const GcShaderHandle& hShader, 
 												 uint index, 
												 const FwVector4& vec, 
												 uint rowOffset )
{
	// alias the memory to floats
	union { v128 m_v; float m_f[4]; } adapter;
	adapter.m_v = vec.QuadwordValue();

	// call with a float array
	SetVertexProgramConstant( hShader, index, adapter.m_f, rowOffset, 1 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets some or all of a matrix value into an array of shader constant rows.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetVertexProgramConstant( const GcShaderHandle& hShader, 
												 uint index, 
												 const FwMatrix44& mat, 
												 uint rowOffset, 
												 uint rowCount )
{
	FW_ASSERT( rowCount <= 4 );

	// alias the memory to floats
	union { v128 m_v[4]; float m_f[16]; } adapter;
	adapter.m_v[0] = mat.GetRow( 0 ).QuadwordValue();
	adapter.m_v[1] = mat.GetRow( 1 ).QuadwordValue();
	adapter.m_v[2] = mat.GetRow( 2 ).QuadwordValue();
	adapter.m_v[3] = mat.GetRow( 3 ).QuadwordValue();

	// call with a float array
	SetVertexProgramConstant( hShader, index, adapter.m_f, rowOffset, rowCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets an array of constants with the given row offset.

	All rows from the offset to the end of the array are assumed to be valid and are uploaded to
	the program.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetFragmentProgramConstant( const GcShaderHandle& hShader, 
												   uint index, 
												   const float* pValues, 
												   uint rowOffset,
												   bool noProgramChange )
{
	const GcShaderResource::Constant* pConstant = hShader->GetResource()->GetConstants() + index;
	FW_ASSERT( rowOffset < pConstant->m_rowCount );
	SetFragmentProgramConstant( hShader, index, pValues, rowOffset, pConstant->m_rowCount - rowOffset, noProgramChange );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a single floating-point scalar into a shader constant row.

	The other components of the constant are set to zero.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetFragmentProgramConstant( const GcShaderHandle& hShader, 											    
												   uint index, 
												   float value, 
												   uint rowOffset,
												   bool noProgramChange )
{
	float pValues[] = { value, 0.0f, 0.0f, 0.0f };
	SetFragmentProgramConstant( hShader, index, pValues, rowOffset, 1, noProgramChange );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a vector value into a shader constant row.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetFragmentProgramConstant( const GcShaderHandle& hShader, 
												   uint index, 
												   const FwVector4& vec, 
												   uint rowOffset,
												   bool noProgramChange )
{
	// alias the memory to floats
	union { v128 m_v; float m_f[4]; } adapter;
	adapter.m_v = vec.QuadwordValue();

	// call with a float array
	SetFragmentProgramConstant( hShader, index, adapter.m_f, rowOffset, 1, noProgramChange );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets some or all of a matrix value into an array of shader constant rows.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetFragmentProgramConstant( const GcShaderHandle& hShader, 
												   uint index, 
												   const FwMatrix44& mat, 
												   uint rowOffset, 
												   uint rowCount,
												   bool noProgramChange )
{
	FW_ASSERT( rowCount <= 4 );

	// alias the memory to floats
	union { v128 m_v[4]; float m_f[16]; } adapter;
	adapter.m_v[0] = mat.GetRow( 0 ).QuadwordValue();
	adapter.m_v[1] = mat.GetRow( 1 ).QuadwordValue();
	adapter.m_v[2] = mat.GetRow( 2 ).QuadwordValue();
	adapter.m_v[3] = mat.GetRow( 3 ).QuadwordValue();

	// call with a float array
	SetFragmentProgramConstant( hShader, index, adapter.m_f, rowOffset, rowCount, noProgramChange );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets an array of floating-point values into vertex program constant storage.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetVertexProgramConstant( uint resourceIndex, 
												 const float* pValues, 
												 uint rowCount )
{
	FW_ASSERT( resourceIndex != GcShader::kInvalidIndex );
	SetVertexProgramConstants( resourceIndex, rowCount, pValues );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a single floating-point scalar into vertex program constant storage.

	The other components of the constant are set to zero.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetVertexProgramConstant( uint resourceIndex, float value )
{
	FW_ASSERT( resourceIndex != GcShader::kInvalidIndex );
	SetVertexProgramConstant( resourceIndex, value, 0.0f, 0.0f, 0.0f );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a vector value into vertex program constant storage.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetVertexProgramConstant( uint resourceIndex, const FwVector4& vec )
{
	FW_ASSERT( resourceIndex != GcShader::kInvalidIndex );
	
	// alias the memory to floats
	union { v128 m_v; float m_f[4]; } adapter;
	adapter.m_v = vec.QuadwordValue();

	// call with a float array
	SetVertexProgramConstant( resourceIndex, adapter.m_f );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets some or all of a matrix value into vertex program constant storage.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetVertexProgramConstant( uint resourceIndex, 
												 const FwMatrix44& mat, 
												 uint rowCount )
{
	FW_ASSERT( resourceIndex != GcShader::kInvalidIndex );

	// alias the memory to floats
	union { v128 m_v[4]; float m_f[16]; } adapter;
	adapter.m_v[0] = mat.GetRow( 0 ).QuadwordValue();
	adapter.m_v[1] = mat.GetRow( 1 ).QuadwordValue();
	adapter.m_v[2] = mat.GetRow( 2 ).QuadwordValue();
	adapter.m_v[3] = mat.GetRow( 3 ).QuadwordValue();

	// call with a float array
	SetVertexProgramConstants( resourceIndex, rowCount, adapter.m_f );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set a vertex attribute.

	@param		resourceIndex  	The vertex attribute index to bind to.
	@param		hStream			The stream.
	@param		fieldIndex		The field within the stream that contains the element.
	@param		offsetInBytes	The offset in bytes to the start of the vertices. This offset is
								to the start of the <b>vertex</b>. The offset from the start of the 
								vertex to the field is added automatically. Usually \c offsetInBytes 
								should be a whole number of vertices, but this is not enforced.
	@param		divider			The 16-bit frequency divider for instanced rendering. A value of
								0 mean the divider is disabled.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetVertexAttribute(	uint resourceIndex,
											const GcStreamBufferHandle& hStream,
											uint fieldIndex,
											uint offsetInBytes,
											uint divider )
{

	SetVertexAttributeInternal(resourceIndex, hStream, fieldIndex, offsetInBytes, divider);

	// :IMPORTANT: (10-10-06 vdiesi) Workaround for RSX hardware bug!
	InvalidatePostTransformCache();
	InvalidatePostTransformCache();
	InvalidatePostTransformCache();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set a texture.

	This function sets textures for fragment program reads.

	@param		resourceIndex	The hardware index to set the texture at.
	@param		hTexture		The texture handle.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetTexture( uint resourceIndex, const GcTextureHandle& hTexture )
{
	FW_ASSERT( resourceIndex < Gc::kMaxSamplers );
	FW_ASSERT( hTexture.IsValid() );

#ifdef ATG_DEBUG_MODE
	// check non-normalised reads are within restrictions
	if( !hTexture->IsNormalised() )
	{
		Gc::TexWrapMode wrapS = hTexture->GetWrapS();
		Gc::TexWrapMode wrapT = hTexture->GetWrapS();

		bool safeS = ( wrapS == Gc::kWrapModeClamp
			|| wrapS == Gc::kWrapModeClampToBorder
			|| wrapS == Gc::kWrapModeClampToEdge );
		bool safeT = ( wrapT == Gc::kWrapModeClamp
			|| wrapT == Gc::kWrapModeClampToBorder
			|| wrapT == Gc::kWrapModeClampToEdge );

		FW_ASSERT_MSG( safeS || FwIsPow2( hTexture->GetWidth() ), 
			( "If a non-normalised wrap or mirror mode is used, then that dimension must be a power of 2" ) );
		FW_ASSERT_MSG( safeT || FwIsPow2( hTexture->GetHeight() ), 
			( "If a non-normalised wrap or mirror mode is used, then that dimension must be a power of 2" ) );
	}

	// check filtering on fp32 textures
	Gc::TexFormat format = hTexture->GetFormat();
	if( format == Gc::kTexFormatR32F || format == Gc::kTexFormatRGBA32F )
	{
		FW_ASSERT_MSG( hTexture->GetMagFilter() == Gc::kFilterNearest, ( "Cannot filter fp32 textures, please use nearest mag filter" ) );
		FW_ASSERT_MSG( hTexture->GetMinFilter() == Gc::kFilterNearest || Gc::kFilterNearestMipMapNearest, ( "Cannot filter fp32 textures, please use nearest[mipmapnearest] min filter" ) );
	}
#endif

	// set on the hardware
	hTexture->CheckAddressValid();
	SetTexture( resourceIndex, &hTexture->m_texture );

	// Profile it
	GC_ADD_METRICS("NumSetTextureCalls", 1);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set a vertex program texture.

	This function sets textures for vertex program reads.

	@param		resourceIndex	The hardware index to set the texture at.
	@param		hTexture		The texture handle.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetVertexProgramTexture( uint resourceIndex, const GcTextureHandle& hTexture )
{
	FW_ASSERT( resourceIndex < Gc::kMaxVertexProgramSamplers );
	FW_ASSERT( hTexture.IsValid() );

	// Ensure the vertex texture type, format and filtering modes are valid.
	FW_ASSERT_MSG(hTexture->GetType() == Gc::kTexture2D,
						("Vertex texture type unsupported - must be Gc::kTexture2D"));
	
	FW_ASSERT_MSG((hTexture->GetFormat() == Gc::kTexFormatR32F) || (hTexture->GetFormat() == Gc::kTexFormatRGBA32F),
						("Vertex texture format unsupported - must be Gc::kTexFormatR32F or Gc::kTexFormatRGBA32F"));
	
	FW_ASSERT_MSG((hTexture->GetMinFilter() == Gc::kFilterNearest) || (hTexture->GetMinFilter() == Gc::kFilterNearestMipMapNearest),
						("Vertex texture min filter unsupported - must be Gc::kNearest or Gc::kFilterNearestMipMapNearest"));
	
	FW_ASSERT_MSG(hTexture->GetMagFilter() == Gc::kFilterNearest,
						("Vertex texture mag filter unsupported - must be Gc::kNearest"));

	FW_ASSERT_MSG( hTexture->IsNormalised(), ( "Vertex texturing must use normalised texture reads" ) );

	// set on the hardware
	hTexture->CheckAddressValid();
	SetVertexProgramTexture( resourceIndex, &hTexture->m_texture );

	// Profile it
	GC_ADD_METRICS("NumSetTextureCalls", 1);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the vertex shader.

	@param		hShader		A valid vertex shader.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SetVertexShader( const GcShaderHandle& hShader )
{
	FW_ASSERT( hShader.IsValid() );
	FW_ASSERT( hShader->GetType() == Gc::kVertexProgram );

	// Send to the pushbuffer
	const Ice::Render::VertexProgram* pProgram = hShader->GetResource()->GetVertexProgram();
	SetVertexProgram( pProgram );

	// Profile it
	GC_ADD_METRICS("NumSetVertexShaderCalls", 1);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Loads the vertex shader instructions.

	@param		hShader		A valid vertex shader.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::LoadVertexShader( const GcShaderHandle& hShader )
{
	FW_ASSERT( hShader.IsValid() );
	FW_ASSERT( hShader->GetType() == Gc::kVertexProgram );

	// Send to the pushbuffer
	const Ice::Render::VertexProgram* pProgram = hShader->GetResource()->GetVertexProgram();
	LoadVertexProgram( pProgram );

	// Profile it
	GC_ADD_METRICS("NumSetVertexShaderCalls", 1);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Selects the loaded vertex shader.

	@param		hShader		A valid vertex shader.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::SelectVertexShader( const GcShaderHandle& hShader )
{
	FW_ASSERT( hShader.IsValid() );
	FW_ASSERT( hShader->GetType() == Gc::kVertexProgram );

	// Send to the pushbuffer
	const Ice::Render::VertexProgram* pProgram = hShader->GetResource()->GetVertexProgram();
	SelectVertexProgram( pProgram );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Draws non-indexed primitives.
**/
//--------------------------------------------------------------------------------------------------

inline void GcContext::DrawArrays( Gc::PrimitiveType primType, uint start, uint count )
{
	FW_ASSERT( count != 0 );

	// send to the pushbuffer
	DrawArrays( ( Ice::Render::DrawMode )primType, start, count );

	GC_ADD_METRICS("NumDrawArraysCalls", 1);
	GC_ADD_METRICS("NumVertices", count);
}

#endif // ndef GC_CONTEXT_INL_H
