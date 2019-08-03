//------------------------------------------------------
//!
//!	\file core\uuid_ps3.cpp
//!
//------------------------------------------------------

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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netex/ifctl.h>
#include <sys/sys_time.h>
#include "core/uuid_ps3.h"

/* various forward declarations */
static int read_state(uint16_t *clockseq, uuid_time_t *timestamp,
    uuid_node_t *node);
static void write_state(uint16_t clockseq, uuid_time_t timestamp,
    uuid_node_t node);
static void format_uuid_v1(uuid_t *uuid, uint16_t clockseq,
    uuid_time_t timestamp, uuid_node_t node);
static void get_ieee_node_identifier(uuid_node_t *node);

static void get_current_time(uuid_time_t *timestamp);
static uint16_t true_random(void);
static void get_system_time(uuid_time_t *uuid_time);

/* set the following to the number of 100ns ticks of the actual
   resolution of your system's clock */
#define UUIDS_PER_TICK 1024

/* uuid_create -- generator a UUID */
int uuid_create(uuid_t *uuid)
{
     uuid_time_t timestamp, last_time;
     uint16_t clockseq;
     uuid_node_t node;
     uuid_node_t last_node;
     int f;

     /* acquire system-wide lock so we're alone */
//     LOCK;
     /* get time, node ID, saved state from non-volatile storage */
     get_current_time(&timestamp);
     get_ieee_node_identifier(&node);
     f = read_state(&clockseq, &last_time, &last_node);

     /* if no NV state, or if clock went backwards, or node ID
        changed (e.g., new network card) change clockseq */
     if (!f || memcmp(&node, &last_node, sizeof node))
         clockseq = true_random();
     else if (timestamp < last_time)
         clockseq++;

     /* save the state for next time */
     write_state(clockseq, timestamp, node);

//    UNLOCK;

     /* stuff fields into the UUID */
     format_uuid_v1(uuid, clockseq, timestamp, node);
     return 1;
}

/* format_uuid_v1 -- make a UUID from the timestamp, clockseq,
                     and node ID */
void format_uuid_v1(uuid_t* uuid, uint16_t clock_seq,
                    uuid_time_t timestamp, uuid_node_t node)
{
    /* Construct a version 1 uuid with the information we've gathered
       plus a few constants. */
    uuid->time_low = (unsigned long)(timestamp & 0xFFFFFFFF);
    uuid->time_mid = (unsigned short)((timestamp >> 32) & 0xFFFF);
    uuid->time_hi_and_version =

        (unsigned short)((timestamp >> 48) & 0x0FFF);
    uuid->time_hi_and_version |= (1 << 12);
    uuid->clock_seq_low = clock_seq & 0xFF;
    uuid->clock_seq_hi_and_reserved = (clock_seq & 0x3F00) >> 8;
    uuid->clock_seq_hi_and_reserved |= 0x80;
    NT_MEMCPY(&uuid->node, &node, sizeof uuid->node);
}

/* data type for UUID generator persistent state */
typedef struct {
    uuid_time_t  ts;       /* saved timestamp */
    uuid_node_t  node;     /* saved node ID */
    uint16_t   cs;       /* saved clock sequence */
} uuid_state;

static uuid_state st;

/* read_state -- read UUID generator state from non-volatile store */
int read_state(uint16_t *clockseq, uuid_time_t *timestamp,
               uuid_node_t *node)
{
    static int inited = 0;

    /* only need to read state once per boot */
    if (!inited) {
		char buffer[ 1024 ];
		Util::GetFiosFilePath( "uuid_state", buffer );
		File fp( buffer, File::FT_READ | File::FT_BINARY );
		if( !fp.IsValid() )
			return 0;
		fp.Read( (char*)&st, sizeof(st) );

        inited = 1;
    }
    *clockseq = st.cs;
    *timestamp = st.ts;
    *node = st.node;
    return 1;
}

/* write_state -- save UUID generator state back to non-volatile
   storage */
void write_state(uint16_t clockseq, uuid_time_t timestamp,
                 uuid_node_t node)
{
    static int inited = 0;
    static uuid_time_t next_save;

    if (!inited) {
        next_save = timestamp;
        inited = 1;
    }

    /* always save state to volatile shared state */
    st.cs = clockseq;
    st.ts = timestamp;
    st.node = node;
    if (timestamp >= next_save) {
		char buffer[ 1024 ];
		Util::GetFiosFilePath( "uuid_state", buffer );
		File fp( buffer, File::FT_WRITE | File::FT_BINARY );
		if( !fp.IsValid() )
			return;
		fp.Write( (char*)&st, sizeof(st) );

        /* schedule next save for 10 seconds from now */
        next_save = timestamp + (10 * 10 * 1000 * 1000);
    }
}

