/***************************************************************************************************
*
*	DESCRIPTION			NetEndpoint.cpp
*
*	NOTES				Implementation of the NetEndpoint class.  For low level comms across
*						an IP4 network.
*
***************************************************************************************************/

#ifndef _NET_END_PC_H
#define _NET_END_PC_H

// Required JAMNet Headers
#include "jamnet/NetAddr.h"
#include "jamnet/NetPacket.h"

/***************************************************************************************************
*
*	CLASS			NetEndpoint
*
*	DESCRIPTION		Implementing a network comms endpoint
*
***************************************************************************************************/
class NetEndpoint
{
public:
	NetEndpoint(NetAddr &obAddr);
	~NetEndpoint(void);

	bool		IsBound() {return m_bBound;}
	bool		Send(NetPacket &obPacket);
	NetPacket*	Recv();

// Implementation
private:
	bool m_bBound;
	NetAddr m_obAddr;

	// Socket Identifier
	SOCKET	 m_sock;

#ifdef NETSTATS
	#pragma message("Net stat gathering enabled.\n")
	// Raw byte Info
	unsigned long	m_iBytesSent;
	unsigned long	m_iBytesRecv;

	// Packet counts
	unsigned int	m_iPacketsSent;
	unsigned int	m_iPacketsRecv;
#endif
};

#endif //_NET_END_PC_H
