/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Manage processing of audits from the PPU side
**/
//--------------------------------------------------------------------------------------------------

#include <sys/process.h>

#include <cell/spurs/control.h>

#include <jobapi/auditmanager.h>
#include <jobapi/embeddedjm.h>
#include <jobapi/jobprintf.h>
#include <jobapi/jobmanagerauditids.h>

#include <string.h>
#include <stdio.h>

//--------------------------------------------------------------------------------------------------

#define AUDIT_DATA( kEnumName, kString )        kString , 
const char* const g_wwsJobManagerAuditText[] =
{
	#include <jobapi/jobmanagerauditdata.inc>
};
#undef AUDIT_DATA 

//--------------------------------------------------------------------------------------------------

void AuditManager::Init( void )
{
	m_pAuditHeader				= NULL;
	m_eachSpuAuditBufferSize	= 0;
	m_numAuditSystems			= 0;
	m_numSpusSharingBigBuffer	= 0xFFFFFFFF;
	m_numBuffers				= 0xFFFFFFFF;
	m_idAllocMarker				= kMaximumAuditId;

	for ( U32 i = 0; i < kMaxSpus; ++i )
	{
		m_cachedU32Exists[i]		= false;
		m_currentNumU64Elements[i]	= 0;
	}

	RegisterAuditData( AuditId::kNopAudit, AuditId::kWwsJobManager_end, "WwsJob_", g_wwsJobManagerAuditText );
}

//--------------------------------------------------------------------------------------------------

AuditManager::AuditManager()
{
	Init();
}

//--------------------------------------------------------------------------------------------------

AuditManager::AuditManager( void* pAuditBufferBase, U32 auditBufferSize, U32 numSpus, WwsJob_NumBuffers numBuffers )
{
	Init();

	InitAuditMemory( pAuditBufferBase, auditBufferSize, numSpus, numBuffers );
}

//--------------------------------------------------------------------------------------------------

void AuditManager::InitAuditMemory( void* pAuditBuffer, U32 auditBufferSize, U32 numSpus, WwsJob_NumBuffers numBuffers )
{
	WWSJOB_ASSERT( m_numSpusSharingBigBuffer == 0xFFFFFFFF );	//Must only be inited once
	WWSJOB_ASSERT( m_numBuffers == 0xFFFFFFFF );				//Must only be inited once
	WWSJOB_ASSERT( WwsJob_IsAligned( pAuditBuffer, 128 ) );
	WWSJOB_ASSERT( WwsJob_IsAligned( auditBufferSize, 128 ) );

	WWSJOB_ASSERT( numSpus <= 6 );
	WWSJOB_ASSERT( (numBuffers == kSingleBuffered) || (numBuffers == kDoubleBuffered) );

	WWSJOB_ASSERT( !sys_process_is_stack( pAuditBuffer ) );//Buffer will be read by SPU so mustn't be on stack

	m_numSpusSharingBigBuffer	= numSpus;
	m_numBuffers				= numBuffers;

	//The buffer passed in starts off with an audit header
	//Then, immediately following the header is an array of separate output buffers, there are numBuffers of buffers for each SPU

	//The array of buffers is layed out in the format:
	// SPU0-Buffer0
	// SPU1-Buffer0
	// SPU2-Buffer0
	// SPU3-Buffer0
	// SPU4-Buffer0
	// SPU5-Buffer0
	// SPU0-Buffer1
	// SPU1-Buffer1
	// SPU2-Buffer1
	// SPU3-Buffer1
	// SPU4-Buffer1
	// SPU5-Buffer1

	//In the SPU, we just apply a bufferNumOffset, and then just add the spuNum to it.

	U32 eachSpuAuditBufferSize;

	if ( pAuditBuffer )
	{
		//Calculate how much space we have available to each SPU
		eachSpuAuditBufferSize = (auditBufferSize - sizeof(AuditBufferHeader)) / (m_numSpusSharingBigBuffer * m_numBuffers);
		//And round down to a multiple of 128
		eachSpuAuditBufferSize = eachSpuAuditBufferSize & 0xFFFFFF80;

		JobPrintf( "AuditManager: Initing Job Manager and Job audits system to use pAuditHeader = 0x%08X, eachSpuAuditBufferSize = 0x%X (numSpus = %d, numBuffers = %d)\n", (U32)pAuditBuffer, eachSpuAuditBufferSize, m_numSpusSharingBigBuffer, m_numBuffers );
	}
	else
	{
		eachSpuAuditBufferSize = 0;

		JobPrintf( "AuditManager: No main memory audit output buffer.  Job and Job Manager audits are disabled.\n" );
	}

	//EmbeddedJobManager::Instance().SetAuditBuffer( pAuditBuffer );
	//EmbeddedJobManager::Instance().SetEachSpuAuditBufferSize( eachSpuAuditBufferSize );

	m_pAuditHeader				= (AuditBufferHeader*) pAuditBuffer;
	m_eachSpuAuditBufferSize	= eachSpuAuditBufferSize;

	if ( m_pAuditHeader )
	{
		m_pAuditHeader->m_inputHeader.m_eachSpuBufferSize	= eachSpuAuditBufferSize;

		//If we've got an audit buffer, make sure it's initialised appropriately
		SetJobManagerAuditsEnabled( false );
		SetJobAuditsEnabled( false );
		SetAuditOutputBufferNum( 0 );
		for ( U32 buffNum = 0; buffNum < m_numBuffers; ++buffNum )
		{
			EmptyAuditBuffersForAllSpus( buffNum );
		}
	}
}

