/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Common header between PPU and SPU for defining audit communication structures
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_AUDIT_TYPES_H
#define WWS_JOB_AUDIT_TYPES_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>
#include <jobsystem/helpers.h>

//--------------------------------------------------------------------------------------------------

struct Audit	// 8(+) bytes, 8 byte aligned
{
	U32				m_time;				///< clock / 150(depending on hardware).  Increases with time, and wraps
	unsigned short	m_numDwords	:3;		///< # of additional dwords in audit (if 7 then get value from m_data)
	unsigned short	m_id		:13;    ///< audit id
	U16				m_data;				///< misc data, defined per audit

	// If m_numDwords > 0, then additional audit data follows here (in multiples of 8 bytes)

} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

struct AuditBufferInputHeader
{
	U32		m_bJobManagerAuditsEnabled	WWSJOB_ALIGNED( 16 );	//Plenty of padding to spare here
	U32		m_bJobAuditsEnabled			WWSJOB_ALIGNED( 16 );	//Plenty of padding to spare here
	U32		m_bufferNumOffset			WWSJOB_ALIGNED( 16 );	//Plenty of padding to spare here
	U32		m_eachSpuBufferSize			WWSJOB_ALIGNED( 16 );	//Plenty of padding to spare here
} WWSJOB_ALIGNED( 16 );

//--------------------------------------------------------------------------------------------------

struct AuditBufferOutputHeader
{
	U32		m_numDwordsWritten;	//Track how many dwords have been written to this SPU's audit buffer
} WWSJOB_ALIGNED( 16 );

//--------------------------------------------------------------------------------------------------

struct AuditBufferHeader
{
	enum
	{
		kMaxSpus		= 6,
		kMaxBuffers		= 2,
	};
	AuditBufferInputHeader	m_inputHeader;							//This is pulled in by each SPU
	AuditBufferOutputHeader	m_outputHeader[kMaxBuffers * kMaxSpus];	//Each of these is written out by the appropriate SPU

	//The output array can take at most kMaxBuffers * kMaxSpus.
	//Note that if this is assigned to say 3 SPUs, double buffered it would be in the format:
	// SPU0-Buff0, SPU1-Buff0, SPU2-Buff0, SPU0-Buff1, SPU1-Buff1, SPU2-Buff1

} WWSJOB_ALIGNED( 128 );	//Audit data following must start 128 byte aligned


//--------------------------------------------------------------------------------------------------

//This structure is written out as the header on the file that is passed to PA Suite
//The rest of the file is then just a direct dump of the user's auit buffer
struct AuditFileHeader
{
	static const U32	kAuditFileVersionNumber = 1;

	U32					m_numSpus;
	U32					m_fileVersionNumber;
	U32					m_spuThreadIds[6];

	static const U32	kMaxWorkloads = 16;
	static const U32	kMaxNameLength = 16;
	char				m_name[kMaxWorkloads][kMaxNameLength];

} WWSJOB_ALIGNED( 16 );

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_AUDIT_TYPES_H */
