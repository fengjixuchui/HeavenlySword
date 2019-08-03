//----------------------------------------------------------------------------------------
//! 
//! \filename core/loader_buffer_ps3.cpp
//! 
//----------------------------------------------------------------------------------------

#include "core/loader_buffer_ps3.h"

#include "core/util.h"
#include "core/mem.h"

//----------------------------------------------------------------------------------------
//!
//! Static, private data.
//!
//----------------------------------------------------------------------------------------
namespace Loader
{
	namespace BufferPrivate
	{
		static const Mem::MEMORY_CHUNK	MemoryChunk		= Mem::MC_LOADER;
		static const uint32_t			MemoryAlignment = 16;				// Bytes.
		static const uint32_t			SizeAlignment	= 16;				// Bytes.
	}
}

//----------------------------------------------------------------------------------------
//!
//! Allocate a memory block.
//!
//----------------------------------------------------------------------------------------
Loader::Buffer::MemBlockID Loader::Buffer::Allocate( uint32_t buffer_size_in_bytes, ResourceType resource_type, const char *filename, void *userdata, Callback callback, volatile void *callback_arg )
{
	ScopedCritical sc( m_Mutex );

	if ( buffer_size_in_bytes != 0 )
	{
		buffer_size_in_bytes = ROUND_POW2( buffer_size_in_bytes, BufferPrivate::SizeAlignment );
	}

	MemBlock block;
	block.m_Memory = buffer_size_in_bytes == 0 ? NULL : reinterpret_cast< void * >( NT_MEMALIGN_CHUNK_RET_NULL( BufferPrivate::MemoryChunk, buffer_size_in_bytes, BufferPrivate::MemoryAlignment ) );
	block.m_ReadSize = 0xffffffff;
	block.m_Callback = callback;
	block.m_CallbackArg = callback_arg;
	block.m_Tag = NOT_READY;
	block.m_Filename = filename;
	block.m_UserData = userdata;
	block.m_ResourceType = resource_type;

	if ( block.m_Memory == NULL )
	{
		if ( buffer_size_in_bytes == 0 )
		{
			// If the requested size was zero, we still return the invalid-id to indicate that we didn't
			// allocate any memory. However, we still want to add the block to our list because this
			// may correspond to a valid object to visit later on (e.g. an error-cube or missing resource).
			m_Blocks.push_back( ntstd::pair< MemBlockID, MemBlock >( uintptr_t( block.m_Memory ), block ) );
		}

		return InvalidMemBlockID;
	}

	m_Blocks.push_back( ntstd::pair< MemBlockID, MemBlock >( uintptr_t( block.m_Memory ), block ) );
	return Loader::Buffer::MemBlockID( block.m_Memory );
}

//----------------------------------------------------------------------------------------
//!
//! Get a pointer to the start of a mem-block from the id.
//!
//----------------------------------------------------------------------------------------
void *Loader::Buffer::GetPtr( MemBlockID mem_id ) const
{
	ntError_p( mem_id != InvalidMemBlockID, ("Invalid mem-block id.") );
	return reinterpret_cast< void * >( mem_id );
}

//----------------------------------------------------------------------------------------
//!
//! Run the callback associated with this mem-block id.
//!
//----------------------------------------------------------------------------------------
void Loader::Buffer::RunCallback( MemBlockID mem_id, Loader::CallbackStage stage ) const
{
	ScopedCritical sc( m_Mutex );

	ntError_p( mem_id != InvalidMemBlockID, ("Invalid mem-block id.") );

	const MemBlock *mem_block = FindBlock( mem_id );
	ntError_p( mem_block != NULL, ("Bad error - can't find this mem-block id. This is really very serious - tell Andrew or Wil.") );

	( *mem_block->m_Callback )( stage, mem_block->m_CallbackArg );
}

//----------------------------------------------------------------------------------------
//!
//! Get a pointer to the start of a mem-block from the id.
//!
//----------------------------------------------------------------------------------------
void Loader::Buffer::Tag( MemBlockID mem_id, Tags tag )
{
	ScopedCritical sc( m_Mutex );

	ntError_p( mem_id != InvalidMemBlockID, ("Invalid mem-block id.") );

	MemBlock *mem_block = FindBlock( mem_id );
	ntError_p( mem_block != NULL, ("Bad error - can't find this mem-block id. This is really very serious - tell Andrew or Wil.") );

	mem_block->m_Tag = tag;
}

//----------------------------------------------------------------------------------------
//!
//! Store the read-size with the mem-block from the id.
//!
//----------------------------------------------------------------------------------------
void Loader::Buffer::StoreReadSize( MemBlockID mem_id, uint32_t filesize )
{
	ScopedCritical sc( m_Mutex );

	ntError_p( mem_id != InvalidMemBlockID, ("Invalid mem-block id.") );

	MemBlock *mem_block = FindBlock( mem_id );
	ntError_p( mem_block != NULL, ("Bad error - can't find this mem-block id. This is really very serious - tell Andrew or Wil.") );

	mem_block->m_ReadSize = filesize;
}

//----------------------------------------------------------------------------------------
//!
//! Goes through each memory-block and visits it.
//!
//----------------------------------------------------------------------------------------
void Loader::Buffer::VisitAll( Loader::MemBlockVisitor visitor )
{
	ScopedCritical sc( m_Mutex );

	for ( BlockMap::iterator it = m_Blocks.begin(); it != m_Blocks.end(); )
	{
		if ( visitor.Visit< MemBlock & >( ( *it ).second ) )
		{
			// Delete the memory and remove this block from the map.
			NT_FREE_CHUNK( BufferPrivate::MemoryChunk, reinterpret_cast< uintptr_t >( ( *it ).second.m_Memory ) );
			it = m_Blocks.erase( it );
		}
		else
		{
			++it;
		}
	}
}

//----------------------------------------------------------------------------------------
//!
//! Ctor.
//!
//----------------------------------------------------------------------------------------
Loader::Buffer::Buffer()
{
}

//----------------------------------------------------------------------------------------
//!
//! Dtor.
//!
//----------------------------------------------------------------------------------------
Loader::Buffer::~Buffer()
{
}

Loader::Buffer::MemBlock *Loader::Buffer::FindBlock( Loader::Buffer::MemBlockID id )
{
	for (	BlockMap::iterator it = m_Blocks.begin();
			it != m_Blocks.end();
			++it )
	{
		ntstd::pair< MemBlockID, MemBlock > &block = *it;
		if ( block.first == id )
		{
			return &( block.second );
		}
	}

	return NULL;
}

const Loader::Buffer::MemBlock *Loader::Buffer::FindBlock( Loader::Buffer::MemBlockID id ) const
{
	for (	BlockMap::const_iterator it = m_Blocks.begin();
			it != m_Blocks.end();
			++it )
	{
		const ntstd::pair< MemBlockID, MemBlock > &block = *it;
		if ( block.first == id )
		{
			return &( block.second );
		}
	}

	return NULL;
}





