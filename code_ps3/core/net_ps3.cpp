
#include "core/net_ps3.h"

#include <sys/select.h>
#include <sys/time.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Global temporary buffer 
#define NET_BUFFER_SIZE 2048
char g_cBuffer[NET_BUFFER_SIZE];

char g_cOptBuffer[NET_BUFFER_SIZE];
int g_iOptBufferIndex=0;

// Run with network code enabled!
//#define DISABLE_NET

bool WinsockStartup()
{
#ifdef DISABLE_NET
	return false;
#else
	// doesn't need to do anything
	return true;
#endif
}

bool WinsockCleanup()
{
#ifdef DISABLE_NET
	return false;
#else
	// doesn't need to do anything
	return true;
#endif
}

bool netstream::OptFlush() const
{
	if (g_iOptBufferIndex > 0)
	{
		if(::send(m_socket, (const char *)g_cOptBuffer, g_iOptBufferIndex, 0) == SOCKET_ERROR)
		{
			g_iOptBufferIndex = 0; // flush anyway, even if we couldn't send what was in our buffer
			return false;
		}
	}
	
	g_iOptBufferIndex = 0;
	return true;
}



bool netstream::OptBufferSend(const void* pcData, int iLen) const
{
	int iRemainingSpace;
	const char* pcIt = (const char*)pcData;
	while (iLen > 0)
	{
		if (g_iOptBufferIndex + iLen < NET_BUFFER_SIZE)
		{
			NT_MEMCPY(g_cOptBuffer + g_iOptBufferIndex, pcIt, iLen);
			g_iOptBufferIndex += iLen;
			return true;
		}

        iRemainingSpace = NET_BUFFER_SIZE - g_iOptBufferIndex;
		iLen -= iRemainingSpace;
		NT_MEMCPY(g_cOptBuffer + g_iOptBufferIndex, pcIt, iRemainingSpace);
		g_iOptBufferIndex += iRemainingSpace;
		pcIt += iRemainingSpace;
		if (!OptFlush())
		{
			return false;
		}
	}
	return true;
}





// --------------------------------------------------------------------------------
// Local utility function.
// Convert a hostname into a network byteorder integer.
// --------------------------------------------------------------------------------
bool getinetaddr(const char* host, sockaddr_in *inAddr)
{
	// inet_addr converts a string containing an IP address
	// to network byteorder integer version of the IP.
	inAddr->sin_addr.s_addr = inet_addr(host);

	// INADDR_NONE is returned if no valid IP could be extracted.
	if(inAddr->sin_addr.s_addr == INADDR_NONE)
	{
		// If no valid IP was extracted, chanses are that we have
		// a domain name instead, so we try to translate the name
		// to an IP.
		hostent *hostEntry = gethostbyname(host);

		// If this also fails, we are unable to determine the address.
		if(hostEntry == NULL) return false;

		// Take the IP address out of the hostent structure
		unsigned long hostAddress = *((unsigned long*)*hostEntry->h_addr_list);
		inAddr->sin_addr.s_addr = hostAddress;
	}

	return true;
}

// --------------------------------------------------------------------------------
// netsocket class
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------------
netsocket::netsocket()
{
	m_socket = 0;
}

// --------------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------------
netsocket::~netsocket()
{
	return;
}

// --------------------------------------------------------------------------------
// Allocate a new socket.
// --------------------------------------------------------------------------------
bool netsocket::create()
{
	close();

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	return m_socket ? true: false;
}

// --------------------------------------------------------------------------------
// Close the socket
// It will gracefully (as it's called in the MSDN docs) close the connection.
//
// for netserver: socket is closed, and no more connections can be accepted.
// for netstream: stream is closed, and no more data can be sent/received.
//
// return value:
//   true if socket/stream was successfully closed.
// --------------------------------------------------------------------------------
bool netsocket::close()
{
	if(isopen() == false) return true;

	int res = 0;

	res = socketclose(m_socket);

	m_socket = 0;

	return res ? false: true;
}

