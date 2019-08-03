/***************************************************************************************************
*
*	DESCRIPTION			NetEndpoint.cpp
*
*	NOTES				Implementation of the NetEndpoint class.  For low level comms across
*						an IP4 network.
*
***************************************************************************************************/

#include "jamnet/netendpoint.h"

#include <sys/time.h>
#include <sys/select.h>

/***************************************************************************************************
*
*	FUNCTION		NetEndpoint::NetEndpoint
*
*	DESCRIPTION		Create a working endpoint.
*
***************************************************************************************************/
NetEndpoint::NetEndpoint(NetAddr &obAddr)
	: m_bBound(false), m_obAddr(obAddr)
{
	// Create our socket
	m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if(m_sock == INVALID_SOCKET)
	{
		// Couldn't construct our socket.
		#ifdef _WIN32
			/*LOG_ERROR*/ntPrintf(GetNetworkErrorString(WSAGetLastError()));
		#endif

		return;
	}

	// Bind to our address
	int iResult = bind(m_sock, (sockaddr*)&(*m_obAddr), sizeof(sockaddr_in));

	if(iResult == SOCKET_ERROR)
	{
		// Unable to bind the socket to this address
		#ifdef _WIN32
			/*LOG_ERROR*/ntPrintf(GetNetworkErrorString(WSAGetLastError()));
		#endif

		// Close the socket
		socketclose(m_sock);
		return;
	}

	#ifdef _NETSTATS
		m_iBytesSent = 0;
		m_iBytesRecv = 0;
		m_iPacketsSent = 0;
		m_iPacketsRecv = 0;
	#endif

	// Connection successful
	m_bBound = true;
}

/***************************************************************************************************
*
*	FUNCTION		NetEndpoint::~NetEndpoint
*
*	DESCRIPTION		Shutdown the endpoint.
*
***************************************************************************************************/
NetEndpoint::~NetEndpoint(void)
{
	if(!m_bBound)
		return;

	//TODO: Insure all outgoing data is sent

	// Close the socket
	socketclose(m_sock);

#ifdef _NETSTATS
	OutputNetStats();
#endif
}

/***************************************************************************************************
*
*	FUNCTION		NetEndpoint::Send
*
*	DESCRIPTION		Send a packet from the endpoint.  Ensure the packet has a destination!
*
***************************************************************************************************/
bool NetEndpoint::Send(NetPacket &obPacket)
{
	// Apply the timestamp
	obPacket.StampSent();

	int iResult = sendto(m_sock, (char*)obPacket.GetData()-NetPacket::HEADERSIZE, obPacket.GetTotalSize(), 
						 0, (sockaddr*)&(*obPacket.GetAddress()), sizeof(sockaddr_in));

	char* pData = (char*)obPacket.GetData();
	pData = (char*)obPacket.GetData()-NetPacket::HEADERSIZE;
	pData = (char*)(obPacket.GetData()-NetPacket::HEADERSIZE);
	pData = ((char*)obPacket.GetData())-NetPacket::HEADERSIZE;

	if(iResult == SOCKET_ERROR)
	{
		// Couldn't send...
		// A success here doesn't mean it's all ok... Might receive a failure message
		// on this connection at a later recv.
		#ifdef _WIN32
			/*LOG_ERROR*/ntPrintf(GetNetworkErrorString(WSAGetLastError()));
		#endif

		return false;
	}

	// Update the netstats
	#ifdef _NETSTATS
		m_iBytesSent += iResult;
		m_iPacketsSent++;
	#endif

	// Send successful
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		NetEndpoint::Recv
*
*	DESCRIPTION		Receives a NetPacket from the endpoint.  There may be more waiting...
*
***************************************************************************************************/
NetPacket*	NetEndpoint::Recv()
{
#ifndef PLATFORM_PS3 //FIXME_WIL select removed in 050
	// Check for read data
	fd_set sRecv;

	FD_ZERO(&sRecv);
	FD_SET(m_sock, &sRecv);

	static timeval tv = {0, 0};

	if (select(0, &sRecv, 0, 0, &tv)>1)
		return 0;	// Nothing to read

	// We've got some data.
	// Gen us a packet:-
	NetPacket::CPrivateData* pobData;
	NetPacket *pobPacket = NetPacket::Create(pobData);
	socklen_t	addrLen = sizeof(sockaddr_in);

	pobData->iLen = recvfrom(	m_sock, (char*)pobData->pData, NetPacket::MAXSIZE, 0, 
								(sockaddr*)&(*pobPacket->GetAddress()), &addrLen);

	pobPacket->StampRecv();

	if(pobData->iLen == SOCKET_ERROR)
	{
	#ifdef _WIN32
			// Handle Error here
			int iErr = WSAGetLastError();

			if(iErr == WSAECONNRESET)
			{
				// The remote endpoint isn't listening to us anymore
				pobPacket->SetError(NETERROR_CONNRESET);
				return pobPacket;
			}
			else
			{
				/*LOG_ERROR*/ntPrintf(GetNetworkErrorString(iErr));
			}
	#endif

		NT_DELETE( pobPacket );
		return 0;
	}

	// Account for packet header
	pobData->iLen -= NetPacket::HEADERSIZE;

	// No Data!
	if(pobPacket->GetDataSize() <= 0)
	{
		NT_DELETE( pobPacket );
		return 0;
	}

	// Update the netstats
	#ifdef _NETSTATS
		m_iBytesRecv += pobPacket->GetLength();
		m_iPacketsRecv++;
	#endif

	// Return our packet
	return pobPacket;
#endif
	return 0;
}
