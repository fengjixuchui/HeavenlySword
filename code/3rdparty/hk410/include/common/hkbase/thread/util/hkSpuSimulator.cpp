/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>

#if defined(HK_SIMULATE_SPU_DMA_ON_CPU)

#include <hkbase/memory/hkLocalBuffer.h>

#include <hkbase/thread/util/hkSpuSimulator.h>
#include <hkbase/thread/hkCriticalSection.h>
#include <hkbase/stream/hkSocket.h>

#include <hkbase/thread/hkSpuUtils.h>
#include <hkbase/thread/hkSpuDmaManager.h>

#include <stdio.h>
#include <hkbase/fwd/hkwindows.h>

hkSpuSimulator::Client* hkSpuSimulator::Client::m_instance = HK_NULL;

static void myPrint(const char*s, void*)
{
	OutputDebugString(s);
	printf( "%s\n", s );
}

#define HK_SPUSIM_ASSERT(id, a, TEXT, request)	\
	{	\
		if ( !(a) )	\
		{	\
			printf("\n\nSPU SIMULATOR ERROR: %s\n\n", TEXT);	\
			OutputDebugString("\n\nSPU SIMULATOR ERROR:\n");	\
			OutputDebugString(TEXT);	\
			OutputDebugString("\n\n");	\
			m_stackTracer.dumpStackTrace(request->m_stackTrace, STACK_DEPTH, myPrint, HK_NULL );	\
			HK_ASSERT2(id, a, TEXT);	\
		}	\
	}



hkSpuSimulator::Server::Server( int port )
{
	m_scheduleTaskLock = new hkCriticalSection();

	m_socket = hkSocket::create();
	HK_ASSERT2( 0xf0435412, m_socket, "Cannot create socket");

	m_socket->listen( port );

	hkString::memSet(m_clientActiveFlags, 0, NUM_ALLOWED_CLIENTS);

}

int hkSpuSimulator::Server::getNumClients()
{
	return m_clients.getSize();
}

void hkSpuSimulator::Server::pollForNewClient()
{
	hkSocket* client = m_socket->pollForNewClient();
	if ( !client )
	{
		return;
	}

	int i;
	for (i = 0; i < NUM_ALLOWED_CLIENTS; ++i)
	{
		if (m_clientActiveFlags[i] == 0)
		{
			m_clientActiveFlags[i] = 1;
			break;
		}
	}
	if (i == NUM_ALLOWED_CLIENTS)
	{
		return;
	}

	ClientInfo* ci = new ClientInfo();

	ci->m_socket = client;
	ci->m_clientId = i;
	ci->m_server = this;
	ci->m_shouldTerminate = 0;

	ci->m_taskSemaphore = &m_taskSemaphore;
	ci->m_scheduleTaskLock = m_scheduleTaskLock;
	ci->m_clientTaskParams = &m_clientTaskParams;

	ci->m_thread.startThread(serve, ci );
	m_clients.pushBack(ci);
}

void hkSpuSimulator::Server::waitForClient()
{
	while ( 1 )
	{
		pollForNewClient();
		if ( getNumClients() )
		{
			break;
		}
		Sleep(10);
	}
}

void hkSpuSimulator::Server::runClientTask( SpuParams& taskParams )
{
	m_scheduleTaskLock->enter();

	m_clientTaskParams.pushBack(taskParams);
	m_taskSemaphore.release();

	HK_ASSERT2( 0xa96afe45, m_clientTaskParams.getSize() <= m_clients.getSize(), "Called runClientTask more times than you have SPU processes running");

	m_scheduleTaskLock->leave();

}



hkSpuSimulator::Server::~Server()
{
	for (int i = 0; i < getNumClients(); i++)
	{
		ClientInfo* ci = m_clients[i];
		TerminateThread(ci->m_thread.getHandle(), 0 );
		ci->m_socket->close();
		delete ci;
	}
	m_socket->close();
	delete m_scheduleTaskLock;
}

void hkSpuSimulator::Server::refreshClientList()
{
	int i = 0;
	while ( i < m_clients.getSize() )
	{
		ClientInfo* ci = m_clients[i];
		if ( ci->m_thread.getStatus() == hkThread::THREAD_TERMINATED )
		{
			m_clientActiveFlags[ci->m_clientId] = 0;
			m_clients.removeAt(i);
		}
		else
		{
			i++;
		}
	}
}

void hkSpuSimulator::Server::terminateClient(int i)
{
	m_clients[i]->m_shouldTerminate = 1;
}


