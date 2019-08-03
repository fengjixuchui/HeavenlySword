//--------------------------------------------------------------------------------------------------
/**
	@file

	@brief		Graphics Core Kernel - Provides initialisation, shutdown and the basic building blocks
				for rendering.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_KERNEL_H
#define GC_KERNEL_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/Gc.h>
#include <Gc/GcContext.h>
#include <Gc/GcValidation.h>

#ifndef ATG_PC_PLATFORM
#include <sys/raw_spu.h>
#endif

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class		

	@brief		Provides initialisation, shutdown and the basic building blocks for rendering.
**/
//--------------------------------------------------------------------------------------------------

class GcKernel
{
public:

	// Initialisation and Shutdown

	static int QueryMemoryRequirementsInBytes( void );

	static void Initialise( const GcInitParams& initParams, void* pClassMemory = NULL );
	static void Shutdown( void );

	static bool IsInitialised();

	// GPU Timeout Callback Function
    
	typedef	void	( TimeoutCallbackFunc )( void* pLastKickedStart, void* pLastKickedEnd );
	
	static void	RegisterTimeoutCallback( TimeoutCallbackFunc* pCallback );
	static TimeoutCallbackFunc*	GetRegisteredTimeoutCallback( void );
	
	// GPU Finished Callback Function
    
	typedef	void	( FinishCallbackFunc )( void* pLastKickedStart, void* pLastKickedEnd );
	
	static void	RegisterFinishCallback( FinishCallbackFunc* pCallback );
	static FinishCallbackFunc*	GetRegisteredFinishCallback( void );

	// Allocation Failure Callback Function
    
	typedef	void	( AllocFailureCallbackFunc )( uint sizeInBytes, uint alignment, Gc::MemoryContext context );
	
	static void	RegisterAllocFailureCallback( AllocFailureCallbackFunc* pCallback );
	static AllocFailureCallbackFunc*	GetRegisteredAllocFailureCallback( void );

	// Display mode access

	static void SetDisplaySwapMode( Gc::DisplaySwapMode mode );

	// Current context access

	static GcContext& GetContext();

	// Back buffer accessors

	static const GcRenderBufferHandle& GetBackBuffer( void );
	static const GcRenderBufferHandle& GetFrontBuffer( void );

	// End of frame operations

	static u32 Present( bool stallGpu = false );
	static u32 Present( uint syncFrameReference, bool stallGpu = false );

	static void SyncPreviousFrame( void );

	// Frame buffer operations

	static void SetRenderTarget( const GcRenderBufferHandle& hDepthBuffer, 
								 const GcRenderBufferHandle& hColourBuffer0, 
								 const GcRenderBufferHandle& hColourBuffer1 = GcRenderBufferHandle(), 
								 const GcRenderBufferHandle& hColourBuffer2 = GcRenderBufferHandle(), 
								 const GcRenderBufferHandle& hColourBuffer3 = GcRenderBufferHandle() );

	static void Clear( int clearBits );

	// Constant set operations 

