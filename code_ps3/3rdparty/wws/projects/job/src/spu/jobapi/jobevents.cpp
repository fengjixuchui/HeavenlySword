/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Job API functions for triggering events back on the PPU
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobapi.h>
#include <jobapi/jobprintf.h>
#include <jobapi/spuinterrupts.h>

#include <sys/spu_event.h>
#include <sys/return_code.h>

//--------------------------------------------------------------------------------------------------

//Note that throwing an event is unsafe, as the event may potentially get dropped and neither the
//PPU nor the SPU would ever know this had happened.  Maybe the SDK will be improved and this will
//be more useable soon?
/*void WwsJob_JobApiThrowEvent( U32 portNum, U32 data1, U32 data2 )
{
	WWSJOB_ASSERT( (data1 & 0xFF000000) == 0 );	//Can't send top byte of data1

	DisableInterrupts();
	U32 ret = sys_spu_thread_throw_event( portNum, data1, data2 );
	EnableInterrupts();

	WWSJOB_ASSERT( CELL_OK == ret );
	WWSJOB_UNUSED( ret );
}*/

//--------------------------------------------------------------------------------------------------

void WwsJob_JobApiSendEvent( U32 portNum, U32 data1, U32 data2 )
{
	WWSJOB_ASSERT( (data1 & 0xFF000000) == 0 );	//Can't send top byte of data1

	I32 ret;
	U32 countFrom = 0x1000;	//Starting from 1 is surely guaranteed to fail until we reach higher numbers

	while ( true )
	{
		DisableInterrupts();
		ret = sys_spu_thread_send_event( portNum, data1, data2 );
		EnableInterrupts();

		if ( ret != EBUSY )
		{
			break;
		}

		//If the handler was busy, keep trying until it accepts the event
		//but spin a (growing) delay loop first in order to avoid live-lock

		U32 count = countFrom;

		while ( count > 0 )
		{
			asm volatile ( "nop" );	//The compiler must not optimize out this loop
			--count;
		}

		//countFrom <<= 1;	//Should really be a rotate so that countFrom can never be zero
		countFrom = si_to_uint( si_roti( si_from_uint( countFrom ), 1 ) );
	}

	WWSJOB_ASSERT( CELL_OK == ret );
}

//--------------------------------------------------------------------------------------------------
