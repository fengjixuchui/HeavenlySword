#ifndef CORE_SSE_H_
#define CORE_SSE_H_

#if defined( PLATFORM_PC )
#	include "core/sse_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "core/sse_emul_ps3.h"
#endif

#endif //CORE_SSE_H_
