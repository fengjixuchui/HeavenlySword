//--------------------------------------------------------------------------------------------------
/**
	@file

	@brief		Graphics command context.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_CONTEXT_H
#define GC_CONTEXT_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/Gc.h>
#include <Gc/GcRenderBuffer.h>
#include <Gc/GcShader.h>
#include <Gc/GcStreamBuffer.h>
#include <Gc/GcTexture.h>
#include <Fw/FwMaths/FwVector4.h>
#include <Fw/FwMaths/FwMatrix44.h>

// Define our reserve check in terms of InlineUnsafe as GcContext derives from it

#ifdef ATG_DEBUG_MODE
#define ATG_CHECK_RESERVE( a ) FW_ASSERT( WillReserveFail( a ) == false )
#else
#define ATG_CHECK_RESERVE( a )
#endif

//--------------------------------------------------------------------------------------------------
//  CLASS DECLARATIONS
//--------------------------------------------------------------------------------------------------

class GcContext : public Ice::Render::InlineUnsafe::CommandContext, public FwNonCopyable
{
public:
	// Address access

	void* GetBeginAddress() const;
	void* GetCurrentAddress() const;

	// Setting up a render target using GcRenderBuffer

	using Ice::Render::InlineUnsafe::CommandContext::SetRenderTarget;

	void SetRenderTarget( const GcRenderBufferHandle& hDepthBuffer, 
						  const GcRenderBufferHandle& hColourBuffer0, 
						  const GcRenderBufferHandle& hColourBuffer1 = GcRenderBufferHandle(), 
						  const GcRenderBufferHandle& hColourBuffer2 = GcRenderBufferHandle(), 
						  const GcRenderBufferHandle& hColourBuffer3 = GcRenderBufferHandle() );

	// Setting vertex or fragment shader constants through GcShader and Fw types

	void SetShaderConstant( const GcShaderHandle& hShader, 
							uint index, 
							const float* pValues, 
							uint rowOffset = 0 );

	void SetShaderConstant( const GcShaderHandle& hShader, 
							uint index, 
							const float* pValues, 
							uint rowOffset, 
							uint rowCount );

	void SetShaderConstant( const GcShaderHandle& hShader, 
							uint index, 
							float value, 
							uint rowOffset = 0 );

	void SetShaderConstant( const GcShaderHandle& hShader, 
							uint index, 
							const FwVector4& vec, 
							uint rowOffset = 0 );

	void SetShaderConstant( const GcShaderHandle& hShader, 
							uint index, 
							const FwMatrix44& mat, 
							uint rowOffset = 0, 
							uint rowCount = 4 );

	// Setting vertex shader constants through GcShader and Fw types

	void SetVertexProgramConstant( const GcShaderHandle& hShader, 
								   uint index, 
								   const float* pValues, 
								   uint rowOffset = 0 );

	void SetVertexProgramConstant( const GcShaderHandle& hShader, 
								   uint index, 
								   const float* pValues, 
								   uint rowOffset,
								   uint rowCount );

	void SetVertexProgramConstant( const GcShaderHandle& hShader, 
								   uint index, 
								   float value, 
								   uint rowOffset = 0 );

	void SetVertexProgramConstant( const GcShaderHandle& hShader, 
								   uint index, 
								   const FwVector4& vec, 
								   uint rowOffset = 0 );

	void SetVertexProgramConstant( const GcShaderHandle& hShader, 
								   uint index, 
								   const FwMatrix44& mat, 
								   uint rowOffset = 0, 
								   uint rowCount = 4 );

	// Setting fragment shader constants through GcShader and Fw types

	void SetFragmentProgramConstant( const GcShaderHandle& hShader, 
								     uint index, 
									 const float* pValues, 
									 uint rowOffset = 0,
									 bool noProgramChange = false );

	void SetFragmentProgramConstant( const GcShaderHandle& hShader, 
									 uint index, 
									 const float* pValues, 
									 uint rowOffset,
									 uint rowCount,
									 bool noProgramChange = false );

	void SetFragmentProgramConstant( const GcShaderHandle& hShader, 
									 uint index, 
									 float value, 
									 uint rowOffset = 0,
									 bool noProgramChange = false );

	void SetFragmentProgramConstant( const GcShaderHandle& hShader, 
									 uint index, 
									 const FwVector4& vec, 
									 uint rowOffset = 0,
									 bool noProgramChange = false );

	void SetFragmentProgramConstant( const GcShaderHandle& hShader, 
									 uint index, 
									 const FwMatrix44& mat, 
									 uint rowOffset = 0, 
									 uint rowCount = 4,
									 bool noProgramChange = false );

	// Expose the base Ice::Render::Context overloads

	using Ice::Render::InlineUnsafe::CommandContext::SetVertexProgramConstant;
	using Ice::Render::InlineUnsafe::CommandContext::SetFragmentProgramConstant;

	// Direct vertex program constant access using Fw types

	void SetVertexProgramConstant( uint resourceIndex, 
								   const float* pValues, 
								   uint rowCount );

	void SetVertexProgramConstant( uint resourceIndex, 
								   float value );

	void SetVertexProgramConstant( uint resourceIndex, 
								   const FwVector4& vec );

	void SetVertexProgramConstant( uint resourceIndex, 
								   const FwMatrix44& mat, 
								   uint rowCount = 4 );

	// Setting vertex attributes using GcStreamBuffer and GcShader

	void SetVertexAttribute( uint resourceIndex, 
							 const GcStreamBufferHandle& hStream, 
							 uint fieldIndex, 
							 uint offsetInBytes = 0, 
							 uint divider = 0 );

	uint SetStream( const GcShaderHandle& hVertexShader, 
					const GcStreamBufferHandle& hStream, 
					uint offsetInBytes = 0, 
					uint divider = 0 );

	void ClearStreams( const GcShaderHandle& hVertexShader );

	// Setting textures using GcTexture

	using Ice::Render::InlineUnsafe::CommandContext::SetTexture;
	using Ice::Render::InlineUnsafe::CommandContext::SetVertexProgramTexture;

	void SetTexture( uint resourceIndex, const GcTextureHandle& hTexture );
	void SetVertexProgramTexture( uint resourceIndex, const GcTextureHandle& hTexture );

	// Setting vertex or fragment shaders using GcShader

	void SetVertexShader( const GcShaderHandle& hShader );

	void LoadVertexShader( const GcShaderHandle& hShader );
	void SelectVertexShader( const GcShaderHandle& hShader );

	void SetFragmentShader( const GcShaderHandle& hShader );
	void RefreshFragmentShader( const GcShaderHandle& hShader );

	// Indexed primitives using GcStreamBuffer

	using Ice::Render::InlineUnsafe::CommandContext::DrawArrays;
	using Ice::Render::InlineUnsafe::CommandContext::DrawElements;

	void DrawArrays( Gc::PrimitiveType primType, uint start, uint count );

	void DrawElements( Gc::PrimitiveType primType, 
					   uint start, 
					   uint count, 
					   const GcStreamBufferHandle& hIndices );

private:

	void SetVertexAttributeInternal(	uint resourceIndex,
										const GcStreamBufferHandle& hStream,
										uint fieldIndex,
										uint offsetInBytes,
										uint divider );
					   
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gc/GcContext.inl>

//--------------------------------------------------------------------------------------------------

#endif // ndef GC_CONTEXT_H
