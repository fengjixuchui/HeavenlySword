//------------------------------------------------------
//!
//!	\file core\uuid_ps3.h
//!
//------------------------------------------------------
#if !defined(CORE_UUID_PS3_H)
#define CORE_UUID_PS3_H

#include <netex/libnetctl.h>

/*
** Copyright (c) 1990- 1993, 1996 Open Software Foundation, Inc.
** Copyright (c) 1989 by Hewlett-Packard Company, Palo Alto, Ca. &
** Digital Equipment Corporation, Maynard, Mass.
** Copyright (c) 1998 Microsoft.
** To anyone who acknowledges that this file is provided "AS IS"
** without any express or implied warranty: permission to use, copy,
** modify, and distribute this file for any purpose is hereby
** granted without fee, provided that the above copyright notices and
** this notice appears in all source code copies, and that none of
** the names of Open Software Foundation, Inc., Hewlett-Packard
** Company, Microsoft, or Digital Equipment Corporation be used in
** advertising or publicity pertaining to distribution of the software
** without specific, written prior permission. Neither Open Software
** Foundation, Inc., Hewlett-Packard Company, Microsoft, nor Digital
** Equipment Corporation makes any representations about the
** suitability of this software for any purpose.
*/

typedef struct {
    uint32_t	time_low;
    uint16_t	time_mid;
    uint16_t	time_hi_and_version;
    uint8_t		clock_seq_hi_and_reserved;
    uint8_t		clock_seq_low;
    uint8_t		node[CELL_NET_CTL_ETHER_ADDR_LEN];
} uuid_t;

typedef uint64_t uuid_time_t;
typedef struct {
    char nodeID[CELL_NET_CTL_ETHER_ADDR_LEN];
} uuid_node_t;

/* uuid_create -- generate a UUID */
int uuid_create(uuid_t * uuid);

/* uuid_compare --  Compare two UUID's "lexically" and return
        -1   u1 is lexically before u2
         0   u1 is equal to u2
         1   u1 is lexically after u2
   Note that lexical ordering is not temporal ordering!
*/
int uuid_compare(uuid_t *u1, uuid_t *u2);

//! puts a text version of the uuid into pBuffer, pBuffer must be at least 32 chars long
void uuid_tostring( uuid_t* uuid, char* pBuffer );

//! fills in uuid from the text buffer
void uuid_fromstring( uuid_t* uuid, const char* pBuffer );

#endif // end CORE_UUID_PS3_H
