/***************************************************************************************************
*
*	DESCRIPTION		NetMan.cpp
*
*	NOTES			Definition of the network manager class
*
***************************************************************************************************/

#include "jamnet/netman.h"
#include "jamnet/netsynchronisationman.h"
#include "jamnet/netendpoint.h"
#include "core/timer.h"

//////////////////////////////////////////////////////
// Register our Network Messages
//////////////////////////////////////////////////////
REGISTER_NETMSG(NetMsgAck);
REGISTER_NETMSG(NetMsgSync);
REGISTER_NETMSG(NetMsgSyncAck);

////////////////////////////////////////////////////////////////////////
// Statics
////////////////////////////////////////////////////////////////////////
bool						NetMan::m_bRunning		= false;
NetEndpoint*				NetMan::m_pobEndpoint	= 0;
CSafeRef<NetClient>		NetMan::m_pobServer(NULL_REF);
NetClient*					NetMan::m_apobClients[NetMan::MAX_CLIENTS];

////////////////////////////////////////////////////////////////////////
// Debug Statistics
////////////////////////////////////////////////////////////////////////
#ifdef _NETSTATS
	unsigned long NetMan::m_iMsgsSent		 = 0;
	unsigned long NetMan::m_iMsgsSentUrgent = 0;
	unsigned long NetMan::m_iMsgsRecv		 = 0;
#endif


/***************************************************************************************************
*
*	FUNCTION		NetMan::Init
*
*	DESCRIPTION		Start the networking services
*
***************************************************************************************************/
bool NetMan::Init(short iPort)
{
	if(m_bRunning)
		return true;

	NT_NEW NetSynchronisationMan;

#ifdef _WIN32
	WSADATA obWSAData;
	int iErr = WSAStartup(MAKEWORD(2,0), &obWSAData);

	if(iErr)
	{
		return false;
	}

	if(LOBYTE(obWSAData.wVersion) != 2)
	{
		// Not supporting wsock version 2
		WSACleanup();
		return false;
	}

	// Create our endpoint
	NetAddr obAddr(INADDR_ANY, iPort);
	m_pobEndpoint = NT_NEW NetEndpoint(obAddr);

	// Did it create ok?
	//ntAssert(m_pobEndPoint);
	if(!m_pobEndpoint)
	{
		WSACleanup();
		return false;
	}
#endif

#ifdef _NETSTATS
	m_iMsgsSent = 0;
	m_iMsgsSentUrgent = 0;
	m_iMsgsRecv = 0;
#endif

	// We've successfully initialised
	m_bRunning = true;
	return true;

}


/***************************************************************************************************
*
*	FUNCTION		NetMan::CleanUp
*
*	DESCRIPTION		Shutdown the networking services and clean up the system.
*
***************************************************************************************************/
void NetMan::CleanUp()
{
	if(!m_bRunning)
		return;

	NetSynchronisationMan::Kill();

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apobClients[i])
		{
			NT_DELETE( m_apobClients[i] );
			m_apobClients[i] = 0;
		}
	}

	m_pobServer == NULL_CLIENT;

	NT_DELETE( m_pobEndpoint );
	m_pobEndpoint = 0;

#ifdef _WIN32
	WSACleanup();
#endif
}


/***************************************************************************************************
*
*	FUNCTION		NetMan::Update
*
*	DESCRIPTION		Do the net update frame.  Receive any waiting packets, decode and despatch the
*					messages contained within.  Send any waiting packets.
*
***************************************************************************************************/
void NetMan::Update()
{
	ntAssert(m_bRunning);

	//
	// Receive
	// -------
	for(NetPacket *pobPacket = m_pobEndpoint->Recv();	pobPacket; pobPacket = m_pobEndpoint->Recv())
	{
		// Handle any errors
		if(pobPacket->GetError())
		{
			if(pobPacket->GetError() == NETERROR_CONNRESET)
			{
				ntPrintf("Conn Reset\n");
				RemoveClient(pobPacket->GetAddress());
			}

			NT_DELETE( pobPacket );
			continue;
		}

		// Decode and Despatch all the messages in the packet
		DecodePacket(*pobPacket);
		NT_DELETE( pobPacket );
	}


	//
	// Send
	// ----
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		NetClient* pobClient = m_apobClients[i];
		
		if(pobClient)
			pobClient->Update();
	}

	NetSynchronisationMan::Get().Update();
}


