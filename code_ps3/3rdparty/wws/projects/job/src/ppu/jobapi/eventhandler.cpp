/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Install an event handler to pick up events form the SPUs
				This handles JobPrintf, spu_printf, immediate mode audits, and JobApiSendEvent
**/
//--------------------------------------------------------------------------------------------------

#include <sys/event.h>
#include <sys/ppu_thread.h>
#include <cell/spurs/lv2_event_queue.h>
#include <spu_printf.h>
#ifndef __SNC__
#include <ppu_intrinsics.h>
#endif

#include <jobsystem/helpers.h>
#include <jobapi/eventhandler.h>
#include <jobapi/auditmanager.h>

//--------------------------------------------------------------------------------------------------

EventHandler::EventHandler( CellSpurs* spurs, int ppuThreadPriority, AuditManager* pAuditManager, UserEventCallback func, U32 queueDepth )
:	m_userEventPortNum( 0xFF ),
	m_pSpurs( NULL ),
	m_pAuditManager( NULL ),
	m_pUserEventCallback( NULL ),
	m_pPrintfCallback( PrintfSpuNum )
{
	InstallEventHandler( spurs, ppuThreadPriority, pAuditManager, func, queueDepth );
}

//--------------------------------------------------------------------------------------------------

void EventHandler::InstallEventHandler( CellSpurs* spurs, int ppuThreadPriority, AuditManager* pAuditManager, UserEventCallback func, U32 queueDepth )
{
	WWSJOB_ASSERT( m_pSpurs == NULL );	//Only  initialize once
	WWSJOB_ASSERT( queueDepth <= 127 );

	const U32 kStackSize = 1024 * 64;

	m_pUserEventCallback	= func;	//Store the user callback function pointer for later usage when we get an event

	m_pAuditManager			= pAuditManager;

	int	ret;
	WWSJOB_UNUSED( ret );

	//It's not actually a bug to have a queue shorter than 6, but I'd be
	//very suspicious of it.  That's not even 1 queue entry per SPU.
	WWSJOB_ASSERT( queueDepth > 6 );  

	//Create our event queue
	sys_event_queue_attribute_t	attr;
	sys_event_queue_attribute_initialize( attr );
	ret = sys_event_queue_create( &m_eventQueue, &attr, SYS_EVENT_QUEUE_LOCAL, queueDepth );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_queue_create failed (%d)\n", ret) );

	//Create a ppu_thread that will be called for handling events
	uint64_t arg = (U32)this;
	ret = sys_ppu_thread_create( &m_eventHandlerThread, EventHandlerThread, arg, ppuThreadPriority, kStackSize, SYS_PPU_THREAD_CREATE_JOINABLE, "Job Manager Event Handler" );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_ppu_thread_create failed (%d)\n", ret) );


	//Create the marker port. This port is used for waiting for queued events to be processed
	ret = sys_event_port_create( &m_markerPort, SYS_EVENT_PORT_LOCAL, kMarkerPortName );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_port_create failed %d\n", ret) );

	//and connect the marker port to the event queue
	ret = sys_event_port_connect_local( m_markerPort, m_eventQueue );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_port_connect_local failed %d\n", ret) );
	


	//Connect to SPURS - Well, really attach it to each SPU in the Spurs object.
	//We attach 4 different ports:
	//	kSpuPortJobPrintf is used for our JobPrintf
	//	kSpuPortPrintf is the port for spu_printf
	//	kSpuPortImmediateAuditOutput is used when audits are output in immediate mode
	//	m_userEventPortNum is allocated to us for usage when events are sent from the job to the PPU

	uint8_t	jobPrintfPort = kSpuPortJobPrintf;
	WWSJOB_ASSERT( jobPrintfPort <= CELL_SPURS_STATIC_PORT_RANGE_BOTTOM );
	ret = cellSpursAttachLv2EventQueue( spurs, m_eventQueue, &jobPrintfPort, false );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("spu_printf_server_initialize: cellSpursAttachLv2EventQueue failed %d\n", ret) );

	uint8_t	spuPrintfPort = kSpuPortPrintf;
	WWSJOB_ASSERT( spuPrintfPort <= CELL_SPURS_STATIC_PORT_RANGE_BOTTOM );
	ret = cellSpursAttachLv2EventQueue( spurs, m_eventQueue, &spuPrintfPort, false );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("spu_printf_server_initialize: cellSpursAttachLv2EventQueue failed %d\n", ret) );

	uint8_t	immediateAuditPort = kSpuPortImmediateAuditOutput;
	WWSJOB_ASSERT( immediateAuditPort <= CELL_SPURS_STATIC_PORT_RANGE_BOTTOM );
	ret = cellSpursAttachLv2EventQueue( spurs, m_eventQueue, &immediateAuditPort, false );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("spu_printf_server_initialize: cellSpursAttachLv2EventQueue failed %d\n", ret) );

	if ( m_pUserEventCallback )
	{
		ret = cellSpursAttachLv2EventQueue( spurs, m_eventQueue, &m_userEventPortNum, true );
		WWSJOB_ASSERT( CELL_OK == ret );
	}
	else
	{
		//if the user hasn't passed in a callback function, then don't hook a port
		m_userEventPortNum = 0xFF;
	}
	//JobPrintf( "event portNum = %d\n", m_userEventPortNum );

	m_pSpurs = spurs;
}