bool hkSpuSimulator::Server::clientHasDied()
{
	for (int i = 0; i < getNumClients(); i++)
	{
		ClientInfo* ci = m_clients[i];
		if ( ci->m_thread.getStatus() == hkThread::THREAD_TERMINATED )
		{
			return true;
		}
	}
	return false;
}

void hkSpuSimulator::read( hkSocket* socket, void* buffer, int size)
{
	void* notReadYet = buffer;
	void* readEnd = hkAddByteOffset(buffer, size);
	while ( notReadYet < readEnd)
	{
		int bytesLeft = int(hkGetByteOffset(notReadYet, readEnd));
		int nbytes = socket->read(notReadYet, bytesLeft);
		if ( nbytes == 0 )
		{
			HK_WARN_ONCE(0xaffe5371, "Abnormal termination: nbytes == 0");
			ExitThread(1);
		}
		notReadYet = hkAddByteOffset( notReadYet, nbytes );
	}
	if ( !socket->isOk() )
	{
		HK_WARN_ONCE(0xaffe5372, "Abnormal termination: !socket->isOk()");
		ExitThread(1);
	}
}

void hkSpuSimulator::write( hkSocket* socket, void* buffer, int size)
{
	void* notWriteYet = buffer;
	void* WriteEnd = hkAddByteOffset(buffer, size);
	while ( notWriteYet < WriteEnd)
	{
		int bytesLeft = int(hkGetByteOffset(notWriteYet, WriteEnd));
		int nbytes = socket->write(notWriteYet, bytesLeft);
		if ( nbytes == 0 )
		{
			HK_WARN_ONCE(0xaffe5373, "Abnormal termination: nbytes == 0");
			ExitThread(1);
		}
		notWriteYet = hkAddByteOffset( notWriteYet, nbytes );
	}
	if ( !socket->isOk() )
	{
		HK_WARN_ONCE(0xaffe5374, "Abnormal termination: !socket->isOk()");
		ExitThread(1);
	}
}

void* HK_CALL hkSpuSimulator::Server::serve( void* param )
{
	ClientInfo* clientInfo = reinterpret_cast<ClientInfo*>(param);

	LargestRequest buffer;

	hkSocket* socket = clientInfo->m_socket;
	while(1)
	{
		read( socket, &buffer, sizeof(LargestRequest) );
		switch(buffer.m_command)
		{
		case COMMAND_GET_ROOT:
			{
				GetRootRequest* r = reinterpret_cast<GetRootRequest*>(&buffer);
				if ( r->m_magicNumber != MAGIC )
				{
					HK_WARN_ONCE(0xaffe5375, "Abnormal termination: r->m_magicNumber != MAGIC");
					ExitThread(1);
				}
				char buf[20];
				hkString::sprintf(buf, "SPU %d", clientInfo->m_clientId);
				write(socket, buf, 20);
				break;
			}

		case COMMAND_GET_MEMORY:
			{
				GetMemRequest* r = reinterpret_cast<GetMemRequest*>(&buffer);
				write( socket, r->m_address, r->m_numBytes);
				break;
			}

		case COMMAND_GET_MEMORY_AND_WIPE:
			{
				GetMemRequest* r = reinterpret_cast<GetMemRequest*>(&buffer);
				write( socket, r->m_address, r->m_numBytes);
				hkString::memSet(r->m_address, 0xce, r->m_numBytes );
				break;
			}

		case COMMAND_CHECK_WIPE_AND_PUT_MEMORY:
			{
				PutMemRequest* r = reinterpret_cast<PutMemRequest*>(&buffer);
				for (unsigned int i=0; i < r->m_numBytes; i++)
				{
					HK_ASSERT2( 0xf0323454, reinterpret_cast<char*>(r->m_address)[i] == 0xce, "Somebody changed the server data, while it was owned by an spu" );
				}
				// no break here
			}
		case COMMAND_PUT_MEMORY:
			{
				PutMemRequest* r = reinterpret_cast<PutMemRequest*>(&buffer);
				read( socket, r->m_address, r->m_numBytes);
				break;
			}


		case COMMAND_ENTER_CRITICAL_SECTION:
			{
				EnterCriticalSectionRequest* r = reinterpret_cast<EnterCriticalSectionRequest*>(&buffer);
				r->m_criticalSection->enter();
				Answer answer;
				write( socket, &answer, sizeof(answer) );
				break;
			}

		case COMMAND_LEAVE_CRITICAL_SECTION:
			{
				LeaveCriticalSectionRequest* r = reinterpret_cast<LeaveCriticalSectionRequest*>(&buffer);
				r->m_criticalSection->leave();
				Answer answer;
				write( socket, &answer, sizeof(answer) );
				break;
			}
		case COMMAND_ACQUIRE_SEMAPHORE:
			{
				AcquireSemaphoreRequest* r = reinterpret_cast<AcquireSemaphoreRequest*>(&buffer);
				DWORD dwWaitResult = WaitForSingleObject( r->m_semaphore, INFINITE );          // zero-second time-out interval
				HK_ASSERT(0xf0324354, dwWaitResult == WAIT_OBJECT_0);
				Answer answer;
				write( socket, &answer, sizeof(answer) );
				break;
			}
		case COMMAND_RELEASE_SEMAPHORE:
			{
				ReleaseSemaphoreRequest* r = reinterpret_cast<ReleaseSemaphoreRequest*>(&buffer);
				ReleaseSemaphore( r->m_semaphore, r->m_count, NULL);
				Answer answer;
				write( socket, &answer, sizeof(answer) );
				break;
			}
		case COMMAND_SHOULD_TERMINATE_REQUEST:
			{
				Answer answer;
				answer.m_value = clientInfo->m_shouldTerminate;
				write( socket, &answer, sizeof(answer) );
				break;
			}
		case COMMAND_SHOULD_RUN_TASK:
			{
				clientInfo->m_taskSemaphore->acquire();

				clientInfo->m_scheduleTaskLock->enter();

				write( socket, &(*clientInfo->m_clientTaskParams)[0], 4 * sizeof(void*));
				clientInfo->m_clientTaskParams->removeAtAndCopy(0);

				clientInfo->m_scheduleTaskLock->leave();

				break;
			}
		default:
			{
				HK_WARN_ONCE(0xaffe5376, "Abnormal termination: Unknown command received from client.");
				ExitThread(1);
			}
		}
	}
}

