//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Graphics Core Render Buffer		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_RENDER_BUFFER_INL_H
#define GC_RENDER_BUFFER_INL_H

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**

	@brief		Gets the width.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcRenderBuffer::GetWidth( void ) const
{
	return m_width;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the height.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcRenderBuffer::GetHeight( void ) const
{
	return m_height;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the buffer format.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::BufferFormat GcRenderBuffer::GetFormat( void ) const
{
	return m_format;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the multi-sample mode of the render buffer.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::MultisampleMode	GcRenderBuffer::GetMultisampleMode( void ) const
{
	return m_multisample;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the render buffer pitch.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcRenderBuffer::GetPitch( void ) const
{
	return m_pitch;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if the render buffer is swizzled.
**/
//--------------------------------------------------------------------------------------------------

inline bool	GcRenderBuffer::IsSwizzled( void ) const
{
	return ( m_pitch == 0 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the pitch for the current data address.

	Only call this if you know what you're doing. If creating a render buffer statically or from
	a texture then the pitch will not need manually adjusting.
**/
//--------------------------------------------------------------------------------------------------

inline void	GcRenderBuffer::SetPitch( uint pitch )
{
	// store the pitch
	m_pitch = pitch;

	// reset the user size if necessary
	if( GetBufferType() == Gc::kUserBuffer )
	{
		if( pitch == 0 ) 
			pitch = FwAlign( m_width*GetBytesPerPixel( m_format ), 64 );
		SetUserSize( pitch*m_height );
	}
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Makes the render buffer reallocate itself from scratch memory.

	This object will only be valid for the current frame.
**/
//--------------------------------------------------------------------------------------------------

inline void	GcRenderBuffer::GetNewScratchMemory()
{
	// get the allocation pitch
	uint pitch = ( m_pitch != 0 ) ? m_pitch : FwAlign( m_width*GetBytesPerPixel( m_format ), 64 );

	// allocate the memory
	GcResource::AllocateScratchMemory( pitch*m_height, Gc::kRenderBufferAlignment );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the user address for this objects data.
**/
//--------------------------------------------------------------------------------------------------

inline void GcRenderBuffer::SetDataAddress( void* pUserAddress )
{
	GcResource::SetUserAddress( pUserAddress );
}

#endif // GC_RENDER_BUFFER_INL_H
