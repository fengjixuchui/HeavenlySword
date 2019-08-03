/***************************************************************************************************
*
*	DESCRIPTION			NetEndpoint.cpp
*
*	NOTES				Implementation of the NetEndpoint class.  For low level comms across
*						an IP4 network.
*
***************************************************************************************************/

#ifndef _NET_END_H
#define _NET_END_H

#if defined( PLATFORM_PC )
#	include "jamnet/netendpoint_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "jamnet/netendpoint_ps3.h"
#endif 

#endif //_NET_END_H
