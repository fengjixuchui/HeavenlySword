/*
* Copyright (c) 2005 Naughty Dog, Inc. 
* A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
* Use and distribution without consent strictly prohibited
*/

#ifndef CAPTURE_API_H
#define CAPTURE_API_H

namespace BeyondDebugging
{

	namespace Network
	{
		/// network initialization helper
		/// don't call this if you initialize your own network 
		void Init ( void );

		/// network un-initialization helper
		/// don't call this if you initialize your own network 
		void Done ( void );
	}

	namespace Capture
	{
		/// initialize debug API
		void Init ( void );

		/// release debug API
		/// call this before you exit your app, or never
		void Done ( void );

		/// debug API heartbeat
		/// call this every frame before the Kick but after the "wait for previous frame"
		void Heartbeat ( unsigned int startIoOffset, unsigned int endIoOffset );

		/// debug API IDLE heartbeat
		/// call this in the loop inside the GPU crash routine, or during IDLE sequences where nothing occurs for a long time
		/// this function is equivalent to the one above, apart from setting up the kick
		void Heartbeat ( void );
	}

	namespace Capture
	{
		/// if it's time to capture ring buffer
		bool IsRingBufferCaptureTime ( void );

		/// specify capturing buffers
		void SetCaptureBuffers (
			unsigned int tempPushBufferOffset		// 8kb, main memory, 128 bytes aligned
		,	unsigned int tempCaptureBuffer0Offset	// 256kb, main memory, 4kb aligned
		,	unsigned int tempCaptureBuffer1Offset	// 256kb, main memory, 4kb aligned
		);

		/// capture ring buffer
		void CaptureRingBuffer ( unsigned int putEnd, volatile unsigned int * putPtr, unsigned int referenceValue );
	}
}

#endif
