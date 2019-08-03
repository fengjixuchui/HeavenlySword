//----------------------------------------------------------------------------------------
//! 
//! \filename exec\exec.h
//! 
//----------------------------------------------------------------------------------------
#if !defined( EXEC_EXEC_H )
#define EXEC_EXEC_H


#if defined( PLATFORM_PC )
#include "exec/exec_pc.h"
#elif defined( PLATFORM_PS3 )

#include "exec/PPU/exec_ps3.h"

#endif

#endif // end EXEC_EXEC_H
