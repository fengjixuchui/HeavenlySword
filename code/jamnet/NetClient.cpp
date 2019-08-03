/***************************************************************************************************
*
*	DESCRIPTION				NetClient.cpp
*
*	NOTES					Implementation of the NetClient class.
*
***************************************************************************************************/

#include "jamnet/netclient.h"
#include "jamnet/netman.h"
#include "core/timer.h"

/***************************************************************************************************
*
*	FUNCTION		NetClient::NetClient
*
*	DESCRIPTION		Create a client from a network address
*
***************************************************************************************************/
NetClient::NetClient(const NetAddr &obAddr, int iID)
	: NetAddr(obAddr),
	  m_iID(iID),
	  m_obAwaitingAck()
{

	m_pobNextPacket = 0;
	m_pobNextPacketData = 0;

	m_pobNextReliablePacket = 0;
	m_pobNextReliablePacketData = 0;

	m_fTimeOffset = FLT_MAX;
	m_iLatencySlot = 0;
	m_fTimeToNextSyn = 0.0f;
	memset(m_fLatency, 0, sizeof(m_fLatency));
}


/***************************************************************************************************
*
*	FUNCTION		NetClient::~NetClient
*
*	DESCRIPTION		Destroy a client
*
***************************************************************************************************/
NetClient::~NetClient(void)
{
	if(m_pobNextPacket)
		NT_DELETE( m_pobNextPacket );

	if(m_pobNextReliablePacket)
		NT_DELETE( m_pobNextReliablePacket );

	m_obAwaitingAck.erase(m_obAwaitingAck.begin(), m_obAwaitingAck.end());
}


/***************************************************************************************************
*
*	FUNCTION		NetClient::operator==
*
*	DESCRIPTION		Compare two NetClients
*
***************************************************************************************************/
bool NetClient::operator==(const NetClient& obRHS) const
{
	return GetAddress() == obRHS.GetAddress();
}


/***************************************************************************************************
*
*	FUNCTION		NetClient::operator!=
*
*	DESCRIPTION		Compare two NetClients
*
***************************************************************************************************/
bool NetClient::operator!=(const NetClient& obRHS) const
{
	return GetAddress() != obRHS.GetAddress();
}


/***************************************************************************************************
*
*	FUNCTION		NetClient::operator==
*
*	DESCRIPTION		Compare this NetClient with a NetAddr
*
***************************************************************************************************/
bool NetClient::operator==(const NetAddr& obRHS) const
{
	return (NetAddr)*this == obRHS;
}


/***************************************************************************************************
*
*	FUNCTION		NetClient::operator!=
*
*	DESCRIPTION		Compare this NetClient with a NetAddr
*
***************************************************************************************************/
bool NetClient::operator!=(const NetAddr&   obRHS) const
{
	return (NetAddr)*this != obRHS;
}


/***************************************************************************************************
*
*	FUNCTION		NetClient::AddToSendQueue
*
*	DESCRIPTION		Queue up a NetMsg to be sent the next network frame.
*
***************************************************************************************************/
void NetClient::AddToSendQueue(const NetMsg& obMsg) const
{
	// Rather than having a list of NetMsg and all the hassle of ref counting and
	// maintaining free-lists and dynamically allocating and freeing variable sized
	// objects we're just going to serialise the data directly into packets.
	// When a packet is filled we'll send it off immediately, at the end of every
	// netframe we'll send any partially filled packets.
	//
	// Extension1: Segregate reliable and regular messages into different packets,
	// that way we wont waste any more bandwidth on resends than is strictly necessary.
	//
	// Extension2: We could serialise into a buffer maintaining priority information
	// about each uint8_t stream and at the end of the netframe evaluate the buffer
	// for bandwidth sending high priority, stalling medium priority and dumping
	// low priority etc. as necessary to avoid oversaturating the network.  This
	// may or may not be required.

	if(!obMsg.Reliable())
	{
		if(m_pobNextPacket)
		{
			if(NetMsg::SerialiseTo(m_pobNextPacketData, obMsg))
				return;

			// That packet hasn't enough room for this message... Send it now!
			NetMan::SendPacket(*m_pobNextPacket);
			NT_DELETE( m_pobNextPacket );
		}

		// Start a new packet
		m_pobNextPacket = NetPacket::Create(m_pobNextPacketData);
		m_pobNextPacketData->obAddr = GetAddress();


		if(!NetMsg::SerialiseTo(m_pobNextPacketData, obMsg))
			ntPrintf(" ### NET ### - NetClient::AddToSendQueue: Message to big for packet.");
	}
	else
	{
		if(m_pobNextReliablePacket)
		{
			if(NetMsg::SerialiseTo(m_pobNextReliablePacketData, obMsg))
				return;

			// That packet hasn't enough room for this message... Send it now!
			NetMan::SendPacket(*m_pobNextReliablePacket);
			NT_DELETE( m_pobNextReliablePacket );
		}

		// Start a new packet
		m_pobNextReliablePacket = NetPacket::Create(m_pobNextReliablePacketData);
		m_pobNextReliablePacketData->obAddr = GetAddress();


		if(!NetMsg::SerialiseTo(m_pobNextReliablePacketData, obMsg))
			ntPrintf(" ### NET ### - NetClient::AddToSendQueue: Message to big for packet.");
	}

}