//--------------------------------------------------------------------------------------------------

void AuditManager::SetJobManagerAuditsEnabled( bool enableAudits )
{
	WWSJOB_ASSERT( m_pAuditHeader );
	m_pAuditHeader->m_inputHeader.m_bJobManagerAuditsEnabled		= enableAudits;
}

//--------------------------------------------------------------------------------------------------

void AuditManager::SetJobAuditsEnabled( bool enableAudits )
{
	WWSJOB_ASSERT( m_pAuditHeader );
	m_pAuditHeader->m_inputHeader.m_bJobAuditsEnabled				= enableAudits;
}

//--------------------------------------------------------------------------------------------------

void AuditManager::SetAuditOutputBufferNum( U32 bufferNum )
{
	WWSJOB_ASSERT( m_pAuditHeader );
	WWSJOB_ASSERT( bufferNum < m_numBuffers );
	m_pAuditHeader->m_inputHeader.m_bufferNumOffset					= bufferNum * m_numSpusSharingBigBuffer;
}

//--------------------------------------------------------------------------------------------------

void AuditManager::EmptyAuditBuffer( U32 bufferIndexNum, U32 spuNum )
{
	WWSJOB_ASSERT( m_pAuditHeader );

	WaitForAudits( bufferIndexNum, spuNum );

	U32 bufferNumOffset					= bufferIndexNum * m_numSpusSharingBigBuffer;
	m_pAuditHeader->m_outputHeader[bufferNumOffset + spuNum].m_numDwordsWritten	= 0;
}

//--------------------------------------------------------------------------------------------------

void AuditManager::EmptyAuditBuffersForAllSpus( U32 bufferIndexNum )
{
	for ( U32 spuNum = 0; spuNum < m_numSpusSharingBigBuffer; ++spuNum )
	{
		EmptyAuditBuffer( bufferIndexNum, spuNum );
	}
}

//--------------------------------------------------------------------------------------------------

const void* AuditManager::GetSpuAuditOutputBuffer( U32 bufferIndexNum, U32 spuNum ) const
{
	WWSJOB_ASSERT( bufferIndexNum < m_numBuffers );
	WWSJOB_ASSERT( spuNum < m_numSpusSharingBigBuffer );

	U32 ptr = (U32) m_pAuditHeader;
	ptr += sizeof( AuditBufferHeader );
	U32 bufferNumOffset					= bufferIndexNum * m_numSpusSharingBigBuffer;
	ptr += (m_eachSpuAuditBufferSize * (bufferNumOffset + spuNum) );

	return (const void*)ptr;
}

//--------------------------------------------------------------------------------------------------

U32 AuditManager::GetNumDwordsInAuditOutputBuffer( U32 bufferIndexNum, U32 spuNum ) const
{
	WWSJOB_ASSERT( bufferIndexNum < m_numBuffers );
	WWSJOB_ASSERT( spuNum < m_numSpusSharingBigBuffer );

	U32 bufferNumOffset					= bufferIndexNum * m_numSpusSharingBigBuffer;
	return m_pAuditHeader->m_outputHeader[bufferNumOffset + spuNum].m_numDwordsWritten;
}

//--------------------------------------------------------------------------------------------------

void AuditManager::WaitForAudits( U32 bufferIndexNum, U32 spuNum ) const
{
	while ( GetNumDwordsInAuditOutputBuffer( bufferIndexNum, spuNum ) == 0xFFFFFFFF )
	{
		//Just keep spinning until the value is no longer 0xFFFFFFFF
	}
}

//--------------------------------------------------------------------------------------------------

void AuditManager::WaitForAuditsForAllSpus( U32 bufferIndexNum ) const
{
	for ( U32 spuNum = 0; spuNum < m_numSpusSharingBigBuffer; ++spuNum )
	{
		WaitForAudits( bufferIndexNum, spuNum );
	}
}

