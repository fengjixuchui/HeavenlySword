/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_RENDER_H
#define ICE_RENDER_H

#define ICERENDER_RSX_A01 1
#define ICERENDER_RSX_A02 2
#define ICERENDER_RSXTYPE ICERENDER_RSX_A02

#include <alloca.h>
#include "icebase.h"
#include "icebitextract.h"
#include "icetextureformats.h"
#include "icerenderconstants.h"

namespace Ice
{
	namespace Render
	{
		//! A Report that describes various internal gpu counters.
		struct Report
		{
			//! A time stamp of when the command was written. (Useful for performance analysis)
			/*! The timestamp is in nano-seconds. */
			U64 m_timeStamp;
			//! The value of the report. This changes meaning based on the report type.
			/*! kReportPixelCount:          The number of depth pass pixels written since the last Report Reset.
			    kReportDepthCullFeedbackA:  A Depth Cull Processor feedback value used to optimize Depth Cull performance. Use with SetDepthCullFeedback().
			    kReportDepthCullFeedbackB:  A Depth Cull Processor feedback value used to optimize Depth Cull performance. Use with SetDepthCullFeedback().
			    kReportDepthCullFeedbackC:  A Depth Cull Processor feedback value. 
			    kReportDepthCullFeedbackD:  A Depth Cull Processor feedback value.
			*/
			U32 m_value;
			//! The status is non-zero on write failure. 
			//! This parameter is only applicable to the Depth Cull processor reports.
			U32 m_status;

			//! Return the timestamp in nano-seconds.
			/*! This is deprecated and will be removed in the next few revisions. */
			U64 InterpretTimeStamp() const DEPRECATED { return m_timeStamp; }
		};
		
		//! Description of the contents of Report::m_value when the report is a kReportDepthCullFeedbackA.
		struct ReportDepthCullFeedbackAValue
		{
			U16 m_maxSlopeWritten;
			U16 m_numSlopeSamplesWritten;
		};
		
		//! Description of the contents of Report::m_value when the report is a kReportDepthCullFeedbackB.
		struct ReportDepthCullFeedbackBValue
		{
			U32 m_totalSamplesWritten;
		};
		
		//! Description of the contents of Report::m_value when the report is a kReportDepthCullFeedbackC.
		struct ReportDepthCullFeedbackCValue
		{
			U32 m_totalSamplesTested;
		};
		
		//! Description of the contents of Report::m_value when the report is a kReportDepthCullFeedbackD.
		struct ReportDepthCullFeedbackDValue
		{
			U32 m_timeStamp;
		};

		//! GPU hardware configuration information.
		struct GpuHardwareConfig
		{
			//! The start of video memory as mapped into user space.
			void *m_videoMemoryBaseAddr;
			//! The total size of memory on the video card.
			U32 m_videoMemorySize;
			//! The total size of usable memory on the video card.
			U32 m_usableVideoMemorySize;
			//! Frequency of the video card's core in Hz.
			U32 m_coreFrequency;
			//! Frequency of the video card's memory in Hz.
			U32 m_videoMemoryFrequency;
			//! Semaphore private RSX memory area base address
			U32 m_semaphoreBaseAddress;
			//! The number of bytes between semaphores.
			U32 m_semaphoreStride;
			//! The current display swap mode.
			DisplaySwapMode m_displaySwapMode;
			//! The current display output width.
			U32 m_displayWidth;
			//! The current display output height.
			U32 m_displayHeight;
			//! The current display output pitch.
			U32 m_displayPitch;
			//! The current display aspect ratio.
			DisplayAspectRatio m_aspectRatio;
			//! An ea to io offset table, 1 entry for every meg in the main address space
			U16 m_ioTable[0x1000];
			//! An ea to io offset table, 1 entry for every meg in the gpu address space
			U16 m_eaTable[0x100];
		};

		//! A structure that represents the memory layout of a section of memory mapped registers.
		struct GpuControl
		{
			//! This gpu memory mapped register contains the put offset within the push buffer.
			/*! This is to be set 4 bytes after the last valid data in the push buffer.
			    The offset must be 4-byte aligned. 
			    If the register is set to an invalid value (out of range, or not aligned) a gpu crash will occur.
			*/
			U32 m_putOffset;
			//! This gpu memory mapped register contains the get offset within the push buffer.
			/*! It is set to the next offset that the gpu will read.
			    The offset must be 4-byte aligned.
			    Access to the mmr is asynchronous and does not block writes or reads to subsequent push buffer commands.
			*/
			U32 m_getOffset;
			//! This gpu memory mapped register contains last encountered reference value.
			/*! The mmr is read-only and any written value is ignored.
			    Access to the mmr is asynchronous and does not block writes or reads to subsequent push buffer commands.
			*/
			I32 m_reference;
		};

#ifdef __SPU__
		//! Translates a VRAM address to the corresponding GPU offset.
		/*! \param addr  A pointer to a location in VRAM.
		    \return      The GPU offset corresponding to the address.
		*/
		static inline U32 TranslateAddressToOffset(void const *vmStart, void const *addr)
		{
			ICE_ASSERT(U32(addr) >= U32(vmStart));
			return U32(addr) - U32(vmStart);
		}
		static inline U32 TranslateAddressToOffset(void const *vmStart, void *addr) { return TranslateAddressToOffset(vmStart, const_cast<void const *>(addr)); }
		static inline U32 TranslateAddressToOffset(void const *vmStart, void volatile *addr) { return TranslateAddressToOffset(vmStart, const_cast<void const *>(addr)); }

