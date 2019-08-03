/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

/*!
 * \file icebatchjobcore.inl
 * \brief Contains PPU implementation of a batch job core built-in functions.
 *
 *	icebatchjobcore.inl expects to be included into a batch job dispatcher
 *	cpp file.  This structure allows the including file to control the
 *	preprocessor defines which control the compilation of this file, including:
 *	BATCHJOB_DISPATCHER_DPRINTS - if not 0, enable debug prints in the batch job
 *		through a SPU LS stack or PPU printf.
 *	BATCHJOB_DISPATCHER_ASSERTS - if not 0, enable debug assertions in the batch job
 *
 *	In addition, icebatchjobcore.inl and it's includes define several macros
 *	that may be useful for building additional built-in functions for the PPU.
 */

#include "icebatchjobcore.h"
#include "icebatchjobdebug.h"

// ========================================================================================================================
//
//								  !!!  Please do NOT mess with this file  !!!!!
//
//						 It is very important to keep it as close to the SPU code as possible,
//									even though the C++ might seem awkward!
//
// ========================================================================================================================

#if ICE_TARGET_PS3_SPU
# error	 icebatchjobcore.inl should not be compiled on the SPU
#else

# if	!PPU_IMMEDIATE_FUNCTIONS
#  define DISPATCHER_UNPACK_ARGS(code) code

static inline void* _LocToPtr(U16 loc, Ice::BatchJob::DispatcherFunctionMemoryMap memoryMap) { return (void*)(&memoryMap[((loc)>>2) & 0x3][(loc) & ~0xF]); }
#  define LocToPtr(loc) _LocToPtr(loc, memoryMap)

#  if ICE_COMPILER_GCC
#   define DISPATCHER_DPRINTF_COMMAND(format, args...) do {} while(0)
#  else
#   define DISPATCHER_DPRINTF_COMMAND	__noop
#   define DEBUG_PtrToLoc(p)	(0)
#  endif
# else
#  define DISPATCHER_UNPACK_ARGS(code)
#  if BATCHJOB_DISPATCHER_DPRINTS
#   if ICE_COMPILER_GCC
#    define DISPATCHER_DPRINTF_COMMAND(format, args...) do { ::printf(format, ## args); } while(0)
#   else
#    define DISPATCHER_DPRINTF_COMMAND	DISPATCHER_PRINTF
#   endif
#  else
#   if ICE_COMPILER_GCC
#    define DISPATCHER_DPRINTF_COMMAND(format, args...) do {} while(0)
#   else
#    define DISPATCHER_DPRINTF_COMMAND	__noop
#    define DEBUG_PtrToLoc(p)	(0)
#   endif
#  endif
# endif


namespace Ice {
	namespace BatchJob {
		// ===================================================================================================================
		//
		// This file comprises C++ code that closely follows the SPU code in ice/batchjob/spu/*.spu
		//
		// ===================================================================================================================

# if	!PPU_IMMEDIATE_FUNCTIONS
		void PluginLoad					DISPATCHER_FN((void	*/*pPlugin*/))			// pointer to a batchjob plugin buffer
		{
			//nothing to do
			DISPATCHER_UNPACK_ARGS(
				(void)param_qw0;
				(void)memoryMap;
			);
			DISPATCHER_DPRINTF_COMMAND("      PluginLoad(%04x)\n", param_qw0[0]);
		}

		void PluginExecute				DISPATCHER_FN((void *pPlugin,				// pointer to a batchjob plugin buffer
													   U16 const *pArgs))			// location of one VU16 of command line arguments
		{
			DISPATCHER_UNPACK_ARGS(
				void *pPlugin =			(void *)				LocToPtr(param_qw0[0]);
				U16 const *pArgs = 		(U16 const *)			LocToPtr(param_qw0[1]);
			)
			DispatcherFunction pFn = *(DispatcherFunction*)((U8*)pPlugin + 0x28);
			DISPATCHER_ASSERT(pFn != NULL);

			DISPATCHER_DPRINTF_COMMAND("      PluginExecute(%04x [%08x], %04x)\n", DEBUG_PtrToLoc(pPlugin), CONV_FROM_PTR(pFn), DEBUG_PtrToLoc(pArgs));
			(*pFn)(pArgs, memoryMap);
		}

		void PluginUnload				DISPATCHER_FN((void	*/*pPlugin*/))			// pointer to a batchjob plugin buffer
		{
			//nothing to do
			DISPATCHER_UNPACK_ARGS(
				(void)param_qw0;
				(void)memoryMap;
			);
			DISPATCHER_DPRINTF_COMMAND("      PluginUnload(%04x)\n", param_qw0[0]);
		}
#endif

		void CopyQuadwords				DISPATCHER_FN((U16 const size,
													   void const *const pSource,
													   void *const pDest))
		{
			DISPATCHER_UNPACK_ARGS(
				U16 size = param_qw0[0];
				void const *pSource = 	LocToPtr(param_qw0[1]);
				void *pDest = 			LocToPtr(param_qw0[2]);
			)
			DISPATCHER_DPRINTF_COMMAND("      CopyQuadwords(%04x, %04x -> %04x)\n", size, DEBUG_PtrToLoc(pSource), DEBUG_PtrToLoc(pDest));
			DISPATCHER_ASSERT(!(size & 0xF) && !(CONV_FROM_PTR(pDest) & 0xF) && !(CONV_FROM_PTR(pSource) & 0xF));

# if ICE_TARGET_PS3_PPU
			VU8 *pDst = (VU8*)pDest;
			VU8 const *pSrc = (VU8 const *)pSource;
			for (U32F sizeCopied = 0; sizeCopied < size; sizeCopied += sizeof(VU8)) {
				*pDst++ = *pSrc++;
			}
# else
			U64 *pDst = (U64*)pDest;
			U64 const *pSrc = (U64 const *)pSource;
			for (U32F sizeCopied = 0; sizeCopied < size; sizeCopied += sizeof(U64)) {
				*pDst++ = *pSrc++;
			}
# endif
		}
	}
}
#endif
