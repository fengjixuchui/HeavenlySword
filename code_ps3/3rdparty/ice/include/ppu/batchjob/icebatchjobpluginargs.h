/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_BATCHJOBPLUGINARGS_H
#define ICE_BATCHJOBPLUGINARGS_H

#include "icebatchjob.h"

namespace Ice {
	namespace BatchJob {
		class CommandStream;

		/// Type for holding an input data argument, the start of which in the input buffer
		/// will be passed as a 2-byte Location argument in the argument list.
		/// It is assumed that the data pointed to by m_pData will persist unchanged through
		/// the execution of the batched task.
		///NOTE: See the comments on AddArgument(NonVolatileData&) about data alignment and padding.
		struct NonVolatileData {
			void const*			m_pData;
			U32 				m_size;
			NonVolatileData() {}
			NonVolatileData(void const* pData, U32 size) : m_pData(pData), m_size(size) {}
		};
		/// Type for holding an input data argument, the start of which in the input buffer
		/// will be passed as a 2-byte Location argument in the argument list.
		/// It is assumed that the data pointed to by m_pData will only persist until the
		/// call to UploadArgs(cmdStream) (which is called by cmdStream.CreatePluginExecuteCommand
		/// or cmdStream.CreatePluginInlineCommand), when a copy of m_pData will be made
		/// to store its value through the execution of the batched task.
		/// This allows the sending of data on the stack, or of data whose value may change
		/// before the batch job finishes executing.
		///NOTE: See the comments on AddArgument(VolatileData&) about data alignment and padding.
		struct VolatileData {
			void const*			m_pData;
			U32 				m_size;
			VolatileData() {}
			VolatileData(void const* pData, U32 size) : m_pData(pData), m_size(size) {}
		};
		/// Type for holding an output data argument, the start of which in the output buffer
		/// will be passed as a 2-byte Location argument in the argument list.
		/// It is assumed that the space pointed to by m_pData will persist to be filled by
		/// the task during its execution.
		///NOTE: See the comments on AddArgument(OutputData&) about data alignment, padding,
		/// and multiple OutputData per command.
		struct OutputData {
			void*				m_pData;
			U32					m_size;
			OutputData() {}
			OutputData(void* pData, U32 size) : m_pData(pData), m_size(size) {}
		};

		/*!	PluginArgs encapsulates the construction of an U16 argument list for
		 *	a call to execute a BatchJobPlugin, in order to hide the internal complexity
		 *	of reserving input buffer space for the command in a CommandStream.
		 *
		 *	Typical usage might look something like:
		 *
		 *	PluginArgs args;
		 *	args.AddArgument( (U16) numData );		// arg[0] is a U16 containing a count of data (starting at hword 0)
		 *	NonVolatileData inputData[2] = {
		 *		NonVolatileData( pData0, dataSize0 ),
		 *		NonVolatileData( pData1, dataSize1 ),
		 *	};
		 *	args.AddArgument( inputData, 2 );		// arg[1] is a U16 Location of the start of input data (starting at hword 1),
		 *											// which is uploaded as 2 DMA elements of total size dataSize0+dataSize1
		 *	args.AddArgument( (float)0.75f );		// arg[2] is a 4-byte float (starting at hword 2)
		 *	args.AddArgument( locWorkData );		// arg[3] is a U16 Location of some data to work on in the work buffer (starting at hword 4)
		 *
		 *	cmdStream.CreatePluginExecuteCommand(locPlugin, args);	// call the plugin with these arguments
		 */
		class PluginArgs
		{
			static const unsigned kMaxArgHwords = 8;		//!< Maximum size of argument list in hwords; must be a multiple of 8
			static const unsigned kMaxDataElements = 64;	//!< Maximum number of DMA or Volatile elements sent by one argument list

