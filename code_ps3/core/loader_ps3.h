//----------------------------------------------------------------------------------------
//! 
//! \filename core_ps3/loader_ps3.h
//! 
//----------------------------------------------------------------------------------------
#ifndef LOADER_PS3_H_
#define LOADER_PS3_H_

#include "core/threadsafequeue.h"
#include "core/loader_callback_ps3.h"
#include "core/loader_resource_types_ps3.h"

namespace Loader
{
	struct Request;

	//
	//	Function interface.
	//
	void Initialise	();
	void Destroy	();

	typedef ntstd::List< Loader::Request > RequestList;
	void AddUserRequestList( RequestList *request_list );

	//
	//	Register a callback per-resource type to handle fixing up that resource.
	//
	typedef void (*ResourceHandler)( const char *filename, void* userdata, void *file_in_memory, uint32_t file_size );
	void RegisterResourceHandler( ResourceType resource, ResourceHandler handler );

	//
	//	Register a callback per-resource type to notify on removal from user queue.
	//
	typedef void (*PreprocessMessage)( const Request &req );
	void RegisterPreProcessMessage( ResourceType resource, PreprocessMessage message );

	//
	//	Update the loader - runs on the main thread and goes through the pool-buffers
	//	looking for anything that can be fixed up and uploaded to video-ram.
	//
	void Update		();

	//
	//	Maximum number of outstanding requests.
	//
	static const uint32_t MaxNumRequests = 2;

	// IMPORTANT: Always create a Request with a ctor call.
	struct Request
	{
		//
		//	Ctors.
		//
				 // Create a data-loading request only.
		explicit Request	( const char *filename, void *userdata, ResourceType resource_type )
		:	m_Filename		( filename )
		,	m_UserData		( userdata )
		,	m_Resource		( resource_type )
		,	m_Callback		( NULL )
		,	m_CallbackArg	( NULL )
		{}

				 // Create a data-loading request with a callback on completion of processing.
		explicit Request	( const char *filename, void *userdata, ResourceType resource_type, Callback callback, volatile void *arg = NULL )
		:	m_Filename		( filename )
		,	m_UserData		( userdata )
		,	m_Resource		( resource_type )
		,	m_Callback		( callback )
		,	m_CallbackArg	( arg )
		{}

		//
		//	Data - never play with these.
		//
		const char *	m_Filename;
		void *			m_UserData;
		ResourceType	m_Resource;
		Callback		m_Callback;
		volatile void *	m_CallbackArg;

		private:
			friend class ThreadSafeQueue< Loader::Request, MaxNumRequests >;
			Request()
			:	m_Filename		( "" )
			,	m_UserData		( NULL )
			,	m_Resource		( LRT_INVALID )
			,	m_Callback		( NULL )
			,	m_CallbackArg	( NULL )
			{}
	};
}

#endif // !LOADER_PS3_H_

