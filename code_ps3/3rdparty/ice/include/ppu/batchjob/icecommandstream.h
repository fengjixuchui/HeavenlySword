/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_COMMANDSTREAM_H
#define ICE_COMMANDSTREAM_H

#include "icebatchjob.h"
#include "icememorybuffer.h"

namespace Ice {
	namespace BatchJob {
		class Plugin;
		class PluginArgs;

		/*!
		 * \brief The command stream records a stream of commands and their associated
		 * arguments and input data-streams in a format suitable for later execution
		 * on the SPU's.
		 *
		 * The resulting output is a chain of DMA lists, each of which will gather
		 * an input buffer containing input data, an associated command stream, and
		 * the DMA list for the following input buffer in the chain.  Data that will not
		 * persist through the execution of the task (headers, command streams, and other 
		 * 'volatiles') are also written to the output in between DMA lists.  The first 
		 * DMA list can then be used to prime an SPU job with its first input buffer.
		 * 
		 * The fundamental output of the command stream is thus the DMA lists. The DMA
		 * lists start with a header comprising four U32's:
		 *   - The location of the command list (after this DMA list has been transferred
		 *     to local memory).
		 *   - The location of any additional volatile data (again, after DMA)
		 *   - The location of the next DMA list header in the chain (again, after DMA)
		 *   - The size in bytes for the following list-DMA.
		 * After the header, is the size+address list-DMA.
		 *
		 * The command list is currently a stream of U16's, starting with the command
		 * enumeration.  Each command expects a predefined number of arguments between 0 
		 * and 8 to follow.
		 *
		 * This class keeps track of all memory being used within input and output buffers
		 * throughout the execution.  Work buffer allocations are tracked separately by
		 * the WorkBuffer class.  Since this is done ahead-of-time, the offsets within 
		 * each input or output buffer are known, and can be stored in a buffer position
		 * independant format as Ice::BatchJob::Location's.
		 */
		class CommandStream
		{
			static const U32 kReserveDmaListEntries =	 64;	// minimum number of dma list entries to reserve for the following buffer
			static const U32 kReserveDmaListSize =  	kReserveDmaListEntries*2*sizeof(U32);
			static const U32 kMaxCommands = 			512;	// maximum number of commands per dma
			static const U32 kMaxVolatileSize = 		256;	// maximum size in bytes of volatiles per dma

		public:
			static const U32 kReserveInputBufferSpace =	kReserveDmaListSize + 0x40;	// 576 bytes of space reserved in input buffer for minimal command list, dma header, and next dma list

			/*!
			 * \brief Constructor.
			 * \param pBuffer 		Pointer to a memory area that we can use for batching, which must persist through the duration of the task.
			 * \param bufferDesc	A description of the buffer sizes associated with this batched job.
			 */
			CommandStream(MemoryBuffer *pBuffer, BatchJobBufferDesc const &bufferDesc);
			~CommandStream() {}

			//! Returns a pointer to the initial DMA list that primes the task.
			U32 const *GetInitialDmaList() const { return (U32 const*)(m_initialDmaHeader + 1); }

			//! Returns the size of the initial DMA list in bytes.
			U32 GetInitialDmaListSize() const { return m_initialDmaHeader->dmaListSize; }

			//!	Restore this command stream to its initial state and begin constructing a new command stream.
			void Reset();

			/*!	Adds an entry in the current DMA list entry.
			 *	All DMA list elements must be a multiple of 16 bytes in size and 16 byte aligned _or_
			 *	size 1, 2, 4, or 8 and (respectively) 1, 2, 4, or 8 byte aligned.
			 *	In the local store, data smaller than 4 bytes is pre-padded with zeroes to place it in 
			 *	the preferred byte or halfword, and data smaller than 16 bytes is then post-padded with
			 *	zeroes out to 16 bytes.
			 * 	\returns A location handle to the input data in the local store input buffer.
			 */
			Location AddDmaListElement(const void *data, U32 size);

