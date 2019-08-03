//--------------------------------------------------------------------------------------------------
/**
	@file		
**/
//--------------------------------------------------------------------------------------------------

#ifndef INCL_GPSHADERSHADERDICTIONARY_H
#define INCL_GPSHADERSHADERDICTIONARY_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fw/FwResource.h>
#include <Fw/FwStd/FwHashedString.h>
#include <Fw/FwStd/FwStdIntrusivePtr.h>
#include <Gc/GcShader.h>

//--------------------------------------------------------------------------------------------------
//	FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------

class GpShaderDictionary;
typedef FwStd::IntrusivePtr< GpShaderDictionary > GpShaderDictionaryHandle;

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Represents a shader dictionary resource.
**/
//--------------------------------------------------------------------------------------------------

class GpShaderDictionaryResource
{
public:
	struct Shader
	{
		u32 m_offset;						//!< The offset of the shader resource.
		u32 m_size;							//!< The size of the shader resource.
	};

	struct Entry
	{
		FwHashedString m_hashedName;		//!< The hashed name of the entry.
		u16 m_vertexShaderIndex;			//!< The index in the vertex shader array.
		u16 m_fragmentShaderIndex;			//!< The index in the fragment shader array.
	};

	u32 m_cookie;							//!< The binary version.
	u32 m_vertexShaderCount;				//!< The number of vertex shaders.
	u32 m_fragmentShaderCount;				//!< The number of fragment shaders.
	u32 m_entryCount;						//!< The number of entries.
    u32 m_fragmentProgramTotalSize;			//!< The total size of all fragment program microcode.
	u32 m_vertexShaderOffset;				//!< The offset to the vertex shader array.
	u32 m_fragmentShaderOffset;				//!< The offset to the fragment shader array.
	u32 m_entryOffset;						//!< The offset to the entry array.

	const void* GetOffset( uint offset ) const
	{
		return reinterpret_cast< const u8* >( this ) + offset;
	}

	const Shader* GetVertexShaders() const
	{
		return reinterpret_cast< const Shader* >( GetOffset( m_vertexShaderOffset ) );
	}

	const Shader* GetFragmentShaders() const
	{
		return reinterpret_cast< const Shader* >( GetOffset( m_fragmentShaderOffset ) );
	}

	const Entry* GetEntries() const
	{
		return reinterpret_cast< const Entry* >( GetOffset( m_entryOffset ) );
	}
};

//--------------------------------------------------------------------------------------------------
/**
	@brief		Represents a shader dictionary.

	
**/
//--------------------------------------------------------------------------------------------------

class GpShaderDictionary
{
public:
	// Query Functions
	
	static int QuerySizeInBytes( const void* pResource );
	static int QuerySizeInBytes( const FwResourceHandle& hResource );

	static int QueryResourceSizeInBytes( const FwResourceHandle& hResource );
	static int QueryResourceSizeInBytes( const void* pResource );

	// Creation

	static GpShaderDictionaryHandle Create( const FwResourceHandle& hResource, 
											Gc::BufferType bufferType = Gc::kStaticBuffer,
											Gc::MemoryContext memoryContext = Gc::kVideoMemory,
											void* pClassMemory = NULL );

	static GpShaderDictionaryHandle Create( const void* pResource, 
											size_t resourceSize,
											Gc::BufferType bufferType = Gc::kStaticBuffer,
											Gc::MemoryContext memoryContext = Gc::kVideoMemory,
											void* pClassMemory = NULL );

	// Access
									 
	uint GetSize() const;

	FwHashedString GetHash( uint index ) const;
	GcShaderHandle const& GetVertexShader( uint index ) const;
	GcShaderHandle const& GetFragmentShader( uint index ) const;

private:
	GpShaderDictionary( const void* pResource, 
						size_t resourceSize,
                        Gc::BufferType bufferType,
						Gc::MemoryContext memoryContext, 
						bool ownsMemory );
	~GpShaderDictionary();

	void LoadShaders( Gc::BufferType bufferType );

	struct Entry
	{
		GcShaderHandle m_vertexShader;
		GcShaderHandle m_fragmentShader;
	};

	const GpShaderDictionaryResource* m_pHeader;	//!< The dictionary resource in memory.
	FwResourceHandle m_hResource;					//!< The resource for the memory (optional).
	Entry* m_pEntries;								//!< The dictionary entries.
	uint m_size;									//!< The number of entries.
	void* m_pFragmentProgramMemory;					//!< A block allocation of fragment program code.
	Gc::MemoryContext m_memoryContext;				//!< The memory context of this allocation.

	bool m_ownsMemory;
	u32 m_refCount;

	// Friends

	friend void	IntrusivePtrAddRef( GpShaderDictionary* p )
	{
		++p->m_refCount;
	}
	friend void	IntrusivePtrRelease( GpShaderDictionary* p )
	{
		if( --p->m_refCount == 0 )
		{
			bool ownsMemory = p->m_ownsMemory;
			p->~GpShaderDictionary();
			if( ownsMemory )
				FW_DELETE_ARRAY( reinterpret_cast< u8* >( p ) );
		}
	}
	friend u32 IntrusivePtrGetRefCount( GpShaderDictionary* p )
	{
		return p->m_refCount;
	}
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		
**/
//--------------------------------------------------------------------------------------------------

inline uint GpShaderDictionary::GetSize() const
{
	return m_size;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		
**/
//--------------------------------------------------------------------------------------------------

inline FwHashedString GpShaderDictionary::GetHash( uint index ) const
{
	FW_ASSERT( index < m_size );
	const GpShaderDictionaryResource::Entry* pResourceEntries = m_pHeader->GetEntries();
	return pResourceEntries[index].m_hashedName;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		
**/
//--------------------------------------------------------------------------------------------------

inline GcShaderHandle const& GpShaderDictionary::GetVertexShader( uint index ) const
{
	FW_ASSERT( index < m_size );
	return m_pEntries[index].m_vertexShader;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		
**/
//--------------------------------------------------------------------------------------------------

inline GcShaderHandle const& GpShaderDictionary::GetFragmentShader( uint index ) const
{
	FW_ASSERT( index < m_size );
	return m_pEntries[index].m_fragmentShader;
}

#endif // ndef INCL_GPSHADERSHADERDICTIONARY_H
