//----------------------------------------------------------------------------------------
//! 
//! \filename core_ps3/loader_ps3.cpp
//! 
//----------------------------------------------------------------------------------------

#include <sys/ppu_thread.h>

#include "core/loader_ps3.h"
#include "core/loader_buffer_ps3.h"

#include "core/threadsafequeue.h"

#include "core/file.h"
#include "core/gfxmem.h"

#include "exec/exec.h"

//----------------------------------------------------------------------------------------
//!
//! Private loader data.
//!
//----------------------------------------------------------------------------------------
namespace
{
	typedef ThreadSafeQueue< Loader::Request, Loader::MaxNumRequests >	RequestQueue;

	static bool							Initialised				= false;

	static RequestQueue					Requests;
	static volatile int32_t 			LoaderThreadRunning;

	static const int32_t				LoaderThreadPriority	= 500;
	static const char *					LoaderThreadName		= "Loader Thread";

	static Loader::Buffer *				LoaderBuffer			= NULL;

	static Loader::ResourceHandler		ResourceHandlers[ Loader::LRT_NUM_RESOURCE_TYPES ];
	static Loader::PreprocessMessage	PreprocessMessageHandlers[ Loader::LRT_NUM_RESOURCE_TYPES ];

	static WaitableEvent *				LoaderThreadEvent;

	static Loader::RequestList *		UserRequestList;
}

namespace Loader
{
	// Internal AddRequest function.
	void								AddRequestInternal( const Loader::Request &req );
}

//----------------------------------------------------------------------------------------
//!
//!	The Update function - responsible for firing off calls to the resource handler
//! functions so we can fixup resources and transfer to video-ram. Runs on the
//! main-thread.
//!
//----------------------------------------------------------------------------------------
namespace Loader
{
	//
	//	This is a fairly dirty way to ensure no-one from outside this file can actually
	//	call Loader::Buffer::VisitAll.
	//	It works because VisitAll takes a MemBlockVisitor object as an argument but
	//	calls the Visit template function within it. External callers don't know what
	//	the template argument for Visit should be and even if they do, they can't
	//	implement the function because the correct type (Loader::Buffer::MemBlock &)
	//	is private to the Loader::Buffer class (MemBlockVisitor is a friend).
	//	The only other way to use the VisitAll function is to let external callers
	//	have access to the specialisation to this type - we prevent this by defining
	//	the specialisation here, in the .cpp file, where it can't be accessed via
	//	external linkage.
	//	It's worth noting that I could've done this far more nicely with some friend
	//	classes, but I also wanted all the Loader code to be in the Loader namespace
	//	and GCC seems to have issues with scoping friends in different namespaces - 
	//	I couldn't get it to compile unfortunately. This works just as well, but is
	//	more difficult to understand. [ARV].
	//
	template <>
	bool MemBlockVisitor::Visit< Loader::Buffer::MemBlock & >( Loader::Buffer::MemBlock &block )
	{
		if ( block.m_Tag == Loader::Buffer::READY_TO_PROCESS || block.m_Memory == NULL )
		{
			ntError_p( block.m_ResourceType < LRT_NUM_RESOURCE_TYPES, ("Invalid resource type.") );

			if ( ResourceHandlers[ block.m_ResourceType ] != NULL )
			{
				// Do the processing.
				( *ResourceHandlers[ block.m_ResourceType ] )( block.m_Filename, block.m_UserData, block.m_Memory, block.m_ReadSize );
			}

			// If the callback is present then call it here.
			if ( block.m_Callback != NULL )
			{
				( *block.m_Callback )( LCS_SYNCPOINT, block.m_CallbackArg );
			}

			// Return true to indicate we've finished with this block and should deallocate it.
			return true;
		}

		return false;
	}
}

