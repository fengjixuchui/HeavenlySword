/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_BATCHJOB_H
#define ICE_BATCHJOB_H

#include "icebase.h"
#include "icebatchjobbuffers.h"

#define BATCHJOB_DPRINTS 0
#define BATCHJOB_DMA_STATS 1

#if ICE_TARGET_PS3_SPU
# include <spu_intrinsics.h>
#endif

#if !ICE_TARGET_PS3_SPU && BATCHJOB_DPRINTS
# if ICE_COMPILER_GCC
# define BATCHJOB_DPRINTF(format, args...) do { ::printf(format, ## args); } while(0)
# else
#  define BATCHJOB_DPRINTF ::printf
# endif
#else
# if ICE_COMPILER_GCC
#  define BATCHJOB_DPRINTF(format, args...) do {} while(0)
# else
#  define BATCHJOB_DPRINTF __noop
# endif
#endif

#ifndef CONV_FROM_PTR
# define CONV_FROM_PTR(p) 	((U32)(p))
#endif
#ifndef CONV_TO_PTR
# define CONV_TO_PTR(p)		((void*)(U32)(p))
#endif

/*! 
 * \file 	icebatchjob.h
 * \brief	Data structures and constants used by batch jobs.
 */
namespace Ice
{
    namespace BatchJob
    {
		/// Maximum size in bytes that one DMA element can send
		static const U32 kDmaMaxSize = 0x4000;

		//----------------------------------------------------------------------------------------

		/*!
		 * Low level batch commands list
		 */
		enum BatchCoreCommand
		{
			// core commands - required by low level CommandStream interface; some implemented directly within dispatcher
			/*00*/ kCmdEnd = 0,					//!< terminates a command list
			/*01*/ kCmdSwap,					//!< marks an input buffer swap position in a command list
			/*02*/ kCmdReserveOutputBuffer,		//!< requests space in an output buffer to DMA out data
			/*03*/ kCmdPluginLoad,				//!< initializes a plugin that was just loaded
			/*04*/ kCmdPluginExecute,			//!< executes an initialized plugin with arguments
			/*05*/ kCmdPluginUnload,			//!< deinitializes a plugin before it is discarded
			/*06*/ kCmdCopyQuadwords,			//!< copies a sequential array of quadwords between two unaliased positions

			kNumCoreCommands,
		};

		/*!
		 * Low level batch commands size list, containing the number of command stream U16's
		 * written by each command, or 1 (for the command) plus the number of arguments.
		 */
		enum BatchCoreCommandSize
		{
			// core commands
			kCmdSizeEnd = 						1,	//!< End()
			kCmdSizeSwap =						1,	//!< Swap()
			kCmdSizeReserveOutputBuffer =		4,	//!< ReserveOutputBuffer( U16 size, void* pMainMemoryAddress )
			kCmdSizePluginLoad =				2,	//!< PluginLoad( Location locPlugin )
			kCmdSizePluginExecute =				3,	//!< PluginLoad( Location locPlugin, Location locArgs )
			kCmdSizePluginUnload =				2,	//!< PluginLoad( Location locPlugin )
			kCmdSizeCopyQuadwords =				4,	//!< CopyQuadwords( U16 size, Location locSrc, Location locDst )
		};

		//----------------------------------------------------------------------------------------

#if ICE_TARGET_PS3_SPU
		typedef VU16 DispatcherFunctionArgs;
		typedef VU32 DispatcherFunctionMemoryMap;
#else
		typedef U16 const* DispatcherFunctionArgs;
		typedef U8 const*const* DispatcherFunctionMemoryMap;

#endif
		/**
		 * All internal command functions and plugin callback functions implement the following prototype:
		 * on the SPU: up to 8 U16 arguments are packed into a qword in 'params', and 4 memory map pointers are packed into a qword in 'memoryMap'.
		 * on the PPU: up to 8 U16 arguments are passed as a U16 array, and 4 memory map pointers are passed as a pointer array.
		 */
		typedef void (*DispatcherFunction)(DispatcherFunctionArgs params, DispatcherFunctionMemoryMap memoryMap);

