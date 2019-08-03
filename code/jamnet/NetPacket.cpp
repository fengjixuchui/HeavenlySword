/***************************************************************************************************
*
*	DESCRIPTION			NetPacket.cpp
*
*	NOTES				Implementation of NetPacket.  Used for serialising to and from NetMsgs]
*						and for transmitting serialised messages and data across a network.
*
***************************************************************************************************/

#include "jamnet/netpacket.h"
#include "core/timer.h"

/***************************************************************************************************
*
*	FUNCTION		NetPacket::NetPacket
*
*	DESCRIPTION
*
***************************************************************************************************/
NetPacket::NetPacket()
{
	Init();
}

inline void NetPacket::Init()
{
	m_eErr = NETERROR_SUCCESS;
	m_bFree = false;
	m_fRecvTimeStamp = 0.0;
}


/***************************************************************************************************
*
*	FUNCTION		NetPacket::Create
*
*	DESCRIPTION		This used to work on my own pooling, probably should do away with this
*					method now...  Useful way to get the privatedata pointer though.
*
*	
*
***************************************************************************************************/
NetPacket* NetPacket::Create(NetPacket::CPrivateData*& pobData)
{
	NetPacket* pobRetval = NT_NEW NetPacket;

	if(pobRetval)
	{
		// Set a pointer to the private data area
		pobData = &pobRetval->m_obData;

		// Blank out the header...  The rest of the data doesn't matter as it's measured...
		memset(pobRetval->m_obData.pData, 0, 8);

		pobRetval->m_fResendTimeStamp = 0.0f;
	}
	else
	{
		pobData = 0;
		ntPrintf("%s(%d) : ntError(%s) - could not create packet.\n", __FILE__, __LINE__, __FUNCTION__);
	}

	return pobRetval;
}


/***************************************************************************************************
*
*	FUNCTION		NetPacket::~NetPacket
*
*	DESCRIPTION
*
***************************************************************************************************/
NetPacket::~NetPacket()
{
}

/***************************************************************************************************
*
*	FUNCTION		NetPacket::GetHeaderP
*
*	DESCRIPTION		Return the packet header for this packet.
*
***************************************************************************************************/
const NetPacketHeader& NetPacket::GetHeader() const
{
	return *((NetPacketHeader*)&m_obData.pData);
}


/***************************************************************************************************
*
*	FUNCTION		NetPacket::StampSent
*
*	DESCRIPTION		Set the time sent.
*
***************************************************************************************************/
void NetPacket::StampSent()
{
	float fTime = (float)CTimer::Get().GetSystemTime();

	// Don't set the timestamp again if it's a resent packet
	if(!RequireAck() || m_fResendTimeStamp == 0.0f)
		*((float*)&(m_obData.pData)) = fTime;

	// Do sent the local sent again time though
	if(RequireAck())
		m_fResendTimeStamp = fTime;
}


/***************************************************************************************************
*
*	FUNCTION		NetPacket::GetRecvTimeStamp
*
*	DESCRIPTION		Get the time sent.
*
***************************************************************************************************/
float NetPacket::GetSentTimeStamp() const
{
	return *((float*)&(m_obData.pData));
}


/***************************************************************************************************
*
*	FUNCTION		NetPacket::StampRecv
*
*	DESCRIPTION		Set the time received.
*
***************************************************************************************************/
void NetPacket::StampRecv()
{
	m_fRecvTimeStamp  = (float)CTimer::Get().GetSystemTime();
}


/***************************************************************************************************
*
*	FUNCTION		NetPacket::GetRecvTimeStamp
*
*	DESCRIPTION		Get the time received.
*
***************************************************************************************************/
float NetPacket::GetRecvTimeStamp() const
{
	return m_fRecvTimeStamp;
}


/***************************************************************************************************
*
*	FUNCTION		NetPacket::RequireAck
*
*	DESCRIPTION		Indicate whether this packet wants an acknowledgement
*
***************************************************************************************************/
bool NetPacket::RequireAck() const
{
	return ((NetPacketHeader*)&m_obData.pData)->btRSVP != 0;
}


/***************************************************************************************************
*
*	FUNCTION		NetPacket::RequireAck
*
*	DESCRIPTION		Indicate whether this packet wants an acknowledgement
*
***************************************************************************************************/
bool NetPacket::IsOriginal() const
{
	return ((NetPacketHeader*)&m_obData.pData)->btResend != 0;
}
