//*******************************************************************************
//																				*
//	Waddef.h.																	*
//																				*
//******************************************************************************/

#ifndef WADDEF_H_
#define WADDEF_H_

namespace Wad
{
	//
	//	Which WAD format is this?
	//
	enum Format
	{
		COMPRESSED_ARCHIVE		= (1<<0),		// 1 | Compressed archive - must also specify compression format.
		UNCOMPRESSED_ARCHIVE	= (1<<1),		// 2 | Uncompressed archive.

		// Compression format...
		ZLIB_COMPRESSED			= (1<<4),		// 16 | .

		MAX_ENUM_MEMBER
	};
	static_assert( MAX_ENUM_MEMBER < 255, Format_enum_must_fit_in_byte );

	//
	//	The header for an Archive.
	//
	struct ArchiveHeader
	{
		char		m_Tag[ 4 ];					// 	0		| "WAD", zero-terminated.
		uint8_t		m_Format;					// 	4		| One of the above - member of the Format enum.
		uint8_t		m_MajorVersion;				// 	5		| e.g. 001
		uint8_t		m_MinorVersion;				// 	6		| e.g. 020
		uint8_t		m_Pad;						//	7		|
		uint32_t	m_NumFiles;					//  8		| The number of files in this archive.
		uint32_t	m_CompressedDataOffset;		//	12		| Offset, in bytes, of the compressed file-data section of the archive.
		uint64_t	m_MD5Digest[ 2 ];			//	16		| MD5 digest of the entire WAD file. Used for checking for newer WAD versions when installing.
		uint32_t	m_WadFileLength;			//	32		| The length of this WAD file - used to check that it has been completely copied over correctly.
		uint8_t		m_PadEnd[ 12 ];				//	36		| Pad to 48 bytes - final size must be a multiple of 16.

		static const char *ClassTag() { return "WAD0"; }
	};
	static_assert( sizeof( ArchiveHeader ) == 48, Size_is_wrong );
	static_assert( ( sizeof( ArchiveHeader ) & 0xf ) == 0, header_must_be_a_multiple_of_16 );

	//
	//	File-data for each file in an archive WAD.
	//
	struct ArchiveFileData
	{
		uint32_t	m_CompressedDataOffset;
		uint32_t	m_CompressedDataLength;
		uint32_t	m_UncompressedDataLength;
		uint32_t	m_Hash;

		ArchiveFileData()
		{}

		ArchiveFileData(	uint32_t data_off, uint32_t comp_data_len,
							uint32_t uncomp_data_len, uint32_t hash )
		:	m_CompressedDataOffset		( data_off )
		,	m_CompressedDataLength		( comp_data_len )
		,	m_UncompressedDataLength	( uncomp_data_len )
		,	m_Hash						( hash )
		{}
	};
	static_assert( sizeof( ArchiveFileData ) == 16, Must_be_16_bytes_long );

	//
	//	Special numbers (e.g. bit-pattern for padding etc...)
	//
	static const uint8_t	PadByte		=	0xab;

	// Version info for the WAD files.
	namespace Version
	{
		static const uint32_t Major = 1;
		static const uint32_t Minor = 30;
	}
}

#endif // !WADDEF_H_




