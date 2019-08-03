/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_FILES_H
#define ICE_FILES_H


namespace Ice
{
	class File
	{
	private:
		
		char		m_filename[256];
		int         m_fd;
		bool        m_ok;
		
	public:
		
		File(const char *name);
		~File();

		bool IsOk() const { return m_ok; }
		
		U64 GetSize() const;
		void Read(U64 start, U64 size, void *data) const;
	};

	extern void SetCurrentWorkingFolder(const char *path);

	extern const char *GetCurrentWorkingFolder(); 

	extern void OutputExistingFiles(const char *path);

	/// get error description
	void GetErrorDescription ( int error, char * paErrorBuffer, int iBufferSize );
}


#endif