	static void SetShaderConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset = 0 );
	static void SetShaderConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset, uint rowCount );
	static void SetShaderConstant( const GcShaderHandle& hShader, uint index, float value, uint rowOffset = 0 );
	static void SetShaderConstant( const GcShaderHandle& hShader, uint index, const FwVector4& vec, uint rowOffset = 0 );
	static void SetShaderConstant( const GcShaderHandle& hShader, uint index, const FwMatrix44& mat, uint rowOffset = 0, uint rowCount = 4 );

	static void SetVertexProgramConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset = 0 );
	static void SetVertexProgramConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset, uint rowCount );
	static void SetVertexProgramConstant( const GcShaderHandle& hShader, uint index, float value, uint rowOffset = 0 );
	static void SetVertexProgramConstant( const GcShaderHandle& hShader, uint index, const FwVector4& vec, uint rowOffset = 0 );
	static void SetVertexProgramConstant( const GcShaderHandle& hShader, uint index, const FwMatrix44& mat, uint rowOffset = 0, uint rowCount = 4 );

	static void SetFragmentProgramConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset = 0, bool noProgramChange = false );
	static void SetFragmentProgramConstant( const GcShaderHandle& hShader, uint index, const float* pValues, uint rowOffset, uint rowCount, bool noProgramChange = false );
	static void SetFragmentProgramConstant( const GcShaderHandle& hShader, uint index, float value, uint rowOffset = 0, bool noProgramChange = false );
	static void SetFragmentProgramConstant( const GcShaderHandle& hShader, uint index, const FwVector4& vec, uint rowOffset = 0, bool noProgramChange = false );
	static void SetFragmentProgramConstant( const GcShaderHandle& hShader, uint index, const FwMatrix44& mat, uint rowOffset = 0, uint rowCount = 4, bool noProgramChange = false );

	// Constant register set operations 

	static void SetVertexProgramConstant( uint resourceIndex, const float* pValues, uint rowCount );
	static void SetVertexProgramConstant( uint resourceIndex, float value );
	static void SetVertexProgramConstant( uint resourceIndex, const FwVector4& vec );
	static void SetVertexProgramConstant( uint resourceIndex, const FwMatrix44& mat, uint rowCount = 4 );

	// Stream set operations
	
	static uint SetStream( const GcShaderHandle& hVertexShader, const GcStreamBufferHandle& hStream, uint offsetInBytes = 0, uint divider = 0 );
	static void ClearStreams( const GcShaderHandle& hVertexShader );

	static void SetVertexAttribute( uint resourceIndex, const GcStreamBufferHandle& hStream, uint fieldIndex, uint offsetInBytes = 0, uint divider = 0 );
	static void SetVertexAttribute1f( uint resourceIndex, float x );
	static void SetVertexAttribute2f( uint resourceIndex, float x, float y );
	static void SetVertexAttribute3f( uint resourceIndex, float x, float y, float z );
	static void SetVertexAttribute4f( uint resourceIndex, float x, float y, float z, float w );
	static void SetVertexAttribute2s( uint resourceIndex, short x, short y );
	static void SetVertexAttribute4s( uint resourceIndex, short x, short y, short z, short w );
	static void SetVertexAttribute4Ns( uint resourceIndex, short x, short y, short z, short w );
	static void SetVertexAttribute4Nub( uint resourceIndex, u8 x, u8 y, u8 z, u8 w );
	
	static void SetVertexAttributeFrequencyMode( uint mode );
	
	static void DisableVertexAttribute( uint resourceIndex );

	// Texture set operations

	static void SetTexture( uint resourceIndex, const GcTextureHandle& hTexture );
	static void DisableTexture( uint resourceIndex );
	
	static void SetVertexProgramTexture( uint resourceIndex, const GcTextureHandle& hTexture );
	static void DisableVertexProgramTexture( uint resourceIndex );

	// Shader binding operations

	static void SetVertexShader( const GcShaderHandle& hShader );

	static void LoadVertexShader( const GcShaderHandle& hShader );
	static void SelectVertexShader( const GcShaderHandle& hShader );

	static void SetFragmentShader( const GcShaderHandle& hShader );
	static void RefreshFragmentShader( const GcShaderHandle& hShader );

	// Draw functions

	static void DrawArrays( Gc::PrimitiveType primType, uint start, uint count );
	static void DrawElements( Gc::PrimitiveType primType, uint start, uint count, const GcStreamBufferHandle& hIndices );

	// Render state

	static void Enable( Gc::RenderState state );
	static void Disable( Gc::RenderState state );
	
	// State arguments
	
	static void SetAlphaFunc( Gc::CmpFunc func, float ref );
	static void SetBlendEquation( Gc::BlendEquation mode );
	static void SetBlendEquationSeparate( Gc::BlendEquation modeRgb, Gc::BlendEquation modeAlpha );
	static void SetBlendFunc( Gc::BlendFunc src, Gc::BlendFunc dest );
	static void SetBlendFuncSeparate( Gc::BlendFunc srcRgb,	Gc::BlendFunc destRgb, Gc::BlendFunc srcAlpha, Gc::BlendFunc destAlpha );
	static void SetBlendColour( float red, float green, float blue, float alpha );
	static void SetFloatBlendColour( float red, float green, float blue, float alpha );
	static void SetClearColour( float red, float green, float blue, float alpha );
	static void SetClearDepthStencil( float depth, int stencil );
	static void SetClearDepthStencil( int value );
	static void SetColourMask( bool red, bool green, bool blue, bool alpha );
	static void SetCullFace( Gc::CullFaceMode mode );
	static void SetDepthBounds( float zmin, float zmax );
	static void SetDepthMinMaxControl( bool cullNearFarEnable, bool clampEnable, bool cullIgnoreW );
	static void SetDepthTest( Gc::CmpFunc func );
	static void SetDepthMask( bool depth );
	static void SetFogMode( Gc::FogMode mode );
	static void SetFogRange( float fmin, float fmax );
	static void SetFogDensity( float density );
	static void SetFrontFace( Gc::FaceType face );
	static void SetLineWidth( float width );	
	static void SetLogicOp( Gc::LogicOp logicOp );
	static void SetMrtBlendEnable( u32 mask );
	static void SetMrtColourMask( u16 mask );
	static void SetMultisampleParameters( bool enabled, bool alphaToCoverage, bool alphaToOne, u16 coverageMask );
	static void SetPointSize( float psize );
	static void SetPointSpriteParameters( bool enabled, uint texCoordMask, Gc::SpriteMode mode );
	static void SetPolygonMode( Gc::PolygonMode mode);
	static void SetPolygonModeSeparate( Gc::PolygonMode modeFront, Gc::PolygonMode modeBack );
	static void SetPolygonOffset( float factor, float units );
	static void SetPrimitiveRestartIndex( uint index );
	static void SetScissor( int x, int y, int w, int h );
	static void SetShadeModel( Gc::ShadeModel shadeModel );
	static void SetStencilTest( Gc::CmpFunc func, int ref, int mask );
	static void SetStencilTestSeparate( Gc::CmpFunc funcFront, int refFront, int maskFront, Gc::CmpFunc funcBack, int refBack, int maskBack );
	static void SetStencilOp( Gc::StencilOp stencilFail, Gc::StencilOp depthFail, Gc::StencilOp depthPass );
	static void SetStencilOpSeparate( Gc::StencilOp stencilFailFront, Gc::StencilOp depthFailFront, Gc::StencilOp depthPassFront, Gc::StencilOp stencilFailBack, Gc::StencilOp depthFailBack, Gc::StencilOp depthPassBack );
    static void SetStencilMask( int mask );
    static void SetStencilMaskSeparate( int maskFront, int MaskBack );
	static void SetViewport( int x, int y, int w, int h, float zmin = 0.0f, float zmax = 1.0f );

	// Flow control

    static void InsertJump( void* pAddr );
    static void InsertCall( void* pAddr );
    static void InsertReturn();

	// Counters

	static u64 GetPresentCounter( void );

	// Enable or disable validation

	static void EnableValidation( bool enabled );

	// Report buffer memory accessor
	
	static void GetReportBufferMemoryInfo(void** ppBase, u32* pSize);
	
	// Scratch memory accessors

	static void SetTotalScratchMemory( uint sizeInBytes );
	static bool QueryGetNewScratchMemory( uint sizeInBytes, uint alignment );
	static void* GetNewScratchMemory( uint sizeInBytes, uint alignment );
	static bool IsValidScratchAddress( void const* pAddress );

	// User memory allocators

	static bool QueryAllocateHostMemory( uint sizeInBytes, uint alignment = 128 );
	static void* AllocateHostMemory( uint sizeInBytes, uint alignment = 128 );
	static void FreeHostMemory( void* pAddress );
	
	static bool QueryAllocateVram( uint sizeInBytes, uint alignment = 128 );
	static void* AllocateVram( uint sizeInBytes, uint alignment = 128 );
	static void FreeVram( void* pAddress );
	
	static uint GetFreeHostMemory();
	static uint GetFreeVram();
	
	// Managed memory info
	
	static void GetManagedHostMemoryInfo(void** ppBase, u32* pSize);
	static void GetManagedVideoMemoryInfo(void** ppBase, u32* pOffset, u32* pSize);
	
	// Last push buffer (i.e. context) kicked accessor

	static void GetLastContextKickedPointers(void** ppStart, void** ppEnd);

    // Reference value that was used to sync the last frame kicked with GcKernel::Present()
	
	static uint GetLastSyncFrameReference();