// The update function just visits each MemBlock in the LoaderBuffer with
// the above Visit specialisation.
void Loader::Update()
{
	if ( UserRequestList != NULL )
	{
		// If we have queue space free then move requests over to the actual multi-threaded queue.
		while ( !Requests.IsFull() && !UserRequestList->empty() )
		{
			// We have at least one user-request and also space on the multi-threaded queue.
			Loader::Request req = UserRequestList->front();
			UserRequestList->pop_front();

			// Notify user that we've taken this request.
			ntError( req.m_Resource >= 0 && req.m_Resource < LRT_NUM_RESOURCE_TYPES );
			if ( PreprocessMessageHandlers[ req.m_Resource ] != NULL )
			{
				( *PreprocessMessageHandlers[ req.m_Resource ] )( req );
			}

			AddRequestInternal( req );
		}

		LoaderThreadEvent->Wake();
	}

	// Visit all the mem-blocks and call handlers + callbacks on any that have finished.
	MemBlockVisitor visitor;
	LoaderBuffer->VisitAll( visitor );
}

//----------------------------------------------------------------------------------------
//!
//! The function that actually does the work of loading a file into some memory.
//!
//----------------------------------------------------------------------------------------
static void LoadData( const char *filename, void *userdata, Loader::ResourceType resource_type, Loader::Callback callback, volatile void *callback_arg )
{
	Loader::Buffer::MemBlockID mem_block_id;

	if	(
		(filename == 0) ||
		(File::Exists(filename) == false)
		)
	{
		// we're just using this block as a callback method, dont bother with any file IO at all.
		mem_block_id = LoaderBuffer->Allocate( 0, resource_type, filename, userdata, callback, callback_arg );
		ntError( mem_block_id == Loader::Buffer::InvalidMemBlockID );
		return;
	}

	// Open the file and allocate a temp' memory pool for this resource.
	File file( filename, File::FT_BINARY | File::FT_READ );

	uint32_t file_size = 0;
	ntError( file.IsValid() );
	file_size = file.GetFileSize();

	ntError_p( file_size <= Mem::LOADER_CHUNK_SIZE, ("Resource %s exceeds memory restrictions on async loader (%.2fMb)", filename, _R( Mem::LOADER_CHUNK_SIZE )/(1024.f*1024.f)));

	//
	//	We loop, calling Allocate, until we get a valid id.
	//	This is okay to do because at some point the main thread will
	//	finish processing the already allocated resource pools and will
	//	deallocate them - at this point we should be able to Allocate.
	//	An InvalidMemBlockID just means that there are no pools currently
	//	available.
	//
	mem_block_id = LoaderBuffer->Allocate( file_size, resource_type, filename, userdata, callback, callback_arg );

	while ( mem_block_id == Loader::Buffer::InvalidMemBlockID )
	{
		LoaderThreadEvent->Wait();
		mem_block_id = LoaderBuffer->Allocate( file_size, resource_type, filename, userdata, callback, callback_arg );
	}

	// Now we have a valid mem-block id we can load the data into it.
	file.Read( LoaderBuffer->GetPtr( mem_block_id ), file_size );

	// Store any info or parameters we need for fixing-up with the pool buffer here.
	LoaderBuffer->StoreReadSize( mem_block_id, file_size );

	// Now we have the data loaded we can push tag our pool buffer as requiring
	// uploading/fixing up/whatever from the main-thread.
	//
	// THIS MUST BE THE LAST LINE IN THIS FUNCTION!
	LoaderBuffer->Tag( mem_block_id, Loader::Buffer::READY_TO_PROCESS );

	// We can't do anything else here as we're on the loader-thread and any uploading
	// to video-ram and fixing-up needs to be done on the main thread.
}

//----------------------------------------------------------------------------------------
//!
//! The loader thread function itself.
//!
//----------------------------------------------------------------------------------------
static void LoaderThreadFunc( uint64_t )
{
	// Initially set up to run.
	AtomicSet( &LoaderThreadRunning, 1 );

	// All processing happens in this while loop.
	// When Loader::Destroy() is called, this while loop will exit because
	// LoaderThreadRunning will be set to 0.
	while( LoaderThreadRunning != 0 )
	{
		if ( !Requests.IsEmpty() )
		{
			Loader::Request request = Requests.PopFront();
			LoadData( request.m_Filename, request.m_UserData, request.m_Resource, request.m_Callback, request.m_CallbackArg );
		}
		else
		{
			LoaderThreadEvent->Wait();
		}
	}

	sys_ppu_thread_exit( 0 );
}

