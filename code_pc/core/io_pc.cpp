/***************************************************************************************************
*
*    _____  ____                     
*   |_   _|/ __ \                    
*     | | | |  | |   ___ _ __  _ __  
*     | | | |  | |  / __| '_ \| '_ \ 
*    _| |_| |__| |_| (__| |_) | |_) |
*   |_____|\____/(_)\___| .__/| .__/ 
*                       | |   | |    
*                       |_|   |_|    
*
*    CHANGES
*
*    01.03.01    mikey    Created
*
***************************************************************************************************/

/*
CFileIterator is used to stream tokens of text from a memory based file into CFileFragments. 
The contents of the file fragment can be querried of EOF or streamed into floats ints and strings

* A token is a set of characters divided by whitespace, brackets or braces.
* A token may contain whitespace etc if enclosed in quotes. 
* C++ style comments are ignored. 
* brackets or braces will increase or decrease indent level. Whole indent levels can therefore be ignored

Example text:-

	"The Mystery Machine" 3.0
	{
		Shaggy 
		"Scooby Doo"
		Freddy Daphne
		Velma // Busy finding her glasses
		"The monster, who is really the friendly old guy from the cafe wearing a mask"
	}		

Example code:-

  	To correctly read the names of the mystery gang into an appropriate container....

	CFileIterator obFileIt(
  	CFileFragment obFrag;
	
	// Read in the name
	if (obFileIt >> obFrag)
	{
		// Set the name of the container from the text fragment (The Mystery Machine)
		m_obContainer.SetName(*obFrag);	

		if (obFileIt >> obFrag)
		{
			// Set the size of the engine (3.0)
			obFrag >> m_obContainer.m_fEngineSize;

			// Read in the opening brace
			if (obFileIt.Indent())
			{
				while (obFileIt >> obFrag)
				{
					// Check for closing brace
					if (obFrag.IsOutdent)
					{
						// leave the function successfully
						return;
					}

					// Copy the name of the passenger from the fragment (Shaggy, Scooby Doo, Freddy, Daphne, Velma, The monster...)
					m_obContainer.AddPassenger(*obFrag);
				}
			}
		}
	}
	obFileIt.Error("Parse ntError");
*/

#include "core/io.h"

// The current indent level for writing
int CSimpleStream::m_gIndentLevel = 0;

/***************************************************************************************************
*	
*	FUNCTION		CFileIterator::CFileIterator
*
*	DESCRIPTION		Constructor
*
*	INPUTS			pcName	-	Name of file to iterate. If it starts with a drive letter, then it
*								is the name of a file local to the Xbox hard drive
*
***************************************************************************************************/

#if 1
CFileIterator::CFileIterator(const char * pcName, const char* pcIndentList, const char* pcOutdentList, const char* pcSeperatorList )
{
	m_pobFile = NT_NEW FileBuffer( pcName );
	m_pcCurrent = const_cast<char*>(**m_pobFile);
	ntAssert(m_pcCurrent);
	m_pcEnd = m_pcCurrent + m_pobFile->GetSize();
	m_iState = TEXT_UNKNOWN;
	m_cCopy = 0;
	m_iIndentLevel = 0;
	m_iLineNumber = 1;
	m_pcIndentList = pcIndentList;
	m_pcOutdentList = pcOutdentList;
	m_pcSeperatorList=pcSeperatorList;
}


/***************************************************************************************************
*	
*	FUNCTION		CFileIterator::~CFileIterator
*
*	DESCRIPTION		Destructor
*
***************************************************************************************************/

CFileIterator::~CFileIterator()
{
	Restore();
	NT_DELETE( m_pobFile );
}


/***************************************************************************************************
*	
*	FUNCTION		CFileIterator::operator >>
*
*	DESCRIPTION		Streams a token of text to a file fragment
*
*	INPUTS			obFrag	-	File fragment to reference next token
*
***************************************************************************************************/

