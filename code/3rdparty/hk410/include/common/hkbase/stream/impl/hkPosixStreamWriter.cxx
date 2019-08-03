/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/stream/hkStreamReader.h>
#if defined(HK_PLATFORM_UNIX)
#	include <fcntl.h>
#	include <unistd.h>
#	include <sys/stat.h>
#	include <dirent.h>
#	define HK_OPEN(_fname) ::open(_fname, O_WRONLY | O_CREAT | O_TRUNC, 0666)
#	define HK_CLOSE(_handle) ::close(_handle)
#	define HK_WRITE(_handle, _buf, _nbytes) ::write( _handle, _buf, _nbytes )
#	define HK_SEEK(_handle, _offset, _whence) ::lseek(_handle, _offset, _whence)
#	define HK_FSYNC(_handle) ::fsync(_handle)
	// check that we can use these interchangeably
	HK_COMPILE_TIME_ASSERT(hkStreamReader::STREAM_SET==SEEK_SET
						&& hkStreamReader::STREAM_CUR==SEEK_CUR
						&& hkStreamReader::STREAM_END==SEEK_END );
#elif defined(HK_PLATFORM_PS2)
#	include <sifdev.h>
#	define HK_OPEN(_fname) sceOpen(_fname, SCE_WRONLY | SCE_CREAT | SCE_TRUNC)
#	define HK_CLOSE(_handle) sceClose(_handle)
#	define HK_WRITE(_handle, _buf, _nbytes) sceWrite( _handle, _buf, _nbytes )
#	define HK_SEEK(_handle, _offset, _whence) sceLseek(_handle, _offset, _whence)
#	define HK_FSYNC(_handle) // nothing
	HK_COMPILE_TIME_ASSERT(hkStreamReader::STREAM_SET==SCE_SEEK_SET
						&& hkStreamReader::STREAM_CUR==SCE_SEEK_CUR
						&& hkStreamReader::STREAM_END==SCE_SEEK_END );
#elif defined(HK_PLATFORM_PSP)
#	include <iofilemgr.h>
#	define HK_OPEN(_fname) sceIoOpen(_fname,  SCE_O_CREAT | SCE_O_WRONLY | SCE_O_TRUNC, 0)
#	define HK_CLOSE(_handle) sceIoClose(_handle)
#	define HK_WRITE(_handle, _buf, _nbytes) sceIoWrite( _handle, _buf, _nbytes )
#	define HK_SEEK(_handle, _offset, _whence) sceIoLseek(_handle, _offset, _whence)
#	define HK_FSYNC(_handle) // nothing
	HK_COMPILE_TIME_ASSERT(hkStreamReader::STREAM_SET==SCE_SEEK_SET
						&& hkStreamReader::STREAM_CUR==SCE_SEEK_CUR
						&& hkStreamReader::STREAM_END==SCE_SEEK_END );
#else
#	error unknown platform
#endif

class hkPosixStreamWriter : public hkStreamWriter
{
	public:

#if defined(HK_PLATFORM_UNIX)
		hkPosixStreamWriter(hkStreambufFactory::StdStream s)
			:	m_owned(false)
		{
			m_handle = (s == hkStreambufFactory::STDERR) ? 2 : 1;
		}
#endif

		hkPosixStreamWriter(const char* fname)
			:	m_owned(true)
		{
			m_handle = HK_OPEN(fname);
		}

		void close()
		{
			if(m_handle >= 0 && m_owned)
			{
				HK_CLOSE(m_handle);
				m_handle = -1;
			}
		}

		virtual ~hkPosixStreamWriter()
		{
			close();
		}

		virtual int write( const void* buf, int nbytes)
		{
			if( m_handle >= 0 )
			{
				int n = HK_WRITE( m_handle, buf, nbytes );
				if( n <= 0 )
				{
					close();
				}
                return n;
			}
			return 0;
		}

		virtual void flush()
		{
			if( m_handle >= 0 )
			{
				HK_FSYNC(m_handle);
			}
		}

		virtual hkBool isOk() const
		{
			return m_handle >= 0;
		}

		virtual hkResult seek( int offset, SeekWhence whence)
		{
			return HK_SEEK(m_handle, offset, whence) != -1
				? HK_SUCCESS
				: HK_FAILURE;
		}

		virtual int tell() const
		{
			return HK_SEEK(m_handle, 0, STREAM_CUR);
		}

	protected:

		int m_handle;
		hkBool m_owned;
};

#undef HK_OPEN
#undef HK_CLOSE
#undef HK_WRITE
#undef HK_SEEK
#undef HK_FSYNC

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