/* get-current_time -- get time as 60-bit 100ns ticks since UUID epoch.
   Compensate for the fact that real clock resolution is
   less than 100ns. */
void get_current_time(uuid_time_t *timestamp)
{
    static int inited = 0;
    static uuid_time_t time_last;
    static uint16_t uuids_this_tick;
    uuid_time_t time_now;

    if (!inited) {
        get_system_time(&time_now);
        uuids_this_tick = UUIDS_PER_TICK;
        inited = 1;
    }

    for ( ; ; ) {
        get_system_time(&time_now);

        /* if clock reading changed since last UUID generated, */
        if (time_last != time_now) {
            /* reset count of uuids gen'd with this clock reading */
            uuids_this_tick = 0;
            time_last = time_now;
            break;
        }
        if (uuids_this_tick < UUIDS_PER_TICK) {
            uuids_this_tick++;
            break;
        }

        /* going too fast for our clock; spin */
    }
    /* add the count of uuids to low order bits of the clock reading */
    *timestamp = time_now + uuids_this_tick;
}

/* true_random -- generate a crypto-quality random number.
   **This sample doesn't do that.** */
static uint16_t true_random(void)
{
    static int inited = 0;
    uuid_time_t time_now;

    if (!inited) {
        get_system_time(&time_now);
        time_now = time_now / UUIDS_PER_TICK;
        srand((unsigned int)
               (((time_now >> 32) ^ time_now) & 0xffffffff));
        inited = 1;
    }

    return rand();
}

/* uuid_compare --  Compare two UUID's "lexically" and return */
#define CHECK(f1, f2) if (f1 != f2) return f1 < f2 ? -1 : 1;
int uuid_compare(uuid_t *u1, uuid_t *u2)
{
    int i;

    CHECK(u1->time_low, u2->time_low);
    CHECK(u1->time_mid, u2->time_mid);

    CHECK(u1->time_hi_and_version, u2->time_hi_and_version);
    CHECK(u1->clock_seq_hi_and_reserved, u2->clock_seq_hi_and_reserved);
    CHECK(u1->clock_seq_low, u2->clock_seq_low)
    for (i = 0; i < 6; i++) {
        if (u1->node[i] < u2->node[i])
            return -1;
        if (u1->node[i] > u2->node[i])
            return 1;
    }
    return 0;
}
#undef CHECK

void uuid_tostring( uuid_t* uuid, char* pBuffer )
{
	// convert the uuid into pBuffer
	sprintf(pBuffer,
			"%08x-%04hx-%04hx-%02hx%02hx-%02hx%02hx%02hx%02hx%02hx%02hx",
			uuid->time_low,
			uuid->time_mid,
			uuid->time_hi_and_version,
			uuid->clock_seq_hi_and_reserved,
			uuid->clock_seq_low,
			uuid->node[0],
			uuid->node[1],
			uuid->node[2],
			uuid->node[3],
			uuid->node[4],
			uuid->node[5]
	);
}

//#define TEST

// takes a character that hopefully is ['0'-'9' || 'A'-'F' || 'a'-'f'] in which case it return 0-15
// if its a bad character returns -1
int HexCharToInt( const char cChar )
{
	int iChar = (int) cChar;

	iChar -= '0'; // fast convert an ascii num to integer
	if( iChar < 0 )
	{
		return -1;
	}
	// if we are greater than 9 we could be a hex character
	if( iChar > 9 )
	{
		iChar = (int) cChar;
		iChar &= 0x5f; // magic upper case 
		iChar -= 'A'; // make us 0 based
		if( iChar < 0 )
		{
			return -1;
		}
		else if( iChar > 5 )
		{
			return -1;
		}
		// turn it into 10-15
		iChar += 10;
	} 

	return iChar;
}

unsigned int HexStringToInt( const char* pString, const int iMaxChars = 32 )
{
	unsigned int acc = 0;

	int iIndex = 0;
	while( pString[iIndex] != 0 && iIndex < iMaxChars )
	{
		int tmp = HexCharToInt( pString[iIndex] );
		iIndex++;

		if( tmp >= 0 )
		{
			acc <<= 4;
			acc += tmp;
		} else
		{
			return acc;
		}
	}

	return acc;
}

