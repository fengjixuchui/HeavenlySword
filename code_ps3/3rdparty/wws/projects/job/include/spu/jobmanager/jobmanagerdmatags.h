/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Certain tag numbers have specific meaning to the job manager
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB_MANAGER_DMA_TAGS_H
#define WWS_JOB_JOB_MANAGER_DMA_TAGS_H

//--------------------------------------------------------------------------------------------------

namespace DmaTagId
{
	enum
	{
		// NOTE: at least one of these equates MUST be 0

		kLoadJob_readCommands,															// will interrupt
		//This is the dma tag used for the input dma that will pull in the load commands.
		//We will trigger an interrupt to process this buffer when it arrives. 

		kLoadJob_readBuffers,			// MUST BE kRunJob_readBuffers  - 1             // will *not* interrupt
		//UseBuffer in the load commands will use this tag to pull in input buffers.

		kRunJob_readBuffers,			// MUST BE kLoadJob_readBuffers + 1				// will *not* interrupt
		//UseBuffer in the run-time commands will use this tag to pull in input buffers. 

		kRunJob_writeShareableBuffers,	// MUST BE kStoreJob_writeShareableBuffers - 1	// will interrupt
		//This tag is used when the load job wants to use memory, but must wait until
		//the run job shareable buffers have been written out

		kStoreJob_writeShareableBuffers,// MUST BE kRunJob_writeShareableBuffers   + 1	// will *not* interrupt
		//This tag is used when the run job wants to use memory, but must wait until
		//the last store job shareable buffers have been written out

		kStoreJob_writeAllShareableBuffers,												// will interrupt
		//This tag is used when the load job wants to use memory, but must wait until
		//the last store job shareable buffers have been written out

		kStoreJob_writeJobBuffers,														// will interrupt
		//The only dma on this tag is the tag agnostic barriered null list with interrupt which
		//we use in order to trigger an interrupt once all user output dmas have completed. 
		//Since we are no longer (currently) request an interrupt for processing when a user's
		//output dmas are completed, this tag is in fact currently not used at all 

		kAudits,
		//This is the tag used by dmas done by the audits system 
				
		kNumUsed,	// this just tells us how many equates there are.  It is not used for anything else



		//When we need to a blocking DMA load, this is the DMA tag we will use.
		//If there was another DMA active on this tag, then the blocking load would
		//wait for that dma also.  This may only be a rare problem, but to further
		//minimise the risk, we choose to use the lowest numbered of the user DMA tags.
		//This is because the allocation algorithm starts from the top and works down,
		//so it's highly likely this tag won't even be allocated.
		//(Allocating a tag permanently just for blocking loads would seem gratuitous).
		kBlockingLoad		= kNumUsed,
	};
}

//--------------------------------------------------------------------------------------------------

//This probably doesn't want to be in here
#define DMA_READ( lsa, mma, length, dmaTagId ) \
		spu_mfcdma32( (void*)(lsa), (mma), (length), (dmaTagId), kDmaGet );

#define DMA_GATHER_READ( lsa, lsListElements, listElementsLength, dmaTagId ) \
		spu_mfcdma32( (void*)(lsa), (lsListElements), (listElementsLength), (dmaTagId), kDmaGetl );

#define DMA_BARRIERED_GATHER_READ( lsa, lsListElements, listElementsLength, dmaTagId ) \
		spu_mfcdma32( (void*)(lsa), (lsListElements), (listElementsLength), (dmaTagId), kDmaGetlb );

#define DMA_WRITE( lsa, mma, length, dmaTagId ) \
		spu_mfcdma32( (void*)(lsa), (mma), (length), (dmaTagId), kDmaPut );

#define DMA_GETLLAR( lsa, mma )	do { spu_mfcdma32( (void*)(lsa), (mma), 128, 0, kDmaGetllar ); } while ( 0 )

#define DMA_PUTLLC( lsa, mma )	do { spu_mfcdma32( (void*)(lsa), (mma), 128, 0, kDmaPutllc ); } while ( 0 )

#define DMA_WAITATOMICSTATUS()	mfc_read_atomic_status()

//--------------------------------------------------------------------------------------------------

// DMA opcodes for SPU DMAs
enum
{
	// Put Commands
	kDmaPut        = 0x20,   // put
	kDmaPutb       = 0x21,   // put with barrier
	kDmaPutf       = 0x22,   // put with fence
	kDmaPutl       = 0x24,   // put list
	kDmaPutlb      = 0x25,   // put list with barrier
	kDmaPutlf      = 0x26,   // put list with fence

	// Get Commands
	kDmaGet        = 0x40,   // get
	kDmaGetb       = 0x41,   // get with barrier
	kDmaGetf       = 0x42,   // get with fence
	kDmaGetl       = 0x44,   // get list
	kDmaGetlb      = 0x45,   // get list with barrier
	kDmaGetlf      = 0x46,   // get list with fence

	// Storage Control Commands
	kDmaSdcrz      = 0x89,   // zeros the contents of a range of EAs
	kDmaSdcrst     = 0x8D,   // stores the modified contents of a range of EAs
	kDmaSdcrf      = 0x8F,   // stores the modified contents of a range of EAs and invalidates block

	// Synchronization Commands
	kDmaPutlluc    = 0xB0,   // put link line unconditional
	kDmaPutllc     = 0xB4,   // put link line conditional
	kDmaPutqlluc   = 0xB8,   // put link line unconditional (queued)
	kDmaSndsig     = 0xA0,   // send signal
	kDmaSndsigb    = 0xA1,   // send signal with barrier
	kDmaSndsigf    = 0xA2,   // send signal with fence
	kDmaBarrier    = 0xC0,   // barrier
	kDmaMfceieio   = 0xC8,   // mfc enforce in-order execution of I/O transactions
	kDmaMfcsync    = 0xCC,   // mfc synchronization
	kDmaGetllar    = 0xD0    // get link line and reserve
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB_MANAGER_DMA_TAGS_H */