hkSpuSimulator::Client::Client( const char* serverName, int port )
{
	m_socket = hkSocket::create();
	m_lockCount = 0;

	while (1)
	{
		hkResult res = m_socket->connect( serverName, port );
		if ( res == HK_SUCCESS)
		{
			LargestRequest buffer;
			GetRootRequest* r = reinterpret_cast<GetRootRequest*>(&buffer);
			r->m_command = COMMAND_GET_ROOT;
			r->m_magicNumber = MAGIC;
			write(m_socket, &buffer, sizeof(buffer));
			read(m_socket, m_name, 20);
			break;
		}
		Sleep(10);
	}
	m_instance = this;
}

hkSpuSimulator::Client::~Client()
{
	delete m_socket;
	m_instance = HK_NULL;
}

void hkSpuSimulator::Client::getFromMainMemoryInternal(void* dstOnSpu, const void* srcOnPpu, int size)
{
	LargestRequest buffer;
	GetMemRequest* r = reinterpret_cast<GetMemRequest*>(&buffer);
	r->m_command = COMMAND_GET_MEMORY;
	r->m_address = const_cast<void*>(srcOnPpu);
	r->m_numBytes = size;
	write(m_socket, &buffer, sizeof(buffer));
	read(m_socket, dstOnSpu, size);
}

void hkSpuSimulator::Client::getFromMainMemoryInternalWipeServer(void* dstOnSpu, const void* srcOnPpu, int size)
{
	LargestRequest buffer;
	GetMemRequest* r = reinterpret_cast<GetMemRequest*>(&buffer);
	r->m_command = COMMAND_GET_MEMORY_AND_WIPE;
	r->m_address = const_cast<void*>(srcOnPpu);
	r->m_numBytes = size;
	write(m_socket, &buffer, sizeof(buffer));
	read(m_socket, dstOnSpu, size);
}

void hkSpuSimulator::Client::putToMainMemoryInternal(void* dstOnPpu, const void* srcOnSpu, int size)
{
	LargestRequest buffer;
	PutMemRequest* r = reinterpret_cast<PutMemRequest*>(&buffer);
	r->m_command = COMMAND_PUT_MEMORY;
	r->m_address = dstOnPpu;
	r->m_numBytes = size;
	write(m_socket, &buffer, sizeof(buffer));
	write(m_socket, const_cast<void*>(srcOnSpu), size);
}

