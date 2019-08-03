/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/fwd/hkcstdio.h>
#include <dolphin.h>

/// A stream through DVD interface for GameCube
class hkGameCubeDvdReader : public hkStreamReader
{
	public:
	
		hkGameCubeDvdReader(const char* fname)
			:	m_fileOffset(-1),
				m_fileLen(-1)
		{
			if ( DVDOpen( const_cast<char*>(fname), &m_dvdInfo ) == TRUE )
			{
				m_fileLen = DVDGetLength( &m_dvdInfo );
				m_fileOffset = 0;
			}
		}
		
		virtual ~hkGameCubeDvdReader()
		{	
			if( m_fileLen != -1 )
			{
				DVDClose(&m_dvdInfo);
			}
		}
		
		virtual hkBool isOk() const
		{
			return m_fileLen >= 0 && m_fileOffset <= m_fileLen;
		}
		
		virtual int read(void* buf, int nbytes)
		{
			if( isOk() )
			{
				if ( (m_fileOffset + nbytes) > m_fileLen )
				{
					nbytes = m_fileLen - m_fileOffset;
					// round it up;
					nbytes = OSRoundUp32B(nbytes);
				}

				int r = DVDRead(&m_dvdInfo, buf, nbytes, m_fileOffset);
					
				if ( r >= 1 )
				{
					m_fileOffset += r;
					return r;
				}
				else
				{
					m_fileOffset = m_fileLen + 1;
				}
			}
			return 0;
		}

		virtual hkBool seekTellSupported() const
		{
			return true;
		}

		virtual hkResult seek( int offset, SeekWhence whence )
		{
			HK_ASSERT2(0x49a3f3bf,  (m_fileOffset& 0x03) == 0, "Seek position not multiple of 32 bytes");

			long realoffset = 0;
			switch(whence)
			{
				case STREAM_SET:
					realoffset = offset;
					break;
				case STREAM_CUR:
					realoffset = m_fileOffset + offset;
					break;
				case STREAM_END:
					realoffset = m_fileLen + offset;
					break;
				default:
					HK_ASSERT2(0x3b67976e, 0, "No such seek type");
			}

			HK_ASSERT(0x47a955c5, realoffset >= 0);
			HK_ASSERT(0x433f332e, realoffset <= m_fileLen);

			hkInt32 status = DVDSeek(&m_dvdInfo, realoffset);
			if(status == 0)
			{
				m_fileOffset = realoffset;
				return HK_SUCCESS;
			}
			else
			{
				return HK_FAILURE;
			}
		}

		virtual int tell() const
		{
			return m_fileOffset;
		}

	protected:
	
		DVDFileInfo m_dvdInfo;
		int m_fileOffset;
		int m_fileLen;
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
