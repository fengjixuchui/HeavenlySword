//--------------------------------------------------
//!
//!	\file renderer_ps3.h
//! The PS3 version of the renderer singleton
//!
//--------------------------------------------------

#if !defined( GFX_RENDERER_PS3_H_ )
#define GFX_RENDERER_PS3_H_

#ifndef GC_KERNEL_H
#include <Gc/GcKernel.h>
#endif

#ifndef GFX_VERTEXDECLARATION_H
#include "gfx/vertexdeclaration.h"
#endif

#ifndef GFX_DEBUGSHADER_PS3_H
#include "gfx/debugshader_ps3.h"
#endif

#ifndef GFX_GPUREPORTS_PS3_H
#include "gfx/gpureports_ps3.h"
#endif

class Renderer;

#if !defined( CORE_DOUBLEENDERFRAMEALLOCATOR_H )
#include "core/doubleenderframeallocator.h"
#endif

//! The renderer for all game objects.
class RendererPlatform
{
public:

	friend class Renderer;

	Renderer* m_pThis;

	//! Sets the texture on a given sampler stage.
	void SetTexture( int iStage, GcTextureHandle pTexture, bool bForce = false );

	//! Gets the amount of RAM currently being managed by the Direct3D graphics device (0.0f to 1.0f range)
	float GetMemoryUsage() const;

	//! Wrapper on GcKernel::SetStream. MUST be called after Renderer::SetVertexShader
	//! NOTE! bound streams must be cleared when rendering is finished
	uint32_t SetStream( const VBHandle& hStream, uint32_t offsetInBytes = 0, uint32_t divider = 0 );

	//! Wrapper on GcKernel::SetStream. MUST be called after Renderer::SetVertexShader
	void ClearStreams();

	//! Wrappered GC draw calls
	void DrawPrimitives( Gc::PrimitiveType primType, u_int iStartVertex, u_int iVertexCount );
	void DrawIndexedPrimitives( Gc::PrimitiveType primType, u_int iStartIndex, u_int iIndexCount, const IBHandle& hIndices );	
	
	//! Called to signal we've adjusted some PS parameters, and the cached PS may need refreshing.
	inline void ForcePSRefresh() { m_bForcePSRefresh = true; }

	// so we can validate filter modes
	static inline bool Is32bitFloatFormat( Gc::TexFormat fmt )
	{
		if	(
			(fmt == Gc::kTexFormatR32F) ||
			(fmt == Gc::kTexFormatRGBA32F)
			)
			return true;
		return false;
	}

	// so we can chose an appropriate clear method
	static inline bool IsFloatBufferFormat( Gc::BufferFormat fmt )
	{
		if	(
			(fmt == Gc::kBufferFormatRGBA16F) ||
			(fmt == Gc::kBufferFormatRGBA32F) ||
			(fmt == Gc::kBufferFormatR32F)
			)
			return true;
		return false;
	}

	static VBHandle CreateVertexStream(	int vertexCount,
										int vertexStride,
										int	fieldCount,
										const GcStreamField*	pFieldArray,
										Gc::BufferType bufferType  = Gc::kStaticBuffer,
										void* pMem = NULL);

	static IBHandle	CreateIndexStream(	Gc::StreamIndexType indexType,
										uint count,
										Gc::BufferType bufferType = Gc::kStaticBuffer,
										void* pMem = NULL);
	//! Presents the device.
	void PostPresent();
	void FlushCaches();

	//! gets the start of the end push buffer
	void* GetStartPushBufferAddr();

	//! gets the current end push buffer address
	void* GetCurrentPushBufferAddr();

	//! dumps the push buffer between start and end to a file called Name (remember app_home)
	void DumpPushBuffer( void* pStart, void *pEnd, const char* pName );

	#ifdef _PROFILING
	void SetProfilePSSuspension( bool bOn ) { m_ProfilePSSuspended = bOn; }
	#endif

	//! helper function for all the fullscreen passes we have scattered about
	void DrawFullscreenQuad();

	DoubleEnderFrameAllocator	m_PixelShaderMainSpace;

#ifdef _PROFILING
	GPUReport*	m_pGPUReport;
#endif

private:
	void ResetTextureStates();	//!< Resets the texture stage state cache.
	void SubmitTextures();		//!< submits bound textures to the GPU

	const GcTextureHandle& GetNullTexure();

	GFX_FILL_MODE m_eFillMode;	//!< Our drawing mode

	static const u_int	MAX_SAMPLERS = 16;

	// Platform specific texture and sampler state cache
	GcTextureHandle				m_aTextures[MAX_SAMPLERS];						//!< The currently set textures.
	TEXTUREADDRESS_TYPE			m_aTexAddrMode[MAX_SAMPLERS];					//!< The current sampler address mode.
	TEXTUREFILTER_TYPE			m_aTexFilterModes[MAX_SAMPLERS];				//!< The current sampler filter mode.
	Gc::AnisotropyLevel			m_aTexAnisotropicFilterLevel[MAX_SAMPLERS];		//!< The current sampler anisotropic filtering state
	uint32_t					m_iTexturesToSubmit;


	// Our zero texture when rendering from an empty stage
	Texture::Ptr	m_pNullTexture;
	
	// signal to modify our caching behaviour.
	bool			m_bForcePSRefresh;	

	// validation bool to make sure our stream is set
	bool			m_bStreamSet;	

	// only valid when profiling
#ifdef _PROFILING
	bool			m_ProfilePSSuspended;
	DebugShader *	m_suspendedPS;
#endif

	// handly shader and geometry for fullscreen passes on PS3
	DebugShader	*	m_fullscreenVS;
	VBHandle		m_fullscreenData;

	uintptr_t		m_GCMemSpace;
};


#endif // end GFX_RENDERER_PS3_H
