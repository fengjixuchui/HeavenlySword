//**************************************************************************************
//
//	ntStreamWriter_ps3.h
//	
//	PS3 replacement for hkStdioStreamWriter.cxx - they use fopen etc... this version
//	uses our own filesystem classes/functions.
//
//**************************************************************************************

#ifndef NTSTREAMWRITER_PS3_
#define NTSTREAMWRITER_PS3_

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
	class ntStreamWriter : public hkStreamWriter
	{
		public:
			//
			//	Ctor, dtor.
			//
			ntStreamWriter	( const char *nameIn )
			:	m_File		( nameIn, File::FT_BINARY | File::FT_WRITE )
			,	m_isOk		( m_File.IsValid() )
			{}

			virtual ~ntStreamWriter()
			{}

		public:
			//
			//	File functions.
			//
			virtual int write( const void *buf, int nbytes )
			{
				m_File.Write( buf, nbytes );
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

#endif // !NTSTREAMWRITER_PS3_