//----------------------------------------------------------------------------------------
//!
//! Initialise the threaded file loading.
//!
//----------------------------------------------------------------------------------------
void Loader::Initialise()
{
	ntError_p( !Initialised, ("You've already initialised the Loader.") );

	for ( uint32_t i=0;i<Loader::LRT_NUM_RESOURCE_TYPES;i++ )
	{
		ResourceHandlers[ i ] = NULL;
		PreprocessMessageHandlers[ i ] = NULL;
	}

	LoaderBuffer = NT_NEW Loader::Buffer();

	// Check the request queue is good to go.
	ntError_p( Requests.IsEmpty(), ("You are initialising the loader but it already has requests in the queue.") );

	// Create the thread synchronisation event.
	LoaderThreadEvent = NT_NEW WaitableEvent();

	// Set the user-side request list to NULL.
	UserRequestList = NULL;

	// Create the loader thread.
	Exec::CreatePPUThread( &LoaderThreadFunc, 0, LoaderThreadPriority, LoaderThreadName );

	Initialised = true;
}

//----------------------------------------------------------------------------------------
//!
//! Destroy the threaded file loading.
//!
//----------------------------------------------------------------------------------------
void Loader::Destroy()
{
	ntError_p( Initialised, ("You haven't called Initialise() yet.") );

	Initialised = false;

	// Check we've finished all our requests before destroying the thread.
	ntError_p( Requests.IsEmpty(), ("You are destroying the loader when requests are still outstanding.") );

	// Tell the thread to exit.
	AtomicSet( &LoaderThreadRunning, 0 );
	LoaderThreadEvent->Wake();				// Must wake otherwise the thread may never be given time to exit!
	while ( LoaderThreadRunning )
	{
		// Wait until the thread exits.
	}

	NT_DELETE( LoaderThreadEvent );
	LoaderThreadEvent = NULL;

	NT_DELETE( LoaderBuffer );
	LoaderBuffer = NULL;
}

//----------------------------------------------------------------------------------------
//!
//! Adds a load request to the queue.
//!
//----------------------------------------------------------------------------------------
void Loader::AddRequestInternal( const Loader::Request &req )
{
	ntError_p( Initialised, ("You haven't called Initialise() yet.") );
	ntError_p( !Requests.IsFull(), ("Badness. Request queue is full - this should never happen") );

	Requests.PushBack( req );

	if ( req.m_Callback != NULL )
	{
		LoaderThreadEvent->Wake();
	}
}

//----------------------------------------------------------------------------------------
//!
//! Adds a load request to the queue.
//!
//----------------------------------------------------------------------------------------
void Loader::AddUserRequestList( RequestList *request_list )
{
	ntError_p( request_list != NULL, ("Cannot pass a NULL request queue.") );
	UserRequestList = request_list;
}

//----------------------------------------------------------------------------------------
//!
//! Registers a resource handler with a resource type.
//!
//----------------------------------------------------------------------------------------
void Loader::RegisterResourceHandler( Loader::ResourceType resource, Loader::ResourceHandler handler )
{
	ntError_p( Initialised, ("You haven't called Initialise() yet.") );
	ntError_p( resource < Loader::LRT_NUM_RESOURCE_TYPES, ("Invalid resource type.") );
	ntError_p( handler != NULL, ("You must specify a valid handler.") );
	ntError_p( ResourceHandlers[ resource ] == NULL, ("You've already specified this handler.") );	// Must want to be able to change handlers in future.

	ResourceHandlers[ resource ] = handler;
}

//----------------------------------------------------------------------------------------
//!
//! Registers a preprocess message handler with a resource type.
//!
//----------------------------------------------------------------------------------------
void Loader::RegisterPreProcessMessage( Loader::ResourceType resource, Loader::PreprocessMessage message )
{
	ntError_p( Initialised, ("You haven't called Initialise() yet.") );
	ntError_p( resource < Loader::LRT_NUM_RESOURCE_TYPES, ("Invalid resource type.") );
	ntError_p( message != NULL, ("You must specify a valid message.") );
	ntError_p( PreprocessMessageHandlers[ resource ] == NULL, ("You've already specified this message.") );	// Must want to be able to change handlers in future.

	PreprocessMessageHandlers[ resource ] = message;
}








