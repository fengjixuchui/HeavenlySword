//*******************************************************************************
//	
//	StandardArchive.h.
//	
//******************************************************************************/

#ifndef STANDARDARCHIVE_H_
#define STANDARDARCHIVE_H_

#include "core/archive.h"
#include "core/waddef.h"

class File;

class StandardArchive : public Archive
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
		explicit StandardArchive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, MediaTypes media_type );
		explicit StandardArchive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, int32_t area_number, MediaTypes media_type );
		~StandardArchive();

	private:
		//
		//	No copying these around.
		//
		StandardArchive( const StandardArchive & )				NOT_IMPLEMENTED;
		StandardArchive &operator = ( const StandardArchive & )	NOT_IMPLEMENTED;
};

#endif // !STANDARDARCHIVE_H_

