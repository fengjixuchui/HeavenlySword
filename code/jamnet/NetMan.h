/***************************************************************************************************
*
*	DESCRIPTION		NetMan.cpp
*
*	NOTES			Definition of the network manager class
*
***************************************************************************************************/

#ifndef _NET_MAN_H
#define _NET_MAN_H

#include "jamnet/netclient.h"

const float ACK_TIMELIMIT = 10.0f;

class NetMan
{
public:
	static bool Init(short iPort);
	static void CleanUp();
	static bool IsRunning() {return m_bRunning;}

	static void Update();

	// Client Management
	static const CSafeRef<NetClient> AddClient(const NetAddr& obAddr);
	static const CSafeRef<NetClient> FindClient(const NetAddr& obAddr);
	static void						  RemoveClient(const CSafeRef<NetClient> obClient);
	static void						  RemoveClient(const NetAddr& obAddr);

	// Server Management
	static const CSafeRef<NetClient> SetServer(const NetAddr &obAddr);
	static const CSafeRef<NetClient> GetServer() {return m_pobServer;}

	// Sending
	static void SendTo(const NetAddr& obAddr, const NetMsg& obMsg);
	static void Send(const CSafeRef<NetClient>& obClient, const NetMsg& obMsg);
	static void SendServer(const NetMsg& obMsg);
	static void SendAll(const NetMsg& obMsg);
	static void SendPacket(NetPacket &obPacket);

// Private Helper Funcs
protected:
	static void DecodePacket(const class NetPacket& obPacket);

// Constants
public:
	static const int MAX_CLIENTS = 32;

// NetPacket Pool
public:

// Static Data
private:
	static bool  m_bRunning;
	static class NetEndpoint* m_pobEndpoint;

	static CSafeRef<NetClient> m_pobServer;
	static NetClient*			m_apobClients[MAX_CLIENTS];

// Debug Statistics
#ifdef _NETSTATS
	static unsigned long m_iMsgsSent;
	static unsigned long m_iMsgsSentUrgent;
	static unsigned long m_iMsgsRecv;
#endif
};


/***************************************************************************************************
*
*	CLASS			NetMsgAck
*
*	DESCRIPTION		Acknowledge a reliable packet
*
***************************************************************************************************/
class NetMsgAck : public NetMsg
{
public:
	DECLARE_NETMSG_TYPE(NETMSG_ACK);

	NetMsgAck() {m_fAckedTimeStamp = 0.0f;}
	NetMsgAck(float fAckedTimeStamp) {m_fAckedTimeStamp = fAckedTimeStamp;}

	// Interface
	virtual	int	 GetLength() const {return sizeof(float);}
	virtual	int  SerialiseFrom(const uint8_t* pData) 
	{
		m_fAckedTimeStamp = *((float*)pData);
		return GetLength();
	}
	virtual int  SerialiseTo(uint8_t* pData) const
	{
		*((float*)pData) = m_fAckedTimeStamp;
		return GetLength();		
	}
	virtual void Despatch(CSafeRef<NetClient> pobClient, const class NetAddr& obAddr) const;

	virtual bool IsUrgent() const {return false;}
	virtual bool Reliable() const {return false;}

private:
	float m_fAckedTimeStamp;
};


/***************************************************************************************************
*
*	CLASS			NetMsgSync
*
*	DESCRIPTION		
*
***************************************************************************************************/
class NetMsgSync : public NetMsg
{
public:
	DECLARE_NETMSG_TYPE(NETMSG_SYN);

	NetMsgSync() {;}

	// Interface
	virtual	int	 GetLength() const {return 0;}
	virtual	int  SerialiseFrom(const uint8_t*) {return 0;}
	virtual int  SerialiseTo(uint8_t*) const	{return 0;}
	virtual void Despatch(CSafeRef<NetClient> pobClient, const class NetAddr& obAddr) const;

	virtual bool IsUrgent() const {return false;}
	virtual bool Reliable() const {return false;}
};


/***************************************************************************************************
*
*	CLASS			NetMsgSyncAck
*
*	DESCRIPTION		
*
***************************************************************************************************/
class NetMsgSyncAck : public NetMsg
{
public:
	URGENT;
	DECLARE_NETMSG_TYPE(NETMSG_SYN_ACK);

	NetMsgSyncAck() {m_fSynTimeStamp = 0.0f;}
	NetMsgSyncAck(float fSynTimeStamp) {m_fSynTimeStamp = fSynTimeStamp;}

	// Interface
	virtual	int	 GetLength() const {return sizeof(float)*2;}
	virtual	int  SerialiseFrom(const uint8_t* pData) 
	{
		m_fSynTimeStamp	  = ((float*)pData)[0];
		m_fRemoteGameTime = ((float*)pData)[1];
		return GetLength();
	}
	virtual int  SerialiseTo(uint8_t* pData) const;
	virtual void Despatch(CSafeRef<NetClient> pobClient, const class NetAddr& obAddr) const;

	virtual bool Reliable() const {return false;}

private:
	float m_fSynTimeStamp;   // Timestamp of the original SYN packet
	float m_fRemoteGameTime; // The game time of the remote game
};

#endif // _NET_MAN_H
