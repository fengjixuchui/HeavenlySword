//--------------------------------------------------------------------------------------------------
/**
	@file

	@brief		Provides validation for straight-line graphics code.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_VALIDATION_H
#define GC_VALIDATION_H

#include <Gc/Gc.h>
#include <Fw/FwStd/FwHashedString.h>

#ifdef ATG_DEBUG_MODE
#define GC_STATE_VALIDATION_ENABLED
#endif // def ATG_DEBUG_MODE

//--------------------------------------------------------------------------------------------------
/**
	@class

	@brief		Render buffer object.
**/
//--------------------------------------------------------------------------------------------------

class GcValidation
{
public:
	GcValidation();

	void EnableValidation( bool enabled );

	void SetRenderTarget( const GcRenderBufferHandle& hDepthBuffer, 
						  const GcRenderBufferHandle& hColourBuffer0, 
						  const GcRenderBufferHandle& hColourBuffer1, 
						  const GcRenderBufferHandle& hColourBuffer2, 
						  const GcRenderBufferHandle& hColourBuffer3 );
	void Clear( int clearBits );
	void Present();

	void SetStream( const GcShaderHandle& hVertexShader, const GcStreamBufferHandle& hStream, uint divider );
	void ClearStreams( const GcShaderHandle& hVertexShader );

	void SetVertexAttribute( uint resourceIndex, uint count, uint divider );
	void SetVertexAttribute( uint resourceIndex );
	void DisableVertexAttribute( uint resourceIndex );
	void SetVertexAttributeFrequencyMode( uint mode );

	void SetTexture( uint resourceIndex, const GcTextureHandle& hTexture );
	void DisableTexture( uint resourceIndex );
	
	void SetVertexProgramTexture( uint resourceIndex, const GcTextureHandle& hTexture );
	void DisableVertexProgramTexture( uint resourceIndex );

	void SetShaderConstant( const GcShaderHandle& hShader, bool noProgramChange = false );

	void SetVertexShader( const GcShaderHandle& hShader );
	void SetFragmentShader( const GcShaderHandle& hShader );
	void RefreshFragmentShader( const GcShaderHandle& hShader );

	void DrawArrays( uint start, uint count );
	void DrawElements();

	void Enable( Gc::RenderState state );
	void Disable( Gc::RenderState state );

	void SetColourMask( bool red, bool green, bool blue, bool alpha );
	void SetDepthMask( bool depth );
	void SetMultisampleParameters( bool enabled );

private:
	void Draw();

#ifdef GC_STATE_VALIDATION_ENABLED
	
	static const uint	kMaxSamplerResourceIndices = Gc::kMaxSamplers;

#ifdef ATG_PC_PLATFORM
	// :NOTE: (vdiesi -- 14-11-05) Account for vertex shader sampler resource differences between platforms.
	static const uint	kMaxVertexSamplerResourceIndices = kMaxSamplerResourceIndices;
#else
	static const uint	kMaxVertexSamplerResourceIndices = Gc::kMaxVertexProgramSamplers;
#endif
	
	bool				m_validationEnabled;

	bool				m_pValidAttributes[ Gc::kMaxAttributes ];			//!< True for valid attributes.
	bool				m_pDividerAttributes[ Gc::kMaxAttributes ];			//!< True for attributes that have non-zero dividers.
	bool				m_pExpectedAttributes[ Gc::kMaxAttributes ];		//!< Expected vertex shader attributes.
	FwHashedString		m_pExpectedAttributeHashes[ Gc::kMaxAttributes ];	//!< Expected vertex shader attribute hashes.
	uint				m_pAttributeVertexCounts[ Gc::kMaxAttributes ];		//!< The number of vertices available on each attribute.
	uint				m_pAttributeFrequencyDivider[ Gc::kMaxAttributes ];	//!< The frequency divider set for this attribute.
	uint				m_vertexAttributeFrequencyMode;						//!< Bit field containing modulo/divide mode for each attribute.

	bool				m_pValidSamplers[ kMaxSamplerResourceIndices ];						//!< True for valid samplers.
	bool				m_pExpectedSamplers[ kMaxSamplerResourceIndices ];					//!< Expected samplers.
	FwHashedString		m_pExpectedSamplerHashes[ kMaxSamplerResourceIndices ];				//!< Expected sampler hashes.