// --------------------------------------------------------------------------------
// Determines wether the socket is open or not.
//
// return value:
//   for netserver: true if socket is listening for connections.
//   for netstream: true if stream is open.
// --------------------------------------------------------------------------------
bool netsocket::isopen() const
{
	return m_socket ? true : false;
}


// --------------------------------------------------------------------------------
// return value:
//   The name of the host. Host is the computer on which the function was called.
// --------------------------------------------------------------------------------
CHashedString netsocket::gethostname() const
{
	sockaddr_in hostAddress;
	socklen_t	addressLength = sizeof hostAddress;

	if(::getsockname(m_socket, (sockaddr*)&hostAddress, &addressLength) == 0)
	{
		return CHashedString(inet_ntoa(hostAddress.sin_addr));
	}
	return CHashedString();
}

// --------------------------------------------------------------------------------
// netstream class
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
// Automatically closes the stream on desrtuction.
// For example, if the user forgot to close the connection before exiting
// the program, the desturctor will close the connection when it goes out of scope.
// --------------------------------------------------------------------------------
netstream::~netstream()
{
	close();
}

// --------------------------------------------------------------------------------
// Attach a socket to this stream
// --------------------------------------------------------------------------------
bool netstream::attach(SOCKET socket)
{
	if(isopen() == true) return false;

	m_socket = socket;
	return m_socket ? true : false;
}

// --------------------------------------------------------------------------------
// Connect the stream to a remote host.
// There are two types of connect-functions, one takes hostname and port
// as two different parameters, while the other takes them both in one parameter.
// If you have the portnumber as a separate integer value, you call this
// two-parameter variant.
//
// port:
//   Port number as an unsigned integer.
//
// name:
//   Name of the host to connect to as a string.
//   name can either be an IP address or a domain name.
//   ex. "192.168.1.1" or "host.domain.com"
//
// return value:
//   true if connection was successfull (server accepted connection)
//   false if connection failed, possible causes:
//    - server rejected connection attempt
//    - unable to resolve domain name or find IP
//    - already connected
// --------------------------------------------------------------------------------
bool netstream::connect(const char* hostName, int hostPort)
{
	if(isopen() == true) return false;
	
	sockaddr_in inAddr;
	inAddr.sin_family = AF_INET;
	inAddr.sin_port   = htons((u_short)hostPort);

	if(getinetaddr(hostName, &inAddr) == false) return false;

	if(create() == false) return false;
	if(::connect(m_socket, (sockaddr*)&inAddr, sizeof inAddr) != 0) return false;

	return true;
}



// --------------------------------------------------------------------------------
// Send a buffer to the stream
//
// data:
//   Pointer to buffer of data to send.
//
// len:
//   Number of Bytes to send.
//
// return value:
//   true if successfully sent
//   false if failed to send
// --------------------------------------------------------------------------------
bool netstream::send(const void *data, int len) const
{
	if(isopen() == false) return false;

#if 0
	if(::send(m_socket, (const char *)data, len, 0) == SOCKET_ERROR)
	{
		return false;
	}

	return true;
#else
	return OptBufferSend(data, len);
#endif
}

// --------------------------------------------------------------------------------
// Receive data from the stream
//
// data:
//   Pointer to buffer where the data is to be stored.
//
// len:
//   Number of Bytes to receive.
//
// len number of Bytes is read from the stream. If len is less than the number
// of available Bytes in the stream, recv will block until more data has arrived
// and the entire buffer is filled.
//
// return value:
//   number of Bytes received
// --------------------------------------------------------------------------------
int netstream::recv(void *data, int len) const
{
	if(isopen() == false) return false;

	int BytesReceived = 0;

	// Keep on extracting until the whole buffer is filled
	while(BytesReceived < len)
	{
		BytesReceived += ::recv(m_socket, ((char *)data) + BytesReceived, len - BytesReceived, 0);
	}

	return BytesReceived;
}

