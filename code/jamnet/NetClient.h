/***************************************************************************************************
*
*	DESCRIPTION			NetClient.h
*
*	NOTES				Descibes the NetClient class
*
***************************************************************************************************/

#ifndef _NET_CLIENT_H
#define _NET_CLIENT_H

// References
/////////////
#include "core/saferef.h"
#include "jamnet/netaddr.h"
#include "jamnet/netmsg.h"

// Constants
//////////////////////////////////////////////
const int	LATENCY_TRACKER_SLOTS  = 10;	// Number of latency checks to keep
const float SYN_PERIOD			   = 5.0f;	// Rate at which to send lantency check (SYN) packets

#define NULL_CLIENT CSafeRef<NetClient>(NULL_REF)

/***************************************************************************************************
*
*	CLASS			NetClient
*
*	DESCRIPTION		A network target we're talking with...
*
***************************************************************************************************/
class NetClient : public NetAddr, public CSafeRefServer<NetClient>
{
public:
	NetClient(const NetAddr& obAddr, int iID);
	virtual ~NetClient();

	int GetID() const {return m_iID;}

	// Comparison
	bool operator==(const NetClient& obRHS) const;
	bool operator!=(const NetClient& obRHS) const;
	bool operator==(const NetAddr&   obRHS) const;
	bool operator!=(const NetAddr&   obRHS) const;

	// Send Queue Functions
	void AddToSendQueue(const NetMsg& obMsg) const;
	void Update();

	// Acknowledgement and Synchronisation
	void Ack(float fTimeStamp) const;
	void Syn(float fSent, float fRecv, float fRemote);

	// Address Accessor
	const NetAddr& GetAddress() const {return *this;}

// Implementation
private:
	int m_iID;

	//Network Time
	float m_fTimeOffset;						// The imputed difference between our game timer and theirs
	float m_fLatency[LATENCY_TRACKER_SLOTS];	// The packet transit times to this client
	float m_fLatencyAvg;						// The average latency on this link
	int   m_iLatencySlot;
	float m_fTimeToNextSyn;

	// Waiting Packets
	mutable class NetPacket* m_pobNextPacket;
	mutable NetPacket::CPrivateData* m_pobNextPacketData;

	mutable class NetPacket* m_pobNextReliablePacket;
	mutable NetPacket::CPrivateData* m_pobNextReliablePacketData;
	
	mutable ntstd::List<NetPacket*> m_obAwaitingAck;
};

#endif // _NET_CLIENT_H

