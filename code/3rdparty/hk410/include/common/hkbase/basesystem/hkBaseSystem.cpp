/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/memory/hkMemory.h>
#include <hkbase/config/hkConfigVersion.h>
#include <hkbase/basesystem/hkBaseSystem.h>
#include <hkbase/monitor/hkMonitorStream.h>

#include <hkbase/error/hkDefaultError.h>
#include <hkbase/stream/hkDefaultStreambufFactory.h>
#include <hkbase/stream/hkSocket.h>
#include <hkbase/thread/util/hkMultiThreadLock.h>

#if defined(HK_PLATFORM_PS3)
#	include <sys/ppu_thread.h>
#endif

#ifdef HK_DEBUG
#	include <hkbase/fwd/hkcstdio.h>
	using namespace std;
#endif

hkBool hkBaseSystemIsInitialized;
#if defined(HK_DEBUG)
	hkBool hkBaseSystemInitVerbose = true;
#else
	hkBool hkBaseSystemInitVerbose; /* = false; */
#endif

// We create a dummy singleton so that even if no other singleton
// is used, the singleton list should not be empty.  Therefore if
// the singleton list is empty, it means that the static constructors
// which register the singletons were not called. This could be
// caused by not calling mwInit() or possibly an incorrect linker file.
class hkDummySingleton : public hkSingleton<hkDummySingleton>
{
	public:

		hkDummySingleton() {}

		virtual void forceLinkage() { }
};

static void HK_CALL showHavokBuild()
{
#if defined(HK_DEBUG)
	if( hkBaseSystemInitVerbose == true )
	{
		printf("---------------------------\n");
		printf(" Havok - Build (%d)\n",HAVOK_BUILD_NUMBER);
		printf(" Base system initialized.   \n");
		printf("----------------------------\n");
	}
#endif
}


void hkBaseSystem::initSingletons()
{
	// ptrToCur is a handle to the current node.
	hkSingletonInitNode** ptrToCur = &hkSingletonInitList;
	hkSingletonInitNode* cur = *ptrToCur;

	hkArray<hkSingletonInitNode*> again;

#	if defined(HK_DEBUG) && !defined( HK_PLATFORM_PSP )
		if (cur == HK_NULL )
		{
			// *****************************************************************
			// The singletons list is empty - the static variable constructors have not been called.
			// If you are using Metrowerks, check that you have called mwInit()
			// prior to calling hkBaseSystem::init()
			// *****************************************************************

			printf("\n*****************************************************************\n");
			printf("Havok Error:\n");
			printf("The singletons list is empty - the static variable constructors have\n");
			printf("not been called. Check that the global constructor chain has been called.\n");
			printf("Hint: if you are using Metrowerks, check that you have called mwInit()\n");
			printf("prior to calling hkBaseSystem::init().\n");
			printf("*****************************************************************\n");


			HK_BREAKPOINT();
		}
#	endif

	while(cur)
	{
		if( *cur->m_value == HK_NULL )
		{
			void* p = (*cur->m_func)();
			if(p)
			{
				// constructed ok
				*cur->m_value = p;
				ptrToCur = &cur->m_next;
				cur = cur->m_next;
			}
			else
			{
				// If a singleton creation function returns HK_NULL it means
				// that the object was not ready to be constructed (perhaps
				// it depends on another singleton?)
				// Remove it from the list and save it for later
				again.pushBack(cur);
				cur = cur->m_next;
				(*ptrToCur)->m_next = HK_NULL;
				*ptrToCur = cur;
			}
		}
		else // skip already done nodes
		{
			ptrToCur = &cur->m_next;
			cur = cur->m_next;
		}
	}

	// Go through the deferred list.
	while( again.getSize() )
	{
		int origSize = again.getSize();
		for(int i = origSize-1; i>=0; --i)
		{
			cur = again[i];
			HK_ASSERT(0x491ec52a, cur->m_next == HK_NULL);

			void* p = (*cur->m_func)();
			if(p)
			{
				// succeeded, put it back on the global list.
				*cur->m_value = p;
				*ptrToCur = cur;
				ptrToCur = &cur->m_next;
				again.removeAt(i);
			}
		}
		HK_ASSERT2(0x14db3060,  again.getSize() < origSize, "cycle detected in singleton construction");
	}
	// array of singleton nodes is now sorted by dependency.
}


