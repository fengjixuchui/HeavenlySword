/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKBASE_HK_THREAD_LOCAL_POINTER_H
#define HKBASE_HK_THREAD_LOCAL_POINTER_H

#if (HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED)
  
// For Win32 style, we can use Tls* calls which allow the 
// libs to be used inside dynamically loaded DLLs 
#	if defined(HK_COMPILER_MSVC)

		// change this define to alternate between __declspec and explict TLS
#		define HK_CONFIG_THREAD_USE_TLS

#		if defined(HK_CONFIG_THREAD_USE_TLS)
#			define HK_TLS_OUT_OF_INDEXES 0xffffffff
#			define HK_THREAD_LOCAL(TYPE) hkThreadLocalData< TYPE >
#			define HK_THREAD_LOCAL_IMPL(TYPE) hkThreadLocalData< TYPE >
#			define HK_THREAD_LOCAL_GET(VAR) VAR.getData()
#			define HK_THREAD_LOCAL_SET(VAR, VAL) VAR.setData(VAL)
#			if defined(HK_PLATFORM_WIN32) //WINBASE import define
#				define HK_TLS_IMPORT __declspec(dllimport)
#			else
#				define HK_TLS_IMPORT 
#			endif

			extern "C" {
				HK_TLS_IMPORT unsigned long _stdcall TlsAlloc(void);
				HK_TLS_IMPORT void* _stdcall TlsGetValue( const unsigned long dwTlsIndex );		
				HK_TLS_IMPORT int _stdcall TlsSetValue( const unsigned long dwTlsIndex, void* lpTlsValue);
				HK_TLS_IMPORT int _stdcall TlsFree( const unsigned long dwTlsIndex );
			}

			/// A platform independent wrapper for thread local storage
			/// We assume we always just store a pointer (or data the same size or smaller than a pointer) 
			/// ie,  sizeof(T) <= sizeof(char*)
			template < typename T > 
			class hkThreadLocalData
			{
				public:
					hkThreadLocalData() { m_slotID = (hkUlong)TlsAlloc(); }
					~hkThreadLocalData() { TlsFree((unsigned long)m_slotID); }

					HK_FORCE_INLINE T getData() 
					{
						return (T)(hkUlong) TlsGetValue((unsigned long)m_slotID) ;
					}

					HK_FORCE_INLINE void setData(T p) 
					{
						hkUlong v = (hkUlong)p;
						TlsSetValue((unsigned long)m_slotID, (void*)v );
					}

				protected:
					hkUlong m_slotID; 
			};
#			define HK_THREAD_LOCAL_INIT(TYPE, VAR) VAR.setData( new TYPE )
#			define HK_THREAD_LOCAL_INIT_NULL( VAR) VAR.setData( HK_NULL )
#			define HK_THREAD_LOCAL_QUIT(VAR) { delete VAR.getData(); VAR.setData(HK_NULL); }

#		else
#			define HK_THREAD_LOCAL(TYPE) __declspec(thread) TYPE
#			define HK_THREAD_LOCAL_IMPL(TYPE) TYPE
#		endif
#	elif defined(HK_PLATFORM_PS3) 

		// TLS properly supported in later SDKS, 090 anyway (perhaps was also true 085 etc.)
#	    if 0 // HK_CELL_SDK_VERSION >= 0x090000

#			define HK_THREAD_LOCAL(TYPE) __thread TYPE
#			define HK_THREAD_LOCAL_IMPL(TYPE) TYPE

#       else // Use a limited home-rolled version

#			include <cell/sync.h>			
#			include <sys/ppu_thread.h>	
			
			// On PPU, so really 2 is normal setting, and only searched to end 
			// if not allocated yet, but have a calling thread perhaps, on some 
			// demos should not allow for more 4(ps3) or 6 (for Xbox360 etc), and we may have interupt/signal 
			// handling threads anyway. 