void hkSpuSimulator::Client::checkServerAndPutToMainMemoryInternal(void* dstOnPpu, const void* srcOnSpu, int size)
{
	LargestRequest buffer;
	PutMemRequest* r = reinterpret_cast<PutMemRequest*>(&buffer);
	r->m_command = COMMAND_CHECK_WIPE_AND_PUT_MEMORY;
	r->m_address = dstOnPpu;
	r->m_numBytes = size;
	write(m_socket, &buffer, sizeof(buffer));
	write(m_socket, const_cast<void*>(srcOnSpu), size);
}

hkSpuSimulator::Client::DmaRequest* hkSpuSimulator::Client::findDmaRequest( const void* ppu, const void* spu, int size )
{
	DmaRequest* found = HK_NULL;
	int modSize = (size<1)? 1: size;
	for (int i=0; i < m_dmaRequests.getSize();i++)
	{
		DmaRequest* r = &m_dmaRequests[i];

		//
		// if a ppu block was supplied, check if the current dma request matches for both supplied ppu and spu blocks
		//
		if ( ppu )
		{
			//
			// verify that the supplied ppu and spu blocks at least partially overlap with the dma request's original ppu and spu blocks
			//
			if ( ppu < hkAddByteOffsetConst(r->m_memoryOnPpu, r->m_memorySize) && hkAddByteOffsetConst(ppu,modSize) > r->m_memoryOnPpu )
			{
				if ( spu && spu < hkAddByteOffsetConst(r->m_memoryOnSpu, r->m_memorySize) && hkAddByteOffsetConst(spu,modSize) > r->m_memoryOnSpu )
				{
					found = r; break;
				}
			}

			//
			// issue an error if the spu partially overlaps but the ppu didn't
			//
			if ( spu && spu < hkAddByteOffsetConst(r->m_memoryOnSpu, r->m_memorySize) && hkAddByteOffsetConst(spu,modSize) > r->m_memoryOnSpu )
			{
				HK_SPUSIM_ASSERT( 0xf0343234, false, "Mismatched dma-request found, did you miss a performFinalChecks() call?", r);
			}

		}

		//
		// if no ppu block was supplied, just check if the current dma request matches for the supplied spu block
		//
		if ( spu && spu < hkAddByteOffsetConst(r->m_memoryOnSpu, r->m_memorySize) && hkAddByteOffsetConst(spu,modSize) > r->m_memoryOnSpu )
		{
			found = r; break;
		}
	}

	if ( found )
	{
		if ( !size )
		{
			size = found->m_memorySize;
		}

		// check consistency
		if ( ppu )
		{
			HK_SPUSIM_ASSERT( 0xf0343240, ppu                            >= found->m_memoryOnPpu,                                            "Mismatched dma-request found, did you miss a performFinalChecks() call?", found);
			HK_SPUSIM_ASSERT( 0xf0343241, hkAddByteOffsetConst(ppu,size) <= hkAddByteOffsetConst(found->m_memoryOnPpu, found->m_memorySize), "Mismatched dma-request found, did you miss a performFinalChecks() call?", found);
		}
		if ( spu && ppu )
		{
			int offsetPpu = hkGetByteOffsetInt( found->m_memoryOnPpu, ppu);
			int offsetSpu = hkGetByteOffsetInt( found->m_memoryOnSpu, spu);
			HK_SPUSIM_ASSERT( 0xf0343242, offsetPpu == offsetSpu, "Mismatched dma-request found, did you miss a performFinalChecks() call?", found);
		}
	}

	return found;
}

void hkSpuSimulator::Client::wipeSpu(DmaRequest* request)
{
	hkString::memSet( request->m_memoryOnSpu, 0xce, request->m_memorySize  );
}

void hkSpuSimulator::Client::copyBufferToSpu(DmaRequest* request)
{
	hkString::memCpy(request->m_memoryOnSpu, request->m_buffer, request->m_memorySize);
}

void hkSpuSimulator::Client::copyBufferToPpu(hkSpuSimulator::Client::DmaRequest* request)
{
	putToMainMemoryInternal(request->m_memoryOnPpu, request->m_buffer, request->m_memorySize);
}

void hkSpuSimulator::Client::checkForSpuWipe(DmaRequest* request, int assertId, const char* errorMsg)
{
	const hkUchar* spu = (hkUchar*)request->m_memoryOnSpu;
	int size           = request->m_memorySize;

	for (int i=0; i<size; i++)
	{
		HK_SPUSIM_ASSERT(assertId, spu[i] == 0xce, errorMsg, request);
	}
}

