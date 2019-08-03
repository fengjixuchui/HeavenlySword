//-------------------------------------------------------
//!
//!	\file core\blurayfile_ps3.h
//!	
//!	File class to transparently access blu-ray and if
//! not present to search the default media object.
//!
//-------------------------------------------------------

#ifndef BLURAYFILE_H_
#define BLURAYFILE_H_

class BluRayFile
{
	public:
		//! default do nothing ctor 
		BluRayFile();

		//------------------------------------------------
		//! ctor for opening read only the specified file
		//! \param pFileName filename to open/create
		//! Files are always opened as binary on PS3 and
		//! since this is opening a file on the blu-ray
		//! drive we obviously can't write to it...
		//------------------------------------------------
		BluRayFile( const char *pFileName );

		//! dtor
		~BluRayFile();

		//! opens the file of type
		void Open( const char *pFileName );

		//! closes the file.
		void Close();

		//! is this file valid (open correctly etc.)
		bool IsValid() const;

		//! does this file exist at all?
		static bool Exists( const char* pFileName );

		//! Read from disk.
		size_t Read( void* restrict pOut, size_t sizeToRead );

		//! Returns our current offset from the start of the file.
		size_t Tell() const;

		//! Sets our current offset from the start of the file for all subsequent operations.
		enum SeekMode
		{
			SEEK_FROM_START = 0,
			SEEK_FROM_END,
			SEEK_FROM_CURRENT,

			NUM_SEEK_MODES
		};
		void Seek( int32_t offset, SeekMode mode );

		//! Gets the size of a file
		size_t GetFileSize();

	private:
		//!	Prevent copying and assignment.
		BluRayFile( const BluRayFile & )				/*NOT_IMPLEMENTED*/;
		BluRayFile &operator = ( const BluRayFile & )	/*NOT_IMPLEMENTED*/;

	private:
		int	m_iFileHandle;
};

#endif // !BLURAYFILE_H_



