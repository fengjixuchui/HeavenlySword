/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <libpc.h>

#include <sifrpc.h>
#include <sifdev.h>
#include <sys/types.h>
#include <eekernel.h>

#include <libeenet.h>
#include <libeenet/sys/select.h>
#include <libeenet/sys/socket.h>
#include <libeenet/eenetctl.h>
#include <libeenet/ifaddrs.h>
#include <libeenet/net/if.h>
#include <libeenet/netinet/in.h>
#include <libeenet/arpa/inet.h>

typedef int socket_t;

// EENet mem pool
#define EENET_MEMSIZE	(384*1024) // 384KB, should be big enough. Need to do some stats to figure out normal usage with VDB.
#define EENET_TPL		32
#define EENET_APP_PRIO	48
static u_int* g_eenet_buf = HK_NULL;

// EENet Event handling (so we wait for success)
static int sema_id = 0;
static int event_flag = 0;
#define Ev_Attach         0x01
#define Ev_UpCompleted    0x02
#define Ev_DownCompleted  0x04
#define Ev_DetachCompleted 0x08

// RPC to the IOP (ent_cnf.irx (with decode turned on) )
#define EENET_IFNAME "smap0" // the ethernet adapater
#define ENT_CNF_SIFRPC_NUM			0x0a31108e
#define ENT_CNF_SIFRPC_LOAD_CONFIG      1
#define ENT_CNF_SIFRPC_SET_CONFIG       2
#define ENT_CNF_SIFRPC_SET_CONFIG_ADDR  3
#define ee_rpc_size(size) ((size + 15) & ~15)
#define iop_rpc_size(size) ((size + 3) & ~3)

#define ENTCNFRPC_BUFSIZE	(512) // bytes
static sceSifClientData g_cd;
static int* g_rpc_buffer = HK_NULL;



//////////////////////////////////////////////////////////////////////////
//
// BSD sockt impl, very similar to normal BSD socket code
//

