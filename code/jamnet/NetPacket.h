/***************************************************************************************************
*
*	DESCRIPTION			NetPacket.cpp
*
*	NOTES				Implementation of NetPacket.  Used for serialising to and from NetMsgs]
*						and for transmitting serialised messages and data across a network.
*
***************************************************************************************************/

#ifndef _NET_PACKET_H
#define _NET_PACKET_H

#include "jamnet/netaddr.h"
#include "jamnet/neterror.h"

/***************************************************************************************************
*
*	CLASS			NetPacketHeader
*
*	DESCRIPTION		The structure describing the header of the JAM network packets.
*
*					fTimestamp = Time packet was SENT
*					btInstant  = 1: Marked for immediate delivery, 0: Marked for normal delivery
*					btRSVCP	   = 1: Requires an ACK,               0: Normal packet
*					btResend   = 1: This packet is a resend,       0: This is the original packet
*
***************************************************************************************************/
struct NetPacketHeader
{
	float fTimestamp;
	char  btInstant:1; char btRSVP:1; char btResend:1;  char cPadding:5;
};

/***************************************************************************************************
*
*	CLASS			NetPacket
*
*	DESCRIPTION		The JAM network packet
*
***************************************************************************************************/
class NetPacket
{
// Packet Constants
public:
	const static int MAXSIZE = 1450;
	const static int HEADERSIZE = sizeof(NetPacketHeader);
	const static int MAXDATA = MAXSIZE - HEADERSIZE;

// Internal Classes
public:
	struct CPrivateData
	{
	public:
		CPrivateData() {iLen = 0;}
		NetPacketHeader& GetHeader() {return *((NetPacketHeader*)&pData);}

		NetAddr obAddr;
		int		 iLen;
		uint8_t	 pData[MAXSIZE];
	};

// Methods
public:
	~NetPacket();
	void Init();

	const NetAddr&			GetAddress() const {return m_obData.obAddr;}
	const uint8_t*				GetData() const {return (uint8_t*)m_obData.pData+HEADERSIZE;}
	const NetPacketHeader&	GetHeader() const;
	int						GetDataSize() const {return m_obData.iLen;}
	int						GetTotalSize() const {return m_obData.iLen+HEADERSIZE;}

	// TimeStamping
	void  StampSent();
	void  StampRecv();
	float GetSentTimeStamp() const;
	float GetRecvTimeStamp() const;
	float GetResendTimeStamp() const  {return m_fResendTimeStamp;}

	// Info
	bool RequireAck() const;
	bool IsOriginal() const;

	// Errors
	void	 SetError(NETERROR eErr) const {m_eErr = eErr;}
	NETERROR GetError() const {return m_eErr;}

// Creator function
public:
	static NetPacket* Create(CPrivateData*& pobData);

private:
	NetPacket();

// Implementation
private:
	CPrivateData m_obData;

	mutable NETERROR m_eErr;
	float			 m_fRecvTimeStamp;
	float			 m_fResendTimeStamp;
	bool			 m_bFree;
};

#endif // _NET_PACKET_H
