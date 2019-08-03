//*******************************************************************************
//	
//	CompressedFile.h.
//	
//******************************************************************************/

#ifndef COMPRESSEDFILE_H_
#define COMPRESSEDFILE_H_

#include "core/file.h"
#include "core/archive.h"
#include "core/mem.h"

class CompressedFile
{
	public:
		//
		//	Accessors.
		//
		bool		IsValid		()	const	{ return m_Data != NULL; }
		size_t		Tell		()	const	{ return m_Offset; }
		size_t		GetFileSize	()	const	{ return m_Length; }

		size_t		Read		( void * restrict pOut, size_t sizeToRead );
		size_t		Seek		( int32_t offset, File::SeekMode mode );

		uint8_t *	GetData		()			{ return m_Data; }

		void		TakeOwnershipOfMemory()	{ ntError_p( m_OwnsMemory, ("Someone else has already claimed ownership of this object's memory!") ); m_OwnsMemory = false; }

	public:
		//
		//	Ctors, dtor.
		//
		CompressedFile()
		:	m_Data		( NULL )
		,	m_Length	( 0 )
		,	m_Offset	( 0 )
		,	m_Chunk		( Mem::MC_MISC )
		,	m_OwnsMemory( true )
		{}

		~CompressedFile();

	private:
		//
		//	CompressedIndex can create one of these, but no copying!
		//
		friend class CompressedArchive;
		CompressedFile( const void *compressed_data, uint32_t comp_length, uint32_t uncomp_length, Mem::MEMORY_CHUNK chunk, uint32_t alignment );

		CompressedFile( const CompressedFile & )				NOT_IMPLEMENTED;
		CompressedFile &operator = ( const CompressedFile & )	NOT_IMPLEMENTED;

	private:
		//
		//	Aggregated members.
		//
		uint8_t *			m_Data;			// The uncompressed data of the opened file.
		uint32_t			m_Length;		// The length of the data, in bytes.
		uint32_t			m_Offset;		// The offset for file operations, in bytes, from the start of the data.
		Mem::MEMORY_CHUNK	m_Chunk;		// The chunk we allocate our memory in.
		uint32_t			m_Alignment;	// The required alignment of the memory for the file contents.
		bool				m_OwnsMemory;	// true if we own our memory and should delete it, false if someone has called TakeOwnershipOfMemory.
};

class CompressedArchive : public Archive
{
	public:
		//
		//	Return data/length for the named file in this index.
		//
		bool		OpenFile	( const char *filename, File *file /*OUTPUT*/, Mem::MEMORY_CHUNK chunk, uint32_t alignment ) const;

	public:
		//
		//	Ctor, dtor.
		//	Create from a compressed file on disk - file will be loaded then
		//	decompressed in memory.
		//
		explicit CompressedArchive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, MediaTypes media_type );
		explicit CompressedArchive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, int32_t area_number, MediaTypes media_type );
		~CompressedArchive();

	private:
		//
		//	No copying these around.
		//
		CompressedArchive( const CompressedArchive & )				NOT_IMPLEMENTED;
		CompressedArchive &operator = ( const CompressedArchive & )	NOT_IMPLEMENTED;
};

#endif // !COMPRESSEDFILE_H_