/***************************************************************************************************
*
*	FUNCTION		NetMan::AddClient
*
*	DESCRIPTION		Create a new client and add it to our client list.
*
*	RETURN VALUE	Returns a const pointer to the NetClient object.
*
***************************************************************************************************/
const CSafeRef<NetClient> NetMan::AddClient(const NetAddr& obAddr)
{
	// Check they're not actually already a client...
	CSafeRef<NetClient> obClient = FindClient(obAddr);

	if(!obClient.IsNull())
		return obClient;

	// No? Well try to find a free spot
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(!m_apobClients[i])
		{
			m_apobClients[i] = NT_NEW NetClient(obAddr, i);
			NetSynchronisationMan::Get().AddClient(m_apobClients[i]->MakeHandle());
			return m_apobClients[i]->MakeHandle();
		}

	// No free spaces
	return CSafeRef<NetClient>(NULL_REF);

	/*
	NetClient* pobRetVal = NT_NEW NetClient(obAddr);
	m_obClientList.push_back(pobRetVal);
	return pobRetVal->MakeHandle();*/
}


/***************************************************************************************************
*
*	FUNCTION		NetMan::FindClient
*
*	DESCRIPTION		Find a client with the specified NetAddr in our client list.
*
***************************************************************************************************/
const CSafeRef<NetClient> NetMan::FindClient(const NetAddr& obAddr)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apobClients[i] && m_apobClients[i]->GetAddress() == obAddr)
			return m_apobClients[i]->MakeHandle();
	}
	

	return CSafeRef<NetClient>(NULL_REF);
}


/***************************************************************************************************
*
*	FUNCTION		NetMan::RemoveClient
*
*	DESCRIPTION		Remove a client from the session
*
*	INPUTS			NetClient& obClient - Client to be removed from our session
*
***************************************************************************************************/
void NetMan::RemoveClient(const CSafeRef<NetClient> obClient)
{
	ntPrintf(" ### NET ### RemoveClient - Write this function\n");

}


