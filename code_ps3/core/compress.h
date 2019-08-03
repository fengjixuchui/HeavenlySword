//*******************************************************************************
//	
//	Compress.h.
//	
//******************************************************************************/

#ifndef COMPRESS_H_
#define COMPRESS_H_

namespace Compress
{
	struct Block
	{
		void *		m_Mem;		// A block of memory.
		uint32_t	m_Length;	// The length of the above block, in bytes.
	};

	uint32_t	GetCompressBound( uint32_t uncompressed_length );
	void		Unpack			( Block data_in, Block &decompress_space );
	void		Pack			( Block data_in, Block &packed_data_space );
}

#endif // !COMPRESS_H_

