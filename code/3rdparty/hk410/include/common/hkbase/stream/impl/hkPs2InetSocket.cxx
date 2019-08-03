/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <libpc.h>

#define sceInsockDisableSocketSymbolAliases
#include <libinsck.h>

#define NETBUFSIZE 4096
static u_int* g_net_buf = HK_NULL;
static sceSifMClientData g_cd;

void hkPs2Socket::close()
{
	if(m_socket != INVALID_SOCKET)
	{
		sceInsockShutdown(m_socket, SCE_INSOCK_SHUT_RDWR);
		m_socket = INVALID_SOCKET;
	}
}

hkResult hkPs2Socket::createSocket()
{
	m_socket = sceInsockSocket(SCE_INSOCK_AF_INET, SCE_INSOCK_SOCK_STREAM, 0);
	if(m_socket < 0)
	{
		HK_WARN(0x683f986a, "Error creating socket!");
		return HK_FAILURE;
	}
	return HK_SUCCESS;
}

// It is not yet possible to setup a client socket on PS2...
// There has not yet been a need to implement this...
hkResult hkPs2Socket::connect(const char* servername, int portNumber)
{
	HK_ASSERT2(0x454742e5, 0, "Client socket creation in hkBase PS2 Inet has not yet been implemented");
	return HK_FAILURE;
}

int hkPs2Socket::read(void* buf, int nbytes)
{
	if(m_socket != INVALID_SOCKET)
	{
		long n = sceInsockRecv(m_socket, static_cast<char*>(buf), unsigned(nbytes), 0);
		if(n > 0)
		{
			return int(n);
		}
		HK_WARN(0x4bb09a0f, "Read fail! Was the receiving end of socket closed?");
		close();
	}
	return 0;
}

int hkPs2Socket::write(const void* buf, int nbytes)
{
	if(m_socket != INVALID_SOCKET)
	{
		long n = sceInsockSend(m_socket, static_cast<const char*>(buf), unsigned(nbytes), 0);
		if( n > 0 )
		{
			return int(n);
		}
		HK_WARN(0x4cb4c0c7, "Write fail! Was the receiving end of socket closed?");
		close();
	}
	return 0;
}

hkResult hkPs2Socket::listen(int port)
{
	if( createSocket() != HK_SUCCESS)
	{
		return HK_FAILURE;
	}

	struct sceInsockSockaddrIn sin;
	hkString::memSet(&sin, 0, sizeof(sin));
	sin.sin_family = SCE_INSOCK_AF_INET;
	sin.sin_port = sceInsockHtons((unsigned short)port);

	if( sceInsockBind(m_socket, (struct sceInsockSockaddr*)&sin, sizeof(sin)) < 0 )
	{
		HK_WARN(0x1575b87d, "Error binding to socket!");
		close();
		return HK_FAILURE;
	}
	
	if( sceInsockListen(m_socket, 2) < 0 )
	{
		HK_WARN(0x1e06b822, "Error listening to socket!");
		close();
		return HK_FAILURE;
	}
	return HK_SUCCESS;
}

hkSocket* hkPs2Socket::pollForNewClient()
{
	if( m_socket != INVALID_SOCKET )
	{
		sceInetPollFd_t	fds;
		fds.cid = m_socket;
		fds.events = sceINET_POLLIN;

		// poll for net client connections
		if(sceInsockPoll(&fds, 1, 0) >= 0 )
		{
			if(fds.revents & sceINET_POLLIN) 
			{
				struct sceInsockSockaddrIn sin;
				unsigned sinLen = sizeof(sin);
				int client_socket = sceInsockAccept(m_socket, (struct sceInsockSockaddr*)&sin, &sinLen);

				if( client_socket > 0 )
				{
					HK_REPORT("[connected] from " << sceInsockInetNtoa(sin.sin_addr) << " " << sceInsockNtohs(sin.sin_port) );
					
					//unsigned int optval = 1;
					//int ret = setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(unsigned int));
					//if(ret < 0) { HK_WARN(0x4dd6610b, "Error accepting a connection!"); }

					return new hkPs2Socket(client_socket);			
				}
				
				HK_WARN(0x120613b4, "Error accepting a connection!");
			}
		}
	}

	return HK_NULL;	
}


void HK_CALL hkPs2NetworkInit()
{
	// *** PERFORMANCE COUNTER ISSUE ***
	// NOTE: The performance timers will and must be restarted when/if the 
	// visual debugger is initialized because setting up the network 
	// connection requires more than 7 seconds if a DHCP lookup is involved,
	// this will cause the performance timers to emit an exception which
	// cannot be caught by the user and therefore results in a crash!
	scePcStop();

#define RETURN_IF_FAIL(A,B,C) HK_MULTILINE_MACRO_BEGIN \
		if((A)<0) { HK_WARN(B,C); return; } \
		HK_MULTILINE_MACRO_END
	
	g_net_buf = (u_int*)hkAlignedAllocate<char>( 64, NETBUFSIZE, HK_MEMORY_CLASS_BASE );

	int status = sceLibnetInitialize( &g_cd, NETBUFSIZE, sceLIBNET_STACKSIZE, sceLIBNET_PRIORITY );
	RETURN_IF_FAIL(status, 0x65702438, "(PS2) Cannot initialize libNet!");

	status = sceLibnetRegisterHandler( &g_cd, g_net_buf );
	RETURN_IF_FAIL(status, 0x15a9bffb, "(PS2) Cannot register SIF handler!");

	status = load_set_conf_extra( &g_cd, g_net_buf,
			hkPs2Socket::s_netDbAbsolutePath, hkPs2Socket::s_combination,
			sceLIBNETF_AUTO_UPIF | sceLIBNETF_DECODE);
	RETURN_IF_FAIL(status, 0x75d7853a, "(PS2) load_set_conf() failed!");
	
	int if_id[sceLIBNET_MAX_INTERFACE];
	struct sceInetAddress myaddr;
	status = sceLibnetWaitGetAddress( &g_cd, g_net_buf, if_id, sceLIBNET_MAX_INTERFACE, &myaddr, sceLIBNETF_AUTO_UPIF );
	RETURN_IF_FAIL(status, 0x2cc21183, "(PS2) wait_get_addr() failed!");

	char myptrname[1024];
	status = sceInetAddress2String( &g_cd, g_net_buf, myptrname, 1024, &myaddr );
	RETURN_IF_FAIL(status, 0x2a918115, "(PS2) sceInetAddress2String() failed!");

	HK_REPORT( "Server IP address is: " << myptrname);
	
}

void HK_CALL hkPs2NetworkQuit()
{
	// shutdown inet	
	int ret;

	ret = sceInetCtlDownInterface( &g_cd, g_net_buf, 0 );
	if ( ret < 0 ) 
	{
		HK_ERROR(0x0, "sceInetCtlDownInterface() failed: " << ret );
	}
	ret = sceLibnetUnregisterHandler( &g_cd, g_net_buf );
	if ( ret < 0 ) 
	{
		HK_ERROR(0x0, "sceLibnetUnregisterHandler() failed: " << ret );
	}
	ret = sceLibnetTerminate( &g_cd );
	if ( ret < 0 ) 
	{
		HK_ERROR(0x0, "sceLibnetTerminate() failed: " << ret );
	}

	hkAlignedDeallocate<char>( (char*)g_net_buf ); g_net_buf = HK_NULL;
}

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