	bool				m_pValidVertexSamplers[ kMaxVertexSamplerResourceIndices ];			//!< True for valid vertex program samplers.
	bool				m_pExpectedVertexSamplers[ kMaxVertexSamplerResourceIndices ];		//!< Expected vertex program samplers.
	FwHashedString		m_pExpectedVertexSamplerHashes[ kMaxVertexSamplerResourceIndices ];	//!< Expected vertex program sampler hashes.

	bool				m_validDepthBuffer;									//!< True when valid depth buffer.
	bool				m_pValidColourBuffers[ Gc::kMaxRenderTargets ];		//!< True for valid colour buffers.
	bool				m_renderTargetMultisampled;							//!< True when the render target is multi-sampled.
	Gc::BufferFormat	m_colourBufferFormat;								//!< The current colour buffer format.
	uint				m_colourMask;										//!< A bitwise representation of colour mask.
	bool				m_depthMask;										//!< The current depth write mask.
	bool				m_depthTestEnabled;									//!< True if the depth test is enabled.
	bool				m_multiSampleEnabled;								//!< True if multi-sampling is enabled.
	bool				m_alphaTestEnabled;									//!< True if alpha-test is enabled.
	
	bool				m_validVertexShader;								//!< True when vertex shader valid.	
	
	bool				m_validFragmentShader;								//!< True when fragment shader valid.	
	void const*			m_fragmentShader;									//!< Current fragment program microcode address.
	bool				m_validFragmentShaderConstants;						//!< True when constants valid.
	void const*			m_lastFragmentProgramConstantDest;					//!< The last fragment program constant microcode address.

	bool				m_gammaCorrectedWritesEnabled;		//!< True when gamma-correcting writes.
	bool				m_fragmentProgramWritesHalves;		//!< True when fragment program writes half results.
#endif // def GC_STATE_VALIDATION_ENABLED
};

#ifndef GC_STATE_VALIDATION_ENABLED
inline GcValidation::GcValidation() {}
inline void GcValidation::EnableValidation( bool ) {}
inline void GcValidation::SetRenderTarget( const GcRenderBufferHandle&, const GcRenderBufferHandle&, const GcRenderBufferHandle&, const GcRenderBufferHandle&, const GcRenderBufferHandle& ) {}
inline void GcValidation::Clear( int ) {}
inline void GcValidation::Present() {}
inline void GcValidation::SetStream( const GcShaderHandle&, const GcStreamBufferHandle&, uint ) {}
inline void GcValidation::ClearStreams( const GcShaderHandle&) {}
inline void GcValidation::SetVertexAttribute( uint ) {}
inline void GcValidation::SetVertexAttribute( uint, uint, uint ) {}
inline void GcValidation::DisableVertexAttribute( uint ) {}
inline void GcValidation::SetVertexAttributeFrequencyMode( uint ) {}
inline void GcValidation::SetTexture( uint, const GcTextureHandle& ) {}
inline void GcValidation::DisableTexture( uint ) {}
inline void GcValidation::SetVertexProgramTexture( uint, const GcTextureHandle& ) {}
inline void GcValidation::DisableVertexProgramTexture( uint ) {}
inline void GcValidation::SetShaderConstant( const GcShaderHandle&, bool ) {}
inline void GcValidation::SetVertexShader( const GcShaderHandle& ) {}
inline void GcValidation::SetFragmentShader( const GcShaderHandle& ) {}
inline void GcValidation::RefreshFragmentShader( const GcShaderHandle& ) {}
inline void GcValidation::DrawArrays( uint, uint ) {}
inline void GcValidation::DrawElements() {}
inline void GcValidation::Enable( Gc::RenderState ) {}
inline void GcValidation::Disable( Gc::RenderState ) {}
inline void GcValidation::SetColourMask( bool, bool, bool, bool ) {}
inline void GcValidation::SetDepthMask( bool ) {}
inline void GcValidation::SetMultisampleParameters( bool ) {}
inline void GcValidation::Draw() {}
#endif // ndef GC_STATE_VALIDATION_ENABLED

#endif // ndef GC_VALIDATION_H
