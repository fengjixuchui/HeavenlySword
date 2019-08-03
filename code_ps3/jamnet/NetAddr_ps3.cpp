/***************************************************************************************************
*
*	DESCRIPTION				NetAddr.cpp
*
*	NOTES					Not IPv6 compatible
*
***************************************************************************************************/

#include "jamnet/netaddr.h"
#include <sys/socket.h>
#include <netdb.h>

/***************************************************************************************************
*
*	FUNCTION		NetAddr::NetAddr
*
*	DESCRIPTION		Create a network address from a dotted format and a port number
*
***************************************************************************************************/
/*
NetAddr::NetAddr(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned short iPort)
{
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.S_un.S_un_b.s_b1 = c1;
	m_addr.sin_addr.S_un.S_un_b.s_b1 = c2;
	m_addr.sin_addr.S_un.S_un_b.s_b1 = c3;
	m_addr.sin_addr.S_un.S_un_b.s_b1 = c4;
	m_addr.sin_port = htons(iPort);
}
*/

/***************************************************************************************************
*
*	FUNCTION		NetAddr::NetAddr
*
*	DESCRIPTION		Create a network address from a long and a port number
*
***************************************************************************************************/
NetAddr::NetAddr(unsigned long lIPAddr, unsigned short iPort)
{
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = htonl(lIPAddr);
	m_addr.sin_port = htons(iPort);
}


/***************************************************************************************************
*
*	FUNCTION		NetAddr::NetAddr
*
*	DESCRIPTION		Create a network address from a host name and a port number
*
***************************************************************************************************/
NetAddr::NetAddr(const char* pszAddr, unsigned short iPort)
{
	hostent *pobHost = gethostbyname(pszAddr);

	if(pobHost)
	{
		in_addr *pIAddr = (in_addr*)pobHost->h_addr_list[0];
		m_addr.sin_family = AF_INET;
		m_addr.sin_addr = *pIAddr;
		m_addr.sin_port = htons(iPort);
	}
}


/***************************************************************************************************
*
*	FUNCTION		NetAddr::NetAddr
*
*	DESCRIPTION		Copy Constructor
*
***************************************************************************************************/
NetAddr::NetAddr(const NetAddr &obRHS)
{
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = obRHS.m_addr.sin_addr.s_addr;
	m_addr.sin_port = obRHS.m_addr.sin_port;
}


/***************************************************************************************************
*
*	FUNCTION		NetAddr::operator=
*
*	DESCRIPTION		Assignment operator
*
***************************************************************************************************/
NetAddr& NetAddr::operator =(const NetAddr &obRHS)
{
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = obRHS.m_addr.sin_addr.s_addr;
	m_addr.sin_port = obRHS.m_addr.sin_port;

	return *this;
}


/***************************************************************************************************
*
*	FUNCTION		NetAddr::operator==
*
*	DESCRIPTION		Compare this NetAddr with another
*
***************************************************************************************************/
bool NetAddr::operator ==(const NetAddr& obRHS) const
{
	if(m_addr.sin_addr.s_addr != obRHS.m_addr.sin_addr.s_addr)
		return false;

	if(m_addr.sin_port != obRHS.m_addr.sin_port)
		return false;

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		NetAddr::operator!=
*
*	DESCRIPTION		Compare this NetAddr with another
*
***************************************************************************************************/
bool NetAddr::operator !=(const NetAddr& obRHS) const
{
	if(m_addr.sin_addr.s_addr == obRHS.m_addr.sin_addr.s_addr)
		return false;

	if(m_addr.sin_port == obRHS.m_addr.sin_port)
		return false;

	return true;
}