bool CFileIterator::operator >> (CFileFragment& obFrag)
{
	Restore();
	if (FindSolid())
	{
		obFrag.SetState(m_iState);
		obFrag.SetStart(m_pcCurrent);
		obFrag.SetTop(m_pcEnd);
		
		// Other text
		if (NextState())
		{
			obFrag.SetEnd(m_pcCurrent);
			Save();
			return true;
		}
	}
	obFrag.SetState(TEXT_END);
	return false;
}


/***************************************************************************************************
*	
*	FUNCTION		CFileIterator::Step
*
*	DESCRIPTION		process the current character in the file according to the iterator's current
*					state.
*
***************************************************************************************************/

void CFileIterator::Step()
{
	if (m_pcCurrent >= m_pcEnd)
	{
		m_iState = TEXT_END;
		return;
	}
	else
	{
		switch (m_iState)
		{
		// ------------------------------------------------------------------------------
		case TEXT_UNKNOWN:

			if (strchr(m_pcSeperatorList, *m_pcCurrent))
			{
				m_iState = TEXT_SEPERATOR;
			}
			else
			{
				switch(*m_pcCurrent)
				{
				case '/':
					if (QueryNext() == '/')
					{
						m_iState = TEXT_COMMENT;
						break;
					}
					m_iState = TEXT_TEXT;
					break;

				case ' ':
				case '\t':
				case '\r':
					m_iState = TEXT_WHITE;
					break;

				case '\"':
					m_iState = TEXT_QUOTE;
					m_pcCurrent++;
					break;

				case '\n':
					m_iLineNumber++;
					m_pcCurrent++;
					break;

				default:
					m_iState = TEXT_TEXT;
					break;

				};
				break;
			}
			break;

		// ------------------------------------------------------------------------------
		case TEXT_WHITE:
			switch(*m_pcCurrent)
			{
				case '\n':
					m_iLineNumber++;
				case ' ':
				case '\t':
				case '\r':
					m_pcCurrent++;
					break;
				default:
					m_iState = TEXT_UNKNOWN;
					break;
			};				

			break;

		// ------------------------------------------------------------------------------
		case TEXT_COMMENT:
			if (*m_pcCurrent == '\n')
			{
				m_iState = TEXT_UNKNOWN;
			}
			m_pcCurrent++;
			break;

		// ------------------------------------------------------------------------------
		case TEXT_SEPERATOR:

			if (strchr(m_pcIndentList, *m_pcCurrent))
					m_iIndentLevel++;

			if (strchr(m_pcOutdentList, *m_pcCurrent))
					m_iIndentLevel--;

			m_iState = TEXT_UNKNOWN;
			m_pcCurrent++;
			break;

		// ------------------------------------------------------------------------------
		case TEXT_QUOTE:
			if (*m_pcCurrent == '\"')
			{
				m_iState = TEXT_SKIP;
				break;
			}
			if (*m_pcCurrent == '\n')
			{
				m_iState = TEXT_ERROR;
				Error("Unexpected newline in quoted text");
				break;
			}
			m_pcCurrent++;
			break;
		
		// ------------------------------------------------------------------------------
		case TEXT_TEXT:

			if (strchr(m_pcSeperatorList, *m_pcCurrent))
			{
				m_iState = TEXT_UNKNOWN;
			}
			else
			{
				switch (*m_pcCurrent)
				{
					case ' ':
					case '\t':
					case '\n':
					case '\r':
					case '\"':
						m_iState = TEXT_UNKNOWN;
						break;
					default:
						m_pcCurrent++;
						break;
				}
			}
			break;

		// ------------------------------------------------------------------------------

		case TEXT_ERROR:	
		case TEXT_END:
			break;

		// ------------------------------------------------------------------------------
		case TEXT_SKIP:
			m_iState = TEXT_UNKNOWN;
			m_pcCurrent++;		// Skip this character
			break;
		// ------------------------------------------------------------------------------
		default:
			break;
		};
	}
}


/***************************************************************************************************
*	
*	FUNCTION		CFileIterator::NextState
*
*	DESCRIPTION		Step the file until the next state occurs
*
*	RESULT			true	=	Next state found
*					false	=	Next state not found (End of File)
*
***************************************************************************************************/

