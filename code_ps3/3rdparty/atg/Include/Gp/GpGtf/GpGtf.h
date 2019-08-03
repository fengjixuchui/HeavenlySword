//--------------------------------------------------------------------------------------------------
/**
	@file

	@brief		GTF Loading Code.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef GPGTF_GPGTF_H
#define GPGTF_GPGTF_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/Gc.h>
#include <Gc/GcTexture.h>
#include <cell/gcm.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		GTF Resource Layout.
**/
//--------------------------------------------------------------------------------------------------

struct GpGtfResource
{
	u32 m_version;			//!< The dds2gtf converter version.
	u32 m_size;				//!< Total size of texture data (excluding header and attributes).
	u32 m_numTextures;		//!< The number of texture attributes.
};

//--------------------------------------------------------------------------------------------------
/**
	@brief		GTF Resource Attribute Layout.
**/
//--------------------------------------------------------------------------------------------------

struct GpGtfAttribute
{
	u32 m_id;				//!< The texture ID. What is this?
	u32 m_offsetToTex;		//!< The offset to the texture data from the beginning of the file.
	u32 m_textureSize;		//!< The size of the texture data.
	CellGcmTexture m_tex;	//!< A memory mapped CellGcmTexture struct.
};

//--------------------------------------------------------------------------------------------------
/**
	@brief		GTF Loader.

	The static LoadTexture functions are shortcuts to load the first texture from a GTF resource 
	that is fully in memory.  All remaining functions are for multi-step loading of GTF resources
	that can be partially in memory and/or contain multiple files.
**/
//--------------------------------------------------------------------------------------------------

class GpGtf
{
public:
	// Simple (single texture only) interface

	static GcTextureHandle	LoadTexture( const FwResourceHandle& hResource,	
										 void* pClassMemory = NULL,
										 Gc::MemoryContext memoryContext = Gc::kVideoMemory );

	static GcTextureHandle	LoadTexture( const void* pResource, 
										 uint resourceSize,
										 void* pClassMemory = NULL,
										 Gc::MemoryContext memoryContext = Gc::kVideoMemory );

	
	// Construction

	GpGtf();

	void					SetResource( const FwResourceHandle& hResource );
	void					SetResource( const void* pResource, uint resourceSize );


	// Multiple texture interface

	uint					GetTextureCount() const;

	GcTextureHandle			LoadTexture( uint index, 
										 void* pClassMemory = NULL,
										 Gc::MemoryContext memoryContext = Gc::kVideoMemory ) const;

	// Multiple texture interface for headers only resources

	GcTextureHandle			CreateEmptyTexture( uint index, 
												Gc::BufferType bufferType = Gc::kStaticBuffer,
												void* pClassMemory = NULL,
												Gc::MemoryContext memoryContext = Gc::kVideoMemory ) const;

	uint					GetTextureDataOffset( uint index ) const;
	uint					GetTextureDataSize( uint index ) const;

private:
	const GpGtfAttribute*	GetAttribute( uint index ) const;
	void					CopyTextureData( uint index, const GcTextureHandle& hTexture ) const;

	FwResourceHandle		m_hResource;		//!< Holds the resource handle to keep it valid.
	const GpGtfResource*	m_pResource;		//!< A pointer to the start of the resource.
	uint					m_resourceSize;		//!< The size of the resource.
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Loads the first texture from a GTF resource.

	This function assumes that the entire GTF resource (including data) has been loaded.
**/
//--------------------------------------------------------------------------------------------------

inline GcTextureHandle GpGtf::LoadTexture( const FwResourceHandle& hResource, 
										   void* pClassMemory, 
										   Gc::MemoryContext memoryContext )
{
	GpGtf gtf;
	gtf.SetResource( hResource );
	FW_ASSERT( gtf.GetTextureCount() > 0 );
	return gtf.LoadTexture( 0, pClassMemory, memoryContext );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Loads the first texture from a GTF resource.

	This function assumes that the entire GTF resource (including data) has been loaded.
**/
//--------------------------------------------------------------------------------------------------

inline GcTextureHandle GpGtf::LoadTexture( const void* pResource, 
										   uint resourceSize,
										   void* pClassMemory, 
										   Gc::MemoryContext memoryContext )
{
	GpGtf gtf;
	gtf.SetResource( pResource, resourceSize );
	FW_ASSERT( gtf.GetTextureCount() > 0 );
	return gtf.LoadTexture( 0, pClassMemory, memoryContext );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		
**/
//--------------------------------------------------------------------------------------------------

inline void GpGtf::SetResource( const FwResourceHandle& hResource )
{
	m_hResource = hResource;
	SetResource( hResource.GetData(), hResource.GetSize() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		
**/
//--------------------------------------------------------------------------------------------------

inline uint GpGtf::GetTextureCount() const
{
	FW_ASSERT( m_pResource );
	return m_pResource->m_numTextures;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Loads the nth texture from a GTF resource.

	This function assumes that the entire GTF resource (including data) has been loaded.
**/
//--------------------------------------------------------------------------------------------------

inline GcTextureHandle GpGtf::LoadTexture( uint index, 
										   void* pClassMemory, 
										   Gc::MemoryContext memoryContext ) const
{
	GcTextureHandle hTexture = CreateEmptyTexture( index, Gc::kStaticBuffer, pClassMemory, memoryContext );
	CopyTextureData( index, hTexture );
	return hTexture;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		
**/
//--------------------------------------------------------------------------------------------------

inline const GpGtfAttribute* GpGtf::GetAttribute( uint index ) const
{
	// check arguments
	FW_ASSERT( m_pResource );
	FW_ASSERT_MSG( index < GetTextureCount(), ( "Index out of bounds" ) );

	// advance to the start of the attribute array
	const GpGtfAttribute* pAttributes = reinterpret_cast< const GpGtfAttribute* >(
		reinterpret_cast< const u8* >( m_pResource ) + sizeof( GpGtfResource )
	);

	// advance to the correct index
	return pAttributes + index;
}

#endif // ndef GPGTF_GPGTF_H
