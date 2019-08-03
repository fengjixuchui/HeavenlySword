/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_BATCHJOBBUFFERS_H
#define ICE_BATCHJOBBUFFERS_H

/*! 
 * \file 	icebatchjobbuffers.h
 * \brief	Constants related to local store buffers and locations used by batch jobs.
 */

namespace Ice
{
    namespace BatchJob
    {
		/*!
		 * Local store addresses are represented as handles in the following format:
		 * location = (address - address0[buffer]) | bufferTag;
		 *
		 * With 16 bit handles, his allows for buffers up to 64k in size.  Should a larger work buffer 
		 * be required, we can always shift the offset down by 1 or 2 bits. and still have 2 bits for 
		 * the bufferHandle.
		 */
		typedef U16 Location;

		/*!
		 * Location buffer tags for each of the 3 batch job local store buffer types:
		 * input, output, and work.
		 *
		 * kLocOutput is a special case, with its offset is relative to the current output buffer location, 
		 * which is set by a preceding ReserveOutputBuffer command.  kLocVolatile is also a special case,
		 * and points at a sub-section of the current input buffer where volatile data is stored.
		 */
		enum LocationBufferTag
		{
			kLocInput = 		0x0000,		//!< relative to current input buffer start
			kLocWork =			0x0004,		//!< relative to work buffer start
			kLocOutput =		0x0008,		//!< output to current output buffer, current output offset
			kLocVolatile =		0x000c,		//!< relative to current input buffer volatile data start
		};

		/*!
		 * The job manager requires each set of buffers to be assigned a numeric handle.
		 */
		enum JobManagerBufferSet
		{
			kInputBufferSet 	= 0,
			kWorkBufferSet,
			kOutputBufferSet,
			kCodeBufferSet,			// only used by WWS job manager
		};

		/*!
		 * Number of buffers in each set used by batch jobs
		 */
		enum BatchJobBufferCount
		{
			kNumInputBuffers	= 2,
			kNumWorkBuffers		= 1,
			kNumOutputBuffers	= 2,
			kNumCodeBuffers		= 1,
		};

		/*!
		 *	A description of the data buffers associated with a batch job
		 */
		struct BatchJobBufferDesc {
			U32 sizeofInputBuffer;	// size of each input buffer (2 buffers) - must be a multiple of 1K pages and no more than 64K
			U32 sizeofWorkBuffer;	// size of work buffer (1) - must be a multiple of 1K pages and no more than 64K
			U32 sizeofOutputBuffer;	// size of each output buffer (2 buffers) - must be a multiple of 1K pages and no more than 64K
		};
	}
};

#endif //ICE_BATCHJOBBUFFERS_H