//--------------------------------------------------------------------------------------------------

U16 AuditManager::AllocIdRange( U32 numIds )
{
	WWSJOB_ASSERT( (m_idAllocMarker - numIds) >= AuditId::kWwsJobManager_end );

	//Alloc from the end backwards
	m_idAllocMarker -= numIds;

	return m_idAllocMarker;
}

//--------------------------------------------------------------------------------------------------

void AuditManager::RegisterAuditData( U16 baseValidID, U16 endValidIDs /*exclusive*/, const char* systemName, const char* const* pAuditFormatStringArray )
{
	//endValidIDs is the id that is immediately after the finalValidId
	WWSJOB_ASSERT( baseValidID < endValidIDs );

	//Check there's no overlaps with the audits we've registered already
	for ( U32 checkAuditId = 0; checkAuditId < m_numAuditSystems; ++checkAuditId )
	{
		const AuditSystemData* pCheckData = &m_auditSystemData[checkAuditId];

		if ( ( pCheckData->m_baseValidId >= endValidIDs ) || ( pCheckData->m_finalValidId < baseValidID ) )
		{
			//Either pCheckData is fully after the current audits or fully before them, so we're safe
		}
		else
		{
			JobPrintf( "ERROR: \"%s\" [%d->%d] overlaps with \"%s\" [%d->%d)]\n",
							systemName, baseValidID, endValidIDs,
							pCheckData->m_pSystemName, pCheckData->m_baseValidId, pCheckData->m_finalValidId );
			WWSJOB_ASSERT( false );
		}
	}

	WWSJOB_ASSERT( m_numAuditSystems < kMaxAuditSystems );
	AuditSystemData* pAuditSystemData = &m_auditSystemData[m_numAuditSystems];

	pAuditSystemData->m_baseValidId				= baseValidID;
	pAuditSystemData->m_finalValidId			= endValidIDs - 1;
	pAuditSystemData->m_pSystemName				= systemName;
	pAuditSystemData->m_pAuditFormatStringArray	= pAuditFormatStringArray;

	++m_numAuditSystems;
}

//--------------------------------------------------------------------------------------------------

U16 AuditManager::AllocIdRangeAndRegisterAuditData( U32 numIds, const char* systemName, const char* const* pAuditFormatStringArray )
{
	U16 auditBaseId = AllocIdRange( numIds );
	RegisterAuditData( auditBaseId, auditBaseId + numIds, systemName, pAuditFormatStringArray );
	return auditBaseId;
}

//--------------------------------------------------------------------------------------------------