//--------------------------------------------------------------------------------------------------

void EventHandler::RemoveEventHandler( CellSpurs* spurs )
{
	WWSJOB_ASSERT( spurs == m_pSpurs );	//check the user has passed in the same spurs object
	//Note that I'm deliberately not using the cached pointer to the spurs object to make it
	//clear that there's a dependency on this being called before the spurs object is destoryed.
	m_pSpurs  = NULL;	

	int ret;
	WWSJOB_UNUSED( ret );

	//Disconnect ourselves from the SPURS SPUs
	ret = cellSpursDetachLv2EventQueue( spurs, kSpuPortJobPrintf );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("cellSpursDetachLv2EventQueue failed %d\n", ret) );
	ret = cellSpursDetachLv2EventQueue( spurs, kSpuPortPrintf );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("cellSpursDetachLv2EventQueue failed %d\n", ret) );
	ret = cellSpursDetachLv2EventQueue( spurs, kSpuPortImmediateAuditOutput );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("cellSpursDetachLv2EventQueue failed %d\n", ret) );
	if ( m_pUserEventCallback )
	{
		ret = cellSpursDetachLv2EventQueue( spurs, m_userEventPortNum );
		WWSJOB_ASSERT_MSG( CELL_OK == ret, ("cellSpursDetachLv2EventQueue failed %d\n", ret) );
	}

	//Disconnect and destroy the marker port
	ret = sys_event_port_disconnect( m_markerPort );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_disconnect failed %d\n", ret) );
	ret = sys_event_port_destroy( m_markerPort );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_port_destroy failed %d\n", ret) );


	sys_event_port_t	terminatingPort;

	//Create the terminating port. This port is used only for shutdown
	ret = sys_event_port_create( &terminatingPort, SYS_EVENT_PORT_LOCAL, kTerminatingPortName );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_port_create failed %d\n", ret) );
	//and connect the terminating port to the event queue
	ret = sys_event_port_connect_local( terminatingPort, m_eventQueue );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_port_connect_local failed %d\n", ret) );

	//Send event for termination.
	ret = sys_event_port_send( terminatingPort, 0, 0, 0 );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_port_send failed %d\n", ret) );

	//And wait for termination of the handler thread 
	uint64_t	exit_code;
	ret = sys_ppu_thread_join( m_eventHandlerThread, &exit_code );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_ppu_thread_join failed %d\n", ret) );

	//Disconnect and destroy the terminating port
	ret = sys_event_port_disconnect( terminatingPort );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_disconnect failed %d\n", ret) );
	ret = sys_event_port_destroy( terminatingPort );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_port_destroy failed %d\n", ret) );


	//And clean the event_queue
	ret = sys_event_queue_destroy( m_eventQueue, 0 );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_queue_destroy failed %d\n", ret) );
}

//--------------------------------------------------------------------------------------------------