			/// DataElement holds data for a call to either cmdStream.AddDmaListElement() or cmdStream.AddVolatile()
			/// until we know the total input buffer size required to send our commands and can resolve our argument list.
			struct DataElement {
				void const*			m_pData;
				union {
					struct {
						U32					m_size:18;
						I32					m_argPos:5;
						U32					m_bVolatile:1;
						U32					m_bOutput:1;
						U32					z_unused:7;
					};
					U32				m_sizeAndFlags;
				};
			};

			U16	m_argData[kMaxArgHwords];					//!< our U16 argument list 
			unsigned m_argDataHwords;						//!< number of U16s used from our argument list
			unsigned m_numDmaElements;						//!< number of NonVolatileData elements attached to our argument list
			unsigned m_numOutputDmaElements;				//!< number of OutputData elements attached to our argument list
			U32 m_dmaSize;									//!< total size of NonVolatileData elements attached to our argument list
			U32 m_volatileSize;								//!< total size of VolatileData elements attached to our argument list

			unsigned m_numDataElements;						//!< total number of DataElement's attached to our argument list
			DataElement m_dataList[kMaxDataElements];		//!< storage for DataElement's attached to our argument list

		public:
			/// Construct an empty argument list
			PluginArgs();
			/// Restore this PluginArgs to a pristine state
			void Reset();