void hkPs2Socket::close()
{
	if(m_socket != INVALID_SOCKET)
	{
		sceEENetClose(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

hkResult hkPs2Socket::createSocket()
{
	close();
	m_socket = static_cast<socket_t>( sceEENetSocket(AF_INET, SOCK_STREAM, 0) );
	if(m_socket == INVALID_SOCKET)
	{
		HK_WARN(0x3b98e883, "Error creating socket!");
		return HK_FAILURE;
	}
	return HK_SUCCESS;
}


int hkPs2Socket::read(void* buf, int nbytes)
{
	if(m_socket != INVALID_SOCKET)
	{
		int n = sceEENetRecv(m_socket, static_cast<char*>(buf), nbytes, 0);
		if ( n <= 0 )
		{
			HK_WARN(0x4bb09a0f, "Read fail! Was the receiving end of socket closed?");
			close();	
		}
		else
			return n;
	}
	return 0;
}

int hkPs2Socket::write( const void* buf, int nbytes)
{
	if(m_socket != INVALID_SOCKET)
	{
		int n = sceEENetSend(m_socket, static_cast<const char*>(buf), nbytes, 0);
		if(n <= 0)
		{
			HK_WARN(0x4cb4c0c7, "Socket send fail! Was the receiving end of socket closed?");
			close();	
		}
		else
		{
			return n;
		}
	}
	return 0;
}

static hkBool HK_CALL hkIsDigit(int c)
{
	return c >= '0' && c <= '9';
}

#undef connect
hkResult hkPs2Socket::connect(const char* servername, int portNumber)
{
	// find the address of the server
	struct sockaddr_in server;
	{
		hkString::memSet(&server,0,sizeof(server));
		server.sin_family = AF_INET;
		server.sin_port = htons( (unsigned short)portNumber);

		if(hkIsDigit(servername[0]))
		{
			//server.sin_addr.S_un.S_addr = inet_addr(servername);
			server.sin_addr.s_addr = ::inet_addr(servername);
		}
		else
		{
			struct hostent* hp;
			hp = ::gethostbyname(servername);

			if(hp)
			{
				hkString::memCpy(&(server.sin_addr),hp->h_addr,hp->h_length);
			}
			else
			{
				HK_WARN(0x1f2dd0e8, "Invalid server address!");
				return HK_FAILURE;
			}
		}
	}

	if( m_socket == INVALID_SOCKET )
	{
		if (createSocket() != HK_SUCCESS )
		{
			return HK_FAILURE;
		}
	}

	if( sceEENetConnect(m_socket, (struct sockaddr*)&server, sizeof(server)) < 0 )
	{
		HK_WARN(0x46d25e96, "Cannot connect to server!");
		close();
		return HK_FAILURE;
	}
	return HK_SUCCESS;
}

#undef listen
hkResult hkPs2Socket::listen(int port)
{
	if (m_socket < 0)
	{
		return HK_FAILURE;
	}

	// bind to specified port
	struct sockaddr_in local;
	hkString::memSet(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons( (unsigned short)port );

	/*union
	{
		int reuseAddress;
		char data[1];
	} option;
	option.reuseAddress = 1;
	setsockopt ( m_socket, SOL_SOCKET, SO_REUSEADDR, &option.data[0], sizeof(option) );
*/

	if( sceEENetBind(m_socket,(struct sockaddr*)&local,sizeof(local) ) < 0 )
	{
		HK_WARN(0x661cf90d, "Error binding to socket!");
		close();
		return HK_FAILURE;
	}

	// put the server socket into a listening state
	if( sceEENetListen(m_socket,5) < 0 )
	{
		HK_WARN(0x14e1a0f9, "Error listening to socket!");
		close();
		return HK_FAILURE;
	}

	return HK_SUCCESS;
}

hkSocket* hkPs2Socket::pollForNewClient()
{
	HK_ASSERT2( 0x73993156, m_socket != INVALID_SOCKET, "");

	// poll the listener socket for new client sockets
	if( m_socket != INVALID_SOCKET )
	{
		fd_set readFds;
		FD_ZERO(&readFds);
		FD_SET(m_socket, &readFds);

//		fd_set exceptFds;
//		FD_ZERO(&exceptFds);
//		FD_SET(m_socket, &exceptFds);

		// see if there is and client trying to connect
		timeval t;
		t.tv_sec = 0;  /* seconds */
		t.tv_usec = 10; /* microseconds. We give non zero as thread must let eenet do something.. */

		int numHits = sceEENetSelect(FD_SETSIZE, &readFds, (fd_set *)HK_NULL, (fd_set *)HK_NULL, &t);
		if( (numHits > 0) && FD_ISSET(m_socket, &readFds) )
		{
			struct sockaddr_in from;
			socklen_t fromlen = sizeof(from);

			socket_t s = static_cast<socket_t>( sceEENetAccept(m_socket, (struct sockaddr*)&from, &fromlen) );

			if( s < 0 )
			{
				HK_WARN(0x774fad25, "Error accepting a connection!");
			}
			else
			{
				// Add the current connection to the servers list
			//	unsigned int optval = 1;
			//	::setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, sizeof (unsigned int));

				return new hkPs2Socket(s);
			}
		}
		else if(numHits < 0)
		{
			HK_WARN(0x3fe16171, "select() error " << sceEENetErrno);
		}
	}

	return HK_NULL;
}



/////////
//
// Event handling used by the default setup
//

#define WaitEvent(event) \
	while (1) { \
	WaitSema(sema_id); \
	if(event_flag & (event)) \
	break; \
	} \

static int _createSemaphore(int init_count, int max_count)
{
	struct SemaParam sema_param;
	int sid;

	hkString::memSet(&sema_param, 0, sizeof(struct SemaParam));
	sema_param.initCount = init_count;
	sema_param.maxCount = max_count;
	sid = CreateSema(&sema_param);

	return sid;
}

static void _netEventHandler(const char *ifname, int af, int type)
{
	switch (type) {
	case sceEENETCTL_IEV_Attach:
		if (strcmp(ifname, EENET_IFNAME))
			break;
		event_flag |= Ev_Attach;
		SignalSema(sema_id);
		break;
	case sceEENETCTL_IEV_UpCompleted:
		event_flag |= Ev_UpCompleted;
		SignalSema(sema_id);
		break;
	case sceEENETCTL_IEV_DownCompleted:
		event_flag |= Ev_DownCompleted;
		SignalSema(sema_id);
		break;
	case sceEENETCTL_IEV_DetachCompleted:
		if (strcmp(ifname, EENET_IFNAME))
			break;
		event_flag |= Ev_DetachCompleted;
		SignalSema(sema_id);
		break;
	}

	return;
}

//////////////////////////////
//  
// RPC for cnf data, used by the default setup
//

static int _bindSifRpc(sceSifClientData *bd, unsigned int request, unsigned int mode)
{
	int i;

	for (;;) {
		if (sceSifBindRpc(bd, request, mode) < 0) 
		{
			HK_ERROR(0x0,"_bindSifRpc error.");
			return (-1);
		}
		if (bd->serve != 0)
		{
			break;
		}
		i = 0x10000;
		while (--i);
	}
	return (0);
}

static int _callSifRpc(sceSifClientData *bd, unsigned int fno,
					   unsigned int mode, void *send, int ssize, void *receive, int rsize,
					   sceSifEndFunc end_func, void *end_parm)
{
	int i;

	for (;;) 
	{
		if (sceSifCallRpc(bd, fno, mode, send, ssize, receive, rsize, end_func, end_parm) == 0) 
		{
			break;
		}
		i = 0x10000;
		while (--i);
	}
	return (0);
}

static int _entCnfInit(void)
{
	if ((g_rpc_buffer = (int*)hkAlignedAllocate<int>( 64, ENTCNFRPC_BUFSIZE, HK_MEMORY_CLASS_BASE )) == NULL) 
	{
		HK_ERROR(0x0, "hkAlignedAllocate() failed for eenet cnf rpc buffer alloc.");
	}

	return _bindSifRpc(&g_cd, ENT_CNF_SIFRPC_NUM, 0);
}

static int _entCnfLoadConfig(const char *fname, const char *usr_name)
{
	int len;

	hkString::strCpy((char *)g_rpc_buffer, fname);
	len = hkString::strLen(fname) + 1;
	hkString::strCpy((char *)g_rpc_buffer + len, usr_name);
	len += hkString::strLen(usr_name) + 1;

	_callSifRpc( &g_cd, ENT_CNF_SIFRPC_LOAD_CONFIG, 0, (void *)g_rpc_buffer,	ee_rpc_size(len),
		(void *)g_rpc_buffer, ee_rpc_size(sizeof(u_int)), 0, 0 );

	return (g_rpc_buffer[0]);
}

static int _entCnfSetConfig()
{
	_callSifRpc( &g_cd, ENT_CNF_SIFRPC_SET_CONFIG,	0,
		NULL, 0, (void *)g_rpc_buffer, ee_rpc_size(sizeof(u_int)),
		0, 0 );

	return (g_rpc_buffer[0]);
}

static int _entCnfSetConfigAddr(u_int addr)
{
	g_rpc_buffer[0] = (int)addr;

	_callSifRpc( &g_cd, ENT_CNF_SIFRPC_SET_CONFIG_ADDR,
		0,	(void *)g_rpc_buffer,
		ee_rpc_size(sizeof(u_int)),	(void *)g_rpc_buffer,
		ee_rpc_size(sizeof(u_int)),	0, 0	);

	return (g_rpc_buffer[0]);
}

//////////////////////////////////////////////////////////////////////////
//
// Default EE Net setup
//

void HK_CALL hkPs2NetworkInit()
{

	// *** PERFORMANCE COUNTER ISSUE ***
	// NOTE: The performance timers will and must be restarted when/if the 
	// visual debugger is initialized because setting up the network 
	// connection requires more than 7 seconds if a DHCP lookup is involved,
	// this will cause the performance timers to emit an exception which
	// cannot be caught by the user and therefore results in a crash!
	scePcStop();


	int ret;
	if ((sema_id = _createSemaphore(0, 255)) < 0) 
	{
		HK_ERROR(0x0, "_createSemaphore() failed.");
	}

	/* initialize eenet library */
	if ((g_eenet_buf = (u_int*)hkAlignedAllocate<char>( 64, EENET_MEMSIZE, HK_MEMORY_CLASS_BASE )) == NULL) 
	{
		HK_ERROR(0x0, "hkAlignedAllocate() failed for eenet mem pool alloc.");
	}
	if ((ret = sceEENetInit(g_eenet_buf, EENET_MEMSIZE, EENET_TPL, 8192, EENET_APP_PRIO)) < 0) 
	{
		HK_ERROR(0x0, "sceEENetInit() failed.\n");
	}

#ifdef HK_DEBUG
	sceEENetSetLogLevel(EENET_LOG_DEBUG);
#endif

	const int autoUp = 1; // 1 == off
	if ((ret = sceEENetCtlInit(8192, EENET_APP_PRIO, 8192, EENET_APP_PRIO, 8192, EENET_APP_PRIO, 1, autoUp)) < 0) 
	{
		HK_ERROR(0x0, "sceEENetCtlInit() failed.\n");
	}

	if ((ret = sceEENetCtlRegisterEventHandler(_netEventHandler)) < 0) 
	{
		HK_ERROR(0x0, "sceEENetCtlRegisterEventHandler() failed.\n");
	}

	if ((ret = _entCnfInit()) < 0) 
	{
		HK_ERROR(0x0, "_entCnfInit() failed.\n");
	}

	/* register device to eenet */
	if ((ret = sceEENetDeviceSMAPReg(8192, 8192)) < 0) 
	{
		HK_ERROR(0x0, "ret of sceEENetDeviceSMAPReg() = " << ret);
	}

	WaitEvent(Ev_Attach);


	if ((ret = _entCnfLoadConfig(hkPs2Socket::s_netDbAbsolutePath, hkPs2Socket::s_combination)) < 0) 
	{
		HK_ERROR(0x0, "_entCnfLoadConfig() failed.");
	}

	if ((ret = _entCnfSetConfig()) < 0) 
	{
		HK_ERROR(0x0, "_entCnfSetConfig() failed.");	
	}

	if ((ret = sceEENetCtlUpInterface(EENET_IFNAME)) < 0) 
	{
		HK_ERROR(0x0,"sceEENetCtlUpInterface() failed.");
	}

	/* wait for interface initialization completed */
	WaitEvent(Ev_UpCompleted);
}


void HK_CALL hkPs2NetworkQuit();
{	
	int ret;

	sceEENetFreeThreadinfo(GetThreadId());

	/* down interface */
	if ((ret = sceEENetCtlDownInterface(EENET_IFNAME)) < 0) 
	{
		HK_ERROR(0x0,"sceEENetCtlDownInterface() failed.\n");
	}

	WaitEvent(Ev_DownCompleted);

	if ((ret = sceEENetDeviceSMAPUnreg()) < 0) 
	{
		HK_ERROR(0x0,"sceEENetDeviceSMAPUnreg() failed.\n");
	}

	WaitEvent(Ev_DetachCompleted);

	if ((ret = sceEENetCtlUnregisterEventHandler(_netEventHandler)) < 0) 
	{
		HK_ERROR(0x0,"sceEENetCtlUnRegisterEventHandler() failed.\n");
	}
	if ((ret = sceEENetCtlTerm()) < 0) 
	{
		HK_ERROR(0x0,"sceEENetCtlTerm() failed.\n");
	}
	if ((ret = sceEENetTerm()) < 0) 
	{
		HK_ERROR(0x0,"sceEENetTerm() failed.\n");
	}

	hkAlignedDeallocate<char>( (char*)g_eenet_buf ); g_eenet_buf= HK_NULL;
	hkAlignedDeallocate<char>( (char*)g_rpc_buffer ); g_rpc_buffer= HK_NULL;
	DeleteSema(sema_id); sema_id = 0;
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
