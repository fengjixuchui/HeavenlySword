/***************************************************************************************************
* PS3 PS3 PS3 PS3 PS3 PS3 PS3
*    _____  ____    _     
*   |_   _|/ __ \  | |    
*     | | | |  | | | |__  
*     | | | |  | | | '_ \ 
*    _| |_| |__| |_| | | |
*   |_____|\____/(_)_| |_|
*                         
* ** SEE IO.CPP FOR EXAMPLE USAGE OF THESE CLASSES **
*                         
*    CHANGES
*
*    01.03.01    mikey    Created
*
***************************************************************************************************/


#ifndef CORE_IO_PS3_H
#define CORE_IO_PS3_H

#include "core/file.h"

// ---- File iterator states ----

#define TEXT_END		0	// End of File
#define TEXT_ERROR		1	// Error in text

#define TEXT_UNKNOWN	2	// Text type is unknown
#define TEXT_WHITE		3	// Text is whitespace
#define TEXT_COMMENT	4	// Text is a comment
#define TEXT_SKIP		5	// Allows the last " in a quote to be skipped on the next step
#define TEXT_EOL		6

#define TEXT_SOLID		16	// MARKER - ITEMS BELOW THIS POINT ARE NON TEXT
#define TEXT_TEXT		17	// Text is actually text
#define TEXT_SEPERATOR	18	// Text is a ({})
#define TEXT_QUOTE		19	// Text is enclosed within quotes


// ---- Forward references ----
class CFileIterator;
class CHashedString;
class FileBuffer;


/***************************************************************************************************
*	
*	CLASS			CSimpleStream
*
*	DESCRIPTION		a really simple stream to connect file stream writing and network stream writing
*
***************************************************************************************************/

class CSimpleStream
{
public:
	//Construct/Destruct
	CSimpleStream() {};
	virtual ~CSimpleStream() {};

	// Write the data out
	virtual CSimpleStream& operator<<(const char * pcVal) = 0;

	// Write a CHashedString out 
//	virtual CSimpleStream &operator <<(const CHashedString &data) = 0;

	// Write a float
	virtual CSimpleStream &operator << (float fValue) = 0;

	// Write an int
	virtual CSimpleStream &operator << (int iValue) = 0;

	// Indent the media by a given amount
	virtual void Indent() = 0;

	// Write an endline to the media
	virtual void Endline() = 0;

	// Is this a network stream ?
	virtual bool IsNetStream() { return false; };

	static void IncrementIndentLevel() {m_gIndentLevel++;}
	static void DecrementIndentLevel() {m_gIndentLevel--;}

protected:
	// Current indent level
	static int m_gIndentLevel;

};




/***************************************************************************************************
*	
*	CLASS			ofstream
*
*	DESCRIPTION		a really basic ofstream substitute.
*
*	USAGE			An example. Note the ntError checking has been deferred until end to avoid
*					clutter. Writes to bad streams are safely ignored.
*
*					{
*						ofstream obOut( "really_important_file" );
*						obOut << "Here is the answer:" << '\n';
*						obOut << 42 << '\n' << endl;
*						obOut << "So now you know.";
*
*						// flush buffers so we've written everything before
*						// checking status.
*						obOut.flush();
*
*						if( obOut.bad() )
*							BailOut("Brown trousers time");
*
*					}	// file closed upon destruction
*
*
***************************************************************************************************/

class	IOOfstream : public CSimpleStream
{
public:
	// open file on construction (will overwrite old file of same name)
	IOOfstream( const char* pcOutFilename );

	// stream is flushed and closed upon destruction.
	~IOOfstream();

	// Fns for writing
	virtual CSimpleStream& operator<<(const char * pcVal);
	IOOfstream& operator<<(char cVal);
	virtual CSimpleStream& operator<<(float fVal);
	virtual CSimpleStream& operator<<(int iVal);
	IOOfstream& operator<<(unsigned int uiVal);
	virtual CSimpleStream& operator <<(const CHashedString &data);

	// Fns to check state of stream. writes can still be performed on a bad stream
	// (they'll just be ignored), so you don't have to check for errors after _every_ write.
	inline bool good() const;
	inline bool bad() const;

	// write any pending data to file.
	IOOfstream& flush();

	// Indent the line within the given media by given amount
	virtual void Indent();

	// Terminate a line
	virtual void Endline();

private:

	static const u_int m_guiBufSize=1024;
	char		m_acBuf[ m_guiBufSize ];
	u_int 		m_uiCurrentPos;
	File		m_File;
};

inline bool IOOfstream::good() const
	{ return ( m_File.IsValid() ); }

inline bool IOOfstream::bad() const
	{ return !good(); }



