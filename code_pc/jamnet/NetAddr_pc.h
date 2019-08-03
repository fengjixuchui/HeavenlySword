/***************************************************************************************************
*
*	DESCRIPTION				NetAddr.cpp
*
*	NOTES					Not IPv6 compatible
*
***************************************************************************************************/

#ifndef _NET_ADDR_PC_H
#define _NET_ADDR_PC_H

#include <winsock2.h>

class NetAddr
{
public:
	NetAddr() {;} // Default blank constructor

	NetAddr(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned short iPort);
	NetAddr(unsigned long lIPAddr, unsigned short iPort);
	NetAddr(const char* pszHost, unsigned short iPort);

	// Copy and assignment
	NetAddr(const NetAddr &obRHS);
	NetAddr& operator=(const NetAddr& obRHS);

	// Comparison
	bool	operator==(const NetAddr& obRHS) const;
	bool	operator!=(const NetAddr& obRHS) const;

	// Host/Port numbers
	unsigned long  GetHost() const {return ntohl(m_addr.sin_addr.S_un.S_addr);}
	unsigned short GetPort() const {return ntohs(m_addr.sin_port);}

	// Get low level address
	const sockaddr_in& operator*() const {return m_addr;}

private:
	sockaddr_in m_addr;
};

#endif // _NET_ADDR_PC_H