		//! Translates a GPU offset to the corresponding VRAM address.
		/*! \param offset  A GPU offset.
		    \return        The VRAM address corresponding to the offset.
		*/
		static inline void *TranslateOffsetToAddress(void const *vmStart, U32 offset)
		{
			return (void *) (U32(vmStart) + offset);
		}

		//! Translates a main memory address to the corresponding Io address.
		/*! \param addr  A pointer to a location in main memory.
		    \return      The Io address corresponding to the main memory address specified.
		*/
		static inline U32 TranslateAddressToIoOffset(U16 const *ioTable, void const *addr)
		{
			U32 offset = ioTable[(U32)addr >> 20];
			if(offset >= 256)
				return 0xFFFFFFFF;
			U32 ea = (U32)addr;
			return (offset << 20) | (ea & 0x000FFFFF);
		}
		static inline U32 TranslateAddressToIoOffset(U16 const *ioTable, void *addr) { return TranslateAddressToIoOffset(ioTable, const_cast<void const *>(addr)); }
		static inline U32 TranslateAddressToIoOffset(U16 const *ioTable, void volatile *addr) { return TranslateAddressToIoOffset(ioTable, const_cast<void const *>(addr)); }

		//! Translates an Io address to the corresponding main memory address.
		/*! \param ioOffset  An Io Address.
		    \return          The main memory address corresponding to the IO address.
		*/
		static inline void *TranslateIoOffsetToAddress(U16 const *eaTable, U32 offset)
		{
			ICE_ASSERT((offset >> 20) < 256);
			U32 ea = eaTable[offset >> 20];
			if(ea >= 4096)
				return NULL;
			return (void*)((ea << 20) | (offset & 0x000FFFFF));
		}

#else
		//! A pointer to Gpu memory mapped registers which describe the status of the gpu fifo.
		extern volatile GpuControl *g_gpuControl;
		
		//! The GPU's hardware configuration.  
		/*! This is filled in by a call to Initialize() and then never changes. */
		extern GpuHardwareConfig g_gpuHardwareConfig;

#ifndef __SPU__
		//! The null fragment program's io offset;
		extern U32 g_nullFragmentProgramOffset;
#endif

		//! Translates a VRAM address to the corresponding GPU offset.
		/*! \param addr  A pointer to a location in VRAM.
		    \return      The GPU offset corresponding to the address.
		*/
		static inline U32 TranslateAddressToOffset(void const *addr)
		{
			ICE_ASSERT(U32(addr) >= U32(g_gpuHardwareConfig.m_videoMemoryBaseAddr));
			ICE_ASSERT(U32(addr) - U32(g_gpuHardwareConfig.m_videoMemoryBaseAddr) < g_gpuHardwareConfig.m_usableVideoMemorySize);
			return U32(addr) - U32(g_gpuHardwareConfig.m_videoMemoryBaseAddr);
		}
		static inline U32 TranslateAddressToOffset(void *addr) { return TranslateAddressToOffset(const_cast<void const *>(addr)); }
		static inline U32 TranslateAddressToOffset(void volatile *addr) { return TranslateAddressToOffset(const_cast<void const *>(addr)); }

		//! Translates a GPU offset to the corresponding VRAM address.
		/*! \param offset  A GPU offset.
		    \return        The VRAM address corresponding to the offset.
		*/
		static inline void *TranslateOffsetToAddress(U32 offset)
		{
			ICE_ASSERT(offset < g_gpuHardwareConfig.m_usableVideoMemorySize);
			return (void *) (U32(g_gpuHardwareConfig.m_videoMemoryBaseAddr) + offset);
		}

		//! Maps a region of main memory to be viewable by the Gpu.
		/*! \param addr  A pointer to a location in main memory. Must be 1 megabyte aligned.
		    \param size  The size of the region to map. Must be a multiple of 1 megabyte.
		    \return      The IO address corresponding to the main memory address.
		*/
		U32 MapMainMemoryToIoOffset(void const *addr, U32 size);

		//! Translates a main memory address to the corresponding Io address.
		/*! \param addr  A pointer to a location in main memory.
		    \return      The Io address corresponding to the main memory address specified.
		*/
		static inline U32 TranslateAddressToIoOffset(void const *addr)
		{
			U32 offset = g_gpuHardwareConfig.m_ioTable[(U32)addr >> 20];
			if(offset >= 256)
				return 0xFFFFFFFF;
			U32 ea = (U32)addr;
			return Rlwimi(ea, offset, 20, 0, 11);
		}
		static inline U32 TranslateAddressToIoOffset(void *addr) { return TranslateAddressToIoOffset(const_cast<void const *>(addr)); }
		static inline U32 TranslateAddressToIoOffset(void volatile *addr) { return TranslateAddressToIoOffset(const_cast<void const *>(addr)); }

