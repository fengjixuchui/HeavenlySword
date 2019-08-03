//----------------------------------------------------------------------------------------
//! 
//! \filename exec\execspujobadder_ps3.h
//! 
//----------------------------------------------------------------------------------------
#if !defined( EXEC_EXECSPUJOBADDER_PS3_H )
#define EXEC_EXECSPUJOBADDER_PS3_H

#include <cell/spurs/types.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//The PPU will write out a params structure in this format for this job
//then an spu job can use this to start more jobs off, there are lots of 
//data fields you can pack some small stuff/parameters in. Note the data fields
//are not used by exec
//The SPU job code will expect to read in a params structure in this format.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct ExecSPUJobAdder
{
	uint32_t				m_eaCurrentSPUCommandListIndex;	// shared command list pool memory index (in bytes)
	uint32_t				m_eaSPUCommandListBuffer;		// start of the command list buffer in EA
	uint32_t				m_eaCurrentArgumentListIndex;	// shared spu arguement index variable (in SPUArguementList)
	uint32_t				m_eaArgumentListSpace;			// start of the arguement lits in EA

	uint32_t				m_eaSpuModule;
	uint32_t				m_spuModuleFileSize;
	uint32_t				m_spuModuleRequiredBufferSizeInPages;
	uint32_t				m_eaJobList;

	uint32_t				m_eaSpurs;
	CellSpursWorkloadId		m_workloadId;
	uint32_t				m_NumWWSJobManagerSPUs;
	uint32_t				m_data0[5+16]; // make it 128 byte aligned
}  __attribute__ ((aligned( 128 )));


#endif