			/*!	Adds a volatile data entry to the current DMA volatile table.
			 *	Volatile data may have any size or alignment, but will be padded as follows to best match the
			 *	behavior of AddDmaListElement:
			 *	1 or 2 byte data is pre-padded with zeroes to 4 bytes to place it in the preferred byte or halfword.
			 *	All data is then post-padded with zeroes out to a multiple of 16 bytes.
			 * 	\returns A location handle to the input data in the local store volatile buffer (within the input buffer)
			 */
			Location AddVolatile(const void *data, U32 size);

			/*! Check to see if space is available in the input buffers and temporary
			 *	buffers for a sequence of commands that will send the specified total
			 *	amount of input and command data.
			 *	Returns true if an input buffer swap will be required.
			 */
			bool IsSwapRequired(U32 dataSize, U32 volatileSize, U32 dmaEntries, U32 cmdEntries) const;

			/*! Check to see that space is available in the input buffers and temporary
			 *	buffers for a sequence of commands that will send the specified total
			 *	amount of input and command data.  If not, swap input buffers.
			 */
			void Reserve(U32 dataSize, U32 volatileSize, U32 dmaEntries, U32 cmdEntries);

			/*!	Helper function to check that Reserve was called with the correct
			 *	parameters.  Call CheckReserve() at the end of the block of commands
			 *	Reserve() was associated with to make certain its parameters were
			 *	correct.
			 */
			void CheckReserve()
			{
#if ICEDEBUG
				ICE_ASSERT(m_command_reserve != NULL);	// We probably have a nested Reserve() or a mismatched CheckReserve()...
				ICE_ASSERT(m_command == m_command_reserve);
				ICE_ASSERT(m_inputSize == m_inputSize_reserve);
				ICE_ASSERT(m_volatileSize == m_volatileSize_reserve);
				//NOTE: if we are combining adjacent DMAs, we may have added fewer DmaListElements than we reserved:
				ICE_ASSERT(m_pBuffer->GetCurrentPos() <= (void const*)m_dmaList_reserve);
				m_command_reserve = NULL;
				m_inputSize_reserve = m_volatileSize_reserve = 0;
				m_dmaList_reserve = NULL;
#endif
			}