void hkSpuSimulator::Client::checkForPpuWipe(DmaRequest* request, int assertId, const char* errorMsg)
{
	int size = request->m_memorySize;

	hkInplaceArray<char,16000> buffer; buffer.setSize(size);
	getFromMainMemoryInternal(buffer.begin(), request->m_memoryOnPpu, size);

	//
	// build a fake dma-request where we camouflage our temporary copy of the ppu memory as 'regular' spu memory so that
	// we can use our checkForSpuWipe() utility function ;)
	//
	DmaRequest fakeRequest;
	{
		fakeRequest.m_memoryOnSpu = buffer.begin();
		fakeRequest.m_memorySize  = size;
	}

	checkForSpuWipe(&fakeRequest, assertId, errorMsg);
}

void hkSpuSimulator::Client::checkSpuEqualsBuffer(DmaRequest* request, int assertId, const char* errorMsg)
{
	const char* spu    = (const char*)request->m_memoryOnSpu;
	const char* buffer = (const char*)request->m_buffer;
	int size           = request->m_memorySize;

	for (int i=0; i<size; i++)
	{
		HK_SPUSIM_ASSERT(assertId, spu[i] == buffer[i], errorMsg, request);
	}
}

void hkSpuSimulator::Client::checkPpuEqualsBuffer(DmaRequest* request, int assertId, const char* errorMsg)
{
	const char* ppu    = (const char*)request->m_memoryOnPpu;
	const char* buffer = (const char*)request->m_buffer;
	int size           = request->m_memorySize;

	hkInplaceArray<char,16000> tmpBuffer; tmpBuffer.setSize(size);
	getFromMainMemoryInternal(tmpBuffer.begin(), ppu, size);

	for (int i =0 ; i < size; i++)
	{
		HK_SPUSIM_ASSERT(assertId, tmpBuffer[i] == buffer[i], errorMsg, request);
	}
}

hkSpuSimulator::Client::DmaRequest* hkSpuSimulator::Client::createNewRequest( const void* ppu, const void* spu, int size, int dmaGroup )
{
	//
	// check if there are more then 10 requests with the exact same dma-group and memory size; if so, most likely a performFinalChecks()
	// is missing somewhere
	//
	{
		int numMatches = 0;
		for (int i =0; i < m_dmaRequests.getSize();i++)
		{
			DmaRequest* r = & m_dmaRequests[i];
			if ( r->m_dmaGroup == dmaGroup && r->m_memorySize == size)
			{
				numMatches++;
			}
		}
		HK_ASSERT2( 0xf0f45465, numMatches < 10, "There is a missing performFinalChecks()");
	}

	//
	// create new dma-request
	//
	DmaRequest* request;
	{
		request									= m_dmaRequests.expandBy(1);
		request->m_dmaGroup						= dmaGroup;
		request->m_memoryOnSpu					= const_cast<void*>(spu);
		request->m_memoryOnPpu					= const_cast<void*>(ppu);
		request->m_buffer						= hkAllocateChunk<char>(size, HK_MEMORY_CLASS_BASE);
		request->m_memorySize					= size;
		request->m_deferFinalChecksUntilWait	= false;

		m_stackTracer.getStackTrace(&request->m_stackTrace[0], STACK_DEPTH);
	}

	return request;
}

void hkSpuSimulator::Client::getFromMainMemory(void* dstOnSpu, const void* srcOnPpu, int size, int mode, int dmaGroup)
{
	DmaRequest* request = findDmaRequest( srcOnPpu, dstOnSpu, size );
	HK_SPUSIM_ASSERT( 0xf031d565, !request, "Old request found, did you miss a performFinalChecks() call?", request);

	request = createNewRequest( srcOnPpu, dstOnSpu, size, dmaGroup );

	wipeSpu(request);

	switch (mode)
	{
		case hkSpuDmaManager::READ_ONLY: 
			request->m_stage = STAGE_READONLY_GET;
			getFromMainMemoryInternal( request->m_buffer, srcOnPpu, size );
			break;

		case hkSpuDmaManager::READ_COPY: 
			request->m_stage = STAGE_READCOPY_GET;
			getFromMainMemoryInternal( request->m_buffer, srcOnPpu, size );
			break;

		case hkSpuDmaManager::READ_WRITE: 
			request->m_stage = STAGE_READWRITE_GET;
			getFromMainMemoryInternalWipeServer( request->m_buffer, srcOnPpu, size );
			break;
	}
}