		//! Translates an Io address to the corresponding main memory address.
		/*! \param ioOffset  An Io Address.
		    \return          The main memory address corresponding to the IO address.
		*/
		static inline void *TranslateIoOffsetToAddress(U32 offset)
		{
			ICE_ASSERT((offset >> 20) < 256);
			U32 ea = g_gpuHardwareConfig.m_eaTable[offset >> 20];
			if(ea >= 4096)
				return NULL;
			return (void*)Rlwimi(offset, ea, 20, 0, 11);
		}
#endif
	}
}

#include "icetexture.h"
#include "icetexturereduced.h"
#include "icerendertarget.h"
#include "icevertexprogram.h"
#include "icefragmentprogram.h"
#include "icecommandcontext.h"
#ifndef __SPU__
#include "iceperfevents.h"
#endif
#include "icerenderprocs.h"
#ifndef __SPU__
#include "icememory.h"
#endif
#include "iceglobalcommandcontext.h"

#ifndef __SPU__
namespace Ice
{
	namespace Render
	{
		// INITIALIZATION AND TERMINATION FUNCTIONS


		//! Initializes the rendering library.
		/*! \param mode                      The display mode.
		    \param outMode                   The color format of the output buffer.
		    \param outputColorBufferPitch    The size in bytes of a row in the output color buffer. (This is ignored if mode is kDisplayAutoLinear or kDisplayAutoTiled.)
		    \param defaultCommandAddress     A pointer to the start of a memory region to be made gpu accessable. This gets mapped to io offset 0. Must be 1 megabyte aligned.
		    \param defaultCommandSize        The size of the region in bytes. Must be a multiple of 1 megabyte.
		    \param initialReferenceValue     Initial reference value.
		    \param enablePerformanceReports  Set this to true if you want to perform performance reports. (Performance reports disable the gpu's ability to perform power management)
		    \return                          The amount of default command memory used by IceRender and not available to the application.
		*/
		U32 Initialize(DisplayMode mode, DisplayOutputMode outMode, U32 outputColorBufferPitch, void *defaultCommandAddress, U32 defaultCommandSize, I32 initialReferenceValue, bool enablePerformanceReports=false) WARN_UNUSED_RESULT;

		//! Terminates the rendering library.
		void Terminate();

		// MISC FUNCTIONS & STRUCTURES

		//! Kicks the command context. Queues a render target for display. The render target must have previously
		//! been registered for display using the RegisterRenderTargetForDisplay() function.
		/*! \param startIoOffset  The beginning of the frame's Gpu commands.
		    \param end            The end of the frame's Gpu commands. The 32-bits of data at *end must always be available for internal use.
		    \param target         The render target to scanout.
		    \param reference      The reference value to set when the previous frame's push buffer is completely consumed.
		    \param stallGpu       The gpu will not automatically continue processing of the next frame if this is set.
		    \return               An Io address to the last valid instruction that the Gpu can process. (!!Not the same as end!!)
		*/
		U32 KickAndDisplayRenderTarget(U32 startIoOffset, U32 *end, const RenderTarget *target, I32 reference, bool stallGpu = false);

		//! Gets the last reference completed.
		/*! \return  The last reference identifier completed.
		*/
		static inline I32 GetReference()
		{
			return g_gpuControl->m_reference;
		}

		//! Tests a reference for completion.
		/*! \param value  The reference value.
		    \return       True if the reference has been consumed; false otherwise.
		*/
		static inline bool TestReference(I32 value)
		{
			return GetReference() - value >= 0;
		}

		//! Retrieves the address of the value of the semaphore of the specified index.
		/*! \param index  The semaphore index.
		    \return       The address of the value of the semaphore.
		*/
		static inline volatile U32 *GetSemaphoreAddress(U32 index)
		{
			ICE_ASSERT(index < kMaxSemaphores);
			return (volatile U32*)(g_gpuHardwareConfig.m_semaphoreBaseAddress + index * g_gpuHardwareConfig.m_semaphoreStride);
		}

		//! Retrieves the value of the semaphore of the specified index.
		/*! \param index  The semaphore index.
		    \return       The value of the semaphore.
		*/
		static inline U32 GetSemaphoreValue(U32 index)
		{
			ICE_ASSERT(index < kMaxSemaphores);
			return *(volatile U32*)(g_gpuHardwareConfig.m_semaphoreBaseAddress + index * g_gpuHardwareConfig.m_semaphoreStride);
		}

		//! Stores the value specified at the semaphore of the specified index.
		/*! \param index  The semaphore index.
		    \param value  The value to store.
		*/
		static inline void SetSemaphoreValue(U32 index, U32 value)
		{
			ICE_ASSERT(index < kMaxSemaphores);
			*(volatile U32*)(g_gpuHardwareConfig.m_semaphoreBaseAddress + index * g_gpuHardwareConfig.m_semaphoreStride) = value;
		}
	}
}
#endif

#endif // ICE_RENDER_H