#ifdef ATG_PC_PLATFORM
	static void ResizeColourBuffers( uint width, uint height );		// PC only
#endif

	// Get display dimensions

	static u32	GetDisplayWidth();
	static u32	GetDisplayHeight();
	
	// Get display pitch
	
	static u32	GetDisplayPitch();

	// Get aspect ratio
	static Gc::AspectRatio GetDisplayAspectRatio();

	// Get aspect ratio value
	static float GetDisplayAspectRatioValue();
	    
	// Get Validation State :NOTE: Use with care
	
	static GcValidation&	GetValidationState();

private:
	
	// Functions to track host and video memory allocations. So we can make sure
	// all allocations have been freed before shutdown.
	
	static void HostMemoryAddRef();
	static void HostMemoryRelease();
	
	static void VramAddRef();
	static void VramRelease(); 

	// Synchronises to the previous frame before a present
	
	static void SyncPreviousFrameInternal(const char* pFuncName);
	
	// Resets the scratch memory at the beginning of a frame

	static void ResetFrameScratchMemory();

	// Utility function for half precision conversion

	static short FloatToHalf( int i );
	
	// Constants
	
#ifdef ATG_ASSERTS_ENABLED
	static const char*			ms_pIsUninitialisedMessage;
#endif

	// Aggregated member-attributes for TOC usage reduction.
	
	struct Instance
	{
		bool					m_isInitialised;
		
		u8*						m_pClassMem;
		
		bool					m_ownsHostMem;
		u8*						m_pHostMem;
		u32						m_hostMemByteSize;
		
		uint					m_reportBufferSize;						///< Report Buffer size (can be zero)
		void*					m_pReportBuffer;						///< Report Buffer used and managed by Gc users
		
		Ice::Render::Report*	m_pReports;								///< Used for GcKernel internal reports

		uint					m_contextSize;
		u8*						m_pContextMem;
		
		GcRenderBufferHandle	m_hCommonTiledBuffer;
		GcRenderBufferHandle	m_hBackBuffer;
		GcRenderBufferHandle	m_hFrontBuffer;
		
		uint					m_referenceValue;
		
		u64						m_presentCounter;

		Ice::Render::Region*	m_pAllocatorRegionMem;					///< Ice::Render allocator region memory (if owned by GcKernel)
		
		int						m_hostMemoryRefCount;
		int						m_vramRefCount;
		uint					m_freeHostMemory;
		uint					m_freeVram;
		
		void*					m_pScratchAllocation;
		uint					m_totalScratchSize;
		void*					m_pScratchBegin;
		void*					m_pScratchCurrent;
		void*					m_pScratchEnd;
		
		TimeoutCallbackFunc*		m_pTimeoutCallback;
		FinishCallbackFunc*			m_pFinishCallback;
		AllocFailureCallbackFunc*	m_pAllocFailureCallback;
		
		void*					m_pLastKickedStart;
		void*					m_pLastKickedEnd;

		u32						m_displayWidth;
		u32						m_displayHeight;
		u32						m_displayPitch;

		Gc::AspectRatio			m_displayAspectRatio;
	};
	
	static Instance						ms_instance;			///< Aggregated instance attributes.
	static GcContext					ms_context;				///< Global command context.
	static GcValidation					ms_validation;			///< Global command context validation.


	// GcResource needs to update memory tracking
	friend class GcResource;									
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gc/GcKernel.inl>

//--------------------------------------------------------------------------------------------------

#endif	// GC_KERNEL_H
