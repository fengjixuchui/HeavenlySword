//**************************************************************************************
//
//	ntConsoleWriter_ps3.h
//	
//	PS3 replacement for the Havok console stream writer.
//
//**************************************************************************************

#ifndef NTCONSOLEWRITER_PS3_
#define NTCONSOLEWRITER_PS3_

#ifndef PLATFORM_PS3
#	error This header file is a PS3 header!
#endif // !PLATFORM_PS3

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include <hkbase/hkBase.h>
#include <hkbase/stream/hkStreamWriter.h>

// Include the NinjaTheory file-system.
#include "core/file.h"

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	NinjaTheory custom Havok stream reader class.
//**************************************************************************************
namespace Physics
{
	class ntConsoleWriter : public hkStreamWriter
	{
		public:
			//
			//	Ctor, dtor.
			//
			ntConsoleWriter	( hkStreambufFactory::StdStream s )
			{
				static const char *StreamName[] =
				{
					"HavokStdOut.txt",
					"HavokStdErr.txt"
				};

				ntError( int32_t( s ) < 2 );
				m_File.Open( StreamName[ s > 1 ? 1 : s ], File::FT_TEXT | File::FT_WRITE );
				m_isOk = m_File.IsValid();
			}

			virtual ~ntConsoleWriter()
			{}

		public:
			//
			//	File functions.
			//
			virtual int write( const void *buf, int nbytes )
			{
				m_File.Write( buf, nbytes );
				static const int32_t TextBufferLength = 4096;
				static char sTextBuffer[ TextBufferLength ];

				int text_len = nbytes > TextBufferLength-1 ? TextBufferLength-1 : nbytes;
				NT_MEMCPY( sTextBuffer, buf, text_len );
				sTextBuffer[ text_len ] = '\0';

				ntPrintf( "[Havok]:\t%s\n", (const char *)sTextBuffer );

				return nbytes;
			}

			virtual hkBool isOk() const
			{
				return m_isOk;
			}

			virtual void flush()
			{
				m_File.Flush();
			}

		public:
			//
			//	Seek/Tell functionality.
			//
			virtual hkBool seekTellSupported() const
			{
				return true;
			}		

			virtual hkResult seek( int offset, SeekWhence whence )
			{
				File::SeekMode mode;
				switch ( whence )
				{
					case STREAM_SET:
						mode = File::SEEK_FROM_START;
						break;

					case STREAM_END:
						mode = File::SEEK_FROM_END;
						break;

					case STREAM_CUR:
						mode = File::SEEK_FROM_CURRENT;
						break;

					default:
						ntError_p( false, ("Havok attempting to seek with unknown mode.") );
						mode = File::SEEK_FROM_CURRENT;
						break;

				}

				m_File.Seek( offset, mode );

				return HK_SUCCESS;
			}

			virtual int tell() const
			{
				return m_File.Tell();
			}

		public:
			//
			//	Aggregated members - havok has these as public. Bugger.
			//
			File 	m_File;
			bool	m_isOk;
	};
}

#endif // !NTCONSOLEWRITER_PS3_
