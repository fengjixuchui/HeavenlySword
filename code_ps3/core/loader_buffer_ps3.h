//----------------------------------------------------------------------------------------
//! 
//! \filename core/loader_buffer_ps3.h
//! 
//----------------------------------------------------------------------------------------
#ifndef LOADER_BUFFER_PS3_H_
#define LOADER_BUFFER_PS3_H_

#include "core/loader_callback_ps3.h"
#include "core/loader_resource_types_ps3.h"
#include "core/syncprims.h"

namespace Loader
{
	// Forward declare a visitor class.
	struct MemBlockVisitor
	{
		template < typename T >
		bool Visit( T );		// Implemented in loader_ps3.cpp.
	};

	// The buffer-memory for the loader.
	class Buffer
	{
		public:
			//
			//	Allocate/Deallocate a block of memory.
			//
			typedef uintptr_t MemBlockID;
			static const MemBlockID InvalidMemBlockID	= 0xffffffff;

			MemBlockID	Allocate		( uint32_t size_in_bytes, ResourceType resource_type, const char *filename, void* userdata,
										  Callback callback = NULL, volatile void *callback_arg = NULL );

		public:
			//
			//	Functions associated with memory-block ids.
			//
						//	Return a pointer to the block associated with this id.
			void *		GetPtr			( MemBlockID mem_id )						const;

						//	Run the callback associated with this memory-block id, if there is one.
			void		RunCallback		( MemBlockID mem_id, CallbackStage stage )	const;

			enum Tags
			{
				NOT_READY			= 0,
				READY_TO_PROCESS	= 1
			};
			void		Tag				( MemBlockID mem_id, Tags tag );

			void		StoreReadSize	( MemBlockID mem_id, uint32_t filesize );

		public:
			//
			//	Visit each memory-block. Although this is public, you can't actually call
			//	it; trust me on this. This is intended behaviour. See the comment above
			//	Loader::Update in loader_ps3.cpp. [ARV].
			//
			void		VisitAll		( MemBlockVisitor visitor );

		public:
			//
			//	Ctor, dtor.
			//
			Buffer	();
			~Buffer	();

		private:
			//
			//	Prevent copying and assignment.
			//
			Buffer( const Buffer & )				/* NOT_IMPLEMENTED */;
			Buffer &operator = ( const Buffer & )	/* NOT_IMPLEMENTED */;

		private:
			//
			//	Array of allocated blocks.
			//
			friend struct MemBlockVisitor;

			mutable CriticalSection	m_Mutex;

			struct MemBlock
			{
				void *			m_Memory;
				uint32_t		m_ReadSize;
				Callback		m_Callback;
				volatile void *	m_CallbackArg;
				Tags			m_Tag;
				const char *	m_Filename;
				void *			m_UserData;
				ResourceType	m_ResourceType;
			};
			typedef ntstd::List< ntstd::pair< MemBlockID, MemBlock > >	BlockMap;
			BlockMap			m_Blocks;

			MemBlock *			FindBlock( MemBlockID id );
			const MemBlock *	FindBlock( MemBlockID id )	const;
	};
}


#endif // !LOADER_BUFFER_PS3_H_

