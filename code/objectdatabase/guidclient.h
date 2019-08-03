//------------------------------------------------------
//!
//!	\file core\guidclient.h
//!
//------------------------------------------------------

#ifndef _GUID_CLIENT_H
#define _GUID_CLIENT_H

#if defined( PLATFORM_PC )
#	include "objectdatabase/guidclient_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "objectdatabase/guidclient_ps3.h"
#endif

#endif