void uuid_fromstring( uuid_t* uuid, const char* pBuffer )
{
	// 000000000011111111112222222222333333
	// 012345678901234567890123456789012345
	// d27ef222-2004-4884-868f-2971b4d6a637


	int iLen = strlen(pBuffer);
	// this is meant to be a faster load not using sscanf...

	// we need to validate this is a guid, I use string length
	// and dashes in the appropopiate places... its just possible
	// something might pass this still (i.e. have non hex-decimal
	// chars) but meh...
	if( (iLen != 36) ||
		(pBuffer[ 8] != '-') ||
		(pBuffer[13] != '-') ||
		(pBuffer[18] != '-') ||
		(pBuffer[23] != '-') )
	{
		// clear it so you get the null guid anyway
		memset( uuid, 0, sizeof(uuid_t) );
		return;
	}

	uuid->time_low =						(uint32_t)HexStringToInt( &pBuffer[0] );
	uuid->time_mid =						(uint16_t)HexStringToInt( &pBuffer[9] );
	uuid->time_hi_and_version =				(uint16_t)HexStringToInt( &pBuffer[14] );
	uuid->clock_seq_hi_and_reserved =		(uint8_t)HexStringToInt( &pBuffer[19], 2  );
	uuid->clock_seq_low =					(uint8_t)HexStringToInt( &pBuffer[21], 2 );
	uuid->node[0] =							(uint8_t) HexStringToInt( &pBuffer[24], 2 );
	uuid->node[1] =							(uint8_t) HexStringToInt( &pBuffer[26], 2 );
	uuid->node[2] =							(uint8_t) HexStringToInt( &pBuffer[28], 2 );
	uuid->node[3] =							(uint8_t) HexStringToInt( &pBuffer[30], 2 );
	uuid->node[4] =							(uint8_t) HexStringToInt( &pBuffer[32], 2 );
	uuid->node[5] =							(uint8_t) HexStringToInt( &pBuffer[34], 2 );

#if defined(TEST)

	uuid_t* pOld = uuid;
	uuid_t test_uuid;
	uuid = &test_uuid;

	uint16_t buf[8]; // 16 bit for alignment reasons
	// convert the pBuffer into uuid
	sscanf(	pBuffer,
			"%08x-%04hx-%04hx-%02hx%02hx-%02hx%02hx%02hx%02hx%02hx%02hx",
			&uuid->time_low,
			&uuid->time_mid,
			&uuid->time_hi_and_version,
			&buf[0],
			&buf[1],
			&buf[2],
			&buf[3],
			&buf[4],
			&buf[5],
			&buf[6],
			&buf[7]
	);

    uuid->clock_seq_hi_and_reserved = (uint8_t)buf[0];
	uuid->clock_seq_low = (uint8_t)buf[1];
	uuid->node[0] = (uint8_t)buf[2];
	uuid->node[1] = (uint8_t)buf[3];
	uuid->node[2] = (uint8_t)buf[4];
	uuid->node[3] = (uint8_t)buf[5];
	uuid->node[4] = (uint8_t)buf[6];
	uuid->node[5] = (uint8_t)buf[7];

	ntAssert( uuid_compare(pOld, uuid) == 0 );
#endif
}
 
/* system dependent call to get IEEE node ID.
	This using the PS3 network library to obtain the HW MAC address
*/
void get_ieee_node_identifier(uuid_node_t *node)
{
    static int inited = 0;
    static uuid_node_t saved_node;

    if (!inited) {

		int state;
		int result = cellNetCtlGetState(&state);
		ntError_p( result == CELL_OK, ("Error getting network state: %i",result) );

		union CellNetCtlInfo netInfo;
		result = cellNetCtlGetInfo(CELL_NET_CTL_INFO_ETHER_ADDR, &netInfo);
		ntError_p( result == CELL_OK, ("Error getting MAC address: %i",result) );

		NT_MEMCPY( &saved_node, netInfo.ether_addr.data, CELL_NET_CTL_ETHER_ADDR_LEN );
        inited = 1;
    }

    *node = saved_node;
}

void get_system_time(uuid_time_t *uuid_time)
{
	system_time_t tp;

	// PS3 doesn't seem to have a RTC at the mo... so we 
	// get the system time, this is obviously a fairly bad idea
	// well do for now but long term need a better solution...
	tp = sys_time_get_system_time();

    *uuid_time = (uuid_time_t)tp;
}
