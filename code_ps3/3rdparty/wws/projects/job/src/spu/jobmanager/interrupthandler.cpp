/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		interrupthandler.cpp

	@brief		Install an interrupt handler at LS0 that will be called on stall and notify events
**/
//--------------------------------------------------------------------------------------------------

#include <cell/spurs/types.h>
#include <cell/spurs/policy_module.h>
#include <cell/spurs/ready_count.h>
#include <spu_mfcio.h>
#include <stddef.h>

#include <jobapi/jobspudma.h>
#include <jobapi/jobdefinition.h>
#include <jobapi/audittypes.h>
#include <jobapi/jobmanagerauditids.h>
#include <jobapi/jobapi.h>
#include <jobapi/spumoduleheader.h>
#include <jobapi/jobdefinition.h>
#include <jobmanager/interrupthandler.h>
#include <jobmanager/allocatejob.h>
#include <jobmanager/auditwriter.h>
#include <jobmanager/jobmanagerdmatags.h>
#include <jobmanager/spustack.h>
#include <jobmanager/jobheadercache.h>
#include <jobmanager/jobmanager.h>
#include <jobmanager/data.h>

//--------------------------------------------------------------------------------------------------

#define SPU_BRANCH_INSTR( addr )	( 0x30000000 | (((U32) addr) << 5) )
#define SPU_STOP( signal )			( 0x00000000 | ((U32) signal) )

//--------------------------------------------------------------------------------------------------

extern "C" void SpuInterruptHandlerAsm( void );

//--------------------------------------------------------------------------------------------------

#if WWS_JOB_USE_C_VERSION!=0
void InstallInterruptHandler( void )
{
	//At LS address zero, write a branch instruction to our asm interrupt handler
	*(uint32_t*)0 = SPU_BRANCH_INSTR( SpuInterruptHandlerAsm );

	spu_write_event_mask( MFC_LIST_STALL_NOTIFY_EVENT );	//Dma List Stall And Notify Event
}
#endif

//--------------------------------------------------------------------------------------------------

#if WWS_JOB_USE_C_VERSION!=0
void ShutdownInterruptHandler( void )
{
	DisableInterrupts();

	spu_write_event_mask( 0x0 );	//Turn off all events

	//Clear out our branch at LS address zero
	*(U32*)0 = SPU_STOP( 0 );
}
#endif

//--------------------------------------------------------------------------------------------------

//namespace Wws
//{
//namespace Job
//{
	extern "C"  void WwsJob_Interrupts( U32 dmaTagMask );
//}
//};

#ifdef VERBOSE_ASSERTS_ENABLED
Bool32 g_bInInterruptHanlder = false;
#endif

extern "C" void InterruptHandlerCallback( void );
#if WWS_JOB_USE_C_VERSION!=0
void InterruptHandlerCallback( void )
{
	//The interrupt handler will save off all registers before calling this C function

#ifdef VERBOSE_ASSERTS_ENABLED
	//Check to make sure we don't get a recursive call to the interrupt handler.
	WWSJOB_VERBOSE_ASSERT( g_bInInterruptHanlder == false );
	g_bInInterruptHanlder = true;
#endif

	STORE_TIMING_AUDIT( AuditId::kWwsJob_InterruptHandler_begin );

	//If someone calls a null function pointer, the interrupt handler will mean that we end up here
	//We can tell if we're here by a mistaken call from user code, rather than an interrupt based
	//on whether or not interrupts are enabled.
	WWSJOB_ASSERT( !AreInterruptsEnabled() );
	//If interrupts are enabled, then we're in here by mistake.  Check for null function pointer
	//calls or possibly a broken build process.

	//Acknowledge the interrupt
	spu_write_event_ack( spu_read_event_status() );

	U32 activeTags = 0;

	//Check if we've got any dma list stall and notify event tags to acknowledge
	if ( mfc_stat_list_stall_status() > 0 )
	{	//This will always  be true since it's the only reason we use the interrupt handler

		//Find out which tags we need to acknowledge
		U32 tags = mfc_read_list_stall_status();

		//And acknowledge the stall and notifies  so that the dmas start again
		activeTags = tags;
		do
		{
			//One tag at a time
			U32 tagId = 31 - si_to_uint( si_clz( si_from_uint( tags ) ) );
			//Acknowledge the tag
			mfc_write_list_stall_ack( tagId );
			//Clear it from our list of tags to process
			tags ^= (1 << tagId);

		} while ( tags );

		//Note that at this point, the dmalists have been restarted.
		//They only have one more null dmalist element to go so shouldn't be long,
		//however this does take some amount of time.
		//If at this point you were to query the dmatag, it'd still be active.
		//for this reason, in WwsJob_Interrupts we'll spin a loop to wait for the dmalist
		//to fully end so that code executed during the interrupt handler is safe
		//and can poll the dmatag appropriately.
		//(This only needs to be done for certain dmatags, not necessarily all)

		//Now do the actual processing logic of this interrupt.
		WwsJob_Interrupts( activeTags );
	}

	//Just before returning from the interrupt handler, let's make sure we haven't
	//gone over the allocated stack space.
	//The interrupt handler (currently) uses a *lot* of extra stack space, so this
	//is probably the most risky part.
	CheckStackOverflowMarker();

	STORE_TIMING_AUDIT( AuditId::kWwsJob_InterruptHandler_end );

#ifdef VERBOSE_ASSERTS_ENABLED
	g_bInInterruptHanlder = false;
#endif
}
#endif

//--------------------------------------------------------------------------------------------------

#if WWS_JOB_USE_C_VERSION!=0
//This function adds a barrier command that waits for all dmas across all dma tags to have completed
//before it then triggers an interrupt
//(Sadly, it also prevents later dmas from moving earlier, but we can't do anything about that)
//We take a dma tag as a param because this passes info to the interrupt handler so that it then
//knows what to process.
//Note that we dont' actually use this function at present anyway.
/*void WwsJob_StartTagAgnosticBarrieredNullListWithInterrupt( U32 dmaTagId )
{
	spu_mfcdma32(	0, 
					0, 
					0,
					dmaTagId,
					MFC_BARRIER_CMD	//This barriers against all previous dmas
					);
	spu_mfcdma32(	0, 
					(U32)g_nullDmaInterrupt, 
					sizeof(g_nullDmaInterrupt),
					dmaTagId,
					MFC_GETLB_CMD	//This then triggers the interrupt
					);
}*/
#endif

//--------------------------------------------------------------------------------------------------

#if WWS_JOB_USE_C_VERSION!=0
//This functions will cause an interrupt to be generated after all previous dmas on the specified
//tag have been completed.
//It has nothing to do with dmas on other tags and these can go either before after this depending
//on what's most optimal for the hardware.
void WwsJob_StartTagSpecificBarrieredNullListWithInterrupt( U32 dmaTagId )
{
	spu_mfcdma32(	0, 
					(U32)g_nullDmaInterrupt, 
					sizeof(g_nullDmaInterrupt),
					dmaTagId,
					MFC_GETLB_CMD	//This barrier only barriers against earlier things on the same tag
					);
}
#endif

//--------------------------------------------------------------------------------------------------