void hkSpuSimulator::Client::putToMainMemory(void* dstOnPpu, const void* srcOnSpu, int size, int mode, int dmaGroup)
{
	DmaRequest* request = findDmaRequest( dstOnPpu, srcOnSpu, size );

	if ( mode == hkSpuDmaManager::WRITE_NEW )
	{
		HK_SPUSIM_ASSERT( 0xf031d566, !request, "WRITE_NEW: Old request found. Did you miss a performFinalChecks() call?", request);

		request = createNewRequest( dstOnPpu, srcOnSpu, size, dmaGroup );
		request->m_stage = STAGE_WRITENEW_PUT;

		// the getFromMainMemory() part here is actually superfluous, we only need it to initiate a wipe on the server; whatever data we 'get' here
		// is immediately overwritten through the memCpy() below
		getFromMainMemoryInternalWipeServer( request->m_buffer, dstOnPpu, size );

		hkString::memCpy(request->m_buffer, srcOnSpu, size);

		return;
	}

	if ( mode == hkSpuDmaManager::WRITE_BACK || mode == hkSpuDmaManager::WRITE_BACK_SUBSET )
	{
		HK_SPUSIM_ASSERT( 0xf031d568, request, "READ_WRITE: Missing getFromMainMemory() request.", request);
		HK_SPUSIM_ASSERT( 0xf031d569, request->m_stage == STAGE_READWRITE_GETWAIT || request->m_stage == STAGE_READWRITE_PARTIALPUT, "READ_WRITE: Missing waitForDmaCompletion() request.", request);
		if ( mode == hkSpuDmaManager::WRITE_BACK )
		{
			HK_SPUSIM_ASSERT( 0xf0315439, request->m_memoryOnPpu == dstOnPpu, "READ_WRITE: A put has to match the corresponding read, use WRITE_BACK_SUBSET otherwise", request);
			HK_SPUSIM_ASSERT( 0xf0315439, request->m_memorySize  == size,     "READ_WRITE: A put has to match the corresponding read, use WRITE_BACK_SUBSET otherwise", request);
		}

		// update request's stack trace so that it points to the last 'dma write' command
		m_stackTracer.getStackTrace(&request->m_stackTrace[0], STACK_DEPTH);

		int offset = hkGetByteOffsetInt(request->m_memoryOnPpu, dstOnPpu);
		hkString::memCpy(hkAddByteOffset(request->m_buffer, offset), srcOnSpu, size);
		request->m_stage = (mode == hkSpuDmaManager::WRITE_BACK) ? STAGE_READWRITE_PUT : STAGE_READWRITE_PARTIALPUT;
		request->m_dmaGroup = dmaGroup;

		return;
	}

	HK_SPUSIM_ASSERT( 0xf031d567, false, "Unknown mode", request);
}

void hkSpuSimulator::Client::waitDmaGroup( int bits )
{

		// backwards loop because elements can be deleted while iterating
	for ( int i=m_dmaRequests.getSize()-1; i>=0; i-- )
	{
		DmaRequest* request = &m_dmaRequests[i];
		if ( request->m_dmaGroup >= 0 && ((1 << request->m_dmaGroup) & bits) )
		{

			switch ( request->m_stage )
			{
				case STAGE_READONLY_GET:
				{
					checkForSpuWipe(request, 0xaf635ef3, "READ_ONLY: Read only memory on spu was overwritten.");
					copyBufferToSpu(request);

					request->m_stage = STAGE_READONLY_WAIT;
					break;
				}

				case STAGE_READCOPY_GET: 
				{
					checkForSpuWipe(request, 0xaf635ef1, "READ_COPY: Spu memory was overwritten before getFromMainMemory() was finished.");
					copyBufferToSpu(request);

					request->m_stage = STAGE_READCOPY_WAIT;
					break;
				}

				case STAGE_READWRITE_GET:
				{
					checkForSpuWipe(request, 0xaf6354f3, "READ_WRITE: Spu memory was overwritten before getFromMainMemory() was finished.");
					copyBufferToSpu(request);

					request->m_stage = STAGE_READWRITE_GETWAIT;
					break;
				}

				case STAGE_READWRITE_PARTIALPUT:
				case STAGE_READWRITE_PUT:
				{
					checkSpuEqualsBuffer(request, 0xaf53ee42, "READ_WRITE: Source data on spu changed while waiting for dma to finish.");

					request->m_stage = STAGE_READWRITE_PUTWAIT;
					break;
				}

				case STAGE_WRITENEW_PUT: 
				{
					checkForPpuWipe     (request, 0xaf23ee46, "WRITE_NEW: Destination memory on ppu was changed before dma wait was called.");
					checkSpuEqualsBuffer(request, 0xaf53ee46, "WRITE_NEW: Source data on spu changed while waiting for dma to finish.");
					copyBufferToPpu     (request);
					request->m_stage = STAGE_WRITENEW_PUTWAIT;
					break;
				}

				default:
				{
					HK_SPUSIM_ASSERT(0xaf364785, false, "Unknown type or unexpected WAIT type encountered.", request);
				}

			}

			request->m_dmaGroup = -1;	// dma handled 
			if ( request->m_deferFinalChecksUntilWait )
			{
				performFinalChecks( request->m_memoryOnPpu, request->m_memoryOnSpu, request->m_memorySize );
			}
		}
	}
}