/***************************************************************************************************
*     _____ ______ _ _      ______                                       _   
*    / ____|  ____(_) |    |  ____|                                     | |  
*   | |    | |__   _| | ___| |__   _ __  __ _  __ _ _ __ ___   ___ _ __ | |_ 
*   | |    |  __| | | |/ _ \  __| | '__|/ _` |/ _` | '_ ` _ \ / _ \ '_ \| __|
*   | |____| |    | | |  __/ |    | |  | (_| | (_| | | | | | |  __/ | | | |_ 
*    \_____|_|    |_|_|\___|_|    |_|   \__,_|\__, |_| |_| |_|\___|_| |_|\__|
*                                              __/ |                         
*                                             |___/                          
*                           
*	DESCRIPTION		Container for a token of text taken from a file iterator
*
***************************************************************************************************/

class CFileFragment
{
public:
	friend class CFileIterator;

	// Constructor
	CFileFragment(CFileIterator& obIterator)
		: m_obIterator(obIterator)
	{
		m_pcStart	= 0;
		m_pcEnd		= 0;
		m_pcTop		= 0;
		m_iState	= 0;
	}

	// Destructor
	~CFileFragment()
	{
	}

	// Is the current fragment an indent?
	inline bool IsIndent();

	// Is the current fragment an outdent
	inline bool IsOutdent();

	// Is the current fragment a seperator
	inline bool IsSeparator();

	// Is the fragment valid
	bool IsValid()
	{
		return m_iState > TEXT_ERROR;
	}

	// Get the size of the fragment in Bytes (not including the zero terminator)
	u_int GetSize() {return m_pcEnd-m_pcStart;};

	// Access the contents of the fragment as a string
	const char* operator*()
	{
		ntAssert(IsValid());
		return m_pcStart;
	}

	// Get the next token
	inline void Step();

	// Stream contents to string
	CFileFragment& operator >> (char* pcText)
	{
		ntAssert(IsValid());
		
		strcpy(pcText, m_pcStart);
		return *this;
	}

	// Stream contents to float
	CFileFragment& operator >> (float& fValue)
	{
		ntAssert(IsValid());
		fValue = ( float )atof(m_pcStart);
		return *this;
	}

	// Stream contents to int
	CFileFragment& operator >> (int& iValue)
	{
		ntAssert(IsValid());
		
		if (*m_pcStart == 'b')
		{
			// binary input
			iValue = 0;
			char* pcIn = m_pcStart+1;
			while((*pcIn == '0') || (*pcIn == '1'))
			{
				iValue <<= 1;
				if (*pcIn == '1')
				{
					iValue |= 0x01;
				}
				pcIn++;
			}
		}
		else
		{
			// default input
			iValue = atoi(m_pcStart);
		}
		return *this;
	}

protected:

	// Set the fragment to a given state
	void SetState(int iState)
	{
		m_iState = iState;
	}

	// Set the start of the fragment
	void SetStart(char* pcCurrent)
	{
		m_pcStart = pcCurrent;
	}

	// Set the top of the fragment's file
	void SetTop(char* pcFileEnd)
	{
		m_pcTop = pcFileEnd;
	}

	// Set the end of the fragment
	void SetEnd(char* pcCurrent)
	{
		// If this ntAssert fails it is usually because you have an empty fragment in your file e.g. ""
		// This will fail even if the empty fragment is within comments - GH
		ntAssert(pcCurrent> m_pcStart);

		m_pcEnd = pcCurrent;
		// Make sure that m_pcEnd is ALWAYS inside the file.... this is because of zero writes when parsing
		if (m_pcEnd >= m_pcTop)
		{
			m_pcEnd = m_pcTop-1;
		}
	}

	// ---- Members ----
	int m_iState;		// Current state
	char* m_pcStart;	// Pointer to start of string
	char* m_pcEnd;		// Pointer to end of string
	char* m_pcTop;		// Top of the file... m_pcEnd must never be equal or greater than this value
	CFileIterator&	m_obIterator; // The iterator it was constructed from
};


/***************************************************************************************************
*     _____ ______ _ _      _____ _                   _              
*    / ____|  ____(_) |    |_   _| |                 | |             
*   | |    | |__   _| | ___  | | | |_  ___ _ __  __ _| |_  ___  _ __ 
*   | |    |  __| | | |/ _ \ | | | __|/ _ \ '__|/ _` | __|/ _ \| '__|
*   | |____| |    | | |  __/_| |_| |_|  __/ |  | (_| | |_| (_) | |   
*    \_____|_|    |_|_|\___|_____|\__|\___|_|   \__,_|\__|\___/|_|   
*                                                                    
*                           
*	DESCRIPTION		Class to iterate through a file seeking fragments of text.
*
***************************************************************************************************/

