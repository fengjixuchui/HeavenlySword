
#ifndef _NET_STREAM_H
#define _NET_STREAM_H

// Based on this code. It has been modified.

/********************************************************************

	filename:   netstream.h
	author:     Mikael Swartling
	e-mail:     e98msv@du.se

	description:
	------------
	Network streams is a set of easy-to-use classes to deal with peer-to-peer
	communication over the Internet. It uses << and >> operators to stream
	pre/user defined datatypes, and commands for sending/receiving arbitrary
	buffers of data over a connection. It operates on a very low level, making
	it independent of any application-level protocol.

	-----

	This software is provided 'as-is', without any express or implied warranty.
	In no event will the author be held liable for any damages arising from the
	use of this software.

	Permission is hereby granted to use, copy, modify, and distribute this
	source code, or portions hereof, for any purpose, without fee, subject to
	the following restrictions:
	
	1. The origin of this source code must not be misrepresented. You must not
	   claim that you wrote the original software. If you use this software in
	   a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.

	2. Altered versions must be plainly marked as such and must not be
	   misrepresented as being the original source.
		
	3. This notice must not be removed or altered from any source distribution.

*********************************************************************/

// use the correct version of winsock library based on machine
#include <winsock2.h>
#include "core/io.h"

bool WinsockStartup();
bool WinsockCleanup();


// --------------------------------------------------------------------------------
// netsocket:
// It provides some functions that is common to both netstream and netserver.
// --------------------------------------------------------------------------------
class netsocket 
{
public:

	netsocket();
	virtual ~netsocket();

	bool   close();
	bool   isopen() const;
	CKeyString gethostname() const;

protected:

	bool create();

protected:
	SOCKET m_socket;
};


// --------------------------------------------------------------------------------
// netsream:
// Stream data from/to another computer.
// --------------------------------------------------------------------------------
class netstream : public netsocket, public CSimpleStream
{
public:
	netstream() : netsocket() {}
	~netstream();

	bool attach(SOCKET socket);

	bool connect(const CKeyString &hostName, int hostPort);

	bool send(const void *data, int len) const;
	int  recv(      void *data, int len) const;

	int  canrecv();

	CKeyString getpeername() const;

	template<typename T> netstream &operator <<(const T &data) {send(&data, sizeof T);return *this;}
	template<typename T> netstream &operator >>(      T &data) {recv(&data, sizeof T);return *this;}

	CSimpleStream &operator <<(const CKeyString &data);
	netstream &operator >>(      CKeyString &data);
	CSimpleStream &operator <<(const char * data);

	CSimpleStream &operator << (float fValue);
	CSimpleStream &operator << (int iValue);
	
	void EndBlock();

	// Helper functions for serialisation
	void Indent();
	void Endline();

	// Is this a network stream ?
	virtual bool IsNetStream() { return true; };
	bool OptFlush() const;
	bool OptBufferSend(const void* pcData, int iLen) const;

};

// --------------------------------------------------------------------------------
// netserver:
// The class for dealing with incomming connections.
// --------------------------------------------------------------------------------
class netserver : public netsocket
{
public:
	netserver() : netsocket() {}
	~netserver();

	bool listen(int port, const CKeyString &adapter = CKeyString(""), int backlog = 5);
	bool accept(netstream &socket) const;

	bool canaccept() const;
};

class netclient : public netsocket
{
public:
	netclient() : netsocket() {}
	~netclient();

	bool connect(const int port, const CKeyString &targetIP, netstream &stream);
};

#endif //_NET_STREAM_H