void hkSpuSimulator::Client::performFinalChecks( const void* ppu, const void* spu, int size )
{
	DmaRequest* request = findDmaRequest( ppu, spu, size );

	HK_ASSERT2      ( 0xf031d56e, request,                                "Missing dma get & wait call.");
	HK_SPUSIM_ASSERT( 0xf032d66e, !ppu || request->m_memoryOnPpu == ppu,  "You tried to perform a partial final check on a bigger block", request);
	HK_SPUSIM_ASSERT( 0xf032d76e, !spu || request->m_memoryOnSpu == spu,  "You tried to perform a partial final check on a bigger block", request);
	HK_SPUSIM_ASSERT( 0xf033d86e, !size|| request->m_memorySize  == size, "You tried to perform a partial final check on a bigger block", request);
	HK_SPUSIM_ASSERT( 0xf031d96e, request->m_dmaGroup            == -1,   "Missing wait call."                                          , request);


	switch ( request->m_stage )
	{
		case STAGE_READONLY_WAIT:
		{
			checkPpuEqualsBuffer(request, 0xafee2311, "READ_ONLY: Read only memory on ppu was overwritten.");
			checkSpuEqualsBuffer(request, 0xaf53ee43, "READ_ONLY: Read only memory on spu was overwritten.");
			wipeSpu				(request);
			break;
		}

		case STAGE_READCOPY_WAIT:
		{
			break;
		}

		case STAGE_READWRITE_GETWAIT:
		case STAGE_READWRITE_PUTWAIT:
		{
			checkForPpuWipe(request, 0xafee2315, "READ_WRITE: Locked memory on ppu was changed.");
			copyBufferToPpu(request);
			break;
		}

		case STAGE_WRITENEW_PUTWAIT:
		{
			break;
		}

		default:
		{
			HK_SPUSIM_ASSERT(0xaf36478f, false, "Missing wait call", request);
		}

	}

	hkDeallocateChunk( (char*)request->m_buffer, request->m_memorySize, HK_MEMORY_CLASS_BASE);
	int index = int(request - m_dmaRequests.begin());
	m_dmaRequests.removeAt(index);

}

void hkSpuSimulator::Client::tryToPerformFinalChecks( const void* dstOnPpu, const void* srcOnSpu, int size )
{
	DmaRequest* request = findDmaRequest( dstOnPpu, srcOnSpu, size );
	if ( request )
	{
		performFinalChecks(dstOnPpu, srcOnSpu, size );
	}
}

void hkSpuSimulator::Client::deferFinalChecksUntilWait( const void* dstOnPpu, const void* srcOnSpu, int size )
{
	DmaRequest* request = findDmaRequest( dstOnPpu, srcOnSpu, size );
	HK_SPUSIM_ASSERT( 0xf032e445, request, "Cannot find request", request);
	request->m_deferFinalChecksUntilWait = true;
}

