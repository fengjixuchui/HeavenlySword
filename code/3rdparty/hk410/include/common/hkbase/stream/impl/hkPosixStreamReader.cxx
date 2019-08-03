/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#if defined(HK_PLATFORM_UNIX)
#	include <fcntl.h>
#	include <unistd.h>
#	include <sys/stat.h>
#	include <dirent.h>
#	define HK_OPEN(_fname) ::open(_fname, O_RDONLY, 0666)
#	define HK_CLOSE(_handle) ::close(_handle)
#	define HK_READ(_handle, _buf, _nbytes) ::read( _handle, _buf, _nbytes )
#	define HK_SEEK(_handle, _offset, _whence) ::lseek(_handle, _offset, _whence)
	// check that we can use these interchangeably
	HK_COMPILE_TIME_ASSERT(hkStreamReader::STREAM_SET==SEEK_SET
						&& hkStreamReader::STREAM_CUR==SEEK_CUR
						&& hkStreamReader::STREAM_END==SEEK_END );
#elif defined(HK_PLATFORM_PS2)
#	include <sifdev.h>
#	define HK_OPEN(_fname) sceOpen(_fname, SCE_RDONLY)
#	define HK_CLOSE(_handle) sceClose(_handle)
#	define HK_READ(_handle, _buf, _nbytes) sceRead( _handle, _buf, _nbytes )
#	define HK_SEEK(_handle, _offset, _whence) sceLseek(_handle, _offset, _whence)
	HK_COMPILE_TIME_ASSERT(hkStreamReader::STREAM_SET==SCE_SEEK_SET
						&& hkStreamReader::STREAM_CUR==SCE_SEEK_CUR
						&& hkStreamReader::STREAM_END==SCE_SEEK_END );
#elif defined(HK_PLATFORM_PSP)
#	include <iofilemgr.h>
#	define HK_OPEN(_fname) sceIoOpen(_fname, SCE_O_RDONLY, 0)
#	define HK_CLOSE(_handle) sceIoClose(_handle)
#	define HK_READ(_handle, _buf, _nbytes) sceIoRead( _handle, _buf, _nbytes )
#	define HK_SEEK(_handle, _offset, _whence) sceIoLseek(_handle, _offset, _whence)
	HK_COMPILE_TIME_ASSERT(hkStreamReader::STREAM_SET==SCE_SEEK_SET
						&& hkStreamReader::STREAM_CUR==SCE_SEEK_CUR
						&& hkStreamReader::STREAM_END==SCE_SEEK_END );
#else
#	error unknown platform
#endif

class hkPosixStreamReader : public hkStreamReader
{
	public:

		hkPosixStreamReader( const char* nameIn )
		{
			const char* name = nameIn;
#		if defined(HK_PLATFORM_PS2) || defined(HK_PLATFORM_PSP)
			hkString p(nameIn);
			if ( (p.beginsWith("host0") == false) && (p.beginsWith("cdrom0") == false))
			{
				p = hkString("host0:") + p;
			}
			name = p.cString();
#		endif
			m_handle = HK_OPEN(name);
		}

		void close()
		{
			if(m_handle >= 0 )
			{
				HK_CLOSE(m_handle);
				m_handle = -1;
			}
		}

		virtual ~hkPosixStreamReader()
		{
			close();
		}

		virtual int read( void* buf, int nbytes)
		{
			if( m_handle >= 0 )
			{
				int nread = HK_READ( m_handle, buf, nbytes );
				if(nread <= 0)
				{
					close();
				}
				return nread;
			}
			return 0; 
		}

		virtual hkBool isOk() const
		{
			return m_handle >= 0;
		}

		virtual hkBool seekTellSupported() const
		{
			return true;
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

		int m_handle;
};

#undef HK_OPEN
#undef HK_CLOSE
#undef HK_READ
#undef HK_SEEK

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
