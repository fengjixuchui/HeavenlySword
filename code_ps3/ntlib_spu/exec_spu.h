//---------------------------------------------------------------
//!
//! \file exec_spu.h
//!
//!	Helper functions for a subset of exec from an SPU.
//!
//! SUPER C:
//! This is a 'super' C implementation, it should be 
//! treated as C code using C++ style syntax and not 
//! true C++ code. In particular, ctors must not be 
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//! \see http://kong:8080/confluence/display/CH/SuperC
//!
//---------------------------------------------------------------

#ifndef	NTLIB_EXEC_SPU_H_
#define	NTLIB_EXEC_SPU_H_

#ifndef __SPU__
#	error This file can only be included in an SPU project.
#endif // !__SPU__

#include "exec/execspujobadder_ps3.h"

class Exec
{
public:
	//! runs a SPU task.
	//! the argument list is LS memory, that will be processed and DMA'ed to main memory automagically 
	//! for the next task to pick up and get your lovely parameters... All DMABuffers referenced must be valid
	//! EA address for the next task (fairly obviously but worth repeating)
	//! note spuargement list is modified but in a 'safe' way, so you can reuse the same arguement list
	static void RunTask( ExecSPUJobAdder* pTask, SPUArgumentList* pArgumentList, uint32_t dependency_counter_to_deprecate = 0 );

	//! Add a dependency job from the SPU. You need to pass up a DependencyCounter
	//! object pointer from main-memory and pass this EA to dependency_counter.
	static void AddBarrierJob( ExecSPUJobAdder* pParams, uint32_t dependency_counter );

	//! Initialise the dependency counter at the given main-mem EA to the given count.
	static void InitDependency( uint32_t dependency_counter_ea, uint16_t count, const ExecSPUJobAdder *jobAdder );

	//! this kicks off the new task, readyCount should be number of tasks or number of spus I think...
	static void JobApiSetReadyCount( uint64_t eaSpurs, CellSpursWorkloadId workloadId, uint32_t readyCount );

protected:

};

#endif
