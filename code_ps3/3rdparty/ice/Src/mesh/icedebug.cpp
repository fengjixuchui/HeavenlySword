/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

// icedebug.cpp
#ifndef __SPU__

#include "icedebug.h"
#include "icemeshinternal.h"

using namespace Ice;
using namespace Ice::MeshProc;

static F32 F16toF32( F16 val )
{
	// Extract the sign
	int s = (val >> 15) & 0x00000001;
	// Extract the exponent
	int e = (val >> 10) & 0x0000001f;
	// Extract the mantissa
	int m =  val        & 0x000003ff;

	if (e == 0)
	{
		if (m == 0)
		{
			s <<= 31;
			return *(float*)&s;
		}
		else
		{
			// Denormalized number -- renormalize it
			while (!(m & 0x00000400))
			{
				m <<= 1;
				e -=  1;
			}

			e += 1;
			m &= ~0x00000400;
		}
	}
	else if (e == 31)
	{
		if (m == 0)
		{
			// Positive or negative infinity
			s = (s << 31) | 0x7f800000;
			return *(float*)&s;
		}
		else
		{
			// NAN -- preserve sign and mantissa bits
			s = (s << 31) | 0x7f800000 | (m << 13);
			return *(float*)&s;
		}
	}

	// Normalized number
	e = e + (127 - 15);
	m = m << 13;

	// Assemble s, e and m.
	I32 intval = (s << 31) | (e << 23) | m;
	return *(F32*)&intval;
}

struct DebugFormatWord
{
	U8	m_ID;
	U8	m_ComponentCount;
	U8	m_AttributeNum;
	U8	m_Type;
	U8	m_Offset;
};

void Ice::MeshProc::DebugDumpVertexStream( U8* pData, U32* pFormat )
{
    U32 attributeCount = *pFormat++;
	U32 bytesPerVert = *pFormat++;
	DebugFormatWord formatWords[16];

	for( U32 i = 0; i < attributeCount; ++i )
	{
		U8* pFormatWord = (U8*)pFormat;
        formatWords[i].m_ID					= pFormatWord[0];
		formatWords[i].m_ComponentCount		= (pFormatWord[1] >> 4) & 0xf;
		formatWords[i].m_AttributeNum		= (pFormatWord[1] >> 0) & 0xf;
		formatWords[i].m_Type				= pFormatWord[2];
		formatWords[i].m_Offset				= pFormatWord[3];
		++pFormat;
	}

	PRINTF("*NV Vertex Stream Dump*\n");
	PRINTF("*Format Info*\n");
	PRINTF("\tAttribute Count:\t%d\n", attributeCount);
	PRINTF("\tBytes Per Vert:\t%d\n", bytesPerVert);
	for( U32 i = 0; i < attributeCount; ++i )
	{
		PRINTF( "\tFormat Word: %d\n", i);
		PRINTF( "\t\tID:\t%d\n", formatWords[i].m_ID);
		PRINTF( "\t\tComponentCount:\t%d\n", formatWords[i].m_ComponentCount);
		PRINTF( "\t\tAttributeNum:\t%d\n", formatWords[i].m_AttributeNum);
		PRINTF( "\t\tType:\t%d\n", formatWords[i].m_Type);
		PRINTF( "\t\tOffset:\t%d\n", formatWords[i].m_Offset);
	}

	PRINTF("*Vertexes*\n");
	for( U32 i = 0; i < attributeCount; ++i )
	{
		PRINTF("\tAttribute: %d\n", i);
		switch(formatWords[i].m_Type)
		{
		case kF32:
			{
				PRINTF("\t\tF32: ");
				for( U32 ii = 0; ii < formatWords[i].m_ComponentCount; ++ii )
				{
					PRINTF("%3.3f ", *(F32*)(pData));
					pData += 4;
				}
				PRINTF("\n");
				break;
			}
		case kF16:
			{
				PRINTF("\t\tF16: ");
				for( U32 ii = 0; ii < formatWords[i].m_ComponentCount; ++ii )
				{
					F32 value = F16toF32( *(U16*)(pData) );
					PRINTF("%3.3f ", value);
					pData += 2;
				}
				PRINTF("\n");
				break;
			}
		case kI16n:
			{
				PRINTF("\t\tI16n: ");
				for( U32 ii = 0; ii < formatWords[i].m_ComponentCount; ++ii )
				{
					PRINTF("%x ", *(U16*)(pData));
					pData += 2;
				}
				PRINTF("\n");
				break;
			}
		case kI16:
			{
				PRINTF("\t\tI16: ");
				for( U32 ii = 0; ii < formatWords[i].m_ComponentCount; ++ii )
				{
					PRINTF("%x ", *(U16*)(pData));
					pData += 2;
				}
				PRINTF("\n");
				break;
			}
		case kX11Y11Z10n:
			{
				PRINTF("\t\tX11Y11Z10n: ");
				for( U32 ii = 0; ii < formatWords[i].m_ComponentCount; ++ii )
				{
					PRINTF("%x ", *(U32*)(pData));
					pData += 4;
				}
				PRINTF("\n");
				break;
			}
		case kU8n:
			{
				PRINTF("\t\tU8n: ");
				for( U32 ii = 0; ii < formatWords[i].m_ComponentCount; ++ii )
				{
					PRINTF("%x ", *(U8*)(pData));
					pData += 1;
				}
				PRINTF("\n");
				break;
			}
		case kU8:
			{
				PRINTF("\t\tU8: ");
				for( U32 ii = 0; ii < formatWords[i].m_ComponentCount; ++ii )
				{
					PRINTF("%x ", *(U8*)(pData));
					pData += 1;
				}
				PRINTF("\n");
				break;
			}
		default: break;
		}
	}
}

void Ice::MeshProc::DebugDumpLodInfo( U16* pData )
{
	PRINTF("*LOD Info Dump*\n");
	U32 lodsInTable = pData[3];
	lodsInTable = lodsInTable > 6 ? 6 : lodsInTable;
	for( U32 i = 0; i < lodsInTable; ++i )
	{
		PRINTF("\tLOD %d - First Vert:%d Vert Count:%d Halo Vert Count:%d LODs In Table:%d\n",
		    i, pData[0], pData[1], pData[2], pData[3]);
		pData+=4;
	}
}

void Ice::MeshProc::DebugDumpQWords( U32* pData, U32 numQWords )
{
	for( U32 i = 0; i < numQWords; ++i, pData += 4 )
	{
		PRINTF("%x: %x %x %x %x\n", (U32)pData, pData[0],  pData[1],  pData[2],  pData[3]);
	}
}

#endif//__SPU__