// --------------------------------------------------------------------------------
// Check wether there are data to be read from the stream or not.
//
// This function will also check wether the peer is alive or not.
// If the connection was interrupted for some reason, then connection
// is automatically closed on this side aswell, to prevent handling a closed
// connection.
//
// Possible causes of an interrupted connection:
//  - peer closed connection
//  - lost connection to peer
//  - remote computer halted/rebooted
//
// return value:
//   number of Bytes pending to receive
//
//   If a connection is interrupted, canrecv() will return zero and close
//   the connection. Any following call to isopen() will return false, and you
//   can assume the connection was interrupted in one way or another.
// --------------------------------------------------------------------------------
int netstream::canrecv()
{
#ifndef DISABLE_NET
	fd_set readFD;
	
//	readFD.fd_count    = 1;
//	readFD.fd_array[0] = m_socket;

	FD_ZERO(&readFD);
	FD_SET(m_socket, &readFD);

	timeval timeout;
	timeout.tv_sec  = 0;
	timeout.tv_usec = 0;

	// Socketselect is used to determine status on one or more sockets.
	// readFD contains all sockets we want to look at, in this case we only want
	// to look at one socket, the socket used by the stream.
	// Select returns the number of sockets who wants to tell us something.
	if(socketselect(m_socket + 1, &readFD, NULL, NULL, &timeout) > 0)
	{
		// This socket want to tell us something, now we need to
		// determine what it want.

		uint8_t data;

		// We use MSG_PEEK so we don't remove any important information
		// for the user. ::recv returns the number of Bytes pending to receive.
		int BytesPending = ::recv(m_socket, (char*)&data, 1, MSG_PEEK);

		// If no Bytes where extracted, it's trying to tell us something else.
		// In this case, it can only tell us one thing: the connection was lost/closed.
		// We close the connection on our side aswell, and tell the suer no Bytes is
		// waiting to be read.
		if(BytesPending == 0) 
			close();

		// Return the number of Bytes pending.
		return BytesPending;
	}
#endif 
	// If we get here, the socket don't have anything to tell us,
	// and therefore no Bytes can be read.
	return 0;
}

// --------------------------------------------------------------------------------
// return value:
//   Name of peer. Peer is the computer "in the other end" of the stream.
// --------------------------------------------------------------------------------
CHashedString netstream::getpeername() const
{
	sockaddr_in hostAddress;
	socklen_t	addressLength = sizeof(hostAddress);

	if(::getpeername(m_socket, (sockaddr*)&hostAddress, &addressLength) == 0)
	{
		return CHashedString(inet_ntoa(hostAddress.sin_addr));
	}

	return CHashedString();
}

// ALEXEY_TODO :

// --------------------------------------------------------------------------------
// Stream a string to the other computer.
// --------------------------------------------------------------------------------
//CSimpleStream &netstream::operator <<(const CHashedString &data)
//{
//	//send(*data, data.GetLength());
//	send(&data, sizeof(data));
//
//	return *this;
//}

// --------------------------------------------------------------------------------
// Extract a string from the stream.
// --------------------------------------------------------------------------------
//netstream &netstream::operator >>(CHashedString &data)
//{
//	int iBufIndex = 0;
//
//	// Extract byte after byte, until a null character is extracted, and insert
//	// it into the string.
//	//while(true)
//	//{
//	//	recv(&g_cBuffer[iBufIndex], 1);
//	//	if(g_cBuffer[iBufIndex] == '\0') break;
//	//	iBufIndex++;
//	//	ntAssert(iBufIndex < NET_BUFFER_SIZE);
//	//}
//
//
//	//data = g_cBuffer;
//	return *this;
//}

// --------------------------------------------------------------------------------
// Stream a null terminated character array.
// --------------------------------------------------------------------------------
CSimpleStream &netstream::operator <<(const char *data)
{
	// Since char* is naturally ended with a null character, we don't need to
	// go through the process of finding extra null characters.
	send(data, strlen(data));

	return *this;
}

CSimpleStream &netstream::operator <<(float fValue)
{
	sprintf(g_cBuffer, "%f", fValue);
	send(g_cBuffer, strlen(g_cBuffer));
	return *this;
}

CSimpleStream &netstream::operator <<(int iValue)
{
	sprintf(g_cBuffer, "%d", iValue);
	send(g_cBuffer, strlen(g_cBuffer));
	return *this;
}

void netstream::EndBlock()
{
	char iBuffer = 0;
	send(&iBuffer, 1);
	OptFlush();
}


