/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_DMA_H
#define ICE_DMA_H

namespace Ice 
{
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

	// Other useful DMA constants
	enum 
	{
		kDmaMaxSize    = 0x4000  // maximum size of a single DMA (16KB)
	};
}

#endif