void HK_CALL hkBaseSystem::quitSingletons()
{
	// destroy singletons in reverse order to creation.
	hkInplaceArray<hkSingletonInitNode*,128> nodes;
	hkSingletonInitNode* cur = hkSingletonInitList;
	while(cur)
	{
		nodes.pushBack(cur);
		cur = cur->m_next;
	}

	for(int i = nodes.getSize()-1; i >= 0; --i)
	{
		hkReferencedObject* obj = static_cast<hkReferencedObject*>(*nodes[i]->m_value);
		obj->removeReference();
		*nodes[i]->m_value = HK_NULL;
	}
}

hkResult hkBaseSystem::initThread( hkThreadMemory* threadMemory )
{
	hkThreadMemory::init();
	hkThreadMemory::replaceInstance( threadMemory );
	hkMonitorStream::init();
	return HK_SUCCESS;
}

//  Initialize Havok's subsystems.
hkResult HK_CALL hkBaseSystem::init(hkMemory* memoryManager, hkThreadMemory* threadMemory, hkErrorReportFunction errorReportFunction, void* errorReportObject)
{

	if(hkBaseSystemIsInitialized==false)
	{	
		// Init this main threads monitor stream
		hkMonitorStream::init();

#if HK_CONFIG_MEMORY_CLASS == HK_CONFIG_MEMORY_CLASS_ENABLED
		if (memoryManager)
		{
			hkMemory::replaceInstance( memoryManager );

			// Make a default per thread mem manager (for this init calling thread)
			if ( !threadMemory )
			{
				threadMemory = new hkThreadMemory(memoryManager);
				initThread( threadMemory );
				threadMemory->removeReference();
			}
			else
			{
				initThread( threadMemory );
			}
			// Init our calling thread data (will use the TLS slots as previously allocated)
		}
		else
		{
			HK_BREAKPOINT();
			
			// No memory manager system specified.
			// You must specify a memory manager when initializing hkBaseSystem. 
			// Use hkPoolMemory if no memory manager is present or no custom memory
			// manager is available.

			return HK_FAILURE;
		}
#endif

		hkStreambufFactory::replaceInstance( new hkDefaultStreambufFactory );
		hkError::replaceInstance( new hkDefaultError(errorReportFunction, errorReportObject) );

		initSingletons();
		hkDummySingleton::getInstance().forceLinkage();

		hkBaseSystemIsInitialized = true;
		showHavokBuild();

		hkMultiThreadLock::staticInit();
	}

	return HK_SUCCESS;
}


hkResult hkBaseSystem::clearThreadResources(  )
{
	hkMonitorStream::getInstance().quit();
	hkThreadMemory::getInstance().releaseCachedMemory();
	hkThreadMemory::replaceInstance( HK_NULL );
	hkThreadMemory::quit();
	return HK_SUCCESS;
}


//  Quit the subsystems. It is safe to call multiple times.
hkResult HK_CALL hkBaseSystem::quit()
{
	if(hkBaseSystemIsInitialized==true)
	{
		hkMultiThreadLock::staticQuit();

		quitSingletons();

		// Be nice to the network and shut it down gracefully (if we used it and init'd it)
		if ( hkSocket::s_platformNetInitialized && hkSocket::s_platformNetQuit )
		{
			hkSocket::s_platformNetQuit();
			hkSocket::s_platformNetInitialized = false;
		}

		// reverse order to init.
		hkError::replaceInstance( HK_NULL );
		hkStreambufFactory::replaceInstance( HK_NULL );

		clearThreadResources();
#if HK_CONFIG_MEMORY_CLASS == HK_CONFIG_MEMORY_CLASS_ENABLED
		hkMemory::replaceInstance( HK_NULL );
#endif

		hkBaseSystemIsInitialized = false;
	}
	return HK_SUCCESS;
}

hkBool HK_CALL hkBaseSystem::isInitialized()
{
	return hkBaseSystemIsInitialized;
}

#if defined(HK_COMPILER_MWERKS)
#	pragma force_active on
#endif

HK_SINGLETON_IMPLEMENTATION(hkDummySingleton);


/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