// --------------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------------
void netstream::Indent()
{
	int iLevel = m_gIndentLevel;
	ntAssert(iLevel < NET_BUFFER_SIZE -1);

	char* pobInd = g_cBuffer;
	while (iLevel > 0)
	{
		*pobInd++ = '\t';
		iLevel--;
	}
	
	send(g_cBuffer, pobInd-g_cBuffer);
}


void netstream::Endline() 
{
	*this << "\r\n";
}


// --------------------------------------------------------------------------------
// netserver class
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------------
netserver::~netserver()
{
	close();
}

// --------------------------------------------------------------------------------
// Tell socket to listen for incomming connections.
//
// port:
//   The port number to listen on.
//
// interface:
//   Specifies which interface to use to listen on. Interface is the specific
//   adapter to listen on. Usefull if you have more than one network adaper but
//   only wants to listen for connection on one of them.
//
// backlog:
//   The maximum number of incomming connections that can wait for
//   acception at any given time, before connections are automatically rejected
//   by the socket handler (this happens on driver/os level, and not by these
//   functions).
//
// interface and backlog are optional values and can be ignored.
// interface defaults to an empty string, which will listen to any adapter.
// backlog defaults to 5 incomming connections.
//
// return value:
//   true if socket successfully placed into a listening state
//   false otherwise
// --------------------------------------------------------------------------------
bool netserver::listen(int port, const char* adapter, int backlog)
{
	if(isopen() == true) return false;
	
	sockaddr_in inAddr;
	inAddr.sin_family = AF_INET;
	inAddr.sin_port   = htons((u_short)port);

	if(adapter == NULL)
	{
		// If no string is passed, we assume any adapter is valid...
		inAddr.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		// ... otherwise we need to get the IP of the adapter
		if(getinetaddr(adapter, &inAddr) == false) return false;
	}

	if(create() == false) return false;
	if(::bind(m_socket, (sockaddr*)&inAddr, sizeof(inAddr)) != 0) return false;
	if(::listen(m_socket, backlog) != 0) return false;

	return true;
}


// --------------------------------------------------------------------------------
// Accept an incomming connection and attach it to a netstream.
//
// socket:
//   The stream to which you want to attach the incomming connection.
//
// return value:
//   true if socket was successfully attached to stream
//   false otherwise
// --------------------------------------------------------------------------------
bool netserver::accept(netstream& socket) const
{
	// Accept the incomming socket
	SOCKET s = ::accept(m_socket, NULL, 0);

	if(s == INVALID_SOCKET) return false;

	// Attach the socket to a stream
	return socket.attach(s);
}

// --------------------------------------------------------------------------------
// Checks is there is an incomming connection waiting
//
// return value:
//   true if there is a connection waiting
//   false if no connection is waiting
// --------------------------------------------------------------------------------
bool netserver::canaccept() const
{
#ifndef DISABLE_NET
	fd_set readFD;
	FD_ZERO(&readFD);
	FD_SET(m_socket, &readFD);

	timeval timeout;
	timeout.tv_sec  = 0;
	timeout.tv_usec = 0;
	
	// Select is described in canrecv()
	int res = socketselect(m_socket + 1, &readFD, NULL, NULL, &timeout);

	// If a socket is trying to tell us something, and this socket is
	// in a listening state, it means someone is trying to connect to us.
	return res > 0 ? true : false;
#endif

	return false;
}


// --------------------------------------------------------------------------------
// netclient class
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
// 
// --------------------------------------------------------------------------------
netclient::~netclient()
{
	close();
}

bool netclient::connect(const int port, const char* targetIP, netstream &stream)
{
	if (isopen()) return false;

	sockaddr_in inAddr;
	inAddr.sin_family = AF_INET;
	inAddr.sin_port = htons((u_short)port);;
	
	if (getinetaddr(targetIP, &inAddr) == false) return false;
	if (create() == false) return false;
	if (::connect(m_socket, (sockaddr*)&inAddr, sizeof(inAddr)) != 0) return false;

	// Attach the socket to a stream
	return stream.attach(m_socket);
}
