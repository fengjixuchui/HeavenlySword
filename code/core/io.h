#ifndef CORE_IO_H_
#define CORE_IO_H_

#if defined( PLATFORM_PC )
#	include "core/io_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "core/io_ps3.h"
#endif

#endif //_IO_H_