// Step until next state is found. False indicates end of file.
bool CFileIterator::NextState()
{
	int iCurrent = m_iState;
	while (m_iState == iCurrent)
	{
		Step();

		if (m_iState <= TEXT_ERROR)
		{
			return false;
		}
	}
	return true;
}

/***************************************************************************************************
*	
*	FUNCTION		CFileIterator::NextState
*
*	DESCRIPTION		Step until next solid character is found
*
*	RESULT			true	=	Next solid found
*					false	=	Next solid not found (End of File)
*
***************************************************************************************************/

bool CFileIterator::FindSolid()
{
	while (m_iState > TEXT_ERROR)
	{
		if (m_iState < TEXT_SOLID)
		{
			Step();
		}
		else
		{
			return true;
		}
	}
	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CFileIterator::Error
*
*	DESCRIPTION		Generate an ntError message
*
*	INPUTS			pcError	-	Pointer to text to be displayed for ntError
*
***************************************************************************************************/

void CFileIterator::Error(const char * pcError)
{
	ntPrintf("*** CFileIterator::Error ***\n");
	ntPrintf("Line number %d\n", m_iLineNumber);
	ntPrintf("Error - \"%s\"\n", pcError);
	ntAssert(0);
}

/***************************************************************************************************
*	
*	FUNCTION		CFileIterator::Indent
*
*	DESCRIPTION		Get next token and check it is an indent
*
*	RESULT			true	=	Indent found
*					false	=	Indent not found (End of File)
*
***************************************************************************************************/

bool CFileIterator::Indent()
{
	CFileFragment obFrag(*this);
	if (*this >> obFrag)
	{
		if (obFrag.IsIndent())
		{
			return true;
		}
	}
	return false;
}

/***************************************************************************************************
*	
*	FUNCTION		CFileIterator::Outdent
*
*	DESCRIPTION		Get next token and check it is an Outdent
*
*	RESULT			true	=	Outdent found
*					false	=	Outdent not found (End of File)
*
***************************************************************************************************/

bool CFileIterator::Outdent()
{
	CFileFragment obFrag(*this);
	if (*this >> obFrag)
	{
		if (obFrag.IsOutdent())
		{
			return true;
		}
	}
	return false;
}




/***************************************************************************************************
*	
*	FUNCTION		CFileIterator::NextChunk
*
*	DESCRIPTION		Get next token and check it is an Outdent
*
*	RESULT			true	=	Outdent found
*					false	=	Outdent not found (End of File)
*
***************************************************************************************************/

// Find the next item chunk
bool CFileIterator::NextChunk()
{
	CFileFragment obFrag(*this);

	int iCurrentLevel = GetIndentLevel();

	*this >> obFrag;

	if (obFrag.IsIndent())
	{
		while (*this >> obFrag)
		{
			if (GetIndentLevel() <= iCurrentLevel)
			{
				return true;
			}
		}
	}
	return false;
}

#if 1

/***************************************************************************************************
*	
*	CLASS			ofstream constructor & operators
*
*	DESCRIPTION		a really basic ofstream substitute.
*
***************************************************************************************************/

IOOfstream::IOOfstream( const char* pcOutFilename ) :
	m_uiCurrentPos(0)
{
	m_hFile = CreateFile( pcOutFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL );

	if ( m_hFile == INVALID_HANDLE_VALUE )
	{
		LPVOID lpMsgBuf = 0;

		if (!FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL ))
		{
		}
		else if (lpMsgBuf)
		{
			ntPrintf("Failed to create ofstream() output file : Result code %d = %s\n" , GetLastError(), (LPCTSTR)lpMsgBuf );
		}
	}
}


IOOfstream::~IOOfstream()
{
	if( good() )
	{
		flush();

		if ( m_hFile != INVALID_HANDLE_VALUE )
			CloseHandle( m_hFile );
	}
}