			/// Add a 2-byte unsigned integer argument to this argument list, and return the hword position of the argument or -1 if out of space
			int AddArgument( U16 argU16 );
			/// Add a 2-byte signed integer argument to this argument list, and return the hword position of the argument or -1 if out of space
			int AddArgument( I16 argI16 ) { return AddArgument((U16)argI16); }
			/// Add a 4-byte unsigned integer argument to this argument list, and return the hword position of the argument or -1 if out of space
			int AddArgument( U32 argU32 );
			/// Add a 4-byte signed integer argument to this argument list, and return the hword position of the argument or -1 if out of space
			int AddArgument( I32 argI32 ) { return AddArgument((U32)argI32); }
			/// Add an 8-byte signed integer argument to this argument list, and return the hword position of the argument or -1 if out of space
			int AddArgument( U64 argU64 );
			/// Add an 8-byte signed integer argument to this argument list, and return the hword position of the argument or -1 if out of space
			int AddArgument( I64 argI64 ) { return AddArgument((U64)argI64); }
			/// Add a 4-byte float argument to this argument list, and return the hword position of the argument or -1 if out of space
			int AddArgument( F32 argF32 );
			/// Add an 8-byte double argument to this argument list, and return the hword position of the argument or -1 if out of space
			int AddArgument( F64 argF64 );
			/// Schedule a DMA to send the given data buffer, add a 2-byte Location of the DMA'd data to the argument list,
			/// and return the hword position of the argument or -1 if out of space or invalid argument.
			///NOTE: While the data pointed to by nonVolatileData.m_pData must persist through execution of the batch job, 
			/// nonVolatileData itself is not required to persist beyond this function call.
			///NOTE: NonVolatile pData must be aligned based on its size, and at some sizes, will
			///	DMA some bytes past the end of pData.	The overall size in the local store after
			///	DMA will always be padded out with zeros to a multiple of 16 bytes.
			///		Size	Align		Note
			/// 	----	-----		---------------------------------
			///		   1	    1		LS layout: 000D 0000 0000 0000 (prefered byte)
			///		   2	    2		LS layout: 00DD	0000 0000 0000 (prefered hword)
			///		   3	    4		LS layout: DDDx 0000 0000 0000 (x = read beyond end of pData!)
			///		   4	    4		LS layout: DDDD 0000 0000 0000 (prefered word)
			///		 5-7	    8		LS layout: DDDD Dxxx 0000 0000 (x = 1-3 bytes read beyond end of pData!)
			///		   8	    8		LS layout: DDDD DDDD 0000 0000 (prefered dword)
			///		  >8	   16		LS layout: DDDD DDDD DDDx xxxx (if Size is not 16 byte aligned, x = 0xF-(S&0xF) bytes read beyond end of pData!)
			/// Due to the complexity of these rules, it is generally safer to always send data
			/// that is 16 byte aligned in position and size, or possibly data that is 4 or
			/// 8 bytes in size and 4 or 8 byte aligned.
			int AddArgument( NonVolatileData const& nonVolatileData );
			/// Schedule volatile DMA to send a copy of the given data buffer, add a 2-byte Location of the DMA'd data to the argument list,
			/// and return the hword position of the argument or -1 if out of space or invalid argument.
			///NOTE: While the data pointed to by volatileData.m_pData must persist until the call to UploadArgs, 
			/// volatileData itself is not required to persist beyond this function call.
			///NOTE: Volatile data is not required to be aligned in position or size, but will
			///	always be padded out to a multiple of 16 bytes in size with zeroes.  The resulting
			/// position of the data in the local store for any size will always match that of 
			/// NonVolatileData, but any bytes that would have been reads beyond the end of pData
			/// are guaranteed to be zeroed instead.
			int AddArgument( VolatileData const& volatileData );
			/// Schedule output DMA to receive a copy of the given data buffer, add a 2-byte Location of the DMA'd data to the argument list,
			/// and return the hword position of the argument or -1 if out of space or invalid argument.
			///NOTE: While the data pointed to by outputData.m_pData must persist through execution of the batch job, 
			/// outputData itself is not required to persist beyond this function call.
			///NOTE: It is currently dangerous for one command to use multiple OutputData buffers, as
			/// any ReserveOutputBuffer command may force the flushing of previous output buffers in 
			/// order to make room.  For this reason, AddArgument(OutputData&) will fail if another 
			/// OutputData has already been added, and we do not provide an AddArgument(OutputData*, unsinged).
			///NOTE: OutputData pData must be aligned based on its size using the same rules as NonVolatileData.
			int AddArgument( OutputData const& outputData );
			/// Schedule multiple DMAs to send the given array of data buffers, add a 2-byte Location of the start of the DMA'd data 
			/// to the argument list, and return the hword position of the argument or -1 if out of space or invalid argument.
			///NOTE: While the data pointed to by the elements of pNonVolatileDataList must persist through execution of the batch job, 
			/// the list itself is not required to persist beyond this function call.
			///NOTE: See the comments on AddArgument(NonVolatileData&) about data alignment and padding.
			int AddArgument( NonVolatileData const* pNonVolatileDataList, unsigned count );
			/// Schedule volatile DMA to send copies of the given array of data buffers, add a 2-byte Location of the start of the DMA'd data 
			/// to the argument list, and return the hword position of the argument or -1 if out of space or invalid argument.
			///NOTE: While the data pointed to by the elements of pVolatileDataList must persist until the call to UploadArgs, the list 
			/// itself is not required to persist beyond this function call.
			///NOTE: See the comments on AddArgument(VolatileData&) about data alignment and padding.
			int AddArgument( VolatileData const* pVolatileDataList, unsigned count );

			/// Returns the number of dma elements required to send this argument list
			unsigned GetNumDmaElements() const { return m_numDmaElements; }
			/// Returns the number of output dma elements required by this argument list
			unsigned GetNumOutputDmaElements() const { return m_numOutputDmaElements; }
			/// Returns the total size of dma elements required to send this argument list
			U32 GetDmaSize() const { return m_dmaSize; }
			/// Returns the total size of volatiles required to send this argument list
			U32 GetVolatileSize() const { return m_volatileSize + (m_argDataHwords ? 0x10 : 0); }
			/// Returns the total size of data required to send this argument list
			U32 GetTotalSize() const { return GetDmaSize() + GetVolatileSize(); }

			/// Adds commands and data to the command stream to send all input data and
			/// the final resulting argument list.
			/// Returns the Location of the argument list.
			Location UploadArgs(CommandStream &cmdStream);
		};

	}
}

#endif
