//
// Network Error Messages
//

#ifdef PLATFORM_PC

#include <winsock2.h>

char *GetNetworkErrorString(int iErr)
{
	switch(iErr)
	{
	case WSANOTINITIALISED:
		return "WSANOTINITIALISED";
		break;
	case WSAENETDOWN:
		return "WSAENETDOWN";
		break;
	case WSAEACCES:
		return "WSAEACCES";
		break;
	case WSAEINVAL:
		return "WSAEINVAL";
		break;
	case WSAEINTR:
		return "WSAEINTR";
		break;
	case WSAEINPROGRESS:
		return "WSAEINPROGRESS";
		break;
	case WSAEFAULT:
		return "WSAEFAULT";
		break;
	case WSAENETRESET:
		return "WSAENETRESET";
		break;
	case WSAENOBUFS:
		return "WSAENOBUFS";
		break;
	case WSAENOTCONN:
		return "WSAENOTCONN";
		break;
	case WSAENOTSOCK:
		return "WSAENOTSOCK";
		break;
	case WSAEOPNOTSUPP:
		return "WSAEOPNOTSUPP";
		break;
	case WSAESHUTDOWN:
		return "WSAESHUTDOWN";
		break;
	case WSAEWOULDBLOCK:
		return "WSAEWOULDBLOCK";
		break;
	case WSAEMSGSIZE:
		return "WSAEMSGSIZE";
		break;
	case WSAEHOSTUNREACH:
		return "WSAEHOSTUNREACH";
		break;
	case WSAECONNABORTED:
		return "WSAECONNABORTED";
		break;
	case WSAECONNRESET:
		return "WSAECONNRESET";
		break;
	case WSAEADDRNOTAVAIL:
		return "WSAEADDRNOTAVAIL";
		break;
	case WSAEAFNOSUPPORT:
		return "WSAEAFNOSUPPORT";
		break;
	case WSAEDESTADDRREQ:
		return "WSAEDESTADDRREQ";
		break;
	case WSAENETUNREACH:
		return "WSAENETUNREACH";
		break;
	case WSAETIMEDOUT :
		return "WSAETIMEDOUT ";
		break;
	}

	return "WSAE? - An unknown ntError has occurred.";
}

#else 

char *GetNetworkErrorString(int iErr)
{
	return "UNKNOWN ERROR";
};

#endif
