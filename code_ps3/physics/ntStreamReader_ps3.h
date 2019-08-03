//**************************************************************************************
//
//	ntStreamReader_ps3.h
//	
//	PS3 replacement for hkStdioStreamReader.cxx - they use fopen etc... this version
//	uses our own filesystem classes/functions.
//
//**************************************************************************************

#ifndef NTSTREAMREADER_PS3_
#define NTSTREAMREADER_PS3_

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

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	NinjaTheory custom Havok stream reader class.
//**************************************************************************************
namespace Physics
{
	class ntStreamReader : public hkStreamReader
	{
		public:
			//
			//	Ctor, dtor.
			//
			ntStreamReader	( const char *nameIn )
			:	m_File		( nameIn, File::FT_BINARY | File::FT_READ )
			,	m_isOk		( m_File.IsValid() )
			{
			}

			virtual ~ntStreamReader()
			{
			}

		public:
			//
			//	File functions.
			//
			virtual int read( void* buf, int nbytes)
			{
				size_t nread( m_File.Read( buf, nbytes ) );
				if ( nread != size_t( nbytes ) )
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

				// Invalidate the file marker, if we have one.
				m_Marker.m_InUse = false;

				return HK_SUCCESS;
			}

			virtual int tell() const
			{
				return m_File.Tell();
			}

			virtual int skip( int nbytes )
			{
				m_File.Seek( nbytes, File::SEEK_FROM_CURRENT );
				return nbytes;
			}

		public:
			//
			//	Mark support.
			//
			virtual hkBool markSupported() const
			{
				return true;
			}

			virtual hkResult setMark( int markLimit )
			{
				m_Marker.m_OffsetFromStart = m_File.Tell();
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

				ntError_p( m_File.Tell() >= m_Marker.m_OffsetFromStart, ("We must have used Seek to get here, but Seek should invalidate the marker - but the marker is valid.") );
				if ( m_File.Tell() - m_Marker.m_OffsetFromStart > m_Marker.m_ValidPeriodLength )
				{
					// We've gone on too far for the marker to be valid.
					return HK_FAILURE;
				}

				m_File.Seek( m_Marker.m_OffsetFromStart, File::SEEK_FROM_START );
				return HK_SUCCESS;
			}

		private:
			//
			//	Aggregated members.
			//
			File 	m_File;
			bool	m_isOk;

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

#endif // !NTSTREAMREADER_PS3_