/***************************************************************************************************
*
*	FUNCTION		NetMan::RemoveClient
*
*	DESCRIPTION		Remove a client from the session
*
*	INPUTS			NetAddr& obAddr - Address of client to be removed
*
***************************************************************************************************/
void NetMan::RemoveClient(const NetAddr& obAddr)
{	
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apobClients[i] && m_apobClients[i]->GetAddress() == obAddr)
		{
			bool bServ = m_pobServer.Valid();

			NT_DELETE( m_apobClients[i] );
			m_apobClients[i] = 0;

			if(bServ && !m_pobServer.Valid())
			{
				ntPrintf(" ### NET ### - Server down...");
				// Now handle this gracefully, elect a new server, drop out to the front end... etc...
				exit(0);
			}
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		NetMan::SetServer
*
*	DESCRIPTION		Specifies the network address of this sessions server.
*
*	INPUTS			NetAddr& obAddr - Address of the server.
*
***************************************************************************************************/
const CSafeRef<NetClient> NetMan::SetServer(const NetAddr &obAddr)
{
	m_pobServer = AddClient(obAddr);

	return m_pobServer;
}


/***************************************************************************************************
*
*	FUNCTION		NetMan::SendTo
*
*	DESCRIPTION		Send a message to a designated address, not necessarily an active client in
*					our network session.
*
*	INPUTS			NetMsg& obMsg - Message to be sent.
*
***************************************************************************************************/
void NetMan::SendTo(const NetAddr& obAddr, const NetMsg& obMsg)
{
	NetPacket::CPrivateData* pobData = 0;
	NetPacket* pobPacket = NetPacket::Create(pobData);

	pobData->obAddr = obAddr;
	NetMsg::SerialiseTo(pobData, obMsg);

	m_pobEndpoint->Send(*pobPacket);
	NT_DELETE( pobPacket );
}


/***************************************************************************************************
*
*	FUNCTION		NetMan::Send
*
*	DESCRIPTION		Send a message to a specific client
*
*	INPUTS			NetMsg& obMsg - Message to be sent.
*
***************************************************************************************************/
void NetMan::Send(const CSafeRef<NetClient>& obClient, const NetMsg& obMsg)
{
	ntAssert(obClient);

	if(obMsg.IsUrgent())
	{
		// This message is really important.  It can't wait for the next NetMan Update call!
		// Send it now!  It means we can't maintain the optimum packet size but we won't
		// incur any extra latency...  Messages like attack requests and results should
		// be marked as urgent.
		SendTo(obClient->GetAddress(), obMsg);

		#ifdef _NETSTATS
			m_iMsgsSentUrgent++;
		#endif
	}
	else
	{
		// This message isn't so urgent, we'll queue it up for sending with any others at the
		// end of this update.  This way we can reduce the number of total packets we send.
		obClient->AddToSendQueue(obMsg);

		#ifdef _NETSTATS
			m_iMsgsSent++;
		#endif
	}
}


/***************************************************************************************************
*
*	FUNCTION		NetMan::SendServer
*
*	DESCRIPTION		Send a message to our server
*
*	INPUTS			NetMsg& obMsg - Message to be sent.
*
***************************************************************************************************/
void NetMan::SendServer(const NetMsg& obMsg)
{
	if(m_pobServer.Valid())
	{
		Send(m_pobServer, obMsg);
		return;
	}

	//LOG_MESSAGE("Trying to send message to non-existant server.");
	ntPrintf("Trying to send message to non-existant server.");
}


/***************************************************************************************************
*
*	FUNCTION		NetMan::SendAll
*
*	DESCRIPTION		Send a message to all clients in the session.
*
*	INPUTS			NetMsg& obMsg - Message to be sent.
*
***************************************************************************************************/
void NetMan::SendAll(const NetMsg& obMsg)
{
	if(obMsg.IsUrgent())
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			NetClient* pobClient = m_apobClients[i];
			
			if(pobClient)
				SendTo(pobClient->GetAddress(), obMsg);
		}
	}
	else
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			NetClient* pobClient = m_apobClients[i];
			
			if(pobClient)
				pobClient->AddToSendQueue(obMsg);
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		NetMan::SendPacket
*
*	DESCRIPTION		Transmit a packet of data from our endpoint
*
*	INPUTS			NetPacket& obPacket
*
***************************************************************************************************/
void NetMan::SendPacket(NetPacket &obPacket)
{
	ntAssert(m_pobEndpoint);
	m_pobEndpoint->Send(obPacket);
}


