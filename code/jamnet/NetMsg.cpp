/***************************************************************************************************
*
*	DESCRIPTION		Template Class Factory
*
*	NOTES			John Lusty 2004
*
***************************************************************************************************/

#include "jamnet/netmsg.h"

// Our class factory
DECLARE_CLASS_FACTORY(CFactoryStorageArray, NetMsg, NETMSG_TYPE, gNetMsgFactory);

/***************************************************************************************************
*
*	FUNCTION		NetMsg::SerialiseTo													[STATIC]
*
*	DESCRIPTION		Serialise a message into a packet with it's packet ID appended
*
*	INPUTS			NetPacket::CPrivateData* - pointer to the private datasection of the packet
*					NetMsg*				  - pointer to message to serialise into the packet
*
***************************************************************************************************/
bool NetMsg::SerialiseTo(NetPacket::CPrivateData*& pobData, const NetMsg& obMsg)
{
	ntAssert(pobData);
	if(NetPacket::MAXDATA - pobData->iLen < obMsg.GetLength())
		return false;

	pobData->pData[NetPacket::HEADERSIZE + pobData->iLen++] = (uint8_t)obMsg.GetType();

	int iSerialised = obMsg.SerialiseTo((uint8_t*)&pobData->pData[pobData->iLen + NetPacket::HEADERSIZE]); 
	pobData->iLen += iSerialised;

	// Do we want an acknowledgement?
	if(obMsg.Reliable())
		((NetPacketHeader*)pobData->pData)->btRSVP = 1;

	// SerialiseTo returns the number of Bytes serialised, or -1 in case of insufficient space
	return iSerialised >= 0;
}