			//! Add a command with no arguments into the current command stream.
			void AddCommand(U16 cmd) {
				ICE_ASSERT(m_command < s_commandCache + kMaxCommands);
				AddCommandU16(cmd);
			}
			//! Add a command with 1 argument into the current command stream.
			void AddCommand(U16 cmd, U16 arg0)
			{
				ICE_ASSERT(m_command+1 < s_commandCache + kMaxCommands);
				AddCommandU16(cmd);
				AddCommandU16(arg0);
			}
			//! Add a command with 2 arguments into the current command stream.
			void AddCommand(U16 cmd, U16 arg0, U16 arg1) {
				ICE_ASSERT(m_command+2 < s_commandCache + kMaxCommands);
				AddCommandU16(cmd);
				AddCommandU16(arg0);
				AddCommandU16(arg1);
			}
			//! Add a command with 3 arguments into the current command stream.
			void AddCommand(U16 cmd, U16 arg0, U16 arg1, U16 arg2) {
				ICE_ASSERT(m_command+3 < s_commandCache + kMaxCommands);
				AddCommandU16(cmd);
				AddCommandU16(arg0);
				AddCommandU16(arg1);
				AddCommandU16(arg2);
			}
			//! Add a command with 4 arguments into the current command stream.
			void AddCommand(U16 cmd, U16 arg0, U16 arg1, U16 arg2, U16 arg3) {
				ICE_ASSERT(m_command+4 < s_commandCache + kMaxCommands);
				AddCommandU16(cmd);
				AddCommandU16(arg0);
				AddCommandU16(arg1);
				AddCommandU16(arg2);
				AddCommandU16(arg3);
			}
			//! Add a command with 5 arguments into the current command stream.
			void AddCommand(U16 cmd, U16 arg0, U16 arg1, U16 arg2, U16 arg3, U16 arg4) {
				ICE_ASSERT(m_command+5 < s_commandCache + kMaxCommands);
				AddCommandU16(cmd);
				AddCommandU16(arg0);
				AddCommandU16(arg1);
				AddCommandU16(arg2);
				AddCommandU16(arg3);
				AddCommandU16(arg4);
			}
			//! Add a command with 6 arguments into the current command stream.
			void AddCommand(U16 cmd, U16 arg0, U16 arg1, U16 arg2, U16 arg3, U16 arg4, U16 arg5) {
				ICE_ASSERT(m_command+6 < s_commandCache + kMaxCommands);
				AddCommandU16(cmd);
				AddCommandU16(arg0);
				AddCommandU16(arg1);
				AddCommandU16(arg2);
				AddCommandU16(arg3);
				AddCommandU16(arg4);
				AddCommandU16(arg5);
			}
			//! Add a command with 7 arguments into the current command stream.
			void AddCommand(U16 cmd, U16 arg0, U16 arg1, U16 arg2, U16 arg3, U16 arg4, U16 arg5, U16 arg6) {
				ICE_ASSERT(m_command+7 < s_commandCache + kMaxCommands);
				AddCommandU16(cmd);
				AddCommandU16(arg0);
				AddCommandU16(arg1);
				AddCommandU16(arg2);
				AddCommandU16(arg3);
				AddCommandU16(arg4);
				AddCommandU16(arg5);
				AddCommandU16(arg6);
			}
			//! Add a command with 8 arguments into the current command stream.
			void AddCommand(U16 cmd, U16 arg0, U16 arg1, U16 arg2, U16 arg3, U16 arg4, U16 arg5, U16 arg6, U16 arg7) {
				ICE_ASSERT(m_command+8 < s_commandCache + kMaxCommands);
				AddCommandU16(cmd);
				AddCommandU16(arg0);
				AddCommandU16(arg1);
				AddCommandU16(arg2);
				AddCommandU16(arg3);
				AddCommandU16(arg4);
				AddCommandU16(arg5);
				AddCommandU16(arg6);
				AddCommandU16(arg7);
			}

			//! End the command stream with an end command.
			void EndCommands();

			/*!	Creates a command to reserve DMA output buffer space for a following command.
			 *
			 *	Subsequent commands may use the returned location to output to the reserved space.
			 *	Note that the current interface requires all output from one command to be DMA'd
			 *	out to one location in main memory, as the output buffer may swap as necessary
			 *	between calls to CreateReserveOutputBufferCommand.
			 */
			Location CreateReserveOutputBufferCommand(void *pResultBuffer, U32 size);

			/*!	Loads a plugin and stores it at locPlugin in the work buffer.
			 *	locWorkBuffer will generally be a location returned by WorkBuffer::AllocHigh()
			 *	or WorkBuffer::AllocLow() with size plugin.GetLocalStoreSize() bytes.
			 *	Loading a plugin also causes the BSS section to be zeroed and all global
			 *	constructors to be called.
			 */
			void CreatePluginLoadCommand(Plugin const& plugin, Location locPlugin);

			/*!	Unloads the plugin at locPlugin in the work buffer.
			 *	While not strictly necessary (as it is unlikely the global destructors in your
			 *	code will have any important side effects), to be strictly correct, you should call 
			 *	this function before discarding the work buffer space used by each plugin.
			 *	Unloading a plugin causes all global destructors to be called.
			 */
			void CreatePluginUnloadCommand(Location locPlugin);

