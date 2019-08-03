//
// Network Error Messages
//

#ifndef _NET_ERROR_H
#define _NET_ERROR_H

enum NETERROR
{
	NETERROR_SUCCESS = 0,
	NETERROR_NODATA,
	NETERROR_CONNRESET,
	NETERROR_UNKNOWN
};

char *GetNetworkErrorString(int iErr);

#define NETLOG_WARNING(s) {ntPrintf("%s(%d): warning - %s\n", __FILE__, __LINE__, s);}

#endif // _NET_ERROR_H
