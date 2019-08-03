/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_BATCHJOBCORE_H
#define ICE_BATCHJOBCORE_H

#include "icebatchjob.h"

/*!
 * \file icebatchjobcore.h
 * \brief Core functions for batch jobs implemented in SPU code.
 *
 * When animating on PPU, these functions are implemented by icebatchjobcore.cpp.
 * If PPU_IMMEDIATE_FUNCTIONS is defined to 1, the old style calling convention (with
 * separate arguments rather than standardized U16 array command list arguments) version of
 * the functions will be defined.  You will also have to #include the cpp file
 * into another cpp file with the PPU_IMMEDIATE_FUNCTIONS defined first, in order
 * to compile the immediate version of the functions.  This relies on C++ function
 * overloading to resolve which function call is intended.
 */

#if ICE_TARGET_PS3_SPU || !PPU_IMMEDIATE_FUNCTIONS
		// functions following the SPU/PPU dispatcher calling convention take arguments packed into a qword, plus a memory map qword for unpacking pointers:
# define DISPATCHER_FN(args) (Ice::BatchJob::DispatcherFunctionArgs param_qw0, Ice::BatchJob::DispatcherFunctionMemoryMap memoryMap)
#else
		// functions for PPU immediate mode use the original unpacked function arguments
# define DISPATCHER_FN(args) args
#endif

namespace Ice
{
    namespace BatchJob
    {
#if ICE_TARGET_PS3_SPU
		// Begin functions implemented in SPU code
		extern "C" {
#endif

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdPluginLoad.
		 * 
		 * On the SPU, PluginLoad executes the BatchJobPluginLoad() code at 
		 * (pPlugin + pPlugin->m_loadEntryPoint), which sets up the BSS section, calls
		 * all global constructors for the plugin, and sets g_batchJobFunctionTable to
		 * point at the batchjob command function table.
		 * 
		 * On the PPU, PluginLoad has nothing to do.
		 * 
		 * This function has no immediate mode equivalent.
		 */
#if	ICE_TARGET_PS3_SPU || !PPU_IMMEDIATE_FUNCTIONS
		void PluginLoad				DISPATCHER_FN((void	*pPlugin));			// pointer to a batchjob plugin buffer
#endif

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdPluginExecute.
		 * 
		 * On the SPU, PluginExecute executes the BatchJobPluginExecute() code at 
		 * (pPlugin + pPlugin->m_entryPoint) with the command line quadword at pArgs, following 
		 * the same calling convention as dispatcher functions.  
		 * BatchJobPluginExecute() is a wrapper around BatchJobPluginMain() which sets up
		 * $126 for position independent code before passing through its parameters.
		 * 
		 * On the PPU, PluginExecute instead executes the PPU function stored at
		 * pPlugin->m_pPpuBatchModeFunction.
		 * 
		 * This function has no immediate mode equivalent, as immediate mode can simply execute
		 * the code directly.
		 */
#if	ICE_TARGET_PS3_SPU || !PPU_IMMEDIATE_FUNCTIONS
		void PluginExecute			DISPATCHER_FN((void	*pPlugin,			// pointer to a batchjob plugin buffer
												   U16 const *pArgs));		// location of one VU16 of command line params
#endif

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdPluginUnload.
		 * 
		 * On the SPU, PluginUnload executes the BatchJobPluginUnload() code at 
		 * (pPlugin + pPlugin->m_unloadEntryPoint), which calls all global destructors
		 * for the plugin in preparation for discarding the plugin.
		 * 
		 * On the PPU, PluginUnload has nothing to do.
		 * 
		 * This function has no immediate mode equivalent.
		 */
#if	ICE_TARGET_PS3_SPU || !PPU_IMMEDIATE_FUNCTIONS
		void PluginUnload			DISPATCHER_FN((void	*pPlugin));			// pointer to a batchjob plugin buffer
#endif

		/**
		 * PPU debug implementation of the SPU task function by the same name, associated with batch command kCmdCopyQuadwords.
		 * 
		 * Copies quadword aligned data from pSource to pDest, with a resolution of one quadword -
		 * i.e. it is guaranteed not to overwrite data beyond size bytes, which must be a multiple
		 * of quadwords or 16 bytes.  CopyQuadwords copies in increasing address order.
		 */
		void CopyQuadwords			DISPATCHER_FN((U16 size,			// multiple of 16 bytes 
												   void const *pSource,	// qword aligned
												   void *pDest));		// qword aligned

#if ICE_TARGET_PS3_SPU
		}	// extern "C"
		// End functions implemented in SPU code
#endif
	}
};

#endif //ICE_BATCHJOBCORE_H