#			define HK_TLS_MAX_THREADS 4 
#			define HK_TLS_NO_INDEX  (hkUlong(-1))

			// There is no TLS support in the 0.4.0 tools, so we have to hard code it in a mapping.

			// This code assumes that the thread ids do NOT CHANGE (can add threads, but threads should
			// not be deleted and new ones created, thus leaving phantom empty entries)
			// Luckily that is how our thread pool works anyway, so ok for internal use.
			void hkThreadLocalDataGetNewEntry(CellSyncMutex* mutex, hkUint64* tidToIndex, const sys_ppu_thread_t tid);

			void hkThreadLocalDataNotInitAssert();

			template < typename T > 
			class hkThreadLocalData
			{
			public:

				hkThreadLocalData() 
				{ 
					// busy wait low level mutex (only used to get new entry anyway)
					// Can't use libsync here as we comonly have TLS as global data, so before the libsync PRX is loaded..
					m_tidToIndexMutex.uint_val = 0xffffffff;

					for(int i=0; i < HK_TLS_MAX_THREADS; ++i)
					{
						m_tidToIndex[i] = HK_TLS_NO_INDEX;
					}
				}

				~hkThreadLocalData() 
				{ 
					// nothing required.
				}


				HK_FORCE_INLINE int findTlsIndex()
				{
					register hkUint64 tid; 
					// r13 == thread id in our setup (set in hkBaseSystem::initThread())
					__asm__ volatile ( "ori %0,13,0" : "=r"( tid ) ); 

					if ( m_tidToIndex[0] == tid ) return 0;
					if ( m_tidToIndex[1] == tid ) return 1;
					if ( m_tidToIndex[2] == tid ) return 2;
#ifdef HK_DEBUG
					if ( m_tidToIndex[3] != tid ) hkThreadLocalDataNotInitAssert();
#endif
					return 3;
				}


				HK_FORCE_INLINE void getStorage()
				{
					register hkUint64 tid; 
					// r13 == thread id in our setup (set in hkBaseSystem::initThread())
					__asm__ volatile ( "ori %0,13,0" : "=r"( tid ) ); 

					if ( m_tidToIndex[0] == tid ) return;
					if ( m_tidToIndex[1] == tid ) return;
					if ( m_tidToIndex[2] == tid ) return;
					if ( m_tidToIndex[3] == tid ) return;

					hkThreadLocalDataGetNewEntry(&m_tidToIndexMutex, &m_tidToIndex[0], tid);
				}

				HK_FORCE_INLINE T& getData() 
				{
					register hkUint64 tid; 
					// r13 == thread id in our setup (set in hkBaseSystem::initThread())
					__asm__ volatile ( "ori %0,13,0" : "=r"( tid ) ); 

					if ( m_tidToIndex[0] == tid ) return m_storage[0];
					if ( m_tidToIndex[1] == tid ) return m_storage[1];
					if ( m_tidToIndex[2] == tid ) return m_storage[2];
#ifdef HK_DEBUG
					if ( m_tidToIndex[3] != tid ) hkThreadLocalDataNotInitAssert();
#endif
					return m_storage[3];
				}

				void freeStorage()
				{
					if ( m_tidToIndexMutex.uint_val == 0xffffffff )
						cellSyncMutexInitialize(&m_tidToIndexMutex);

					cellSyncMutexLock(&m_tidToIndexMutex);
					hkUint64 tid; 
					// r13 == thread id in our setup (set in hkBaseSystem::initThread())
					__asm__ volatile ( "ori %0,13,0" : "=r"( tid ) ); 

					for(int i = 0; i < HK_TLS_MAX_THREADS; ++i)
					{
						if (m_tidToIndex[i] == tid)
						{
							m_tidToIndex[i] = HK_TLS_NO_INDEX;
							//m_storage[i] = (T)HK_NULL;
							break;
						}
					}
					cellSyncMutexUnlock(&m_tidToIndexMutex);
					return;
				}

				HK_FORCE_INLINE void setData(T p) 
				{
					const int tidIndex = findTlsIndex();
					m_storage[tidIndex] = p;
				}
				
			protected:

				// Any more complicate structure requires more processing overhead to access
				CellSyncMutex m_tidToIndexMutex;
				hkUint64 m_tidToIndex[HK_TLS_MAX_THREADS]; 
				T m_storage[HK_TLS_MAX_THREADS];

			};
#			define HK_THREAD_LOCAL(TYPE) hkThreadLocalData< TYPE >
#			define HK_THREAD_LOCAL_IMPL(TYPE) hkThreadLocalData< TYPE >
#			define HK_THREAD_LOCAL_GET(VAR) VAR.getData()
#			define HK_THREAD_LOCAL_SET(VAR, VAL) VAR.setData(VAL)
#			define HK_THREAD_LOCAL_INIT(TYPE, VAR) VAR.getStorage()
#			define HK_THREAD_LOCAL_INIT_NULL( VAR) { VAR.getStorage(); VAR.setData( HK_NULL ); }
#			define HK_THREAD_LOCAL_QUIT(VAR) VAR.freeStorage()

#		endif // __thread

#	elif defined(HK_PLATFORM_PS3SPU) 

#	define HK_THREAD_LOCAL(TYPE) TYPE // XXX This will not be the same size as the above, so never try to DMA up TLS :)
#	define HK_THREAD_LOCAL_IMPL(TYPE) TYPE

#	else
#		error fixme, no TLS defined on this platform yet multithreading is enabled
#	endif 
#else
#	define HK_THREAD_LOCAL(TYPE) TYPE
#	define HK_THREAD_LOCAL_IMPL(TYPE) TYPE
#endif

#ifndef HK_THREAD_LOCAL_GET 
#	define HK_THREAD_LOCAL_GET(VAR) (VAR)
#	define HK_THREAD_LOCAL_SET(VAR, VAL) (VAR)=(VAL)
#endif

#ifndef HK_THREAD_LOCAL_INIT
#	define HK_THREAD_LOCAL_INIT_NULL(A)
#	define HK_THREAD_LOCAL_INIT(TYPE, A)
#	define HK_THREAD_LOCAL_QUIT(A)
#endif

#endif // HKBASE_HK_THREAD_LOCAL_POINTER_H


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
