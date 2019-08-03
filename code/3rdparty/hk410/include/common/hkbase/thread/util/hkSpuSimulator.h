/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_SPU_SIMULATOR_H
#define HK_UTILITIES2_SPU_SIMULATOR_H

#include <hkbase/thread/hkSemaphore.h>
#include <hkbase/thread/hkThread.h>
#include <hkbase/memory/hkStackTracer.h>

class hkCriticalSection;
class hkSocket;



namespace hkSpuSimulator
{

	struct SpuParams
	{
		void* m_param0;
		void* m_param1;
		void* m_param2;
		void* m_param3;
	};

		/// The server simulating the main PPU memory
	class Server 
	{
		public:
				/// start the server with the given parameters for all clients
			Server( int port );

				/// returns the number of clients
			int getNumClients();

				/// checks whether a new client connected
			void pollForNewClient();

				/// waits until there is at least one client
			void waitForClient();

			void runClientTask( SpuParams& taskParams );

			void refreshClientList();

			bool clientHasDied();

			void terminateClient(int i);

			~Server();


		public:
			static void* HK_CALL serve( void* param );

			hkSocket* m_socket;

			struct ClientInfo
			{
				Server*  m_server;
				hkThread m_thread;
				hkSocket* m_socket;
				int	m_shouldTerminate;
				int m_clientId;

				hkSemaphore* m_taskSemaphore;
				hkCriticalSection* m_scheduleTaskLock;
				hkArray<SpuParams>* m_clientTaskParams;

			};

			enum 
			{
				NUM_ALLOWED_CLIENTS = 6
			};

			hkArray<ClientInfo*> m_clients;
			char m_clientActiveFlags[NUM_ALLOWED_CLIENTS];

			hkArray<SpuParams> m_clientTaskParams;
			hkSemaphore m_taskSemaphore;
			hkCriticalSection* m_scheduleTaskLock;
	};

		/// The client simulating one SPU
	class Client
	{
		public:
				/// Creates a client which will try to connect to the server immediately
				/// If there is no server yet, this function call will stall until one becomes available
			Client( const char* serverName, int port );

			~Client();

			// Start fetching data from main memory. 
			void getFromMainMemory(void* dstOnSpu, const void* srcOnPpu, int size, int /*hkSpuDmaManager::READ_MODE*/ mode, int dmaGroup);

			void putToMainMemory(void* dstOnPpu, const void* srcOnSpu, int size, int /*hkSpuDmaManager::WRITE_MODE*/ mode, int dmaGroup);

			void waitDmaGroup( int bits );

			void performFinalChecks			( const void* dstOnPpu, const void* srcOnSpu, int size );
			void tryToPerformFinalChecks	( const void* dstOnPpu, const void* srcOnSpu, int size );
			void deferFinalChecksUntilWait	( const void* dstOnPpu, const void* srcOnSpu, int size );

			bool checkShouldTerminate();

				/// enter a critical section, Note that the pointer can either point to a critical section in spu or ppu memory, both works
			void enterCriticalSection( hkCriticalSection* criticalSection );

				/// leave a critical section, Note that the pointer can either point to a critical section in spu or ppu memory, both works
			void leaveCriticalSection( hkCriticalSection* criticalSection );

			void acquireSemaphore( void* semaphore );

			void releaseSemaphore( void* semaphore, int count );

			static Client* getInstance() { return m_instance; }

				
				/// Spurs task emulation
			bool checkShouldRunTask( SpuParams& params );


			void getFromMainMemoryInternal(void* dstOnSpu, const void* srcOnPpu, int size);
			void getFromMainMemoryInternalWipeServer(void* dstOnSpu, const void* srcOnPpu, int size);

			void putToMainMemoryInternal(void* dstOnPpu, const void* srcOnSpu, int size);
			void checkServerAndPutToMainMemoryInternal(void* dstOnPpu, const void* srcOnSpu, int size);

			void convertReadOnlyToReadWrite(void* ppu, const void* spu, int size);

			void performExitChecks();

		public:
			static Client* m_instance;
			hkSocket* m_socket;
			char m_name[20];

				// read only
				// =========
				//   getFromMainMemory()		spu := wipe
				//								buffer := ppu
				//
				//   waitForCompletion()		check stage == STAGE_READONLY_GET
				//								check spu == wipe
				//								spu := buffer
				//
				//   performFinalChecks()		check stage == STAGE_READONLY_WAIT
				//								check ppu == buffer
				//								check spu == buffer
				//								spu := wipe

				// read copy
				// =========
				//   getFromMainMemory()		spu := wipe
				//								buffer := ppu
				//
				//   waitForCompletion()		check stage == STAGE_READCOPY_GET
				//								check spu == wipe
				//								spu := buffer
				//
				//   performFinalChecks()		check stage == STAGE_READCOPY_WAIT