IOOfstream& IOOfstream::flush()
{
	DWORD	dwBytesWritten;
	
	if( !m_uiCurrentPos || bad() )
		return *this;

	if ( WriteFile( m_hFile, m_acBuf, m_uiCurrentPos, &dwBytesWritten, NULL ) )
	{
		if ( m_uiCurrentPos == dwBytesWritten )
		{
			m_uiCurrentPos = 0;
			return *this;
		}
		else
		{
			ntPrintf( "ERROR: WriteFile() didn't write all requested Bytes\n" );
		}
	}
	else
	{
		ntPrintf( "ERROR: WriteFile() failed\n" );
	}

	// Something went wrong - either WriteFile() failed, or it didn't write everything - so close the
	// file and invalidate the handle

	CloseHandle( m_hFile );
	m_hFile = INVALID_HANDLE_VALUE;

	return *this;
}


CSimpleStream& IOOfstream::operator<<(const char * pcVal)
{
	if( bad() )
		return *this;

	uint32_t uiRemaining = strlen( pcVal );
	const char* pSrc=pcVal;

	while( uiRemaining )
	{
		uint32_t uiRun = uiRemaining;
		if( uiRun > m_guiBufSize - m_uiCurrentPos )
		{
			uiRun = m_guiBufSize - m_uiCurrentPos;
		}

		NT_MEMCPY( m_acBuf + m_uiCurrentPos, pSrc, uiRun );
		pSrc+=uiRun;
		m_uiCurrentPos += uiRun;
		uiRemaining -= uiRun;

		if( m_uiCurrentPos == m_guiBufSize )
			flush();
	}

	return *this;
}

IOOfstream& IOOfstream::operator<<(char cVal)
{
	if( bad() )
		return *this;

	if( m_uiCurrentPos+1 > m_guiBufSize )
		flush();

	m_acBuf[ m_uiCurrentPos++ ] = cVal;

	return *this;
}

CSimpleStream& IOOfstream::operator<<(float fVal)
{
	const uint32_t uiMaxSize = 32;

	if( bad() )
		return *this;

	if( m_uiCurrentPos+uiMaxSize > m_guiBufSize )
		flush();

	uint32_t uiCount = sprintf( m_acBuf+m_uiCurrentPos, "%g", fVal );
	ntAssert_p( uiCount < uiMaxSize, ( "Evil buffer overflow! - increase uiMaxSize" ) );
	m_uiCurrentPos += uiCount;

	return *this;
}

CSimpleStream& IOOfstream::operator<<(int iVal)
{
	const uint32_t uiMaxSize = 32;

	if( bad() )
		return *this;

	if( m_uiCurrentPos+uiMaxSize > m_guiBufSize )
		flush();

	uint32_t uiCount = sprintf( m_acBuf+m_uiCurrentPos, "%d", iVal );
	ntAssert_p( uiCount < uiMaxSize, ( "Evil buffer overflow! - increase uiMaxSize" ) );
	m_uiCurrentPos += uiCount;

	return *this;
}

IOOfstream& IOOfstream::operator<<(unsigned int uiVal)
{
	const uint32_t uiMaxSize = 32;

	if( bad() )
		return *this;

	if( m_uiCurrentPos+uiMaxSize > m_guiBufSize )
		flush();

	uint32_t uiCount = sprintf( m_acBuf+m_uiCurrentPos, "%d", uiVal );
	ntAssert_p( uiCount < uiMaxSize, ( "Evil buffer overflow! - increase uiMaxSize" ) );
	m_uiCurrentPos += uiCount;

	return *this;
}

CSimpleStream& IOOfstream::operator <<(const CKeyString &data)
{
	return *this << ntStr::GetString(data);
}


void IOOfstream::Indent()
{
	char cBuffer[64];
	ntAssert(m_gIndentLevel < 64 -1);

	int iLevel = m_gIndentLevel;

	char* pobInd = cBuffer;
	while (iLevel > 0)
	{
		*pobInd++ = '\t';
		iLevel--;
	}
	
	*pobInd = '\0';
	*this << cBuffer;
}

void IOOfstream::Endline()
{
	*this << "\r\n";
}


#endif
#endif