void hkSpuSimulator::Client::convertReadOnlyToReadWrite(void* ppu, const void* spu, int size)
{
	DmaRequest* request = findDmaRequest( ppu, spu, size );

	HK_SPUSIM_ASSERT( 0xaf313561, request, "Missing getFromMainMemory request", request);

	// it's ok if we are already in READ_WRITE mode; immediately return as there's nothing to do
	if ( request->m_stage == STAGE_READWRITE_GETWAIT )
	{
		return;
	}

	HK_SPUSIM_ASSERT( 0xaf313562, request->m_stage == STAGE_READONLY_WAIT, "You can only convert an original READ_ONLY DMA to READ_WRITE!", request);

	// convert stage to READ_WRITE
	request->m_stage = STAGE_READWRITE_GETWAIT;

	// we need to temporarily backup our original data in the buffer as the call to getFromMainMemoryInternalWipeServer() below
	// will overwrite our buffer with the original data from ppu. we will restore our current data afterwards
	{
		int size = request->m_memorySize;

		void* backupBuffer = hkAllocateChunk<char>(size, HK_MEMORY_CLASS_BASE);
		hkString::memCpy(backupBuffer, request->m_buffer, size);

		getFromMainMemoryInternalWipeServer( request->m_buffer, ppu, size );

		hkString::memCpy(request->m_buffer, backupBuffer, size);
		hkDeallocateChunk( (char*)backupBuffer, size, HK_MEMORY_CLASS_BASE);
	}
}

void hkSpuSimulator::Client::performExitChecks()
{
	if ( m_dmaRequests.getSize() > 0 )
	{
		OutputDebugString("\n\n****************************************\nThere are still unfinished dma requests pending. Dumping stack traces now:\n");

		{
			for (int i=0; i<m_dmaRequests.getSize(); i++)
			{
				DmaRequest* r = &m_dmaRequests[i];
				m_stackTracer.dumpStackTrace(r->m_stackTrace, STACK_DEPTH, myPrint, HK_NULL );
				OutputDebugString("\n\n");
			}
		}

		HK_ASSERT2(0xaf7351fe, m_dmaRequests.getSize() == 0, "There are still unfinished dma requests pending. Stack traces dumped.");
	}
}

bool hkSpuSimulator::Client::checkShouldTerminate()
{
	LargestRequest buffer;
	buffer.m_command = COMMAND_SHOULD_TERMINATE_REQUEST;
	write(m_socket, &buffer, sizeof(buffer));
	Answer answer;
	read(m_socket, &answer, sizeof(Answer));
	if (answer.m_value == 0 )
	{
		return false;
	}
	else
	{
		return true;
	}
}


void hkSpuSimulator::Client::enterCriticalSection( hkCriticalSection* criticalSection )
{
	LargestRequest buffer;
	EnterCriticalSectionRequest* r = reinterpret_cast<EnterCriticalSectionRequest*>(&buffer);
	r->m_command = COMMAND_ENTER_CRITICAL_SECTION;
	r->m_criticalSection = criticalSection->m_this;
	write(m_socket, &buffer, sizeof(buffer));

	Answer answer;
	read(m_socket, &answer, sizeof(Answer));

}


void hkSpuSimulator::Client::leaveCriticalSection( hkCriticalSection* criticalSection )
{
	LargestRequest buffer;
	LeaveCriticalSectionRequest* r = reinterpret_cast<LeaveCriticalSectionRequest*>(&buffer);
	r->m_command = COMMAND_LEAVE_CRITICAL_SECTION;
	r->m_criticalSection = criticalSection->m_this;
	write(m_socket, &buffer, sizeof(buffer));

	Answer answer;
	read(m_socket, &answer, sizeof(Answer));
}



void hkSpuSimulator::Client::acquireSemaphore( void* semaphore )
{
	LargestRequest buffer;
	AcquireSemaphoreRequest* r = reinterpret_cast<AcquireSemaphoreRequest*>(&buffer);
	r->m_command = COMMAND_ACQUIRE_SEMAPHORE;
	r->m_semaphore = semaphore;
	write(m_socket, &buffer, sizeof(buffer));

	Answer answer;
	read(m_socket, &answer, sizeof(Answer));
	return;
}


void hkSpuSimulator::Client::releaseSemaphore( void* semaphore, int count )
{
	LargestRequest buffer;
	ReleaseSemaphoreRequest* r = reinterpret_cast<ReleaseSemaphoreRequest*>(&buffer);
	r->m_command = COMMAND_RELEASE_SEMAPHORE;
	r->m_semaphore = semaphore;
	r->m_count = count;
	write(m_socket, &buffer, sizeof(buffer));

	Answer answer;
	read(m_socket, &answer, sizeof(Answer));
	return;
}

bool hkSpuSimulator::Client::checkShouldRunTask( SpuParams& params )
{
	LargestRequest buffer;
	buffer.m_command = COMMAND_SHOULD_RUN_TASK;
	write(m_socket, &buffer, sizeof(buffer));
	read(m_socket, &params, sizeof(SpuParams));
	return true;

}


#endif // HK_SIMULATE_SPU_DMA_ON_CPU

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