void EventHandler::StallForQueuedEvents( void )
{
	//We need to know when the event handler has processed all events that are currently queued

	volatile U32 markerValue;

	markerValue = 1;	//Set our maker to 1 and wait for the handler to set it to 0
	__sync();

	//Send an event to the event handler to tell it we're waiting on the marker being set to 0
	int ret;
	do
	{	
		ret = sys_event_port_send( m_markerPort, (U32)&markerValue, 0, 0 );
	} while( ret == EBUSY );
	WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_port_send failed %d\n", ret) );

	//Block until the marker is set to 0 (ie. all preceding queued events have been processed)
	while ( markerValue )
	{
	}
}

//--------------------------------------------------------------------------------------------------

void EventHandler::EventHandlerThread( uint64_t arg )
{
	const EventHandler* pThis = (const EventHandler*) (U32) arg;

	while ( true )
	{
		sys_event_t	event;

		int ret = sys_event_queue_receive( pThis->m_eventQueue, &event, 0 );
		WWSJOB_UNUSED( ret );
		WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_event_queue_receive failed (%d)\n", ret) );

		//This function doesn't actually exist yet.
		//I've just put this assert in to remind myself that I'd really like to put this assert in
		//WWSJOB_ASSERT( sys_event_queue_num_dropped_events() == 0 );

		switch ( event.source )
		{
		case kTerminatingPortName:
			JobPrintf( "Finalizing the SPU event handler\n" );
			sys_ppu_thread_exit( 0 );
			break;
		case kMarkerPortName:
			{
				//Acknowledge the event by setting the marker to zero.
				volatile U32* pMarkerValue = (volatile U32*)(U32) event.data1;
				*pMarkerValue = 0;
				__sync();
			}
			continue;
		default:
			//Carry on to the code below
			break;
		}

		sys_spu_thread_t spu = event.data1;
		U32 spuNum	= ((U32)spu) / 0x1000000;					//:HACK: Dodgy calculation of the spuNum

		U32 portNum	= (event.data2 >> 32L);

		switch ( portNum )
		{
		case kSpuPortPrintf:
			//JobBasePrintf( "SPU%d: Warning: \"spu_printf\" called instead of \"JobPrintf\"\n", spuNum );
			//fall through
		case kSpuPortJobPrintf:
			{
				char buffer[256];
				int sret = spu_thread_snprintf( buffer, sizeof(buffer), spu, event.data3 );

				pThis->m_pPrintfCallback( spuNum, buffer );

				ret = sys_spu_thread_write_spu_mb( spu, sret );
				WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_spu_thread_write_spu_mb failed (%d)\n", ret) );
			}
			break;

		case kSpuPortImmediateAuditOutput:
			WWSJOB_ASSERT( (event.data2 & 0xFFFFFFFF) == 0 );

			//printf( "ImmediateAuditData = 0x%08X\n", (U32)event.data3 );
			WWSJOB_ASSERT( pThis->m_pAuditManager );	//This instance of the EventHandler must have an Audit Manager
														// instance in order to process immediate mode audits
			pThis->m_pAuditManager->ImmediateModeDataU32( event.data3, spuNum, AuditManager::PrintAudit, NULL );

			ret = sys_spu_thread_write_spu_mb( spu, 0x34543 );	//param value is irrelevant
			WWSJOB_ASSERT_MSG( CELL_OK == ret, ("sys_spu_thread_write_spu_mb failed (%d)\n", ret) );
			break;

		default:
			WWSJOB_ASSERT_MSG( portNum == pThis->m_userEventPortNum, ("Invalid portNum for event from SPU\n") );
			WWSJOB_ASSERT_MSG( pThis->m_pUserEventCallback, ("SPU callback event received but no handler function registered\n") );
			pThis->m_pUserEventCallback( event.data2 & 0xFFFFFF, event.data3 );
			break;
		}
	}

#ifndef __SNC__
	// never reach here
	WWSJOB_ASSERT( false );
#endif
}

//--------------------------------------------------------------------------------------------------

void EventHandler::PrintfSpuNum( U32 spuNum, const char* pPrintBuffer )
{
	JobBasePrintf( "SPU%d: %s", spuNum, pPrintBuffer );
}

//--------------------------------------------------------------------------------------------------
