//*******************************************************************************
//	
//	Archive.h.
//	
//******************************************************************************/

#ifndef ARCHIVE_H_
#define ARCHIVE_H_

#include "core/waddef.h"
#include "core/mem.h"

// Pre-declare the File class.
class	File;

//
//	Interface for Archive types.
//
class Archive
{
	public:
		//
		//	Open a file within this archive.
		//	  In:		'filename', the file to attempt to open from the archive.
		//	  In/Out:	'file', this must be a valid pointer. The object will be setup by the function.
		//	  RetVal:	'true' if the file was found and opened, 'false' otherwise.
		//
		virtual bool	OpenFile	( const char *filename, File *file, Mem::MEMORY_CHUNK chunk, uint32_t alignment )	const = 0;

		//
		//	Checks for the presence of a file in any archive.
		//	  In:		'filename', the file to seach for.
		//	  RetVal:	'true' if the file is found, false otherwise.
		//
		bool			FileExists	( const char *filename )	const;

		//
		//	Return the hash of this archive's filename.
		//
		uint32_t		GetNameHash	()							const	{ return m_ArchiveName.GetHash(); }

		//
		//	Query area-archive information.
		//
		bool			IsAreaArchive()							const	{ return m_IsAreaArchive; }
		int32_t			GetAreaNumber()							const	{ ntError_p( m_IsAreaArchive, ("You cannot get the area-number of a non-area archive.") ); return m_AreaNumber; }

	public:
		//
		//	Static creation of Archives from a file. Type of archive is
		//	automatically determined from the file.
		//
		static Archive *Create		( const char *filename, MediaTypes media_type );						// Create a non-area archive, e.g. from a directory wad.
		static Archive *Create		( const char *filename, int32_t area_number, MediaTypes media_type );	// Create an area archive, e.g. a wad created from an arm file.

	public:
		//
		//	Anyone can delete...
		//
		virtual ~Archive();

	protected:
		//
		//	Allow base classes to instantiate.
		//
		explicit Archive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, MediaTypes media_type );
		explicit Archive( const char *filename, int32_t archive_fd, const Wad::ArchiveHeader &archive_header, int32_t area_number, MediaTypes media_type );

	private:
		//
		//	Prevent default construction.
		//
		Archive() {}

	private:
		//
		//	Prevent copying and assignment.
		//
		Archive( const Archive & )				NOT_IMPLEMENTED;
		Archive &operator = ( const Archive & )	NOT_IMPLEMENTED;

	protected:
		//
		//	Helper functions.
		//
		static uint32_t	GetHash					( const char *filename );
		static void		ReadHeader				( int32_t archive_fd, Wad::ArchiveHeader &archive_header );

		void			Insert					( Wad::ArchiveFileData file_data );
		void			CreateArchiveFromFile	( int32_t archive_fd, const Wad::ArchiveHeader &header );
		const Wad::ArchiveFileData *GetFileData	( uint32_t hash )		const;

	protected:
		//
		//	Members visible to derived classes.
		//
		CKeyString		m_ArchiveName;					// The hashed filename of the archive itself.
		uint32_t		m_NumFiles;						// The number of files individually compressed in this archive.

		typedef ntstd::Vector< Wad::ArchiveFileData >	DataVector;
		typedef ntstd::Vector< DataVector >				HashTable;

		HashTable		m_HashTable;					// Hash-table for looking up offsets to file data in the compressed chunk.
														// The hash is created from the filename and used as an index (mod table size)
														// - this gives an DataVector, we then search the DataVector for this hash
														// and return the matching ArchiveFileData structure - which gives us, amongst
														// other things, the offset of the file from the start of the compressed data
														// chunk.

		uint32_t		m_DataOffset;					// Offset in the file of the compressed data.

		bool			m_IsAreaArchive;				// Are we an archive created from an ARM file?
		int32_t			m_AreaNumber;					// If we were created from an ARM file then which area-number does our data
														// correspond to.
		MediaTypes		m_MediaType;					// The media-device where this archive is located (and hence to be read from).

		static const uint32_t	TableSize = 1021;		// For good hashing, this should be prime and a reasonable size in comparison
														// to the number of files present in the archive.
};

//
//	Manager for Archive objects.
//
class ArchiveManager : public Singleton< ArchiveManager >
{
	public:
		//
		//	Open a file from a loaded archive.
		//	  In:		'filename', the file to attempt to open.
		//	  In/Out:	'file', must be a valid pointer. Will be returned ready for use.
		//	  RetVal:	'true' if the file was found and opened, 'false' otherwise.
		//
		bool	OpenFile		( const char *filename, File *file, Mem::MEMORY_CHUNK chunk, uint32_t alignment )	const;

		//
		//	Checks for the presence of a file in any archive.
		//	  In:		'filename', the file to seach for.
		//	  RetVal:	'true' if the file is found, false otherwise.
		//
		bool	FileExists		( const char *filename )				const;

		//
		//	Add an Archive object to the manager.
		//	  In:		'filename', the filename of the archive to add.
		//	  In:		'area_number', the number associated with this area.
		//
		void	AddArchive		( const char *filename, MediaTypes media_type );
		void	AddAreaArchive	( const char *filename, int32_t area_number, MediaTypes media_type );

		//
		//	Remove an Archive object from the manager.
		//	  In:		'filename', the filename of the archive to remove.
		//
		void	RemoveArchive	( const char *filename );

		//
		//	Check whether an archive has been loaded.
		//
		bool	ArchiveExists	( const char *filename )				const;
		bool	IsAreaWadLoaded	( int32_t area_number )					const;

	public:
		//
		//	Ctor, dtor.
		//
		ArchiveManager();
		~ArchiveManager();

	private:
		//
		//	Helper functions.
		//
		typedef ntstd::List< Archive * >	ArchiveContainer;
		ArchiveContainer::iterator ArchiveExists_Internal( const char *filename )	const;

	private:
		//
		//	Aggregated members.
		//
		mutable ArchiveContainer			m_Archives;
};

#endif // !ARCHIVE_H_