				// read write
				// ==========
				//   getFromMainMemory()		buffer := ppu
				//								ppu := wipe
				//								spu := wipe
				//
				//	 waitForCompletion()		check stage == STAGE_READWRITE_GET
				//								check spu == wipe
				//								spu := buffer
				//
				// {
				//   partial putToMainMemory()	check stage == STAGE_READWRITE_GETWAIT | STAGE_READWRITE_PARTIALPUT
				//								part of buffer := part of spu
				// }*
				//
				//   putToMainMemory()			check stage == STAGE_READWRITE_GETWAIT | STAGE_READWRITE_PARTIALPUT
				//								buffer := spu
				//
				//   waitForCompletion()		check stage == STAGE_READWRITE_PUT | STAGE_READWRITE_PARTIALPUT
				//								check spu == buffer
				//
				//   performFinalChecks()		check stage == STAGE_READWRITE_GETWAIT | STAGE_READWRITE_PUTWAIT
				//								check ppu == wipe
				//								ppu := buffer

				// write new
				// =========
				//   putToMainMemory()			ppu := wipe
				//								buffer := spu
				//
				//   waitForCompletion()		check stage == STAGE_WRITENEW_PUT
				//								check ppu == wipe
				//								check spu == buffer
				//								ppu := buffer
				//
				//   performFinalChecks()		check stage == STAGE_WRITENEW_WAIT

			enum Stage
			{
				STAGE_READONLY_GET,
				STAGE_READONLY_WAIT,

				STAGE_READCOPY_GET, 
				STAGE_READCOPY_WAIT, 

				STAGE_READWRITE_GET,
				STAGE_READWRITE_GETWAIT,
				STAGE_READWRITE_PARTIALPUT,
				STAGE_READWRITE_PUT,
				STAGE_READWRITE_PUTWAIT,

				STAGE_WRITENEW_PUT, 
				STAGE_WRITENEW_PUTWAIT, 
			};

			enum
			{
				STACK_DEPTH = 10
			};

			struct DmaRequest
			{
				Stage	m_stage;
				int		m_dmaGroup;
				void*	m_memoryOnSpu;
				void*	m_memoryOnPpu;
				void*	m_buffer;
				int		m_memorySize;
				hkBool	m_deferFinalChecksUntilWait;
				hkUlong	m_stackTrace[STACK_DEPTH];
			};

		protected:

			hkArray<DmaRequest> m_dmaRequests;
			hkStackTracer m_stackTracer;
			int m_lockCount;

		protected:

			DmaRequest* findDmaRequest  (const void* dstOnSpu, const void* srcOnPpu, int size);
			DmaRequest* createNewRequest(const void* ppu,      const void* spu,      int size, int dmaGroup);

			void wipeSpu				(DmaRequest* request);
			void copyBufferToSpu		(DmaRequest* request);
			void copyBufferToPpu		(DmaRequest* request);
			void checkForSpuWipe		(DmaRequest* request, int assertId, const char* errorMsg);
			void checkForPpuWipe		(DmaRequest* request, int assertId, const char* errorMsg);
			void checkSpuEqualsBuffer	(DmaRequest* request, int assertId, const char* errorMsg);
			void checkPpuEqualsBuffer	(DmaRequest* request, int assertId, const char* errorMsg);
	};

	enum Command
	{
		COMMAND_GET_ROOT,
		COMMAND_GET_MEMORY,
		COMMAND_GET_MEMORY_AND_WIPE,
		COMMAND_PUT_MEMORY,
		COMMAND_CHECK_WIPE_AND_PUT_MEMORY,
		COMMAND_ENTER_CRITICAL_SECTION,
		COMMAND_LEAVE_CRITICAL_SECTION,
		COMMAND_INTERLOCKED_EXCHANGE_ADD,
		COMMAND_SHOULD_RUN_TASK,
		COMMAND_ACQUIRE_SEMAPHORE,
		COMMAND_RELEASE_SEMAPHORE,
		COMMAND_SHOULD_TERMINATE_REQUEST
	} ;

	enum { MAGIC = 0x10324356 };

	struct Request 
	{
		hkEnum<Command,hkUint32> m_command;
	};

	struct GetRootRequest: public Request
	{
		hkUint32 m_magicNumber;
	};

	struct GetMemRequest: public Request
	{
		void* m_address;
		hkUint32 m_numBytes;
	};

	struct PutMemRequest: public Request
	{
		void* m_address;
		hkUint32 m_numBytes;
	};

	struct EnterCriticalSectionRequest: public Request
	{
		hkCriticalSection* m_criticalSection;
	};

	struct Answer
	{
		hkUlong m_value;
	};

	struct LeaveCriticalSectionRequest: public Request
	{
		hkCriticalSection* m_criticalSection;
	};

	struct InterlockedExchangeAddRequest: public Request
	{
		hkUlong* m_var;
		int		 m_addValue;
	};
	struct AcquireSemaphoreRequest : public Request
	{
		void* m_semaphore;
	};

	struct ReleaseSemaphoreRequest: public Request
	{
		void* m_semaphore;
		int	m_count;
	};

	struct LargestRequest: public Request
	{
		hkUint32 m_padding[31];
	};

	// tries to read n bytes and terminates the current thread if connection is lost
	void read( hkSocket* socket, void* buffer, int size);

	// tries to write n bytes and terminates the current thread if connection is lost
	void write( hkSocket* socket, void* buffer, int size);
}




#endif // HK_UTILITIES2_SPU_SIMULATOR_H

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