		/// Helper function to convert a Location parameter into a pointer, given the current memoryMap
		inline void* LocationToPointer(Location param, DispatcherFunctionMemoryMap memoryMap)
		{
#if ICE_TARGET_PS3_SPU
			return (void*)( spu_extract( spu_rlqwbyte(memoryMap, param), 0) + ((U32)param &~0xF) );
#else
			return (void*)( memoryMap[(param & 0xC)>>2] + ((U32)param &~0xF) );
#endif
		}

		// Helper functions to extract parameters from a DispatcherFunctionArgs list
		/// Extract an unsigned hword parameter at params[iParam]
		inline U16 ExtractParamU16(DispatcherFunctionArgs params, unsigned iParam)
		{
#if ICE_TARGET_PS3_SPU
			return spu_extract( params, iParam );
#else
			return params[iParam];
#endif
		}
		/// Extract a signed hword parameter at params[iParam]
		inline I16 ExtractParamI16(DispatcherFunctionArgs params, unsigned iParam) { return (I16)ExtractParamU16(params, iParam); }
		/// Extract an unsigned word parameter at params[iParam0 ... iParam0+1]
		inline U32 ExtractParamU32(DispatcherFunctionArgs params, unsigned iParam0)
		{
#if ICE_TARGET_PS3_SPU
			return spu_extract( (VU32)spu_rlqwbyte((VU8)params, iParam0*2), 0 );
#else
			return ((U32)params[iParam0] << 16) | (U32)params[iParam0+1];
#endif
		}
		/// Extract a signed word parameter at params[iParam0 ... iParam0+1]
		inline I32 ExtractParamI32(DispatcherFunctionArgs params, unsigned iParam0) { return (I32)ExtractParamU32(params, iParam0); }
		/// Extract an unsigned dword parameter at params[iParam0 ... iParam0+3]
		inline U64 ExtractParamU64(DispatcherFunctionArgs params, unsigned iParam0)
		{
#if ICE_TARGET_PS3_SPU
			return spu_extract( (VU64)spu_rlqwbyte((VU8)params, iParam0*2), 0 );
#else
			return ((U64)params[iParam0] << 48) | ((U64)params[iParam0+1] << 32) | ((U64)params[iParam0+2] << 16) | (U64)params[iParam0+3];
#endif
		}
		/// Extract a signed dword parameter at params[iParam0 ... iParam0+3]
		inline I64 ExtractParamI64(DispatcherFunctionArgs params, unsigned iParam0) { return (I64)ExtractParamU64(params, iParam0); }
		/// Extract a pointer parameter from the Location at params[iParam], given the current memoryMap
		inline void* ExtractParamPointer(DispatcherFunctionArgs params, unsigned iParam, DispatcherFunctionMemoryMap memoryMap) { return LocationToPointer( (Location)ExtractParamU16(params, iParam), memoryMap ); }
		/// Extract a single-precision float parameter at params[iParam0 ... iParam0+1]
		inline F32 ExtractParamF32(DispatcherFunctionArgs params, unsigned iParam0)
		{
			union {
				U32 i;
				F32 f;
			} fi;
			fi.i = ExtractParamU32(params, iParam0);
			return fi.f;
		}
		/// Extract a double-precision float parameter at params[iParam0 ... iParam0+3]
		inline F64 ExtractParamF64(DispatcherFunctionArgs params, unsigned iParam0)
		{
			union {
				U64 i;
				F64 f;
			} fi;
			fi.i = ExtractParamU64(params, iParam0);
			return fi.f;
		}

		//----------------------------------------------------------------------------------------

		/**
		 * List of possible job manager audit types
		 */
		enum
		{
			kAuditStart = 0,
			kAuditInitStart,
			kAuditInitEnd,
			kAuditCmdStart,
			kAuditCmd,
			kAuditInputUseBufferStart,
			kAuditInputUseBufferEnd,
			kAuditInputDmaStart,
			kAuditInputDmaEnd,
			kAuditInputFreeBufferStart,
			kAuditInputFreeBufferEnd,
			kAuditOutputUseBufferStart,
			kAuditOutputUseBufferEnd,
			kAuditOutputDmaStart,
			kAuditOutputDmaEnd,
			kAuditEnd,
			kAuditNull,
		};
	}
};

#endif //ICE_BATCHJOB_H