			/*!	Creates a command to execute the code at locCode with the given arguments. 
			 *	locPlugin will generally be a work buffer location previously filled with a call 
			 *  to CreateLoadPluginCommand().
			 *
			 *	Typical calling code might look something like:
			 *
			 *	// Create the plugin (once)
			 *	pMyContext->myPlugin = Plugin("myplugin.bin");	// load from a file "myplugin.bin"
			 *
			 *	// allocate space and load the plugin (before executing)
			 *	Location locPlugin = pWorkBuffer->AllocHigh(pMyContext->myPlugin.GetLocalStoreSize(), kWorkTypePlugInCode);
			 *	if (locPlugin == WorkBuffer::kLocationNull) { ICE_ASSERT(!"Out of work buffer space"); }
			 *	pCommandStream->CreatePluginLoadCommand(pMyContext->myPlugin, 		// the binary plugin code buffer to run 
			 *											locPlugin);					// Location in work buffer to load plugin
			 *
			 *	// execute the plugin (may happen multiple times)
			 *	Location locJointParams = pWorkBuffer->FindLocationByType(Ice::BatchJob::kWorkTypeJointParams);
			 *	PluginArgs pluginArgs;
			 *	pluginArgs.AddArgument( NonVolatileData( pMyContext->pInputData, pMyContext->inputDataSize ) );		// arg0 = locInputData:	some data to upload for use by the function call
			 *	pluginArgs.AddArgument( (U16)pMyContext->numData );													// arg1 = a count of how many elements we've uploaded in pInputData for processing, for instance
			 *	pluginArgs.AddArgument( locJointParams );															// arg2 = the location of the JointParams array in the work buffer for processing, for instance
			 *	pCommandStream->CreatePluginExecuteCommand(locPlugin, pluginArgs);
			 *
			 *	// unload the plugin and deallocate the space
			 *	pCommandStream->CreatePluginUnloadCommand(locPlugin);				// Location in work buffer to load plugin
			 *	pWorkBuffer->Free(locPlugin);
			 *
			 *	NOTE: in order to simulate executing a plugin in PPU batched mode, you must specify your
			 *	substitute function using plugin.SetPpuBatchModeFunction().
			 */
			void CreatePluginExecuteCommand(Location locPlugin, PluginArgs& pluginArgs);

			/*!	Creates a command to execute the code in the binary buffer pCode starting
			 *	at entry point offset codeEntry, with the given arguments.
			 *	The code will be executed directly in the input buffer and will not persist,
			 *	so binSize + bssSize + inputDataSize must be less than the size of the input buffer
			 *  minus kReserveInputBufferSpace bytes (24000 bytes for a 24K input buffer).
			 *
			 *	Typical calling code might look something like:
			 *
			 *	Location locJointParams = pWorkBuffer->FindLocationByType(Ice::BatchJob::kWorkTypeJointParams);
			 *	PluginArgs pluginArgs;
			 *	pluginArgs.AddArgument( NonVolatileData( pMyContext->pInputData, pMyContext->inputDataSize ) );		// arg0 = locInputData:	some data to upload for use by the function call
			 *	pluginArgs.AddArgument( (U16)pMyContext->numData );													// arg1 = a count of how many elements we've uploaded in pInputData for processing, for instance
			 *	pluginArgs.AddArgument( locJointParams );															// arg2 = the location of the JointParams array in the work buffer for processing, for instance
			 *	pCommandStream->CreatePluginInlineCommand(pMyContext->myPlugin, pluginArgs);
			 *
			 *	NOTE: in order to simulate executing a plugin in PPU batched mode, you must specify your
			 *	substitute function using plugin.SetPpuBatchModeFunction().
			 */
			void CreatePluginInlineCommand(Plugin const& plugin, PluginArgs& pluginArgs);
			/*!	Returns true if it is possible to fit the given plugin and input data into a
			 *	single input buffer for inline execution.
			 */
			bool CanCreatePluginInlineCommand(Plugin const& plugin, PluginArgs& pluginArgs) const;

			//! For internal use in debug printing the current command list position
			U16 DEBUG_GetCommandPos() const { return ((U8 const*)m_command - (U8 const*)s_commandCache); }

#if BATCHJOB_DMA_STATS
			struct DmaStats {
				U32 bytesSent;					//!< total bytes sent to SPU
				U32 bytesRecd;					//!< total bytes received from SPU
				U32 bytesSentBlocks;			//!< bytesSent, taking into account 128 byte block access
				U32 bytesRecdBlocks;			//!< bytesRecd, taking into account 128 byte block access
			};