class CFileIterator
{
public:
	// Friends
	friend class CFileFragment;

	// Constructor
	CFileIterator(const char * pcName, const char* pcIndentList, const char* pcOutdentList, const char* pcSeperatorList  );

	// Destructor
	~CFileIterator();

	// Stream the next token into a fragment
	bool operator >> (CFileFragment& obFrag);

	// Get the next item and check it is an indent seperator
	bool Indent();
	// Get the next item and check it is an outdent seperator
	bool Outdent();

	// Get the current indentation level
	int GetIndentLevel() const {return m_iIndentLevel;};

	// Report an ntError associated with the Iterator/File
	void Error(const char* pc);

	// Find the next item chunk
	bool NextChunk();

	// Return the next character without advancing the iterator
	char QueryNext()
	{
		// Check for end of file
		ntAssert(m_pcCurrent < m_pcEnd-1);
		return *(m_pcCurrent+1);
	}

	// Return the current line number
	int GetLineNumber ()
	{
		return m_iLineNumber;
	}

protected:

	// Process the current character in the file according to the iterator's current state.
	void Step();

	// Step until next state is found. False indicates end of file.
	bool NextState();

	// Find the next solid character in the file
	bool FindSolid();

	// Save the current end character of a token
	void Save()
	{
		ntAssert_p(m_pcCurrent < m_pcEnd, ( "Sorry, I need a whitespace at end of file" ) );
		m_cCopy = *m_pcCurrent;
		*m_pcCurrent = 0;
	}

	// Restore the current end character of a token
	void Restore()
	{
		if (m_cCopy && m_pcCurrent)
		{
			if (m_pcCurrent < m_pcEnd)
			{
				*m_pcCurrent = m_cCopy;
			}
		}
	}

	// ---- members ----
	FileBuffer*	m_pobFile;
	char*	m_pcCurrent;
	char*	m_pcEnd;
	int		m_iState;
	int		m_iIndentLevel;
	char	m_cCopy; 
	int		m_iLineNumber;

	const char* m_pcIndentList;
	const char* m_pcOutdentList;
	const char* m_pcSeperatorList;
};

/***************************************************************************************************
*     _____ ______ _ _      
*    / ____|  ____(_) |     
*   | |    | |__   _| | ___ 
*   | |    |  __| | | |/ _ \
*   | |____| |    | | |  __/
*    \_____|_|    |_|_|\___|
*                           
*	DESCRIPTION		Container for a file to be made memory resident
*
***************************************************************************************************/

class FileBuffer
{
public:
	// Friends
	friend class CFileIterator;

	// Constructor
	FileBuffer(const char * pcName, bool bBinary = false)
	{
		// open the file
		uint32_t flags = File::FT_READ;
		if (bBinary)
			flags |= File::FT_BINARY;

		File tmp( pcName, flags );
		user_error_p( tmp.IsValid(), ("Couldn't open file '%s'", pcName ) );

		if ( tmp.IsCompressedFile() )
		{
			// If this file is a compressed file then it's already been read
			// into memory and decompressed - we therefore don't need to read
			// it in (memcpy it) again, we can just take ownership of the
			// memory.
			m_pcBuffer = (char *)tmp.GetCompressedFileRawData();
			tmp.TakeOwnershipOfMemory();
		}
		else
		{
			// get the size and allocate enough storage
			size_t fileSize = tmp.GetFileSize();
			ntAssert( fileSize > 0 );
			m_pcBuffer = NT_NEW char[fileSize+1];	// Allows extra 0 for the get to append a null terminator

			// read the data
			size_t readSize = tmp.Read( m_pcBuffer, fileSize );
			ntAssert( readSize == fileSize );
			m_pcBuffer[readSize] = 0;
		}

		m_iSize = (int)tmp.GetFileSize();
	}

	const char * operator* () const {return m_pcBuffer;};
	const char * operator* () {return m_pcBuffer;};

	int	GetSize() {return m_iSize;};

	// Destructor
	~FileBuffer()
	{
		NT_DELETE_ARRAY( m_pcBuffer );
	}

protected:

	int m_iSize;
	char* m_pcBuffer;
};


// Get the next token
void CFileFragment::Step()
{
	m_obIterator >> *this;
}

// Is the current fragment an indent?
bool CFileFragment::IsIndent() 
{
	return !!strchr(m_obIterator.m_pcIndentList, *m_pcStart);
}

// Is the current fragment an outdent
bool CFileFragment::IsOutdent() 
{
	return !!strchr(m_obIterator.m_pcOutdentList, *m_pcStart);
}

bool CFileFragment::IsSeparator()
{
	return !!strchr(m_obIterator.m_pcSeperatorList, *m_pcStart);
}

#endif //_IO_H