void AuditManager::GetAuditFormatString( U16 id, const char** ppSystem, const char** ppText ) const
{
	for ( U32 systemNum = 0; systemNum < m_numAuditSystems; ++systemNum )
	{
		const AuditSystemData* pAuditSystemData = &m_auditSystemData[systemNum];

		U16 baseValidId						= pAuditSystemData->m_baseValidId;
		U16 finalValidId					= pAuditSystemData->m_finalValidId;
		const char* pSystemName				= pAuditSystemData->m_pSystemName;
		const char* const* pAuditTextArray	= pAuditSystemData->m_pAuditFormatStringArray;

		if ( (baseValidId <= id) && (id <= finalValidId) )
		{
			*ppSystem	= pSystemName;
			*ppText		= pAuditTextArray[id-baseValidId];
			//JobPrintf( "id %d indexed into 0x%08X at index %d to return %s\n", id, pAuditTextArray, id-baseValidId, *ppText );
			return;
		}
	}

	//Failed to find it so return NULL
	*ppSystem	= NULL;
	*ppText		= NULL;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define NUM_SPACES_TO_PARAMS 50

//This is just a helper class used only within this file for accumulating text into a buffer for printing
class AuditManagerStringBuffer
{
public:
						AuditManagerStringBuffer( void )			: m_pPrintBuffer( &m_printBuffer[0] )		{ }
	U32					AppendString( const char* fmt, ... ) __attribute__( ( format( printf, 2, 3 ) ) );
	void				AddSpaces( U32 numSpaces );
	U32					GetCurrentNumChars( void ) const		{ return (U32)(m_pPrintBuffer - &m_printBuffer[0]); }
	void				AddNewLine( void );
	void				IndentToParameters( U32 alreadyIndentedDepth = 0 );

private:
	void				AddChar( char character );
	void				Print( void );
	void				Reset( void )							{ m_pPrintBuffer = &m_printBuffer[0]; }

	static const U32	kStringBufferSize = 255;

	char				m_printBuffer[kStringBufferSize+1];
	char*				m_pPrintBuffer;
};

//--------------------------------------------------------------------------------------------------

U32 AuditManagerStringBuffer::AppendString( const char* fmt, ... )
{
	va_list args;

	U32 spaceLeft = kStringBufferSize+1 - GetCurrentNumChars();
	va_start( args, fmt );
	U32 numCharsPrinted = vsnprintf( m_pPrintBuffer, spaceLeft, fmt, args );
	va_end( args );

	m_pPrintBuffer += numCharsPrinted;

	return numCharsPrinted;
}

//--------------------------------------------------------------------------------------------------

void AuditManagerStringBuffer::AddChar( char character )
{
	if ( GetCurrentNumChars() < kStringBufferSize+1 )
	{
		*m_pPrintBuffer = character;
		++m_pPrintBuffer;
	}
}

//--------------------------------------------------------------------------------------------------

void AuditManagerStringBuffer::AddSpaces( U32 numSpaces )
{
	while ( numSpaces > 0 )
	{
		AddChar( ' ' );
		--numSpaces;
	}
}

//--------------------------------------------------------------------------------------------------

void	AuditManagerStringBuffer::Print( void )
{
	JobBasePrintf( m_printBuffer );
}

//--------------------------------------------------------------------------------------------------

void	AuditManagerStringBuffer::AddNewLine( void )
{
	AddChar( '\n' );
	AddChar( '\0' );
	Print();
	Reset();
}

//--------------------------------------------------------------------------------------------------

void AuditManagerStringBuffer::IndentToParameters( U32 alreadyIndentedDepth )
{
	const U32 paramSpaces = 6/*SPU#: */ + NUM_SPACES_TO_PARAMS;
	WWSJOB_ASSERT( alreadyIndentedDepth < paramSpaces );
	AddSpaces( paramSpaces - alreadyIndentedDepth );
}


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

// find char in string.  If found, return index in string, else return neg value
static I32 FindChar( const char *pText, U32 textIndex, char searchChar )
{
	for( U32 i = textIndex ; 1/*always*/ ; i++ )
	{
		char c = pText[i];
		if( c == searchChar )
		{	// found character
			return(i);
		}

		if( c == '\0' )
		{	// reached end of string
			return -1;
		}
	}
}

//--------------------------------------------------------------------------------------------------

static U32 SkipWhiteSpace( const char *pText, U32 textIndex )
{
	for( U32 i = textIndex ; 1/*always*/ ; i++ )
	{
		// skip tab
		char c = pText[i];
		if( c == '\t' ||  c == ' '  ||  c == ',' )
			continue;
											
		// found non-space
		return(i);
	}
}

//--------------------------------------------------------------------------------------------------

bool AuditManager::PrintAudit( const AuditManager* pAuditManager, U32 spuNum, U16 id, U32 time, bool hwordIsValid, U16 hData, U16 numDwords, const void* pAuditData, void* pUserData )
{
	WWSJOB_UNUSED( pUserData );

	// Accumulate the strings into the printBuffer so that the final print is done at one time
	AuditManagerStringBuffer printBuffer;
	
	// get system & text string for audit id
	const char* pSystem;
	const char* pText;
	pAuditManager->GetAuditFormatString( id, &pSystem, &pText );

	if ( pSystem == NULL )
	{
		//This is just for printing audits that haven't had system data registered for them
		//The user should always register system data, so this shouldn't be seen
		U32 numCharsPrinted = printBuffer.AppendString( "SPU%d: %08X UNKNOWN AUDIT (Id = 0x%03x)", spuNum, time, id );

		printBuffer.IndentToParameters( numCharsPrinted );

		if ( hwordIsValid )
		{
			printBuffer.AppendString( "0x%04X ", hData );
		}

		for ( U16 dwordNo = 0; dwordNo < numDwords; ++dwordNo )
		{
			U64 dword = ((const U64*)pAuditData)[dwordNo];

			// ensure you don't overflow print buffer (this code prints even less so lines aren't too long)
			if ( printBuffer.GetCurrentNumChars() + 2+8+1+8+1 >= 140/* 140 must be <= AuditManagerStringBuffer::kStringBufferSize */ )
			{	// you *must* print buffer as it would overflow below (or just get too long)

				printBuffer.AddNewLine();

				// indent to where dwords were above
				printBuffer.IndentToParameters();
				if( hwordIsValid )
				{
					printBuffer.AppendString( "       " );
				}
			}

			printBuffer.AppendString( "0x%08X_%08X ", (U32)(dword >> 32), (U32)(dword & 0xFFFFFFFF) );
		}

		printBuffer.AddNewLine();

		return false;
	}

	// get 1st & last char index in text
	U32 textIndex = 0;
	U32 textLength = strlen( pText );
	I32 textIndex2; // temp use

	const U32 kMaxNameLength = 80;
	char name[kMaxNameLength+1] = "";
	U32 nameLength;

	// if audit text starts with "/n" then print blank lines here
	while( pText[textIndex] == '\n' )
	{
		printBuffer.AddNewLine();
		textIndex++;
	}

	// get command string
	textIndex2 = FindChar( pText, textIndex, ':' );
	if( textIndex2 >= (I32)textIndex )
	{	// colon found

		// include = with name
		textIndex2++;

		// ensure command name is not too long
		nameLength = textIndex2 - textIndex;
		WWSJOB_ASSERT( nameLength <= kMaxNameLength );

		// get command name with colon
		strncpy( name, pText + textIndex, nameLength );

		// pad name with spaces so it reaches the params
		I32 numPadChars = NUM_SPACES_TO_PARAMS - 9/*time*/ - strlen( pSystem ) - nameLength;
		I32 i;
		if( numPadChars > 0 )
		{
			for( i = 0 ; i < numPadChars ; i++ )
				name[nameLength + i] = ' ';
			name[nameLength + i] = '\0';
		}

		// skip spaces after colon (including comma)
		textIndex = SkipWhiteSpace( pText, textIndex2);
	}

	// print time, system, and command name
	printBuffer.AppendString( "SPU%d: %08X %s%s", spuNum, time, pSystem, name );

	// init data to hword or first dword

	U64 dword;
	I32 numBitsInDword;
	const U64 *pDword = (const U64*) pAuditData;
	if ( hwordIsValid )
	{
		// start with hword, which was not included in count of dwords
		numDwords++;
		dword = ((U64)hData) << (64-16);//(U64)pAudit->m_data << (64-16);
		numBitsInDword = 16;
	}
	else
	{
		dword = *pDword;
		pDword++;
		numBitsInDword = 64;
	}

	// print data fields
	while( textIndex < textLength )
	{
		char c = pText[textIndex + 0];
		char d = pText[textIndex + 1];
		char e = pText[textIndex + 2];
		
		if( c == '\n' )
		{	// got "\n"

			// print \n and indent to parameters
			printBuffer.AddNewLine();

			printBuffer.IndentToParameters();
			textIndex ++;

		}
		else
		{
			WWSJOB_ASSERT( numDwords > 0 );

			if( c == 'p'  &&  d == 'a'  &&  e == 'd' )
			{	// got "pad"

				textIndex += 3;

				c = pText[textIndex + 0];
				if( c >= '0'  &&  c <= '6' )
				{
					d = pText[textIndex + 1];
					I32 numPadBits = (c - '0') * 10 + d - '0';
					WWSJOB_ASSERT( numPadBits >= 0  &&  numPadBits <= 64 );
					dword <<= numPadBits;
					numBitsInDword -= numPadBits;
					textIndex += 2;				
				}
				else
				{
					// ignore rest of data in current dword
					numBitsInDword = 0;
				}
	
				// after a pad, skip input whitespace but do *not* print it
				textIndex = SkipWhiteSpace( pText, textIndex );
			}
			else
			{	
				// print field name (with =)
				textIndex2 = FindChar( pText, textIndex, '=' );
				// we must have field name followed by "="
				WWSJOB_ASSERT( textIndex2 >= 0 );

				nameLength = textIndex2 - textIndex + 1/*include = */;
				WWSJOB_ASSERT( nameLength <= kMaxNameLength );

				strncpy( name, pText + textIndex, nameLength );

				if( nameLength == 2  &&  pText[textIndex] == '_' )
				{	// got "_=" which we'll print as just "_"
					name[1] = '\0';
				}
				else
				{	// normal "name=" is printed that way
					name[nameLength] = '\0';
				}

				printBuffer.AppendString( name );
				textIndex += nameLength;
	
				// now we expect one of: U01 to U64, or I01 to I64, or dump
				c = pText[textIndex + 0];
				d = pText[textIndex + 1];
				e = pText[textIndex + 2];

				if( c == 'U' )
				{	// expect U01 to U64
				
					I32 numBits = (d - '0') * 10 + e - '0';
					WWSJOB_ASSERT( numBits >= 1  &&  numBits <= 64 );

					U64 value = dword >> (64 - numBits);

					if( numBits <= 32 )
					{
						// get name to be "%01X" to "%08X"
						snprintf( name, kMaxNameLength+1, "%%%02dX", (numBits+3) >> 2 );
						printBuffer.AppendString( name, value );
					}
					else
					{
						// get name to be "%08X_%01X" to "%08X_%08X" based on *last* 32 bits
						snprintf( name, kMaxNameLength+1, "%%08X_%%%02dX", (numBits-32+3) >> 2 );
						printBuffer.AppendString( name, value >> 32, value & 0xFFFFFFFF );
					}

					dword <<= numBits;
					numBitsInDword -= numBits;
					textIndex += 3;
				}
				else if( c == 'I' )
				{	// expect I01 to I64
				
					I32 numBits = (d - '0') * 10 + e - '0';
					WWSJOB_ASSERT( numBits >= 1  &&  numBits <= 64 );

					U64 value = dword >> (64 - numBits);
					char sign = '+';

					if( value & (1 << (numBits-1)) )
					{
						value = (~value + 1) & ((1 << numBits) - 1);
						sign = '-';
					}
					if( numBits <= 32 )
					{
						// get name to be "%c%01X" to "%c%08X"
						snprintf( name, kMaxNameLength+1, "%%c%%%02dX", (numBits+3) >> 2 );
						printBuffer.AppendString( name, sign, value );
					}
					else
					{
						// get name to be "%c%08X_%01X" to "%c%08X_%08X" based on *last* 32 bits
						snprintf( name, kMaxNameLength+1, "%%c%%08X_%%%02dX", (numBits-32+3) >> 2 );
						printBuffer.AppendString( name, sign, value >> 32, value & 0xFFFFFFFF );
					}

					dword <<= numBits;
					numBitsInDword -= numBits;
					textIndex += 3;
				}
				else if( c == 'F'  &&  d == '3'  &&  e == '2' )
				{	// got F32

					I32 numBits = 32;

					union F32Etc
					{	F32 m_f32;
						U32 m_u32;
					};
					F32Etc tmpf;
					tmpf.m_u32 = dword >> (64 - numBits);
					printBuffer.AppendString( "%13.6e", tmpf.m_f32 );

					dword <<= numBits;
					numBitsInDword -= numBits;
					textIndex += 3;
				}
				else if( c == 'd'  &&  d == 'u'  &&  e == 'm'  &&  pText[textIndex+3] == 'p')
				{
					// partial dword required a "pad" before a "name=dump"
					WWSJOB_ASSERT( numBitsInDword == 64 );

					// get blank name as big as last name=, but with 4 less chars to make room for index:
					if( nameLength < 4 )
						nameLength = 0;
					else
						nameLength -= 4;
					for( U32 i = 0 ; i < nameLength ; i++ )
						name[i] = ' ';
					name[nameLength] = '\0';
					
					// print 4 dwords per line
					for( U32 i = 0 ; 1/*forever*/ ; i++ )
					{
						if( (i & 3) == 0  &&  i > 0 )
						{
							printBuffer.IndentToParameters();
							printBuffer.AppendString( "%s%03X:", name, i << 3 );
						}
	
						const char* pFormatString;
						if( (i & 3) == 0 )		
							pFormatString = "%08X_%08X";
						else if( (i & 3) == 2 )
							pFormatString = "  %08X_%08X";
						else
							pFormatString = "_%08X_%08X";

						printBuffer.AppendString( pFormatString, (U32)(dword >> 32), (U32)(dword & 0xFFFFFFFF) );

						if( (i & 3) == 3  &&  numDwords > 1 )
						{
							printBuffer.AddNewLine();
						}
							
						if( numDwords == 1 )
						{
							break; // exit "for" loop
						}

						dword = *pDword;
						pDword++;
						numDwords--;
					}

					numBitsInDword = 0;
					textIndex += 4;
				}
				else
				{
					// we expected one of: U01 to U64, or I01 to I64, or dump
					WWSJOB_ASSERT(0);
				}
			}	
		}
		
		// print whitespace, including comma
		textIndex2 = SkipWhiteSpace( pText, textIndex );
		nameLength = textIndex2 - textIndex;
		if( nameLength > 0 )
		{
			WWSJOB_ASSERT( nameLength <= kMaxNameLength );
			strncpy( name, pText + textIndex, nameLength );
			name[nameLength] = '\0';
			printBuffer.AppendString( name );
			textIndex = textIndex2;
		}
		
		// check to get next dword

		// text requested more data than existed in dword
		WWSJOB_ASSERT( numBitsInDword >= 0 );

		if( numBitsInDword == 0 )
		{	// need to get next dword
			// note this may fetch dword which doesn't exist, as long as it's not used

			dword = *pDword;
			pDword++;
			numDwords--;
			numBitsInDword = 64;
		}
	}

	printBuffer.AddNewLine();
	
	if( numBitsInDword < 64 )
	{
		pDword++;
		numDwords--;
	}
					
	// you had extra dwords which you didn't use
	WWSJOB_ASSERT( numDwords == 0 );

	return true;	//return true to say that we handled printing this audit.
}

//--------------------------------------------------------------------------------------------------

void AuditManager::ProcessAuditBuffer( U32 bufferIndexNum, U32 spuNum, U32 maxAuditsToProcess, ProcessAuditFunction func, WwsJob_EnableHeaderPrinting enableHeaderPrinting, void* pUserData ) const
{
	WWSJOB_ASSERT( bufferIndexNum < m_numBuffers );
	WWSJOB_ASSERT( spuNum < m_numSpusSharingBigBuffer );

	WaitForAudits( bufferIndexNum, spuNum );

	//The user calls this function which iterates over the data for them
	//and calls a function pointer to process each individual audit

	//The user will probably pass in AuditManager::PrintAudit for the func pointer,
	//but is allowed to write their own function

	U32 eachSpuAuditBufferSize = m_eachSpuAuditBufferSize;
	const Audit* pAudit = (const Audit*)GetSpuAuditOutputBuffer( bufferIndexNum, spuNum );
	const Audit* pAuditEnd/*excl*/ = pAudit + (eachSpuAuditBufferSize / 8);
	U32 numDwordsToProcess = GetNumDwordsInAuditOutputBuffer( bufferIndexNum, spuNum );

	if ( maxAuditsToProcess == 0 )
	{
		//JobBasePrintf( "Not printing any audits for SPU%d\n", spuNum );
		return;
	}
	if ( numDwordsToProcess == 0 )
	{
		if ( enableHeaderPrinting == kPrintHeaders )
			JobBasePrintf( "No audits to print for SPU%d\n", spuNum );
		return;
	}

	if ( enableHeaderPrinting == kPrintHeaders )
		JobBasePrintf( "Printing Audits buffer for SPU%d (baseTime=0x%08X):\n", spuNum, pAudit->m_time );

	U32 auditIndex = 0;
	while ( pAudit < pAuditEnd  &&  numDwordsToProcess > 0 )
	{
		//U64 *pDbg = (U64*)( (U64)pAudit );
		//JobBasePrintf( "at 0x%lx, audit = 0x%08x_%08x\n", (U64)pDbg, *pDbg >> 32, *pDbg & 0xFFFFFFFF );

		U16 id = pAudit->m_id;
		U32 numDwords = pAudit->m_numDwords;
		bool hwordIsValid = true;
		if( numDwords == 7 )
		{
			numDwords = pAudit->m_data;
			hwordIsValid = false;
		}
		const U32 *pWords	= (const U32*) &pAudit[1];
		U16 hData			= pAudit->m_data;
		U32 time			= pAudit->m_time;

		if ( (pAudit + 1 + numDwords) > pAuditEnd )
		{
			if ( enableHeaderPrinting == kPrintHeaders )
				JobBasePrintf( "***NOTE: SPU%d Audit buffer overflowed.  Data lost.\n", spuNum );
			//Don't print this final audit as we don't have all the data for it
			break;
		}
		WWSJOB_ASSERT( 1 + numDwords <= numDwordsToProcess );

		switch ( id )
		{
		case AuditId::kNopAudit:
			//Don't print this audit
			break;

		#if 0
		case AuditId::kEndOfAudits:
			//Don't process any more audits
			return;
			break;
		#endif
		
		default:
			func( this, spuNum, id, time, hwordIsValid, hData, numDwords, pWords, pUserData );
			break;
		}

		++pAudit;
		pAudit += numDwords;
		++auditIndex;
		numDwordsToProcess -= 1 + numDwords;

		if ( auditIndex >= maxAuditsToProcess )
		{
			//Don't process any more audits
			return;
		}
	}

	if ( pAudit == pAuditEnd  && ( enableHeaderPrinting == kPrintHeaders ) )
	{
		JobBasePrintf( "***NOTE: SPU%d Audit buffer filled.  Data may have been lost.\n", spuNum );
	}

	//Check we didn't go past the end of the buffer
	WWSJOB_ASSERT( pAudit <= pAuditEnd );
}

//--------------------------------------------------------------------------------------------------

void AuditManager::ProcessAuditBuffersForAllSpus( U32 bufferIndexNum, ProcessAuditFunction func, WwsJob_EnableHeaderPrinting enableHeaderPrinting, void* pUserData ) const
{
	WWSJOB_ASSERT( bufferIndexNum < m_numBuffers );

	if ( enableHeaderPrinting == kPrintHeaders )
		JobBasePrintf( "Processing audit buffer (0x%X)\n", (U32) m_pAuditHeader );

	for ( U32 spuNum = 0; spuNum < m_numSpusSharingBigBuffer; ++spuNum )
	{
		ProcessAuditBuffer( bufferIndexNum, spuNum, 0xFFFFFFFF, func, enableHeaderPrinting, pUserData );
	}
}

//--------------------------------------------------------------------------------------------------

void AuditManager::ImmediateModeDataU64( U64 data64, U32 spuNum, ProcessAuditFunction func, void* pUserData	)
{
	//This function takes in one U64 at a time and accumulates them.
	//Once it has enough for one audit, it processes it appropriately
	//Then resets its cache and starts again.

	WWSJOB_ASSERT( spuNum < kMaxSpus );

	WWSJOB_ASSERT( m_currentNumU64Elements[spuNum] < kMaxDwordsInSingleAudit );
	m_currentAuditDataCache[spuNum][m_currentNumU64Elements[spuNum]].m_u64 = data64;
	++m_currentNumU64Elements[spuNum];

	const Audit* pAudit	= &m_currentAuditDataCache[spuNum][0].m_audit;
	const U32 *pWords	= &m_currentAuditDataCache[spuNum][1].m_u32[0];

	U32 numDwords = pAudit->m_numDwords;
	bool hwordIsValid = true;
	if( numDwords == 7 )
	{
		numDwords = pAudit->m_data;
		hwordIsValid = false;
	}

	//Do we have enough elements to print the current audi yet?
	if ( m_currentNumU64Elements[spuNum] == (numDwords+1) )
	{
		U16 id		= pAudit->m_id;
		U16 hData	= pAudit->m_data;
		U32 time	= pAudit->m_time;

		switch ( id )
		{
		case AuditId::kNopAudit:
			//Don't print this audit
			break;

		#if 0
		case AuditId::kEndOfAudits:
			//Don't process any more audits
			return;
			break;
		#endif
		
		default:
			func( this, spuNum, id, time, hwordIsValid, hData, numDwords, pWords, pUserData );
			break;
		}

		//We've just printed the last audit, so clear the cache
		m_currentNumU64Elements[spuNum] = 0;
	}
}

//--------------------------------------------------------------------------------------------------

void AuditManager::ImmediateModeDataU32( U32 data, U32 spuNum, ProcessAuditFunction func, void* pUserData )
{
	//This function concatenates two consecutive U32s to make U64 and pass it on

	if ( m_cachedU32Exists[spuNum] )
	{
		U64 val64 = (((U64)m_cachedU32Value[spuNum]) << 32ULL) | ((U64)data);

		ImmediateModeDataU64( val64, spuNum, func, pUserData );

		m_cachedU32Exists[spuNum]	= false;
	}
	else
	{
		m_cachedU32Value[spuNum]	= data;
		m_cachedU32Exists[spuNum]	= true;
	}
}

//--------------------------------------------------------------------------------------------------

void AuditManager::WritePaSuiteFile( CellSpurs* pSpurs, const char* name )
{
	AuditFileHeader fileHeader;
	fileHeader.m_numSpus			= m_numSpusSharingBigBuffer;
	fileHeader.m_fileVersionNumber	= AuditFileHeader::kAuditFileVersionNumber;

	CellSpursInfo spursInfo;
	int ret = cellSpursGetInfo( pSpurs, &spursInfo );
	WWSJOB_ASSERT( CELL_OK == ret );
	WWSJOB_ASSERT( spursInfo.nSpus == (int)m_numSpusSharingBigBuffer );
	for ( U32 spuNum = 0; spuNum < 6; ++spuNum )
	{
		fileHeader.m_spuThreadIds[spuNum] = spursInfo.spuThreads[spuNum];
	}

	for ( U32 workloadNum = 0; workloadNum < AuditFileHeader::kMaxWorkloads; ++workloadNum )
	{
		char* nameBuffer = fileHeader.m_name[workloadNum];

		sprintf( nameBuffer, "List name %d", workloadNum );
	}

	U32 auditBufferSize = m_eachSpuAuditBufferSize * (m_numSpusSharingBigBuffer * m_numBuffers) + sizeof(AuditBufferHeader);

	FILE* fp = fopen( name, "wb" );
	WWSJOB_ASSERT( fp );
	ret = fwrite( &fileHeader, sizeof(fileHeader), 1, fp );
	WWSJOB_ASSERT( ret == 1 ); //ie. 1 fileHeader object has been written
	ret = fwrite( (void*)m_pAuditHeader, auditBufferSize, 1, fp );
	WWSJOB_ASSERT( ret == 1 ); //ie. 1 object of huge size has been written
    ret = fclose( fp );
	WWSJOB_ASSERT( ret == 0 );
}

//--------------------------------------------------------------------------------------------------
