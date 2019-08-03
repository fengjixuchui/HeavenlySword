/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Helper functions to run dmas in an interrupt safe manner
**/
//--------------------------------------------------------------------------------------------------

#include <jobapi/jobspudma.h>

#include <sys/return_code.h>

//--------------------------------------------------------------------------------------------------

void JobDmaLargeCmd( uintptr_t ls, U32 ea, U32 size, U32 tag, U32 cmd )
{
	WWSJOB_ASSERT( WwsJob_IsAligned( ls, 16 ) );
	WWSJOB_ASSERT( WwsJob_IsAligned( ea, 16 ) );
	WWSJOB_ASSERT( WwsJob_IsAligned( size, 16 ) );
	WWSJOB_ASSERT( tag < 32 );

	U32 lsAddress	= (U32) ls;
	U32 mmAddress	= ea;
	U32 mmLength	= size;

  #ifdef PRESERVE_INTERRUPT_STATUS
	Bool32 enabled = AreInterruptsEnabled();
	DisableInterrupts();
  #else
  	WWSJOB_ASSERT( AreInterruptsEnabled() );
	DisableInterrupts();
  #endif

	do
	{
		// max of 16K per dma
		U32 length = (mmLength > 0x4000) ? 0x4000 : mmLength;

		// start dma of main memory into miscData.m_buffer
		spu_mfcdma32( (void*)lsAddress, mmAddress, length, tag, cmd );
		lsAddress += length;
		mmAddress += length;
		mmLength  -= length;

	} while( mmLength > 0 );

  #ifdef PRESERVE_INTERRUPT_STATUS
	if(enabled)
		EnableInterrupts();
  #else
	EnableInterrupts();
  #endif
}

//--------------------------------------------------------------------------------------------------
