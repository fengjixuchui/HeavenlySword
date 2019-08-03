/***************************************************************************************************
*
*	DESCRIPTION				NetAddr.cpp
*
*	NOTES					Not IPv6 compatible
*
***************************************************************************************************/

#ifndef _NET_ADDR_H
#define _NET_ADDR_H

#if defined( PLATFORM_PC )
#	include "jamnet/netaddr_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "jamnet/netaddr_ps3.h"
#endif 

#endif // _NET_ADDR_H
