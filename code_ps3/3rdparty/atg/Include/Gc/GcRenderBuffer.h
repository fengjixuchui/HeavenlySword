//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Graphics Core Render Buffer		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_RENDER_BUFFER_H
#define GC_RENDER_BUFFER_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/Gc.h>
#include <Gc/GcResource.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class

	@brief		Render buffer object.
**/
//--------------------------------------------------------------------------------------------------

class GcRenderBuffer : public GcResource
{
public:

	// Query Functions

	static int	QuerySizeInBytes();

	static int	QueryResourceSizeInBytes(	uint 					width,
							   				uint					height,
							   				Gc::BufferFormat		format,
							   				Gc::MultisampleMode		multisample = Gc::kMultisampleNone,
											Gc::BufferType			bufferType = Gc::kStaticBuffer);

	
	// Creation
	
	static	GcRenderBufferHandle	Create(	uint width, 
											uint height, 
											Gc::BufferFormat format, 
											Gc::MultisampleMode multisample = Gc::kMultisampleNone,
											Gc::BufferType bufferType = Gc::kStaticBuffer,
											void* pClassMemory = NULL,
											Gc::MemoryContext memoryContext = Gc::kVideoMemory );

	static	GcRenderBufferHandle	Create( const GcTextureHandle& hTexture, 
											Gc::MultisampleMode multisample = Gc::kMultisampleNone,
											void* pClassMemory = NULL );

	// Accessors
	
	uint				GetWidth( void ) const;
	uint				GetHeight( void ) const;
	Gc::BufferFormat	GetFormat( void ) const;
	Gc::MultisampleMode	GetMultisampleMode( void ) const;
	uint				GetPitch( void ) const;
	bool				IsSwizzled( void ) const;

	void				SetPitch( uint pitch );

	// Render Buffer Data Interface

	bool				QueryGetNewScratchMemory() const;
	void				GetNewScratchMemory();
	void				SetDataAddress( void* pUserAddress );

private:
	
	// Construction is hidden. Users use GcRenderBuffer::Create() instead
	GcRenderBuffer( uint width, 
					uint height, 
					Gc::BufferFormat format, 
					Gc::MultisampleMode multisample, 
					uint pitch,
					Gc::BufferType bufferType, 
					bool ownsClassMemory );

	// Internal create method
	static GcRenderBufferHandle		Create( uint width, 
											uint height, 
											Gc::BufferFormat format, 
											Gc::MultisampleMode multisample, 
											uint pitch,
											Gc::BufferType bufferType, 
											void* pClassMemory );

	// Attributes
	uint					m_width;			///< Width.
	uint					m_height;			///< Height.

	Gc::BufferFormat		m_format;			///< Buffer format.
	Gc::MultisampleMode		m_multisample;		///< Multisample mode.

	uint					m_pitch;			///< The pitch a row of pixels (or 0 if swizzled).
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gc/GcRenderBuffer.inl>

//--------------------------------------------------------------------------------------------------

#endif // GC_RENDER_BUFFER_H
