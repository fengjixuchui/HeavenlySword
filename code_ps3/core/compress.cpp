//*******************************************************************************
//	
//	Compress.cpp.
//	
//******************************************************************************/

#include "core/compress.h"

#include "3rdparty/zlib/zlib.h"

void Compress::Unpack( Block data_in, Block &decompress_space )
{
	ntError( data_in.m_Mem != NULL );
	ntError( data_in.m_Length > 0 );
	ntError( decompress_space.m_Length > 0 );
	ntError( decompress_space.m_Mem != NULL );

	uLongf decompressed_length = (uLongf)decompress_space.m_Length;
	int32_t ret_val = uncompress( (Bytef *)decompress_space.m_Mem, &decompressed_length, (Bytef *)data_in.m_Mem, data_in.m_Length );
	if ( ret_val != Z_OK )
	{
		switch ( ret_val )
		{
			case Z_MEM_ERROR:
				ntPrintf( "\tFATAL ERROR: Zlib: Not enough memory (Unpack).\n" );
				break;

			case Z_BUF_ERROR:
				ntPrintf( "\tFATAL ERROR: Zlib: Not enough room in decompression buffer.\n" );
				break;

			case Z_DATA_ERROR:
				ntPrintf( "\tFATAL ERROR: Zlib: Input data corrupt.\n" );
				break;

			default:
				ntPrintf( "\tFATAL ERROR: Zlib: Unknown error - PackInPlace.\n" );
				break;
		};

		ntError( false );
		return;
	}

	ntError( decompress_space.m_Length == (uint32_t)decompressed_length );
	decompress_space.m_Length = (uint32_t)decompressed_length;
}

uint32_t Compress::GetCompressBound( uint32_t uncompressed_length )
{
	return compressBound( uncompressed_length );
}

void Compress::Pack( Block data_in, Block &packed_data_space )
{
	ntError( data_in.m_Mem != NULL );
	ntError( data_in.m_Length > 0 );
	ntError( packed_data_space.m_Mem != NULL );
	ntError( packed_data_space.m_Length >= GetCompressBound( data_in.m_Length ) );

	uLongf packed_length = packed_data_space.m_Length;
	packed_data_space.m_Length = 0;
	int32_t ret_val = compress( (Bytef *)packed_data_space.m_Mem, &packed_length, (const Bytef *)data_in.m_Mem, data_in.m_Length );
	if ( ret_val != Z_OK )
	{
		switch ( ret_val )
		{
			case Z_MEM_ERROR:
				ntPrintf( "\tFATAL ERROR: Zlib: Not enough memory (Pack).\n" );
				break;

			case Z_BUF_ERROR:
				ntPrintf( "\tFATAL ERROR: Zlib: Not enough room in compression buffer.\n" );
				break;

			default:
				ntPrintf( "\tFATAL ERROR: Zlib: Unknown error - PackInPlace.\n" );
				break;
		};

		ntError( false );
		return;
	}

	packed_data_space.m_Length = packed_length;
}




