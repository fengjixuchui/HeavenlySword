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

#ifndef WWS_JOB_EVENT_HANDLER_H
#define WWS_JOB_EVENT_HANDLER_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>
#include <jobsystem/helpers.h>

//--------------------------------------------------------------------------------------------------

struct CellSpurs;
class AuditManager;

//--------------------------------------------------------------------------------------------------

class EventHandler
{
	/*enum TtyOutputStyle
	{
		kTtyOutputToMainStream		= 1,
		kTtyOutputToOneStreamPerSpu	= 0,
	};*/

	typedef void (*UserEventCallback)( U32 data1, U32 data2 );
	typedef void (*PrintfCallback)( U32 spuNum, const char* pPrintBuffer );

public:
						EventHandler()	: m_userEventPortNum( 0xFF ), m_pSpurs( NULL ), m_pAuditManager( NULL ), m_pUserEventCallback( NULL ), m_pPrintfCallback( PrintfSpuNum ) {}
						EventHandler( CellSpurs* spurs, int ppuThreadPriority, AuditManager* pAuditManager = NULL, UserEventCallback func = NULL, U32 queueDepth = 127 );
						~EventHandler() { WWSJOB_ASSERT_MSG( m_pSpurs == NULL, ("Error: event handler was not removed\n") ); }

	void				InstallEventHandler( CellSpurs* spurs, int ppuThreadPriority, AuditManager* pAuditManager = NULL, UserEventCallback func = NULL, U32 queueDepth = 127 );
	void				RemoveEventHandler( CellSpurs* spurs );

	U32					GetPortNum( void ) const	{ WWSJOB_ASSERT( m_userEventPortNum != 0xFF ); return m_userEventPortNum; }

	void				SetPrintfCallback( PrintfCallback func ) { WWSJOB_ASSERT( func ); m_pPrintfCallback = func; }

	void				StallForQueuedEvents( void );

	static void			PrintfSpuNum( U32 spuNum, const char* pPrintfBuffer );

private:
						EventHandler( const EventHandler& );
	EventHandler		operator=( const EventHandler& );

	static void			EventHandlerThread( uint64_t arg ) __attribute__((noreturn));

	sys_event_queue_t	m_eventQueue;
	sys_ppu_thread_t	m_eventHandlerThread;
	uint8_t				m_userEventPortNum;
	sys_event_port_t	m_markerPort;	//Use in StallForQueuedEvents

	CellSpurs*			m_pSpurs;	//Only used for debug checking
	AuditManager*		m_pAuditManager;

	UserEventCallback	m_pUserEventCallback;
	PrintfCallback		m_pPrintfCallback;

	static const U32	kTerminatingPortName			= 0xFEE1DEAD;
	static const U32	kMarkerPortName					= 0xD0A77;

	static const U32	kSpuPortPrintf					= 1;	//This is the port spu_printf writes to
	static const U32	kSpuPortJobPrintf				= 15;	//Must be consistent with jobprintf.spu.s
	static const U32	kSpuPortImmediateAuditOutput	= 14;	//Must be consistent with auditwriter.cpp/auditwriter.spu
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_EVENT_HANDLER_H */