/***************************************************************************************************
*
*	FUNCTION		NetMan::DecodePacket
*
*	DESCRIPTION		Extract and despatch all the messages in a packet
*
*	INPUTS			NetPacket& obPacket - Packet to decode.
*
***************************************************************************************************/
void NetMan::DecodePacket(const class NetPacket& obPacket)
{
#ifdef _NET_DEBUG
	// Check Packet Header
	const NetPacketHeader obHeader = obPacket.GetHeader();

	char s[1024];
	sprintf(s, "NET- Packet[%f]", obHeader.fTimestamp);
	OutputDebugString(s);
#endif

	// Find the client for this packet
	CSafeRef<NetClient> pobClient = FindClient(obPacket.GetAddress());

	// Send an Ack if requested...
	if(obPacket.RequireAck() && pobClient)
	{
		NetMsgAck obAck(obPacket.GetSentTimeStamp());
		Send(pobClient, obAck);
		ntPrintf("###NET### ACK!\n");
	}
	
	// Get message data
	const uint8_t* pData = obPacket.GetData();
	int	  iLen		  = obPacket.GetDataSize();
	int   iPtr		  = 0;

	while(iPtr < iLen)
	{
		NETMSG_TYPE eType = NETMSG_TYPE(pData[iPtr++]);
		NetMsg *pobMsg = gNetMsgFactory->GetInstance(eType);
		
		if(pobMsg)
		{
			int iLen = pobMsg->SerialiseFrom(&pData[iPtr]);
			if(iLen < 0)
			{
				ntPrintf("Received packet containing unknown message ID...");
				obPacket.SetError(NETERROR_UNKNOWN);
				return;
			}

			iPtr += iLen;
			
			#ifdef _NET_DEBUG
				char s[1024];
				sprintf(s, " (m%d)", pobMsg->GetType());
				OutputDebugString(s);
			#endif
			
			// Propagate timestamps
			pobMsg->m_fRecvTimeStamp = obPacket.GetRecvTimeStamp();
			pobMsg->m_fSentTimeStamp = obPacket.GetSentTimeStamp();

			// Despatch the message
			pobMsg->Despatch(pobClient, obPacket.GetAddress());
			#ifdef _NETSTATS
				m_iMsgsRecv++;
			#endif

			NT_DELETE( pobMsg );
		}
		else
		{
			#ifdef _NET_DEBUG
				OutputDebugString(" (INVALID)");
			#endif

			// This packet contains a message we don't understand
			// Or it's somehow corrupted
			NT_DELETE( pobMsg );
			break;
		}
	}

	#ifdef _NET_DEBUG
		OutputDebugString("\n");
	#endif
}



/***************************************************************************************************
*
*	FUNCTION		NetMsgAck::Despatch
*
*	DESCRIPTION		Acknowledgement message, remove waiting message from the queue
*
***************************************************************************************************/
void NetMsgAck::Despatch(CSafeRef<NetClient> pobClient, const NetAddr&) const
{
	if(!pobClient)
	{
		// Don't acknowledge packets from non-clients.
		// They shouldn't be sending reliable packets and it will just waste our
		// precious band-width
		return;
	}

	pobClient->Ack(m_fAckedTimeStamp);
}



/***************************************************************************************************
*
*	FUNCTION		NetMsgSync::Despatch
*
*	DESCRIPTION		Acknowledgement message, remove waiting message from the queue
*
***************************************************************************************************/
void NetMsgSync::Despatch(CSafeRef<NetClient> pobClient, const NetAddr&) const
{
	if(!pobClient)
	{
		// Need a client for SYN handshakes
		return;
	}

	// Send back a SYN_ACK packet
	NetMsgSyncAck obMsg(GetSentTimeStamp());
	NetMan::Send(pobClient, obMsg);
}


/***************************************************************************************************
*
*	FUNCTION		NetMsgSyncAck::Despatch
*
*	DESCRIPTION		Acknowledgement message, remove waiting message from the queue
*
***************************************************************************************************/
void NetMsgSyncAck::Despatch(CSafeRef<NetClient> pobClient, const NetAddr&) const
{
	if(!pobClient)
	{
		// Need a client for SYN handshakes
		return;
	}

	pobClient->Syn(m_fSynTimeStamp, GetRecvTimeStamp(), m_fRemoteGameTime);
}


/***************************************************************************************************
*
*	FUNCTION		NetMsgSyncAck::SerialiseTo
*
*	DESCRIPTION		Serialise the NetMsgSyncAck packet to a uint8_t stream
*
***************************************************************************************************/
int NetMsgSyncAck::SerialiseTo(uint8_t* pData) const
{

	((float*)pData)[0] = m_fSynTimeStamp;
	((float*)pData)[1] = (float)CTimer::Get().GetSystemTime();

	return sizeof(float)*2;		
}
