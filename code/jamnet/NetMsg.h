/***************************************************************************************************
*
*	DESCRIPTION		Template Class Factory
*
*	NOTES			John Lusty 2004
*
***************************************************************************************************/

#ifndef _NET_MSG_H
#define _NET_MSG_H

#include "core/saferef.h"
#include "jamnet/netpacket.h"
#include "core/classfactory.h"

typedef uint8_t NETMSG_TYPE;

// JAMNet Messages
const NETMSG_TYPE NETMSG_ACK = 200;
const NETMSG_TYPE NETMSG_SYN = 201;
const NETMSG_TYPE NETMSG_SYN_ACK = 202;
const NETMSG_TYPE NETMSG_SYNC_VAR = 203;
const NETMSG_TYPE NETMSG_SYNC_FUNC = 204;
const NETMSG_TYPE NETMSG_SYNC_KILLOBJ = 205;

class NetClient;

/***************************************************************************************************
*
*	CLASS			NetMsg
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/
class NetMsg
{
public:
	NetMsg() {m_fSentTimeStamp = m_fRecvTimeStamp = 0.0;}
	virtual ~NetMsg() {};

	// Interface
	virtual NETMSG_TYPE	GetType() const = 0;
	virtual	int			GetLength() const = 0;
	virtual	int			SerialiseFrom(const uint8_t* pData) = 0;
	virtual int			SerialiseTo(uint8_t* pData) const = 0;
	virtual void		Despatch(CSafeRef<NetClient> pobClient, const class NetAddr& obAddr) const = 0;

	// Meta Data
	virtual bool IsUrgent() const = 0; // Send quickly, don't wait on a net frame
	virtual bool Reliable() const = 0; // Ensure it arrives, no guarantee on ordering though
  //virtual bool Ordered() const {return false;} // POSSIBLE EXTENSION: Ensure correct ordering...

	static bool SerialiseTo(NetPacket::CPrivateData*& pobData, const NetMsg& obMsg);

// Timestamping functions
public:
	float GetSentTimeStamp() const {return m_fSentTimeStamp;}
	float GetRecvTimeStamp() const {return m_fRecvTimeStamp;}

private:
	float m_fSentTimeStamp;
	float m_fRecvTimeStamp;
	friend class NetMan;
};

// Our Class Factory
PUBLISH_CLASS_FACTORY(CFactoryStorageArray, NetMsg, NETMSG_TYPE, gNetMsgFactory);

// NetMsg Declaration Macro Helpers
#define DECLARE_NETMSG_TYPE(type) virtual NETMSG_TYPE GetType() const {return type;} \
								  static const NETMSG_TYPE eType = type

#define URGENT   virtual bool IsUrgent() const {return true;}
#define RELIABLE virtual bool Reliable() const {return true;}

#define REGISTER_NETMSG(netmsg) \
	REGISTER_CLASS(NetMsg, NETMSG_TYPE, CFactoryStorageArray, gNetMsgFactory, netmsg, netmsg::eType)

#endif // _NET_MSG_H
