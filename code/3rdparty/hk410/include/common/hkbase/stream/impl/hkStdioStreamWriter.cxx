/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/string/hkString.h>
#include <hkbase/fwd/hkcstdio.h>
using namespace std;

class hkStdioStreamWriter : public hkStreamWriter
{
	public:

		hkStdioStreamWriter(hkStreambufFactory::StdStream s)
			:	m_owned(false)
		{
			m_handle = (s == hkStreambufFactory::STDERR) ? stderr : stdout;
		}

		hkStdioStreamWriter(const char* nameIn)
			:	m_owned(true)
		{
			if( nameIn )
			{
				open(nameIn);
			}
		}

		void open( const char* nameIn, const char* mode = "wb")
		{
			HK_ASSERT( 0x95867432, nameIn != HK_NULL );
			const char* name = nameIn;
#		if defined(HK_PLATFORM_XBOX) || defined (HK_PLATFORM_XBOX360)
			hkString p(nameIn);
			p = p.replace("/", "\\");
			int properStart = 0;
			while ( (p[properStart] == '.') || (p[properStart] == '\\'))
			{
				properStart++;
			}
			// might start with D already, or some other Xbox drive or cache nam, if not, add D:
			if (hkString::strChr(p.cString(), ':') == HK_NULL)
			{
				p = hkString("D:") + hkString(p.cString()[properStart] == '\\'? "" : "\\") + &(p.cString()[properStart]);
			}
			name = p.cString();
#		elif defined(HK_PLATFORM_PS3)
			hkString p(nameIn);
			if (!p.beginsWith("/")) // inc ./
				p = hkString("/app_home/") + p; // 050 by default mounts files under app_home
			name = p.cString();
#		endif
			m_handle = fopen(name, mode);
		}

		void close()
		{
			if(m_handle > 0 && m_owned)
			{
				fclose(m_handle);
			}
			m_handle = HK_NULL;
		}

		virtual ~hkStdioStreamWriter()
		{
			close();
		}

		virtual int write( const void* buf, int nbytes)
		{
			if( m_handle != HK_NULL )
			{
				int n = static_cast<int>( fwrite( buf, 1, nbytes, m_handle ) );
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
			if( m_handle != HK_NULL )
			{
				fflush(m_handle);
			}
		}

		virtual hkBool isOk() const
		{
			return m_handle != HK_NULL;
		}

		virtual hkBool markSupported() const
		{
#		ifdef HK_PLATFORM_PS3
			return false; // it techincaly does, but on current setups it is painfully slow to assume it (internal buffering slow)
#		else
			return true;
#		endif
		}

		virtual hkBool seekTellSupported() const
		{
			return true;
		}

		virtual hkResult seek( int offset, SeekWhence whence)
		{
			return fseek(m_handle, offset, whence) == 0
				? HK_SUCCESS
				: HK_FAILURE;
		}

		virtual int tell() const
		{
			return ftell(m_handle);
		}

	protected:

		FILE* m_handle;
		hkBool m_owned;
};

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
