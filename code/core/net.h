#ifndef CORE_NET_H_
#define CORE_NET_H_

#if defined( PLATFORM_PC )
#	include "core/net_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "core/net_ps3.h"
#endif

#endif //CORE_NET_H_
