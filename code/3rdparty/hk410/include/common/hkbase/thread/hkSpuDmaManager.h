/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SPU_DMA_MANAGER_H
#define HK_SPU_DMA_MANAGER_H

#include <hkbase/hkBase.h>


enum
{
	HK_SPU_DMA_GROUP_STALL	= 14,
	HK_SPU_DMA_GROUP_IDLE	= 15, // DO NOT use this dma group for any get/put operations! use it only for 'idle' waiting.
};


	/// Use this for asynchronous or synchronous dmas
class hkSpuDmaManager
{

	public:
		enum READ_MODE
		{
				// use this if you only want a pure read only copy
			READ_ONLY,	
				// use this if you want to copy the data, want to change it but do not want to write it back
			READ_COPY,
			READ_WRITE
		};

			/// Start fetching data from main memory. 
		HK_FORCE_INLINE static void HK_CALL getFromMainMemory(void* dstOnSpu, const HK_CPU_PTR(void*) srcInMainMemory, int size, READ_MODE mode, int dmaGroupId = HK_SPU_DMA_GROUP_STALL);

			/// Start fetching small data from main memory (1,2,4, or 8 bytes). 
		HK_FORCE_INLINE static void HK_CALL getFromMainMemorySmall(void* dstOnSpu, const HK_CPU_PTR(void*) srcInMainMemory, int size, READ_MODE mode, int dmaGroupId = HK_SPU_DMA_GROUP_STALL);

			/// Fetch data from main memory.
			///
			/// Stalls the thread until data has arrived. Note that this might actually stall longer
			/// than necessary if there are any additional ongoing dma's that share the same id.
		static void HK_CALL getFromMainMemoryAndWaitForCompletion(void* dstOnSpu, const HK_CPU_PTR(void*) srcInMainMemory, int size, READ_MODE mode, int dmaGroupId = HK_SPU_DMA_GROUP_STALL);

			/// Fetch a small amount of data from main memory (1,2,4, or 8 bytes).
			///
			/// Stalls the thread until data has arrived. Note that this might actually stall longer
			/// than necessary if there are any additional ongoing dma's that share the same id.
		static void HK_CALL getFromMainMemorySmallAndWaitForCompletion(void* dstOnSpu, const HK_CPU_PTR(void*) srcInMainMemory, int size, READ_MODE mode, int dmaGroupId = HK_SPU_DMA_GROUP_STALL);

			/// Call this function once you do not need a fetched memory any more. This is only used for debugging on WIN32 and compiles to nothing on PS3
		HK_FORCE_INLINE static void HK_CALL performFinalChecks( const HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size );

		HK_FORCE_INLINE static void HK_CALL tryToPerformFinalChecks(const HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size );

		HK_FORCE_INLINE static void HK_CALL deferFinalChecksUntilWait(const HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size );

		enum WRITE_MODE
		{
			WRITE_BACK,
			WRITE_BACK_SUBSET, 
			WRITE_NEW
		};

			/// Start writing data back to main memory.
		HK_FORCE_INLINE static void HK_CALL putToMainMemory(HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size, WRITE_MODE mode, int dmaGroupId = HK_SPU_DMA_GROUP_STALL);

			/// Start writing small amount of data back to main memory (1,2,4, or 8 bytes).
		HK_FORCE_INLINE static void HK_CALL putToMainMemorySmall(HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size, WRITE_MODE mode, int dmaGroupId = HK_SPU_DMA_GROUP_STALL);

			/// Write data back to main memory.
			///
			/// Stalls the thread until data has been written. Note that this might actually stall longer
			/// than necessary if there are any additional ongoing dma's that share the same id.
		static void HK_CALL putToMainMemoryAndWaitForCompletion(HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size, WRITE_MODE mode, int dmaGroupId = HK_SPU_DMA_GROUP_STALL);

			/// Write data back to main memory (1,2,4, or 8 bytes).
			///
			/// Stalls the thread until data has been written. Note that this might actually stall longer
			/// than necessary if there are any additional ongoing dma's that share the same id.
		static void HK_CALL putToMainMemorySmallAndWaitForCompletion(HK_CPU_PTR(void*) dstInMainMemory, const void* srcOnSpu, int size, WRITE_MODE mode, int dmaGroupId = HK_SPU_DMA_GROUP_STALL);


			/// Stall the thread until dma is finished.
			///
			/// (i.e. wait until all data from the dmaGroupId group is available or has been written).
		HK_FORCE_INLINE static void HK_CALL waitForDmaCompletion(int dmaGroupId = HK_SPU_DMA_GROUP_STALL);

			/// Stall the thread until all dma has been finished.
		HK_FORCE_INLINE static void HK_CALL waitForAllDmaCompletion();

			/// this function converts the internal logic from READ_ONLY to READ_WRITE 
			///
			/// you can only do this after the calls to getFromMainMemory(READ_ONLY) and waitDmaGroup() have returned
			/// note: it is actually ok to call this function when already in READ_WRITE mode; it will immediately return then.
			// note: this is currently only used for contact constraints as all atoms are DMAed as READ_ONLY, yet
			// contact constraints need to write-back data to ppu
		HK_FORCE_INLINE static void HK_CALL convertReadOnlyToReadWrite(void* ppuPtr, const void* spuPtr, int size);

};

	/// this utility allows simple access to variable in main memory and can be used either from the SPU, PPU or CPU
struct hkSpuDmaUtils
{
	/// Convenient and slow function to increment a value, returns the new value
	static hkInt32 HK_CALL incrementInt32InMainMemory( HK_CPU_PTR(hkInt32*) variable, int increment, int dmaGroupId = HK_SPU_DMA_GROUP_STALL );

	/// Convenient and slow function set a variable
	static void HK_CALL setInt32InMainMemory( HK_CPU_PTR(hkInt32*) variable, hkInt32 value, int dmaGroupId = HK_SPU_DMA_GROUP_STALL );
	
		/// put a float to main memory & wait for completion
	static void HK_CALL setFloat32InMainMemory( HK_CPU_PTR(float*) dstInMainMemory, hkReal f, int dmaGroupId = HK_SPU_DMA_GROUP_STALL );

		/// put a pntr to main memory & wait for completion
	static void HK_CALL setPntrInMainMemory( HK_CPU_PTR(void**) dstInMainMemory, void* p, int dmaGroupId = HK_SPU_DMA_GROUP_STALL );

		/// put a char to main memory & wait for completion
	static void HK_CALL setChar8InMainMemory( HK_CPU_PTR(hkChar*) dstInMainMemory, hkChar c, int dmaGroupId = HK_SPU_DMA_GROUP_STALL );
};


#if defined HK_PLATFORM_PS3SPU
#	include <hkbase/thread/ps3/hkPs3SpuDmaManager.inl>
#elif defined HK_SIMULATE_SPU_DMA_ON_CPU
#	include <hkbase/thread/win32/hkWin32SpuDmaManager.inl>
#else
#	include <hkbase/thread/impl/hkEmptySpuDmaManager.inl>
#endif

#endif // HK_SPU_DMA_MANAGER_H


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
