#ifndef CORE_TIMER_H
#define CORE_TIMER_H

#if defined( PLATFORM_PC )
#	include "core/timer_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "core/timer_ps3.h"
#endif


#endif //CORE_TIMER_H
