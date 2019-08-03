//**************************************************************************************
//
//	ntBufferedStreamReader_ps3.h
//	
//	Same as the ntStreamReader class only buffers the entire file first and reads
//	from memory instead of disk.
//
//**************************************************************************************

#ifndef NTBUFFEREDSTREAMREADER_PS3_
#define NTBUFFEREDSTREAMREADER_PS3_

#ifndef PLATFORM_PS3
#	error This header file is a PS3 header!
#endif // !PLATFORM_PS3

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include <hkbase/hkBase.h>
#include <hkbase/stream/hkStreamReader.h>

// Include the NinjaTheory file-system.
#include "core/file.h"

#include "physics/ntStreamDebug_ps3.h"

//**************************************************************************************
//	NinjaTheory custom Havok stream reader class.
//**************************************************************************************
namespace Physics
{
	class ntBufferedStreamReader : public hkStreamReader
	{
		public:
			//
			//	Ctor, dtor.
			//
			ntBufferedStreamReader	( const char *nameIn )
			:	m_MemoryFile		( NULL )
			,	m_FileSize			( 0 )
			,	m_isOk				( false )
			,	m_CurrentPosition	( 0 )
			{
				File file( nameIn, File::FT_BINARY | File::FT_READ );
				m_isOk = file.IsValid();

				if ( !m_isOk )
				{
					return;
				}

				m_FileSize = file.GetFileSize();
				m_MemoryFile = NT_NEW char[ m_FileSize ];

				if ( m_MemoryFile == NULL )
				{
					m_FileSize = 0;
					m_isOk = false;
					return;
				}

				size_t read_length = file.Read( m_MemoryFile, m_FileSize );
				UNUSED( read_length );
				ntError_p( read_length == m_FileSize, ("Incorrect number of bytes read.") );

#				ifdef DEBUG_STREAM_OUTPUT
					ntPrintf( "[Havok]:\tOpened ntBufferedStreamReader for %s.\n", nameIn );
#				endif
			}

			virtual ~ntBufferedStreamReader()
			{
				NT_DELETE_ARRAY( m_MemoryFile );
				m_MemoryFile = NULL;

#				ifdef DEBUG_STREAM_OUTPUT
					ntPrintf( "[Havok]:\tClosed ntBufferedStreamReader.\n" );
#				endif
			}

		public:
			//
			//	File functions.
			//
			virtual int read( void *buf, int nbytes )
			{
#				ifdef DEBUG_STREAM_OUTPUT
					ntPrintf( "[Havok]:\tReading %i bytes, filepos = %i, filesize = %i.\n", nbytes, m_CurrentPosition, m_FileSize );
#				endif

				ntError( m_FileSize >= m_CurrentPosition );

				int32_t nread = nbytes <= int( m_FileSize ) - int( m_CurrentPosition ) ? nbytes : int( m_FileSize ) - int( m_CurrentPosition );

				NT_MEMCPY( buf, m_MemoryFile + m_CurrentPosition, nread );
				m_CurrentPosition += nread;

				if ( nread != nbytes )
				{
					m_isOk = false;
				}

				return nread;
			}

			virtual hkBool isOk() const
			{
				return m_isOk;
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
				switch ( whence )
				{
					case STREAM_SET:
						ntError_p( offset < int( m_FileSize ), ("Invalid seek offset.") );
						m_CurrentPosition = offset;
						break;

					case STREAM_END:
						ntError_p( offset < int( m_FileSize ), ("Invalid seek offset.") );
						m_CurrentPosition = m_FileSize - offset;
						break;

					case STREAM_CUR:
						ntError_p( offset < int( m_FileSize ) - int( m_CurrentPosition ), ("Invalid seek offset") );
						m_CurrentPosition += offset;
						break;

					default:
						ntError_p( false, ("Havok attempting to seek with unknown mode.") );
						break;

				}

				// Invalidate the file marker, if we have one.
				m_Marker.m_InUse = false;

				return HK_SUCCESS;
			}

			virtual int tell() const
			{
				return m_CurrentPosition;
			}

			virtual int skip( int nbytes )
			{
				if ( nbytes > int( m_FileSize ) - int( m_CurrentPosition ) )
				{
					nbytes = int( m_FileSize ) - int( m_CurrentPosition );
				}

				m_CurrentPosition += nbytes;

				return nbytes;
			}

		public:
			//
			//	Mark support.
			//
			virtual hkBool markSupported() const
			{
				return false;
			}

			virtual hkResult setMark( int markLimit )
			{
				m_Marker.m_OffsetFromStart = m_CurrentPosition;
				m_Marker.m_ValidPeriodLength = markLimit;
				m_Marker.m_InUse = true;

				return HK_SUCCESS;
			}

			virtual hkResult rewindToMark()
			{
				if ( !m_Marker.m_InUse )
				{
					return HK_FAILURE;
				}

				ntError_p( m_CurrentPosition >= m_Marker.m_OffsetFromStart, ("We must have used Seek to get here, but Seek should invalidate the marker - but the marker is valid.") );
				if ( m_CurrentPosition - m_Marker.m_OffsetFromStart > m_Marker.m_ValidPeriodLength )
				{
					// We've gone on too far for the marker to be valid.
					return HK_FAILURE;
				}

				m_CurrentPosition = m_Marker.m_OffsetFromStart;
				return HK_SUCCESS;
			}

		private:
			//
			//	Aggregated members.
			//
			char *	m_MemoryFile;
			size_t	m_FileSize;
			bool	m_isOk;
			size_t	m_CurrentPosition;

			struct FileMarker
			{
				size_t	m_OffsetFromStart;			// The position of the marker within the file.
				size_t	m_ValidPeriodLength;		// The marker is only valid for this number of bytes beyond its marked position.
				bool	m_InUse;

				FileMarker()
				:	m_OffsetFromStart	( 0 )
				,	m_ValidPeriodLength	( 0 )
				,	m_InUse				( false )
				{}
			};
			FileMarker	m_Marker;
	};
}

#endif // !NTBUFFEREDSTREAMREADER_PS3_