/***************************************************************************************************
*
*	FUNCTION		NetClient::Update
*
*	DESCRIPTION		Send queued up packets
*
***************************************************************************************************/
void NetClient::Update()
{
	// Send SYN if it's that time...
	m_fTimeToNextSyn -= CTimer::Get().GetSystemTimeChange();
	if(m_fTimeToNextSyn < 0.0f)
	{
		m_fTimeToNextSyn = SYN_PERIOD;

		NetMsgSync obMsg;
		AddToSendQueue(obMsg);	
	}


	// Send the packets we've been filling this frame
	if(m_pobNextPacket)
	{
		NetMan::SendPacket(*m_pobNextPacket);
		NT_DELETE( m_pobNextPacket );
		m_pobNextPacket = 0;
	}

	if(m_pobNextReliablePacket)
	{
		NetMan::SendPacket(*m_pobNextReliablePacket);
		m_obAwaitingAck.push_back(m_pobNextReliablePacket);
		m_pobNextReliablePacket = 0;
	}

	// Check for resends...
	float fTime = (float)CTimer::Get().GetSystemTime();

	for(ntstd::List<NetPacket*>::iterator obIt = m_obAwaitingAck.begin(); obIt != m_obAwaitingAck.end(); obIt++)
	{
		if(fTime > (*obIt)->GetResendTimeStamp() + ACK_TIMELIMIT)
		{
			NetMan::SendPacket(**obIt);

			ntPrintf("Resending packet...\n");
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		NetClient::Ack
*
*	DESCRIPTION		Acknowledge receipt of a reliable packet
*
***************************************************************************************************/
void NetClient::Ack(float fTimeStamp) const
{
	for(ntstd::List<NetPacket*>::iterator it = m_obAwaitingAck.begin(); it != m_obAwaitingAck.end(); it++)
	{
		if((*it)->GetSentTimeStamp() == fTimeStamp)
		{
			NT_DELETE( (*it) );
			m_obAwaitingAck.erase(it);
			return;
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		NetClient::Syn
*
*	DESCRIPTION		Process the results a time synchronisation handshake
*
*	INPUTS			fSent	- Local time the SYN message was sent
*					fRecv	- Local time the SYN_ACK message was received
*					fRemote - Remote game time at the point this message was sent...
*
***************************************************************************************************/
void NetClient::Syn(float fSent, float fRecv, float fRemote)
{
	// Compute latency and time-delta
	float fLatency = (fRecv - fSent) / 2.0f;
	float fNow = (float)CTimer::Get().GetSystemTime();
	float fDelta = fRemote + fLatency - fNow;

	ntPrintf("SYN RECV - Local:%.2f Remote:%.2f latency:%.2f delta:%.2f\n", fNow, fRemote, fLatency, fDelta);

	// Save the time-delta if it's lower than we previously thought...
	// (We assume clocks are sufficiently accurate and we have no need
	//  to adjust the time delta upward.)
	if(fDelta < m_fTimeOffset)
		m_fTimeOffset = fDelta;

	// Update latency
	m_fLatency[m_iLatencySlot++ % LATENCY_TRACKER_SLOTS] = fLatency;
}