			void DEBUG_ResetStats() { memset(&m_stats, 0, sizeof(DmaStats)); }
			DmaStats& DEBUG_GetStats() { return m_stats; }
			DmaStats const& DEBUG_GetStats() const { return m_stats; }
#endif

		protected:
			//! Add a U16 command or argument into the current command stream.
			inline void AddCommandU16(U16 cmd) { *m_command++ = cmd; }

			//! This is a support routine that link all of the pointers to commands and DMA lists,
			//! so that we end up with one final chain of inputs.
			//! @param isLast If this is the last in the chain.
			void WrapChain(bool isLast);

			//! Execute an input buffer swap
			void Swap();

			struct DmaHeader {
				U16 volatileLoc;
				U16 commandLoc;
				U16 pad0;
				U16 dmaHeaderLoc;
				U32 pad1;	// was dmaEaHi
				U32 dmaListSize;
			};
			struct DmaListElement {
				U32 size;
				U32 eaLo;
			};

			const BatchJobBufferDesc m_kBufferDesc;

			DmaHeader *m_initialDmaHeader;		//!< Pointer to the initial DMA header and list for the task created by this CommandStream
			DmaHeader *m_dmaHeader;				//!< Pointer to the current DMA-list header.
			DmaListElement *m_prevDmaListPatch;	//!< Pointer to the previous DMA-list element that must be chained to the current buffer or NULL.

			U16 *m_command;				//!< Pointer to the current command list entry.
			U32F m_inputSize;			//!< Current size of allocated space in input buffer.
			U32F m_volatileSize;		//!< Current size of allocated space for volatiles in input buffer.
			U32F m_inputSizePrev;		//!< Size of previous input buffer - so we can check when the size when we add the current DMA list.
			U32  m_prevDmaEndEaLo;		//!< Address of the end of data sent by the last call to AddDmaListElement

			MemoryBuffer *m_pBuffer;	//!< Pointer to a memory buffer to allocate command and DMA lists from.

#if BATCHJOB_DMA_STATS
			DmaStats m_stats;			//!< Stats collection, total bytes sent/recd
#endif
#if ICEDEBUG
			U16 const *m_command_reserve;	//!< Pointer to expected command list position at CheckReserve().
			U32F m_inputSize_reserve;		//!< Expected size of allocated space in input buffer at CheckReserve().
			U32F m_volatileSize_reserve;	//!< Expected size of allocated space for volatiles in input buffer at CheckReserve().
			DmaListElement const *m_dmaList_reserve;	//!< Pointer to expected end of dma list at CheckReserve().
#endif

			static U8 s_volatileCache[kMaxVolatileSize];	//<! Local cache to store volatile data for current dma
			static U16 s_commandCache[kMaxCommands];		//<! Local cache to build commands for current dma
		};

		//! Convert a 32 bit argument into two 16 bit arguments.
		inline U16 Arg32hword0( U32 arg ) { return (U16)(arg >> 16); }
		inline U16 Arg32hword1( U32 arg ) { return (U16)arg; }
		//! Convert a 32 bit float argument into two 16 bit arguments.
		inline U16 Arg32hword0( F32 arg )
		{
			union {
				I32 i;
				F32 f;
			} fi;
			fi.f = arg;
			return (U16)(fi.i >> 16);
		}
		inline U16 Arg32hword1( F32 arg )
		{
			union {
				I32 i;
				F32 f;
			} fi;
			fi.f = arg;
			return (U16)fi.i;
		}
		//! Convert a 64 bit argument into 4 16 bit arguments.
		inline U16 Arg64hword0( U64 arg ) { return (U16)(arg >> 48); }
		inline U16 Arg64hword1( U64 arg ) { return (U16)(arg >> 32); }
		inline U16 Arg64hword2( U64 arg ) { return (U16)(arg >> 16); }
		inline U16 Arg64hword3( U64 arg ) { return (U16)arg; }

	}
}

#endif
